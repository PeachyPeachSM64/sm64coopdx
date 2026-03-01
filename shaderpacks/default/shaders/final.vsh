#version 120

attribute vec4 aVtxPos;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = aVtxPos;
}
