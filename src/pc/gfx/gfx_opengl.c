#include <stdint.h>
#include <stdbool.h>

#ifdef __MINGW32__
# define FOR_WINDOWS 1
#else
# define FOR_WINDOWS 0
#endif

#ifndef VSCODE
#if FOR_WINDOWS || defined(OSX_BUILD)
# define GLEW_STATIC
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

#include "gfx_rendering_api.h"
#include "game/memory.h"

#define VERTEX_RING_BUFFER 0
#if VERTEX_RING_BUFFER
#define VERTEX_RING_BUFFER_BATCHES 8
#define VERTEX_BUFFER_LENGTH (MAX_BUFFERED_VERTS * VERTEX_RING_BUFFER_BATCHES)
#endif

struct GLShaderProgram {
    GLuint program;
    GLint *attrib_locations;
    GLint *uniform_locations;
    struct ColorCombiner cc;
    const struct ExtendedCombiner *ec[2];
};

struct GLTexture {
    GLuint gltex;
    struct TextureProperties properties;
};

static struct {
    GLuint vbo;
    GLuint vao;
    GLuint ubo;
#if VERTEX_RING_BUFFER
    size_t vbo_offset;
#endif

    struct GLShaderProgram shader_program;
    struct ShaderUniforms uniforms;
    struct ExtendedCombiners combiners;

    struct GLTexture *textures[MAX_TEXTURES];
    struct GrowingArray *texture_cache;
    int current_tile;
} gl;

static const GLenum GL_TYPES[] = {
    [SHADER_DATA_TYPE_INT8]   = GL_BYTE,
    [SHADER_DATA_TYPE_UINT8]  = GL_UNSIGNED_BYTE,
    [SHADER_DATA_TYPE_INT16]  = GL_SHORT,
    [SHADER_DATA_TYPE_UINT16] = GL_UNSIGNED_SHORT,
    [SHADER_DATA_TYPE_INT32]  = GL_INT,
    [SHADER_DATA_TYPE_UINT32] = GL_UNSIGNED_INT,
    [SHADER_DATA_TYPE_FLOAT]  = GL_FLOAT,
};

  ////////////////
 // Attributes //
////////////////

static void gfx_opengl_vertex_array_locate_attribs(struct GLShaderProgram *prg) {
    const size_t num_attribs = gfx_shaders_get_num_attribs();
    prg->attrib_locations = malloc(sizeof(*prg->attrib_locations) * num_attribs);
    if (!prg->attrib_locations) {
        sys_fatal("Cannot create attribute locations");
    }

    for (size_t i = 0; i < num_attribs; ++i) {
        const struct ShaderDataProperties *attrib = gfx_shaders_get_attrib_properties(i);
        prg->attrib_locations[i] = glGetAttribLocation(prg->program, attrib->name);
        if (prg->attrib_locations[i] == -1) {
            sys_fatal("Cannot locate attribute \"%s\" in vertex shader", attrib->name);
        }
    }
}

static void gfx_opengl_vertex_array_set_attribs(struct GLShaderProgram *prg, size_t buffer_offset) {
    const size_t num_attribs = gfx_shaders_get_num_attribs();
    const size_t stride = gfx_shaders_get_attribs_stride();
    for (size_t i = 0; i < num_attribs; ++i) {
        const struct ShaderDataProperties *attrib = gfx_shaders_get_attrib_properties(i);
        for (size_t j = 0; j < attrib->count; ++j) {
            int index = prg->attrib_locations[i] + j;
            const GLenum type = GL_TYPES[attrib->type];
            glEnableVertexAttribArray(index);
            if (type == GL_FLOAT) {
                glVertexAttribPointer(index, attrib->components, type, false, stride, (const void *) (buffer_offset + attrib->offset + j * attrib->stride));
            } else {
                glVertexAttribIPointer(index, attrib->components, type, stride, (const void *) (buffer_offset + attrib->offset + j * attrib->stride));
            }
        }
    }
}

  //////////////
 // Uniforms //
//////////////

