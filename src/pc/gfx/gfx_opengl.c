#include <stdint.h>
#include <stdbool.h>

#ifndef _LANGUAGE_C
# define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#ifdef __MINGW32__
# define FOR_WINDOWS 1
#else
# define FOR_WINDOWS 0
#endif

#if FOR_WINDOWS || defined(OSX_BUILD)
# define GLEW_STATIC
#ifndef VSCODE
# include <GL/glew.h>
#endif
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL3/SDL.h>
#ifdef USE_GLES
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include "../platform.h"
#include "../configfile.h"
#include "gfx_cc.h"
#include "gfx_rendering_api.h"
#include "gfx_pc.h"

#define TEX_CACHE_STEP 512

#define NUM_TEX_UNIFORMS 2
#define NUM_TEXTURES 2




enum {
    UNIFORM_CCF = NUM_TEX_UNIFORMS * NUM_TEXTURES,
    UNIFORM_SHADER_COMMANDS,
    UNIFORM_FRAME_COUNT,
    UNIFORM_LIGHT_MAP,
    UNIFORM_SHADER_FLAGS,
    UNIFORM_SHADER_FLAG_VALUES,
    UNIFORM_FILTERING,

    UNIFORM_COUNT,
};


static const char UBER_VERT[] = {
#embed "uber.vert"
,0
};

static const char UBER_FRAG[] = {
#embed "uber.frag"
,0
};

struct ShaderProgram {
    uint64_t hash;
    GLuint opengl_program_id;
    uint8_t num_inputs;
    uint8_t num_floats;
    GLint attrib_locations[16];
    GLint uniform_locations[UNIFORM_COUNT];
    uint8_t attrib_sizes[16];
    uint8_t num_attribs;
    struct ColorCombiner *cc;
};

struct GLTexture {
    GLuint gltex;
    GLfloat size[2];
    bool filter;
};

static struct ShaderProgram shader_program_pool[CC_MAX_SHADERS];
static uint8_t shader_program_pool_size = 0;
static uint8_t shader_program_pool_index = 0;
static GLuint opengl_vbo;
static GLuint opengl_vao;

static int tex_cache_size = 0;
static int num_textures = 0;
static struct GLTexture *tex_cache = NULL;

static struct ShaderProgram *opengl_prg = NULL;
static struct GLTexture *opengl_tex[2];
static int opengl_curtex = 0;

static uint32_t frame_count;

static bool gfx_opengl_z_is_from_0_to_1(void) {
    return false;
}

static void gfx_opengl_vertex_array_set_attribs(struct ShaderProgram *prg) {
    size_t num_floats = prg->num_floats;
    size_t pos = 0;

    for (int i = 0; i < prg->num_attribs; i++) {
        glEnableVertexAttribArray(prg->attrib_locations[i]);
        glVertexAttribPointer(prg->attrib_locations[i], prg->attrib_sizes[i], GL_FLOAT, GL_FALSE, num_floats * sizeof(float), (void *) (pos * sizeof(float)));
        pos += prg->attrib_sizes[i];
    }
}

