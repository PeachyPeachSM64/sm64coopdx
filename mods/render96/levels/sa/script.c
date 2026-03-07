#include <ultra64.h>
#include "sm64.h"
#include "behavior_data.h"
#include "model_ids.h"
#include "seq_ids.h"
#include "dialog_ids.h"
#include "segment_symbols.h"
#include "level_commands.h"

#include "game/level_update.h"

#include "levels/scripts.h"

#include "actors/common1.h"

#include "make_const_nonconst.h"
#include "levels/sa/header.h"

/* Fast64 begin persistent block [scripts] */
/* Fast64 end persistent block [scripts] */

const LevelScript level_sa_entry[] = {
	INIT_LEVEL(),
	LOAD_MIO0(0x7, _sa_segment_7SegmentRomStart, _sa_segment_7SegmentRomEnd), 
	LOAD_MIO0_TEXTURE(0x9, _inside_mio0SegmentRomStart, _inside_mio0SegmentRomEnd), 
	LOAD_MIO0(0xa, _clouds_skybox_mio0SegmentRomStart, _clouds_skybox_mio0SegmentRomEnd), 
	LOAD_MIO0(0xb, _effect_mio0SegmentRomStart, _effect_mio0SegmentRomEnd), 
	LOAD_MIO0(0x5, _group4_mio0SegmentRomStart, _group4_mio0SegmentRomEnd), 
	LOAD_RAW(0xc, _group4_geoSegmentRomStart, _group4_geoSegmentRomEnd), 
	LOAD_MIO0(0x6, _group13_mio0SegmentRomStart, _group13_mio0SegmentRomEnd), 
	LOAD_RAW(0xd, _group13_geoSegmentRomStart, _group13_geoSegmentRomEnd), 
	ALLOC_LEVEL_POOL(),
	MARIO(MODEL_MARIO, 0x00000001, bhvMario), 
	JUMP_LINK(script_func_global_5),
	JUMP_LINK(script_func_global_14),
	/* Fast64 begin persistent block [level commands] */
	/* Fast64 end persistent block [level commands] */

	AREA(1, sa_area_1),
		WARP_NODE(0x0A, LEVEL_SA, 0x01, 0x0A, WARP_NO_CHECKPOINT),
		WARP_NODE(0xF0, LEVEL_CASTLE, 0x01, 0x27, WARP_NO_CHECKPOINT),
		WARP_NODE(0xF1, LEVEL_BITDW, 0x01, 0x28, WARP_NO_CHECKPOINT),
		OBJECT(E_MODEL_BETA_BOO_KEY, -318, -160, -38, 0, 0, 0, 0x00080000, bhvBetaBooKey),
		OBJECT(E_MODEL_NONE, 2500, -2050, 0, 0, 0, 0, 0x00FFFFFF, bhvPointLight),
		OBJECT(E_MODEL_NONE, -2500, -2050, 0, 0, 0, 0, 0x00FF00FF, bhvPointLight),
		OBJECT(E_MODEL_NONE, 0, -4250, 0, 0, 0, 0, 0, bhvHiddenRedCoinStar),
		OBJECT(E_MODEL_NONE, 0, -1000, 0, 0, 0, 0, 0, bhvFishSpawner),
		OBJECT(E_MODEL_NONE, 0, -1000, 0, 0, 0, 0, (0x02 << 16), bhvFishSpawner),
		MARIO_POS(0x01, 90, 0, -1535, 0),
		OBJECT(E_MODEL_NONE, 0, -2050, -2500, 0, 0, 0, 0xFF0000FF, bhvPointLight),
		OBJECT(MODEL_NONE, 0, -1535, 0, 0, 90, 0, (0x0A << 16), bhvSwimmingWarp),
		OBJECT(E_MODEL_NONE, 0, -2050, 2500, 0, 0, 0, 0xFFFF00FF, bhvPointLight),
		TERRAIN(sa_area_1_collision),
		MACRO_OBJECTS(sa_area_1_macro_objs),
		SET_BACKGROUND_MUSIC(0x0003, SEQ_LEVEL_WATER),
		TERRAIN_TYPE(TERRAIN_WATER),
		/* Fast64 begin persistent block [area commands] */
		/* Fast64 end persistent block [area commands] */
	END_AREA(),
	FREE_LEVEL_POOL(),
	MARIO_POS(0x01, 90, 0, -1535, 0),
	CALL(0, lvl_init_or_update),
	CALL_LOOP(1, lvl_init_or_update),
	CLEAR_LEVEL(),
	SLEEP_BEFORE_EXIT(1),
	EXIT(),
};