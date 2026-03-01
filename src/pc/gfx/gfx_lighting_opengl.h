#ifndef GFX_LIGHTING_OPENGL_H
#define GFX_LIGHTING_OPENGL_H

#include <stdint.h>
#include <stdbool.h>
#include "gfx_lighting.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t shadowFBO[GFX_MAX_LIGHTS];
    uint32_t shadowTextures[GFX_MAX_LIGHTS];
    uint32_t shadowCubemaps[GFX_MAX_LIGHTS];
    
    uint32_t lightingUBO;
    
    int maxShadowMaps;
    bool isInitialized;
} OpenGLLightingContext;

void gfx_opengl_lighting_init(void);
void gfx_opengl_lighting_shutdown(void);

void gfx_opengl_lighting_setup_shadow_maps(void);
void gfx_opengl_lighting_render_shadows(void);
void gfx_opengl_lighting_bind_for_rendering(uint32_t programId);

void gfx_opengl_lighting_update_uniforms(uint32_t programId);

uint32_t gfx_opengl_lighting_compile_shader(const char* vertexSrc, const char* fragmentSrc, const char* geometrySrc);
bool gfx_opengl_lighting_check_shader_compile(uint32_t shader, const char* shaderType);
bool gfx_opengl_lighting_check_program_link(uint32_t program);

#ifdef __cplusplus
}
#endif

#endif