static inline void gfx_opengl_set_shader_uniforms(struct ShaderProgram *prg) {

    struct CCFeatures ccf = { 0 };
    gfx_cc_get_features(prg->cc, &ccf);

    int ccf_flags = 0;
    ccf_flags |= (1 <<  4) * ccf.used_textures[0];
    ccf_flags |= (1 <<  5) * ccf.used_textures[1];
    ccf_flags |= (1 <<  6) * ccf.color_alpha_same[0];
    ccf_flags |= (1 <<  7) * ccf.color_alpha_same[1];
    ccf_flags |= (1 <<  8) * ccf.do_single[0];
    ccf_flags |= (1 <<  9) * ccf.do_single[1];
    ccf_flags |= (1 << 10) * ccf.do_single[2];
    ccf_flags |= (1 << 11) * ccf.do_single[3];
    ccf_flags |= (1 << 12) * ccf.do_multiply[0];
    ccf_flags |= (1 << 13) * ccf.do_multiply[1];
    ccf_flags |= (1 << 14) * ccf.do_multiply[2];
    ccf_flags |= (1 << 15) * ccf.do_multiply[3];
    ccf_flags |= (1 << 16) * ccf.do_mix[0];
    ccf_flags |= (1 << 17) * ccf.do_mix[1];
    ccf_flags |= (1 << 18) * ccf.do_mix[2];
    ccf_flags |= (1 << 19) * ccf.do_mix[3];
    ccf_flags |= (1 << 20) * ccf.do_noise;
    ccf_flags |= (1 << 21) * prg->cc->cm.use_alpha;
    ccf_flags |= (1 << 22) * prg->cc->cm.use_fog;
    ccf_flags |= (1 << 23) * prg->cc->cm.texture_edge;
    ccf_flags |= (1 << 24) * prg->cc->cm.use_dither;
    ccf_flags |= (1 << 25) * prg->cc->cm.use_2cycle;
    ccf_flags |= (1 << 26) * prg->cc->cm.light_map;
    ccf_flags |= (1 << 27) * prg->cc->cm.world_geometry;
    glUniform1i(prg->uniform_locations[UNIFORM_CCF], ccf_flags);

    static int sShaderCommands[16];
    for (int i = 0; i != 16; ++i) {
        sShaderCommands[i] = prg->cc->shader_commands[i];
    }


    glUniform1iv(prg->uniform_locations[UNIFORM_SHADER_COMMANDS], 16, sShaderCommands);

    // (opt_alpha && opt_dither) || ccf.do_noise
    /*if (prg->used_noise ccf.do_noise) {*/ glUniform1f(prg->uniform_locations[UNIFORM_FRAME_COUNT], (float)frame_count); /*}*/
    if (/*prg->used_lightmap*/ prg->cc->cm.light_map) { glUniform3f(prg->uniform_locations[UNIFORM_LIGHT_MAP], gVertexColor[0] / 255.0f, gVertexColor[1] / 255.0f, gVertexColor[2] / 255.0f); }
    if (/*prg->world_geometry*/ prg->cc->cm.world_geometry) {
        glUniform1iv(prg->uniform_locations[UNIFORM_SHADER_FLAGS], SHADER_FLAG_MAX, gShaderFlags);
        glUniform1fv(prg->uniform_locations[UNIFORM_SHADER_FLAG_VALUES], SHADER_FLAG_MAX, gShaderFlagValues);
    }
    glUniform1i(prg->uniform_locations[UNIFORM_FILTERING], configFiltering);
}

static inline void gfx_opengl_set_texture_uniforms(struct ShaderProgram *prg, const int tile) {

    struct CCFeatures ccf = { 0 };
    gfx_cc_get_features(prg->cc, &ccf);

    if (/*prg->used_textures[tile]*/ ccf.used_textures[tile] && opengl_tex[tile]) {
        glUniform2f(prg->uniform_locations[tile*2 + 0], opengl_tex[tile]->size[0], opengl_tex[tile]->size[1]);
        glUniform1i(prg->uniform_locations[tile*2 + 1], opengl_tex[tile]->filter);
    }
}

static void gfx_opengl_unload_shader(struct ShaderProgram *old_prg) {
    if (old_prg != NULL) {
        for (int i = 0; i < old_prg->num_attribs; i++)
            glDisableVertexAttribArray(old_prg->attrib_locations[i]);
        if (old_prg == opengl_prg)
            opengl_prg = NULL;
    } else {
        opengl_prg = NULL;
    }
}

static void gfx_opengl_load_shader(struct ShaderProgram *new_prg) {
    opengl_prg = new_prg;
    glUseProgram(new_prg->opengl_program_id);
    gfx_opengl_vertex_array_set_attribs(new_prg);
    gfx_opengl_set_shader_uniforms(new_prg);
    gfx_opengl_set_texture_uniforms(new_prg, 0);
    gfx_opengl_set_texture_uniforms(new_prg, 1);
}

static struct ShaderProgram *gfx_opengl_create_and_load_new_shader(struct ColorCombiner* cc) {
    char vs_buf[8192];
    char fs_buf[32768];
    size_t vs_len = 0;
    size_t fs_len = 0;
    size_t num_floats = 4;