static void gfx_opengl_locate_uniforms(struct GLShaderProgram *prg) {
    const size_t num_uniforms = gfx_shaders_get_num_uniforms();
    prg->uniform_locations = malloc(sizeof(*prg->uniform_locations) * num_uniforms);
    if (!prg->uniform_locations) {
        sys_fatal("Cannot create uniform locations");
    }

    for (size_t i = 0; i < num_uniforms; ++i) {
        const struct ShaderDataProperties *uniform = gfx_shaders_get_uniform_properties(i);
        prg->uniform_locations[i] = glGetUniformLocation(prg->program, uniform->name);
        if (prg->uniform_locations[i] == -1) {
            sys_fatal("Cannot locate uniform \"%s\" in fragment shader", uniform->name);
        }
    }

    // Samplers
    GLint sampler_location = glGetUniformLocation(prg->program, "uTextures");
    if (sampler_location == -1) {
        sys_fatal("Cannot locate uniform \"uTextures\" in fragment shader");
    }
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        glUniform1i(sampler_location + i, i);
    }
}

static void gfx_opengl_set_texture_uniforms(struct GLShaderProgram *prg, const int tile) {
    if ((prg->cc.used_textures & USED_TEXTURE(tile)) && gl.textures[tile]) {
        gfx_shaders_update_uniforms(&gl.uniforms, NULL, &gl.textures[tile]->properties, tile);
    }
}

