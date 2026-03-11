#include "pc/rom_assets.h"
// 0x0702FDD8 - 0x0702FDF0
static const Lights1 inside_castle_seg7_lights_0702FDD8 = gdSPDefLights1(
    0x5f, 0x5f, 0x5f,
    0xff, 0xff, 0xff, 0x28, 0x28, 0x28
);

// 0x0702FDF0 - 0x0702FE70
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_0702FDF0, 0x00396340, 232834, 0x0002fdf0, 128);

// 0x0702FE70 - 0x0702FF70
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_0702FE70, 0x00396340, 232834, 0x0002fe70, 240);

// 0x0702FF70 - 0x0702FFF0
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_0702FF70, 0x00396340, 232834, 0x0002ff60, 112);

// 0x0702FFF0 - 0x070300E0
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_0702FFF0, 0x00396340, 232834, 0x0002ffd0, 240);

// 0x070300E0 - 0x070301D0
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_070300E0, 0x00396340, 232834, 0x000300c0, 240);

// 0x070301D0 - 0x070302B0
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_070301D0, 0x00396340, 232834, 0x000301b0, 224);

// 0x070302B0 - 0x070303B0
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_070302B0, 0x00396340, 232834, 0x00030290, 256);

// 0x070303B0 - 0x07030490
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_070303B0, 0x00396340, 232834, 0x00030390, 224);

// 0x07030490 - 0x07030590
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030490, 0x00396340, 232834, 0x00030470, 256);

// 0x07030590 - 0x07030670
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030590, 0x00396340, 232834, 0x00030570, 240);

// 0x07030670 - 0x07030760
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030670, 0x00396340, 232834, 0x00030660, 224);

// 0x07030760 - 0x07030860
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030760, 0x00396340, 232834, 0x00030740, 224);

// 0x07030860 - 0x07030940
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030860, 0x00396340, 232834, 0x00030820, 240);

// 0x07030940 - 0x07030A40
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030940, 0x00396340, 232834, 0x00030910, 224);

// 0x07030A40 - 0x07030B30
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030A40, 0x00396340, 232834, 0x000309f0, 256);

// 0x07030B30 - 0x07030C20
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030B30, 0x00396340, 232834, 0x00030af0, 256);

// 0x07030C20 - 0x07030D20
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030C20, 0x00396340, 232834, 0x00030bf0, 224);

// 0x07030D20 - 0x07030E20
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030D20, 0x00396340, 232834, 0x00030cd0, 224);

// 0x07030E20 - 0x07030F10
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030E20, 0x00396340, 232834, 0x00030db0, 224);

// 0x07030F10 - 0x07030FF0
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030F10, 0x00396340, 232834, 0x00030e90, 224);

// 0x07030FF0 - 0x07031070
ROM_ASSET_LOAD_VTX(inside_castle_seg7_vertex_07030FF0, 0x00396340, 232834, 0x00030f70, 256);

// 0x07031070 - 0x070310D8
static const Gfx inside_castle_seg7_dl_07031070[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, inside_09004000),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPLight(&inside_castle_seg7_lights_0702FDD8.l, 1),
    gsSPLight(&inside_castle_seg7_lights_0702FDD8.a, 2),
    gsSPVertex(inside_castle_seg7_vertex_0702FDF0, 8, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  4, 0x0),
    gsSP2Triangles( 0,  5,  3, 0x0,  0,  2,  6, 0x0),
    gsSP2Triangles( 0,  4,  7, 0x0,  0,  6,  5, 0x0),
    gsSPEndDisplayList(),
};

// 0x070310D8 - 0x07031168
static const Gfx inside_castle_seg7_dl_070310D8[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, inside_09005000),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPVertex(inside_castle_seg7_vertex_0702FE70, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  6,  4, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(11, 14, 12, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_0702FF70, 7, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP1Triangle( 0,  6,  1, 0x0),
    gsSPEndDisplayList(),
};

// 0x07031168 - 0x07031588
const Gfx inside_castle_seg7_dl_07031168[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, inside_09003000),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPVertex(inside_castle_seg7_vertex_0702FFF0, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  8,  9, 0x0),
    gsSP2Triangles( 3, 10,  4, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(11, 13, 14, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_070300E0, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_070301D0, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  1,  3,  2, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 10, 12, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_070302B0, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_070303B0, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  9,  7, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 10, 12, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030490, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030590, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP2Triangles( 4, 12,  5, 0x0,  9, 13, 14, 0x0),
    gsSP1Triangle( 9, 14, 10, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030670, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  0,  2, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030760, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030860, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  9,  7, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 10, 12, 13, 0x0),
    gsSP1Triangle( 0,  2, 14, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030940, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0,  8, 13,  9, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030A40, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  9,  7, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030B30, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  9, 10, 11, 0x0),
    gsSP2Triangles(10, 12, 11, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030C20, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 4,  6,  5, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030D20, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0,  7,  9, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030E20, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030F10, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0,  4, 13,  5, 0x0),
    gsSPVertex(inside_castle_seg7_vertex_07030FF0, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  5,  7,  6, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0,  8, 11,  9, 0x0),
    gsSP2Triangles(12, 13, 14, 0x0, 12, 14, 15, 0x0),
    gsSPEndDisplayList(),
};

// 0x07031588 - 0x07031608
const Gfx inside_castle_seg7_dl_07031588[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
    gsSPClearGeometryMode(G_SHADING_SMOOTH),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPDisplayList(inside_castle_seg7_dl_07031070),
    gsSPDisplayList(inside_castle_seg7_dl_070310D8),
    gsSPDisplayList(inside_castle_seg7_dl_07031168),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPSetGeometryMode(G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};
