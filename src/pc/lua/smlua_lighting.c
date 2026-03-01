#include "smlua.h"
#include "pc/gfx/gfx_lighting.h"
#include "pc/gfx/gfx_shader_pack.h"
#include "pc/configfile.h"

static int smlua_lighting_init(UNUSED lua_State* L) {
    gfx_lighting_init();
    return 0;
}

static int smlua_lighting_shutdown(UNUSED lua_State* L) {
    gfx_lighting_shutdown();
    return 0;
}

static int smlua_lighting_reset(UNUSED lua_State* L) {
    gfx_lighting_reset();
    return 0;
}

static int smlua_lighting_add_point_light(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 6)) { return 0; }
    
    float pos[3] = {
        smlua_to_number(L, 1),
        smlua_to_number(L, 2),
        smlua_to_number(L, 3)
    };
    
    float color[3];
    lua_getfield(L, 4, "r");
    color[0] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 4, "g");
    color[1] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 4, "b");
    color[2] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    float intensity = smlua_to_number(L, 5);
    float radius = smlua_to_number(L, 6);
    
    int lightIndex = gfx_lighting_add_point_light(pos, color, intensity, radius);
    lua_pushinteger(L, lightIndex);
    return 1;
}

static int smlua_lighting_add_directional_light(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 5)) { return 0; }
    
    float dir[3] = {
        smlua_to_number(L, 1),
        smlua_to_number(L, 2),
        smlua_to_number(L, 3)
    };
    
    float color[3];
    lua_getfield(L, 4, "r");
    color[0] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 4, "g");
    color[1] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 4, "b");
    color[2] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    float intensity = smlua_to_number(L, 5);
    
    int lightIndex = gfx_lighting_add_directional_light(dir, color, intensity);
    lua_pushinteger(L, lightIndex);
    return 1;
}

static int smlua_lighting_add_spot_light(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 11)) { return 0; }
    
    float pos[3] = {
        smlua_to_number(L, 1),
        smlua_to_number(L, 2),
        smlua_to_number(L, 3)
    };
    
    float dir[3] = {
        smlua_to_number(L, 4),
        smlua_to_number(L, 5),
        smlua_to_number(L, 6)
    };
    
    float color[3];
    lua_getfield(L, 7, "r");
    color[0] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 7, "g");
    color[1] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 7, "b");
    color[2] = smlua_to_number(L, -1);
    lua_pop(L, 1);
    
    float intensity = smlua_to_number(L, 8);
    float radius = smlua_to_number(L, 9);
    float innerAngle = smlua_to_number(L, 10);
    float outerAngle = smlua_to_number(L, 11);
    
    int lightIndex = gfx_lighting_add_spot_light(pos, dir, color, intensity, radius, innerAngle, outerAngle);
    lua_pushinteger(L, lightIndex);
    return 1;
}

static int smlua_lighting_remove_light(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 1)) { return 0; }
    
    int lightIndex = smlua_to_integer(L, 1);
    gfx_lighting_remove_light(lightIndex);
    return 0;
}

static int smlua_lighting_enable_light(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 2)) { return 0; }
    
    int lightIndex = smlua_to_integer(L, 1);
    bool enabled = smlua_to_boolean(L, 2);
    gfx_lighting_enable_light(lightIndex, enabled);
    return 0;
}

static int smlua_lighting_set_light_shadow(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 2)) { return 0; }
    
    int lightIndex = smlua_to_integer(L, 1);
    bool castShadows = smlua_to_boolean(L, 2);
    gfx_lighting_set_light_shadow(lightIndex, castShadows);
    return 0;
}

static int smlua_lighting_set_ambient(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_range(L, 1, 2)) { return 0; }
    
    lua_getfield(L, 1, "r");
    lua_getfield(L, 1, "g");
    lua_getfield(L, 1, "b");
    float color[3] = {
        smlua_to_number(L, -3),
        smlua_to_number(L, -2),
        smlua_to_number(L, -1)
    };
    lua_pop(L, 3);
    
    float intensity = 1.0f;
    if (lua_gettop(L) >= 2) {
        intensity = smlua_to_number(L, 2);
    }
    
    gfx_lighting_set_ambient(color, intensity);
    return 0;
}

static int smlua_lighting_enable(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 1)) { return 0; }
    
    bool enabled = smlua_to_boolean(L, 1);
    gfx_lighting_enable(enabled);
    configLightingEnabled = enabled;
    return 0;
}

