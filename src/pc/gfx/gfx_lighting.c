#include "gfx_lighting.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

LightingState gLightingState = { 0 };

void gfx_lighting_init(void) {
    memset(&gLightingState, 0, sizeof(LightingState));
    
    gLightingState.ambientLight[0] = 0.3f;
    gLightingState.ambientLight[1] = 0.3f;
    gLightingState.ambientLight[2] = 0.3f;
    gLightingState.ambientIntensity = 1.0f;
    
    gLightingState.shadowBias = 0.005f;
    gLightingState.shadowSamples = 4;
    gLightingState.shadowsEnabled = false;
    gLightingState.lightingEnabled = false;
    gLightingState.activeLightCount = 0;
    gLightingState.isDirty = false;
}

void gfx_lighting_shutdown(void) {
    for (int i = 0; i < GFX_MAX_LIGHTS; i++) {
        if (gLightingState.shadowMaps[i].shadowMapFBO != 0) {
        }
    }
    
    memset(&gLightingState, 0, sizeof(LightingState));
}

void gfx_lighting_reset(void) {
    gLightingState.activeLightCount = 0;
    for (int i = 0; i < GFX_MAX_LIGHTS; i++) {
        gLightingState.lights[i].type = LIGHT_TYPE_NONE;
    }
    gLightingState.isDirty = true;
}

int gfx_lighting_add_point_light(float pos[3], float color[3], float intensity, float radius) {
    if (gLightingState.activeLightCount >= GFX_MAX_LIGHTS) {
        return -1;
    }
    
    int index = gLightingState.activeLightCount++;
    GfxLight* light = &gLightingState.lights[index];
    
    light->type = LIGHT_TYPE_POINT;
    light->point.position[0] = pos[0];
    light->point.position[1] = pos[1];
    light->point.position[2] = pos[2];
    light->point.color[0] = color[0];
    light->point.color[1] = color[1];
    light->point.color[2] = color[2];
    light->point.intensity = intensity;
    light->point.radius = radius;
    
    light->point.attenuation[0] = 1.0f;
    light->point.attenuation[1] = 4.5f / radius;
    light->point.attenuation[2] = 75.0f / (radius * radius);
    
    light->point.castShadows = 0;
    light->point.enabled = 1;
    
    gLightingState.isDirty = true;
    return index;
}

