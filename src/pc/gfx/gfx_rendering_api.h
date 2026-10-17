#ifndef GFX_RENDERING_API_H
#define GFX_RENDERING_API_H

#include "gfx_cc.h"

struct GfxRenderingAPI {
    bool (*z_is_from_0_to_1)(void);
    bool (*update_cc)(struct ColorCombiner *cc);
    void (*update_uniforms)();
    void (*shader_get_info)(shader_inputs_t *shader_inputs, bool used_textures[]);
    uint32_t (*new_texture)(void);
    void (*select_texture)(int tile, uint32_t texture_id);
    void (*upload_texture)(const uint8_t *rgba32_buf, int width, int height);
    void (*set_sampler_parameters)(int sampler, uint8_t filter, uint32_t cms, uint32_t cmt);
    void (*set_depth_test)(bool depth_test);
    void (*set_depth_mask)(bool z_upd);
    void (*set_zmode_decal)(bool zmode_decal);
    void (*set_viewport)(int x, int y, int width, int height);
    void (*set_scissor)(int x, int y, int width, int height);
    void (*set_use_alpha)(bool use_alpha);
    void (*draw_triangles)(struct VertexAttribs *buf_vbo, size_t buf_vbo_len, size_t buf_vbo_num_tris);
    void (*init)(void);
    void (*on_resize)(void);
    void (*start_frame)(void);
    void (*end_frame)(void);
    void (*finish_render)(void);
    const char* (*get_name)(void);
    void (*shutdown)(void);
};

#endif
