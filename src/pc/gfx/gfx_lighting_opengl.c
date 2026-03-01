#ifdef RAPI_GL

#include "gfx_lighting_opengl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _LANGUAGE_C
# define _LANGUAGE_C
#endif

#ifdef __MINGW32__
# define FOR_WINDOWS 1
#else
# define FOR_WINDOWS 0
#endif

#if FOR_WINDOWS || defined(OSX_BUILD)
# define GLEW_STATIC
# include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#ifdef WAPI_SDL2
# if defined(__has_include) && __has_include(<SDL2/SDL.h>)
#  include <SDL2/SDL.h>
# else
#  include <SDL.h>
# endif
# ifdef USE_GLES
#  if defined(__has_include) && __has_include(<SDL2/SDL_opengles2.h>)
#   include <SDL2/SDL_opengles2.h>
#  else
#   include <SDL_opengles2.h>
#  endif
# else
#  if defined(__has_include) && __has_include(<SDL2/SDL_opengl.h>)
#   include <SDL2/SDL_opengl.h>
#  else
#   include <SDL_opengl.h>
#  endif
# endif
#elif defined(WAPI_SDL1)
# include <SDL/SDL.h>
# ifndef GLEW_STATIC
#  include <SDL/SDL_opengl.h>
# endif
#endif

static OpenGLLightingContext sLightingCtx = { 0 };

void gfx_opengl_lighting_init(void) {
    if (sLightingCtx.isInitialized) {
        return;
    }
    
    memset(&sLightingCtx, 0, sizeof(OpenGLLightingContext));
    
    GLint maxTexUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
    sLightingCtx.maxShadowMaps = (maxTexUnits > 8) ? 8 : maxTexUnits - 2;
    
    sLightingCtx.isInitialized = true;
    
    printf("OpenGL Lighting initialized (max shadow maps: %d)\n", sLightingCtx.maxShadowMaps);
}

void gfx_opengl_lighting_shutdown(void) {
    if (!sLightingCtx.isInitialized) {
        return;
    }
    
    for (int i = 0; i < GFX_MAX_LIGHTS; i++) {
        if (sLightingCtx.shadowFBO[i] != 0) {
            glDeleteFramebuffers(1, &sLightingCtx.shadowFBO[i]);
        }
        if (sLightingCtx.shadowTextures[i] != 0) {
            glDeleteTextures(1, &sLightingCtx.shadowTextures[i]);
        }
        if (sLightingCtx.shadowCubemaps[i] != 0) {
            glDeleteTextures(1, &sLightingCtx.shadowCubemaps[i]);
        }
    }
    
    if (sLightingCtx.lightingUBO != 0) {
        glDeleteBuffers(1, &sLightingCtx.lightingUBO);
    }
    
    memset(&sLightingCtx, 0, sizeof(OpenGLLightingContext));
}

