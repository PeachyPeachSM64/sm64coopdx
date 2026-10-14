#version 130

// defined in gfx_pc.h
#define SHADER_FLAG_MAX  8

// defined in gfx_cc.h
#define SHADER_0         0
#define SHADER_INPUT_1   1
#define SHADER_INPUT_2   2
#define SHADER_INPUT_3   3
#define SHADER_INPUT_4   4
#define SHADER_INPUT_5   5
#define SHADER_INPUT_6   6
#define SHADER_INPUT_7   7
#define SHADER_INPUT_8   8
#define SHADER_TEXEL0    9
#define SHADER_TEXEL0A   10
#define SHADER_TEXEL1    11
#define SHADER_TEXEL1A   12
#define SHADER_1         13
#define SHADER_COMBINED  14
#define SHADER_COMBINEDA 15
#define SHADER_NOISE     16

// defined in gfx_cc.h
#define CCF_FLAG_COLOR_ALPHA_SAME   0   // 2 bits
#define CCF_FLAG_DO_SINGLE          2   // 4 bits
#define CCF_FLAG_DO_MULTIPLY        6   // 4 bits
#define CCF_FLAG_DO_MIX             10  // 4 bits
#define CCF_FLAG_DO_NOISE           14  // 1 bit
#define CCF_FLAG_USE_ALPHA          15  // 1 bit
#define CCF_FLAG_USE_FOG            16  // 1 bit
#define CCF_FLAG_TEX_EDGE           17  // 1 bit
#define CCF_FLAG_USE_DITHER         18  // 1 bit
#define CCF_FLAG_USE_2CYCLE         19  // 1 bit
#define CCF_FLAG_LIGHT_MAP          20  // 1 bit
#define CCF_FLAG_WORLD_GEOMETRY     21  // 1 bit
#define CCF_FLAG_USED_TEXTURE       22  // MAX_TEXTURES bits

#define CCF_HAS_FLAG(_flag_)        (((uCCF) & (1 << (CCF_FLAG_##_flag_        ))) != 0)
#define CCF_HAS_FLAG_i(_flag_, _i_) (((uCCF) & (1 << (CCF_FLAG_##_flag_ + (_i_)))) != 0)


out vec4 oFragColor;

in vec2 vTexCoord0;
in vec2 vTexCoord1;
in vec4 vFog;
in vec2 vLightMap;
in vec4 vInput1;
in vec4 vInput2;
in vec4 vInput3;
in vec4 vInput4;
in vec4 vInput5;
in vec4 vInput6;
in vec4 vInput7;
in vec4 vInput8;

uniform float uFrameCount;
uniform vec3  uLightMapColor;
uniform int   uShaderFlags[SHADER_FLAG_MAX];
uniform float uShaderFlagValues[SHADER_FLAG_MAX];

// CC features
uniform int uCCF;

// Combine mode
uniform int uShaderCommands[16];

// Textures
uniform sampler2D uTex0; uniform vec2 uTex0Size; uniform int uTex0Filter;
uniform sampler2D uTex1; uniform vec2 uTex1Size; uniform int uTex1Filter;

#define FILTER_THREE_POINT 2



vec4 noise = vec4(0.0);
vec4 texColor0 = vec4(0.0, 0.0, 0.0, 1.0);
vec4 texColor1 = vec4(0.0, 0.0, 0.0, 1.0);





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

vec4 sampleTex(in sampler2D tex, in vec2 uv, in vec2 texSize, in int filter) {
    if (filter == FILTER_THREE_POINT) {
        return filter3point(tex, uv, texSize);
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



vec4 shader_item_to_vec4(in vec4 texel, in int item) {
    switch (item) {
        case SHADER_0:         return vec4(0.0);
        case SHADER_1:         return vec4(1.0);
        case SHADER_INPUT_1:   return vInput1;
        case SHADER_INPUT_2:   return vInput2;
        case SHADER_INPUT_3:   return vInput3;
        case SHADER_INPUT_4:   return vInput4;
        case SHADER_INPUT_5:   return vInput5;
        case SHADER_INPUT_6:   return vInput6;
        case SHADER_INPUT_7:   return vInput7;
        case SHADER_INPUT_8:   return vInput8;
        case SHADER_TEXEL0:    return texColor0;
        case SHADER_TEXEL0A:   return texColor0.aaaa;
        case SHADER_TEXEL1:    return texColor1;
        case SHADER_TEXEL1A:   return texColor1.aaaa;
        case SHADER_COMBINED:  return texel;
        case SHADER_COMBINEDA: return texel.aaaa;
        case SHADER_NOISE:     return noise;
    }
    return vec4(0);
}

vec4 combine_colors(vec4 texel, int cycle, bool do_single, bool do_multiply, bool do_mix, bool is_alpha) {
    int offset = 8 * cycle + (is_alpha ? 4 : 0);

    // D
    if (do_single) {
        return shader_item_to_vec4(texel, uShaderCommands[offset + 3]);
    }

    // A * C
    if (do_multiply) {
        return shader_item_to_vec4(texel, uShaderCommands[offset + 0]) *
               shader_item_to_vec4(texel, uShaderCommands[offset + 2]);
    }

    // (A - B) * C
    if (do_mix) {
        return mix(
            shader_item_to_vec4(texel, uShaderCommands[offset + 1]),
            shader_item_to_vec4(texel, uShaderCommands[offset + 0]),
            shader_item_to_vec4(texel, uShaderCommands[offset + 2])
        );
    }

    // (A - B) * C + D
    return (
        shader_item_to_vec4(texel, uShaderCommands[offset + 0]) -
        shader_item_to_vec4(texel, uShaderCommands[offset + 1])
    ) * shader_item_to_vec4(texel, uShaderCommands[offset + 2])
      + shader_item_to_vec4(texel, uShaderCommands[offset + 3]);
}






void main() {
    vec4 texel = vec4(0.0, 0.0, 0.0, 0.0);

    // Tex colors
    if (CCF_HAS_FLAG_i(USED_TEXTURE, 0)) {
        texColor0 = sampleTex(uTex0, vTexCoord0, uTex0Size, uTex0Filter);
    }
    if (CCF_HAS_FLAG_i(USED_TEXTURE, 1)) {
        if (CCF_HAS_FLAG(LIGHT_MAP)) {
            texColor1 = sampleTex(uTex1, vLightMap, uTex1Size, uTex1Filter);
            texColor0.rgb *= uLightMapColor.rgb;
            texColor1.rgb = clamp(texColor1.rgb * texColor1.rgb + texColor1.rgb, 0.0, 1.0);
        } else {
            texColor1 = sampleTex(uTex1, vTexCoord1, uTex1Size, uTex1Filter);
        }
    }

    // Noise
    if ((CCF_HAS_FLAG(USE_ALPHA) && CCF_HAS_FLAG(USE_DITHER)) || CCF_HAS_FLAG(DO_NOISE)) {
        noise = vec4(random(floor(vec3(gl_FragCoord.xy, uFrameCount))));
    }

    // Color combiner
    int cycles = CCF_HAS_FLAG(USE_2CYCLE) ? 2 : 1;
    for (int cycle = 0, cycle_rgb = 0, cycle_a = 1; cycle < cycles; cycle++, cycle_rgb += 2, cycle_a += 2) {
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