static int smlua_lighting_enable_shadows(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 1)) { return 0; }
    
    bool enabled = smlua_to_boolean(L, 1);
    gfx_lighting_enable_shadows(enabled);
    configLightingShadowsEnabled = enabled;
    return 0;
}

static int smlua_lighting_get_enabled(lua_State* L) {
    lua_pushboolean(L, gLightingState.lightingEnabled);
    return 1;
}

static int smlua_lighting_get_shadows_enabled(lua_State* L) {
    if (L == NULL) { return 0; }
    
    lua_pushboolean(L, gLightingState.shadowsEnabled);
    return 1;
}

static int smlua_lighting_set_shadow_quality(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 1)) { return 0; }
    
    unsigned int quality = smlua_to_integer(L, 1);
    gfx_lighting_set_shadow_quality(quality);
    return 0;
}

static int smlua_lighting_get_shadow_quality(lua_State* L) {
    if (L == NULL) { return 0; }
    
    extern unsigned int configLightingShadowQuality;
    lua_pushinteger(L, configLightingShadowQuality);
    return 1;
}

static int smlua_lighting_get_shadow_map_size(lua_State* L) {
    if (L == NULL) { return 0; }
    
    int size = gfx_lighting_get_shadow_map_size();
    lua_pushinteger(L, size);
    return 1;
}

static int smlua_shader_pack_get_count(lua_State* L) {
    int count = gfx_shader_pack_get_count();
    lua_pushinteger(L, count);
    return 1;
}

static int smlua_shader_pack_activate(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 1)) { return 0; }
    
    int packIndex = smlua_to_integer(L, 1);
    gfx_shader_pack_activate(packIndex);
    return 0;
}

static int smlua_shader_pack_deactivate(UNUSED lua_State* L) {
    gfx_shader_pack_deactivate();
    return 0;
}

static int smlua_shader_pack_set_enabled(lua_State* L) {
    if (L == NULL) { return 0; }
    
    if (!smlua_functions_valid_param_count(L, 1)) { return 0; }
    
    bool enabled = smlua_to_boolean(L, 1);
    gfx_shader_pack_set_enabled(enabled);
    configLightingShaderPacksEnabled = enabled;
    return 0;
}

static int smlua_shader_pack_is_enabled(lua_State* L) {
    lua_pushboolean(L, gfx_shader_pack_is_enabled());
    return 1;
}

void smlua_bind_lighting_functions(lua_State* L) {
    smlua_bind_function(L, "lighting_init", smlua_lighting_init);
    smlua_bind_function(L, "lighting_shutdown", smlua_lighting_shutdown);
    smlua_bind_function(L, "lighting_reset", smlua_lighting_reset);
    
    smlua_bind_function(L, "lighting_add_point_light", smlua_lighting_add_point_light);
    smlua_bind_function(L, "lighting_add_directional_light", smlua_lighting_add_directional_light);
    smlua_bind_function(L, "lighting_add_spot_light", smlua_lighting_add_spot_light);
    
    smlua_bind_function(L, "lighting_remove_light", smlua_lighting_remove_light);
    smlua_bind_function(L, "lighting_enable_light", smlua_lighting_enable_light);
    smlua_bind_function(L, "lighting_set_light_shadow", smlua_lighting_set_light_shadow);
    
    smlua_bind_function(L, "lighting_set_ambient", smlua_lighting_set_ambient);
    smlua_bind_function(L, "lighting_enable", smlua_lighting_enable);
    smlua_bind_function(L, "lighting_enable_shadows", smlua_lighting_enable_shadows);
    
    smlua_bind_function(L, "lighting_get_enabled", smlua_lighting_get_enabled);
    smlua_bind_function(L, "lighting_get_shadows_enabled", smlua_lighting_get_shadows_enabled);
    smlua_bind_function(L, "lighting_set_shadow_quality", smlua_lighting_set_shadow_quality);
    smlua_bind_function(L, "lighting_get_shadow_quality", smlua_lighting_get_shadow_quality);
    smlua_bind_function(L, "lighting_get_shadow_map_size", smlua_lighting_get_shadow_map_size);
    
    smlua_bind_function(L, "shader_pack_get_count", smlua_shader_pack_get_count);
    smlua_bind_function(L, "shader_pack_activate", smlua_shader_pack_activate);
    smlua_bind_function(L, "shader_pack_deactivate", smlua_shader_pack_deactivate);
    smlua_bind_function(L, "shader_pack_set_enabled", smlua_shader_pack_set_enabled);
    smlua_bind_function(L, "shader_pack_is_enabled", smlua_shader_pack_is_enabled);
}