static void gfx_opengl_upload_shader_uniforms(struct GLShaderProgram *prg, const struct ShaderUniforms *uniforms) {
    void *data = gfx_shaders_get_uniforms_data(uniforms);
    if (data) {
        const size_t num_uniforms = gfx_shaders_get_num_uniforms();
        for (size_t i = 0; i < num_uniforms; ++i) {
            const struct ShaderDataProperties *uniform = gfx_shaders_get_uniform_properties(i);
            GLint location = prg->uniform_locations[i];
            switch2 (uniform->type, uniform->components) {
                case2 (SHADER_DATA_TYPE_INT32, 1): glUniform1iv(location, uniform->count, (int *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_INT32, 2): glUniform2iv(location, uniform->count, (int *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_INT32, 3): glUniform3iv(location, uniform->count, (int *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_INT32, 4): glUniform4iv(location, uniform->count, (int *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_FLOAT, 1): glUniform1fv(location, uniform->count, (float *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_FLOAT, 2): glUniform2fv(location, uniform->count, (float *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_FLOAT, 3): glUniform3fv(location, uniform->count, (float *) ((uintptr_t) data + uniform->offset)); break;
                case2 (SHADER_DATA_TYPE_FLOAT, 4): glUniform4fv(location, uniform->count, (float *) ((uintptr_t) data + uniform->offset)); break;
                default: sys_fatal("Uniform type not implemented: %d, %d", uniform->type, uniform->components);
            }
        }
    }
}

  ////////////////////////
 // Extended combiners //
////////////////////////

static void gfx_opengl_locate_extended_combiners(struct GLShaderProgram *prg) {
    GLuint ubo_index = glGetUniformBlockIndex(prg->program, "ExtendedCombinerBlock");
    if (ubo_index == GL_INVALID_INDEX) {
        sys_fatal("Cannot locate uniform block \"ExtendedCombinerBlock\" in fragment shader");
    }

    const size_t ops_stride = gfx_shaders_get_extended_combiners_ops_stride();
    const size_t constants_stride = gfx_shaders_get_extended_combiners_constants_stride();
    glGenBuffers(1, &gl.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, gl.ubo);
    glBufferData(GL_UNIFORM_BUFFER, (ops_stride + constants_stride) * 2, NULL, GL_STREAM_DRAW);

    GLuint binding_point_index = 0;
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, gl.ubo);
    glUniformBlockBinding(prg->program, ubo_index, binding_point_index);
}

static inline void gfx_opengl_upload_extended_combiner(void *ops, void *constants, bool alpha) {
    const size_t ops_stride = gfx_shaders_get_extended_combiners_ops_stride();
    const size_t constants_stride = gfx_shaders_get_extended_combiners_constants_stride();
    const size_t ops_offset = alpha * ops_stride;
    const size_t constants_offset = alpha * constants_stride;
    glBufferSubData(GL_UNIFORM_BUFFER, ops_offset + constants_offset, ops_stride, (void *) ((uintptr_t) ops + ops_offset));
    glBufferSubData(GL_UNIFORM_BUFFER, ops_offset + constants_offset + ops_stride, constants_stride, (void *) ((uintptr_t) constants + constants_offset));
}

static void gfx_opengl_upload_extended_combiners(struct GLShaderProgram *prg, const struct ExtendedCombiners *combiners) {
    void *ops = gfx_shaders_get_extended_combiners_ops_data(combiners);
    void *constants = gfx_shaders_get_extended_combiners_constants_data(combiners);
    if (ops && constants) {
        if (combiners->color) {
            gfx_opengl_upload_extended_combiner(ops, constants, false);
        }
        if (combiners->alpha) {
            gfx_opengl_upload_extended_combiner(ops, constants, true);
        }
    }
}

  /////////////
 // Shaders //
/////////////

static const char GL_UBERSHADER_HEADER[] = "\
#version 330\n\
#define SHADER_CODE\n\
#define static\n\
";

static const char GL_UBERSHADER_VERT[] = {
#ifndef VSCODE
#embed "gfx_cc.inl"
,
#embed "gfx_shaders.inl"
,
#embed "shaders/opengl/ubershader.vert.glsl"
,0
#endif
};

static const char GL_UBERSHADER_FRAG[] = {
#ifndef VSCODE
#embed "gfx_cc.inl"
,
#embed "gfx_cc_ext.inl"
,
#embed "gfx_shaders.inl"
,
#embed "shaders/opengl/ubershader.frag.glsl"
,0
#endif
};

#define fatal_error(type, id, message) { \
    GLint max_length = 0; \
    glGet##type##iv(id, GL_INFO_LOG_LENGTH, &max_length); \
    char *error_log = (char *) malloc(max_length); \
    fprintf(stderr, message "\n"); \
    glGet##type##InfoLog(id, max_length, &max_length, error_log); \
    fprintf(stderr, "%s\n", error_log); \
    sys_fatal(message " (see terminal)"); \
}

static void gfx_opengl_load_ubershader(void) {
    GLint success;

    //
    // Create and build vertex shader
    //

    const GLchar *vertex_sources[] = {
        GL_UBERSHADER_HEADER,
        GL_UBERSHADER_VERT,
    };
    const GLint vertex_lengths[] = {
        sizeof(GL_UBERSHADER_HEADER) - 1,
        sizeof(GL_UBERSHADER_VERT) - 1,
    };

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, ARRAY_COUNT(vertex_sources), vertex_sources, vertex_lengths);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        fatal_error(Shader, vertex_shader, "Vertex shader compilation failed");
    }

    //
    // Create and build fragment shader
    //

    const GLchar *fragment_sources[] = {
        GL_UBERSHADER_HEADER,
        GL_UBERSHADER_FRAG,
    };
    const GLint fragment_lengths[] = {
        sizeof(GL_UBERSHADER_HEADER) - 1,
        sizeof(GL_UBERSHADER_FRAG) - 1,
    };

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, ARRAY_COUNT(fragment_sources), fragment_sources, fragment_lengths);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        fatal_error(Shader, fragment_shader, "Fragment shader compilation failed");
    }

    //
    // Create and link shader program
    //

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        fatal_error(Program, shader_program, "Program linkage failed");
    }

    gl.shader_program.program = shader_program;

    // Set up attributes
    gfx_opengl_vertex_array_locate_attribs(&gl.shader_program);
    gfx_opengl_vertex_array_set_attribs(&gl.shader_program, 0);

    // Set up uniforms
    gfx_opengl_locate_uniforms(&gl.shader_program);

    // Set up extended combiners
    gfx_opengl_locate_extended_combiners(&gl.shader_program);
}

static bool gfx_opengl_z_is_from_0_to_1(void) {
    return false;
}

static bool gfx_opengl_update_cc(struct ColorCombiner *cc) {
    if (memcmp(&gl.shader_program.cc, cc, sizeof(*cc)) != 0) {
        gl.shader_program.cc = *cc;
        return true;
    }
    return false;
}

static void gfx_opengl_update_uniforms(void) {
    struct GLShaderProgram *prg = &gl.shader_program;

    gfx_shaders_update_uniforms(&gl.uniforms, &prg->cc, NULL, 0);
    for (int t = 0; t < MAX_TEXTURES; t++) {
        gfx_opengl_set_texture_uniforms(prg, t);
    }

    gfx_shaders_update_extended_combiners(&gl.combiners, &prg->cc);
}

