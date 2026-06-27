--- @type integer
FONT_TINY = -1

--- @type integer
ANIM_FLAG_FORWARD = (1 << 1)

------------------------
-- E_MODEL duplicates --
------------------------

E_MODEL_BURN_SMOKE_UNUSED          = E_MODEL_BURN_SMOKE
E_MODEL_BOWSER_BOMB_CHILD_OBJ      = E_MODEL_BOWSER_BOMB
E_MODEL_BOWSER2                    = E_MODEL_BOWSER_BITS_DEFEATED
E_MODEL_BOB_BUBBLY_TREE            = E_MODEL_BUBBLY_TREE
E_MODEL_WF_BUBBLY_TREE             = E_MODEL_BUBBLY_TREE
E_MODEL_CCM_CABIN_DOOR             = E_MODEL_CABIN_DOOR
E_MODEL_CCM_SNOW_TREE              = E_MODEL_SNOW_TREE
E_MODEL_BBH_HAUNTED_DOOR           = E_MODEL_HAUNTED_DOOR
E_MODEL_HMC_WOODEN_DOOR            = E_MODEL_WOODEN_DOOR
E_MODEL_HMC_METAL_DOOR             = E_MODEL_METAL_DOOR
E_MODEL_HMC_HAZY_MAZE_DOOR         = E_MODEL_HAZY_MAZE_DOOR
E_MODEL_SSL_PALM_TREE              = E_MODEL_PALM_TREE
E_MODEL_SL_SNOW_TREE               = E_MODEL_SNOW_TREE
E_MODEL_WDW_BUBBLY_TREE            = E_MODEL_BUBBLY_TREE
E_MODEL_THI_BUBBLY_TREE            = E_MODEL_BUBBLY_TREE
E_MODEL_THI_WARP_PIPE              = E_MODEL_WARP_PIPE
E_MODEL_BITDW_WARP_PIPE            = E_MODEL_WARP_PIPE
E_MODEL_BITS_WARP_PIPE             = E_MODEL_WARP_PIPE
E_MODEL_BOWSER_1_YELLOW_SPHERE     = E_MODEL_BOWSER_YELLOW_SPHERE
E_MODEL_VCUTM_WARP_PIPE            = E_MODEL_WARP_PIPE
E_MODEL_CASTLE_GROUNDS_BUBBLY_TREE = E_MODEL_BUBBLY_TREE
E_MODEL_CASTLE_GROUNDS_WARP_PIPE   = E_MODEL_WARP_PIPE
E_MODEL_CASTLE_GROUNDS_CASTLE_DOOR = E_MODEL_CASTLE_DOOR
E_MODEL_CASTLE_GROUNDS_METAL_DOOR  = E_MODEL_METAL_DOOR
E_MODEL_CASTLE_CASTLE_DOOR         = E_MODEL_CASTLE_DOOR
E_MODEL_CASTLE_WOODEN_DOOR         = E_MODEL_WOODEN_DOOR
E_MODEL_CASTLE_METAL_DOOR          = E_MODEL_METAL_DOOR
E_MODEL_CASTLE_CASTLE_DOOR_UNUSED  = E_MODEL_CASTLE_DOOR
E_MODEL_CASTLE_WOODEN_DOOR_UNUSED  = E_MODEL_WOODEN_DOOR
E_MODEL_CASTLE_KEY_DOOR            = E_MODEL_KEY_DOOR
E_MODEL_CASTLE_STAR_DOOR_8_STARS   = E_MODEL_STAR_DOOR
E_MODEL_CASTLE_STAR_DOOR_30_STARS  = E_MODEL_STAR_DOOR
E_MODEL_CASTLE_STAR_DOOR_50_STARS  = E_MODEL_STAR_DOOR
E_MODEL_CASTLE_STAR_DOOR_70_STARS  = E_MODEL_STAR_DOOR
E_MODEL_COURTYARD_SPIKY_TREE       = E_MODEL_SPIKY_TREE
E_MODEL_COURTYARD_WOODEN_DOOR      = E_MODEL_WOODEN_DOOR

-----------------------
-- Renamed functions --
-----------------------

rom_hack_cam_set_collisions = camera_romhack_set_collisions
camera_romhack_allow_centering = camera_romhack_allow_switchable
camera_romhack_get_allow_centering = camera_romhack_get_allow_switchable

cur_obj_enable_rendering_2 = cur_obj_enable_rendering
cur_obj_can_mario_activate_textbox_2 = cur_obj_can_mario_activate_textbox
cur_obj_play_sound_1 = cur_obj_play_sound_if_visible
cur_obj_play_sound_2 = cur_obj_play_sound_and_rumble_if_visible

obj_has_model_extended = obj_has_model
obj_get_model_id_extended = obj_get_model_id
obj_set_model_extended = obj_set_model

bhv_star_door_loop_2 = bhv_star_door_loop_update_render_state
reset_rumble_timers_2 = reset_rumble_timers_vibrate

--------------------
-- Math functions --
--------------------
--- Note: These functions were originally in smlua_math_utils.h,
--- but performed worse (~2x slower) than built-in Lua math functions

min = math.min
minf = math.min
max = math.max
maxf = math.max
sqr = math.sqr
sqrf = math.sqr
clamp = math.clamp
clampf = math.clamp
hypotf = math.hypot
absf_2 = math.abs
bit_shift_left = function (shift) return math.u8(1 << shift) end