void gfx_opengl_lighting_setup_shadow_maps(void) {
    if (!gLightingState.shadowsEnabled) {
        return;
    }
    
    for (int i = 0; i < gLightingState.activeLightCount && i < sLightingCtx.maxShadowMaps; i++) {
        GfxLight* light = &gLightingState.lights[i];
        bool needsShadowMap = false;
        bool needsCubemap = false;
        
        switch (light->type) {
            case LIGHT_TYPE_POINT:
                needsShadowMap = light->point.castShadows;
                needsCubemap = true;
                break;
            case LIGHT_TYPE_DIRECTIONAL:
                needsShadowMap = light->directional.castShadows;
                needsCubemap = false;
                break;
            case LIGHT_TYPE_SPOT:
                needsShadowMap = light->spot.castShadows;
                needsCubemap = false;
                break;
            default:
                break;
        }
        
        if (!needsShadowMap) {
            continue;
        }
        
        if (sLightingCtx.shadowFBO[i] == 0) {
            glGenFramebuffers(1, &sLightingCtx.shadowFBO[i]);
        }
        
        if (needsCubemap && sLightingCtx.shadowCubemaps[i] == 0) {
            int shadowSize = gfx_lighting_get_shadow_map_size();
            if (shadowSize == 0) continue;
            
            glGenTextures(1, &sLightingCtx.shadowCubemaps[i]);
            glBindTexture(GL_TEXTURE_CUBE_MAP, sLightingCtx.shadowCubemaps[i]);
            
            for (int face = 0; face < 6; face++) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT,
                           shadowSize, shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            }
            
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            
            glBindFramebuffer(GL_FRAMEBUFFER, sLightingCtx.shadowFBO[i]);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sLightingCtx.shadowCubemaps[i], 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else if (!needsCubemap && sLightingCtx.shadowTextures[i] == 0) {
            int shadowSize = gfx_lighting_get_shadow_map_size();
            if (shadowSize == 0) continue;
            
            glGenTextures(1, &sLightingCtx.shadowTextures[i]);
            glBindTexture(GL_TEXTURE_2D, sLightingCtx.shadowTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowSize, shadowSize, 
                        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            
            glBindFramebuffer(GL_FRAMEBUFFER, sLightingCtx.shadowFBO[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sLightingCtx.shadowTextures[i], 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
}

void gfx_opengl_lighting_render_shadows(void) {
}

void gfx_opengl_lighting_bind_for_rendering(uint32_t programId) {
    if (!gLightingState.lightingEnabled || !sLightingCtx.isInitialized) {
        return;
    }
    
    gfx_opengl_lighting_update_uniforms(programId);
    
    if (gLightingState.shadowsEnabled) {
        for (int i = 0; i < gLightingState.activeLightCount && i < sLightingCtx.maxShadowMaps; i++) {
            if (sLightingCtx.shadowTextures[i] != 0) {
                glActiveTexture(GL_TEXTURE2 + i);
                glBindTexture(GL_TEXTURE_2D, sLightingCtx.shadowTextures[i]);
            } else if (sLightingCtx.shadowCubemaps[i] != 0) {
                glActiveTexture(GL_TEXTURE2 + i);
                glBindTexture(GL_TEXTURE_CUBE_MAP, sLightingCtx.shadowCubemaps[i]);
            }
        }
    }
}

void gfx_opengl_lighting_update_uniforms(uint32_t programId) {
    if (!gLightingState.lightingEnabled) {
        return;
    }
    
    GLint loc;
    
    loc = glGetUniformLocation(programId, "uLightingEnabled");
    if (loc >= 0) glUniform1i(loc, gLightingState.lightingEnabled ? 1 : 0);
    
    loc = glGetUniformLocation(programId, "uNumLights");
    if (loc >= 0) glUniform1i(loc, gLightingState.activeLightCount);
    
    loc = glGetUniformLocation(programId, "uAmbientLight");
    if (loc >= 0) glUniform3fv(loc, 1, gLightingState.ambientLight);
    
    loc = glGetUniformLocation(programId, "uAmbientIntensity");
    if (loc >= 0) glUniform1f(loc, gLightingState.ambientIntensity);
    
    for (int i = 0; i < gLightingState.activeLightCount; i++) {
        char uniformName[64];
        GfxLight* light = &gLightingState.lights[i];
        
        snprintf(uniformName, sizeof(uniformName), "uLights[%d].type", i);
        loc = glGetUniformLocation(programId, uniformName);
        if (loc >= 0) glUniform1i(loc, (int)light->type);
        
        switch (light->type) {
            case LIGHT_TYPE_POINT:
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].position", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->point.position);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].color", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->point.color);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].intensity", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->point.intensity);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].radius", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->point.radius);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].attenuation", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->point.attenuation);
                break;
                
            case LIGHT_TYPE_DIRECTIONAL:
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].direction", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->directional.direction);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].color", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->directional.color);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].intensity", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->directional.intensity);
                break;
                
            case LIGHT_TYPE_SPOT:
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].position", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->spot.position);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].direction", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->spot.direction);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].color", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->spot.color);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].intensity", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->spot.intensity);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].radius", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->spot.radius);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].innerCutoff", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->spot.innerCutoff);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].outerCutoff", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform1f(loc, light->spot.outerCutoff);
                
                snprintf(uniformName, sizeof(uniformName), "uLights[%d].attenuation", i);
                loc = glGetUniformLocation(programId, uniformName);
                if (loc >= 0) glUniform3fv(loc, 1, light->spot.attenuation);
                break;
                
            default:
                break;
        }
    }
}

uint32_t gfx_opengl_lighting_compile_shader(const char* vertexSrc, const char* fragmentSrc, const char* geometrySrc) {
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLuint geometryShader = 0;
    GLuint program = 0;
    
    if (vertexSrc) {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSrc, NULL);
        glCompileShader(vertexShader);
        
        if (!gfx_opengl_lighting_check_shader_compile(vertexShader, "vertex")) {
            glDeleteShader(vertexShader);
            return 0;
        }
    }
    
    if (fragmentSrc) {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
        glCompileShader(fragmentShader);
        
        if (!gfx_opengl_lighting_check_shader_compile(fragmentShader, "fragment")) {
            if (vertexShader) glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }
    }
    
    if (geometrySrc) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometrySrc, NULL);
        glCompileShader(geometryShader);
        
        if (!gfx_opengl_lighting_check_shader_compile(geometryShader, "geometry")) {
            if (vertexShader) glDeleteShader(vertexShader);
            if (fragmentShader) glDeleteShader(fragmentShader);
            glDeleteShader(geometryShader);
            return 0;
        }
    }
    
    program = glCreateProgram();
    if (vertexShader) glAttachShader(program, vertexShader);
    if (fragmentShader) glAttachShader(program, fragmentShader);
    if (geometryShader) glAttachShader(program, geometryShader);
    
    glLinkProgram(program);
    
    if (!gfx_opengl_lighting_check_program_link(program)) {
        if (vertexShader) glDeleteShader(vertexShader);
        if (fragmentShader) glDeleteShader(fragmentShader);
        if (geometryShader) glDeleteShader(geometryShader);
        glDeleteProgram(program);
        return 0;
    }
    
    if (vertexShader) glDeleteShader(vertexShader);
    if (fragmentShader) glDeleteShader(fragmentShader);
    if (geometryShader) glDeleteShader(geometryShader);
    
    return program;
}

bool gfx_opengl_lighting_check_shader_compile(uint32_t shader, const char* shaderType) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Shader compilation error (%s):\n%s\n", shaderType, infoLog);
        return false;
    }
    
    return true;
}

bool gfx_opengl_lighting_check_program_link(uint32_t program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "Shader program linking error:\n%s\n", infoLog);
        return false;
    }
    
    return true;
}

#endif
