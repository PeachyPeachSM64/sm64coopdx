// aaaa
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



// CC sources
#define A ((ABCD >> 24) & 0xFF)
#define B ((ABCD >> 16) & 0xFF)
#define C ((ABCD >>  8) & 0xFF)
#define D ((ABCD >>  0) & 0xFF)

// CC features
#define CCF_GET(_flag_)       (uCCF >> CCF_##_flag_)
#define CCF_HAS_FLAG(_flag_)  ((CCF_GET(_flag_) & 1) != 0)


cbuffer ShaderConstants : register(b0) {
    int uCCS[CC_CHANNELS * CC_CYCLES];
    int uCCF;

    float3 uTexProperties[MAX_TEXTURES];

    float uNoiseFrameCount;
    float3 uLightMapColor;

    int uShaderFlags;
    float uShaderFlagValues[SHADER_FLAG_MAX];
};

Texture2D<float4> uTextures[MAX_TEXTURES] : register(t0);
SamplerState uTextureSamplers[MAX_TEXTURES] : register(s0);



//
// Color combiner
//

struct CCInputs {
    float4 inputs[MAX_SHADER_INPUTS];
    float4 texColors[MAX_TEXTURES];
    float4 noise;
};

float4 ccs_to_vec4(in CCInputs cc, in float4 texel, in int ccs) {
    switch (ccs) {
        case CCS_0:         return float4(0.0, 0.0, 0.0, 0.0);
        case CCS_1:         return float4(1.0, 1.0, 1.0, 1.0);
        case CCS_TEXEL0:    return cc.texColors[0];
        case CCS_TEXEL0A:   return cc.texColors[0].aaaa;
        case CCS_TEXEL1:    return cc.texColors[1];
        case CCS_TEXEL1A:   return cc.texColors[1].aaaa;
        case CCS_COMBINED:  return texel;
        case CCS_COMBINEDA: return texel.aaaa;
        case CCS_NOISE:     return cc.noise;
    }
    if (ccs >= SHADER_INPUT_FIRST && ccs <= SHADER_INPUT_LAST) {
        return cc.inputs[ccs - SHADER_INPUT_FIRST];
    }
    return float4(0.0, 0.0, 0.0, 0.0);
}

float4 combine_colors(in CCInputs cc, in float4 texel, in int ABCD) {

    // D
    if (C == CCS_0) {
        return ccs_to_vec4(cc, texel, D);
    }

    // A * C
    if (B == CCS_0 && D == CCS_0) {
        return ccs_to_vec4(cc, texel, A) *
               ccs_to_vec4(cc, texel, C);
    }

    // (A - B) * C + B = lerp(C, B, A)
    if (B == D) {
        return lerp(
            ccs_to_vec4(cc, texel, B),
            ccs_to_vec4(cc, texel, A),
            ccs_to_vec4(cc, texel, C)
        );
    }

    // (A - B) * C + D
    return (
        ccs_to_vec4(cc, texel, A) -
        ccs_to_vec4(cc, texel, B)
    ) * ccs_to_vec4(cc, texel, C)
      + ccs_to_vec4(cc, texel, D);
}

//
// Global shader effects
//

