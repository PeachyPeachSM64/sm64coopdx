#include "gfx_lighting.h"
#include <math.h>

void gfx_lighting_calculate_vertex_color(float pos[3], float normal[3], float colorOut[3]) {
    if (!gLightingState.lightingEnabled) {
        colorOut[0] = 255.0f;
        colorOut[1] = 255.0f;
        colorOut[2] = 255.0f;
        return;
    }
    
    colorOut[0] = gLightingState.ambientLight[0] * gLightingState.ambientIntensity * 255.0f;
    colorOut[1] = gLightingState.ambientLight[1] * gLightingState.ambientIntensity * 255.0f;
    colorOut[2] = gLightingState.ambientLight[2] * gLightingState.ambientIntensity * 255.0f;
    
    float normLen = sqrtf(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
    if (normLen < 0.0001f) {
        return;
    }
    
    float normalizedNormal[3] = {
        normal[0] / normLen,
        normal[1] / normLen,
        normal[2] / normLen
    };
    
    for (int i = 0; i < gLightingState.activeLightCount; i++) {
        GfxLight* light = &gLightingState.lights[i];
        
        if (light->type == LIGHT_TYPE_POINT && light->point.enabled) {
            float lightDir[3] = {
                light->point.position[0] - pos[0],
                light->point.position[1] - pos[1],
                light->point.position[2] - pos[2]
            };
            
            float distance = sqrtf(lightDir[0]*lightDir[0] + lightDir[1]*lightDir[1] + lightDir[2]*lightDir[2]);
            
            if (distance > light->point.radius || distance < 0.0001f) {
                continue;
            }
            
            lightDir[0] /= distance;
            lightDir[1] /= distance;
            lightDir[2] /= distance;
            
            float diffuse = normalizedNormal[0] * lightDir[0] + 
                          normalizedNormal[1] * lightDir[1] + 
                          normalizedNormal[2] * lightDir[2];
            
            if (diffuse < 0.0f) diffuse = 0.0f;
            
            float attenuation = 1.0f / (light->point.attenuation[0] + 
                                       light->point.attenuation[1] * distance + 
                                       light->point.attenuation[2] * distance * distance);
            
            float contribution = diffuse * attenuation * light->point.intensity;
            
            colorOut[0] += light->point.color[0] * contribution * 255.0f;
            colorOut[1] += light->point.color[1] * contribution * 255.0f;
            colorOut[2] += light->point.color[2] * contribution * 255.0f;
            
        } else if (light->type == LIGHT_TYPE_DIRECTIONAL && light->directional.enabled) {
            float lightDir[3] = {
                -light->directional.direction[0],
                -light->directional.direction[1],
                -light->directional.direction[2]
            };
            
            float diffuse = normalizedNormal[0] * lightDir[0] + 
                          normalizedNormal[1] * lightDir[1] + 
                          normalizedNormal[2] * lightDir[2];
            
            if (diffuse < 0.0f) diffuse = 0.0f;
            
            float contribution = diffuse * light->directional.intensity;
            
            colorOut[0] += light->directional.color[0] * contribution * 255.0f;
            colorOut[1] += light->directional.color[1] * contribution * 255.0f;
            colorOut[2] += light->directional.color[2] * contribution * 255.0f;
        }
    }
    
    if (colorOut[0] > 255.0f) colorOut[0] = 255.0f;
    if (colorOut[1] > 255.0f) colorOut[1] = 255.0f;
    if (colorOut[2] > 255.0f) colorOut[2] = 255.0f;
}
