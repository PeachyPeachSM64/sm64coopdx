// Other defines
#define SHADER_FLAG_MAX     8 // defined in gfx_pc.h
#define FILTER_THREE_POINT  2

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
#define CCF_HAS_FLAG(_flag_)        (((uCCF) & (1 << (CCF_FLAG_##_flag_        ))) != 0)
#define CCF_HAS_FLAG_i(_flag_, _i_) (((uCCF) & (1 << (CCF_FLAG_##_flag_ + (_i_)))) != 0)

// Textures
uniform sampler2D uTextures[MAX_TEXTURES];
uniform vec3 uTexProperties[MAX_TEXTURES]; // x is width, y is height, z is filter

// Other uniforms
uniform float uFrameCount;
uniform vec3  uLightMapColor;
uniform int   uShaderFlags[SHADER_FLAG_MAX];    // TODO: Bitfield
uniform float uShaderFlagValues[SHADER_FLAG_MAX];











// 3 point texture filtering
// Original author: ArthurCarvalho
// Modified GLSL implementation by twinaphex, mupen64plus-libretro project.

#define TEX_OFFSET(off) texture2D(tex, texCoord - (off)/texSize)

vec4 filter3point(in sampler2D tex, in vec2 texCoord, in vec2 texSize) {
    vec2 offset = fract(texCoord*texSize - vec2(0.5));
    offset -= step(1.0, offset.x + offset.y);
    vec4 c0 = TEX_OFFSET(offset);
    vec4 c1 = TEX_OFFSET(vec2(offset.x - sign(offset.x), offset.y));
    vec4 c2 = TEX_OFFSET(vec2(offset.x, offset.y - sign(offset.y)));
    return c0 + abs(offset.x)*(c1-c0) + abs(offset.y)*(c2-c0);
}

vec4 sampleTex(in sampler2D tex, in vec2 uv, in vec3 texProperties) {
    if (texProperties.z == FILTER_THREE_POINT) {
        return filter3point(tex, uv, texProperties.xy);
    }
    return texture2D(tex, uv);
}

float dither4x4(vec2 position, float brightness) {
    int x = int(mod(position.x, 4.0));
    int y = int(mod(position.y, 4.0));
    int index = x + y * 4;
    float limit = 0.0;
    if (x < 8) {
        if (index == 0) limit = 0.0625;
        if (index == 1) limit = 0.5625;
        if (index == 2) limit = 0.1875;
        if (index == 3) limit = 0.6875;
        if (index == 4) limit = 0.8125;
        if (index == 5) limit = 0.3125;
        if (index == 6) limit = 0.9375;
        if (index == 7) limit = 0.4375;
        if (index == 8) limit = 0.25;
        if (index == 9) limit = 0.75;
        if (index == 10) limit = 0.125;
        if (index == 11) limit = 0.625;
        if (index == 12) limit = 1.0;
        if (index == 13) limit = 0.5;
        if (index == 14) limit = 0.875;
        if (index == 15) limit = 0.375;
    }
    return brightness < limit ? 0.0 : 1.0;
}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
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

vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

float random(in vec3 value) {
    float random = dot(sin(value), vec3(12.9898, 78.233, 37.719));
    return fract(sin(random) * 143758.5453);
}




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