static GLuint gfx_opengl_new_texture(void) {
    if (!gl.texture_cache) {
        gl.texture_cache = growing_array_init(NULL, 256, malloc, free);
    }
    struct GLTexture *tex = growing_array_alloc(gl.texture_cache, sizeof(struct GLTexture));
    if (!tex) {
        sys_fatal("Ran out of memory when creating texture!");
    }
    glGenTextures(1, &tex->gltex);
    return gl.texture_cache->count - 1;
}

static void gfx_opengl_select_texture(int tile, GLuint texture_id) {
    gl.textures[tile] = (struct GLTexture *) gl.texture_cache->buffer[texture_id];
    gl.current_tile = tile;
    glActiveTexture(GL_TEXTURE0 + tile);
    glBindTexture(GL_TEXTURE_2D, gl.textures[tile]->gltex);
    gfx_opengl_set_texture_uniforms(&gl.shader_program, tile);
}

static void gfx_opengl_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    if (width <= 0 || height <= 0) { sys_fatal("Texture dimensions are invalid!"); }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba32_buf);
    gl.textures[gl.current_tile]->properties.width = width;
    gl.textures[gl.current_tile]->properties.height = height;
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
    gl.current_tile = tile;
    if (gl.textures[tile]) {
        gl.textures[tile]->properties.filter = filter;
        gfx_opengl_set_texture_uniforms(&gl.shader_program, tile);
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

    // Set vertex buffer
    void *data = gfx_shaders_get_attribs_data(buf_vbo, buf_vbo_len);
    if (data) {
        const size_t stride = gfx_shaders_get_attribs_stride();
#if VERTEX_RING_BUFFER
        if (gl.vbo_offset + buf_vbo_len > VERTEX_BUFFER_LENGTH) {
            gl.vbo_offset = 0;
        }
        if (gl.vbo_offset == 0) {
            glBufferData(GL_ARRAY_BUFFER, VERTEX_BUFFER_LENGTH * stride, NULL, GL_STREAM_DRAW);
        }
        size_t buffer_offset = gl.vbo_offset * stride;
        gfx_opengl_vertex_array_set_attribs(&gl.shader_program, buffer_offset);
        glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, buf_vbo_len * stride, data);
        gl.vbo_offset += buf_vbo_len;
#else
        glBufferData(GL_ARRAY_BUFFER, stride * buf_vbo_len, data, GL_STREAM_DRAW);
#endif
    }

    // Upload uniforms if they changed since last draw
    if (gl.uniforms.dirty) {
        gfx_opengl_upload_shader_uniforms(&gl.shader_program, &gl.uniforms);
        gl.uniforms.dirty = false;
    }

    // Upload extended combiners if they changed since the last draw
    if (gl.combiners.dirty) {
        gfx_opengl_upload_extended_combiners(&gl.shader_program, &gl.combiners);
        gl.combiners.dirty = false;
    }

    // Draw triangles
    if (data) {
        glDrawArrays(GL_TRIANGLES, 0, 3 * buf_vbo_num_tris);
    }
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
        sys_fatal("Could not init GLEW:\n%s", glewGetErrorString(err));
#endif

    // check GL version
    int vmajor = 0;
    int vminor = 0;
    bool is_es = false;
    if (!gl_get_version(&vmajor, &vminor, &is_es) || !gl_version_is_supported(vmajor, vminor, is_es)) {
        sys_fatal("OpenGL 2.1+ is required.\nReported version: %s%d.%d", is_es ? "ES" : "", vmajor, vminor);
    }

    glGenBuffers(1, &gl.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
#if VERTEX_RING_BUFFER
    gl.vbo_offset = 0;
#endif

    if (vmajor >= 3 && !is_es) {
        glGenVertexArrays(1, &gl.vao);
        glBindVertexArray(gl.vao);
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
    // Reset color combiner to force a uniforms update on next update_cc
    memset(&gl.shader_program.cc, 0, sizeof(gl.shader_program.cc));

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
    growing_array_free(&gl.texture_cache);
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
