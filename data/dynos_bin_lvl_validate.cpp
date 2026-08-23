#include "dynos.cpp.h"
#include <map>
extern "C" {
#include "include/level_commands.h"
}

struct LevelScriptCommand {
    u8 id;
    u8 size;
    LevelScript command[16];
};

#define LVL_COMMAND_ID(...) \
    (u8) (((LevelScript[]){ __VA_ARGS__ })[0])

#define LVL_COMMAND(cmd) { \
    LVL_COMMAND_ID(cmd), { \
        .id = LVL_COMMAND_ID(cmd), \
        .size = 4 * (u8) (sizeof((LevelScript[]){ cmd }) / sizeof(LevelScript)), \
        .command = { cmd } \
    } \
}

static std::map<u8, struct LevelScriptCommand> sLevelScriptCommands = {
LVL_COMMAND( EXECUTE                 (0, 0, 0, PTYPE_PNTR_LVL)),
LVL_COMMAND( EXIT_AND_EXECUTE        (0, 0, 0, PTYPE_PNTR_LVL)),
LVL_COMMAND( EXIT                    ()),
LVL_COMMAND( SLEEP                   (0)),
LVL_COMMAND( SLEEP_BEFORE_EXIT       (0)),
LVL_COMMAND( JUMP                    (PTYPE_PNTR_LVL)),
LVL_COMMAND( JUMP_LINK               (PTYPE_PNTR_LVL)),
LVL_COMMAND( RETURN                  ()),
LVL_COMMAND( JUMP_LINK_PUSH_ARG      (0)),
LVL_COMMAND( JUMP_N_TIMES            ()),
LVL_COMMAND( LOOP_BEGIN              ()),
LVL_COMMAND( LOOP_UNTIL              (0, 0)),
LVL_COMMAND( JUMP_IF                 (0, 0, PTYPE_PNTR_LVL)),
LVL_COMMAND( JUMP_LINK_IF            (0, 0, PTYPE_PNTR_LVL)),
LVL_COMMAND( SKIP_IF                 (0, 0)),
LVL_COMMAND( SKIP                    ()),
LVL_COMMAND( SKIP_NOP                ()),
LVL_COMMAND( CALL                    (0, PTYPE_FUNC_LVL)),
LVL_COMMAND( CALL_LOOP               (0, PTYPE_FUNC_LVL)),
LVL_COMMAND( SET_REG                 (0)),
LVL_COMMAND( PUSH_POOL               ()),
LVL_COMMAND( POP_POOL                ()),
LVL_COMMAND( FIXED_LOAD              (0, 0, 0)),
LVL_COMMAND( LOAD_RAW                (0, 0, 0)),
LVL_COMMAND( LOAD_MIO0               (0, 0, 0)),
LVL_COMMAND( LOAD_MARIO_HEAD         (0)),
LVL_COMMAND( LOAD_MIO0_TEXTURE       (0, 0, 0)),
LVL_COMMAND( INIT_LEVEL              ()),
LVL_COMMAND( CLEAR_LEVEL             ()),
LVL_COMMAND( ALLOC_LEVEL_POOL        ()),
LVL_COMMAND( FREE_LEVEL_POOL         ()),
LVL_COMMAND( AREA                    (0, PTYPE_PNTR_GEO)),
LVL_COMMAND( END_AREA                ()),
LVL_COMMAND( LOAD_MODEL_FROM_DL      (0, 0, 0)),
LVL_COMMAND( LOAD_MODEL_FROM_GEO     (0, PTYPE_PNTR_GEO)),
LVL_COMMAND( CMD23                   (0, PTYPE_PNTR_GEO, 0)),
LVL_COMMAND( OBJECT_WITH_ACTS        (0, 0, 0, 0, 0, 0, 0, 0, PTYPE_PNTR_BHV, 0)),
LVL_COMMAND( MARIO                   (0, 0, PTYPE_PNTR_BHV)),
LVL_COMMAND( WARP_NODE               (0, 0, 0, 0, 0)),
LVL_COMMAND( PAINTING_WARP_NODE      (0, 0, 0, 0, 0)),
LVL_COMMAND( INSTANT_WARP            (0, 0, 0, 0, 0)),
LVL_COMMAND( LOAD_AREA               (0)),
LVL_COMMAND( CMD2A                   (0)),
LVL_COMMAND( MARIO_POS               (0, 0, 0, 0, 0)),
LVL_COMMAND( CMD2C                   ()),
LVL_COMMAND( CMD2D                   ()),
LVL_COMMAND( TERRAIN                 (PTYPE_PNTR_COL)),
LVL_COMMAND( ROOMS                   (PTYPE_PNTR_ROOM)),
LVL_COMMAND( SHOW_DIALOG             (0, 0)),
LVL_COMMAND( TERRAIN_TYPE            (0)),
LVL_COMMAND( NOP                     ()),
LVL_COMMAND( TRANSITION              (0, 0, 0, 0, 0)),
LVL_COMMAND( BLACKOUT                (0)),
LVL_COMMAND( GAMMA                   (0)),
LVL_COMMAND( SET_BACKGROUND_MUSIC    (0, 0)),
LVL_COMMAND( SET_MENU_MUSIC          (0)),
LVL_COMMAND( STOP_MUSIC              (0)),
LVL_COMMAND( MACRO_OBJECTS           (PTYPE_PNTR_MACRO)),
LVL_COMMAND( CMD3A                   (0, 0, 0, 0, 0)),
LVL_COMMAND( WHIRLPOOL               (0, 0, 0, 0, 0, 0)),
LVL_COMMAND( GET_OR_SET              (0, 0)),
LVL_COMMAND( ADV_DEMO                ()),
LVL_COMMAND( CLEAR_DEMO_PTR          ()),
LVL_COMMAND( OBJECT_WITH_ACTS_EXT    (0, 0, 0, 0, 0, 0, 0, 0, PTYPE_PNTR_BHV|PTYPE_LUAV, 0)),
LVL_COMMAND( OBJECT_WITH_ACTS_EXT2   (PTYPE_LUAV, 0, 0, 0, 0, 0, 0, 0, PTYPE_PNTR_BHV|PTYPE_LUAV, 0)),
LVL_COMMAND( LOAD_MODEL_FROM_GEO_EXT (0, PTYPE_LUAV)),
LVL_COMMAND( JUMP_AREA_EXT           (0, 0, PTYPE_PNTR_LVL)),
LVL_COMMAND( OBJECT_EXT_LUA_PARAMS   (0, PTYPE_LUAV, PTYPE_LUAV, PTYPE_LUAV, PTYPE_LUAV, PTYPE_LUAV, PTYPE_LUAV, PTYPE_LUAV, PTYPE_LUAV, PTYPE_PNTR_BHV|PTYPE_LUAV, 0)),
LVL_COMMAND( SHOW_DIALOG_EXT         (0, PTYPE_LUAV, PTYPE_LUAV)),
};

