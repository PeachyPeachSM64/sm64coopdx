//
// VSInput is the vertex shader attributes.
// PSInput is the pixel shader inputs.
// PSInput's position must be `SV_POSITION`.
//

struct VSInput {
    float4 position                : aVtxPos;
    float2 texCoords[MAX_TEXTURES] : aTexCoords0;
    float4 primitive               : aPrimitive;
    float4 shade                   : aShade;
    float4 environment             : aEnvironment;
    float4 lod                     : aLod;
    float4 fog                     : aFog;
    float2 lightMap                : aLightMap;
};

struct PSInput {
    float4 position                : SV_POSITION;
    float2 texCoords[MAX_TEXTURES] : vTexCoords0;
    float4 primitive               : vPrimitive;
    float4 shade                   : vShade;
    float4 environment             : vEnvironment;
    float4 lod                     : vLod;
    float4 fog                     : vFog;
    float2 lightMap                : vLightMap;
};

PSInput VSMain(VSInput vs) {
    PSInput ps;
    for (int t = 0; t < MAX_TEXTURES; ++t) {
        ps.texCoords[t] = vs.texCoords[t];
    }
    ps.primitive = vs.primitive;
    ps.shade = vs.shade;
    ps.environment = vs.environment;
    ps.lod = vs.lod;
    ps.fog = vs.fog;
    ps.lightMap = vs.lightMap;
    ps.position = vs.position;
    return ps;
}