vec4 combine_colors(vec4 texel, int cycle, bool do_single, bool do_multiply, bool do_mix, bool is_alpha) {
    int ABCD = uCCS[2 * cycle + (is_alpha ? 1 : 0)];

    // D
    if (do_single) {
        return ccs_to_vec4(texel, D);
    }

    // A * C
    if (do_multiply) {
        return ccs_to_vec4(texel, A) *
               ccs_to_vec4(texel, C);
    }

    // (A - B) * C + B = lerp(C, B, A)
    if (do_mix) {
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






void main() {
    vec4 texel = vec4(0.0, 0.0, 0.0, 0.0);

    // Tex colors
    for (int i = 0; i != MAX_TEXTURES; ++i) {
        if (CCF_HAS_FLAG_i(USED_TEXTURE, i)) {
            if (i == 1 && CCF_HAS_FLAG(LIGHT_MAP)) {
                texColors[1] = sampleTex(uTextures[1], vLightMap, uTexProperties[1]);
                texColors[0].rgb *= uLightMapColor.rgb;
                texColors[1].rgb = clamp(texColors[1].rgb * texColors[1].rgb + texColors[1].rgb, 0.0, 1.0);
            } else {
                texColors[i] = sampleTex(uTextures[i], vTexCoords[i], uTexProperties[i]);
            }
        }
    }

    // Noise
    if ((CCF_HAS_FLAG(USE_ALPHA) && CCF_HAS_FLAG(USE_DITHER)) || CCF_HAS_FLAG(DO_NOISE)) {
        noise = vec4(random(floor(vec3(gl_FragCoord.xy, uFrameCount))));
    }

    // Color combiner
    int cycles = CCF_HAS_FLAG(USE_2CYCLE) ? 2 : 1;
    for (int cycle = 0, cycle_rgb = 0, cycle_a = 1; cycle < cycles; cycle++, cycle_rgb += CC_CHANNELS, cycle_a += CC_CHANNELS) {
        if (!CCF_HAS_FLAG_i(COLOR_ALPHA_SAME, cycle) && CCF_HAS_FLAG(USE_ALPHA)) {
            texel = vec4(
                combine_colors(texel, cycle, CCF_HAS_FLAG_i(DO_SINGLE, cycle_rgb), CCF_HAS_FLAG_i(DO_MULTIPLY, cycle_rgb), CCF_HAS_FLAG_i(DO_MIX, cycle_rgb), false).rgb,
                combine_colors(texel, cycle, CCF_HAS_FLAG_i(DO_SINGLE, cycle_a), CCF_HAS_FLAG_i(DO_MULTIPLY, cycle_a), CCF_HAS_FLAG_i(DO_MIX, cycle_a), true).a
            );
        } else {
            texel = combine_colors(texel, cycle, CCF_HAS_FLAG_i(DO_SINGLE, cycle_rgb), CCF_HAS_FLAG_i(DO_MULTIPLY, cycle_rgb), CCF_HAS_FLAG_i(DO_MIX, cycle_rgb), false);
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
    if (CCF_HAS_FLAG(WORLD_GEOMETRY)) {

        // hue
        if (uShaderFlags[0] == 1) {
            vec3 hsv = rgb2hsv(texel.rgb);
            hsv.x = fract(hsv.x + uShaderFlagValues[0]);
            texel.rgb = hsv2rgb(hsv);
        }

        // saturation
        if (uShaderFlags[1] == 1) {
            const vec3 w = vec3(0.2125, 0.7154, 0.0721);
            vec3 intensity = vec3(dot(texel.rgb, w));
            texel.rgb = mix(intensity, texel.rgb, uShaderFlagValues[1]);
        }

        // brightness
        if (uShaderFlags[2] == 1) {
            texel.rgb *= uShaderFlagValues[2];
        }

        // contrast
        if (uShaderFlags[3] == 1) {
            texel.rgb = 0.5 + uShaderFlagValues[3] * (texel.rgb - 0.5);
        }

        // exposure
        if (uShaderFlags[4] == 1) {
            texel.rgb = texel.rgb + (uShaderFlagValues[4] - 2) * texel.rgb + texel.rgb;
        }

        // dithering
        if (uShaderFlags[5] == 1) {
            texel.rgb *= dither4x4(gl_FragCoord.xy, dot(texel.rgb, vec3(0.299, 0.587, 0.114)));
        }

        // posterization
        if (uShaderFlags[6] == 1) {
            int levels = int(max(1.0, uShaderFlagValues[6]));
            texel.rgb = floor(texel.rgb * levels) / levels;
        }

        // scan lines
        if (uShaderFlags[7] == 1) {
            float scan = sin(gl_FragCoord.y * 1.5) * 0.04;
            texel.rgb -= scan * uShaderFlagValues[7];
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
