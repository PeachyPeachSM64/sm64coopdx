#include <stdint.h>
#include <stdbool.h>

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

#include <SDL3/SDL.h>
#ifdef USE_GLES
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include "gfx_rendering_api.h"

#define TEX_CACHE_STEP 512

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

struct GLExtendedCombiner {
    int ops[CC_EXT_MAX_OPS / sizeof(int)];
    int num_ops[4];
    float constants[CC_EXT_MAX_CONSTANTS][4];
};

// Make sure data is aligned to 16 bytes boundaries
static_assert(offsetof(struct GLExtendedCombiner, num_ops) % 16 == 0);
static_assert(offsetof(struct GLExtendedCombiner, constants) % 16 == 0);
static_assert(sizeof(struct GLExtendedCombiner) % 16 == 0);

static GLuint opengl_vbo;
static GLuint opengl_vao;
static GLuint opengl_ubo;

static GLuint opengl_ubo_index;

static int tex_cache_size = 0;
static int num_textures = 0;
static struct GLTexture *tex_cache = NULL;

static struct GLShaderProgram opengl_prg = {0};
static struct ShaderUniforms opengl_uniforms = {0};
static struct GLTexture *opengl_tex[MAX_TEXTURES];
static int opengl_curtex = 0;

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
        sys_fatal("Error occurred when trying to locate vertex attributes:\nCannot create attribute locations");
    }

    for (size_t i = 0; i < num_attribs; ++i) {
        const struct ShaderDataProperties *attrib = gfx_shaders_get_attrib_properties(i);
        prg->attrib_locations[i] = glGetAttribLocation(prg->program, attrib->name);
        if (prg->attrib_locations[i] == -1) {
            sys_fatal("Error occurred when trying to locate vertex attributes:\nCannot locate attribute \"%s\" in vertex shader", attrib->name);
        }
    }
}

static void gfx_opengl_vertex_array_set_attribs(struct GLShaderProgram *prg) {
    const size_t num_attribs = gfx_shaders_get_num_attribs();
    const size_t stride = gfx_shaders_get_attribs_stride();
    for (size_t i = 0; i < num_attribs; ++i) {
        const struct ShaderDataProperties *attrib = gfx_shaders_get_attrib_properties(i);
        for (size_t j = 0; j < attrib->count; ++j) {
            int index = prg->attrib_locations[i] + j;
            const GLenum type = GL_TYPES[attrib->type];
            glEnableVertexAttribArray(index);
            if (type == GL_FLOAT) {
                glVertexAttribPointer(index, attrib->components, type, false, stride, (const void *) (attrib->offset + j * attrib->stride));
            } else {
                glVertexAttribIPointer(index, attrib->components, type, stride, (const void *) (attrib->offset + j * attrib->stride));
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
        sys_fatal("Error occurred when trying to locate shader uniforms:\nCannot create uniform locations");
    }

    for (size_t i = 0; i < num_uniforms; ++i) {
        const struct ShaderDataProperties *uniform = gfx_shaders_get_uniform_properties(i);
        prg->uniform_locations[i] = glGetUniformLocation(prg->program, uniform->name);
    }

    // Samplers
    GLint sampler_location = glGetUniformLocation(prg->program, "uTextures");
    for (int i = 0; i < MAX_TEXTURES; ++i) { glUniform1i(sampler_location + i, i); }
}

static void gfx_opengl_update_extended_combiner_uniform(struct GLShaderProgram *prg, int index) {
    const struct ExtendedCombiner *ec = prg->cc.ec[index];

    // Update ubo only when necessary
    if (prg->ec[index] == ec) {
        return;
    }

    // Convert extended combiner to a GPU friendly struct
    struct GLExtendedCombiner glec = {0};
    glec.num_ops[0] = ec->num_ops;
    // glec.num_constants[0] = ec->num_constants;
    for (uint32_t i = 0; i < ec->num_ops; ++i) {
        glec.ops[i / 4] |= (ec->ops[i] << (8 * (i % 4)));
    }
    memcpy(glec.constants, ec->constants, sizeof(*ec->constants) * ec->num_constants);

    // glBindBuffer(GL_UNIFORM_BUFFER, opengl_ubo); 
    // GLvoid *p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY); 
    // memcpy(p, &glec, sizeof(glec));
    // glUnmapBuffer(GL_UNIFORM_BUFFER);


    glBindBuffer(GL_UNIFORM_BUFFER, opengl_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, index * sizeof(glec), sizeof(glec), &glec);

    prg->ec[index] = ec;
}

static void gfx_opengl_set_texture_uniforms(struct GLShaderProgram *prg, const int tile) {
    if ((prg->cc.used_textures & USED_TEXTURE(tile)) && opengl_tex[tile]) {
        gfx_shaders_update_uniforms(&opengl_uniforms, NULL, &opengl_tex[tile]->properties, tile);
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
                default: sys_fatal("not implemented");
            }
        }
    }
}

  /////////////
 // Shaders //
