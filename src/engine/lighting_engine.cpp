#include "lighting_engine.h"
extern "C" {
#include "math_util.h"
#include "pc/lua/smlua.h"
#include <float.h>
}
#undef clamp
#undef min
#undef max
#include <vector>
#include <algorithm>
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#define C_FIELD extern "C"

struct LELight
{
    s16 id;
    Vec3f pos;
    Color color;
    f32 radius;
    f32 intensity;
    bool useSurfaceNormals;
};

Color gLEAmbientColor = { 127, 127, 127 };
static std::vector<LELight> sLightPool;
static std::vector<LELight*> sActiveLights;
static s16 sLightID = -1;
static enum LEMode sMode = LE_MODE_AFFECT_ALL_SHADED_AND_COLORED;
static enum LEToneMapping sToneMapping = LE_TONE_MAPPING_WEIGHTED;
static bool sEnabled = false;
static u8 sMaxLightsPerVertex = 4;

static u32 sLightRevision = 1;
static u32 sLightBoundsRevision = 0;
static bool sHasInfluentialLights = false;
static Vec3f sLightBoundsMin = { 0 };
static Vec3f sLightBoundsMax = { 0 };

  ///////////
 // utils //
///////////

static inline f32 rsqrt(f32 value) {
#ifdef __SSE__
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(value)));
#else
    return 1.0f / sqrtf(value);
#endif
}

static inline void color_set(Color color, u8 r, u8 g, u8 b) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
}

static inline void color_copy(Color dest, Color src) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
}

static inline u8 clamp_u8(f32 value) {
    s32 v = (s32) value;
    if (v < 0) { return 0; }
    if (v > UINT8_MAX) { return UINT8_MAX; }
    return (u8) v;
}

  ///////////
 // cache //
///////////

#define LE_CACHE_CELL_SIZE 64
#define LE_CACHE_SIZE 8192

struct LECachedSample {
    u32 frame;
    u32 revision;
    s32 qx, qy, qz;
    u32 normalPacked;
    u16 scalarQ;
    u16 pad;
    Vec3f color;
    f32 weight;
    bool valid;
};

static struct LECachedSample sLECache[LE_CACHE_SIZE] = { 0 };

static inline s32 le_quantize_pos(f32 v) {
    return (s32) floorf(v / (f32) LE_CACHE_CELL_SIZE);
}

static inline u16 le_quantize_scalar(f32 s) {
    s32 q = (s32) (s * 16.0f);
    if (q < 0) { return 0; }
    if (q > UINT16_MAX) { return UINT16_MAX; }
    return (u16) q;
}

static inline u32 le_pack_normal(Vec3f n) {
    if (n == NULL) { return 0; }
    f32 nx = std::clamp(n[0], -1.0f, 1.0f);
    f32 ny = std::clamp(n[1], -1.0f, 1.0f);
    f32 nz = std::clamp(n[2], -1.0f, 1.0f);
    u32 ix = (u32) (s32) ((nx * 127.0f) + 128.0f);
    u32 iy = (u32) (s32) ((ny * 127.0f) + 128.0f);
    u32 iz = (u32) (s32) ((nz * 127.0f) + 128.0f);
    if (ix > 255) { ix = 255; }
    if (iy > 255) { iy = 255; }
    if (iz > 255) { iz = 255; }
    return (ix) | (iy << 8) | (iz << 16);
}

static inline u32 le_hash_key(s32 qx, s32 qy, s32 qz, u32 normalPacked, u16 scalarQ) {
    u32 h = (u32) qx * 0x9E3779B1u;
    h ^= (u32) qy * 0x85EBCA77u;
    h ^= (u32) qz * 0xC2B2AE3Du;
    h ^= normalPacked * 0x27D4EB2Du;
    h ^= (u32) scalarQ * 0x165667B1u;
    return h;
}

static bool le_cache_get(Vec3f pos, Vec3f normal, f32 lightIntensityScalar, Vec3f outColor, f32* outWeight) {
    s32 qx = le_quantize_pos(pos[0]);
    s32 qy = le_quantize_pos(pos[1]);
    s32 qz = le_quantize_pos(pos[2]);
    u32 normalPacked = le_pack_normal(normal);
    u16 scalarQ = le_quantize_scalar(lightIntensityScalar);

    u32 idx = le_hash_key(qx, qy, qz, normalPacked, scalarQ) & (LE_CACHE_SIZE - 1);
    struct LECachedSample* e = &sLECache[idx];
    if (!e->valid) { return false; }
    if (e->frame != gGlobalTimer) { return false; }
    if (e->revision != sLightRevision) { return false; }
    if (e->qx != qx || e->qy != qy || e->qz != qz) { return false; }
    if (e->normalPacked != normalPacked) { return false; }
    if (e->scalarQ != scalarQ) { return false; }

    vec3f_copy(outColor, e->color);
    *outWeight = e->weight;
    return true;
}

static void le_cache_put(Vec3f pos, Vec3f normal, f32 lightIntensityScalar, Vec3f inColor, f32 inWeight) {
    s32 qx = le_quantize_pos(pos[0]);
    s32 qy = le_quantize_pos(pos[1]);
    s32 qz = le_quantize_pos(pos[2]);
    u32 normalPacked = le_pack_normal(normal);
    u16 scalarQ = le_quantize_scalar(lightIntensityScalar);

    u32 idx = le_hash_key(qx, qy, qz, normalPacked, scalarQ) & (LE_CACHE_SIZE - 1);
    struct LECachedSample* e = &sLECache[idx];
    e->frame = gGlobalTimer;
    e->revision = sLightRevision;
    e->qx = qx;
    e->qy = qy;
    e->qz = qz;
    e->normalPacked = normalPacked;
    e->scalarQ = scalarQ;
    vec3f_copy(e->color, inColor);
    e->weight = inWeight;
    e->valid = true;
}

  ////////////
 // engine //
////////////

static void le_recompute_light_bounds(void) {
    sHasInfluentialLights = false;
    sLightBoundsMin[0] = FLT_MAX;
    sLightBoundsMin[1] = FLT_MAX;
    sLightBoundsMin[2] = FLT_MAX;
    sLightBoundsMax[0] = -FLT_MAX;
    sLightBoundsMax[1] = -FLT_MAX;
    sLightBoundsMax[2] = -FLT_MAX;

    for (const auto light : sActiveLights) {
        if (light->intensity <= 0.0f || light->radius <= 0.0f) { continue; }
        sLightBoundsMin[0] = std::min(sLightBoundsMin[0], light->pos[0] - light->radius);
        sLightBoundsMin[1] = std::min(sLightBoundsMin[1], light->pos[1] - light->radius);
        sLightBoundsMin[2] = std::min(sLightBoundsMin[2], light->pos[2] - light->radius);
        sLightBoundsMax[0] = std::max(sLightBoundsMax[0], light->pos[0] + light->radius);
        sLightBoundsMax[1] = std::max(sLightBoundsMax[1], light->pos[1] + light->radius);
        sLightBoundsMax[2] = std::max(sLightBoundsMax[2], light->pos[2] + light->radius);
        sHasInfluentialLights = true;
    }

    sLightBoundsRevision = sLightRevision;
}

static inline bool le_pos_may_be_affected_by_lights(Vec3f pos) {
    if (sLightBoundsRevision != sLightRevision) {
        le_recompute_light_bounds();
    }
    if (!sHasInfluentialLights) { return false; }
    if (pos[0] < sLightBoundsMin[0] || pos[0] > sLightBoundsMax[0]) { return false; }
    if (pos[1] < sLightBoundsMin[1] || pos[1] > sLightBoundsMax[1]) { return false; }
    if (pos[2] < sLightBoundsMin[2] || pos[2] > sLightBoundsMax[2]) { return false; }
    return true;
}

C_FIELD bool le_is_enabled(void) {
    // this is needed because we don't want to make vanilla darker,
    // and we don't want to set the ambient color to { 255, 255, 255 }
    // because then no one could see the effect of their lights
    return sEnabled;
}

C_FIELD void le_set_mode(enum LEMode mode) {
    sMode = mode;
}

C_FIELD enum LEMode le_get_mode(void) {
    return sMode;
}

C_FIELD void le_set_tone_mapping(enum LEToneMapping toneMapping) {
    sToneMapping = toneMapping;
}

C_FIELD void le_get_ambient_color(VEC_OUT Color out) {
    color_copy(out, gLEAmbientColor);
}

C_FIELD void le_set_ambient_color(u8 r, u8 g, u8 b) {
    if (gLEAmbientColor[0] == r && gLEAmbientColor[1] == g && gLEAmbientColor[2] == b) {
        sEnabled = true;
        return;
    }
    color_set(gLEAmbientColor, r, g, b);
    sEnabled = true;
    sLightRevision++;
}

C_FIELD void le_set_max_lights_per_vertex(u8 count) {
    sMaxLightsPerVertex = count;
}

static inline void le_tone_map_total_weighted(Color out, Color inAmbient, Vec3f inColor, f32 weight) {
    out[0] = clamp_u8((inAmbient[0] + inColor[0]) / weight);
    out[1] = clamp_u8((inAmbient[1] + inColor[1]) / weight);
    out[2] = clamp_u8((inAmbient[2] + inColor[2]) / weight);
}

