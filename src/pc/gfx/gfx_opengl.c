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
#include "gfx_pc.h"
#include "gfx_rendering_api.h"
#include "gfx_shaders.h"

#define TEX_CACHE_STEP 512

#define tsizeof(TYPE, MEMBER) sizeof(((TYPE *)0)->MEMBER)

struct GLShaderProgram {
    GLuint program;
    GLint attrib_locations[MAX_ATTRIBUTES];
    GLint uniform_locations[MAX_UNIFORMS];
    struct ColorCombiner cc;
};

struct GLTexture {
    GLuint gltex;
    GLfloat size[2];
    uint8_t filter;
};

static GLuint opengl_vbo;
static GLuint opengl_vao;

static int tex_cache_size = 0;
static int num_textures = 0;
static struct GLTexture *tex_cache = NULL;

static struct GLShaderProgram opengl_prg = {0};
static struct GLTexture *opengl_tex[MAX_TEXTURES];
static int opengl_curtex = 0;

static uint32_t frame_count;

  ////////////////
 // Attributes //
////////////////

struct GLAttribute {
    const char *name;
    size_t count;
    size_t size;
    GLenum type;
    bool normalized;
    size_t pointer;
    size_t offset;
};

static const struct GLAttribute GL_ATTRIBUTES[] = {
    [ATTRIBUTE_POS] = { "aVtxPos", 1, 4, GL_FLOAT, false, offsetof(struct VertexAttribs, pos), 0 },
    [ATTRIBUTE_TEX_COORDS] = { "aTexCoords", MAX_TEXTURES, 2, GL_FLOAT, false, offsetof(struct VertexAttribs, tex_coords), tsizeof(struct VertexAttribs, tex_coords[0]) },
    [ATTRIBUTE_INPUTS] = { "aInputs", MAX_SHADER_INPUTS, 4, GL_UNSIGNED_BYTE, true, offsetof(struct VertexAttribs, inputs), tsizeof(struct VertexAttribs, inputs[0]) },
    [ATTRIBUTE_FOG] = { "aFog", 1, 4, GL_UNSIGNED_BYTE, true, offsetof(struct VertexAttribs, fog), 0 },
    [ATTRIBUTE_LIGHT_MAP] = { "aLightMap", 1, 2, GL_UNSIGNED_SHORT, true, offsetof(struct VertexAttribs, light_map) },
};

static void gfx_opengl_vertex_array_locate_attribs(struct GLShaderProgram *prg) {
    for (size_t i = 0; i < MAX_ATTRIBUTES; ++i) {
        const struct GLAttribute *attrib = &GL_ATTRIBUTES[i];
        prg->attrib_locations[i] = glGetAttribLocation(prg->program, attrib->name);
    }
}

static void gfx_opengl_vertex_array_set_attribs(struct GLShaderProgram *prg) {
    const size_t stride = sizeof(struct VertexAttribs);
    for (size_t i = 0; i < MAX_ATTRIBUTES; ++i) {
        const struct GLAttribute *attrib = &GL_ATTRIBUTES[i];
        for (size_t j = 0; j < attrib->count; ++j) {
            int index = prg->attrib_locations[i] + j;
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, attrib->size, attrib->type, attrib->normalized, stride, (const void *) (attrib->pointer + j * attrib->offset));
        }
    }
}

  //////////////
 // Uniforms //
//////////////

struct GLUniform {
    const char *name;
};

static const struct GLUniform GL_UNIFORMS[] = {
    [UNIFORM_CCS] = { "uCCS" },
    [UNIFORM_CCF] = { "uCCF" },
    [UNIFORM_NOISE_FRAME_COUNT] = { "uNoiseFrameCount" },
    [UNIFORM_LIGHT_MAP_COLOR] = { "uLightMapColor" },
    [UNIFORM_SHADER_FLAGS] = { "uShaderFlags" },
    [UNIFORM_SHADER_FLAG_VALUES] = { "uShaderFlagValues" },
    [UNIFORM_TEX_PROPERTIES] = { "uTexProperties" },
};