    // Vertex shader
    num_floats = 4 + 2 * NUM_TEXTURES + 4 + 2 + /*ccf.num_inputs*/8 * 4;

    strcpy(vs_buf, UBER_VERT);
    vs_len = strlen(vs_buf);

    // Fragment shader
    strcpy(fs_buf, UBER_FRAG);
    fs_len = strlen(fs_buf);

    vs_buf[vs_len] = '\0';
    fs_buf[fs_len] = '\0';

    // puts("Vertex shader:");
    // puts(vs_buf);
    // puts("Fragment shader:");
    // puts(fs_buf);
    // puts("End");

    const GLchar *sources[2] = { vs_buf, fs_buf };
    const GLint lengths[2] = { vs_len, fs_len };
    GLint success;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &sources[0], &lengths[0]);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint max_length = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &max_length);
        char error_log[1024];
        fprintf(stderr, "Vertex shader compilation failed\n");
        glGetShaderInfoLog(vertex_shader, max_length, &max_length, &error_log[0]);
        fprintf(stderr, "%s\n", &error_log[0]);
        sys_fatal("vertex shader compilation failed (see terminal)");
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &sources[1], &lengths[1]);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint max_length = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);
        char error_log[1024];
        fprintf(stderr, "Fragment shader compilation failed\n");
        glGetShaderInfoLog(fragment_shader, max_length, &max_length, &error_log[0]);
        fprintf(stderr, "%s\n", &error_log[0]);
        sys_fatal("fragment shader compilation failed (see terminal)");
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    size_t num_attribs = 0;

    struct ShaderProgram *prg = &shader_program_pool[shader_program_pool_index];
    shader_program_pool_index = (shader_program_pool_index + 1) % CC_MAX_SHADERS;
    if (shader_program_pool_size < CC_MAX_SHADERS) { shader_program_pool_size++; }

    prg->attrib_locations[num_attribs] = glGetAttribLocation(shader_program, "aVtxPos");
    prg->attrib_sizes[num_attribs] = 4;
    ++num_attribs;

    for (int t = 0; t < 2; t++) {
        char name[16];
        sprintf(name, "aTexCoord%d", t);
        prg->attrib_locations[num_attribs] = glGetAttribLocation(shader_program, name);
        prg->attrib_sizes[num_attribs] = 2;
        ++num_attribs;
    }

    prg->attrib_locations[num_attribs] = glGetAttribLocation(shader_program, "aFog");
    prg->attrib_sizes[num_attribs] = 4;
    ++num_attribs;

    prg->attrib_locations[num_attribs] = glGetAttribLocation(shader_program, "aLightMap");
    prg->attrib_sizes[num_attribs] = 2;
    ++num_attribs;

    for (int i = 0; i < 8; i++) {
        char name[16];
        sprintf(name, "aInput%d", i + 1);
        prg->attrib_locations[num_attribs] = glGetAttribLocation(shader_program, name);
        prg->attrib_sizes[num_attribs] = 4;
        ++num_attribs;
    }

    prg->hash = cc->hash;
    prg->opengl_program_id = shader_program;
    prg->num_inputs = 8;//ccf.num_inputs;
    prg->num_floats = num_floats;
    prg->num_attribs = num_attribs;

    glUseProgram(shader_program);
    for (int t = 0; t < 2; t++) {
        char name[16];
        sprintf(name, "uTex%d", t);
        GLint sampler_location = glGetUniformLocation(shader_program, name);
        sprintf(name, "uTex%dSize", t);
        prg->uniform_locations[t * 2] = glGetUniformLocation(shader_program, name);
        sprintf(name, "uTex%dFilter", t);
        prg->uniform_locations[t * 2 + 1] = glGetUniformLocation(shader_program, name);
        glUniform1i(sampler_location, t);
    }

    prg->uniform_locations[UNIFORM_CCF] = glGetUniformLocation(shader_program, "uCCF");
    prg->uniform_locations[UNIFORM_SHADER_COMMANDS] = glGetUniformLocation(shader_program, "uShaderCommands");
    prg->uniform_locations[UNIFORM_FRAME_COUNT] = glGetUniformLocation(shader_program, "uFrameCount");
    prg->uniform_locations[UNIFORM_LIGHT_MAP] = glGetUniformLocation(shader_program, "uLightmapColor");
    prg->uniform_locations[UNIFORM_SHADER_FLAGS] = glGetUniformLocation(shader_program, "uShaderFlags");
    prg->uniform_locations[UNIFORM_SHADER_FLAG_VALUES] = glGetUniformLocation(shader_program, "uShaderFlagValues");
    prg->uniform_locations[UNIFORM_FILTERING] = glGetUniformLocation(shader_program, "uFilter");

    prg->cc = cc;

    gfx_opengl_load_shader(prg);

    return prg;
}