static inline void le_tone_map_weighted(Color out, Color inAmbient, Vec3f inColor, f32 weight) {
    out[0] = clamp_u8(inAmbient[0] + (inColor[0] / weight));
    out[1] = clamp_u8(inAmbient[1] + (inColor[1] / weight));
    out[2] = clamp_u8(inAmbient[2] + (inColor[2] / weight));
}

static inline void le_tone_map_clamp(Color out, Color inAmbient, Vec3f inColor) {
    out[0] = clamp_u8(inAmbient[0] + inColor[0]);
    out[1] = clamp_u8(inAmbient[1] + inColor[1]);
    out[2] = clamp_u8(inAmbient[2] + inColor[2]);
}

static inline void le_tone_map_reinhard(Color out, Color inAmbient, Vec3f inColor) {
    inColor[0] += inAmbient[0];
    inColor[1] += inAmbient[1];
    inColor[2] += inAmbient[2];

    out[0] = clamp_u8((inColor[0] / (inColor[0] + 255.0f)) * 255.0f);
    out[1] = clamp_u8((inColor[1] / (inColor[1] + 255.0f)) * 255.0f);
    out[2] = clamp_u8((inColor[2] / (inColor[2] + 255.0f)) * 255.0f);
}

static void le_tone_map(Color out, Color inAmbient, Vec3f inColor, f32 weight) {
    switch (sToneMapping) {
        case LE_TONE_MAPPING_TOTAL_WEIGHTED: le_tone_map_total_weighted(out, inAmbient, inColor, weight); break;
        case LE_TONE_MAPPING_WEIGHTED:       le_tone_map_weighted(out, inAmbient, inColor, weight);       break;
        case LE_TONE_MAPPING_CLAMP:          le_tone_map_clamp(out, inAmbient, inColor);                  break;
        case LE_TONE_MAPPING_REINHARD:       le_tone_map_reinhard(out, inAmbient, inColor);               break;
    }
}

static void le_update_active_lights() {
    sActiveLights.clear();
    for (auto& light : sLightPool) {
        if (light.intensity > 0.0f && light.radius > 0.0f) {
            sActiveLights.push_back(&light);
        }
    }
}

static inline OPTIMIZE_O3 void le_calculate_light_contribution(const LELight& light, Vec3f pos, Vec3f normal, f32 lightIntensityScalar, Vec3f outColor, f32& weight, u8& contribution) {
    // vector to light
    f32 diffX = light.pos[0] - pos[0];
    f32 diffY = light.pos[1] - pos[1];
    f32 diffZ = light.pos[2] - pos[2];

    // squared distance check
    f32 dist2 = (diffX * diffX) + (diffY * diffY) + (diffZ * diffZ);
    f32 radius2 = light.radius * light.radius;
    if (dist2 > radius2 || dist2 <= 0) { return; }

    // attenuation & intensity
    f32 att = 1.0f - (dist2 / radius2);
    f32 brightness = att * light.intensity * lightIntensityScalar;

    if (light.useSurfaceNormals && normal) {
        // normalize diff
        f32 invLen = rsqrt(dist2);
        diffX *= invLen;
        diffY *= invLen;
        diffZ *= invLen;

        // lambert term
        f32 nl = (normal[0] * diffX) + (normal[1] * diffY) + (normal[2] * diffZ);
        if (nl <= 0.0f) { return; }

        // modulate by normal
        brightness *= nl;
    }

    // accumulate
    outColor[0] += light.color[0] * brightness;
    outColor[1] += light.color[1] * brightness;
    outColor[2] += light.color[2] * brightness;
    weight += brightness;
    contribution++;
}

C_FIELD OPTIMIZE_O3 void le_calculate_vertex_lighting(const Vtx_t* v, Vec3f pos, VEC_OUT Color out) {
    if (!le_pos_may_be_affected_by_lights(pos)) {
        out[0] = (u8) (v->cn[0] * (gLEAmbientColor[0] / 255.0f));
        out[1] = (u8) (v->cn[1] * (gLEAmbientColor[1] / 255.0f));
        out[2] = (u8) (v->cn[2] * (gLEAmbientColor[2] / 255.0f));
        return;
    }

    // clear color
    Vec3f color = { 0 };

    // accumulate lighting
    f32 weight = 1.0f;
    if (!le_cache_get(pos, NULL, 1.0f, color, &weight)) {
        vec3f_set(color, 0, 0, 0);
        weight = 1.0f;
        u8 contribution = 0;
        for (LELight* light : sActiveLights) {
            le_calculate_light_contribution(*light, pos, NULL, 1.0f, color, weight, contribution);
            if (contribution == sMaxLightsPerVertex) { break; }
        }
        le_cache_put(pos, NULL, 1.0f, color, weight);
    }

    // tone map and output
    Color vtxAmbient = {
        (u8)(v->cn[0] * (gLEAmbientColor[0] / 255.0f)),
        (u8)(v->cn[1] * (gLEAmbientColor[1] / 255.0f)),
        (u8)(v->cn[2] * (gLEAmbientColor[2] / 255.0f)),
    };
    le_tone_map(out, vtxAmbient, color, weight);
}

