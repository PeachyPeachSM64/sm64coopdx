#include "dynos.cpp.h"
extern "C" {
#include "behavior_table.h"
#include "levels/scripts.h"
#include "object_fields.h"
#include "engine/behavior_script.h"
#include "engine/level_script.h"
#include "engine/surface_load.h"
#include "game/object_helpers.h"
#include "game/segment2.h"
#include "game/level_geo.h"
#include "game/level_update.h"
#include "game/moving_texture.h"
#include "game/paintings.h"
#include "game/geo_misc.h"
#include "game/mario_misc.h"
#include "game/mario_actions_cutscene.h"
#include "game/obj_behaviors.h"
#include "game/obj_behaviors_2.h"
#include "game/screen_transition.h"
#include "game/object_list_processor.h"
#include "game/behavior_actions.h"
#include "game/rendering_graph_node.h"
#include "game/skybox.h"
#include "menu/level_select_menu.h"
#include "menu/intro_geo.h"

#include "actors/common0.h"
#include "actors/common1.h"
#include "actors/group0.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group3.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group8.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group16.h"
#include "actors/group17.h"
#include "actors/custom0.h"
#include "actors/zcustom0.h"

#include "levels/bbh/header.h"
#include "levels/bitdw/header.h"
#include "levels/bitfs/header.h"
#include "levels/bits/header.h"
#include "levels/bob/header.h"
#include "levels/bowser_1/header.h"
#include "levels/wdw/header.h"
#include "levels/bowser_2/header.h"
#include "levels/bowser_2/header.h"
#include "levels/bowser_3/header.h"
#include "levels/bowser_3/header.h"
#include "levels/castle_courtyard/header.h"
#include "levels/castle_grounds/header.h"
#include "levels/castle_inside/header.h"
#include "levels/ccm/header.h"
#include "levels/cotmc/header.h"
#include "levels/ddd/header.h"
#include "levels/ending/header.h"
#include "levels/hmc/header.h"
#include "levels/jrb/header.h"
#include "levels/lll/header.h"
#include "levels/pss/header.h"
#include "levels/rr/header.h"
#include "levels/sa/header.h"
#include "levels/sl/header.h"
#include "levels/ssl/header.h"
#include "levels/thi/header.h"
#include "levels/totwc/header.h"
#include "levels/ttc/header.h"
#include "levels/ttm/header.h"
#include "levels/vcutm/header.h"
#include "levels/wdw/header.h"
#include "levels/wf/header.h"
#include "levels/wmotr/header.h"
#include "levels/menu/header.h"

#include "dynos_mgr_builtin_externs.h"
#include "textures.h"
}

#define define_builtin(_ptr) (const void*)#_ptr, (const void*)_ptr
#define define_builtin_ptr(_ptr) (const void*)#_ptr, (const void*)&_ptr

#define define_animation_builtin(_ptr) (const void*)#_ptr, (const void*)_ptr

#define MGR_FIND_DATA(_DataTable, _Cast)                               \
    size_t _count = sizeof(_DataTable) / (2 * sizeof(_DataTable[0]));  \
    for (u32 _i = 0; _i < _count; _i++) {                              \
        if (!strcmp((const char*)_DataTable[_i * 2 + 0], aDataName)) { \
            return _Cast _DataTable[_i * 2 + 1];                       \
        }                                                              \
    }                                                                  \
    return NULL;

#define MGR_FIND_DATA_FROM_TABLES(_DataTable, _DataTable2, _Cast)       \
    size_t _count = sizeof(_DataTable) / (2 * sizeof(_DataTable[0]));   \
    for (u32 _i = 0; _i < _count; _i++) {                               \
        if (!strcmp((const char*)_DataTable[_i * 2 + 0], aDataName)) {  \
            return _Cast _DataTable[_i * 2 + 1];                        \
        }                                                               \
    }                                                                   \
    _count = sizeof(_DataTable2) / (2 * sizeof(_DataTable2[0]));        \
    for (u32 _i = 0; _i < _count; _i++) {                               \
        if (!strcmp((const char*)_DataTable2[_i * 2 + 0], aDataName)) { \
            return _Cast _DataTable2[_i * 2 + 1];                       \
        }                                                               \
    }                                                                   \
    return NULL;

#define MGR_FIND_NAME(_DataTable)                                      \
    size_t _count = sizeof(_DataTable) / (2 * sizeof(_DataTable[0]));  \
    for (u32 _i = 0; _i < _count; _i++) {                              \
        if ((const void*)_DataTable[_i * 2 + 1] == aData) {            \
            return (const char*)_DataTable[_i * 2 + 0];                \
        }                                                              \
    }                                                                  \
    return NULL;

  /////////////////////
 // Script Pointers //
/////////////////////

static const void* sDynosBuiltinScriptPtrs[] = {
    define_builtin(level_main_scripts_entry),
    define_builtin(script_func_global_1),
    define_builtin(script_func_global_2),
    define_builtin(script_func_global_3),
    define_builtin(script_func_global_4),
    define_builtin(script_func_global_5),
    define_builtin(script_func_global_6),
    define_builtin(script_func_global_7),
    define_builtin(script_func_global_8),
    define_builtin(script_func_global_9),
    define_builtin(script_func_global_10),
    define_builtin(script_func_global_11),
    define_builtin(script_func_global_12),
    define_builtin(script_func_global_13),
    define_builtin(script_func_global_14),
    define_builtin(script_func_global_15),
    define_builtin(script_func_global_16),
    define_builtin(script_func_global_17),
    define_builtin(script_func_global_18),
    define_builtin(level_bbh_entry),
    define_builtin(level_bitdw_entry),
    define_builtin(level_bitfs_entry),
    define_builtin(level_bits_entry),
    define_builtin(level_bob_entry),
    define_builtin(level_bowser_1_entry),
    define_builtin(level_bowser_2_entry),
    define_builtin(level_bowser_3_entry),
    define_builtin(level_castle_courtyard_entry),
    define_builtin(level_castle_grounds_entry),
    define_builtin(level_castle_inside_entry),
    define_builtin(level_ccm_entry),
    define_builtin(level_cotmc_entry),
    define_builtin(level_ddd_entry),
    define_builtin(level_ending_entry),
    define_builtin(level_hmc_entry),
    define_builtin(level_jrb_entry),
    define_builtin(level_lll_entry),
    define_builtin(level_pss_entry),
    define_builtin(level_rr_entry),
    define_builtin(level_sa_entry),
    define_builtin(level_sl_entry),
    define_builtin(level_ssl_entry),
    define_builtin(level_thi_entry),
    define_builtin(level_totwc_entry),
    define_builtin(level_ttc_entry),
    define_builtin(level_ttm_entry),
    define_builtin(level_vcutm_entry),
    define_builtin(level_wdw_entry),
    define_builtin(level_wf_entry),
    define_builtin(level_wmotr_entry),
    define_builtin(level_main_menu_entry_1),
};

const void *gDynosLevelScriptsOriginal[] = {
    [LEVEL_NONE] = NULL,
#define STUB_LEVEL(_0, levelNum, _2, _3, _4, _5, _6, _7, _8) [levelNum] = NULL,
#define DEFINE_LEVEL(_0, levelNum, _2, folder, _4, _5, _6, _7, _8, _9, _10) \
    [levelNum] = level_ ## folder ## _entry,
#include "levels/level_defines.h"
#undef STUB_LEVEL
#undef DEFINE_LEVEL
};

const void* DynOS_Builtin_ScriptPtr_GetFromName(const char* aDataName) {
    MGR_FIND_DATA(sDynosBuiltinScriptPtrs, (const void*));
}

const char* DynOS_Builtin_ScriptPtr_GetFromData(const void* aData) {
    MGR_FIND_NAME(sDynosBuiltinScriptPtrs);
}

  //////////////////
 // Level Macros //
//////////////////

static const void* sDynosBuiltinLvlMacros[] = {
    define_builtin(bbh_seg7_macro_objs),
    define_builtin(bitdw_seg7_macro_objs),
    define_builtin(bitfs_seg7_macro_objs),
    define_builtin(bits_seg7_macro_objs),
    define_builtin(bob_seg7_macro_objs),
    define_builtin(castle_courtyard_seg7_macro_objs),
    define_builtin(castle_grounds_seg7_macro_objs),
    define_builtin(ccm_seg7_area_1_macro_objs),
    define_builtin(ccm_seg7_area_2_macro_objs),
    define_builtin(cotmc_seg7_macro_objs),
    define_builtin(ddd_seg7_area_1_macro_objs),
    define_builtin(ddd_seg7_area_2_macro_objs),
    define_builtin(hmc_seg7_macro_objs),
    define_builtin(jrb_seg7_area_1_macro_objs),
    define_builtin(jrb_seg7_area_2_macro_objs),
    define_builtin(lll_seg7_area_1_macro_objs),
    define_builtin(lll_seg7_area_2_macro_objs),
    define_builtin(pss_seg7_macro_objs),
    define_builtin(rr_seg7_macro_objs),
    define_builtin(sa_seg7_macro_objs),
    define_builtin(sl_seg7_area_1_macro_objs),
    define_builtin(sl_seg7_area_2_macro_objs),
    define_builtin(ssl_seg7_area_1_macro_objs),
    define_builtin(ssl_seg7_area_2_macro_objs),
    define_builtin(ssl_seg7_area_3_macro_objs),
    define_builtin(thi_seg7_area_1_macro_objs),
    define_builtin(thi_seg7_area_2_macro_objs),
    define_builtin(thi_seg7_area_3_macro_objs),
    define_builtin(totwc_seg7_macro_objs),
    define_builtin(ttc_seg7_macro_objs),
    define_builtin(ttm_seg7_area_1_macro_objs),
    define_builtin(ttm_seg7_area_2_macro_objs),
    define_builtin(vcutm_seg7_macro_objs),
    define_builtin(wdw_seg7_area_1_macro_objs),
    define_builtin(wdw_seg7_area_2_macro_objs),
    define_builtin(wf_seg7_macro_objs),
    define_builtin(wmotr_seg7_macro_objs),
};

