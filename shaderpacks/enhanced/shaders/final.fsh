#version 120

varying vec2 vTexCoord;

uniform sampler2D uCompositeTexture;

vec3 tonemapReinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

vec3 gammaCorrect(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}

void main() {
    vec4 color = texture2D(uCompositeTexture, vTexCoord);
    
    color.rgb = tonemapReinhard(color.rgb);
    color.rgb = gammaCorrect(color.rgb, 2.2);
    
    gl_FragColor = color;
}