static struct ShaderProgram *gfx_opengl_lookup_shader(struct ColorCombiner* cc) {
    for (size_t i = 0; i < shader_program_pool_size; i++) {
        if (shader_program_pool[i].hash == cc->hash) {
            return &shader_program_pool[i];
        }
    }
    return NULL;
}

static void gfx_opengl_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    *num_inputs = prg->num_inputs;

    // if (gCC) {

    struct CCFeatures ccf = { 0 };
    gfx_cc_get_features(prg->cc, &ccf);
    used_textures[0] = ccf.used_textures[0];
    used_textures[1] = ccf.used_textures[1];

    // }

    // used_textures[0] = prg->used_textures[0];
    // used_textures[1] = prg->used_textures[1];
}

static GLuint gfx_opengl_new_texture(void) {
    if (num_textures >= tex_cache_size) {
        tex_cache_size += TEX_CACHE_STEP;
        tex_cache = realloc(tex_cache, sizeof(struct GLTexture) * tex_cache_size);
        if (!tex_cache) sys_fatal("out of memory allocating texture cache");
        // invalidate these because they might be pointing to garbage now
        opengl_tex[0] = NULL;
        opengl_tex[1] = NULL;
    }
    glGenTextures(1, &tex_cache[num_textures].gltex);
    return num_textures++;
}

static void gfx_opengl_select_texture(int tile, GLuint texture_id) {
    opengl_tex[tile] = tex_cache + texture_id;
    opengl_curtex = tile;
    glActiveTexture(GL_TEXTURE0 + tile);
    glBindTexture(GL_TEXTURE_2D, opengl_tex[tile]->gltex);
    gfx_opengl_set_texture_uniforms(opengl_prg, tile);
}

static void gfx_opengl_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    if (width <= 0 || height <= 0) { sys_fatal("Texture dimensions are invalid!"); }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba32_buf);
    opengl_tex[opengl_curtex]->size[0] = width;
    opengl_tex[opengl_curtex]->size[1] = height;
}

static uint32_t gfx_cm_to_opengl(uint32_t val) {
    if (val & G_TX_CLAMP) {
        return GL_CLAMP_TO_EDGE;
    }
    return (val & G_TX_MIRROR) ? GL_MIRRORED_REPEAT : GL_REPEAT;
}

static void gfx_opengl_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms, uint32_t cmt) {
    const GLenum filter = linear_filter ? GL_LINEAR : GL_NEAREST;
    glActiveTexture(GL_TEXTURE0 + tile);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gfx_cm_to_opengl(cms));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gfx_cm_to_opengl(cmt));
    opengl_curtex = tile;
    if (opengl_tex[tile]) {
        opengl_tex[tile]->filter = linear_filter;
        gfx_opengl_set_texture_uniforms(opengl_prg, tile);
    }
}

