#version 120

attribute vec4 aVtxPos;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uModelMatrix;

void main() {
    gl_Position = uLightSpaceMatrix * uModelMatrix * aVtxPos;
}
