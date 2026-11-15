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

// Output fragment color
out vec4 oFragColor;

// Fragment inputs
in vec2 vTexCoords[MAX_TEXTURES];
in vec4 vPrimitive;
in vec4 vShade;
in vec4 vEnvironment;
in vec4 vLod;
in vec4 vAmbient;
in vec4 vLights[MAX_ACTIVE_LIGHTS];
in vec4 vFog;
in vec2 vLightMap;

// Uniforms (packed)
uniform ivec4 uCCS;
uniform ivec4 uFeatures;
uniform ivec4 uTexProperties[MAX_TEXTURES]; // x is width, y is height, z is filter
uniform vec4 uShaderFlagValues[PACKED_4(SHADER_FLAG_MAX)];

// Uniforms (unpacked)
#define uCCS_(cycle, alpha)     uCCS[2 * (cycle) + (alpha)]
#define uCCF                    uFeatures[0]
#define uNoiseRandomSeed        uFeatures[1]
#define uLightMapColor          uFeatures[2]
#define uShaderFlags            uFeatures[3]
#define uShaderFlagValue(flag)  PACKED_GET_4(uShaderFlagValues, flag)

// CC features
#define CCF_GET(_flag_)         (uCCF >> CCF_##_flag_)
#define CCF_HAS_FLAG(_flag_)    ((CCF_GET(_flag_) & 1) != 0)

// Textures
uniform sampler2D uTextures[MAX_TEXTURES];

// Extended combiners
struct ExtendedCombiner {
    ivec4 ops[PACKED_16(CC_EXT_MAX_OPS)];
    vec4 constants[CC_EXT_MAX_CONSTANTS];
};
layout(std140) uniform ExtendedCombinerBlock {
    ExtendedCombiner uExtCC[2];
};

//
// Color combiner
//

vec4 gNoise;
vec4 gTexColors[MAX_TEXTURES];

vec4 ccs_to_vec4(in vec4 texel, in int ccs) {
    switch (ccs) {
        case CCS_0:         return vec4(0.0);
        case CCS_1:         return vec4(1.0);
        case CCS_COMBINED:  return texel;
        case CCS_COMBINEDA: return texel.aaaa;
        case CCS_PRIM:      return vPrimitive;
        case CCS_PRIMA:     return vPrimitive.aaaa;
        case CCS_SHADE:     return vShade;
        case CCS_SHADEA:    return vShade.aaaa;
        case CCS_ENV:       return vEnvironment;
        case CCS_ENVA:      return vEnvironment.aaaa;
        case CCS_LOD:       return vLod;
        case CCS_NOISE:     return gNoise;
        case CCS_AMBIENT:   return vAmbient;
    }
    if (ccs >= CCS_LIGHT_FIRST && ccs <= CCS_LIGHT_LAST) {
        int i = ccs - CCS_LIGHT_FIRST;
        return ((i & 1) == 0 ?
                vLights[i >> 1] :
                vLights[i >> 1].aaaa);
    }
    if (ccs >= CCS_TEXEL_FIRST && ccs <= CCS_TEXEL_LAST) {
        int i = ccs - CCS_TEXEL_FIRST;
        return ((i & 1) == 0 ?
                gTexColors[i >> 1] :
                gTexColors[i >> 1].aaaa);
    }
    return vec4(0);
}

vec4 combine_colors(in vec4 texel, in int ABCD) {
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
        return ccs_to_vec4(texel, A)
             * ccs_to_vec4(texel, C);
    }

    // (A - B) * C + B = lerp(B, A, C)
    if (B == D) {
        return mix(
            ccs_to_vec4(texel, B),
            ccs_to_vec4(texel, A),
            ccs_to_vec4(texel, C)
        );
    }

    // (A - B) * C + D
    return (
        ccs_to_vec4(texel, A)
      - ccs_to_vec4(texel, B)
    ) * ccs_to_vec4(texel, C)
      + ccs_to_vec4(texel, D);
}

//
// Extended combiner
//

// Note: in GLSL, declaring these as global helps reducing GPU register pressure
vec4 sStack[CC_EXT_STACK_SIZE];
vec4 sRegisters[CC_EXT_MAX_REGISTERS];

#define ARG(n) sStack[top - (n)]
#define STACK_PUSH(args, operation) { \
    vec4 res = (operation); \
    sStack[top - (args)] = res; \
    top -= ((args) - 1); \
}

