//
// VSInput is the vertex shader attributes.
// PSInput is the pixel shader inputs.
// PSInput's position must be `SV_POSITION`.
//

// Vertex attributes (packed)
struct VSInput {
    float4 position                          : aVtxPos;
    float4 texCoords[PACKED_2(MAX_TEXTURES)] : aTexCoords0;
    int4 inputs                              : aInputs;
    int4 lights[PACKED_4(MAX_ACTIVE_LIGHTS)] : aLights0;
    int4 features                            : aFeatures;
};

// Fragment inputs
struct PSInput {
    float4 position                  : SV_POSITION;
    float2 texCoords[MAX_TEXTURES]   : vTexCoords0;
    float4 primitive                 : vPrimitive;
    float4 shade                     : vShade;
    float4 environment               : vEnvironment;
    float4 lod                       : vLod;
    float4 ambient                   : vAmbient;
    float4 lights[MAX_ACTIVE_LIGHTS] : vLights0;
    float4 fog                       : vFog;
    float2 lightMap                  : vLightMap;
};

float4 unpack_color(in int rgba) {
    return float4(
        float(UNPACK(rgba, 24, 8)) / 255.0,
        float(UNPACK(rgba, 16, 8)) / 255.0,
        float(UNPACK(rgba,  8, 8)) / 255.0,
        float(UNPACK(rgba,  0, 8)) / 255.0
    );
}

float2 unpack_light_map(in int uv) {
    return float2(
        float(UNPACK(uv, 16, 16)) / 65535.0,
        float(UNPACK(uv,  0, 16)) / 65535.0
    );
}

PSInput VSMain(VSInput vs) {
    PSInput ps;

    [unroll]
    for (int t = 0; t < MAX_TEXTURES; t++) {
        ps.texCoords[t].x = PACKED_GET_2(vs.texCoords, t, 0);
        ps.texCoords[t].y = PACKED_GET_2(vs.texCoords, t, 1);
    }

    [unroll]
    for (int i = 0; i < MAX_ACTIVE_LIGHTS; i++) {
        ps.lights[i] = unpack_color(PACKED_GET_4(vs.lights, i));
    }

    ps.primitive   = unpack_color(vs.inputs[0]);
    ps.shade       = unpack_color(vs.inputs[1]);
    ps.environment = unpack_color(vs.inputs[2]);
    ps.lod         = unpack_color(vs.inputs[3]).aaaa;
    ps.ambient     = float4(unpack_color(vs.inputs[3]).rgb, 1.0);
    ps.fog         = unpack_color(vs.features[0]);
    ps.lightMap    = unpack_light_map(vs.features[1]);

    ps.position = vs.position;
    return ps;
}
