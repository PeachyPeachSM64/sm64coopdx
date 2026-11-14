// defined in gfx_pc.h
#define SHADER_FLAG_HUE             0
#define SHADER_FLAG_SATURATION      1
#define SHADER_FLAG_BRIGHTNESS      2
#define SHADER_FLAG_CONTRAST        3
#define SHADER_FLAG_EXPOSURE        4
#define SHADER_FLAG_DITHERING       5
#define SHADER_FLAG_POSTERIZATION   6
#define SHADER_FLAG_SCANLINES       7
#define SHADER_FLAG_MAX             8

#define FILTER_THREE_POINT          2

// Fragment inputs
static PSInput vInputs;

// Uniforms (packed)
cbuffer ShaderUniforms : register(b0) {
    uint4 uCCS[CC_CYCLES / 2];
    uint4 uFeatures;
    uint4 uTexProperties[MAX_TEXTURES]; // x is width, y is height, z is filter
    float4 uShaderFlagValues[(SHADER_FLAG_MAX + 3) / 4];
};

// Uniforms (unpacked)
#define uCCS_(cycle, alpha)     uCCS[(cycle) / 2][2 * ((cycle) % 2) + (alpha)]
#define uCCF                    uFeatures[0]
#define uNoiseRandomSeed        uFeatures[1]
#define uLightMapColor          uFeatures[2]
#define uShaderFlags            uFeatures[3]
#define uShaderFlagValue(flag)  uShaderFlagValues[(flag) / 4][(flag) % 4]

// CC features
#define CCF_GET(_flag_)       (uCCF >> CCF_##_flag_)
#define CCF_HAS_FLAG(_flag_)  ((CCF_GET(_flag_) & 1) != 0)

// Textures
Texture2D uTextures[MAX_TEXTURES] : register(t0);
SamplerState uTextureSamplers[MAX_TEXTURES] : register(s0);

// Extended combiners
cbuffer ExtendedCombiners : register(b1) {
    uint4  uExtCC_Ops[2 * (CC_EXT_MAX_OPS / 16)];
    float4 uExtCC_Constants[2 * CC_EXT_MAX_CONSTANTS];
};

//
// Color combiner
//

static float4 gNoise;
static float4 gTexColors[MAX_TEXTURES];

float4 ccs_to_vec4(in float4 texel, in uint ccs) {
    switch (ccs) {
        case CCS_0:         return float4(0.0, 0.0, 0.0, 0.0);
        case CCS_1:         return float4(1.0, 1.0, 1.0, 1.0);
        case CCS_COMBINED:  return texel;
        case CCS_COMBINEDA: return texel.aaaa;
        case CCS_PRIM:      return vInputs.primitive;
        case CCS_PRIMA:     return vInputs.primitive.aaaa;
        case CCS_SHADE:     return vInputs.shade;
        case CCS_SHADEA:    return vInputs.shade.aaaa;
        case CCS_ENV:       return vInputs.environment;
        case CCS_ENVA:      return vInputs.environment.aaaa;
        case CCS_LOD:       return vInputs.lod;
        case CCS_NOISE:     return gNoise;
        case CCS_AMBIENT:   return vInputs.ambient;
    }
    if (ccs >= CCS_LIGHT_FIRST && ccs <= CCS_LIGHT_LAST) {
        uint i = ccs - CCS_LIGHT_FIRST;
        return (i % 2 == 0 ?
                vInputs.lights[i / 2] :
                vInputs.lights[i / 2].aaaa);
    }
    if (ccs >= CCS_TEXEL_FIRST && ccs <= CCS_TEXEL_LAST) {
        uint i = ccs - CCS_TEXEL_FIRST;
        return (i % 2 == 0 ?
                gTexColors[i / 2] :
                gTexColors[i / 2].aaaa);
    }
    return float4(0.0, 0.0, 0.0, 0.0);
}

