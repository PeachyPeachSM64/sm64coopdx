#ifdef RAPI_WHB

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <gx2/display.h>
#include <gx2/draw.h>
#include <gx2/state.h>
#include <gx2/swap.h>
#include <whb/gfx.h>
#include <whb/log.h>

#include "gfx_rendering_api.h"
#include "gfx_cc.h"

#define WHB_LOG(msg) WHBLogPrintf("[WHB] " msg)

static uint32_t frame_count;
static int s_current_height;

void GX2SetViewport(float x, float y, float width, float height, float nearZ, float farZ);
void GX2SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void GX2DrawDone(void);

struct ShaderProgram {
    uint64_t hash;
    uint8_t num_inputs;
    bool used_textures[2];
};

static struct ShaderProgram shader_program_pool[64];
static uint8_t shader_program_pool_size = 0;

static bool gfx_whb_z_is_from_0_to_1(void) {
    return false;
}

static void gfx_whb_unload_shader(struct ShaderProgram *old_prg) {
    (void)old_prg;
}

static void gfx_whb_load_shader(struct ShaderProgram *new_prg) {
    (void)new_prg;
}

static struct ShaderProgram *gfx_whb_create_and_load_new_shader(struct ColorCombiner* cc) {
    WHB_LOG("create_and_load_new_shader called");
    
    struct ShaderProgram *prg = &shader_program_pool[shader_program_pool_size];
    if (shader_program_pool_size < 64) {
        shader_program_pool_size++;
    }
    
    prg->hash = cc->hash;
    prg->num_inputs = 2;
    prg->used_textures[0] = false;
    prg->used_textures[1] = false;
    
    return prg;
}

static struct ShaderProgram *gfx_whb_lookup_shader(struct ColorCombiner* cc) {
    for (size_t i = 0; i < shader_program_pool_size; i++) {
        if (shader_program_pool[i].hash == cc->hash) {
            return &shader_program_pool[i];
        }
    }
    return NULL;
}

static void gfx_whb_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs, bool used_textures[2]) {
    if (prg) {
        *num_inputs = prg->num_inputs;
        used_textures[0] = prg->used_textures[0];
        used_textures[1] = prg->used_textures[1];
    } else {
        *num_inputs = 2;
        used_textures[0] = false;
        used_textures[1] = false;
    }
}

static uint32_t sTextureCounter = 1;

static uint32_t gfx_whb_new_texture(void) {
    return sTextureCounter++;
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
    GX2SetViewport(x, s_current_height - y - height, width, height, 0.0f, 1.0f);
}

static void gfx_whb_set_scissor(int x, int y, int width, int height) {
    GX2SetScissor(x, s_current_height - y - height, width, height);
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
    WHB_LOG("gfx_whb_init: starting WHB graphics initialization");
    frame_count = 0;
    s_current_height = 0;
    
    if (!WHBGfxInit()) {
        WHB_LOG("gfx_whb_init: WHBGfxInit FAILED!");
    } else {
        WHB_LOG("gfx_whb_init: WHBGfxInit succeeded");
    }
}

static void gfx_whb_on_resize(void) {
}

static void gfx_whb_start_frame(void) {
    frame_count++;

    GX2ColorBuffer* tvBuffer = WHBGfxGetTVColourBuffer();
    if (tvBuffer != NULL) {
        s_current_height = (int)tvBuffer->surface.height;
    } else {
        WHB_LOG("start_frame: WARNING - NULL TV buffer!");
        s_current_height = 720;
    }

    WHBGfxBeginRenderTV();
    WHBGfxClearColor(0.1f, 0.1f, 0.2f, 1.0f);
}

static void gfx_whb_end_frame(void) {
    GX2Flush();
    GX2DrawDone();
    WHBGfxFinishRenderTV();
    GX2CopyColorBufferToScanBuffer(WHBGfxGetTVColourBuffer(), GX2_SCAN_TARGET_DRC);
}

static void gfx_whb_finish_render(void) {
}

static void gfx_whb_shutdown(void) {
    WHBGfxShutdown();
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