static void gfx_opengl_locate_uniforms(struct GLShaderProgram *prg) {
    for (size_t i = 0; i < MAX_UNIFORMS; ++i) {
        const struct GLUniform *uniform = &GL_UNIFORMS[i];
        opengl_prg.uniform_locations[i] = glGetUniformLocation(prg->program, uniform->name);
    }

    // Samplers
    GLint sampler_location = glGetUniformLocation(prg->program, "uTextures");
    for (int i = 0; i < MAX_TEXTURES; ++i) { glUniform1i(sampler_location + i, i); }
}

static inline void gfx_opengl_set_shader_uniforms(struct GLShaderProgram *prg) {

    // CC sources
    static int sCCS[CC_CHANNELS * CC_CYCLES];
    for (int i = 0; i < ARRAY_COUNT(sCCS); ++i) {
        sCCS[i] = (
            (prg->cc.ccs[i * CC_COMPONENTS + 0] << 24) |
            (prg->cc.ccs[i * CC_COMPONENTS + 1] << 16) |
            (prg->cc.ccs[i * CC_COMPONENTS + 2] << 8) |
            (prg->cc.ccs[i * CC_COMPONENTS + 3] << 0)
        );
    }
    glUniform1iv(prg->uniform_locations[UNIFORM_CCS], ARRAY_COUNT(sCCS), sCCS);

    // CC features
    int ccf = 0;
    ccf |= (prg->cc.use_noise << CCF_NOISE);
    ccf |= (prg->cc.use_alpha << CCF_USE_ALPHA);
    ccf |= (prg->cc.use_fog << CCF_USE_FOG);
    ccf |= (prg->cc.texture_edge << CCF_TEX_EDGE);
    ccf |= (prg->cc.use_dither << CCF_USE_DITHER);
    ccf |= (prg->cc.use_2cycle << CCF_USE_2CYCLE);
    ccf |= (prg->cc.light_map << CCF_LIGHT_MAP);
    ccf |= (prg->cc.world_geometry << CCF_WORLD_GEOMETRY);
    ccf |= (prg->cc.used_textures << CCF_USED_TEXTURE);
    glUniform1i(prg->uniform_locations[UNIFORM_CCF], ccf);

    // Frame counter for noise effect
    if ((prg->cc.use_alpha && prg->cc.use_dither) || prg->cc.use_noise) {
        glUniform1f(prg->uniform_locations[UNIFORM_NOISE_FRAME_COUNT], (float) frame_count);
    }

    // Lightmap
    if (prg->cc.light_map) {
        glUniform3f(prg->uniform_locations[UNIFORM_LIGHT_MAP_COLOR], gVertexColor[0] / 255.0f, gVertexColor[1] / 255.0f, gVertexColor[2] / 255.0f);
    }

    // Global shader effects
    if (prg->cc.world_geometry) {
        int shader_flags = 0;
        for (int i = 0; i < SHADER_FLAG_MAX; ++i) { shader_flags |= (gShaderFlags[i] << i); }
        glUniform1i(prg->uniform_locations[UNIFORM_SHADER_FLAGS], shader_flags);
        glUniform1fv(prg->uniform_locations[UNIFORM_SHADER_FLAG_VALUES], SHADER_FLAG_MAX, gShaderFlagValues);
    }
}

static inline void gfx_opengl_set_texture_uniforms(struct GLShaderProgram *prg, const int tile) {
    if ((prg->cc.used_textures & USED_TEXTURE(tile)) && opengl_tex[tile]) {
        glUniform3f(prg->uniform_locations[UNIFORM_TEX_PROPERTIES],
            opengl_tex[tile]->size[0],
            opengl_tex[tile]->size[1],
            opengl_tex[tile]->filter
        );
    }
}

  /////////////
 // Shaders //
/////////////

static const char UBERSHADER_VERT[] = {
'#','v','e','r','s','i','o','n',' ','1','3','0','\n'
,
#embed "gfx_cc.inl"
,
#embed "shaders/ubershader.vert.glsl"
,0
};

static const char UBERSHADER_FRAG[] = {
'#','v','e','r','s','i','o','n',' ','1','3','0','\n'
,
#embed "gfx_cc.inl"
,
#embed "shaders/ubershader.frag.glsl"
,0
};