C_FIELD OPTIMIZE_O3 void le_calculate_lighting_color(Vec3f pos, VEC_OUT Color out, f32 lightIntensityScalar) {
    if (!le_pos_may_be_affected_by_lights(pos)) {
        color_copy(out, gLEAmbientColor);
        return;
    }

    // clear color
    Vec3f color = { 0 };

    // accumulate lighting
    f32 weight = 1.0f;
    if (!le_cache_get(pos, NULL, lightIntensityScalar, color, &weight)) {
        vec3f_set(color, 0, 0, 0);
        weight = 1.0f;
        u8 contribution = 0;
        for (LELight* light : sActiveLights) {
            le_calculate_light_contribution(*light, pos, NULL, lightIntensityScalar, color, weight, contribution);
            if (contribution == sMaxLightsPerVertex) { break; }
        }
        le_cache_put(pos, NULL, lightIntensityScalar, color, weight);
    }

    // tone map and output
    le_tone_map(out, gLEAmbientColor, color, weight);
}

C_FIELD OPTIMIZE_O3 void le_calculate_lighting_color_with_normal(Vec3f pos, Vec3f normal, VEC_OUT Color out, f32 lightIntensityScalar) {
    if (!le_pos_may_be_affected_by_lights(pos)) {
        color_copy(out, gLEAmbientColor);
        return;
    }

    // normalize normal
    if (normal) { vec3f_normalize(normal); }

    // clear color
    Vec3f color = { 0 };

    // accumulate lighting
    f32 weight = 1.0f;
    if (!le_cache_get(pos, normal, lightIntensityScalar, color, &weight)) {
        vec3f_set(color, 0, 0, 0);
        weight = 1.0f;
        u8 contribution = 0;
        for (LELight* light : sActiveLights) {
            le_calculate_light_contribution(*light, pos, normal, lightIntensityScalar, color, weight, contribution);
            if (contribution == sMaxLightsPerVertex) { break; }
        }
        le_cache_put(pos, normal, lightIntensityScalar, color, weight);
    }

    // tone map and output
    le_tone_map(out, gLEAmbientColor, color, weight);
}

C_FIELD void le_calculate_lighting_dir(Vec3f pos, VEC_OUT Vec3f out) {
    Vec3f lightingDir = { 0, 0, 0 };
    s16 count = 1;

    for (LELight* light : sActiveLights) {
        f32 diffX = light->pos[0] - pos[0];
        f32 diffY = light->pos[1] - pos[1];
        f32 diffZ = light->pos[2] - pos[2];
        f32 dist = (diffX * diffX) + (diffY * diffY) + (diffZ * diffZ);
        f32 radius = light->radius * light->radius;
        if (dist > radius) { continue; }

        Vec3f dir = {
            pos[0] - light->pos[0],
            pos[1] - light->pos[1],
            pos[2] - light->pos[2],
        };
        vec3f_normalize(dir);

        f32 intensity = (1 - (dist / radius)) * light->intensity;
        lightingDir[0] += dir[0] * intensity;
        lightingDir[1] += dir[1] * intensity;
        lightingDir[2] += dir[2] * intensity;

        count++;
    }

    out[0] = lightingDir[0] / (f32)(count);
    out[1] = lightingDir[1] / (f32)(count);
    out[2] = lightingDir[2] / (f32)(count);
    vec3f_normalize(out);
}

C_FIELD s16 le_add_light(f32 x, f32 y, f32 z, u8 r, u8 g, u8 b, f32 radius, f32 intensity) {
    if (sLightPool.size() >= LE_MAX_LIGHTS) {
        LOG_LUA_LINE("LE light count cannot exceed %d lights!", LE_MAX_LIGHTS);
        return -1;
    }

    LELight newLight;
    newLight.id = ++sLightID;
    newLight.pos[0] = x;
    newLight.pos[1] = y;
    newLight.pos[2] = z;
    newLight.color[0] = r;
    newLight.color[1] = g;
    newLight.color[2] = b;
    newLight.radius = radius;
    newLight.intensity = intensity;
    newLight.useSurfaceNormals = true;

    sLightPool.push_back(newLight);

    le_update_active_lights();

    sLightRevision++;
    sEnabled = true;
    return sLightID;
}

