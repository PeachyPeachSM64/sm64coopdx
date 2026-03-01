#version 120

attribute vec4 aVtxPos;
attribute vec2 aTexCoord;
attribute vec3 aVertexNormal;
attribute vec3 aVertexPosition;

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vFragPos;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

void main() {
    vTexCoord = aTexCoord;
    vNormal = normalize(uNormalMatrix * aVertexNormal);
    vFragPos = vec3(uModelViewMatrix * vec4(aVertexPosition, 1.0));
    
    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);
}
