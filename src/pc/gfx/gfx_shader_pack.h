#ifndef GFX_SHADER_PACK_H
#define GFX_SHADER_PACK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SHADER_PACKS 16
#define MAX_SHADER_NAME 64
#define MAX_SHADER_PATH 256
#define MAX_SHADER_SOURCE 65536

typedef struct {
    char name[MAX_SHADER_NAME];
    char vertexPath[MAX_SHADER_PATH];
    char fragmentPath[MAX_SHADER_PATH];
    char geometryPath[MAX_SHADER_PATH];
    
    char* vertexSource;
    char* fragmentSource;
    char* geometrySource;
    
    uint32_t programId;
    bool isCompiled;
    bool supportsLighting;
    bool supportsShadows;
    
    int uniformLocations[32];
    int attribLocations[16];
} ShaderPackProgram;

typedef struct {
    char name[MAX_SHADER_NAME];
    char path[MAX_SHADER_PATH];
    char author[MAX_SHADER_NAME];
    char version[16];
    char description[256];
    
    ShaderPackProgram compositeShader;
    ShaderPackProgram finalShader;
    ShaderPackProgram shadowShader;
    
    bool isLoaded;
    bool isActive;
    
    int drawDistance;
    float shadowDistance;
    bool dynamicHandLight;
    bool oldLighting;
} ShaderPack;

typedef struct {
    ShaderPack packs[MAX_SHADER_PACKS];
    int packCount;
    int activePackIndex;
    bool customShadersEnabled;
    
    char shaderPacksDirectory[MAX_SHADER_PATH];
} ShaderPackManager;

extern ShaderPackManager gShaderPackManager;

void gfx_shader_pack_init(void);
void gfx_shader_pack_shutdown(void);

void gfx_shader_pack_scan_directory(const char* directory);
bool gfx_shader_pack_load(const char* packPath);
void gfx_shader_pack_unload(int packIndex);
void gfx_shader_pack_activate(int packIndex);
void gfx_shader_pack_deactivate(void);

ShaderPack* gfx_shader_pack_get_active(void);
int gfx_shader_pack_get_count(void);
ShaderPack* gfx_shader_pack_get_by_index(int index);
ShaderPack* gfx_shader_pack_get_by_name(const char* name);

bool gfx_shader_pack_compile_program(ShaderPackProgram* program);
void gfx_shader_pack_use_program(ShaderPackProgram* program);

void gfx_shader_pack_set_enabled(bool enabled);
bool gfx_shader_pack_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif
