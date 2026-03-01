#include "gfx_shader_pack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

#include "pc/fs/fs.h"

ShaderPackManager gShaderPackManager = { 0 };

static char* load_file_contents(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open shader file: %s\n", filepath);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fileSize > MAX_SHADER_SOURCE) {
        fprintf(stderr, "Shader file too large: %s (%ld bytes)\n", filepath, fileSize);
        fclose(file);
        return NULL;
    }
    
    char* buffer = (char*)malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for shader: %s\n", filepath);
        fclose(file);
        return NULL;
    }
    
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    buffer[bytesRead] = '\0';
    fclose(file);
    
    return buffer;
}

static bool file_exists(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

void gfx_shader_pack_init(void) {
    memset(&gShaderPackManager, 0, sizeof(ShaderPackManager));
    
    snprintf(gShaderPackManager.shaderPacksDirectory, MAX_SHADER_PATH, "shaderpacks");
    
    gShaderPackManager.customShadersEnabled = false;
    gShaderPackManager.activePackIndex = -1;
    gShaderPackManager.packCount = 0;
}

void gfx_shader_pack_shutdown(void) {
    for (int i = 0; i < gShaderPackManager.packCount; i++) {
        gfx_shader_pack_unload(i);
    }
    
    memset(&gShaderPackManager, 0, sizeof(ShaderPackManager));
    gShaderPackManager.activePackIndex = -1;
}

void gfx_shader_pack_scan_directory(const char* directory) {
    DIR* dir = opendir(directory);
    if (!dir) {
        fprintf(stderr, "Failed to open shader packs directory: %s\n", directory);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        char packPath[MAX_SHADER_PATH];
        snprintf(packPath, MAX_SHADER_PATH, "%s%c%s", directory, PATH_SEPARATOR, entry->d_name);
        
        struct stat statbuf;
        if (stat(packPath, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            gfx_shader_pack_load(packPath);
        }
    }
    
    closedir(dir);
}

bool gfx_shader_pack_load(const char* packPath) {
    if (gShaderPackManager.packCount >= MAX_SHADER_PACKS) {
        fprintf(stderr, "Maximum shader packs reached\n");
        return false;
    }
    
    char propertiesPath[MAX_SHADER_PATH];
    snprintf(propertiesPath, MAX_SHADER_PATH, "%s%cshaderpack.properties", packPath, PATH_SEPARATOR);
    
    ShaderPack* pack = &gShaderPackManager.packs[gShaderPackManager.packCount];
    memset(pack, 0, sizeof(ShaderPack));
    
    strncpy(pack->path, packPath, MAX_SHADER_PATH - 1);
    
    const char* packName = strrchr(packPath, PATH_SEPARATOR);
    if (packName) {
        packName++;
    } else {
        packName = packPath;
    }
    strncpy(pack->name, packName, MAX_SHADER_NAME - 1);
    
    FILE* propFile = fopen(propertiesPath, "r");
    if (propFile) {
        char line[512];
        while (fgets(line, sizeof(line), propFile)) {
            if (strncmp(line, "author=", 7) == 0) {
                sscanf(line + 7, "%63[^\n]", pack->author);
            } else if (strncmp(line, "version=", 8) == 0) {
                sscanf(line + 8, "%15[^\n]", pack->version);
            } else if (strncmp(line, "description=", 12) == 0) {
                sscanf(line + 12, "%255[^\n]", pack->description);
            } else if (strncmp(line, "shadowDistance=", 15) == 0) {
                sscanf(line + 15, "%f", &pack->shadowDistance);
            }
        }
        fclose(propFile);
    }
    
    if (pack->shadowDistance == 0.0f) {
        pack->shadowDistance = 10000.0f;
    }
    
    snprintf(pack->compositeShader.vertexPath, MAX_SHADER_PATH, "%s%cshaders%ccomposite.vsh", packPath, PATH_SEPARATOR, PATH_SEPARATOR);
    snprintf(pack->compositeShader.fragmentPath, MAX_SHADER_PATH, "%s%cshaders%ccomposite.fsh", packPath, PATH_SEPARATOR, PATH_SEPARATOR);
    snprintf(pack->finalShader.vertexPath, MAX_SHADER_PATH, "%s%cshaders%cfinal.vsh", packPath, PATH_SEPARATOR, PATH_SEPARATOR);
    snprintf(pack->finalShader.fragmentPath, MAX_SHADER_PATH, "%s%cshaders%cfinal.fsh", packPath, PATH_SEPARATOR, PATH_SEPARATOR);
    snprintf(pack->shadowShader.vertexPath, MAX_SHADER_PATH, "%s%cshaders%cshadow.vsh", packPath, PATH_SEPARATOR, PATH_SEPARATOR);
    snprintf(pack->shadowShader.fragmentPath, MAX_SHADER_PATH, "%s%cshaders%cshadow.fsh", packPath, PATH_SEPARATOR, PATH_SEPARATOR);
    
    strncpy(pack->compositeShader.name, "composite", MAX_SHADER_NAME - 1);
    strncpy(pack->finalShader.name, "final", MAX_SHADER_NAME - 1);
    strncpy(pack->shadowShader.name, "shadow", MAX_SHADER_NAME - 1);
    
    pack->compositeShader.supportsLighting = true;
    pack->finalShader.supportsLighting = false;
    pack->shadowShader.supportsShadows = true;
    
    pack->isLoaded = true;
    
    gShaderPackManager.packCount++;
    
    printf("Loaded shader pack: %s (by %s)\n", pack->name, pack->author);
    
    return true;
}

void gfx_shader_pack_unload(int packIndex) {
    if (packIndex < 0 || packIndex >= gShaderPackManager.packCount) {
        return;
    }
    
    ShaderPack* pack = &gShaderPackManager.packs[packIndex];
    
    if (pack->compositeShader.vertexSource) {
        free(pack->compositeShader.vertexSource);
        pack->compositeShader.vertexSource = NULL;
    }
    if (pack->compositeShader.fragmentSource) {
        free(pack->compositeShader.fragmentSource);
        pack->compositeShader.fragmentSource = NULL;
    }
    if (pack->finalShader.vertexSource) {
        free(pack->finalShader.vertexSource);
        pack->finalShader.vertexSource = NULL;
    }
    if (pack->finalShader.fragmentSource) {
        free(pack->finalShader.fragmentSource);
        pack->finalShader.fragmentSource = NULL;
    }
    if (pack->shadowShader.vertexSource) {
        free(pack->shadowShader.vertexSource);
        pack->shadowShader.vertexSource = NULL;
    }
    if (pack->shadowShader.fragmentSource) {
        free(pack->shadowShader.fragmentSource);
        pack->shadowShader.fragmentSource = NULL;
    }
    
    if (packIndex == gShaderPackManager.activePackIndex) {
        gShaderPackManager.activePackIndex = -1;
    }
    
    for (int i = packIndex; i < gShaderPackManager.packCount - 1; i++) {
        gShaderPackManager.packs[i] = gShaderPackManager.packs[i + 1];
    }
    
    gShaderPackManager.packCount--;
}

void gfx_shader_pack_activate(int packIndex) {
    if (packIndex < 0 || packIndex >= gShaderPackManager.packCount) {
        return;
    }
    
    if (gShaderPackManager.activePackIndex >= 0) {
        gShaderPackManager.packs[gShaderPackManager.activePackIndex].isActive = false;
    }
    
    gShaderPackManager.activePackIndex = packIndex;
    gShaderPackManager.packs[packIndex].isActive = true;
    
    printf("Activated shader pack: %s\n", gShaderPackManager.packs[packIndex].name);
}

void gfx_shader_pack_deactivate(void) {
    if (gShaderPackManager.activePackIndex >= 0) {
        gShaderPackManager.packs[gShaderPackManager.activePackIndex].isActive = false;
        gShaderPackManager.activePackIndex = -1;
    }
}

ShaderPack* gfx_shader_pack_get_active(void) {
    if (gShaderPackManager.activePackIndex >= 0 && gShaderPackManager.activePackIndex < gShaderPackManager.packCount) {
        return &gShaderPackManager.packs[gShaderPackManager.activePackIndex];
    }
    return NULL;
}

int gfx_shader_pack_get_count(void) {
    return gShaderPackManager.packCount;
}

ShaderPack* gfx_shader_pack_get_by_index(int index) {
    if (index >= 0 && index < gShaderPackManager.packCount) {
        return &gShaderPackManager.packs[index];
    }
    return NULL;
}

ShaderPack* gfx_shader_pack_get_by_name(const char* name) {
    for (int i = 0; i < gShaderPackManager.packCount; i++) {
        if (strcmp(gShaderPackManager.packs[i].name, name) == 0) {
            return &gShaderPackManager.packs[i];
        }
    }
    return NULL;
}

bool gfx_shader_pack_compile_program(ShaderPackProgram* program) {
    if (!program) {
        return false;
    }
    
    return true;
}

void gfx_shader_pack_use_program(ShaderPackProgram* program) {
    if (!program || !program->isCompiled) {
        return;
    }
}

void gfx_shader_pack_set_enabled(bool enabled) {
    gShaderPackManager.customShadersEnabled = enabled;
}

bool gfx_shader_pack_is_enabled(void) {
    return gShaderPackManager.customShadersEnabled;
}