float4 combine_colors(in float4 texel, in uint ABCD) {
    #define A UNPACK(ABCD, 24, 8)
    #define B UNPACK(ABCD, 16, 8)
    #define C UNPACK(ABCD,  8, 8)
    #define D UNPACK(ABCD,  0, 8)

    // D
    if (C == CCS_0) {
        return ccs_to_vec4(texel, D);
    }

    // A * C
    if (B == CCS_0 && D == CCS_0) {
        return ccs_to_vec4(texel, A) *
               ccs_to_vec4(texel, C);
    }

    // (A - B) * C + B = lerp(C, B, A)
    if (B == D) {
        return lerp(
            ccs_to_vec4(texel, B),
            ccs_to_vec4(texel, A),
            ccs_to_vec4(texel, C)
        );
    }

    // (A - B) * C + D
    return (
        ccs_to_vec4(texel, A) -
        ccs_to_vec4(texel, B)
    ) * ccs_to_vec4(texel, C)
      + ccs_to_vec4(texel, D);
}

//
// Extended combiner
//

#define ARG(n) stack[top - (n)]
#define STACK_PUSH(args, operation) { \
    float4 res = (operation); \
    stack[top - (args)] = res; \
    top -= ((args) - 1); \
}

float cc_ext_dot_channel(in float4 arg1, in uint channel) {
    switch (channel) {
        case CC_EXT_DOT_PARAM_0: return 0.0;
        case CC_EXT_DOT_PARAM_1: return 1.0;
        case CC_EXT_DOT_PARAM_R: return arg1.r;
        case CC_EXT_DOT_PARAM_G: return arg1.g;
        case CC_EXT_DOT_PARAM_B: return arg1.b;
        case CC_EXT_DOT_PARAM_A: return arg1.a;
    }
    return 0.0;
}

float4 cc_ext_dot(in float4 arg1, in float4 arg2) {
    return float4(
        cc_ext_dot_channel(arg1, uint(arg2.r)),
        cc_ext_dot_channel(arg1, uint(arg2.g)),
        cc_ext_dot_channel(arg1, uint(arg2.b)),
        cc_ext_dot_channel(arg1, uint(arg2.a))
    );
}

float4 combine_colors_ext(in float4 texel, in uint alpha) {
    const uint offsetOps = alpha * (CC_EXT_MAX_OPS / 16);
    const uint offsetConstants = alpha * (CC_EXT_MAX_CONSTANTS);

    float4 stack[CC_EXT_STACK_SIZE];
    float4 registers[CC_EXT_MAX_REGISTERS];
    uint top = 0;

    for (uint i = 0; i < CC_EXT_MAX_OPS; ++i) {
        uint op = (uExtCC_Ops[offsetOps + (i / 16)][(i / 4) % 4] >> (8 * (i % 4))) & 0xFF;
        switch (op) {

            // Values
            case CC_EXT_0:          STACK_PUSH(0, float4(0.0, 0.0, 0.0, 0.0)); break;
            case CC_EXT_1:          STACK_PUSH(0, float4(1.0, 1.0, 1.0, 1.0)); break;
            case CC_EXT_COMBINED:   STACK_PUSH(0, texel); break;
            case CC_EXT_COMBINEDA:  STACK_PUSH(0, texel.aaaa); break;
            case CC_EXT_PRIM:       STACK_PUSH(0, vInputs.primitive); break;
            case CC_EXT_PRIMA:      STACK_PUSH(0, vInputs.primitive.aaaa); break;
            case CC_EXT_SHADE:      STACK_PUSH(0, vInputs.shade); break;
            case CC_EXT_SHADEA:     STACK_PUSH(0, vInputs.shade.aaaa); break;
            case CC_EXT_ENV:        STACK_PUSH(0, vInputs.environment); break;
            case CC_EXT_ENVA:       STACK_PUSH(0, vInputs.environment.aaaa); break;
            case CC_EXT_LOD:        STACK_PUSH(0, vInputs.lod); break;
            case CC_EXT_NOISE:      STACK_PUSH(0, gNoise); break;
            case CC_EXT_AMBIENT:    STACK_PUSH(0, vInputs.ambient); break;

            // Operators
            case CC_EXT_DOT:        STACK_PUSH(2, cc_ext_dot(ARG(1), ARG(2))); break;
            case CC_EXT_NEG:        STACK_PUSH(1, -ARG(1)); break;
            case CC_EXT_NOT:        STACK_PUSH(1, float4(!bool4(ARG(1)))); break;
            case CC_EXT_POW:        STACK_PUSH(2, pow(ARG(1), ARG(2))); break;
            case CC_EXT_MUL:        STACK_PUSH(2, (ARG(1) * ARG(2))); break;
            case CC_EXT_DIV:        STACK_PUSH(2, (ARG(1) / ARG(2))); break;
            case CC_EXT_MOD:        STACK_PUSH(2, fmod(ARG(1), ARG(2))); break;
            case CC_EXT_ADD:        STACK_PUSH(2, (ARG(1) + ARG(2))); break;
            case CC_EXT_SUB:        STACK_PUSH(2, (ARG(1) - ARG(2))); break;
            case CC_EXT_LT:         STACK_PUSH(2, float4(ARG(1) < ARG(2))); break;
            case CC_EXT_LEQ:        STACK_PUSH(2, float4(ARG(1) <= ARG(2))); break;
            case CC_EXT_GT:         STACK_PUSH(2, float4(ARG(1) > ARG(2))); break;
            case CC_EXT_GEQ:        STACK_PUSH(2, float4(ARG(1) >= ARG(2))); break;
            case CC_EXT_EQ:         STACK_PUSH(2, float4(ARG(1) == ARG(2))); break;
            case CC_EXT_NEQ:        STACK_PUSH(2, float4(ARG(1) != ARG(2))); break;
            case CC_EXT_AND:        STACK_PUSH(2, float4(bool4(ARG(1)) && bool4(ARG(2)))); break;
            case CC_EXT_OR:         STACK_PUSH(2, float4(bool4(ARG(1)) || bool4(ARG(2)))); break;
            case CC_EXT_SET:        STACK_PUSH(2, registers[uint(ARG(1).x)] = ARG(2)); break;
            case CC_EXT_END:        top = 0; break;

            // Functions
            case CC_EXT_VEC:        STACK_PUSH(4, float4(ARG(1).r, ARG(2).g, ARG(3).b, ARG(4).a)); break;
            case CC_EXT_LERP:       STACK_PUSH(3, lerp(ARG(1), ARG(2), ARG(3))); break;
            case CC_EXT_INVLERP:    STACK_PUSH(3, ((ARG(3) - ARG(1)) / (ARG(2) - ARG(1)))); break;
            case CC_EXT_REMAP:      STACK_PUSH(5, lerp(ARG(3), ARG(4), ((ARG(5) - ARG(1)) / (ARG(2) - ARG(1))))); break;
            case CC_EXT_IF:         STACK_PUSH(3, lerp(ARG(3), ARG(2), float4(bool4(ARG(1))))); break;
            case CC_EXT_ABS:        STACK_PUSH(1, abs(ARG(1))); break;
            case CC_EXT_MIN:        STACK_PUSH(2, min(ARG(1), ARG(2))); break;
            case CC_EXT_MAX:        STACK_PUSH(2, max(ARG(1), ARG(2))); break;
            case CC_EXT_CLAMP:      STACK_PUSH(3, clamp(ARG(1), ARG(2), ARG(3))); break;
            case CC_EXT_SATURATE:   STACK_PUSH(1, clamp(ARG(1), 0.0, 1.0)); break;
            case CC_EXT_COS:        STACK_PUSH(1, cos(ARG(1))); break;
            case CC_EXT_SIN:        STACK_PUSH(1, sin(ARG(1))); break;
            case CC_EXT_SQRT:       STACK_PUSH(1, sqrt(ARG(1))); break;
            case CC_EXT_RSQRT:      STACK_PUSH(1, rsqrt(ARG(1))); break;
            case CC_EXT_ROUND:      STACK_PUSH(1, round(ARG(1))); break;
            case CC_EXT_FLOOR:      STACK_PUSH(1, floor(ARG(1))); break;
            case CC_EXT_CEIL:       STACK_PUSH(1, ceil(ARG(1))); break;

            default:

            // Lights
            if (op >= CC_EXT_LIGHT_FIRST && op <= CC_EXT_LIGHT_LAST) {
                uint index = op - CC_EXT_LIGHT_FIRST;
                STACK_PUSH(0, index % 2 == 0 ?
                              vInputs.lights[index / 2] :
                              vInputs.lights[index / 2].aaaa);
                break;
            }

            // Textures
            if (op >= CC_EXT_TEXEL_FIRST && op <= CC_EXT_TEXEL_LAST) {
                uint index = op - CC_EXT_TEXEL_FIRST;
                STACK_PUSH(0, index % 2 == 0 ?
                              gTexColors[index / 2] :
                              gTexColors[index / 2].aaaa);
                break;
            }

            // Registers
            if (op >= CC_EXT_REGISTER_FIRST && op <= CC_EXT_REGISTER_LAST) {
                STACK_PUSH(0, registers[op - CC_EXT_REGISTER_FIRST]);
                break;
            }

            // Constants
            if (op >= CC_EXT_CONST_FIRST && op <= CC_EXT_CONST_LAST) {
                STACK_PUSH(0, uExtCC_Constants[offsetConstants + (op - CC_EXT_CONST_FIRST)]);
                break;
            }

            // Terminator
            return clamp(stack[0], 0.0, 1.0);
        }
    }
    return float4(0.0, 0.0, 0.0, 0.0);    
}