/////////////

static const char GL_UBERSHADER_VERSION[] = "#version 330\n";

static const char GL_UBERSHADER_VERT[] = {
#embed "gfx_cc.inl"
,
#embed "shaders/opengl/ubershader.vert.glsl"
,0
};

static const char GL_UBERSHADER_FRAG[] = {
#embed "gfx_cc.inl"
,
#embed "gfx_cc_ext.inl"
,
#embed "shaders/opengl/ubershader.frag.glsl"
,0
};

static void gfx_opengl_load_ubershader(void) {
    GLint success;

    const GLchar *vertex_sources[] = {
        GL_UBERSHADER_VERSION,
        GL_UBERSHADER_VERT,
    };
    const GLint vertex_lengths[] = {
        sizeof(GL_UBERSHADER_VERSION) - 1,
        sizeof(GL_UBERSHADER_VERT) - 1,
    };

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, ARRAY_COUNT(vertex_sources), vertex_sources, vertex_lengths);
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

    // const char *cc_ext_shader_source = gfx_cc_ext_get_shader_source();
    const GLchar *fragment_sources[] = {
        GL_UBERSHADER_VERSION,
        // cc_ext_shader_source,
        GL_UBERSHADER_FRAG,
    };
    const GLint fragment_lengths[] = {
        sizeof(GL_UBERSHADER_VERSION) - 1,
        // strlen(cc_ext_shader_source),
        sizeof(GL_UBERSHADER_FRAG) - 1,
    };


    // printf("%s\n", cc_ext_shader_source);
    // exit(0);



    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, ARRAY_COUNT(fragment_sources), fragment_sources, fragment_lengths);
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

    GLint mvoc;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &mvoc);
    printf("%d\n", mvoc);
    glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &mvoc);
    printf("%d\n", mvoc);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint max_length = 0;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &max_length);
        char error_log[1024];
        fprintf(stderr, "Program link failed\n");
        glGetProgramInfoLog(shader_program, max_length, &max_length, &error_log[0]);
        fprintf(stderr, "%s\n", &error_log[0]);
        sys_fatal("program link failed (see terminal)");
    }

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
    gfx_shaders_update_uniforms(&opengl_uniforms, &opengl_prg.cc, NULL, 0);
    for (int t = 0; t < MAX_TEXTURES; t++) {
        gfx_opengl_set_texture_uniforms(&opengl_prg, t);
    }

    // Extended combiners
    // if (prg->cc.use_ext_color) {
    //     gfx_opengl_update_extended_combiner_uniform(prg, 0);
    // }
    // if (prg->cc.use_ext_alpha && !prg->cc.ext_color_alpha_same) {
    //     gfx_opengl_update_extended_combiner_uniform(prg, 1);
    // }
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
    opengl_tex[opengl_curtex]->properties.width = width;
    opengl_tex[opengl_curtex]->properties.height = height;
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
        opengl_tex[tile]->properties.filter = filter;
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

    // upload uniforms if they changed since last draw
    if (opengl_uniforms.dirty) {
        gfx_opengl_upload_shader_uniforms(&opengl_prg, &opengl_uniforms);
        opengl_uniforms.dirty = false;
    }

    // set vertex buffer and draw triangles
    void *data = gfx_shaders_get_attribs_data(buf_vbo, buf_vbo_len);
    if (data) {
        size_t stride = gfx_shaders_get_attribs_stride();
        glBufferData(GL_ARRAY_BUFFER, stride * buf_vbo_len, data, GL_STREAM_DRAW);
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

    // glGenBuffers(1, &opengl_ubo);
    // glBindBuffer(GL_UNIFORM_BUFFER, opengl_ubo);
    // glBufferData(GL_UNIFORM_BUFFER, sizeof(struct GLExtendedCombiner) * ARRAY_COUNT(opengl_prg.ec), NULL, GL_DYNAMIC_DRAW);

    // opengl_ubo_index = glGetUniformBlockIndex(opengl_prg.program, "ExtendedCombinerBlock");
    // glUniformBlockBinding(opengl_prg.program, opengl_ubo_index, opengl_ubo);

    // Bind to binding point 0
    // glBindBufferBase(GL_UNIFORM_BUFFER, 0, opengl_ubo);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gfx_opengl_load_ubershader();



    glGenBuffers(1, &opengl_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, opengl_ubo); 
    glBufferData(GL_UNIFORM_BUFFER, sizeof(struct GLExtendedCombiner) * ARRAY_COUNT(opengl_prg.ec), NULL, GL_DYNAMIC_DRAW); 
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    opengl_ubo_index = glGetUniformBlockIndex(opengl_prg.program, "ExtendedCombinerBlock");
    GLuint binding_point_index = 0;
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, opengl_ubo);
    glUniformBlockBinding(opengl_prg.program, opengl_ubo_index, binding_point_index);

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
    memset(&opengl_prg.cc, 0, sizeof(opengl_prg.cc));

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