float cc_ext_dot_channel(in vec4 arg1, in int channel) {
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

vec4 cc_ext_dot(in vec4 arg1, in vec4 arg2) {
    return vec4(
        cc_ext_dot_channel(arg1, int(arg2.r)),
        cc_ext_dot_channel(arg1, int(arg2.g)),
        cc_ext_dot_channel(arg1, int(arg2.b)),
        cc_ext_dot_channel(arg1, int(arg2.a))
    );
}

vec4 combine_colors_ext(in vec4 texel, in int alpha) {
    int top = 0;
    for (int i = 0; i < CC_EXT_MAX_OPS; ++i) {
        int op = PACKED_GET_16(uExtCC[alpha].ops, i);
        switch (op) {

            // Values
            case CC_EXT_0:          STACK_PUSH(0, vec4(0)); break;
            case CC_EXT_1:          STACK_PUSH(0, vec4(1)); break;
            case CC_EXT_COMBINED:   STACK_PUSH(0, texel); break;
            case CC_EXT_COMBINEDA:  STACK_PUSH(0, texel.aaaa); break;
            case CC_EXT_PRIM:       STACK_PUSH(0, vPrimitive); break;
            case CC_EXT_PRIMA:      STACK_PUSH(0, vPrimitive.aaaa); break;
            case CC_EXT_SHADE:      STACK_PUSH(0, vShade); break;
            case CC_EXT_SHADEA:     STACK_PUSH(0, vShade.aaaa); break;
            case CC_EXT_ENV:        STACK_PUSH(0, vEnvironment); break;
            case CC_EXT_ENVA:       STACK_PUSH(0, vEnvironment.aaaa); break;
            case CC_EXT_LOD:        STACK_PUSH(0, vLod); break;
            case CC_EXT_NOISE:      STACK_PUSH(0, gNoise); break;
            case CC_EXT_AMBIENT:    STACK_PUSH(0, vAmbient); break;

            // Operators
            case CC_EXT_DOT:        STACK_PUSH(2, cc_ext_dot(ARG(1), ARG(2))); break;
            case CC_EXT_NEG:        STACK_PUSH(1, -ARG(1)); break;
            case CC_EXT_NOT:        STACK_PUSH(1, vec4(not(bvec4(ARG(1))))); break;
            case CC_EXT_POW:        STACK_PUSH(2, pow(ARG(1), ARG(2))); break;
            case CC_EXT_MUL:        STACK_PUSH(2, (ARG(1) * ARG(2))); break;
            case CC_EXT_DIV:        STACK_PUSH(2, (ARG(1) / ARG(2))); break;
            case CC_EXT_MOD:        STACK_PUSH(2, mod(ARG(1), ARG(2))); break;
            case CC_EXT_ADD:        STACK_PUSH(2, (ARG(1) + ARG(2))); break;
            case CC_EXT_SUB:        STACK_PUSH(2, (ARG(1) - ARG(2))); break;
            case CC_EXT_LT:         STACK_PUSH(2, vec4(lessThan(ARG(1), ARG(2)))); break;
            case CC_EXT_LEQ:        STACK_PUSH(2, vec4(lessThanEqual(ARG(1), ARG(2)))); break;
            case CC_EXT_GT:         STACK_PUSH(2, vec4(greaterThan(ARG(1), ARG(2)))); break;
            case CC_EXT_GEQ:        STACK_PUSH(2, vec4(greaterThanEqual(ARG(1), ARG(2)))); break;
            case CC_EXT_EQ:         STACK_PUSH(2, vec4(equal(ARG(1), ARG(2)))); break;
            case CC_EXT_NEQ:        STACK_PUSH(2, vec4(notEqual(ARG(1), ARG(2)))); break;
            case CC_EXT_AND:        STACK_PUSH(2, vec4(bvec4(ARG(1)) && bvec4(ARG(2)))); break;
            case CC_EXT_OR:         STACK_PUSH(2, vec4(bvec4(ARG(1)) || bvec4(ARG(2)))); break;
            case CC_EXT_SET:        STACK_PUSH(2, sRegisters[int(ARG(1).x)] = ARG(2)); break;
            case CC_EXT_END:        top = 0; break;

            // Functions
            case CC_EXT_VEC:        STACK_PUSH(4, vec4(ARG(1).r, ARG(2).g, ARG(3).b, ARG(4).a)); break;
            case CC_EXT_LERP:       STACK_PUSH(3, mix(ARG(1), ARG(2), ARG(3))); break;
            case CC_EXT_INVLERP:    STACK_PUSH(3, ((ARG(3) - ARG(1)) / (ARG(2) - ARG(1)))); break;
            case CC_EXT_REMAP:      STACK_PUSH(5, mix(ARG(3), ARG(4), ((ARG(5) - ARG(1)) / (ARG(2) - ARG(1))))); break;
            case CC_EXT_IF:         STACK_PUSH(3, mix(ARG(3), ARG(2), vec4(bvec4(ARG(1))))); break;
            case CC_EXT_ABS:        STACK_PUSH(1, abs(ARG(1))); break;
            case CC_EXT_MIN:        STACK_PUSH(2, min(ARG(1), ARG(2))); break;
            case CC_EXT_MAX:        STACK_PUSH(2, max(ARG(1), ARG(2))); break;
            case CC_EXT_CLAMP:      STACK_PUSH(3, clamp(ARG(1), ARG(2), ARG(3))); break;
            case CC_EXT_SATURATE:   STACK_PUSH(1, clamp(ARG(1), 0.0, 1.0)); break;
            case CC_EXT_COS:        STACK_PUSH(1, cos(ARG(1))); break;
            case CC_EXT_SIN:        STACK_PUSH(1, sin(ARG(1))); break;
            case CC_EXT_SQRT:       STACK_PUSH(1, sqrt(ARG(1))); break;
            case CC_EXT_RSQRT:      STACK_PUSH(1, (1.0 / sqrt(ARG(1)))); break;
            case CC_EXT_ROUND:      STACK_PUSH(1, floor(ARG(1) + 0.5)); break;
            case CC_EXT_FLOOR:      STACK_PUSH(1, floor(ARG(1))); break;
            case CC_EXT_CEIL:       STACK_PUSH(1, ceil(ARG(1))); break;

            default:

            // Lights
            if (op >= CC_EXT_LIGHT_FIRST && op <= CC_EXT_LIGHT_LAST) {
                int index = op - CC_EXT_LIGHT_FIRST;
                STACK_PUSH(0, (index & 1) == 0 ?
                              vLights[index >> 1] :
                              vLights[index >> 1].aaaa);
                break;
            }

            // Textures
            if (op >= CC_EXT_TEXEL_FIRST && op <= CC_EXT_TEXEL_LAST) {
                int index = op - CC_EXT_TEXEL_FIRST;
                STACK_PUSH(0, (index & 1) == 0 ?
                              gTexColors[index >> 1] :
                              gTexColors[index >> 1].aaaa);
                break;
            }

            // Registers
            if (op >= CC_EXT_REGISTER_FIRST && op <= CC_EXT_REGISTER_LAST) {
                STACK_PUSH(0, sRegisters[op - CC_EXT_REGISTER_FIRST]);
                break;
            }

            // Constants
            if (op >= CC_EXT_CONST_FIRST && op <= CC_EXT_CONST_LAST) {
                STACK_PUSH(0, uExtCC[alpha].constants[op - CC_EXT_CONST_FIRST]);
                break;
            }

            // Terminator
            return clamp(sStack[0], 0.0, 1.0);
        }
    }
    return vec4(0);    
}

//
// Global shader effects
//

float dither4x4(in vec2 position, in float brightness) {
    int x = int(mod(position.x, 4.0));
    int y = int(mod(position.y, 4.0));
    int index = x + y * 4;
    float limit = float((((0x6E4CA280 - 0x10CD10CD * (index / 8)) >> ((index % 8) * 4)) & 0xF) + 1) / 16.0;
    return brightness < limit ? 0.0 : 1.0;
}

vec3 rgb2hsv(in vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(
        abs(q.z + (q.w - q.y) / (6.0 * d + e)), // hue
        d / (q.x + e),                          // saturation
        q.x                                     // value
    );
}

vec3 hsv2rgb(in vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

vec3 shader_effect_hue(in vec3 texel, in float value) {
    vec3 hsv = rgb2hsv(texel);
    hsv.x = fract(hsv.x + value);
    return hsv2rgb(hsv);
}

vec3 shader_effect_saturation(in vec3 texel, in float value) {
    const vec3 w = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(texel, w));
    return mix(intensity, texel, value);
}

vec3 shader_effect_brightness(in vec3 texel, in float value) {
    return texel * value;
}

vec3 shader_effect_contrast(in vec3 texel, in float value) {
    return 0.5 + value * (texel - 0.5);
}

vec3 shader_effect_exposure(in vec3 texel, in float value) {
    return texel + (value - 2.0) * texel + texel;
}

vec3 shader_effect_dithering(in vec3 texel, in float value) {
    return texel * dither4x4(gl_FragCoord.xy, dot(texel, vec3(0.299, 0.587, 0.114)));
}

vec3 shader_effect_posterization(in vec3 texel, in float value) {
    int levels = int(max(1.0, value));
    return floor(texel * levels) / levels;
}

vec3 shader_effect_scanlines(in vec3 texel, in float value) {
    float scan = sin(gl_FragCoord.y * 1.5) * 0.04;
    return texel - scan * value;
}

//
// 3 point texture filtering
// Original author: ArthurCarvalho
// Modified GLSL implementation by twinaphex, mupen64plus-libretro project.
//

#define TEX_OFFSET(off) texture2D(tex, texCoord - (off) / texSize)

vec4 filter_3point(in sampler2D tex, in vec2 texCoord, in ivec2 texSize) {
    vec2 offset = fract(texCoord * vec2(texSize.x, texSize.y) - vec2(0.5));
    offset -= step(1.0, offset.x + offset.y);
    vec4 c0 = TEX_OFFSET(offset);
    vec4 c1 = TEX_OFFSET(vec2(offset.x - sign(offset.x), offset.y));
    vec4 c2 = TEX_OFFSET(vec2(offset.x, offset.y - sign(offset.y)));
    return c0 + abs(offset.x) * (c1 - c0) + abs(offset.y) * (c2 - c0);
}

vec4 sample_tex(in sampler2D tex, in vec2 uv, in ivec3 texProperties) {
    if (texProperties.z == FILTER_THREE_POINT) {
        return filter_3point(tex, uv, texProperties.xy);
    }
    return texture2D(tex, uv);
}

//
// Noise
//

float random(in vec3 value) {
    float random = dot(sin(value), vec3(12.9898, 78.233, 37.719));
    return fract(sin(random) * 143758.5453);
}



void main() {
    vec4 texel = vec4(0.0, 0.0, 0.0, 0.0);

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
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        if ((CCF_GET(USED_TEXTURE) & USED_TEXTURE(i)) != 0) {
            if (i == 1 && useLightMap) {
                vec3 lightMapColor = vec3(
                    float(UNPACK(uLightMapColor, 24, 8)) / 255.0,
                    float(UNPACK(uLightMapColor, 16, 8)) / 255.0,
                    float(UNPACK(uLightMapColor,  8, 8)) / 255.0
                );
                gTexColors[0].rgb *= lightMapColor;
                gTexColors[1] = sample_tex(uTextures[1], vLightMap, uTexProperties[1].xyz);
                gTexColors[1].rgb = clamp(gTexColors[1].rgb * gTexColors[1].rgb + gTexColors[1].rgb, 0.0, 1.0);
            } else {
                gTexColors[i] = sample_tex(uTextures[i], vTexCoords[i], uTexProperties[i].xyz);
            }
        } else {
            gTexColors[i] = vec4(0);
        }
    }

    // Noise
    if ((useAlpha && useDither) || useNoise) {
        gNoise = vec4(random(floor(vec3(gl_FragCoord.xy, float(uNoiseRandomSeed)))));
    }

    // Color combiner
    int cycles = use2Cycle ? 2 : 1;
    for (int cycle = 0; cycle < cycles; cycle++) {
        int ABCD_rgb = uCCS_(cycle, 0);
        int ABCD_a   = uCCS_(cycle, 1);

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

        texel = mod(texel + 0.5, 2.0) - 0.5;
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
        for (int i = 0; i < SHADER_FLAG_MAX; ++i) {
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
            texel = vec4(mix(texel.rgb, vFog.rgb, vFog.a), texel.a);
        } else {
            texel = vec4(mix(texel.rgb, vFog.rgb, vFog.a), 1.0);
        }
    }

    // Dither
    if (useAlpha && useDither) {
        texel.a = gNoise.a < texel.a ? 1.0 : 0.0;
    }

    // Final color
    if (useAlpha) {
        oFragColor = texel;
    } else {
        oFragColor = vec4(texel.rgb, 1.0);
    }
}