static void gfx_opengl_set_depth_test(bool depth_test) {
    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

static void gfx_opengl_set_depth_mask(bool z_upd) {
    glDepthMask(z_upd ? GL_TRUE : GL_FALSE);
}

static void gfx_opengl_set_zmode_decal(bool zmode_decal) {
    if (zmode_decal) {
        glPolygonOffset(-2, -2);
        glEnable(GL_POLYGON_OFFSET_FILL);
    } else {
        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

static void gfx_opengl_set_viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

static void gfx_opengl_set_scissor(int x, int y, int width, int height) {
    glScissor(x, y, width, height);
}

static void gfx_opengl_set_use_alpha(bool use_alpha) {
    if (use_alpha) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

static void gfx_opengl_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    //printf("flushing %d tris\n", buf_vbo_num_tris);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buf_vbo_len, buf_vbo, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3 * buf_vbo_num_tris);
}

static inline bool gl_version_is_supported(int major, int minor, bool is_es) {
    if (is_es) {
        return major >= 2;
    }
    return (major > 2) || (major == 2 && minor >= 1);
}

static inline bool gl_get_version(int *major, int *minor, bool *is_es) {
    const char *vstr = (const char *)glGetString(GL_VERSION);
    if (!vstr || !vstr[0]) return false;

    if (!strncmp(vstr, "OpenGL ES ", 10)) {
        vstr += 10;
        *is_es = true;
    } else if (!strncmp(vstr, "OpenGL ES-CM ", 13)) {
        vstr += 13;
        *is_es = true;
    }

    return (sscanf(vstr, "%d.%d", major, minor) == 2);
}

static void gfx_opengl_init(void) {
#if FOR_WINDOWS || defined(OSX_BUILD)
    GLenum err;
    if ((err = glewInit()) != GLEW_OK)
        sys_fatal("could not init GLEW:\n%s", glewGetErrorString(err));
#endif

    tex_cache_size = TEX_CACHE_STEP;
    tex_cache = calloc(tex_cache_size, sizeof(struct GLTexture));
    if (!tex_cache) sys_fatal("out of memory allocating texture cache");

    // check GL version
    int vmajor = 0;
    int vminor = 0;
    bool is_es = false;
    if (!gl_get_version(&vmajor, &vminor, &is_es) || !gl_version_is_supported(vmajor, vminor, is_es)) {
        sys_fatal("OpenGL 2.1+ is required.\nReported version: %s%d.%d", is_es ? "ES" : "", vmajor, vminor);
    }

    glGenBuffers(1, &opengl_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, opengl_vbo);

    if (vmajor >= 3 && !is_es) {
        glGenVertexArrays(1, &opengl_vao);
        glBindVertexArray(opengl_vao);
    }

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool gfx_opengl_check_compatibility(void) {
    // check GL version
    int vmajor = 0;
    int vminor = 0;
    bool is_es = false;
    if (!gl_get_version(&vmajor, &vminor, &is_es)) {
        return false;
    }
    return gl_version_is_supported(vmajor, vminor, is_es);
}

static void gfx_opengl_on_resize(void) {
}

static void gfx_opengl_start_frame(void) {
    frame_count++;

    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE); // Must be set to clear Z-buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

static void gfx_opengl_end_frame(void) {
}

static void gfx_opengl_finish_render(void) {
}

static const char* gfx_opengl_get_name(void) {
    return "OpenGL";
}

static void gfx_opengl_shutdown(void) {
}

struct GfxRenderingAPI gfx_opengl_api = {
    gfx_opengl_z_is_from_0_to_1,
    gfx_opengl_unload_shader,
    gfx_opengl_load_shader,
    gfx_opengl_create_and_load_new_shader,
    gfx_opengl_lookup_shader,
    gfx_opengl_shader_get_info,
    gfx_opengl_new_texture,
    gfx_opengl_select_texture,
    gfx_opengl_upload_texture,
    gfx_opengl_set_sampler_parameters,
    gfx_opengl_set_depth_test,
    gfx_opengl_set_depth_mask,
    gfx_opengl_set_zmode_decal,
    gfx_opengl_set_viewport,
    gfx_opengl_set_scissor,
    gfx_opengl_set_use_alpha,
    gfx_opengl_draw_triangles,
    gfx_opengl_init,
    gfx_opengl_on_resize,
    gfx_opengl_start_frame,
    gfx_opengl_end_frame,
    gfx_opengl_finish_render,
    gfx_opengl_get_name,
    gfx_opengl_shutdown
};
