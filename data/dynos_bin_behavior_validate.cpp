#include "dynos.cpp.h"
#include <map>
extern "C" {
#include "include/behavior_commands.h"
}

struct BehaviorScriptCommand {
    u8 id;
    u8 size;
    BehaviorScript command[16];
};

#define BHV_COMMAND(cmd) { \
    (u8) (((BehaviorScript[]){ cmd })[0] >> 24), { \
        .id = (u8) (((BehaviorScript[]){ cmd })[0] >> 24), \
        .size = 4 * (u8) (sizeof((BehaviorScript[]){ cmd }) / sizeof(BehaviorScript)), \
        .command = { cmd } \
    } \
}

static std::map<u8, struct BehaviorScriptCommand> sBehaviorScriptCommands = {
BHV_COMMAND( BEGIN                      (0)),
BHV_COMMAND( DELAY                      (0)),
BHV_COMMAND( CALL                       (PTYPE_PNTR_BHV)),
BHV_COMMAND( RETURN                     ()),
BHV_COMMAND( GOTO                       (PTYPE_PNTR_BHV)),
BHV_COMMAND( BEGIN_REPEAT               (0)),
BHV_COMMAND( END_REPEAT                 ()),
BHV_COMMAND( END_REPEAT_CONTINUE        ()),
BHV_COMMAND( BEGIN_LOOP                 ()),
BHV_COMMAND( END_LOOP                   ()),
BHV_COMMAND( BREAK                      ()),
BHV_COMMAND( BREAK_UNUSED               ()),
BHV_COMMAND( CALL_NATIVE                (PTYPE_FUNC_BHV)),
BHV_COMMAND( ADD_FLOAT                  (0, 0)),
BHV_COMMAND( SET_FLOAT                  (0, 0)),
BHV_COMMAND( ADD_INT                    (0, 0)),
BHV_COMMAND( SET_INT                    (0, 0)),
BHV_COMMAND( OR_INT                     (0, 0)),
BHV_COMMAND( BIT_CLEAR                  (0, 0)),
BHV_COMMAND( SET_INT_RAND_RSHIFT        (0, 0, 0)),
BHV_COMMAND( SET_RANDOM_FLOAT           (0, 0, 0)),
BHV_COMMAND( SET_RANDOM_INT             (0, 0, 0)),
BHV_COMMAND( ADD_RANDOM_FLOAT           (0, 0, 0)),
BHV_COMMAND( ADD_INT_RAND_RSHIFT        (0, 0, 0)),
BHV_COMMAND( CMD_NOP_1                  (0)),
BHV_COMMAND( CMD_NOP_2                  (0)),
BHV_COMMAND( CMD_NOP_3                  (0)),
BHV_COMMAND( SET_MODEL                  (0)),
BHV_COMMAND( SPAWN_CHILD                (0, PTYPE_PNTR_BHV)),
BHV_COMMAND( DEACTIVATE                 ()),
BHV_COMMAND( DROP_TO_FLOOR              ()),
BHV_COMMAND( SUM_FLOAT                  (0, 0, 0)),
BHV_COMMAND( SUM_INT                    (0, 0, 0)),
BHV_COMMAND( BILLBOARD                  ()),
BHV_COMMAND( CYLBOARD                   ()),
BHV_COMMAND( HIDE                       ()),
BHV_COMMAND( SET_HITBOX                 (0, 0)),
BHV_COMMAND( CMD_NOP_4                  (0, 0)),
BHV_COMMAND( DELAY_VAR                  (0)),
BHV_COMMAND( BEGIN_REPEAT_UNUSED        (0)),
BHV_COMMAND( LOAD_ANIMATIONS            (0, PTYPE_PNTR_ANIM)),
BHV_COMMAND( ANIMATE                    (0)),
BHV_COMMAND( SPAWN_CHILD_WITH_PARAM     (0, 0, PTYPE_PNTR_BHV)),
BHV_COMMAND( LOAD_COLLISION_DATA        (PTYPE_PNTR_COL)),
BHV_COMMAND( SET_HITBOX_WITH_OFFSET     (0, 0, 0)),
BHV_COMMAND( SPAWN_OBJ                  (0, PTYPE_PNTR_BHV)),
BHV_COMMAND( SET_HOME                   ()),
BHV_COMMAND( SET_HURTBOX                (0, 0)),
BHV_COMMAND( SET_INTERACT_TYPE          (0)),
BHV_COMMAND( SET_OBJ_PHYSICS            (0, 0, 0, 0, 0, 0, 0, 0)),
BHV_COMMAND( SET_INTERACT_SUBTYPE       (0)),
BHV_COMMAND( SCALE                      (0, 0)),
BHV_COMMAND( PARENT_BIT_CLEAR           (0, 0)),
BHV_COMMAND( ANIMATE_TEXTURE            (0, 0)),
BHV_COMMAND( DISABLE_RENDERING          ()),
BHV_COMMAND( SET_INT_UNUSED             (0, 0)),
// BHV_COMMAND( SPAWN_WATER_DROPLET        (PTYPE_PNTR_???)),
BHV_COMMAND( ID                         (0)),
BHV_COMMAND( CALL_EXT                   (PTYPE_LUAV)),
BHV_COMMAND( GOTO_EXT                   (PTYPE_LUAV)),
BHV_COMMAND( CALL_NATIVE_EXT            (PTYPE_LUAV)),
BHV_COMMAND( SPAWN_CHILD_EXT            (0, PTYPE_LUAV)),
BHV_COMMAND( SPAWN_CHILD_WITH_PARAM_EXT (0, 0, PTYPE_LUAV)),
BHV_COMMAND( SPAWN_OBJ_EXT              (0, PTYPE_LUAV)),
BHV_COMMAND( LOAD_ANIMATIONS_EXT        (0, PTYPE_LUAV)),
BHV_COMMAND( LOAD_COLLISION_DATA_EXT    (PTYPE_LUAV)),
BHV_COMMAND( CALL_LUA_FUNC              (PTYPE_LUAV)),
};

