//
// VSInput is the vertex shader attributes.
// PSInput is the pixel shader inputs.
// PSInput's position must be `SV_POSITION`.
//

struct VSInput {
    float4 position                  : aVtxPos;
    float2 texCoords[MAX_TEXTURES]   : aTexCoords0;
    float4 inputs[MAX_SHADER_INPUTS] : aInputs0;
    float4 fog                       : aFog;
    float2 lightMap                  : aLightMap;
};

struct PSInput {
    float4 position                  : SV_POSITION;
    float2 texCoords[MAX_TEXTURES]   : vTexCoords0;
    float4 inputs[MAX_SHADER_INPUTS] : vInputs0;
    float4 fog                       : vFog;
    float2 lightMap                  : vLightMap;
};

PSInput VSMain(VSInput vs) {
    PSInput ps;
    for (int t = 0; t < MAX_TEXTURES; ++t) {
        ps.texCoords[t] = vs.texCoords[t];
    }
    for (int i = 0; i < MAX_SHADER_INPUTS; ++i) {
        ps.inputs[i] = vs.inputs[i];
    }
    ps.fog = vs.fog;
    ps.lightMap = vs.lightMap;
    ps.position = vs.position;
    return ps;
}
