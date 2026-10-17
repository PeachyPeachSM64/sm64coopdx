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

// Vertex attributes
in vec2 vTexCoords[MAX_TEXTURES];
in vec4 vInputs[MAX_SHADER_INPUTS];
in vec4 vFog;
in vec2 vLightMap;

// CC sources
uniform int uCCS[CC_CHANNELS * CC_CYCLES];
#define A ((ABCD >> 24) & 0xFF)
#define B ((ABCD >> 16) & 0xFF)
#define C ((ABCD >>  8) & 0xFF)
#define D ((ABCD >>  0) & 0xFF)

// CC features
uniform int uCCF;
#define CCF_GET(_flag_)       (uCCF >> CCF_##_flag_)
#define CCF_HAS_FLAG(_flag_)  ((CCF_GET(_flag_) & 1) != 0)

// Textures
uniform sampler2D uTextures[MAX_TEXTURES];
uniform vec3 uTexProperties[MAX_TEXTURES]; // x is width, y is height, z is filter

// Other uniforms
uniform float uNoiseFrameCount;
uniform vec3  uLightMapColor;
uniform int   uShaderFlags;
uniform float uShaderFlagValues[SHADER_FLAG_MAX];

//
// Color combiner
//

vec4 noise;
vec4 texColors[MAX_TEXTURES];

vec4 ccs_to_vec4(in vec4 texel, in int ccs) {
    switch (ccs) {
        case CCS_0:         return vec4(0.0);
        case CCS_1:         return vec4(1.0);
        case CCS_TEXEL0:    return texColors[0];
        case CCS_TEXEL0A:   return texColors[0].aaaa;
        case CCS_TEXEL1:    return texColors[1];
        case CCS_TEXEL1A:   return texColors[1].aaaa;
        case CCS_COMBINED:  return texel;
        case CCS_COMBINEDA: return texel.aaaa;
        case CCS_NOISE:     return noise;
    }
    if (ccs >= SHADER_INPUT_FIRST && ccs <= SHADER_INPUT_LAST) {
        return vInputs[ccs - SHADER_INPUT_FIRST];
    }
    return vec4(0);
}

vec4 combine_colors(in vec4 texel, in int ABCD) {

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
        return mix(
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

vec3 shader_effect_scan_lines(in vec3 texel, in float value) {
    float scan = sin(gl_FragCoord.y * 1.5) * 0.04;
    return texel - scan * value;
}

//
// 3 point texture filtering
// Original author: ArthurCarvalho
// Modified GLSL implementation by twinaphex, mupen64plus-libretro project.
//

#define TEX_OFFSET(off) texture2D(tex, texCoord - (off) / texSize)

vec4 filter_3point(in sampler2D tex, in vec2 texCoord, in vec2 texSize) {
    vec2 offset = fract(texCoord * texSize - vec2(0.5));
    offset -= step(1.0, offset.x + offset.y);
    vec4 c0 = TEX_OFFSET(offset);
    vec4 c1 = TEX_OFFSET(vec2(offset.x - sign(offset.x), offset.y));
    vec4 c2 = TEX_OFFSET(vec2(offset.x, offset.y - sign(offset.y)));
    return c0 + abs(offset.x) * (c1 - c0) + abs(offset.y) * (c2 - c0);
}

vec4 sample_tex(in sampler2D tex, in vec2 uv, in vec3 texProperties) {
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

    // Tex colors
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        if ((CCF_GET(USED_TEXTURE) & USED_TEXTURE(i)) != 0) {
            if (i == 1 && CCF_HAS_FLAG(LIGHT_MAP)) {
                texColors[1] = sample_tex(uTextures[1], vLightMap, uTexProperties[1]);
                texColors[0].rgb *= uLightMapColor.rgb;
                texColors[1].rgb = clamp(texColors[1].rgb * texColors[1].rgb + texColors[1].rgb, 0.0, 1.0);
            } else {
                texColors[i] = sample_tex(uTextures[i], vTexCoords[i], uTexProperties[i]);
            }
        }
    }

    // Noise
    if ((CCF_HAS_FLAG(USE_ALPHA) && CCF_HAS_FLAG(USE_DITHER)) || CCF_HAS_FLAG(NOISE)) {
        noise = vec4(random(floor(vec3(gl_FragCoord.xy, uNoiseFrameCount))));
    }

    // Color combiner
    int cycles = CCF_HAS_FLAG(USE_2CYCLE) ? 2 : 1;
    for (int cycle = 0; cycle < cycles; cycle++) {
        int ABCD_rgb = uCCS[2 * cycle + 0];
        int ABCD_a   = uCCS[2 * cycle + 1];
        if (ABCD_rgb != ABCD_a && CCF_HAS_FLAG(USE_ALPHA)) {
            texel = vec4(
                combine_colors(texel, ABCD_rgb).rgb,
                combine_colors(texel, ABCD_a).a
            );
        } else {
            texel = combine_colors(texel, ABCD_rgb);
        }
        texel = mod(texel + 0.5, 2.0) - 0.5;
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
                    case SHADER_FLAG_DITHERING:     texel.rgb = shader_effect_dithering(texel.rgb, value); break;
                    case SHADER_FLAG_POSTERIZATION: texel.rgb = shader_effect_posterization(texel.rgb, value); break;
                    case SHADER_FLAG_SCANLINES:     texel.rgb = shader_effect_scan_lines(texel.rgb, value); break;
                }
            }
        }
    }

    // Fog
    if (CCF_HAS_FLAG(USE_FOG)) {
        if (CCF_HAS_FLAG(USE_ALPHA)) {
            texel = vec4(mix(texel.rgb, vFog.rgb, vFog.a), texel.a);
        } else {
            texel = vec4(mix(texel.rgb, vFog.rgb, vFog.a), 1.0);
        }
    }

    // Dither
    if (CCF_HAS_FLAG(USE_ALPHA) && CCF_HAS_FLAG(USE_DITHER)) {
        texel.a = noise.a < texel.a ? 1.0 : 0.0;
    }

    // Final color
    if (CCF_HAS_FLAG(USE_ALPHA)) {
        oFragColor = texel;
    } else {
        oFragColor = vec4(texel.rgb, 1.0);
    }
}