const MacroObject* DynOS_Builtin_LvlMacro_GetFromName(const char* aDataName) {
    MGR_FIND_DATA(sDynosBuiltinLvlMacros, (const MacroObject*));
}

const char* DynOS_Builtin_LvlMacro_GetFromData(const MacroObject* aData) {
    MGR_FIND_NAME(sDynosBuiltinLvlMacros);
}

  ////////////////
 // Collisions //
////////////////

static const void* sDynosBuiltinCols[] = {
    // Level Collisions
    define_builtin(bbh_seg7_collision_07026B1C),
    define_builtin(bbh_seg7_collision_coffin),
    define_builtin(bbh_seg7_collision_haunted_bookshelf),
    define_builtin(bbh_seg7_collision_level),
    define_builtin(bbh_seg7_collision_merry_go_round),
    define_builtin(bbh_seg7_collision_mesh_elevator),
    define_builtin(bbh_seg7_collision_staircase_step),
    define_builtin(bbh_seg7_collision_tilt_floor_platform),
    define_builtin(bitdw_seg7_collision_0700F688),
    define_builtin(bitdw_seg7_collision_0700F70C),
    define_builtin(bitdw_seg7_collision_0700F7F0),
    define_builtin(bitdw_seg7_collision_0700F898),
    define_builtin(bitdw_seg7_collision_0700F91C),
    define_builtin(bitdw_seg7_collision_0700FA3C),
    define_builtin(bitdw_seg7_collision_0700FB5C),
    define_builtin(bitdw_seg7_collision_0700FC7C),
    define_builtin(bitdw_seg7_collision_0700FD9C),
    define_builtin(bitdw_seg7_collision_level),
    define_builtin(bitdw_seg7_collision_moving_pyramid),
    define_builtin(bitfs_seg7_collision_07015124),
    define_builtin(bitfs_seg7_collision_07015288),
    define_builtin(bitfs_seg7_collision_07015714),
    define_builtin(bitfs_seg7_collision_07015768),
    define_builtin(bitfs_seg7_collision_070157E0),
    define_builtin(bitfs_seg7_collision_07015928),
    define_builtin(bitfs_seg7_collision_inverted_pyramid),
    define_builtin(bitfs_seg7_collision_level),
    define_builtin(bitfs_seg7_collision_sinking_cage_platform),
    define_builtin(bitfs_seg7_collision_sinking_platform),
    define_builtin(bitfs_seg7_collision_squishable_platform),
    define_builtin(bits_seg7_collision_0701A9A0),
    define_builtin(bits_seg7_collision_0701AA0C),
    define_builtin(bits_seg7_collision_0701AA84),
    define_builtin(bits_seg7_collision_0701AC28),
    define_builtin(bits_seg7_collision_0701ACAC),
    define_builtin(bits_seg7_collision_0701AD54),
    define_builtin(bits_seg7_collision_0701ADD8),
    define_builtin(bits_seg7_collision_0701AE5C),
    define_builtin(bits_seg7_collision_0701B0D4),
    define_builtin(bits_seg7_collision_0701B26C),
    define_builtin(bits_seg7_collision_0701B404),
    define_builtin(bits_seg7_collision_0701B59C),
    define_builtin(bits_seg7_collision_0701B734),
    define_builtin(bits_seg7_collision_level),
    define_builtin(bob_seg7_collision_bridge),
    define_builtin(bob_seg7_collision_chain_chomp_gate),
    define_builtin(bob_seg7_collision_gate),
    define_builtin(bob_seg7_collision_level),
    define_builtin(bowser_1_seg7_collision_level),
    define_builtin(bowser_2_seg7_collision_lava),
    define_builtin(bowser_2_seg7_collision_tilting_platform),
    define_builtin(bowser_3_seg7_collision_07004B94),
    define_builtin(bowser_3_seg7_collision_07004C18),
    define_builtin(bowser_3_seg7_collision_07004C9C),
    define_builtin(bowser_3_seg7_collision_07004D20),
    define_builtin(bowser_3_seg7_collision_07004DA4),
    define_builtin(bowser_3_seg7_collision_07004E28),
    define_builtin(bowser_3_seg7_collision_07004EAC),
    define_builtin(bowser_3_seg7_collision_07004F30),
    define_builtin(bowser_3_seg7_collision_07004FB4),
    define_builtin(bowser_3_seg7_collision_07005038),
    define_builtin(bowser_3_seg7_collision_level),
    define_builtin(castle_courtyard_seg7_collision),
    define_builtin(castle_grounds_seg7_collision_cannon_grill),
    define_builtin(castle_grounds_seg7_collision_level),
    define_builtin(castle_grounds_seg7_collision_moat_grills),
    define_builtin(ccm_seg7_area_1_collision),
    define_builtin(ccm_seg7_area_2_collision),
    define_builtin(ccm_seg7_collision_070163F8),
    define_builtin(cotmc_seg7_collision_level),
    define_builtin(ddd_seg7_area_1_collision),
    define_builtin(ddd_seg7_area_2_collision),
    define_builtin(ddd_seg7_collision_bowser_sub_door),
    define_builtin(ddd_seg7_collision_submarine),
    define_builtin(hmc_seg7_collision_0702B65C),
    define_builtin(hmc_seg7_collision_controllable_platform_sub),
    define_builtin(hmc_seg7_collision_controllable_platform),
    define_builtin(hmc_seg7_collision_elevator),
    define_builtin(hmc_seg7_collision_level),
    define_builtin(inside_castle_seg7_area_1_collision),
    define_builtin(inside_castle_seg7_area_2_collision),
    define_builtin(inside_castle_seg7_area_3_collision),
    define_builtin(inside_castle_seg7_collision_ddd_warp_2),
    define_builtin(inside_castle_seg7_collision_ddd_warp),
    define_builtin(inside_castle_seg7_collision_floor_trap),
    define_builtin(inside_castle_seg7_collision_star_door),
    define_builtin(inside_castle_seg7_collision_water_level_pillar),
    define_builtin(jrb_seg7_area_1_collision),
    define_builtin(jrb_seg7_area_2_collision),
    define_builtin(jrb_seg7_collision_0700CEF0),
    define_builtin(jrb_seg7_collision_0700D1DC),
    define_builtin(jrb_seg7_collision_floating_box),
    define_builtin(jrb_seg7_collision_floating_platform),
    define_builtin(jrb_seg7_collision_in_sunken_ship_2),
    define_builtin(jrb_seg7_collision_in_sunken_ship_3),
    define_builtin(jrb_seg7_collision_in_sunken_ship),
    define_builtin(jrb_seg7_collision_pillar_base),
    define_builtin(jrb_seg7_collision_rock_solid),
    define_builtin(lll_seg7_area_1_collision),
    define_builtin(lll_seg7_area_2_collision),
    define_builtin(lll_seg7_collision_0701D21C),
    define_builtin(lll_seg7_collision_drawbridge),
    define_builtin(lll_seg7_collision_falling_wall),
    define_builtin(lll_seg7_collision_floating_block),
    define_builtin(lll_seg7_collision_hexagonal_platform),
    define_builtin(lll_seg7_collision_inverted_pyramid),
    define_builtin(lll_seg7_collision_octagonal_moving_platform),
    define_builtin(lll_seg7_collision_pitoune),
    define_builtin(lll_seg7_collision_puzzle_piece),
    define_builtin(lll_seg7_collision_rotating_fire_bars),
    define_builtin(lll_seg7_collision_rotating_platform),
    define_builtin(lll_seg7_collision_sinking_pyramids),
    define_builtin(lll_seg7_collision_slow_tilting_platform),
    define_builtin(lll_seg7_collision_wood_piece),
    define_builtin(pss_seg7_collision),
    define_builtin(rr_seg7_collision_07029038),
    define_builtin(rr_seg7_collision_07029508),
    define_builtin(rr_seg7_collision_070295F8),
    define_builtin(rr_seg7_collision_0702967C),
    define_builtin(rr_seg7_collision_07029750),
    define_builtin(rr_seg7_collision_07029858),
    define_builtin(rr_seg7_collision_07029924),
    define_builtin(rr_seg7_collision_07029C1C),
    define_builtin(rr_seg7_collision_07029FA4),
    define_builtin(rr_seg7_collision_0702A32C),
    define_builtin(rr_seg7_collision_0702A6B4),
    define_builtin(rr_seg7_collision_donut_platform),
    define_builtin(rr_seg7_collision_elevator_platform),
    define_builtin(rr_seg7_collision_level),
    define_builtin(rr_seg7_collision_pendulum),
    define_builtin(rr_seg7_collision_rotating_platform_with_fire),
    define_builtin(sa_seg7_collision),
    define_builtin(sl_seg7_area_1_collision),
    define_builtin(sl_seg7_area_2_collision),
    define_builtin(sl_seg7_collision_pound_explodes),
    define_builtin(sl_seg7_collision_sliding_snow_mound),
    define_builtin(ssl_seg7_area_1_collision),
    define_builtin(ssl_seg7_area_2_collision),
    define_builtin(ssl_seg7_area_3_collision),
    define_builtin(ssl_seg7_collision_0702808C),
    define_builtin(ssl_seg7_collision_07028274),
    define_builtin(ssl_seg7_collision_070282F8),
    define_builtin(ssl_seg7_collision_07028370),
    define_builtin(ssl_seg7_collision_070284B0),
    define_builtin(ssl_seg7_collision_grindel),
    define_builtin(ssl_seg7_collision_pyramid_elevator),
    define_builtin(ssl_seg7_collision_pyramid_top),
    define_builtin(ssl_seg7_collision_spindel),
    define_builtin(ssl_seg7_collision_tox_box),
    define_builtin(thi_seg7_area_1_collision),
    define_builtin(thi_seg7_area_2_collision),
    define_builtin(thi_seg7_area_3_collision),
    define_builtin(thi_seg7_collision_top_trap),
    define_builtin(totwc_seg7_collision),
    define_builtin(ttc_seg7_collision_07014F70),
    define_builtin(ttc_seg7_collision_07015008),
    define_builtin(ttc_seg7_collision_070152B4),
    define_builtin(ttc_seg7_collision_070153E0),
    define_builtin(ttc_seg7_collision_07015584),
    define_builtin(ttc_seg7_collision_07015650),
    define_builtin(ttc_seg7_collision_07015754),
    define_builtin(ttc_seg7_collision_070157D8),
    define_builtin(ttc_seg7_collision_clock_main_rotation),
    define_builtin(ttc_seg7_collision_clock_pendulum),
    define_builtin(ttc_seg7_collision_clock_platform),
    define_builtin(ttc_seg7_collision_level),
    define_builtin(ttc_seg7_collision_rotating_clock_platform2),
    define_builtin(ttc_seg7_collision_sliding_surface),
    define_builtin(ttm_seg7_area_1_collision),
    define_builtin(ttm_seg7_area_2_collision),
    define_builtin(ttm_seg7_area_3_collision),
    define_builtin(ttm_seg7_area_4_collision),
    define_builtin(ttm_seg7_collision_pitoune_2),
    define_builtin(ttm_seg7_collision_podium_warp),
    define_builtin(ttm_seg7_collision_ukiki_cage),
    define_builtin(vcutm_seg7_collision_0700AC44),
    define_builtin(vcutm_seg7_collision),
    define_builtin(wdw_seg7_area_1_collision),
    define_builtin(wdw_seg7_area_2_collision),
    define_builtin(wdw_seg7_collision_070184C8),
    define_builtin(wdw_seg7_collision_07018528),
    define_builtin(wdw_seg7_collision_070186B4),
    define_builtin(wdw_seg7_collision_arrow_lift),
    define_builtin(wdw_seg7_collision_express_elevator_platform),
    define_builtin(wdw_seg7_collision_rect_floating_platform),
    define_builtin(wdw_seg7_collision_square_floating_platform),
    define_builtin(wf_seg7_collision_070102D8),
    define_builtin(wf_seg7_collision_breakable_wall_2),
    define_builtin(wf_seg7_collision_breakable_wall),
    define_builtin(wf_seg7_collision_bullet_bill_cannon),
    define_builtin(wf_seg7_collision_clocklike_rotation),
    define_builtin(wf_seg7_collision_kickable_board),
    define_builtin(wf_seg7_collision_large_bomp),
    define_builtin(wf_seg7_collision_platform),
    define_builtin(wf_seg7_collision_rotating_platform),
    define_builtin(wf_seg7_collision_sliding_brick_platform),
    define_builtin(wf_seg7_collision_small_bomp),
    define_builtin(wf_seg7_collision_tower_door),
    define_builtin(wf_seg7_collision_tower),
    define_builtin(wf_seg7_collision_trapezoid),
    define_builtin(wf_seg7_collision_tumbling_bridge),
    define_builtin(wmotr_seg7_collision),

    // Actor Collisions
    define_builtin(bbh_seg7_collision_coffin),
    define_builtin(bbh_seg7_collision_haunted_bookshelf),
    define_builtin(bbh_seg7_collision_merry_go_round),
    define_builtin(bbh_seg7_collision_mesh_elevator),
    define_builtin(bbh_seg7_collision_staircase_step),
    define_builtin(bbh_seg7_collision_tilt_floor_platform),
    define_builtin(bitdw_seg7_collision_moving_pyramid),
    define_builtin(bitfs_seg7_collision_inverted_pyramid),
    define_builtin(bitfs_seg7_collision_sinking_cage_platform),
    define_builtin(bitfs_seg7_collision_sinking_platform),
    define_builtin(bitfs_seg7_collision_squishable_platform),
    define_builtin(blue_coin_switch_seg8_collision_08000E98),
    define_builtin(bob_seg7_collision_chain_chomp_gate),
    define_builtin(bowser_2_seg7_collision_tilting_platform),
    define_builtin(breakable_box_seg8_collision_08012D70),
    define_builtin(cannon_lid_seg8_collision_08004950),
    define_builtin(capswitch_collision_050033D0),
    define_builtin(capswitch_collision_05003448),
    define_builtin(castle_grounds_seg7_collision_cannon_grill),
    define_builtin(castle_grounds_seg7_collision_moat_grills),
    define_builtin(checkerboard_platform_seg8_collision_0800D710),
    define_builtin(ddd_seg7_collision_bowser_sub_door),
    define_builtin(ddd_seg7_collision_submarine),
    define_builtin(door_seg3_collision_0301CE78),
    define_builtin(dorrie_seg6_collision_0600F644),
    define_builtin(dorrie_seg6_collision_0600FBB8),
    define_builtin(exclamation_box_outline_seg8_collision_08025F78),
    define_builtin(hmc_seg7_collision_controllable_platform),
    define_builtin(hmc_seg7_collision_controllable_platform_sub),
    define_builtin(hmc_seg7_collision_elevator),
    define_builtin(inside_castle_seg7_collision_floor_trap),
    define_builtin(inside_castle_seg7_collision_star_door),
    define_builtin(inside_castle_seg7_collision_water_level_pillar),
    define_builtin(jrb_seg7_collision_floating_box),
    define_builtin(jrb_seg7_collision_floating_platform),
    define_builtin(jrb_seg7_collision_in_sunken_ship),
    define_builtin(jrb_seg7_collision_in_sunken_ship_2),
    define_builtin(jrb_seg7_collision_in_sunken_ship_3),
    define_builtin(jrb_seg7_collision_pillar_base),
    define_builtin(jrb_seg7_collision_rock_solid),
    define_builtin(lll_hexagonal_mesh_seg3_collision_0301CECC),
    define_builtin(lll_seg7_collision_drawbridge),
    define_builtin(lll_seg7_collision_falling_wall),
    define_builtin(lll_seg7_collision_floating_block),
    define_builtin(lll_seg7_collision_hexagonal_platform),
    define_builtin(lll_seg7_collision_inverted_pyramid),
    define_builtin(lll_seg7_collision_octagonal_moving_platform),
    define_builtin(lll_seg7_collision_pitoune),
    define_builtin(lll_seg7_collision_puzzle_piece),
    define_builtin(lll_seg7_collision_rotating_fire_bars),
    define_builtin(lll_seg7_collision_rotating_platform),
    define_builtin(lll_seg7_collision_sinking_pyramids),
    define_builtin(lll_seg7_collision_slow_tilting_platform),
    define_builtin(lll_seg7_collision_wood_piece),
    define_builtin(metal_box_seg8_collision_08024C28),
    define_builtin(penguin_seg5_collision_05008B88),
    define_builtin(poundable_pole_collision_06002490),
    define_builtin(purple_switch_seg8_collision_0800C7A8),
    define_builtin(rr_seg7_collision_donut_platform),
    define_builtin(rr_seg7_collision_elevator_platform),
    define_builtin(rr_seg7_collision_pendulum),
    define_builtin(rr_seg7_collision_rotating_platform_with_fire),
    define_builtin(sl_seg7_collision_pound_explodes),
    define_builtin(sl_seg7_collision_sliding_snow_mound),
    define_builtin(springboard_collision_05001A28),
    define_builtin(ssl_seg7_collision_0702808C),
    define_builtin(ssl_seg7_collision_grindel),
    define_builtin(ssl_seg7_collision_pyramid_elevator),
    define_builtin(ssl_seg7_collision_pyramid_top),
    define_builtin(ssl_seg7_collision_spindel),
    define_builtin(ssl_seg7_collision_tox_box),
    define_builtin(thi_seg7_collision_top_trap),
    define_builtin(thwomp_seg5_collision_0500B7D0),
    define_builtin(thwomp_seg5_collision_0500B92C),
    define_builtin(ttc_seg7_collision_clock_main_rotation),
    define_builtin(ttc_seg7_collision_clock_pendulum),
    define_builtin(ttc_seg7_collision_clock_platform),
    define_builtin(ttc_seg7_collision_rotating_clock_platform2),
    define_builtin(ttc_seg7_collision_sliding_surface),
    define_builtin(ttm_seg7_collision_pitoune_2),
    define_builtin(ttm_seg7_collision_podium_warp),
    define_builtin(ttm_seg7_collision_ukiki_cage),
    define_builtin(unknown_seg8_collision_080262F8),
    define_builtin(warp_pipe_seg3_collision_03009AC8),
    define_builtin(wdw_seg7_collision_arrow_lift),
    define_builtin(wdw_seg7_collision_express_elevator_platform),
    define_builtin(wdw_seg7_collision_rect_floating_platform),
    define_builtin(wdw_seg7_collision_square_floating_platform),
    define_builtin(wf_seg7_collision_breakable_wall),
    define_builtin(wf_seg7_collision_breakable_wall_2),
    define_builtin(wf_seg7_collision_bullet_bill_cannon),
    define_builtin(wf_seg7_collision_clocklike_rotation),
    define_builtin(wf_seg7_collision_kickable_board),
    define_builtin(wf_seg7_collision_large_bomp),
    define_builtin(wf_seg7_collision_platform),
    define_builtin(wf_seg7_collision_sliding_brick_platform),
    define_builtin(wf_seg7_collision_small_bomp),
    define_builtin(wf_seg7_collision_tower),
    define_builtin(wf_seg7_collision_tower_door),
    define_builtin(whomp_seg6_collision_06020A0C),
    define_builtin(wooden_signpost_seg3_collision_0302DD80),
};

