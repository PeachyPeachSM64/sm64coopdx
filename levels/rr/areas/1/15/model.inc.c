#include "pc/rom_assets.h"

// 0x070127E8 - 0x070128D8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070127E8, 0x00437870, 75979, 0x000127e8, 240);

// 0x070128D8 - 0x070129B8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070128D8, 0x00437870, 75979, 0x000128d8, 224);

// 0x070129B8 - 0x07012AA8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070129B8, 0x00437870, 75979, 0x000129b8, 240);

// 0x07012AA8 - 0x07012B98
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07012AA8, 0x00437870, 75979, 0x00012aa8, 240);

// 0x07012B98 - 0x07012C98
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07012B98, 0x00437870, 75979, 0x00012b98, 256);

// 0x07012C98 - 0x07012D78
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07012C98, 0x00437870, 75979, 0x00012c98, 224);

// 0x07012D78 - 0x07012E68
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07012D78, 0x00437870, 75979, 0x00012d78, 240);

// 0x07012E68 - 0x07012F58
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07012E68, 0x00437870, 75979, 0x00012e68, 240);

// 0x07012F58 - 0x07013038
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07012F58, 0x00437870, 75979, 0x00012f58, 224);

// 0x07013038 - 0x07013128
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013038, 0x00437870, 75979, 0x00013038, 240);

// 0x07013128 - 0x07013208
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013128, 0x00437870, 75979, 0x00013128, 224);

// 0x07013208 - 0x070132F8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013208, 0x00437870, 75979, 0x00013208, 240);

// 0x070132F8 - 0x070133E8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070132F8, 0x00437870, 75979, 0x000132f8, 240);

// 0x070133E8 - 0x070134C8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070133E8, 0x00437870, 75979, 0x000133e8, 224);

// 0x070134C8 - 0x070135C8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070134C8, 0x00437870, 75979, 0x000134c8, 256);

// 0x070135C8 - 0x070136A8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070135C8, 0x00437870, 75979, 0x000135c8, 224);

// 0x070136A8 - 0x07013728
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070136A8, 0x00437870, 75979, 0x000136a8, 128);

// 0x07013728 - 0x07013828
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013728, 0x00437870, 75979, 0x00013728, 256);

// 0x07013828 - 0x070138A8
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013828, 0x00437870, 75979, 0x00013828, 128);

// 0x070138A8 - 0x07013998
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_070138A8, 0x00437870, 75979, 0x000138a8, 240);

// 0x07013998 - 0x07013A88
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013998, 0x00437870, 75979, 0x00013998, 240);

// 0x07013A88 - 0x07013B88
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013A88, 0x00437870, 75979, 0x00013a88, 256);

// 0x07013B88 - 0x07013C88
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013B88, 0x00437870, 75979, 0x00013b88, 256);

// 0x07013C88 - 0x07013D78
ROM_ASSET_LOAD_VTX(rr_seg7_vertex_07013C88, 0x00437870, 75979, 0x00013c88, 96);

