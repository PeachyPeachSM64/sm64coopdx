#version 130

in vec4 aVtxPos;
in vec2 aTexCoord0; out vec2 vTexCoord0;
in vec2 aTexCoord1; out vec2 vTexCoord1;
in vec4 aFog;       out vec4 vFog;
in vec2 aLightMap;  out vec2 vLightMap;
in vec4 aInput1;    out vec4 vInput1;
in vec4 aInput2;    out vec4 vInput2;
in vec4 aInput3;    out vec4 vInput3;
in vec4 aInput4;    out vec4 vInput4;
in vec4 aInput5;    out vec4 vInput5;
in vec4 aInput6;    out vec4 vInput6;
in vec4 aInput7;    out vec4 vInput7;
in vec4 aInput8;    out vec4 vInput8;

void main() {
    vTexCoord0 = aTexCoord0;
    vTexCoord1 = aTexCoord1;
    vFog = aFog;
    vLightMap = aLightMap;
    vInput1 = aInput1;
    vInput2 = aInput2;
    vInput3 = aInput3;
    vInput4 = aInput4;
    vInput5 = aInput5;
    vInput6 = aInput6;
    vInput7 = aInput7;
    vInput8 = aInput8;
    gl_Position = aVtxPos;
}