const Collision* DynOS_Builtin_Col_GetFromName(const char* aDataName) {
    MGR_FIND_DATA(sDynosBuiltinCols, (const Collision*));
}

const char* DynOS_Builtin_Col_GetFromData(const Collision* aData) {
    MGR_FIND_NAME(sDynosBuiltinCols);
}

  ////////////////
 // Animations //
////////////////

static const void* sDynosBuiltinAnims[] = {
    define_builtin_ptr(amp_seg8_anims_08004034),
    define_builtin_ptr(bobomb_seg8_anims_0802396C),
    define_builtin_ptr(chuckya_seg8_anims_0800C070),
    define_builtin_ptr(flyguy_seg8_anims_08011A64),
    define_builtin_ptr(goomba_seg8_anims_0801DA4C),
    define_builtin_ptr(blue_fish_seg3_anims_0301C2B0),
    define_builtin_ptr(bowser_key_seg3_anims_list),
    define_builtin_ptr(butterfly_seg3_anims_030056B0),
    define_builtin_ptr(door_seg3_anims_030156C0),
    define_builtin_ptr(heave_ho_seg5_anims_0501534C),
    define_builtin_ptr(hoot_seg5_anims_05005768),
    define_builtin_ptr(blargg_seg5_anims_0500616C),
    define_builtin_ptr(bully_seg5_anims_0500470C),
    define_builtin_ptr(king_bobomb_seg5_anims_0500FE30),
    define_builtin_ptr(clam_shell_seg5_anims_05001744),
    define_builtin_ptr(manta_seg5_anims_05008EB4),
    define_builtin_ptr(sushi_seg5_anims_0500AE54),
    define_builtin_ptr(unagi_seg5_anims_05012824),
    define_builtin_ptr(eyerok_seg5_anims_050116E4),
    define_builtin_ptr(klepto_seg5_anims_05008CFC),
    define_builtin_ptr(monty_mole_seg5_anims_05007248),
    define_builtin_ptr(ukiki_seg5_anims_05015784),
    define_builtin_ptr(penguin_seg5_anims_05008B74),
    define_builtin_ptr(snowman_seg5_anims_0500D118),
    define_builtin_ptr(spindrift_seg5_anims_05002D68),
    define_builtin_ptr(bookend_seg5_anims_05002540),
    define_builtin_ptr(chair_seg5_anims_05005784),
    define_builtin_ptr(mad_piano_seg5_anims_05009B14),
    define_builtin_ptr(birds_seg5_anims_050009E8),
    define_builtin_ptr(peach_seg5_anims_0501C41C),
    define_builtin_ptr(yoshi_seg5_anims_05024100),
    define_builtin_ptr(lakitu_enemy_seg5_anims_050144D4),
    define_builtin_ptr(spiny_seg5_anims_05016EAC),
    define_builtin_ptr(spiny_egg_seg5_anims_050157E4),
    define_builtin_ptr(wiggler_seg5_anims_0500C874),
    define_builtin_ptr(wiggler_seg5_anims_0500EC8C),
    define_builtin_ptr(bowser_seg6_anims_06057690),
    define_builtin_ptr(bub_seg6_anims_06012354),
    define_builtin_ptr(cyan_fish_seg6_anims_0600E264),
    define_builtin_ptr(seaweed_seg6_anims_0600A4D4),
    define_builtin_ptr(skeeter_seg6_anims_06007DE0),
    define_builtin_ptr(water_ring_seg6_anims_06013F7C),
    define_builtin_ptr(chain_chomp_seg6_anims_06025178),
    define_builtin_ptr(koopa_seg6_anims_06011364),
    define_builtin_ptr(koopa_flag_seg6_anims_06001028),
    define_builtin_ptr(piranha_plant_seg6_anims_0601C31C),
    define_builtin_ptr(whomp_seg6_anims_06020A04),
    define_builtin_ptr(lakitu_seg6_anims_060058F8),
    define_builtin_ptr(mips_seg6_anims_06015634),
    define_builtin_ptr(toad_seg6_anims_0600FB58),
    define_builtin_ptr(chilly_chief_seg6_anims_06003994),
    define_builtin_ptr(moneybag_seg6_anims_06005E5C),
    define_builtin_ptr(dorrie_seg6_anims_0600F638),
    define_builtin_ptr(scuttlebug_seg6_anims_06015064),
    define_builtin_ptr(swoop_seg6_anims_060070D0),
    define_builtin_ptr(castle_grounds_seg7_anims_flags),
};