static void gfx_opengl_load_ubershader(void) {
    const GLchar *sources[] = { UBERSHADER_VERT, UBERSHADER_FRAG };
    const GLint lengths[] = { sizeof(UBERSHADER_VERT) - 1, sizeof(UBERSHADER_FRAG) - 1 };
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
    glUseProgram(shader_program);

    opengl_prg.program = shader_program;

    // Set up attributes
    gfx_opengl_vertex_array_locate_attribs(&opengl_prg);
    gfx_opengl_vertex_array_set_attribs(&opengl_prg);

    // Set up uniforms
    gfx_opengl_locate_uniforms(&opengl_prg);
}

static bool gfx_opengl_z_is_from_0_to_1(void) {
    return false;
}

static bool gfx_opengl_update_cc(struct ColorCombiner *cc) {
    if (memcmp(&opengl_prg.cc, cc, sizeof(*cc)) != 0) {
        opengl_prg.cc = *cc;
        return true;
    }
    return false;
}

static void gfx_opengl_update_uniforms(void) {
    gfx_opengl_set_shader_uniforms(&opengl_prg);
    for (int t = 0; t < MAX_TEXTURES; t++) {
        gfx_opengl_set_texture_uniforms(&opengl_prg, t);
    }
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
    gfx_opengl_set_texture_uniforms(&opengl_prg, tile);
}

static void gfx_opengl_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    if (width <= 0 || height <= 0) { sys_fatal("Texture dimensions are invalid!"); }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba32_buf);
    opengl_tex[opengl_curtex]->size[0] = width;
    opengl_tex[opengl_curtex]->size[1] = height;
}

static uint32_t gfx_cm_to_opengl(uint32_t val) {
    if (val & G_TX_CLAMP) { return GL_CLAMP_TO_EDGE; }
    if (val & G_TX_MIRROR) { return GL_MIRRORED_REPEAT; }
    return GL_REPEAT;
}

static void gfx_opengl_set_sampler_parameters(int tile, uint8_t filter, uint32_t cms, uint32_t cmt) {
    const GLenum gl_filter = filter != 0 ? GL_LINEAR : GL_NEAREST;
    glActiveTexture(GL_TEXTURE0 + tile);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gfx_cm_to_opengl(cms));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gfx_cm_to_opengl(cmt));
    opengl_curtex = tile;
    if (opengl_tex[tile]) {
        opengl_tex[tile]->filter = filter;
        gfx_opengl_set_texture_uniforms(&opengl_prg, tile);
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

static void gfx_opengl_draw_triangles(struct VertexAttribs *buf_vbo, size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct VertexAttribs) * buf_vbo_len, buf_vbo, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3 * buf_vbo_num_tris);
}

static inline bool gl_version_is_supported(int major, int minor, bool is_es) {
    if (is_es) { return major >= 2; }
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

    gfx_opengl_load_ubershader();
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

static const char *gfx_opengl_get_name(void) {
    return "OpenGL";
}

static void gfx_opengl_shutdown(void) {
}

struct GfxRenderingAPI gfx_opengl_api = {
    .z_is_from_0_to_1 = gfx_opengl_z_is_from_0_to_1,
    .update_cc = gfx_opengl_update_cc,
    .update_uniforms = gfx_opengl_update_uniforms,
    .new_texture = gfx_opengl_new_texture,
    .select_texture = gfx_opengl_select_texture,
    .upload_texture = gfx_opengl_upload_texture,
    .set_sampler_parameters = gfx_opengl_set_sampler_parameters,
    .set_depth_test = gfx_opengl_set_depth_test,
    .set_depth_mask = gfx_opengl_set_depth_mask,
    .set_zmode_decal = gfx_opengl_set_zmode_decal,
    .set_viewport = gfx_opengl_set_viewport,
    .set_scissor = gfx_opengl_set_scissor,
    .set_use_alpha = gfx_opengl_set_use_alpha,
    .draw_triangles = gfx_opengl_draw_triangles,
    .init = gfx_opengl_init,
    .on_resize = gfx_opengl_on_resize,
    .start_frame = gfx_opengl_start_frame,
    .end_frame = gfx_opengl_end_frame,
    .finish_render = gfx_opengl_finish_render,
    .get_name = gfx_opengl_get_name,
    .shutdown = gfx_opengl_shutdown
};
