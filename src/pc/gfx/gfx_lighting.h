#ifndef GFX_LIGHTING_H
#define GFX_LIGHTING_H

#include <stdint.h>
#include <stdbool.h>
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GFX_MAX_LIGHTS 32
#define MAX_SHADOW_CASCADES 4
#define SHADOW_MAP_SIZE_DEFAULT 2048

enum ShadowQuality {
    SHADOW_QUALITY_OFF = 0,
    SHADOW_QUALITY_LOW = 1,
    SHADOW_QUALITY_MEDIUM = 2,
    SHADOW_QUALITY_HIGH = 3,
    SHADOW_QUALITY_ULTRA = 4
};

enum LightType {
    LIGHT_TYPE_NONE = 0,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_SPOT
};

typedef struct {
    float position[3];
    float color[3];
    float intensity;
    float radius;
    float attenuation[3];
    uint8_t castShadows;
    uint8_t enabled;
    uint8_t _pad[2];
} PointLight;

typedef struct {
    float direction[3];
    float color[3];
    float intensity;
    uint8_t castShadows;
    uint8_t enabled;
    uint8_t _pad[2];
} DirectionalLight;

typedef struct {
    float position[3];
    float direction[3];
    float color[3];
    float intensity;
    float radius;
    float innerCutoff;
    float outerCutoff;
    float attenuation[3];
    uint8_t castShadows;
    uint8_t enabled;
    uint8_t _pad[2];
} SpotLight;

typedef struct {
    enum LightType type;
    union {
        PointLight point;
        DirectionalLight directional;
        SpotLight spot;
    };
} GfxLight;

typedef struct {
    uint32_t shadowMapFBO;
    uint32_t shadowMapTexture;
    float shadowMatrix[16];
    float nearPlane;
    float farPlane;
    int shadowMapSize;
    bool enabled;
} ShadowMap;

typedef struct {
    GfxLight lights[GFX_MAX_LIGHTS];
    ShadowMap shadowMaps[GFX_MAX_LIGHTS];
    int activeLightCount;
    bool lightingEnabled;
    float ambientLight[3];
    float ambientIntensity;
    
    bool shadowsEnabled;
    float shadowBias;
    int shadowSamples;
    
    bool isDirty;
} LightingState;

extern LightingState gLightingState;

void gfx_lighting_init(void);
void gfx_lighting_shutdown(void);
void gfx_lighting_reset(void);

int gfx_lighting_add_point_light(float pos[3], float color[3], float intensity, float radius);
int gfx_lighting_add_directional_light(float dir[3], float color[3], float intensity);
int gfx_lighting_add_spot_light(float pos[3], float dir[3], float color[3], float intensity, float radius, float innerAngle, float outerAngle);

void gfx_lighting_remove_light(int lightIndex);
void gfx_lighting_update_light(int lightIndex, GfxLight* light);
void gfx_lighting_enable_light(int lightIndex, bool enabled);
void gfx_lighting_set_light_shadow(int lightIndex, bool castShadows);

void gfx_lighting_set_ambient(float color[3], float intensity);
void gfx_lighting_enable(bool enabled);
void gfx_lighting_enable_shadows(bool enabled);

void gfx_lighting_update_shadows(void);
void gfx_lighting_bind_shadow_maps(void);

LightingState* gfx_lighting_get_state(void);
int gfx_lighting_get_shadow_map_size(void);
void gfx_lighting_set_shadow_quality(unsigned int quality);

void gfx_lighting_calculate_vertex_color(float pos[3], float normal[3], float colorOut[3]);

#ifdef __cplusplus
}
#endif

#endif
