// Vertex position
in vec4 aVtxPos;

// Vertex attributes
in vec4 aTexCoords[MAX_TEXTURES / 2];
in ivec4 aInputs;
in ivec4 aFeatures;

// Fragment inputs
out vec2 vTexCoords[MAX_TEXTURES];
out vec4 vPrimitive;
out vec4 vShade;
out vec4 vEnvironment;
out vec4 vLod;
out vec4 vAmbient;
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
    for (int i = 0; i < MAX_TEXTURES; i += 2) {
        vTexCoords[i + 0] = aTexCoords[i / 2].xy;
        vTexCoords[i + 1] = aTexCoords[i / 2].zw;
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
