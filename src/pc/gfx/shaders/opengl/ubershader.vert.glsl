// Vertex position
in vec4 aVtxPos;

// Vertex attributes (packed)
in vec4 aTexCoords[PACKED_2(MAX_TEXTURES)];
in ivec4 aInputs;
in ivec4 aLights[PACKED_4(MAX_ACTIVE_LIGHTS)];
in ivec4 aFeatures;

// Fragment inputs
out vec2 vTexCoords[MAX_TEXTURES];
out vec4 vPrimitive;
out vec4 vShade;
out vec4 vEnvironment;
out vec4 vLod;
out vec4 vAmbient;
out vec4 vLights[MAX_ACTIVE_LIGHTS];
out vec4 vFog;
out vec2 vLightMap;

vec4 unpack_color(in int rgba) {
    return vec4(
        float(UNPACK(rgba, 24, 8)) / 255.0,
        float(UNPACK(rgba, 16, 8)) / 255.0,
        float(UNPACK(rgba,  8, 8)) / 255.0,
        float(UNPACK(rgba,  0, 8)) / 255.0
    );
}

vec2 unpack_light_map(in int uv) {
    return vec2(
        float(UNPACK(uv, 16, 16)) / 65535.0,
        float(UNPACK(uv,  0, 16)) / 65535.0
    );
}

void main() {
    for (int t = 0; t < MAX_TEXTURES; t++) {
        vTexCoords[t].x = PACKED_GET_2(aTexCoords, t, 0);
        vTexCoords[t].y = PACKED_GET_2(aTexCoords, t, 1);
    }

    for (int i = 0; i < MAX_ACTIVE_LIGHTS; i++) {
        vLights[i] = unpack_color(PACKED_GET_4(aLights, i));
    }

    vPrimitive   = unpack_color(aInputs[0]);
    vShade       = unpack_color(aInputs[1]);
    vEnvironment = unpack_color(aInputs[2]);
    vLod         = unpack_color(aInputs[3]).aaaa;
    vAmbient     = vec4(unpack_color(aInputs[3]).rgb, 1.0);
    vFog         = unpack_color(aFeatures[0]);
    vLightMap    = unpack_light_map(aFeatures[1]);

    gl_Position = aVtxPos;
}