//
// Global shader effects
//

float dither4x4(in float2 position, in float brightness) {
    uint x = uint(fmod(position.x, 4.0));
    uint y = uint(fmod(position.y, 4.0));
    uint index = x + y * 4;
    float limit = float((((0x6E4CA280 - 0x10CD10CD * (index / 8)) >> ((index % 8) * 4)) & 0xF) + 1) / 16.0;
    return brightness < limit ? 0.0 : 1.0;
}

float3 rgb2hsv(in float3 c) {
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(
        abs(q.z + (q.w - q.y) / (6.0 * d + e)), // hue
        d / (q.x + e),                          // saturation
        q.x                                     // value
    );
}

float3 hsv2rgb(in float3 c) {
    float3 p = abs(frac(c.xxx + float3(0.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
    return c.z * lerp(float3(1.0, 1.0, 1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

float3 shader_effect_hue(in float3 texel, in float value) {
    float3 hsv = rgb2hsv(texel);
    hsv.x = frac(hsv.x + value);
    return hsv2rgb(hsv);
}

float3 shader_effect_saturation(in float3 texel, in float value) {
    const float3 w = float3(0.2125, 0.7154, 0.0721);
    float d = dot(texel, w);
    float3 intensity = float3(d, d, d);
    return lerp(intensity, texel, value);
}

float3 shader_effect_brightness(in float3 texel, in float value) {
    return texel * value;
}

float3 shader_effect_contrast(in float3 texel, in float value) {
    return 0.5 + value * (texel - 0.5);
}

float3 shader_effect_exposure(in float3 texel, in float value) {
    return texel + (value - 2.0) * texel + texel;
}

float3 shader_effect_dithering(in float3 texel, in float value) {
    return texel * dither4x4(vInputs.position.xy, dot(texel, float3(0.299, 0.587, 0.114)));
}

float3 shader_effect_posterization(in float3 texel, in float value) {
    uint levels = uint(max(1.0, value));
    return floor(texel * levels) / levels;
}

float3 shader_effect_scanlines(in float3 texel, in float value) {
    float scan = sin(vInputs.position.y * 1.5) * 0.04;
    return texel - scan * value;
}

//
// 3 point texture filtering
// Original author: ArthurCarvalho
// Modified GLSL implementation by twinaphex, mupen64plus-libretro project.
//

#define TEX_OFFSET(tex, tSampler, texCoord, off, texSize) tex.Sample(tSampler, texCoord - (off) / texSize)

float4 filter_3point(in Texture2D tex, in SamplerState tSampler, in float2 texCoord, in float2 texSize) {
    float2 offset = frac(texCoord * texSize - float2(0.5, 0.5));
    offset -= step(1.0, offset.x + offset.y);
    float4 c0 = TEX_OFFSET(tex, tSampler, texCoord, offset, texSize);
    float4 c1 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x - sign(offset.x), offset.y), texSize);
    float4 c2 = TEX_OFFSET(tex, tSampler, texCoord, float2(offset.x, offset.y - sign(offset.y)), texSize);
    return c0 + abs(offset.x) * (c1 - c0) + abs(offset.y) * (c2 - c0);
}

float4 sample_tex(in Texture2D tex, in SamplerState tSampler, in float2 uv, in float3 texProperties) {
    if (texProperties.z == FILTER_THREE_POINT) {
        return filter_3point(tex, tSampler, uv, texProperties.xy);
    }
    return tex.Sample(tSampler, uv);
}

//
// Noise
//

float random(in float3 value) {
    float random = dot(sin(value), float3(12.9898, 78.233, 37.719));
    return frac(sin(random) * 143758.5453);
}



float4 PSMain(PSInput ps) : SV_TARGET {
    float4 texel = float4(0.0, 0.0, 0.0, 0.0);
    vInputs = ps;

    // Flags
    bool useAlpha          = CCF_HAS_FLAG(USE_ALPHA);
    bool useFog            = CCF_HAS_FLAG(USE_FOG);
    bool texEdge           = CCF_HAS_FLAG(TEX_EDGE);
    bool useDither         = CCF_HAS_FLAG(USE_DITHER);
    bool use2Cycle         = CCF_HAS_FLAG(USE_2CYCLE);
    bool useLightMap       = CCF_HAS_FLAG(LIGHT_MAP);
    bool worldGeometry     = CCF_HAS_FLAG(WORLD_GEOMETRY);
    bool useNoise          = CCF_HAS_FLAG(USE_NOISE);
    bool useExtColor       = CCF_HAS_FLAG(USE_EXT_COLOR);
    bool useExtAlpha       = CCF_HAS_FLAG(USE_EXT_ALPHA);
    bool extColorAlphaSame = CCF_HAS_FLAG(EXT_COLOR_ALPHA_SAME);
    bool skipColor         = CCF_HAS_FLAG(SKIP_COMBINER_COLOR);
    bool skipAlpha         = CCF_HAS_FLAG(SKIP_COMBINER_ALPHA);

    // Tex colors
    [unroll]
    for (uint i = 0; i < MAX_TEXTURES; ++i) {
        if ((CCF_GET(USED_TEXTURE) & USED_TEXTURE(i)) != 0) {
            if (i == 1 && useLightMap) {
                float3 lightMapColor = float3(
                    float(UNPACK(uLightMapColor, 24, 8)) / 255.0,
                    float(UNPACK(uLightMapColor, 16, 8)) / 255.0,
                    float(UNPACK(uLightMapColor,  8, 8)) / 255.0
                );
                gTexColors[0].rgb *= lightMapColor;
                gTexColors[1] = sample_tex(uTextures[1], uTextureSamplers[1], vInputs.lightMap, uTexProperties[1].xyz);
                gTexColors[1].rgb = clamp(gTexColors[1].rgb * gTexColors[1].rgb + gTexColors[1].rgb, 0.0, 1.0);
            } else {
                gTexColors[i] = sample_tex(uTextures[i], uTextureSamplers[i], vInputs.texCoords[i], uTexProperties[i].xyz);
            }
        } else {
            gTexColors[i] = float4(0.0, 0.0, 0.0, 0.0);
        }
    }

    // Noise
    if ((useAlpha && useDither) || useNoise) {
        float noise = random(floor(float3(vInputs.position.xy, float(uNoiseRandomSeed))));
        gNoise = float4(noise, noise, noise, noise);
    }

    // Color combiner
    uint cycles = use2Cycle ? 2 : 1;
    for (uint cycle = 0; cycle < cycles; cycle++) {
        uint ABCD_rgb = uCCS_(cycle, 0);
        uint ABCD_a   = uCCS_(cycle, 1);

        // Same combiner, compute once for both RGB and alpha
        if ((ABCD_rgb == ABCD_a || !useAlpha) && !skipColor && !skipAlpha) {
            texel = combine_colors(texel, ABCD_rgb);
        }

        // Compute RGB and alpha separately
        else {
            if (!skipColor) {
                texel.rgb = combine_colors(texel, ABCD_rgb).rgb;
            }
            if (!skipAlpha) {
                texel.a = combine_colors(texel, ABCD_a).a;
            }
        }

        texel = fmod(texel + 0.5, 2.0) - 0.5;
    }
    texel = clamp(texel, 0.0, 1.0);

    // Extended combiners
    if (extColorAlphaSame) {
        texel = combine_colors_ext(texel, 0);
    } else {
        if (useExtColor) {
            texel.rgb = combine_colors_ext(texel, 0).rgb;
        }
        if (useExtAlpha) {
            texel.a = combine_colors_ext(texel, 1).a;
        }
    }

    // Tex edge alpha
    if (texEdge && useAlpha) {
        if (texel.a > 0.3) { texel.a = 1.0; }
        else discard;
    }

    // Global shader effects
    if (worldGeometry && uShaderFlags != 0) {
        for (uint i = 0; i < SHADER_FLAG_MAX; ++i) {
            if (UNPACK(uShaderFlags, i, 1) != 0) {
                float value = uShaderFlagValue(i);
                switch (i) {
                    case SHADER_FLAG_HUE:           texel.rgb = shader_effect_hue(texel.rgb, value); break;
                    case SHADER_FLAG_SATURATION:    texel.rgb = shader_effect_saturation(texel.rgb, value); break;
                    case SHADER_FLAG_BRIGHTNESS:    texel.rgb = shader_effect_brightness(texel.rgb, value); break;
                    case SHADER_FLAG_CONTRAST:      texel.rgb = shader_effect_contrast(texel.rgb, value); break;
                    case SHADER_FLAG_EXPOSURE:      texel.rgb = shader_effect_exposure(texel.rgb, value); break;
                    case SHADER_FLAG_DITHERING:     texel.rgb = shader_effect_dithering(texel.rgb, value); break;
                    case SHADER_FLAG_POSTERIZATION: texel.rgb = shader_effect_posterization(texel.rgb, value); break;
                    case SHADER_FLAG_SCANLINES:     texel.rgb = shader_effect_scanlines(texel.rgb, value); break;
                }
            }
        }
    }

    // Fog
    if (useFog) {
        if (useAlpha) {
            texel = float4(lerp(texel.rgb, vInputs.fog.rgb, vInputs.fog.a), texel.a);
        } else {
            texel = float4(lerp(texel.rgb, vInputs.fog.rgb, vInputs.fog.a), 1.0);
        }
    }

    // Dither
    if (useAlpha && useDither) {
        texel.a = gNoise.a < texel.a ? 1.0 : 0.0;
    }

    // Final color
    if (useAlpha) {
        return texel;
    }
    return float4(texel.rgb, 1.0);
}