float dither4x4(in float2 position, in float brightness) {
    int x = int(fmod(position.x, 4.0));
    int y = int(fmod(position.y, 4.0));
    int index = x + y * 4;
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

float3 shader_effect_dithering(in float3 texel, in float2 value) {
    return texel * dither4x4(value, dot(texel, float3(0.299, 0.587, 0.114)));
}

float3 shader_effect_posterization(in float3 texel, in float value) {
    int levels = int(max(1.0, value));
    return floor(texel * levels) / levels;
}

float3 shader_effect_scanlines(in float3 texel, in float2 value) {
    float scan = sin(value.y * 1.5) * 0.04;
    return texel - scan * value.x;
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

    CCInputs cc;
    cc.inputs = ps.inputs;

    // Tex colors
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        if ((CCF_GET(USED_TEXTURE) & USED_TEXTURE(i)) != 0) {
            if (i == 1 && CCF_HAS_FLAG(LIGHT_MAP)) {
                cc.texColors[1] = sample_tex(uTextures[1], uTextureSamplers[1], ps.lightMap, uTexProperties[1]);
                cc.texColors[0].rgb *= uLightMapColor.rgb;
                cc.texColors[1].rgb = clamp(cc.texColors[1].rgb * cc.texColors[1].rgb + cc.texColors[1].rgb, 0.0, 1.0);
            } else {
                cc.texColors[i] = sample_tex(uTextures[i], uTextureSamplers[i], ps.texCoords[i], uTexProperties[i]);
            }
        }
    }

    // Noise
    if ((CCF_HAS_FLAG(USE_ALPHA) && CCF_HAS_FLAG(USE_DITHER)) || CCF_HAS_FLAG(NOISE)) {
        float noise = random(floor(float3(ps.position.xy, uNoiseFrameCount)));
        cc.noise = float4(noise, noise, noise, noise);
    }

    // Color combiner
    int cycles = CCF_HAS_FLAG(USE_2CYCLE) ? 2 : 1;
    for (int cycle = 0; cycle < cycles; cycle++) {
        int ABCD_rgb = uCCS[2 * cycle + 0];
        int ABCD_a   = uCCS[2 * cycle + 1];
        if (ABCD_rgb != ABCD_a && CCF_HAS_FLAG(USE_ALPHA)) {
            texel = float4(
                combine_colors(cc, texel, ABCD_rgb).rgb,
                combine_colors(cc, texel, ABCD_a).a
            );
        } else {
            texel = combine_colors(cc, texel, ABCD_rgb);
        }
        texel = fmod(texel + 0.5, 2.0) - 0.5;
    }
    texel = clamp(texel, 0.0, 1.0);

    // Tex edge alpha
    if (CCF_HAS_FLAG(TEX_EDGE) && CCF_HAS_FLAG(USE_ALPHA)) {
        if (texel.a > 0.3) texel.a = 1.0;
        else discard;
    }

    // Global shader effects
    if (CCF_HAS_FLAG(WORLD_GEOMETRY) && uShaderFlags != 0) {
        for (int i = 0; i < SHADER_FLAG_MAX; ++i) {
            if (((uShaderFlags >> i) & 1) != 0) {
                float value = uShaderFlagValues[i];
                switch (i) {
                    case SHADER_FLAG_HUE:           texel.rgb = shader_effect_hue(texel.rgb, value); break;
                    case SHADER_FLAG_SATURATION:    texel.rgb = shader_effect_saturation(texel.rgb, value); break;
                    case SHADER_FLAG_BRIGHTNESS:    texel.rgb = shader_effect_brightness(texel.rgb, value); break;
                    case SHADER_FLAG_CONTRAST:      texel.rgb = shader_effect_contrast(texel.rgb, value); break;
                    case SHADER_FLAG_EXPOSURE:      texel.rgb = shader_effect_exposure(texel.rgb, value); break;
                    case SHADER_FLAG_DITHERING:     texel.rgb = shader_effect_dithering(texel.rgb, ps.position.xy); break;
                    case SHADER_FLAG_POSTERIZATION: texel.rgb = shader_effect_posterization(texel.rgb, value); break;
                    case SHADER_FLAG_SCANLINES:     texel.rgb = shader_effect_scanlines(texel.rgb, float2(value, ps.position.y)); break;
                }
            }
        }
    }

    // Fog
    if (CCF_HAS_FLAG(USE_FOG)) {
        if (CCF_HAS_FLAG(USE_ALPHA)) {
            texel = float4(lerp(texel.rgb, ps.fog.rgb, ps.fog.a), texel.a);
        } else {
            texel = float4(lerp(texel.rgb, ps.fog.rgb, ps.fog.a), 1.0);
        }
    }

    // Dither
    if (CCF_HAS_FLAG(USE_ALPHA) && CCF_HAS_FLAG(USE_DITHER)) {
        texel.a = cc.noise.a < texel.a ? 1.0 : 0.0;
    }

    // Final color
    if (CCF_HAS_FLAG(USE_ALPHA)) {
        return texel;
    }
    return float4(texel.rgb, 1.0);
}