static u8 sCurCommandId = 0xFF;
static u8 sCurCommandIndex = 0;

void DynOS_Bhv_Validate_Begin() {
    sCurCommandId = 0xFF;
    sCurCommandIndex = 0;
}

bool DynOS_Bhv_Validate_GetPointerTypes(u32 aValue, u32 &outPtrTypes) {
    // figure out which command we're inside
    if (sCurCommandId == 0xFF || sCurCommandIndex >= sBehaviorScriptCommands[sCurCommandId].size / 4) {
        u8 id = (u8) (aValue >> 24);

        // verify id
        if (sBehaviorScriptCommands.count(id) == 0) {
            return false;
        }

        // set current
        sCurCommandId = id;
        sCurCommandIndex = 0;
    }

    // figure out if we expect a pointer
    // index 0 contains the id, it's never a pointer
    if (sCurCommandIndex == 0) {
        outPtrTypes = 0;
    } else {
        outPtrTypes = sBehaviorScriptCommands[sCurCommandId].command[sCurCommandIndex];
    }

    // advance command index
    sCurCommandIndex++;

    return true;
}

static bool DynOS_Bhv_Validate_CheckCommand(const BehaviorScript *aBhv, const Array<BehaviorScript> &aCommands) {
    u8 bhvCommand = (*aBhv >> 24) & 0xFF;
    for (const auto &commandToCheck : aCommands) {
        if (bhvCommand == ((commandToCheck >> 24) & 0xFF)) {
            return true;
        }
    }
    return false;
}

bool DynOS_Bhv_Validate_CheckCommands(GfxData *aGfxData, const DataNode<BehaviorScript> *aNode) {

    // Behavior must have at least 2 commands
    if (aNode->mSize < 2) {
        PrintDataError("  ERROR: Validation failed for behavior %s: Not enough commands (%d).", aNode->mName.begin(), aNode->mSize);
        return false;
    }

    // 1st command must be BEGIN
    if (!DynOS_Bhv_Validate_CheckCommand(aNode->mData + 0, { BEGIN(0) })) {
        PrintDataError("  ERROR: Validation failed for behavior %s: First command of the script must be BEGIN.", aNode->mName.begin());
        return false;
    }

    // 2nd command must be ID
    if (!DynOS_Bhv_Validate_CheckCommand(aNode->mData + 1, { ID(0) })) {
        PrintDataError("  ERROR: Validation failed for behavior %s: Second command of the script must be ID.", aNode->mName.begin());
        return false;
    }

    // Last command must be a terminating command
    if (!DynOS_Bhv_Validate_CheckCommand(aNode->mData + aNode->mSize - 1, {
        CALL(0),
        RETURN(),
        GOTO(0),
        END_LOOP(),
        BREAK(),
        DEACTIVATE(),
        CALL_EXT(0),
        GOTO_EXT(0),
    })) {
        PrintDataError("  ERROR: Validation failed for behavior %s: Last command of the script must be one of:\n    CALL, RETURN, GOTO, END_LOOP, BREAK, DEACTIVATE", aNode->mName.begin());
        return false;
    }

    return true;
}
