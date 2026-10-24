// Vertex position
in vec4 aVtxPos;

// Vertex attributes
in vec2 aTexCoords[MAX_TEXTURES]; out vec2 vTexCoords[MAX_TEXTURES];
in vec4 aPrimitive;               out vec4 vPrimitive;
in vec4 aShade;                   out vec4 vShade;
in vec4 aEnvironment;             out vec4 vEnvironment;
in vec4 aLod;                     out vec4 vLod;
in vec4 aFog;                     out vec4 vFog;
in vec2 aLightMap;                out vec2 vLightMap;

void main() {
    for (int i = 0; i != MAX_TEXTURES; ++i) {
        vTexCoords[i] = aTexCoords[i];
    }
    vPrimitive = aPrimitive;
    vShade = aShade;
    vEnvironment = aEnvironment;
    vLod = aLod;
    vFog = aFog;
    vLightMap = aLightMap;
    gl_Position = aVtxPos;
}