int gfx_lighting_add_directional_light(float dir[3], float color[3], float intensity) {
    if (gLightingState.activeLightCount >= GFX_MAX_LIGHTS) {
        return -1;
    }
    
    int index = gLightingState.activeLightCount++;
    GfxLight* light = &gLightingState.lights[index];
    
    light->type = LIGHT_TYPE_DIRECTIONAL;
    
    float len = sqrtf(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
    if (len > 0.0001f) {
        light->directional.direction[0] = dir[0] / len;
        light->directional.direction[1] = dir[1] / len;
        light->directional.direction[2] = dir[2] / len;
    } else {
        light->directional.direction[0] = 0.0f;
        light->directional.direction[1] = -1.0f;
        light->directional.direction[2] = 0.0f;
    }
    
    light->directional.color[0] = color[0];
    light->directional.color[1] = color[1];
    light->directional.color[2] = color[2];
    light->directional.intensity = intensity;
    light->directional.castShadows = 0;
    light->directional.enabled = 1;
    
    gLightingState.isDirty = true;
    return index;
}

int gfx_lighting_add_spot_light(float pos[3], float dir[3], float color[3], float intensity, float radius, float innerAngle, float outerAngle) {
    if (gLightingState.activeLightCount >= GFX_MAX_LIGHTS) {
        return -1;
    }
    
    int index = gLightingState.activeLightCount++;
    GfxLight* light = &gLightingState.lights[index];
    
    light->type = LIGHT_TYPE_SPOT;
    light->spot.position[0] = pos[0];
    light->spot.position[1] = pos[1];
    light->spot.position[2] = pos[2];
    
    float len = sqrtf(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
    if (len > 0.0001f) {
        light->spot.direction[0] = dir[0] / len;
        light->spot.direction[1] = dir[1] / len;
        light->spot.direction[2] = dir[2] / len;
    } else {
        light->spot.direction[0] = 0.0f;
        light->spot.direction[1] = -1.0f;
        light->spot.direction[2] = 0.0f;
    }
    
    light->spot.color[0] = color[0];
    light->spot.color[1] = color[1];
    light->spot.color[2] = color[2];
    light->spot.intensity = intensity;
    light->spot.radius = radius;
    light->spot.innerCutoff = cosf(innerAngle * 3.14159265f / 180.0f);
    light->spot.outerCutoff = cosf(outerAngle * 3.14159265f / 180.0f);
    
    light->spot.attenuation[0] = 1.0f;
    light->spot.attenuation[1] = 4.5f / radius;
    light->spot.attenuation[2] = 75.0f / (radius * radius);
    
    light->spot.castShadows = 0;
    light->spot.enabled = 1;
    
    gLightingState.isDirty = true;
    return index;
}

void gfx_lighting_remove_light(int lightIndex) {
    if (lightIndex < 0 || lightIndex >= gLightingState.activeLightCount) {
        return;
    }
    
    for (int i = lightIndex; i < gLightingState.activeLightCount - 1; i++) {
        gLightingState.lights[i] = gLightingState.lights[i + 1];
        gLightingState.shadowMaps[i] = gLightingState.shadowMaps[i + 1];
    }
    
    gLightingState.activeLightCount--;
    gLightingState.lights[gLightingState.activeLightCount].type = LIGHT_TYPE_NONE;
    gLightingState.isDirty = true;
}

void gfx_lighting_update_light(int lightIndex, GfxLight* light) {
    if (lightIndex < 0 || lightIndex >= gLightingState.activeLightCount || !light) {
        return;
    }
    
    gLightingState.lights[lightIndex] = *light;
    gLightingState.isDirty = true;
}

void gfx_lighting_enable_light(int lightIndex, bool enabled) {
    if (lightIndex < 0 || lightIndex >= gLightingState.activeLightCount) {
        return;
    }
    
    GfxLight* light = &gLightingState.lights[lightIndex];
    switch (light->type) {
        case LIGHT_TYPE_POINT:
            light->point.enabled = enabled ? 1 : 0;
            break;
        case LIGHT_TYPE_DIRECTIONAL:
            light->directional.enabled = enabled ? 1 : 0;
            break;
        case LIGHT_TYPE_SPOT:
            light->spot.enabled = enabled ? 1 : 0;
            break;
        default:
            break;
    }
    
    gLightingState.isDirty = true;
}

void gfx_lighting_set_light_shadow(int lightIndex, bool castShadows) {
    if (lightIndex < 0 || lightIndex >= gLightingState.activeLightCount) {
        return;
    }
    
    GfxLight* light = &gLightingState.lights[lightIndex];
    switch (light->type) {
        case LIGHT_TYPE_POINT:
            light->point.castShadows = castShadows ? 1 : 0;
            break;
        case LIGHT_TYPE_DIRECTIONAL:
            light->directional.castShadows = castShadows ? 1 : 0;
            break;
        case LIGHT_TYPE_SPOT:
            light->spot.castShadows = castShadows ? 1 : 0;
            break;
        default:
            break;
    }
    
    gLightingState.isDirty = true;
}

void gfx_lighting_set_ambient(float color[3], float intensity) {
    gLightingState.ambientLight[0] = color[0];
    gLightingState.ambientLight[1] = color[1];
    gLightingState.ambientLight[2] = color[2];
    gLightingState.ambientIntensity = intensity;
    gLightingState.isDirty = true;
}

void gfx_lighting_enable(bool enabled) {
    gLightingState.lightingEnabled = enabled;
    gLightingState.isDirty = true;
}

void gfx_lighting_enable_shadows(bool enabled) {
    gLightingState.shadowsEnabled = enabled;
    gLightingState.isDirty = true;
}

void gfx_lighting_update_shadows(void) {
}

void gfx_lighting_bind_shadow_maps(void) {
}

LightingState* gfx_lighting_get_state(void) {
    return &gLightingState;
}

int gfx_lighting_get_shadow_map_size(void) {
    extern unsigned int configLightingShadowQuality;
    
    switch (configLightingShadowQuality) {
        case SHADOW_QUALITY_OFF:
            return 0;
        case SHADOW_QUALITY_LOW:
            return 512;
        case SHADOW_QUALITY_MEDIUM:
            return 1024;
        case SHADOW_QUALITY_HIGH:
            return 2048;
        case SHADOW_QUALITY_ULTRA:
            return 4096;
        default:
            return 1024;
    }
}

void gfx_lighting_set_shadow_quality(unsigned int quality) {
    extern unsigned int configLightingShadowQuality;
    if (quality > SHADOW_QUALITY_ULTRA) {
        quality = SHADOW_QUALITY_ULTRA;
    }
    configLightingShadowQuality = quality;
    gLightingState.isDirty = true;
}
