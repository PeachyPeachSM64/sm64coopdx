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

#include "actors/custom0.h"

#include "make_const_nonconst.h"
#include "levels/fourth_floor/header.h"

const LevelScript level_fourth_floor_entry[] = {
	INIT_LEVEL(),
	LOAD_MIO0(0x7, _fourth_floor_segment_7SegmentRomStart, _fourth_floor_segment_7SegmentRomEnd),
	ALLOC_LEVEL_POOL(),
	MARIO(MODEL_MARIO, 0x00000001, bhvMario),
	LOAD_MODEL_FROM_GEO(MODEL_WARP_PIPE_BOO_BLUE,            warp_pipe_boo_geo),
	LOAD_MODEL_FROM_GEO(MODEL_WARP_PIPE_BOO_RED,             warp_pipe_boo_red_geo),
	LOAD_MODEL_FROM_GEO(MODEL_WARP_PIPE_BOO_GREEN_LOCKED,    warp_pipe_boo_green_locked_geo),
	LOAD_MODEL_FROM_GEO(MODEL_WARP_PIPE_BOO_GREEN_UNLOCKED,  warp_pipe_boo_green_unlocked_geo),
	LOAD_MODEL_FROM_GEO(MODEL_WARP_PIPE_BOO_YELLOW_LOCKED,   warp_pipe_boo_yellow_locked_geo),
	LOAD_MODEL_FROM_GEO(MODEL_WARP_PIPE_BOO_YELLOW_UNLOCKED, warp_pipe_boo_yellow_unlocked_geo),

	AREA(1, fourth_floor_area_1),
		// Blue pipe (Mario) - center back
		OBJECT(MODEL_WARP_PIPE_BOO_BLUE,             311, -110, 2341, 0, 180, 0, 0, bhvWarpPipeBooBlue),
		// Red pipe (Mario alternate)
		OBJECT(MODEL_WARP_PIPE_BOO_RED,              311, -110, 2341, 0, 180, 0, 1, bhvWarpPipeBooRed),
		// Green pipes (Luigi) - right side
		OBJECT(MODEL_WARP_PIPE_BOO_GREEN_LOCKED,    1895, -110, 2302, 0, 190, 0, 0, bhvWarpPipeBooGreenLocked),
		OBJECT(MODEL_WARP_PIPE_BOO_GREEN_UNLOCKED,  1895, -110, 2302, 0, 190, 0, 0, bhvWarpPipeBooGreenUnlocked),
		// Yellow pipes (Wario) - left side
		OBJECT(MODEL_WARP_PIPE_BOO_YELLOW_LOCKED,  -1281, -110, 2270, 0, -190, 0, 2, bhvWarpPipeBooYellowLocked),
		OBJECT(MODEL_WARP_PIPE_BOO_YELLOW_UNLOCKED,-1281, -110, 2270, 0, -190, 0, 2, bhvWarpPipeBooYellowUnlocked),
		OBJECT(MODEL_NONE, 376, -110, -533, 0, 0, 0, 0x000A0000, bhvSpinAirborneWarp),
		// Entry warp node
		WARP_NODE(/*id*/ 0x0A, /*destLevel*/ LEVEL_CASTLE, /*destArea*/ 0x01, /*destNode*/ 0x1F, /*flags*/ WARP_NO_CHECKPOINT),
		// Death/exit warp nodes
		WARP_NODE(/*id*/ 0xF0, /*destLevel*/ LEVEL_CASTLE, /*destArea*/ 0x01, /*destNode*/ 0x1F, /*flags*/ WARP_NO_CHECKPOINT),
		WARP_NODE(/*id*/ 0xF1, /*destLevel*/ LEVEL_CASTLE, /*destArea*/ 0x01, /*destNode*/ 0x1F, /*flags*/ WARP_NO_CHECKPOINT),
		TERRAIN(fourth_floor_area_1_collision),
		MACRO_OBJECTS(fourth_floor_area_1_Area_1_macro_objs),
		SET_BACKGROUND_MUSIC(0x00, SEQ_LEVEL_FOURTH_FLOOR),
		TERRAIN_TYPE(TERRAIN_STONE),
	END_AREA(),

	FREE_LEVEL_POOL(),
	MARIO_POS(0x01, 0, 373, -120, 450),
	CALL(0, lvl_init_or_update),
	CALL_LOOP(1, lvl_init_or_update),
	CLEAR_LEVEL(),
	SLEEP_BEFORE_EXIT(1),
	EXIT(),
};
