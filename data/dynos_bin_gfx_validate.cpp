#include "dynos.cpp.h"
#include <map>
extern "C" {
#include "include/PR/gbi.h"
}

#define GFX_COMMAND(cmd, ptypes) { \
    (u8) cmd, (u32) ptypes \
}

static std::map<u8, u32> sGfxCommands = {
GFX_COMMAND( G_NOOP,             0),
GFX_COMMAND( G_SPNOOP,           0),
GFX_COMMAND( G_RDPFULLSYNC,      0),
GFX_COMMAND( G_RDPTILESYNC,      0),
GFX_COMMAND( G_RDPPIPESYNC,      0),
GFX_COMMAND( G_RDPLOADSYNC,      0),
// GFX_COMMAND( G_MTX,              PTYPE_PNTR_MTX),
GFX_COMMAND( G_POPMTX,           0),
GFX_COMMAND( G_MOVEMEM,          PTYPE_PNTR_LIGHT|PTYPE_PNTR_LIGHT0|PTYPE_PNTR_LIGHTT|PTYPE_PNTR_AMBIENTT),
GFX_COMMAND( G_MOVEWORD,         0),
GFX_COMMAND( G_COPYMEM,          0),
GFX_COMMAND( G_TEXTURE,          0),
GFX_COMMAND( G_VTX,              PTYPE_PNTR_VTX),
GFX_COMMAND( G_DL,               PTYPE_PNTR_GFX),
GFX_COMMAND( G_ENDDL,            0),
GFX_COMMAND( G_GEOMETRYMODE,     0),
GFX_COMMAND( G_TRI1,             0),
GFX_COMMAND( G_TRI2,             0),
GFX_COMMAND( G_SETOTHERMODE_L,   0),
GFX_COMMAND( G_SETOTHERMODE_H,   0),
GFX_COMMAND( G_SETTIMG,          PTYPE_PNTR_TEX),
GFX_COMMAND( G_LOADBLOCK,        0),
GFX_COMMAND( G_LOADTILE,         0),
GFX_COMMAND( G_SETTILE,          0),
GFX_COMMAND( G_SETTILESIZE,      0),
GFX_COMMAND( G_LOADTLUT,         0),
GFX_COMMAND( G_SETENVCOLOR,      0),
GFX_COMMAND( G_SETENVRGB,        0),
GFX_COMMAND( G_SETPRIMCOLOR,     0),
GFX_COMMAND( G_SETFOGCOLOR,      0),
GFX_COMMAND( G_SETFILLCOLOR,     0),
GFX_COMMAND( G_SETCOMBINE,       0),
GFX_COMMAND( G_TEXRECT,          0),
GFX_COMMAND( G_TEXRECTFLIP,      0),
GFX_COMMAND( G_FILLRECT,         0),
GFX_COMMAND( G_SETSCISSOR,       0),
// GFX_COMMAND( G_SETZIMG,          PTYPE_PNTR_???),
// GFX_COMMAND( G_SETCIMG,          PTYPE_PNTR_???),

// extended
GFX_COMMAND( G_TEXCLIP_DJUI,     0),
GFX_COMMAND( G_TEXOVERRIDE_DJUI, PTYPE_PNTR_TEX),
GFX_COMMAND( G_VTX_EXT,          PTYPE_PNTR_VTX),
GFX_COMMAND( G_TRI2_EXT,         0),
GFX_COMMAND( G_TEXADDR_DJUI,     0),
GFX_COMMAND( G_EXECUTE_DJUI,     0),
GFX_COMMAND( G_PPARTTOCOLOR,     0),
GFX_COMMAND( G_STATE_EXT,        0),

// unused/unimplemented
// GFX_COMMAND( G_RDPHALF_1,        0),
// GFX_COMMAND( G_RDPHALF_2,        0),
// GFX_COMMAND( G_LOAD_UCODE,       0),
// GFX_COMMAND( G_DMA_IO,           0),
// GFX_COMMAND( G_SPECIAL_1,        0),
// GFX_COMMAND( G_SPECIAL_2,        0),
// GFX_COMMAND( G_SPECIAL_3,        0),
// GFX_COMMAND( G_SETBLENDCOLOR,    0),
// GFX_COMMAND( G_RDPSETOTHERMODE,  0),
// GFX_COMMAND( G_SETPRIMDEPTH,     0),
// GFX_COMMAND( G_SETCONVERT,       0),
// GFX_COMMAND( G_SETKEYR,          0),
// GFX_COMMAND( G_SETKEYGB,         0),
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
