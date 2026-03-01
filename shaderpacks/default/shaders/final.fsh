#version 120

varying vec2 vTexCoord;

uniform sampler2D uCompositeTexture;

void main() {
    vec4 color = texture2D(uCompositeTexture, vTexCoord);
    gl_FragColor = color;
}