const Animation *DynOS_Builtin_Anim_GetFromName(const char *aDataName) {
    MGR_FIND_DATA(sDynosBuiltinAnims, (const Animation *));
}

const char *DynOS_Builtin_Anim_GetFromData(const Animation *aData) {
    MGR_FIND_NAME(sDynosBuiltinAnims);
}

  ////////////////////
 // Functions Ptrs //
////////////////////

static void *geo_rotate_3d_coin(s32 callContext, void *node, UNUSED void *c) {
    if (callContext == GEO_CONTEXT_RENDER) {
        struct Object *obj = (struct Object *) gCurGraphNodeObject;
        struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) ((struct GraphNode *) node)->next;
        rotNode->rotation[0] = 0;
        rotNode->rotation[1] = obj->oAnimState * 0x0800;
        rotNode->rotation[2] = 0;
        obj->oFaceAnglePitch = 0;
        obj->oMoveAnglePitch = 0;
        obj->oFaceAngleRoll = 0;
        obj->oMoveAngleRoll = 0;
        obj->header.gfx.node.flags &= ~(GRAPH_RENDER_BILLBOARD | GRAPH_RENDER_CYLBOARD);
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   //
//   !! IMPORTANT: ADD NEW ENTRIES AT THE END OF THE sDynosBuiltinFuncs LIST TO PRESERVE INDEXING !!   //
//   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    const char *name;
    const void *func;
    u8 type;
} DynosBuiltinFunction;

#define define_builtin_function(_func, _type) { .name = #_func, .func = (const void *) _func, .type = _type }

static const DynosBuiltinFunction sDynosBuiltinFuncs[] = {
    define_builtin_function(geo_mirror_mario_set_alpha, FUNCTION_GEO),
    define_builtin_function(geo_switch_mario_stand_run, FUNCTION_GEO),
    define_builtin_function(geo_switch_mario_eyes, FUNCTION_GEO),
    define_builtin_function(geo_mario_tilt_torso, FUNCTION_GEO),
    define_builtin_function(geo_mario_head_rotation, FUNCTION_GEO),
    define_builtin_function(geo_switch_mario_hand, FUNCTION_GEO),
    define_builtin_function(geo_mario_hand_foot_scaler, FUNCTION_GEO),
    define_builtin_function(geo_switch_mario_cap_effect, FUNCTION_GEO),
    define_builtin_function(geo_switch_mario_cap_on_off, FUNCTION_GEO),
    define_builtin_function(geo_mario_rotate_wing_cap_wings, FUNCTION_GEO),
    define_builtin_function(geo_switch_mario_hand_grab_pos, FUNCTION_GEO),
    define_builtin_function(geo_render_mirror_mario, FUNCTION_GEO),
    define_builtin_function(geo_mirror_mario_backface_culling, FUNCTION_GEO),
    define_builtin_function(geo_update_projectile_pos_from_parent, FUNCTION_GEO),
    define_builtin_function(geo_update_layer_transparency, FUNCTION_GEO),
    define_builtin_function(geo_switch_anim_state, FUNCTION_GEO),
    define_builtin_function(geo_switch_area, FUNCTION_GEO),
    define_builtin_function(geo_camera_main, FUNCTION_GEO),
    define_builtin_function(geo_camera_fov, FUNCTION_GEO),
    define_builtin_function(geo_envfx_main, FUNCTION_GEO),
    define_builtin_function(geo_skybox_main, FUNCTION_GEO),
    define_builtin_function(geo_wdw_set_initial_water_level, FUNCTION_GEO),
    define_builtin_function(geo_movtex_pause_control, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_water_regions, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_nocolor, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_colored, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_colored_no_update, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_colored_2_no_update, FUNCTION_GEO),
    define_builtin_function(geo_movtex_update_horizontal, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_colored_no_update, FUNCTION_GEO),
    define_builtin_function(geo_painting_draw, FUNCTION_GEO),
    define_builtin_function(geo_painting_update, FUNCTION_GEO),
    define_builtin_function(geo_exec_inside_castle_light, FUNCTION_GEO),
    define_builtin_function(geo_exec_flying_carpet_timer_update, FUNCTION_GEO),
    define_builtin_function(geo_exec_flying_carpet_create, FUNCTION_GEO),
    define_builtin_function(geo_exec_cake_end_screen, FUNCTION_GEO),
    define_builtin_function(geo_cannon_circle_base, FUNCTION_GEO),
    define_builtin_function(geo_move_mario_part_from_parent, FUNCTION_GEO),
    define_builtin_function(geo_bits_bowser_coloring, FUNCTION_GEO),
    define_builtin_function(geo_update_body_rot_from_parent, FUNCTION_GEO),
    define_builtin_function(geo_switch_bowser_eyes, FUNCTION_GEO),
    define_builtin_function(geo_switch_tuxie_mother_eyes, FUNCTION_GEO),
    define_builtin_function(geo_update_held_mario_pos, FUNCTION_GEO),
    define_builtin_function(geo_snufit_move_mask, FUNCTION_GEO),
    define_builtin_function(geo_snufit_scale_body, FUNCTION_GEO),
    define_builtin_function(geo_scale_bowser_key, FUNCTION_GEO),
    { .name = "geo_rotate_coin", .func = (const void *) geo_rotate_3d_coin, .type = FUNCTION_GEO },
    define_builtin_function(geo_offset_klepto_held_object, FUNCTION_GEO),
    define_builtin_function(geo_switch_peach_eyes, FUNCTION_GEO),

    // Co-op specific
    define_builtin_function(geo_mario_set_player_colors, FUNCTION_GEO),
    define_builtin_function(geo_movtex_draw_water_regions_ext, FUNCTION_GEO),
    define_builtin_function(lvl_init_or_update, FUNCTION_LVL),
    define_builtin_function(geo_choose_area_ext, FUNCTION_GEO),

    // Behaviors
    define_builtin_function(bhv_cap_switch_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tiny_star_particles_init, FUNCTION_BHV),
    define_builtin_function(bhv_grindel_thwomp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_koopa_shell_underwater_loop, FUNCTION_BHV),
    define_builtin_function(bhv_door_init, FUNCTION_BHV),
    define_builtin_function(bhv_door_loop, FUNCTION_BHV),
    define_builtin_function(bhv_star_door_loop, FUNCTION_BHV),
    { .name = "bhv_star_door_loop_2", .func = (const void *) bhv_star_door_loop_update_render_state, .type = FUNCTION_BHV },
    define_builtin_function(bhv_star_door_loop_update_render_state, FUNCTION_BHV),
    define_builtin_function(bhv_mr_i_loop, FUNCTION_BHV),
    define_builtin_function(bhv_mr_i_body_loop, FUNCTION_BHV),
    define_builtin_function(bhv_mr_i_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_piranha_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_giant_pole_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pole_init, FUNCTION_BHV),
    define_builtin_function(bhv_pole_base_loop, FUNCTION_BHV),
    define_builtin_function(bhv_thi_huge_island_top_loop, FUNCTION_BHV),
    define_builtin_function(bhv_thi_tiny_island_top_loop, FUNCTION_BHV),
    define_builtin_function(bhv_king_bobomb_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_anchor_mario_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_chest_bottom_init, FUNCTION_BHV),
    define_builtin_function(bhv_beta_chest_bottom_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_chest_lid_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bubble_wave_init, FUNCTION_BHV),
    define_builtin_function(bhv_bubble_maybe_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bubble_player_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_air_bubble_init, FUNCTION_BHV),
    define_builtin_function(bhv_water_air_bubble_loop, FUNCTION_BHV),
    define_builtin_function(bhv_particle_init, FUNCTION_BHV),
    define_builtin_function(bhv_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_waves_init, FUNCTION_BHV),
    define_builtin_function(bhv_small_bubbles_loop, FUNCTION_BHV),
    define_builtin_function(bhv_fish_group_loop, FUNCTION_BHV),
    define_builtin_function(bhv_cannon_base_loop, FUNCTION_BHV),
    define_builtin_function(bhv_cannon_barrel_loop, FUNCTION_BHV),
    define_builtin_function(bhv_cannon_base_unused_loop, FUNCTION_BHV),
    define_builtin_function(bhv_chuckya_loop, FUNCTION_BHV),
    define_builtin_function(bhv_chuckya_anchor_mario_loop, FUNCTION_BHV),
    define_builtin_function(bhv_rotating_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wf_breakable_wall_loop, FUNCTION_BHV),
    define_builtin_function(bhv_kickable_board_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tower_door_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wf_rotating_wooden_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_wf_rotating_wooden_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_fading_warp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_warp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_white_puff_exploding_loop, FUNCTION_BHV),
    define_builtin_function(bhv_spawned_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_spawned_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_coin_init, FUNCTION_BHV),
    define_builtin_function(bhv_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_coin_inside_boo_loop, FUNCTION_BHV),
    define_builtin_function(bhv_coin_formation_init, FUNCTION_BHV),
    define_builtin_function(bhv_coin_formation_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_coin_formation_loop, FUNCTION_BHV),
    define_builtin_function(bhv_temp_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_coin_sparkles_loop, FUNCTION_BHV),
    define_builtin_function(bhv_golden_coin_sparkles_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wall_tiny_star_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pound_tiny_star_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pound_tiny_star_particle_init, FUNCTION_BHV),
    define_builtin_function(bhv_punch_tiny_triangle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_punch_tiny_triangle_init, FUNCTION_BHV),
    define_builtin_function(bhv_tumbling_bridge_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tumbling_bridge_loop, FUNCTION_BHV),
    define_builtin_function(bhv_elevator_init, FUNCTION_BHV),
    define_builtin_function(bhv_elevator_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_mist_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_mist_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_mist_2_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pound_white_puffs_init, FUNCTION_BHV),
    define_builtin_function(bhv_ground_sand_init, FUNCTION_BHV),
    define_builtin_function(bhv_ground_snow_init, FUNCTION_BHV),
    define_builtin_function(bhv_wind_loop, FUNCTION_BHV),
    define_builtin_function(bhv_unused_particle_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ukiki_cage_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ukiki_cage_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bitfs_sinking_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bitfs_sinking_cage_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ddd_moving_pole_loop, FUNCTION_BHV),
    define_builtin_function(bhv_platform_normals_init, FUNCTION_BHV),
    define_builtin_function(bhv_tilting_inverted_pyramid_loop, FUNCTION_BHV),
    define_builtin_function(bhv_squishable_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_moving_flames_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_moving_flames_loop, FUNCTION_BHV),
    define_builtin_function(bhv_rr_rotating_bridge_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flamethrower_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flamethrower_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bouncing_fireball_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bouncing_fireball_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_shock_wave_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flame_mario_loop, FUNCTION_BHV),
    define_builtin_function(bhv_black_smoke_mario_loop, FUNCTION_BHV),
    define_builtin_function(bhv_black_smoke_bowser_loop, FUNCTION_BHV),
    define_builtin_function(bhv_black_smoke_upward_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_fish_splash_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_spindrift_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tower_platform_group_init, FUNCTION_BHV),
    define_builtin_function(bhv_tower_platform_group_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wf_sliding_tower_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wf_elevator_tower_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wf_solid_tower_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_snow_leaf_particle_spawn_init, FUNCTION_BHV),
    define_builtin_function(bhv_tree_snow_or_leaf_loop, FUNCTION_BHV),
    define_builtin_function(bhv_piranha_plant_bubble_loop, FUNCTION_BHV),
    define_builtin_function(bhv_piranha_plant_waking_bubbles_loop, FUNCTION_BHV),
    define_builtin_function(bhv_purple_switch_loop, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_object_loop, FUNCTION_BHV),
    define_builtin_function(bhv_breakable_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pushable_loop, FUNCTION_BHV),
    define_builtin_function(bhv_init_room, FUNCTION_BHV),
    define_builtin_function(bhv_small_water_wave_loop, FUNCTION_BHV),
    define_builtin_function(bhv_yellow_coin_init, FUNCTION_BHV),
    define_builtin_function(bhv_yellow_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_squarish_path_moving_loop, FUNCTION_BHV),
    define_builtin_function(bhv_squarish_path_parent_init, FUNCTION_BHV),
    define_builtin_function(bhv_squarish_path_parent_loop, FUNCTION_BHV),
    define_builtin_function(bhv_heave_ho_loop, FUNCTION_BHV),
    define_builtin_function(bhv_heave_ho_throw_mario_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ccm_touched_star_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_unused_poundable_platform, FUNCTION_BHV),
    define_builtin_function(bhv_beta_trampoline_top_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_trampoline_spring_loop, FUNCTION_BHV),
    define_builtin_function(bhv_jumping_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_boo_cage_init, FUNCTION_BHV),
    define_builtin_function(bhv_boo_cage_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_key_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_key_loop, FUNCTION_BHV),
    define_builtin_function(bhv_grand_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_grand_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_boo_key_loop, FUNCTION_BHV),
    define_builtin_function(bhv_alpha_boo_key_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bullet_bill_init, FUNCTION_BHV),
    define_builtin_function(bhv_bullet_bill_loop, FUNCTION_BHV),
    define_builtin_function(bhv_white_puff_smoke_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_tail_anchor_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_tail_anchor_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_body_anchor_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_body_anchor_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_flame_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tilting_bowser_lava_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_falling_bowser_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_blue_bowser_flame_init, FUNCTION_BHV),
    define_builtin_function(bhv_blue_bowser_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flame_floating_landing_init, FUNCTION_BHV),
    define_builtin_function(bhv_flame_floating_landing_loop, FUNCTION_BHV),
    define_builtin_function(bhv_blue_flames_group_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flame_bouncing_init, FUNCTION_BHV),
    define_builtin_function(bhv_flame_bouncing_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flame_moving_forward_growing_init, FUNCTION_BHV),
    define_builtin_function(bhv_flame_moving_forward_growing_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flame_bowser_init, FUNCTION_BHV),
    define_builtin_function(bhv_flame_bowser_loop, FUNCTION_BHV),
    define_builtin_function(bhv_flame_large_burning_out_init, FUNCTION_BHV),
    define_builtin_function(bhv_blue_fish_movement_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tank_fish_group_loop, FUNCTION_BHV),
    define_builtin_function(bhv_checkerboard_elevator_group_init, FUNCTION_BHV),
    define_builtin_function(bhv_checkerboard_elevator_group_loop, FUNCTION_BHV),
    define_builtin_function(bhv_checkerboard_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_checkerboard_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_key_unlock_door_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_key_course_exit_loop, FUNCTION_BHV),
    define_builtin_function(bhv_invisible_objects_under_bridge_init, FUNCTION_BHV),
    define_builtin_function(bhv_invisible_objects_under_bridge_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_level_pillar_init, FUNCTION_BHV),
    define_builtin_function(bhv_water_level_pillar_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ddd_warp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_moat_grills_loop, FUNCTION_BHV),
    define_builtin_function(bhv_rotating_clock_arm_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ukiki_init, FUNCTION_BHV),
    define_builtin_function(bhv_ukiki_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_sinking_rock_block_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_moving_octagonal_mesh_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_rotating_block_fire_bars_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_rotating_hex_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_wood_piece_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_floating_wood_bridge_loop, FUNCTION_BHV),
    define_builtin_function(bhv_volcano_flames_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_rotating_hexagonal_ring_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_sinking_rectangular_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_sinking_square_platforms_loop, FUNCTION_BHV),
    define_builtin_function(bhv_koopa_shell_loop, FUNCTION_BHV),
    define_builtin_function(bhv_koopa_shell_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tox_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_piranha_plant_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_bowser_puzzle_piece_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_bowser_puzzle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tuxies_mother_loop, FUNCTION_BHV),
    define_builtin_function(bhv_small_penguin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_fish_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_fish_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wdw_express_elevator_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bub_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bub_loop, FUNCTION_BHV),
    define_builtin_function(bhv_exclamation_box_init, FUNCTION_BHV),
    define_builtin_function(bhv_exclamation_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_rotating_exclamation_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sound_spawner_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowsers_sub_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sushi_shark_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sushi_shark_collision_loop, FUNCTION_BHV),
    define_builtin_function(bhv_jrb_sliding_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ship_part_3_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sunken_ship_part_loop, FUNCTION_BHV),
    define_builtin_function(bhv_white_puff_1_loop, FUNCTION_BHV),
    define_builtin_function(bhv_white_puff_2_loop, FUNCTION_BHV),
    define_builtin_function(bhv_blue_coin_switch_loop, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_blue_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_openable_cage_door_loop, FUNCTION_BHV),
    define_builtin_function(bhv_openable_grill_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_level_diamond_loop, FUNCTION_BHV),
    define_builtin_function(bhv_init_changing_water_level_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tweester_sand_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_tweester_loop, FUNCTION_BHV),
    define_builtin_function(bhv_merry_go_round_boo_manager_loop, FUNCTION_BHV),
    define_builtin_function(bhv_animated_texture_loop, FUNCTION_BHV),
    define_builtin_function(bhv_boo_in_castle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_boo_with_cage_init, FUNCTION_BHV),
    define_builtin_function(bhv_boo_with_cage_loop, FUNCTION_BHV),
    define_builtin_function(bhv_boo_init, FUNCTION_BHV),
    define_builtin_function(bhv_big_boo_loop, FUNCTION_BHV),
    define_builtin_function(bhv_courtyard_boo_triplet_init, FUNCTION_BHV),
    define_builtin_function(bhv_boo_loop, FUNCTION_BHV),
    define_builtin_function(bhv_boo_boss_spawned_bridge_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bbh_tilting_trap_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_haunted_bookshelf_loop, FUNCTION_BHV),
    define_builtin_function(bhv_merry_go_round_loop, FUNCTION_BHV),
    define_builtin_function(bhv_play_music_track_when_touched_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_bowser_anchor_loop, FUNCTION_BHV),
    define_builtin_function(bhv_static_checkered_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_castle_floor_trap_init, FUNCTION_BHV),
    define_builtin_function(bhv_castle_floor_trap_loop, FUNCTION_BHV),
    define_builtin_function(bhv_floor_trap_in_castle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sparkle_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_scuttlebug_loop, FUNCTION_BHV),
    define_builtin_function(bhv_scuttlebug_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_whomp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_splash_spawn_droplets, FUNCTION_BHV),
    define_builtin_function(bhv_water_droplet_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_droplet_splash_init, FUNCTION_BHV),
    define_builtin_function(bhv_bubble_splash_init, FUNCTION_BHV),
    define_builtin_function(bhv_idle_water_wave_loop, FUNCTION_BHV),
    define_builtin_function(bhv_shallow_water_splash_init, FUNCTION_BHV),
    define_builtin_function(bhv_wave_trail_shrink, FUNCTION_BHV),
    define_builtin_function(bhv_strong_wind_particle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sl_snowman_wind_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sl_walking_penguin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_menu_button_init, FUNCTION_BHV),
    define_builtin_function(bhv_menu_button_loop, FUNCTION_BHV),
    define_builtin_function(bhv_menu_button_manager_init, FUNCTION_BHV),
    define_builtin_function(bhv_menu_button_manager_loop, FUNCTION_BHV),
    define_builtin_function(bhv_act_selector_star_type_loop, FUNCTION_BHV),
    define_builtin_function(bhv_act_selector_init, FUNCTION_BHV),
    define_builtin_function(bhv_act_selector_loop, FUNCTION_BHV),
    define_builtin_function(bhv_moving_yellow_coin_init, FUNCTION_BHV),
    define_builtin_function(bhv_moving_yellow_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_moving_blue_coin_init, FUNCTION_BHV),
    define_builtin_function(bhv_moving_blue_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_blue_coin_sliding_jumping_init, FUNCTION_BHV),
    define_builtin_function(bhv_blue_coin_sliding_loop, FUNCTION_BHV),
    define_builtin_function(bhv_blue_coin_jumping_loop, FUNCTION_BHV),
    define_builtin_function(bhv_seaweed_init, FUNCTION_BHV),
    define_builtin_function(bhv_seaweed_bundle_init, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_init, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_fuse_smoke_init, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_buddy_init, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_buddy_loop, FUNCTION_BHV),
    define_builtin_function(bhv_cannon_closed_init, FUNCTION_BHV),
    define_builtin_function(bhv_cannon_closed_loop, FUNCTION_BHV),
    define_builtin_function(bhv_whirlpool_init, FUNCTION_BHV),
    define_builtin_function(bhv_whirlpool_loop, FUNCTION_BHV),
    define_builtin_function(bhv_jet_stream_loop, FUNCTION_BHV),
    define_builtin_function(bhv_homing_amp_init, FUNCTION_BHV),
    define_builtin_function(bhv_homing_amp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_circling_amp_init, FUNCTION_BHV),
    define_builtin_function(bhv_circling_amp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_butterfly_init, FUNCTION_BHV),
    define_builtin_function(bhv_butterfly_loop, FUNCTION_BHV),
    define_builtin_function(bhv_hoot_init, FUNCTION_BHV),
    define_builtin_function(bhv_hoot_loop, FUNCTION_BHV),
    define_builtin_function(bhv_beta_holdable_object_init, FUNCTION_BHV),
    define_builtin_function(bhv_beta_holdable_object_loop, FUNCTION_BHV),
    define_builtin_function(bhv_object_bubble_init, FUNCTION_BHV),
    define_builtin_function(bhv_object_bubble_loop, FUNCTION_BHV),
    define_builtin_function(bhv_object_water_wave_init, FUNCTION_BHV),
    define_builtin_function(bhv_object_water_wave_loop, FUNCTION_BHV),
    define_builtin_function(bhv_explosion_init, FUNCTION_BHV),
    define_builtin_function(bhv_explosion_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_bully_death_smoke_init, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_explosion_bubble_init, FUNCTION_BHV),
    define_builtin_function(bhv_bobomb_explosion_bubble_loop, FUNCTION_BHV),
    define_builtin_function(bhv_respawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_small_bully_init, FUNCTION_BHV),
    define_builtin_function(bhv_bully_loop, FUNCTION_BHV),
    define_builtin_function(bhv_big_bully_init, FUNCTION_BHV),
    define_builtin_function(bhv_big_bully_with_minions_init, FUNCTION_BHV),
    define_builtin_function(bhv_big_bully_with_minions_loop, FUNCTION_BHV),
    define_builtin_function(bhv_jet_stream_ring_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_jet_stream_water_ring_init, FUNCTION_BHV),
    define_builtin_function(bhv_jet_stream_water_ring_loop, FUNCTION_BHV),
    define_builtin_function(bhv_manta_ray_water_ring_init, FUNCTION_BHV),
    define_builtin_function(bhv_manta_ray_water_ring_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_bomb_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_bomb_explosion_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_bomb_smoke_loop, FUNCTION_BHV),
    define_builtin_function(bhv_celebration_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_celebration_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_celebration_star_sparkle_loop, FUNCTION_BHV),
    define_builtin_function(bhv_star_key_collection_puff_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_drawbridge_spawner_init, FUNCTION_BHV),
    define_builtin_function(bhv_lll_drawbridge_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_drawbridge_loop, FUNCTION_BHV),
    define_builtin_function(bhv_small_bomp_init, FUNCTION_BHV),
    define_builtin_function(bhv_small_bomp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_large_bomp_init, FUNCTION_BHV),
    define_builtin_function(bhv_large_bomp_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wf_sliding_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_wf_sliding_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_moneybag_init, FUNCTION_BHV),
    define_builtin_function(bhv_moneybag_loop, FUNCTION_BHV),
    define_builtin_function(bhv_moneybag_hidden_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bob_pit_bowling_ball_init, FUNCTION_BHV),
    define_builtin_function(bhv_bob_pit_bowling_ball_loop, FUNCTION_BHV),
    define_builtin_function(bhv_free_bowling_ball_init, FUNCTION_BHV),
    define_builtin_function(bhv_free_bowling_ball_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowling_ball_init, FUNCTION_BHV),
    define_builtin_function(bhv_bowling_ball_loop, FUNCTION_BHV),
    define_builtin_function(bhv_generic_bowling_ball_spawner_init, FUNCTION_BHV),
    define_builtin_function(bhv_generic_bowling_ball_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_thi_bowling_ball_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_rr_cruiser_wing_init, FUNCTION_BHV),
    define_builtin_function(bhv_rr_cruiser_wing_loop, FUNCTION_BHV),
    define_builtin_function(bhv_spindel_init, FUNCTION_BHV),
    define_builtin_function(bhv_spindel_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ssl_moving_pyramid_wall_init, FUNCTION_BHV),
    define_builtin_function(bhv_ssl_moving_pyramid_wall_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_elevator_init, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_elevator_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_elevator_trajectory_marker_ball_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_top_init, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_top_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_top_fragment_init, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_top_fragment_loop, FUNCTION_BHV),
    define_builtin_function(bhv_pyramid_pillar_touch_detector_loop, FUNCTION_BHV),
    define_builtin_function(bhv_waterfall_sound_loop, FUNCTION_BHV),
    define_builtin_function(bhv_volcano_sound_loop, FUNCTION_BHV),
    define_builtin_function(bhv_castle_flag_init, FUNCTION_BHV),
    define_builtin_function(bhv_birds_sound_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ambient_sounds_init, FUNCTION_BHV),
    define_builtin_function(bhv_sand_sound_loop, FUNCTION_BHV),
    define_builtin_function(bhv_castle_cannon_grate_init, FUNCTION_BHV),
    define_builtin_function(bhv_snowmans_bottom_init, FUNCTION_BHV),
    define_builtin_function(bhv_snowmans_bottom_loop, FUNCTION_BHV),
    define_builtin_function(bhv_snowmans_head_init, FUNCTION_BHV),
    define_builtin_function(bhv_snowmans_head_loop, FUNCTION_BHV),
    define_builtin_function(bhv_snowmans_body_checkpoint_loop, FUNCTION_BHV),
    define_builtin_function(bhv_big_boulder_init, FUNCTION_BHV),
    define_builtin_function(bhv_big_boulder_loop, FUNCTION_BHV),
    define_builtin_function(bhv_big_boulder_generator_loop, FUNCTION_BHV),
    define_builtin_function(bhv_wing_cap_init, FUNCTION_BHV),
    define_builtin_function(bhv_wing_vanish_cap_loop, FUNCTION_BHV),
    define_builtin_function(bhv_metal_cap_init, FUNCTION_BHV),
    define_builtin_function(bhv_metal_cap_loop, FUNCTION_BHV),
    define_builtin_function(bhv_normal_cap_init, FUNCTION_BHV),
    define_builtin_function(bhv_normal_cap_loop, FUNCTION_BHV),
    define_builtin_function(bhv_vanish_cap_init, FUNCTION_BHV),
    define_builtin_function(bhv_collect_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_collect_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_star_spawn_init, FUNCTION_BHV),
    define_builtin_function(bhv_star_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_red_coin_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_red_coin_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_red_coin_init, FUNCTION_BHV),
    define_builtin_function(bhv_red_coin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bowser_course_red_coin_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_star_loop, FUNCTION_BHV),
    define_builtin_function(bhv_hidden_star_trigger_loop, FUNCTION_BHV),
    define_builtin_function(bhv_ttm_rolling_log_init, FUNCTION_BHV),
    define_builtin_function(bhv_rolling_log_loop, FUNCTION_BHV),
    define_builtin_function(bhv_lll_rolling_log_init, FUNCTION_BHV),
    define_builtin_function(bhv_1up_common_init, FUNCTION_BHV),
    define_builtin_function(bhv_1up_walking_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_running_away_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_sliding_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_init, FUNCTION_BHV),
    define_builtin_function(bhv_1up_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_jump_on_approach_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_hidden_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_hidden_trigger_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_hidden_in_pole_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_hidden_in_pole_trigger_loop, FUNCTION_BHV),
    define_builtin_function(bhv_1up_hidden_in_pole_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_controllable_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_controllable_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_controllable_platform_sub_loop, FUNCTION_BHV),
    define_builtin_function(bhv_breakable_box_small_init, FUNCTION_BHV),
    define_builtin_function(bhv_breakable_box_small_loop, FUNCTION_BHV),
    define_builtin_function(bhv_sliding_snow_mound_loop, FUNCTION_BHV),
    define_builtin_function(bhv_snow_mound_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_floating_platform_loop, FUNCTION_BHV),
    define_builtin_function(bhv_arrow_lift_loop, FUNCTION_BHV),
    define_builtin_function(bhv_orange_number_init, FUNCTION_BHV),
    define_builtin_function(bhv_orange_number_loop, FUNCTION_BHV),
    define_builtin_function(bhv_manta_ray_init, FUNCTION_BHV),
    define_builtin_function(bhv_manta_ray_loop, FUNCTION_BHV),
    define_builtin_function(bhv_falling_pillar_init, FUNCTION_BHV),
    define_builtin_function(bhv_falling_pillar_loop, FUNCTION_BHV),
    define_builtin_function(bhv_falling_pillar_hitbox_loop, FUNCTION_BHV),
    define_builtin_function(bhv_jrb_floating_box_loop, FUNCTION_BHV),
    define_builtin_function(bhv_decorative_pendulum_init, FUNCTION_BHV),
    define_builtin_function(bhv_decorative_pendulum_loop, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_ship_init, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_ship_loop, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_jrb_init, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_jrb_loop, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_init, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_loop, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_bottom_init, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_bottom_loop, FUNCTION_BHV),
    define_builtin_function(bhv_treasure_chest_top_loop, FUNCTION_BHV),
    define_builtin_function(bhv_mips_init, FUNCTION_BHV),
    define_builtin_function(bhv_mips_loop, FUNCTION_BHV),
    define_builtin_function(bhv_yoshi_init, FUNCTION_BHV),
    define_builtin_function(bhv_koopa_init, FUNCTION_BHV),
    define_builtin_function(bhv_koopa_update, FUNCTION_BHV),
    define_builtin_function(bhv_koopa_race_endpoint_update, FUNCTION_BHV),
    define_builtin_function(bhv_pokey_update, FUNCTION_BHV),
    define_builtin_function(bhv_pokey_body_part_update, FUNCTION_BHV),
    define_builtin_function(bhv_swoop_update, FUNCTION_BHV),
    define_builtin_function(bhv_fly_guy_update, FUNCTION_BHV),
    define_builtin_function(bhv_goomba_init, FUNCTION_BHV),
    define_builtin_function(bhv_goomba_update, FUNCTION_BHV),
    define_builtin_function(bhv_goomba_triplet_spawner_update, FUNCTION_BHV),
    define_builtin_function(bhv_chain_chomp_update, FUNCTION_BHV),
    define_builtin_function(bhv_chain_chomp_chain_part_update, FUNCTION_BHV),
    define_builtin_function(bhv_wooden_post_update, FUNCTION_BHV),
    define_builtin_function(bhv_chain_chomp_gate_init, FUNCTION_BHV),
    define_builtin_function(bhv_chain_chomp_gate_update, FUNCTION_BHV),
    define_builtin_function(bhv_wiggler_update, FUNCTION_BHV),
    define_builtin_function(bhv_wiggler_body_part_update, FUNCTION_BHV),
    define_builtin_function(bhv_enemy_lakitu_update, FUNCTION_BHV),
    define_builtin_function(bhv_camera_lakitu_init, FUNCTION_BHV),
    define_builtin_function(bhv_camera_lakitu_update, FUNCTION_BHV),
    define_builtin_function(bhv_cloud_update, FUNCTION_BHV),
    define_builtin_function(bhv_cloud_part_update, FUNCTION_BHV),
    define_builtin_function(bhv_spiny_update, FUNCTION_BHV),
    define_builtin_function(bhv_monty_mole_init, FUNCTION_BHV),
    define_builtin_function(bhv_monty_mole_update, FUNCTION_BHV),
    define_builtin_function(bhv_monty_mole_hole_update, FUNCTION_BHV),
    define_builtin_function(bhv_monty_mole_rock_update, FUNCTION_BHV),
    define_builtin_function(bhv_platform_on_track_init, FUNCTION_BHV),
    define_builtin_function(bhv_platform_on_track_update, FUNCTION_BHV),
    define_builtin_function(bhv_track_ball_update, FUNCTION_BHV),
    define_builtin_function(bhv_seesaw_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_seesaw_platform_update, FUNCTION_BHV),
    define_builtin_function(bhv_ferris_wheel_axle_init, FUNCTION_BHV),
    define_builtin_function(bhv_ferris_wheel_platform_update, FUNCTION_BHV),
    define_builtin_function(bhv_water_bomb_spawner_update, FUNCTION_BHV),
    define_builtin_function(bhv_water_bomb_update, FUNCTION_BHV),
    define_builtin_function(bhv_water_bomb_shadow_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_rotating_solid_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_rotating_solid_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_pendulum_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_pendulum_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_treadmill_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_treadmill_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_moving_bar_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_moving_bar_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_cog_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_cog_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_pit_block_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_pit_block_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_elevator_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_elevator_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_2d_rotator_init, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_2d_rotator_update, FUNCTION_BHV),
    define_builtin_function(bhv_ttc_spinner_update, FUNCTION_BHV),
    define_builtin_function(bhv_mr_blizzard_init, FUNCTION_BHV),
    define_builtin_function(bhv_mr_blizzard_update, FUNCTION_BHV),
    define_builtin_function(bhv_mr_blizzard_snowball, FUNCTION_BHV),
    define_builtin_function(bhv_sliding_plat_2_init, FUNCTION_BHV),
    define_builtin_function(bhv_sliding_plat_2_loop, FUNCTION_BHV),
    define_builtin_function(bhv_rotating_octagonal_plat_init, FUNCTION_BHV),
    define_builtin_function(bhv_rotating_octagonal_plat_loop, FUNCTION_BHV),
    define_builtin_function(bhv_animates_on_floor_switch_press_init, FUNCTION_BHV),
    define_builtin_function(bhv_animates_on_floor_switch_press_loop, FUNCTION_BHV),
    define_builtin_function(bhv_activated_back_and_forth_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_activated_back_and_forth_platform_update, FUNCTION_BHV),
    define_builtin_function(bhv_recovery_heart_loop, FUNCTION_BHV),
    define_builtin_function(bhv_water_bomb_cannon_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bubble_cannon_barrel_loop, FUNCTION_BHV),
    define_builtin_function(bhv_unagi_init, FUNCTION_BHV),
    define_builtin_function(bhv_unagi_loop, FUNCTION_BHV),
    define_builtin_function(bhv_unagi_subobject_loop, FUNCTION_BHV),
    define_builtin_function(bhv_dorrie_update, FUNCTION_BHV),
    define_builtin_function(bhv_haunted_chair_init, FUNCTION_BHV),
    define_builtin_function(bhv_haunted_chair_loop, FUNCTION_BHV),
    define_builtin_function(bhv_mad_piano_update, FUNCTION_BHV),
    define_builtin_function(bhv_flying_bookend_loop, FUNCTION_BHV),
    define_builtin_function(bhv_bookend_spawn_loop, FUNCTION_BHV),
    define_builtin_function(bhv_haunted_bookshelf_manager_loop, FUNCTION_BHV),
    define_builtin_function(bhv_book_switch_loop, FUNCTION_BHV),
    define_builtin_function(bhv_fire_piranha_plant_init, FUNCTION_BHV),
    define_builtin_function(bhv_fire_piranha_plant_update, FUNCTION_BHV),
    define_builtin_function(bhv_small_piranha_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_fire_spitter_update, FUNCTION_BHV),
    define_builtin_function(bhv_fly_guy_flame_loop, FUNCTION_BHV),
    define_builtin_function(bhv_snufit_loop, FUNCTION_BHV),
    define_builtin_function(bhv_snufit_balls_loop, FUNCTION_BHV),
    define_builtin_function(bhv_horizontal_grindel_init, FUNCTION_BHV),
    define_builtin_function(bhv_horizontal_grindel_update, FUNCTION_BHV),
    define_builtin_function(bhv_eyerok_boss_init, FUNCTION_BHV),
    define_builtin_function(bhv_eyerok_boss_loop, FUNCTION_BHV),
    define_builtin_function(bhv_eyerok_hand_loop, FUNCTION_BHV),
    define_builtin_function(bhv_klepto_init, FUNCTION_BHV),
    define_builtin_function(bhv_klepto_update, FUNCTION_BHV),
    define_builtin_function(bhv_bird_update, FUNCTION_BHV),
    define_builtin_function(bhv_racing_penguin_init, FUNCTION_BHV),
    define_builtin_function(bhv_racing_penguin_update, FUNCTION_BHV),
    define_builtin_function(bhv_penguin_race_finish_line_update, FUNCTION_BHV),
    define_builtin_function(bhv_penguin_race_shortcut_check_update, FUNCTION_BHV),
    define_builtin_function(bhv_coffin_spawner_loop, FUNCTION_BHV),
    define_builtin_function(bhv_coffin_loop, FUNCTION_BHV),
    define_builtin_function(bhv_clam_loop, FUNCTION_BHV),
    define_builtin_function(bhv_skeeter_update, FUNCTION_BHV),
    define_builtin_function(bhv_skeeter_wave_update, FUNCTION_BHV),
    define_builtin_function(bhv_swing_platform_init, FUNCTION_BHV),
    define_builtin_function(bhv_swing_platform_update, FUNCTION_BHV),
    define_builtin_function(bhv_donut_platform_spawner_update, FUNCTION_BHV),
    define_builtin_function(bhv_donut_platform_update, FUNCTION_BHV),
    define_builtin_function(bhv_ddd_pole_init, FUNCTION_BHV),
    define_builtin_function(bhv_ddd_pole_update, FUNCTION_BHV),
    define_builtin_function(bhv_red_coin_star_marker_init, FUNCTION_BHV),
    define_builtin_function(bhv_triplet_butterfly_update, FUNCTION_BHV),
    define_builtin_function(bhv_bubba_loop, FUNCTION_BHV),
    define_builtin_function(bhv_intro_lakitu_loop, FUNCTION_BHV),
    define_builtin_function(bhv_intro_peach_loop, FUNCTION_BHV),
    define_builtin_function(bhv_end_birds_1_loop, FUNCTION_BHV),
    define_builtin_function(bhv_end_birds_2_loop, FUNCTION_BHV),
    define_builtin_function(bhv_intro_scene_loop, FUNCTION_BHV),
    define_builtin_function(bhv_dust_smoke_loop, FUNCTION_BHV),
    define_builtin_function(bhv_yoshi_loop, FUNCTION_BHV),
    define_builtin_function(bhv_volcano_trap_loop, FUNCTION_BHV),

    // mario_misc.h
    define_builtin_function(bhv_toad_message_init, FUNCTION_BHV),
    define_builtin_function(bhv_toad_message_loop, FUNCTION_BHV),
    define_builtin_function(bhv_unlock_door_star_init, FUNCTION_BHV),
    define_builtin_function(bhv_unlock_door_star_loop, FUNCTION_BHV),

    // Other
    define_builtin_function(load_object_collision_model, FUNCTION_BHV),
    define_builtin_function(obj_set_secondary_camera_focus, FUNCTION_BHV),

    // Menu related
    define_builtin_function(lvl_intro_update, FUNCTION_LVL),
    define_builtin_function(geo_intro_super_mario_64_logo, FUNCTION_GEO),
    define_builtin_function(geo_intro_tm_copyright, FUNCTION_GEO),
    define_builtin_function(geo_intro_regular_backdrop, FUNCTION_GEO),
    define_builtin_function(geo_draw_mario_head_goddard, FUNCTION_GEO),

    // Custom
    define_builtin_function(bhv_blue_coin_number_loop, FUNCTION_BHV),
    define_builtin_function(bhv_blue_coin_switch_init, FUNCTION_BHV),
    define_builtin_function(bhv_star_number_loop, FUNCTION_BHV),
    define_builtin_function(spawn_star_number, FUNCTION_BHV),
    define_builtin_function(bhv_ferris_wheel_platform_init, FUNCTION_BHV),
    define_builtin_function(geo_mario_cap_display_list, FUNCTION_GEO),
    define_builtin_function(bhv_ambient_light_update, FUNCTION_BHV),
    define_builtin_function(bhv_point_light_init, FUNCTION_BHV),
    define_builtin_function(bhv_point_light_loop, FUNCTION_BHV),
    define_builtin_function(geo_switch_character_type, FUNCTION_GEO),
};

static const char *sDynosBuiltinFuncTypeNames[] = {
    [0]            = "other",
    [FUNCTION_GEO] = "geo layout",
    [FUNCTION_BHV] = "behavior",
    [FUNCTION_LVL] = "level script",
};

const void* DynOS_Builtin_Func_GetFromName(const char* aDataName, u8 aFuncType) {
    for (const auto &builtinFunc : sDynosBuiltinFuncs) {
        if (builtinFunc.type == aFuncType && strcmp(builtinFunc.name, aDataName) == 0) {
            return builtinFunc.func;
        }
    }
    return NULL;
}

const void* DynOS_Builtin_Func_GetFromIndex(s32 aIndex, u8 aFuncType) {
    s32 count = (s32) (sizeof(sDynosBuiltinFuncs) / sizeof(sDynosBuiltinFuncs[0]));
    if (aIndex < 0 || aIndex >= count) { return NULL; }
    if (sDynosBuiltinFuncs[aIndex].type != aFuncType) { return NULL; }
    return sDynosBuiltinFuncs[aIndex].func;
}

s32 DynOS_Builtin_Func_GetIndexFromData(const void* aData, u8 aFuncType) {
    s32 count = (s32) (sizeof(sDynosBuiltinFuncs) / sizeof(sDynosBuiltinFuncs[0]));
    for (s32 i = 0; i < count; ++i) {
        const auto &builtinFunc = sDynosBuiltinFuncs[i];
        if (builtinFunc.type == aFuncType && builtinFunc.func == aData) {
            return i;
        }
    }
    return -1;
}

static String DynOS_Builtin_Func_CheckMisuse_Internal(s32 aIndex, const char* aDataName, const void* aData, u8 aFuncType) {
    s32 count = (s32) (sizeof(sDynosBuiltinFuncs) / sizeof(sDynosBuiltinFuncs[0]));
    for (s32 i = 0; i < count; ++i) {
        const auto &builtinFunc = sDynosBuiltinFuncs[i];
        if (aFuncType != builtinFunc.type && (
            aIndex == i || (aDataName && strcmp(aDataName, builtinFunc.name) == 0) || aData == builtinFunc.func)) {
            return String(
                "Invalid use of %s function in %s: %s",
                builtinFunc.name,
                sDynosBuiltinFuncTypeNames[builtinFunc.type],
                sDynosBuiltinFuncTypeNames[aFuncType]
            );
        }
    }
    return "";
}

String DynOS_Builtin_Func_CheckMisuse(s32 aIndex, u8 aFuncType) {
    return DynOS_Builtin_Func_CheckMisuse_Internal(aIndex, NULL, NULL, aFuncType);
}

String DynOS_Builtin_Func_CheckMisuse(const char* aDataName, u8 aFuncType) {
    return DynOS_Builtin_Func_CheckMisuse_Internal(-1, aDataName, NULL, aFuncType);
}

String DynOS_Builtin_Func_CheckMisuse(const void* aData, u8 aFuncType) {
    return DynOS_Builtin_Func_CheckMisuse_Internal(-1, NULL, aData, aFuncType);
}

  ///////////////////
 // Display Lists //
///////////////////

static const void *sDynosBuiltinDisplayLists[] = {
#define DISPLAY_LIST(name) define_builtin(name),
#include "include/display_lists.inl"
#undef DISPLAY_LIST
};

const Gfx *DynOS_Builtin_Gfx_GetFromName(const char *aDataName) {
    MGR_FIND_DATA(sDynosBuiltinDisplayLists, (const Gfx *));
}

const char *DynOS_Builtin_Gfx_GetFromData(const Gfx *aData) {
    MGR_FIND_NAME(sDynosBuiltinDisplayLists);
}