static u8 sCurCommandId = 0xFF;
static u8 sCurCommandIndex = 0;

void DynOS_Lvl_Validate_Begin() {
    sCurCommandId = 0xFF;
    sCurCommandIndex = 0;
}

bool DynOS_Lvl_Validate_GetPointerTypes(u32 aValue, u8 &outCommandId, u32 &outPtrTypes) {
    // figure out which command we're inside
    if (sCurCommandId == 0xFF || sCurCommandIndex >= sLevelScriptCommands[sCurCommandId].size / 4) {
        u8 id = (u8) aValue;

        // verify id
        if (sLevelScriptCommands.count(id) == 0) {
            outCommandId = sCurCommandId;
            return false;
        }

        // set current
        sCurCommandId = id;
        sCurCommandIndex = 0;
    }

    // figure out if we expect a pointer
    // index 0 contains the id and size, it's never a pointer
    if (sCurCommandIndex == 0) {
        outPtrTypes = 0;
    } else {
        outPtrTypes = sLevelScriptCommands[sCurCommandId].command[sCurCommandIndex];
    }

    // advance command index
    sCurCommandIndex++;

    outCommandId = sCurCommandId;
    return true;
}

static Array<u8> DynOS_Lvl_Validate_GetCommandIds(const DataNode<LevelScript> *aNode) {
    Array<u8> lvlCommandIds;
    for (s32 i = 0; i < aNode->mSize;) {
        u8 id = (u8) aNode->mData[i];
        if (sLevelScriptCommands.count(id) != 0) {
            lvlCommandIds.Add(id);
            i += sLevelScriptCommands[id].size / 4;
        } else {
            break;
        }
    }
    return lvlCommandIds;
}

bool DynOS_Lvl_Validate_CheckCommands(GfxData *aGfxData, const DataNode<LevelScript> *aNode) {
    Array<u8> lvlCommandIds = DynOS_Lvl_Validate_GetCommandIds(aNode);

    // Level script must have at least 1 command
    if (lvlCommandIds.Count() < 1) {
        PrintDataError("  ERROR: Validation failed for level %s: Not enough commands (%d).", aNode->mName.begin(), lvlCommandIds.Count());
        return false;
    }

    // Penultimate command cannot be a SKIP command
    static const Array<u8> sLvlSkipCommands = {
        LVL_COMMAND_ID(SKIP()),
        LVL_COMMAND_ID(SKIP_IF(0, 0)),
        LVL_COMMAND_ID(SKIP_NOP()),
    };
    if (lvlCommandIds.Count() >= 2 && sLvlSkipCommands.Find(lvlCommandIds[lvlCommandIds.Count() - 2]) != -1) {
        PrintDataError("  ERROR: Validation failed for level %s: Penultimate command of the script cannot be one of:\n    SKIP, SKIP_IF, SKIP_NOP", aNode->mName.begin());
        return false;
    }

    // Last command must be a terminating command
    static const Array<u8> sLvlEndCommands = {
        LVL_COMMAND_ID(EXIT()),
        LVL_COMMAND_ID(EXIT_AND_EXECUTE(0, 0, 0, 0)),
        LVL_COMMAND_ID(JUMP(0)),
        LVL_COMMAND_ID(RETURN()),
    };
    if (sLvlEndCommands.Find(lvlCommandIds[lvlCommandIds.Count() - 1]) == -1) {
        PrintDataError("  ERROR: Validation failed for level %s: Last command of the script must be one of:\n    EXIT, EXIT_AND_EXECUTE, JUMP, RETURN", aNode->mName.begin());
        return false;
    }

    return true;
}

u8 DynOS_Lvl_GetCommandSize(u8 aCmdType) {
    if (sLevelScriptCommands.count(aCmdType) != 0) {
        return sLevelScriptCommands[aCmdType].size;
    }
    return 0;
}