C_FIELD void le_remove_light(s16 id) {
    if (id < 0) { return; }

    auto it = std::find_if(sLightPool.begin(), sLightPool.end(),
        [id](const LELight& light) {
            return light.id == id;
        }
    );

    if (it != sLightPool.end()) {
        sLightPool.erase(it);
    }

    le_update_active_lights();

    sLightRevision++;
}

C_FIELD s16 le_get_light_count(void) {
    return sLightPool.size();
}

C_FIELD bool le_light_exists(s16 id) {
    if (id < 0) { return false; }

    return std::any_of(sLightPool.begin(), sLightPool.end(),
        [id](const LELight& light) {
            return light.id == id;
        }
    );
}

static LELight* le_find_light(s16 id) {
    if (id < 0) { return nullptr; }

    auto it = std::find_if(sLightPool.begin(), sLightPool.end(),
        [id](const LELight& light) {
            return light.id == id;
        }
    );

    // kinda cursed syntax but it works
    return (it != sLightPool.end()) ? &(*it) : nullptr;
}

C_FIELD void le_get_light_pos(s16 id, VEC_OUT Vec3f out) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        vec3f_set(out, light->pos[0], light->pos[1], light->pos[2]);
    }
}

C_FIELD void le_set_light_pos(s16 id, f32 x, f32 y, f32 z) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        if (fabsf(light->pos[0] - x) <= FLT_EPSILON &&
            fabsf(light->pos[1] - y) <= FLT_EPSILON &&
            fabsf(light->pos[2] - z) <= FLT_EPSILON) {
            return;
        }
        light->pos[0] = x;
        light->pos[1] = y;
        light->pos[2] = z;
        sLightRevision++;
    }
}

C_FIELD void le_get_light_color(s16 id, VEC_OUT Color out) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        color_set(out, light->color[0], light->color[1], light->color[2]);
    }
}

C_FIELD void le_set_light_color(s16 id, u8 r, u8 g, u8 b) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        if (light->color[0] == r &&
            light->color[1] == g &&
            light->color[2] == b) {
            return;
        }
        light->color[0] = r;
        light->color[1] = g;
        light->color[2] = b;
        sLightRevision++;
    }
}

C_FIELD f32 le_get_light_radius(s16 id) {
    if (id < 0) { return 0.0f; }

    if (auto* light = le_find_light(id)) {
        return light->radius;
    }

    return 0.0f;
}

C_FIELD void le_set_light_radius(s16 id, f32 radius) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        if (fabsf(light->radius - radius) <= FLT_EPSILON) {
            return;
        }
        light->radius = radius;

        le_update_active_lights();

        sLightRevision++;
    }
}

C_FIELD f32 le_get_light_intensity(s16 id) {
    if (id < 0) { return 0.0f; }

    if (auto* light = le_find_light(id)) {
        return light->intensity;
    }

    return 0.0f;
}

C_FIELD void le_set_light_intensity(s16 id, f32 intensity) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        if (fabsf(light->intensity - intensity) <= FLT_EPSILON) {
            return;
        }
        light->intensity = intensity;
        sLightRevision++;
    }

    le_update_active_lights();
}

C_FIELD bool le_get_light_use_surface_normals(s16 id) {
    if (id < 0) { return false; }

    if (auto* light = le_find_light(id)) {
        return light->useSurfaceNormals;
    }

    return false;
}

C_FIELD void le_set_light_use_surface_normals(s16 id, bool useSurfaceNormals) {
    if (id < 0) { return; }

    if (auto* light = le_find_light(id)) {
        if (light->useSurfaceNormals == useSurfaceNormals) {
            return;
        }
        light->useSurfaceNormals = useSurfaceNormals;
        sLightRevision++;
    }
}

void le_clear(void) {
    sLightPool.clear();
    sLightID = -1;
    sLightRevision++;
    sLightBoundsRevision = 0;
    sHasInfluentialLights = false;

    color_set(gLEAmbientColor, 127, 127, 127);
}

void le_shutdown(void) {
    sEnabled = false;
    sMode = LE_MODE_AFFECT_ALL_SHADED_AND_COLORED;
    sToneMapping = LE_TONE_MAPPING_WEIGHTED;
    sMaxLightsPerVertex = 4;
    le_clear();
}
