#include "dynos.cpp.h"
#include <map>
extern "C" {
#include "include/PR/gbi.h"
}

static std::map<u8, u32> sGfxCommands = {
#define GFX_OPCODE(_symb_, _ptrTypes_) { (u8) _symb_, (u32) _ptrTypes_ },
#include "dynos_bin_gfx_opcodes.inl"
#undef GFX_OPCODE
};

bool DynOS_Gfx_Validate_GetPointerTypes(u32 aWordsW0, u8 &outCommandId, u32 &outPtrTypes) {
    outCommandId = (u8) (aWordsW0 >> 24);

    // verify id
    if (sGfxCommands.count(outCommandId) == 0) {
        return false;
    }

    outPtrTypes = sGfxCommands[outCommandId];

    return true;
}

bool DynOS_Gfx_Validate_CheckCommands(GfxData *aGfxData, const DataNode<Gfx> *aNode) {

    // Display list must have at least 1 command
    if (aNode->mSize < 1) {
        PrintDataError("  ERROR: Validation failed for display list %s: Not enough commands (%d).", aNode->mName.begin(), aNode->mSize);
        return false;
    }

    // Last command must be a terminating command
    static const Array<uintptr_t> sGfxEndCommands = {
        Gfx(gsSPBranchList(0)).words.w0,
        Gfx(gsSPEndDisplayList()).words.w0,
    };
    if (sGfxEndCommands.Find(aNode->mData[aNode->mSize - 1].words.w0) == -1) {
        PrintDataError("  ERROR: Validation failed for display list %s: Last command must be one of:\n    gsSPBranchList, gsSPEndDisplayList", aNode->mName.begin());
        return false;
    }

    return true;
}
