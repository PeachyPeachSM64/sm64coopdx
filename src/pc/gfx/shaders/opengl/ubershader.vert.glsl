#version 330

#define MAX_TEXTURES 2
#define MAX_SHADER_INPUTS 8

// Vertex position
in vec4 aVtxPos;

// Vertex attributes
in vec2 aTexCoords[MAX_TEXTURES];   out vec2 vTexCoords[MAX_TEXTURES];
in vec4 aInputs[MAX_SHADER_INPUTS]; out vec4 vInputs[MAX_SHADER_INPUTS];
in vec4 aFog;                       out vec4 vFog;
in vec2 aLightMap;                  out vec2 vLightMap;

void main() {
    for (int i = 0; i != MAX_TEXTURES; ++i) {
        vTexCoords[i] = aTexCoords[i];
    }
    for (int i = 0; i != MAX_SHADER_INPUTS; ++i) {
        vInputs[i] = aInputs[i];
    }
    vFog = aFog;
    vLightMap = aLightMap;
    gl_Position = aVtxPos;
}