// 0x07013E68 - 0x070142C0
static const Gfx rr_seg7_dl_07013E68[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, sky_09007000),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPVertex(rr_seg7_vertex_070127E8, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(11, 14, 12, 0x0),
    gsSPVertex(rr_seg7_vertex_070128D8, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 10, 13, 11, 0x0),
    gsSPVertex(rr_seg7_vertex_070129B8, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(12, 14, 13, 0x0),
    gsSPVertex(rr_seg7_vertex_07012AA8, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(11, 13, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_07012B98, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 10,  2, 11, 0x0),
    gsSP2Triangles( 0,  2, 10, 0x0, 13, 14, 15, 0x0),
    gsSPVertex(rr_seg7_vertex_07012C98, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  6,  4, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(rr_seg7_vertex_07012D78, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0,  8, 10, 11, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_07012E68, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  7,  5, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(11, 13, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_07012F58, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(rr_seg7_vertex_07013038, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  7,  5, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0,  8, 11,  9, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_07013128, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSPVertex(rr_seg7_vertex_07013208, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12, 13, 0x0),
    gsSP1Triangle(11, 14, 12, 0x0),
    gsSPVertex(rr_seg7_vertex_070132F8, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  8,  9, 0x0),
    gsSP2Triangles(10, 11, 12, 0x0, 10, 13, 11, 0x0),
    gsSP1Triangle( 0,  2, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_070133E8, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7, 10,  8, 0x0, 11, 12,  0, 0x0),
    gsSP2Triangles(11,  0,  2, 0x0,  0, 13,  1, 0x0),
    gsSPVertex(rr_seg7_vertex_070134C8, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  7,  5, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0,  8, 11,  9, 0x0),
    gsSP2Triangles(12, 13, 14, 0x0, 12, 15, 13, 0x0),
    gsSPVertex(rr_seg7_vertex_070135C8, 14, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 3,  4,  0, 0x0,  3,  5,  6, 0x0),
    gsSP2Triangles( 3,  6,  4, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 9, 10,  7, 0x0,  9, 11, 10, 0x0),
    gsSP2Triangles( 7, 12, 13, 0x0,  7, 13,  8, 0x0),
    gsSPVertex(rr_seg7_vertex_070136A8, 8, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSPEndDisplayList(),
};

// 0x070142C0 - 0x07014350
static const Gfx rr_seg7_dl_070142C0[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, sky_09000800),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPVertex(rr_seg7_vertex_07013728, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  2,  3, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0,  8, 10, 11, 0x0),
    gsSP2Triangles(12, 13, 14, 0x0, 12, 14, 15, 0x0),
    gsSPVertex(rr_seg7_vertex_07013828, 8, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSPEndDisplayList(),
};

// 0x07014350 - 0x07014490
static const Gfx rr_seg7_dl_07014350[] = {
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, sky_09008000),
    gsDPLoadSync(),
    gsDPLoadBlock(G_TX_LOADTILE, 0, 0, 32 * 32 - 1, CALC_DXT(32, G_IM_SIZ_16b_BYTES)),
    gsSPVertex(rr_seg7_vertex_070138A8, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 0,  4,  3, 0x0,  0,  2,  5, 0x0),
    gsSP2Triangles( 6,  7,  8, 0x0,  6,  8,  9, 0x0),
    gsSP2Triangles( 6,  9, 10, 0x0,  6, 11,  7, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_07013998, 15, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 4,  5,  6, 0x0,  4,  6,  7, 0x0),
    gsSP2Triangles( 8,  9, 10, 0x0,  8, 11,  9, 0x0),
    gsSP1Triangle(12, 13, 14, 0x0),
    gsSPVertex(rr_seg7_vertex_07013A88, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  6,  4, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSP2Triangles(11, 13, 14, 0x0,  0, 15,  1, 0x0),
    gsSPVertex(rr_seg7_vertex_07013B88, 16, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  3,  4,  5, 0x0),
    gsSP2Triangles( 3,  5,  6, 0x0,  7,  8,  9, 0x0),
    gsSP2Triangles( 7,  9, 10, 0x0, 11, 12, 13, 0x0),
    gsSP2Triangles(11, 14, 12, 0x0,  0, 15,  1, 0x0),
    gsSPEndDisplayList(),
};

// 0x07014490 - 0x07014508
static const Gfx rr_seg7_dl_07014490[] = {
    gsSPVertex(rr_seg7_vertex_07013C88, 6, 0),
    gsSP2Triangles( 0,  1,  2, 0x0,  0,  3,  1, 0x0),
    gsSP2Triangles( 0,  4,  3, 0x0,  0,  5,  4, 0x0),
    gsSPEndDisplayList(),
};

// 0x07014508 - 0x07014590
const Gfx rr_seg7_dl_07014508[] = {
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsDPTileSync(),
    gsDPSetTile(G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, 5, G_TX_NOLOD),
    gsDPSetTileSize(0, 0, 0, (32 - 1) << G_TEXTURE_IMAGE_FRAC, (32 - 1) << G_TEXTURE_IMAGE_FRAC),
    gsSPDisplayList(rr_seg7_dl_07013E68),
    gsSPDisplayList(rr_seg7_dl_070142C0),
    gsSPDisplayList(rr_seg7_dl_07014350),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF),
    gsDPPipeSync(),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPDisplayList(rr_seg7_dl_07014490),
    gsSPSetGeometryMode(G_LIGHTING),
    gsSPEndDisplayList(),
};
