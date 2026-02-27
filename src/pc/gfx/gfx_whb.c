#ifdef RAPI_WHB

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gfx_rendering_api.h"

static bool gfx_whb_z_is_from_0_to_1(void) {
    return false;
}

static void gfx_whb_unload_shader(struct ShaderProgram *old_prg) {
}

static void gfx_whb_load_shader(struct ShaderProgram *new_prg) {
}

static struct ShaderProgram *gfx_whb_create_and_load_new_shader(struct ColorCombiner* cc) {
    (void)cc;
    return NULL;
}

static struct ShaderProgram *gfx_whb_lookup_shader(struct ColorCombiner* cc) {
    (void)cc;
    return NULL;
}

static void gfx_whb_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    (void)prg;
    *num_inputs = 0;
    used_textures[0] = false;
    used_textures[1] = false;
}

static uint32_t gfx_whb_new_texture(void) {
    return 0;
}

static void gfx_whb_select_texture(int tile, uint32_t texture_id) {
    (void)tile;
    (void)texture_id;
}

static void gfx_whb_upload_texture(const uint8_t *rgba32_buf, int width, int height) {
    (void)rgba32_buf;
    (void)width;
    (void)height;
}

static void gfx_whb_set_sampler_parameters(int sampler, bool linear_filter, uint32_t cms, uint32_t cmt) {
    (void)sampler;
    (void)linear_filter;
    (void)cms;
    (void)cmt;
}

static void gfx_whb_set_depth_test(bool depth_test) {
    (void)depth_test;
}

static void gfx_whb_set_depth_mask(bool z_upd) {
    (void)z_upd;
}

static void gfx_whb_set_zmode_decal(bool zmode_decal) {
    (void)zmode_decal;
}

static void gfx_whb_set_viewport(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

static void gfx_whb_set_scissor(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

static void gfx_whb_set_use_alpha(bool use_alpha) {
    (void)use_alpha;
}

static void gfx_whb_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    (void)buf_vbo;
    (void)buf_vbo_len;
    (void)buf_vbo_num_tris;
}

static void gfx_whb_init(void) {
}

static void gfx_whb_on_resize(void) {
}

static void gfx_whb_start_frame(void) {
}

static void gfx_whb_end_frame(void) {
}

static void gfx_whb_finish_render(void) {
}

static void gfx_whb_shutdown(void) {
}

struct GfxRenderingAPI gfx_whb_api = {
    gfx_whb_z_is_from_0_to_1,
    gfx_whb_unload_shader,
    gfx_whb_load_shader,
    gfx_whb_create_and_load_new_shader,
    gfx_whb_lookup_shader,
    gfx_whb_shader_get_info,
    gfx_whb_new_texture,
    gfx_whb_select_texture,
    gfx_whb_upload_texture,
    gfx_whb_set_sampler_parameters,
    gfx_whb_set_depth_test,
    gfx_whb_set_depth_mask,
    gfx_whb_set_zmode_decal,
    gfx_whb_set_viewport,
    gfx_whb_set_scissor,
    gfx_whb_set_use_alpha,
    gfx_whb_draw_triangles,
    gfx_whb_init,
    gfx_whb_on_resize,
    gfx_whb_start_frame,
    gfx_whb_end_frame,
    gfx_whb_finish_render,
    gfx_whb_shutdown,
};

#endif
