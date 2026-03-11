#include <ultra64.h>
#include <string.h>

#include "actors/common1.h"
#include "area.h"
#include "audio/external.h"
#include "behavior_data.h"
#include "camera.h"
#include "course_table.h"
#include "dialog_ids.h"
#include "engine/math_util.h"
#include "game_init.h"
#include "gfx_dimensions.h"
#include "ingame_menu.h"
#include "level_update.h"
#include "levels/castle_grounds/header.h"
#include "memory.h"
#include "object_helpers.h"
#include "print.h"
#include "save_file.h"
#include "segment2.h"
#include "segment7.h"
#include "seq_ids.h"
#include "sm64.h"
#include "types.h"
#include "macros.h"
#include "hardcoded.h"
#include "pc/network/network.h"
#include "pc/djui/djui.h"
#include "pc/djui/djui_panel.h"
#include "pc/djui/djui_panel_pause.h"
#include "pc/utils/misc.h"
#include "sound_init.h"
#include "spawn_sound.h"
#include "pc/mods/mods.h"
#include "data/dynos_mgr_builtin_externs.h"
#include "hud.h"
#include "pc/lua/smlua_hooks.h"
#include "game/camera.h"
#include "level_info.h"
#include "pc/lua/utils/smlua_text_utils.h"
#include "menu/ingame_text.h"
#include "pc/dialog_table.h"
#include "object_list_processor.h"

u16 gDialogColorFadeTimer;
s8 gLastDialogLineNum;
s32 gDialogVariable;
u16 gDialogTextAlpha;
s16 gCutsceneMsgXOffset;
s16 gCutsceneMsgYOffset;
s16 gDialogMinWidth = 0;
s16 gDialogOverrideX = 0;
s16 gDialogOverrideY = 0;
u8 gOverrideDialogPos = 0;
u8 gOverrideDialogColor = 0;
u8 gDialogBgColorR = 0;
u8 gDialogBgColorG = 0;
u8 gDialogBgColorB = 0;
u8 gDialogBgColorA = 0;
u8 gDialogTextColorR = 0;
u8 gDialogTextColorG = 0;
u8 gDialogTextColorB = 0;
u8 gDialogTextColorA = 0;

extern u8 gLastCompletedCourseNum;
extern u8 gLastCompletedStarNum;
u8 gLastCollectedStarOrKey = 0;

enum DialogBoxState {
    DIALOG_STATE_OPENING,
    DIALOG_STATE_VERTICAL,
    DIALOG_STATE_HORIZONTAL,
    DIALOG_STATE_CLOSING
};

enum DialogBoxPageState {
    DIALOG_PAGE_STATE_NONE,
    DIALOG_PAGE_STATE_SCROLL,
    DIALOG_PAGE_STATE_END
};

enum DialogBoxType {
    DIALOG_TYPE_ROTATE, // used in NPCs and level messages
    DIALOG_TYPE_ZOOM    // used in signposts and wall signs and etc
};

enum DialogMark { DIALOG_MARK_NONE = 0, DIALOG_MARK_DAKUTEN = 1, DIALOG_MARK_HANDAKUTEN = 2 };

#define DEFAULT_DIALOG_BOX_ANGLE 90.0f
#define DEFAULT_DIALOG_BOX_SCALE 19.0f

u8 gDialogCharWidths[256] = { // TODO: Is there a way to auto generate this?
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,
    6,  6,  5,  6,  6,  5,  8,  8,  6,  6,  6,  6,  6,  5,  6,  6,
    8,  7,  6,  6,  6,  5,  5,  6,  5,  5,  6,  5,  4,  5,  5,  3,
    7,  5,  5,  5,  6,  5,  5,  5,  5,  5,  7,  7,  5,  5,  4,  4,
    8,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    8,  8,  8,  8,  7,  7,  6,  7,  7,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  6,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    7,  5, 10,  5,  9,  8,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  5,  7,  7,  6,  6,  8,  0,  8, 10,  6,  4, 10,  0,  0
};


s8 gDialogBoxState = DIALOG_STATE_OPENING;
f32 gDialogBoxOpenTimer = DEFAULT_DIALOG_BOX_ANGLE;
f32 gDialogBoxScale = DEFAULT_DIALOG_BOX_SCALE;
s16 gDialogScrollOffsetY = 0;
s8 gDialogBoxType = DIALOG_TYPE_ROTATE;
s32 gDialogID = DIALOG_NONE;
s16 gLastDialogPageStrPos = 0;
s16 gDialogTextPos = 0;
s8 gDialogLineNum = 1;
s8 gLastDialogResponse = 0;
u8 gMenuHoldKeyIndex = 0;
u8 gMenuHoldKeyTimer = 0;
s32 gDialogResponse = 0;

bool gPauseMenuHidden = false;

static Gfx *sDialogOffsetPos;
static Gfx *sDialogRotationPos;
static Gfx *sDialogZoomPos;

static f32 sDialogOffset;
static f32 sDialogOffsetPrev;
static f32 sDialogScale;
static f32 sDialogScalePrev;
static f32 sDialogRotation;
static f32 sDialogRotationPrev;

void patch_dialog_before(void) {
    sDialogOffsetPos   = NULL;
    sDialogRotationPos = NULL;
    sDialogZoomPos     = NULL;
}

void patch_dialog_interpolated(f32 delta) {
    Mtx *matrix;

    if (sDialogOffsetPos != NULL) {
        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (matrix == NULL) { return; }
        f32 interpOffset = delta_interpolate_f32(sDialogOffsetPrev, sDialogOffset, delta);
        guTranslate(matrix, 0, interpOffset, 0);
        gSPMatrix(sDialogOffsetPos, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    }

    if (sDialogRotationPos != NULL) {
        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (matrix == NULL) { return; }

        f32 interpScale = delta_interpolate_f32(sDialogScalePrev, sDialogScale, delta);
        guScale(matrix, 1.0 / interpScale, 1.0 / interpScale, 1.0f);
        gSPMatrix(sDialogRotationPos, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (matrix == NULL) { return; }

        f32 interpRotation = delta_interpolate_f32(sDialogRotationPrev, sDialogRotation, delta);
        guRotate(matrix, interpRotation * 4.0f, 0, 0, 1.0f);
        gSPMatrix((sDialogRotationPos + 1), VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    }

    if (sDialogZoomPos != NULL) {
        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (matrix == NULL) { return; }

        f32 interpScale = delta_interpolate_f32(sDialogScalePrev, sDialogScale, delta);
        guTranslate(matrix, 65.0 - (65.0 / interpScale), (40.0 / interpScale) - 40, 0);
        gSPMatrix(sDialogZoomPos, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

        matrix = (Mtx *) alloc_display_list(sizeof(Mtx));
        if (matrix == NULL) { return; }

        guScale(matrix, 1.0 / interpScale, 1.0 / interpScale, 1.0f);
        gSPMatrix((sDialogZoomPos + 1), VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    }
}

void create_dl_identity_matrix(void) {
    Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));

    if (matrix == NULL) {
        return;
    }

    guMtxIdent(matrix);

    gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
}

static void pause_capitalize_string_sm64(u8 *str64) {
    if (!str64) { return; }
    for (; *str64 != DIALOG_CHAR_TERMINATOR; str64++) {
        if (*str64 >= 0x24 && *str64 <= 0x3D) {
            *str64 -= 26;
        }
    }
}

void create_dl_translation_matrix(s8 pushOp, f32 x, f32 y, f32 z) {
    Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));

    if (matrix == NULL) {
        return;
    }

    guTranslate(matrix, x, y, z);

    if (pushOp == MENU_MTX_PUSH)
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    if (pushOp == MENU_MTX_NOPUSH)
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

void create_dl_rotation_matrix(s8 pushOp, f32 a, f32 x, f32 y, f32 z) {
    Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));

    if (matrix == NULL) {
        return;
    }

    guRotate(matrix, a, x, y, z);

    if (pushOp == MENU_MTX_PUSH)
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    if (pushOp == MENU_MTX_NOPUSH)
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

void create_dl_scale_matrix(s8 pushOp, f32 x, f32 y, f32 z) {
    Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));

    if (matrix == NULL) {
        return;
    }

    guScale(matrix, x, y, z);

    if (pushOp == MENU_MTX_PUSH)
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);

    if (pushOp == MENU_MTX_NOPUSH)
        gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
}

void create_dl_ortho_matrix(void) {
    Mtx *matrix = (Mtx *) alloc_display_list(sizeof(Mtx));

    if (matrix == NULL) {
        return;
    }

    create_dl_identity_matrix();

    guOrtho(matrix, 0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -10.0f, 10.0f, 1.0f);

    // Should produce G_RDPHALF_1 in Fast3D
    gSPPerspNormalize(gDisplayListHead++, 0xFFFF);

    gSPMatrix(gDisplayListHead++, VIRTUAL_TO_PHYSICAL(matrix), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH)
}

void render_generic_char(u8 c) {
    void **fontLUT;
    void *packedTexture;

    fontLUT = segmented_to_virtual(main_font_lut);
    packedTexture = segmented_to_virtual(fontLUT[c]);

    gDPPipeSync(gDisplayListHead++);
    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_16b, 1, VIRTUAL_TO_PHYSICAL(packedTexture));
    gSPDisplayList(gDisplayListHead++, dl_ia_text_tex_settings);
}

struct MultiTextEntry {
    u8 length;
    u8 str[4];
};

#define TEXT_THE_RAW ASCII_TO_DIALOG('t'), ASCII_TO_DIALOG('h'), ASCII_TO_DIALOG('e'), 0x00
#define TEXT_YOU_RAW ASCII_TO_DIALOG('y'), ASCII_TO_DIALOG('o'), ASCII_TO_DIALOG('u'), 0x00

enum MultiStringIDs { STRING_THE, STRING_YOU };

/*
 * Place the multi-text string according to the ID passed. (US, EU)
 * 0: 'the'
 * 1: 'you'
 */
void render_multi_text_string(s8 multiTextID)
{
    s8 i;
    struct MultiTextEntry textLengths[2] = {
        { 3, { TEXT_THE_RAW } },
        { 3, { TEXT_YOU_RAW } },
    };

    for (i = 0; i < textLengths[multiTextID].length; i++) {
        render_generic_char(textLengths[multiTextID].str[i]);
        create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[textLengths[multiTextID].str[i]]), 0.0f, 0.0f);
    }
}

f32 get_generic_dialog_width(u8* dialog) {
    f32 largestWidth = 0;
    f32 width = 0;
    u8* d = dialog;
    while (*d != DIALOG_CHAR_TERMINATOR) {
        if (*d == DIALOG_CHAR_NEWLINE) {
            width = 0;
            d++;
            continue;
        }
        width += (f32)(gDialogCharWidths[*d]);
        largestWidth = MAX(width, largestWidth);
        d++;
    }
    return largestWidth;
}

f32 get_generic_ascii_string_width(const char* ascii) {
    u8 *str = convert_string_ascii_to_sm64(NULL, ascii, false);
    f32 width = get_generic_dialog_width(str);
    free(str);
    return width;
}

f32 get_generic_dialog_height(u8* dialog) {
    s32 lines = 0;
    u8* d = dialog;
    while (*d != DIALOG_CHAR_TERMINATOR) {
        if (*d == DIALOG_CHAR_NEWLINE) { lines++; }
        d++;
    }
    return lines * 14;
}

f32 get_generic_ascii_string_height(const char* ascii) {
    u8 *str = convert_string_ascii_to_sm64(NULL, ascii, false);
    f32 height = get_generic_dialog_height(str);
    free(str);
    return height;
}

void print_generic_ascii_string(s16 x, s16 y, const char* ascii) {
    u8 *str = convert_string_ascii_to_sm64(NULL, ascii, false);
    print_generic_string(x, y, str);
    free(str);
}

#define MAX_STRING_WIDTH 16

/**
 * Prints a generic white string.
 * In JP/EU a IA1 texture is used but in US a IA4 texture is used.
 */
void print_generic_string(s16 x, s16 y, const u8 *str) {
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in EU
    s32 strPos = 0;
    u8 lineNum = 1;
    create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0.0f);

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        switch (str[strPos]) {
            case DIALOG_CHAR_DAKUTEN:
                mark = DIALOG_MARK_DAKUTEN;
                break;
            case DIALOG_CHAR_PERIOD_OR_HANDAKUTEN:
                mark = DIALOG_MARK_HANDAKUTEN;
                break;
            case DIALOG_CHAR_NEWLINE:
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                create_dl_translation_matrix(MENU_MTX_PUSH, x, y - (lineNum * MAX_STRING_WIDTH), 0.0f);
                lineNum++;
                break;
            case DIALOG_CHAR_PERIOD:
                create_dl_translation_matrix(MENU_MTX_PUSH, -2.0f, -5.0f, 0.0f);
                render_generic_char(DIALOG_CHAR_PERIOD_OR_HANDAKUTEN);
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                break;
            case DIALOG_CHAR_SLASH:
                create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * 2), 0.0f, 0.0f);
                break;
            case DIALOG_CHAR_MULTI_THE:
                render_multi_text_string(STRING_THE);
                break;
            case DIALOG_CHAR_MULTI_YOU:
                render_multi_text_string(STRING_YOU);
                break;
            case DIALOG_CHAR_SPACE:
                create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE]), 0.0f, 0.0f);
                break; // ? needed to match
            default:
                render_generic_char(str[strPos]);
                if (mark != DIALOG_MARK_NONE) {
                    create_dl_translation_matrix(MENU_MTX_PUSH, 5.0f, 5.0f, 0.0f);
                    render_generic_char(mark + 0xEF);
                    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                    mark = DIALOG_MARK_NONE;
                }
                create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[str[strPos]]), 0.0f, 0.0f);
                break; // what an odd difference. US added a useless break here.
        }

        strPos++;
    }

    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

/**
 * Prints a hud string depending of the hud table list defined.
 */
void print_hud_lut_string(s8 hudLUT, s16 x, s16 y, const u8 *str) {
    s32 strPos = 0;
    void **hudLUT1 = segmented_to_virtual(menu_hud_lut); // Japanese Menu HUD Color font
    void **hudLUT2 = segmented_to_virtual(main_hud_lut); // 0-9 A-Z HUD Color Font
    u32 curX = x;
    u32 curY = y;

    u32 xStride; // X separation

    if (hudLUT == HUD_LUT_JPMENU) {
        xStride = 16;
    } else { // HUD_LUT_GLOBAL
        xStride = 12; //? Shindou uses this.
    }

    while (str[strPos] != GLOBAR_CHAR_TERMINATOR) {
        switch (str[strPos]) {
            case GLOBAL_CHAR_SPACE:
                curX += 8;
                break;
            default:
                gDPPipeSync(gDisplayListHead++);

                if (hudLUT == HUD_LUT_JPMENU) {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, hudLUT1[str[strPos]]);
                }

                if (hudLUT == HUD_LUT_GLOBAL) {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, hudLUT2[str[strPos]]);
                }

                gSPDisplayList(gDisplayListHead++, dl_rgba16_load_tex_block);
                gSPTextureRectangle(gDisplayListHead++, curX << 2, curY << 2,
                                    (curX + 16) << 2, (curY + 16) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

                curX += xStride;
        }
        strPos++;
    }
}

void print_menu_generic_string(s16 x, s16 y, const u8 *str) {
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in EU
    s32 strPos = 0;
    u32 curX = x;
    u32 curY = y;
    void **fontLUT = segmented_to_virtual(menu_font_lut);

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        switch (str[strPos]) {
            case DIALOG_CHAR_DAKUTEN:
                mark = DIALOG_MARK_DAKUTEN;
                break;
            case DIALOG_CHAR_PERIOD_OR_HANDAKUTEN:
                mark = DIALOG_MARK_HANDAKUTEN;
                break;
            case DIALOG_CHAR_SPACE:
                curX += 4;
                break;
            default:
                gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, fontLUT[str[strPos]]);
                gDPLoadSync(gDisplayListHead++);
                gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES));
                gSPTextureRectangle(gDisplayListHead++, curX << 2, curY << 2, (curX + 8) << 2,
                                    (curY + 8) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

                if (mark != DIALOG_MARK_NONE) {
                    gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_IA, G_IM_SIZ_8b, 1, fontLUT[mark + 0xEF]);
                    gDPLoadSync(gDisplayListHead++);
                    gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_8b_BYTES));
                    gSPTextureRectangle(gDisplayListHead++, (curX + 6) << 2, (curY - 7) << 2,
                                        (curX + 6 + 8) << 2, (curY - 7 + 8) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

                    mark = DIALOG_MARK_NONE;
                }
                curX += gDialogCharWidths[str[strPos]];
        }
        strPos++;
    }
}

void print_credits_string(s16 x, s16 y, const u8 *str) {
    s32 strPos = 0;
    void **fontLUT = segmented_to_virtual(main_credits_font_lut);
    u32 curX = x;
    u32 curY = y;

    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0,
                G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD, G_TX_WRAP | G_TX_NOMIRROR, G_TX_NOMASK, G_TX_NOLOD);
    gDPTileSync(gDisplayListHead++);
    gDPSetTile(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 2, 0, G_TX_RENDERTILE, 0,
                G_TX_CLAMP, 3, G_TX_NOLOD, G_TX_CLAMP, 3, G_TX_NOLOD);
    gDPSetTileSize(gDisplayListHead++, G_TX_RENDERTILE, 0, 0, (8 - 1) << G_TEXTURE_IMAGE_FRAC, (8 - 1) << G_TEXTURE_IMAGE_FRAC);

    while (str[strPos] != GLOBAR_CHAR_TERMINATOR) {
        switch (str[strPos]) {
            case GLOBAL_CHAR_SPACE:
                curX += 4;
                break;
            default:
                gDPPipeSync(gDisplayListHead++);
                gDPSetTextureImage(gDisplayListHead++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, fontLUT[str[strPos]]);
                gDPLoadSync(gDisplayListHead++);
                gDPLoadBlock(gDisplayListHead++, G_TX_LOADTILE, 0, 0, 8 * 8 - 1, CALC_DXT(8, G_IM_SIZ_16b_BYTES));
                gSPTextureRectangle(gDisplayListHead++, curX << 2, curY << 2, (curX + 8) << 2,
                                    (curY + 8) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
                curX += 7;
                break;
        }
        strPos++;
    }
}

void handle_menu_scrolling(s8 scrollDirection, s8 *currentIndex, s8 minIndex, s8 maxIndex) {
    u8 index = 0;

    if (scrollDirection == MENU_SCROLL_VERTICAL) {
        if (gPlayer1Controller->rawStickY > 60) {
            index++;
        }

        if (gPlayer1Controller->rawStickY < -60) {
            index += 2;
        }
    } else if (scrollDirection == MENU_SCROLL_HORIZONTAL) {
        if (gPlayer1Controller->rawStickX > 60) {
            index += 2;
        }

        if (gPlayer1Controller->rawStickX < -60) {
            index++;
        }
    }

    if (((index ^ gMenuHoldKeyIndex) & index) == 2) {
        if (currentIndex[0] == maxIndex) {
            //! Probably originally a >=, but later replaced with an == and an else statement.
            currentIndex[0] = maxIndex;
        } else {
            play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
            currentIndex[0]++;
        }
    }

    if (((index ^ gMenuHoldKeyIndex) & index) == 1) {
        if (currentIndex[0] == minIndex) {
            // Same applies to here as above
        } else {
            play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
            currentIndex[0]--;
        }
    }

    // Clamp currentIndex to prevent out of bounds access
    if (currentIndex[0] < minIndex) {
        currentIndex[0] = minIndex;
    }
    if (currentIndex[0] > maxIndex) {
        currentIndex[0] = maxIndex;
    }

    if (gMenuHoldKeyTimer == 10) {
        gMenuHoldKeyTimer = 8;
        gMenuHoldKeyIndex = 0;
    } else {
        gMenuHoldKeyTimer++;
        gMenuHoldKeyIndex = index;
    }

    if ((index & 3) == 0) {
        gMenuHoldKeyTimer = 0;
    }
}

s16 get_str_x_pos_from_center(s16 centerPos, u8 *str, UNUSED f32 scale) {
    s16 strPos = 0;
    f32 spacesWidth = 0.0f;

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        spacesWidth += gDialogCharWidths[str[strPos]];
        strPos++;
    }
    // return the x position of where the string starts as half the string's
    // length from the position of the provided center.
    return (s16)(centerPos - (s16)(spacesWidth / 2.0));
}
s16 get_string_width(u8 *str) {
    s16 strPos = 0;
    s16 width = 0;

    while (str[strPos] != DIALOG_CHAR_TERMINATOR) {
        width += gDialogCharWidths[str[strPos]];
        strPos++;
    }
    return width;
}

u8 gHudSymCoin[] = { GLYPH_COIN, GLYPH_SPACE };
u8 gHudSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

void print_hud_my_score_coins(s32 useCourseCoinScore, s8 fileNum, s8 courseNum, s16 x, s16 y) {
    u8 strNumCoins[4];
    s16 numCoins;

    if (!useCourseCoinScore) {
        numCoins = (u16)(save_file_get_max_coin_score(courseNum) & 0xFFFF);
    } else {
        numCoins = save_file_get_course_coin_score(fileNum, courseNum);
    }

    if (numCoins != 0) {
        print_hud_lut_string(HUD_LUT_GLOBAL, x, y, gHudSymCoin);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 16, y, gHudSymX);
        int_to_str(numCoins, strNumCoins);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 32, y, strNumCoins);
    }
}

void print_hud_my_score_stars(s8 fileNum, s8 courseNum, s16 x, s16 y) {
    u8 strStarCount[4];
    s16 starCount;
    u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
    UNUSED u16 unused;
    u8 textSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

    starCount = save_file_get_course_star_count(fileNum, courseNum);

    if (starCount != 0) {
        print_hud_lut_string(HUD_LUT_GLOBAL, x, y, textSymStar);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 16, y, textSymX);
        int_to_str(starCount, strStarCount);
        print_hud_lut_string(HUD_LUT_GLOBAL, x + 32, y, strStarCount);
    }
}

void int_to_str(s32 num, u8 *dst) {
    s32 digit1;
    s32 digit2;
    s32 digit3;

    s8 pos = 0;

    if (num > 999) {
        dst[0] = 0x00; dst[1] = DIALOG_CHAR_TERMINATOR;
        return;
    }

    digit1 = num / 100;
    digit2 = (num - digit1 * 100) / 10;
    digit3 = (num - digit1 * 100) - (digit2 * 10);

    if (digit1 != 0) {
        dst[pos++] = digit1;
    }

    if (digit2 != 0 || digit1 != 0) {
        dst[pos++] = digit2;
    }

    dst[pos++] = digit3;
    dst[pos] = DIALOG_CHAR_TERMINATOR;
}

s32 get_dialog_id(void) {
    return gDialogID;
}

void handle_special_dialog_text(s32 dialogID) { // dialog ID tables, in order
    // King Bob-omb (Start), Whomp (Start), King Bob-omb (throw him out), Eyerock (Start), Wiggler (Start)
    enum DialogId dialogBossStart[] = { DIALOG_017, DIALOG_114, DIALOG_128, DIALOG_117, DIALOG_150 };
    // Koopa the Quick (BOB), Koopa the Quick (THI), Penguin Race, Fat Penguin Race (120 stars)
    enum DialogId dialogRaceSound[] = { DIALOG_005, DIALOG_009, DIALOG_055, DIALOG_164 };
    // Red Switch, Green Switch, Blue Switch, 100 coins star, Bowser Red Coin Star
    enum DialogId dialogStarSound[] = { DIALOG_010, DIALOG_011, DIALOG_012, DIALOG_013, DIALOG_014 };
    // King Bob-omb (Start), Whomp (Defeated), King Bob-omb (Defeated, missing in JP), Eyerock (Defeated), Wiggler (Defeated)
    s16 i;

    enum DialogId dialogBossStopFixed[] = { DIALOG_017, DIALOG_115, DIALOG_116, DIALOG_118, DIALOG_152 };
    enum DialogId dialogBossStopVanilla[] = { DIALOG_017, DIALOG_115, DIALOG_118, DIALOG_152 };
    enum DialogId* dialogBossStop = configBugfixKingBobOmbFadeMusic ? dialogBossStopFixed : dialogBossStopVanilla;
    s16 dialogBossStopCount = configBugfixKingBobOmbFadeMusic ? (s16)ARRAY_COUNT(dialogBossStopFixed) : (s16)ARRAY_COUNT(dialogBossStopVanilla);

    for (i = 0; i < (s16) ARRAY_COUNT(dialogBossStart); i++) {
        if (dialogBossStart[i] == dialogID) {
            seq_player_unlower_volume(SEQ_PLAYER_LEVEL, 60);
            play_music(SEQ_PLAYER_LEVEL, SEQUENCE_ARGS(4, SEQ_EVENT_BOSS), 0);
            return;
        }
    }

    for (i = 0; i < (s16) ARRAY_COUNT(dialogRaceSound); i++) {
        if (dialogRaceSound[i] == dialogID && gDialogLineNum == 1) {
            play_race_fanfare();
            return;
        }
    }

    for (i = 0; i < (s16) ARRAY_COUNT(dialogStarSound); i++) {
        if (dialogStarSound[i] == dialogID && gDialogLineNum == 1) {
            play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundSource);
            return;
        }
    }

    for (i = 0; i < dialogBossStopCount; i++) {
        if (dialogBossStop[i] == dialogID) {
            seq_player_fade_out(SEQ_PLAYER_LEVEL, 1);
            return;
        }
    }
}

static u8 *sOverrideDialogHookString = NULL;

static bool sTotwcIntroDialogPausedPhysics = false;
static u32 sTotwcIntroDialogSavedTimeStopState = 0;

static void totwc_intro_dialog_restore_time_stop_state(void) {
    if (!sTotwcIntroDialogPausedPhysics) { return; }

    u32 mask = TIME_STOP_ENABLED | TIME_STOP_ALL_OBJECTS | TIME_STOP_ACTIVE;
    gTimeStopState = (gTimeStopState & ~mask) | (sTotwcIntroDialogSavedTimeStopState & mask);

    sTotwcIntroDialogPausedPhysics = false;
    sTotwcIntroDialogSavedTimeStopState = 0;
}

bool handle_dialog_hook(s32 dialogId) {
    bool openDialogBox = true;
    const char *dialogTextOverride = NULL;
    smlua_call_event_hooks(HOOK_ON_DIALOG, dialogId, &openDialogBox, &dialogTextOverride);
    if (!openDialogBox) {
        if (gCamera->cutscene == CUTSCENE_READ_MESSAGE) { gCamera->cutscene = 0; }
        return false;
    }

    free(sOverrideDialogHookString);
    if (dialogTextOverride != NULL) {
        sOverrideDialogHookString = convert_string_ascii_to_sm64(NULL, dialogTextOverride, false);
    } else {
        sOverrideDialogHookString = NULL;
    }
    return true;
}

void create_dialog_box(s32 dialog) {
    if (handle_dialog_hook(dialog) && gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogBoxType = DIALOG_TYPE_ROTATE;
    }
}

void create_dialog_box_with_var(s32 dialog, s32 dialogVar) {
    if (handle_dialog_hook(dialog) && gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogVariable = dialogVar;
        gDialogBoxType = DIALOG_TYPE_ROTATE;
    }
}

void create_dialog_inverted_box(s32 dialog) {
    if (handle_dialog_hook(dialog) && gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogBoxType = DIALOG_TYPE_ZOOM;
    }
}

void create_dialog_box_with_response(s32 dialog) {
    if (handle_dialog_hook(dialog) && gDialogID == DIALOG_NONE) {
        gDialogID = dialog;
        gDialogBoxType = DIALOG_TYPE_ROTATE;
        gLastDialogResponse = 1;
    }
}

void reset_dialog_render_state(void) {
    level_set_transition(0, NULL);

    totwc_intro_dialog_restore_time_stop_state();

    if (gDialogBoxType == DIALOG_TYPE_ZOOM) {
        trigger_cutscene_dialog(2);
    }

    gDialogBoxScale = 19.0f;
    gDialogBoxOpenTimer = 90.0f;
    gDialogBoxState = DIALOG_STATE_OPENING;
    gDialogID = DIALOG_NONE;
    gDialogTextPos = 0;
    gLastDialogResponse = 0;
    gLastDialogPageStrPos = 0;
    gDialogResponse = 0;
}

#define X_VAL1 -7.0f
#define Y_VAL1 5.0
#define Y_VAL2 5.0f

void render_dialog_box_type(struct DialogEntry *dialog, s8 linesPerBox) {
    UNUSED s32 unused;

    if (gOverrideDialogPos != 0) {
        create_dl_translation_matrix(MENU_MTX_NOPUSH, gDialogOverrideX - 61, 240 - gDialogOverrideY - 5, 0);
    }
    else {
        create_dl_translation_matrix(MENU_MTX_NOPUSH, dialog->leftOffset, dialog->width, 0);
    }

    switch (gDialogBoxType) {
        case DIALOG_TYPE_ROTATE: // Renders a dialog black box with zoom and rotation
            if (gDialogBoxState == DIALOG_STATE_OPENING || gDialogBoxState == DIALOG_STATE_CLOSING) {
                sDialogRotationPos = gDisplayListHead;
                if (gDialogBoxState == DIALOG_STATE_OPENING) {
                    sDialogScale = gDialogBoxScale - 1.5f;
                    sDialogRotation = gDialogBoxOpenTimer - 7.5f;
                } else {
                    sDialogScale = gDialogBoxScale + 2.0f;
                    sDialogRotation = gDialogBoxOpenTimer + 10.0f;
                }
                sDialogScalePrev = gDialogBoxScale;
                sDialogRotationPrev = gDialogBoxOpenTimer;
                create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.0 / sDialogScalePrev, 1.0 / sDialogScalePrev, 1.0f);
                // convert the speed into angle
                create_dl_rotation_matrix(MENU_MTX_NOPUSH, sDialogRotationPrev * 4.0f, 0, 0, 1.0f);
            }
            gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 150);
            break;
        case DIALOG_TYPE_ZOOM: // Renders a dialog white box with zoom
            if (gDialogBoxState == DIALOG_STATE_OPENING || gDialogBoxState == DIALOG_STATE_CLOSING) {
                sDialogZoomPos = gDisplayListHead;
                if (gDialogBoxState == DIALOG_STATE_OPENING) {
                    sDialogScale = gDialogBoxScale - 2;
                } else {
                    sDialogScale = gDialogBoxScale + 2;
                }
                sDialogScalePrev = gDialogBoxScale;
                create_dl_translation_matrix(MENU_MTX_NOPUSH, 65.0 - (65.0 / sDialogScalePrev),
                                              (40.0 / sDialogScalePrev) - 40, 0);
                create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.0 / sDialogScalePrev, 1.0 / sDialogScalePrev, 1.0f);
            }
            gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 150);
            break;
    }
    if (gOverrideDialogColor) {
        gDPSetEnvColor(gDisplayListHead++, gDialogBgColorR, gDialogBgColorG, gDialogBgColorB, gDialogBgColorA);
    }

    f32 dialogWidth = 130 * 1.1f;
    if (dialogWidth < gDialogMinWidth) dialogWidth = gDialogMinWidth;
    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL1, Y_VAL1, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, dialogWidth / 130, ((f32) linesPerBox / Y_VAL2) + 0.1, 1.0f);

    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void change_and_flash_dialog_text_color_lines(s8 colorMode, s8 lineNum) {
    u8 colorFade;

    if (colorMode == 1) {
        if (lineNum == 1) {
            gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
        } else {
            if (lineNum == gDialogLineNum) {
                colorFade = (gSineTable[gDialogColorFadeTimer >> 4] * 50.0f) + 200.0f;
                gDPSetEnvColor(gDisplayListHead++, colorFade, colorFade, colorFade, 255);
            } else {
                gDPSetEnvColor(gDisplayListHead++, 200, 200, 200, 255);
            }
        }
    } else {
        switch (gDialogBoxType) {
            case DIALOG_TYPE_ROTATE:
                break;
            case DIALOG_TYPE_ZOOM:
                gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
                break;
        }
    }
    if (gOverrideDialogColor) {
        gDPSetEnvColor(gDisplayListHead++, gDialogTextColorR, gDialogTextColorG, gDialogTextColorB, gDialogTextColorA)
    }
}

#define X_VAL3 0.0f
#define Y_VAL3 16
void handle_dialog_scroll_page_state(s8 lineNum, s8 totalLines, s8 *pageState, s8 *xMatrix, s16 *linePos)
{
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    if (lineNum == totalLines) {
        pageState[0] = DIALOG_PAGE_STATE_SCROLL;
        return;
    }
    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL3, 2 - (lineNum * Y_VAL3), 0);

    linePos[0] = 0;
    xMatrix[0] = 1;
}
void render_star_count_dialog_text(s8 *xMatrix, s16 *linePos)
{
    s8 tensDigit = gDialogVariable / 10;
    s8 onesDigit = gDialogVariable - (tensDigit * 10); // remainder

    if (tensDigit != 0) {
        if (xMatrix[0] != 1) {
            create_dl_translation_matrix(
                MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix[0] - 1)), 0, 0);
        }

        render_generic_char(tensDigit);
        create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[tensDigit]), 0, 0);
        xMatrix[0] = 1;
        linePos[0]++;
    }

    if (xMatrix[0] != 1) {
        create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix[0] - 1)), 0, 0);
    }

    render_generic_char(onesDigit);
    create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32) gDialogCharWidths[onesDigit], 0, 0);

    linePos[0]++;
    xMatrix[0] = 1;
}

void render_multi_text_string_lines(s8 multiTextId, s8 lineNum, s16 *linePos, s8 linesPerBox, s8 xMatrix, s8 lowerBound)
{
    s8 i;
    struct MultiTextEntry textLengths[2] = {
        { 3, { TEXT_THE_RAW } },
        { 3, { TEXT_YOU_RAW } },
    };

    if (lineNum >= lowerBound && lineNum <= (lowerBound + linesPerBox)) {
        if (linePos[0] != 0 || (xMatrix != 1)) {
            create_dl_translation_matrix(MENU_MTX_NOPUSH, (gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1)), 0, 0);
        }
        for (i = 0; i < textLengths[multiTextId].length; i++) {
            render_generic_char(textLengths[multiTextId].str[i]);
            create_dl_translation_matrix(MENU_MTX_NOPUSH, (gDialogCharWidths[textLengths[multiTextId].str[i]]), 0, 0);
        }
    }
    linePos += textLengths[multiTextId].length;
}

u32 ensure_nonnegative(s16 value) {
    if (value < 0) {
        value = 0;
    }

    return value;
}

void handle_dialog_text_and_pages(s8 colorMode, struct DialogEntry *dialog, s8 lowerBound)
{
    UNUSED s32 pad[2];

    u8 strChar;

    u8 *str = sOverrideDialogHookString != NULL ? sOverrideDialogHookString : segmented_to_virtual(dialog->str);
    s8 lineNum = 1;

    s8 totalLines;

    s8 pageState = DIALOG_PAGE_STATE_NONE;
    UNUSED s8 mark = DIALOG_MARK_NONE; // unused in US, EU
    s8 xMatrix = 1;

    s8 linesPerBox = dialog->linesPerBox;

    s16 strIdx;
    s16 linePos = 0;

    if (gDialogBoxState == DIALOG_STATE_HORIZONTAL) {
        // If scrolling, consider the number of lines for both
        // the current page and the page being scrolled to.
        totalLines = linesPerBox * 2 + 1;
    } else {
        totalLines = linesPerBox + 1;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    strIdx = gDialogTextPos;

    if (gDialogBoxState == DIALOG_STATE_HORIZONTAL) {
        sDialogOffset = gDialogScrollOffsetY + dialog->linesPerBox * 2;
        sDialogOffsetPrev = gDialogScrollOffsetY;
        sDialogOffsetPos = gDisplayListHead;
        create_dl_translation_matrix(MENU_MTX_NOPUSH, 0, (f32) sDialogOffsetPrev, 0);
    }

    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL3, 2 - lineNum * Y_VAL3, 0);

    while (pageState == DIALOG_PAGE_STATE_NONE) {
        change_and_flash_dialog_text_color_lines(colorMode, lineNum);
        strChar = str ? str[strIdx] : DIALOG_CHAR_TERMINATOR;

        switch (strChar) {
            case DIALOG_CHAR_TERMINATOR:
                pageState = DIALOG_PAGE_STATE_END;
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
                break;
            case DIALOG_CHAR_NEWLINE:
                lineNum++;
                handle_dialog_scroll_page_state(lineNum, totalLines, &pageState, &xMatrix, &linePos);
                break;
            case DIALOG_CHAR_DAKUTEN:
                mark = DIALOG_MARK_DAKUTEN;
                break;
            case DIALOG_CHAR_PERIOD_OR_HANDAKUTEN:
                mark = DIALOG_MARK_HANDAKUTEN;
                break;
            case DIALOG_CHAR_SPACE:
                xMatrix++;
                linePos++;
                break;
            case DIALOG_CHAR_SLASH:
                xMatrix += 2;
                linePos += 2;
                break;
            case DIALOG_CHAR_MULTI_THE:
                render_multi_text_string_lines(STRING_THE, lineNum, &linePos, linesPerBox, xMatrix, lowerBound);
                xMatrix = 1;
                break;
            case DIALOG_CHAR_MULTI_YOU:
                render_multi_text_string_lines(STRING_YOU, lineNum, &linePos, linesPerBox, xMatrix, lowerBound);
                xMatrix = 1;
                break;
            case DIALOG_CHAR_STAR_COUNT:
                render_star_count_dialog_text(&xMatrix, &linePos);
                break;
            default: // any other character
                if (lineNum >= lowerBound && lineNum <= lowerBound + linesPerBox) {
                    if (linePos || xMatrix != 1) {
                        create_dl_translation_matrix(
                            MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1)), 0, 0);
                    }

                    render_generic_char(strChar);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32)(gDialogCharWidths[strChar]), 0, 0);
                    xMatrix = 1;
                    linePos++;
                }
        }

        strIdx++;
    }
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    if (gDialogBoxState == DIALOG_STATE_VERTICAL) {
        if (pageState == DIALOG_PAGE_STATE_END) {
            gLastDialogPageStrPos = -1;
        } else {
            gLastDialogPageStrPos = strIdx;
        }
    }

    gLastDialogLineNum = lineNum;
}

#define X_VAL4_1 56
#define X_VAL4_2 47
#define Y_VAL4_1 2
#define Y_VAL4_2 16

void render_dialog_triangle_choice(void) {
    if (gDialogBoxState == DIALOG_STATE_VERTICAL) {
        handle_menu_scrolling(MENU_SCROLL_HORIZONTAL, &gDialogLineNum, 1, 2);
    }

    create_dl_translation_matrix(MENU_MTX_NOPUSH, (gDialogLineNum * X_VAL4_1) - X_VAL4_2, Y_VAL4_1 - (gLastDialogLineNum * Y_VAL4_2), 0);

    if (gDialogBoxType == DIALOG_TYPE_ROTATE) {
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else {
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    }
    if (gOverrideDialogColor) {
        gDPSetEnvColor(gDisplayListHead++, gDialogTextColorR, gDialogTextColorG, gDialogTextColorB, gDialogTextColorA);
    }

    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
}

#define X_VAL5 118.0f
#define Y_VAL5_1 -16
#define Y_VAL5_2 5
#define X_Y_VAL6 0.8f

void render_dialog_string_color(s8 linesPerBox) {
    s32 timer = gGlobalTimer;

    if (timer & 0x08) {
        return;
    }

    f32 triangleOffset = gDialogMinWidth - 143;
    if (triangleOffset < 0) triangleOffset = 0;

    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL5 + triangleOffset, (linesPerBox * Y_VAL5_1) + Y_VAL5_2, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, X_Y_VAL6, X_Y_VAL6, 1.0f);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, -DEFAULT_DIALOG_BOX_ANGLE, 0, 0, 1.0f);

    if (gDialogBoxType == DIALOG_TYPE_ROTATE) { // White Text
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else { // Black Text
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    }
    if (gOverrideDialogColor) {
        gDPSetEnvColor(gDisplayListHead++, gDialogTextColorR, gDialogTextColorG, gDialogTextColorB, gDialogTextColorA);
    }

    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

s16 gMenuMode = -1;
s16 gPrevMenuMode = -1;

u8 *gEndCutsceneStringsEn[] = {
    INGAME_TEXT_PTR(TEXT_FILE_MARIO_EXCLAMATION),
    INGAME_TEXT_PTR(TEXT_POWER_STARS_RESTORED),
    INGAME_TEXT_PTR(TEXT_THANKS_TO_YOU),
    INGAME_TEXT_PTR(TEXT_THANK_YOU_MARIO),
    INGAME_TEXT_PTR(TEXT_SOMETHING_SPECIAL),
    INGAME_TEXT_PTR(TEXT_LISTEN_EVERYBODY),
    INGAME_TEXT_PTR(TEXT_LETS_HAVE_CAKE),
    INGAME_TEXT_PTR(TEXT_FOR_MARIO),
    // This [8] string is actually unused. In the cutscene handler, the developers do not
    // set the 8th one, but use the first string again at the very end, so Peach ends up
    // saying "Mario!" twice. It is likely that she was originally meant to say "Mario?" at
    // the end but the developers changed their mind, possibly because the line recorded
    // sounded more like an exclamation than a question.
    INGAME_TEXT_PTR(TEXT_FILE_MARIO_QUESTION),
    NULL
};

u16 gCutsceneMsgFade = 0;
s16 gCutsceneMsgIndex = -1;
s16 gCutsceneMsgDuration = -1;
s16 gCutsceneMsgTimer = 0;
s8 gDialogCameraAngleIndex = CAM_SELECTION_MARIO;
s8 gDialogCourseActNum = 1;

#define DIAG_VAL1 16
#define DIAG_VAL3 132 // US & EU
#define DIAG_VAL4 5
#define DIAG_VAL2 240 // JP & US

void render_dialog_entries(void) {
    s8 lowerBound = 0;

    bool wasTotwcIntroDialog = sTotwcIntroDialogPausedPhysics;
    if (gDialogID == DIALOG_131 && !sTotwcIntroDialogPausedPhysics) {
        sTotwcIntroDialogPausedPhysics = true;
        sTotwcIntroDialogSavedTimeStopState = gTimeStopState;
        set_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_ALL_OBJECTS);
        gTimeStopState |= TIME_STOP_ACTIVE;
    }

    struct DialogEntry *dialog = dialog_table_get(gDialogID);

    if (dialog == NULL) {
        gDialogID = DIALOG_NONE;
        if (wasTotwcIntroDialog) {
            totwc_intro_dialog_restore_time_stop_state();
        }
        return;
    }


    switch (gDialogBoxState) {
        case DIALOG_STATE_OPENING:
            if (gDialogBoxOpenTimer == DEFAULT_DIALOG_BOX_ANGLE) {
                play_dialog_sound(gDialogID);
                play_sound(SOUND_MENU_MESSAGE_APPEAR, gGlobalSoundSource);
            }

            if (gDialogBoxType == DIALOG_TYPE_ROTATE) {
                gDialogBoxOpenTimer -= 7.5;
                gDialogBoxScale -= 1.5;
            } else {
                gDialogBoxOpenTimer -= 10.0;
                gDialogBoxScale -= 2.0;
            }

            if (gDialogBoxOpenTimer == 0.0f) {
                gDialogBoxState = DIALOG_STATE_VERTICAL;
                gDialogLineNum = 1;
            }
            lowerBound = 1;
            break;
        case DIALOG_STATE_VERTICAL:
            gDialogBoxOpenTimer = 0.0f;

            if ((gPlayer1Controller->buttonPressed & A_BUTTON)
                || (gPlayer1Controller->buttonPressed & B_BUTTON)) {
                if (gLastDialogPageStrPos == -1) {
                    handle_special_dialog_text(gDialogID);
                    gDialogBoxState = DIALOG_STATE_CLOSING;
                } else {
                    gDialogBoxState = DIALOG_STATE_HORIZONTAL;
                    play_sound(SOUND_MENU_MESSAGE_NEXT_PAGE, gGlobalSoundSource);
                }
            }
            lowerBound = 1;
            break;
        case DIALOG_STATE_HORIZONTAL:
            gDialogScrollOffsetY += dialog->linesPerBox * 2;

            if (gDialogScrollOffsetY >= dialog->linesPerBox * DIAG_VAL1) {
                gDialogTextPos = gLastDialogPageStrPos;
                gDialogBoxState = DIALOG_STATE_VERTICAL;
                gDialogScrollOffsetY = 0;
            }
            lowerBound = (gDialogScrollOffsetY / DIAG_VAL1) + 1;
            break;
        case DIALOG_STATE_CLOSING:
            if (gDialogBoxOpenTimer == 20.0f) {
                level_set_transition(0, NULL);
                play_sound(SOUND_MENU_MESSAGE_DISAPPEAR, gGlobalSoundSource);

                if (gDialogBoxType == DIALOG_TYPE_ZOOM) {
                    trigger_cutscene_dialog(2);
                }

                gDialogResponse = gDialogLineNum;
            }

            gDialogBoxOpenTimer = gDialogBoxOpenTimer + 10.0f;
            gDialogBoxScale = gDialogBoxScale + 2.0f;

            if (gDialogBoxOpenTimer == DEFAULT_DIALOG_BOX_ANGLE) {
                gDialogBoxState = DIALOG_STATE_OPENING;
                gDialogID = DIALOG_NONE;
                gDialogTextPos = 0;
                gLastDialogResponse = 0;
                gLastDialogPageStrPos = 0;
                gDialogResponse = 0;
            }
            lowerBound = 1;
            break;
    }

    render_dialog_box_type(dialog, dialog->linesPerBox);

    u32 scissorHeight = ensure_nonnegative(240 + ((dialog->linesPerBox * 80) / DIAG_VAL4) - dialog->width);
    u32 scissorY = ensure_nonnegative(DIAG_VAL2 - dialog->width);
    if (gOverrideDialogPos) {
        scissorHeight = ensure_nonnegative(((dialog->linesPerBox * 80) / DIAG_VAL4) + gDialogOverrideY + 5);
        scissorY = ensure_nonnegative(gDialogOverrideY + 5);
    }

    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE,
                  0,
                  scissorY,
                  SCREEN_WIDTH,
                  scissorHeight);
    handle_dialog_text_and_pages(0, dialog, lowerBound);

    if (gLastDialogPageStrPos == -1 && gLastDialogResponse == 1) {
        render_dialog_triangle_choice();
    }
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 2, 2, SCREEN_WIDTH - BORDER_HEIGHT/2, SCREEN_HEIGHT - BORDER_HEIGHT/2);
    if (gLastDialogPageStrPos != -1 && gDialogBoxState == DIALOG_STATE_VERTICAL) {
        render_dialog_string_color(dialog->linesPerBox);
    }

    if (wasTotwcIntroDialog && gDialogID == DIALOG_NONE) {
        totwc_intro_dialog_restore_time_stop_state();
    }
}

// Calls a gMenuMode value defined by render_menus_and_dialogs cases
void set_menu_mode(s16 mode) {
    if (gMenuMode == -1 || mode == -1) {
        gMenuMode = mode;
    }
}

void reset_cutscene_msg_fade(void) {
    gCutsceneMsgFade = 0;
}

void dl_rgba16_begin_cutscene_msg_fade(void) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCutsceneMsgFade);
}

void dl_rgba16_stop_cutscene_msg_fade(void) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (gCutsceneMsgFade < 250) {
        gCutsceneMsgFade += 25;
    } else {
        gCutsceneMsgFade = 255;
    }
}

u8 ascii_to_credits_char(u8 c) {
    if (c >= 'A' && c <= 'Z') {
        return (c - ('A' - 0xA));
    }

    if (c >= 'a' && c <= 'z') { // remap lower to upper case
        return (c - ('a' - 0xA));
    }

    if (c == ' ') {
        return GLOBAL_CHAR_SPACE;
    }
    if (c == '.') {
        return 0x24;
    }
    if (c == '3') {
        return ASCII_TO_DIALOG('3');
    }
    if (c == '4') {
        return ASCII_TO_DIALOG('4');
    }
    if (c == '6') {
        return ASCII_TO_DIALOG('6');
    }

    return GLOBAL_CHAR_SPACE;
}

void print_credits_str_ascii(s16 x, s16 y, const char *str) {
    s32 pos = 0;
    u8 c = str[pos];
    u8 creditStr[100];

    while (c != 0) {
        creditStr[pos++] = ascii_to_credits_char(c);
        c = str[pos];
    }

    creditStr[pos] = GLOBAR_CHAR_TERMINATOR;

    print_credits_string(x, y, creditStr);
}

void set_cutscene_message(s16 xOffset, s16 yOffset, s16 msgIndex, s16 msgDuration) {
    // is message done printing?
    if (gCutsceneMsgIndex == -1) {
        gCutsceneMsgIndex = msgIndex;
        gCutsceneMsgDuration = msgDuration;
        gCutsceneMsgTimer = 0;
        gCutsceneMsgXOffset = xOffset;
        gCutsceneMsgYOffset = yOffset;
        gCutsceneMsgFade = 0;
    }
}

void do_cutscene_handler(void) {
    s16 x;

    // is a cutscene playing? do not perform this handler's actions if so.
    if (gCutsceneMsgIndex == -1) {
        return;
    }

    create_dl_ortho_matrix();

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCutsceneMsgFade);
    // get the x coordinate of where the cutscene string starts.
    x = get_str_x_pos_from_center(gCutsceneMsgXOffset, gEndCutsceneStringsEn[gCutsceneMsgIndex], 10.0f);
    print_generic_string(x, 240 - gCutsceneMsgYOffset, gEndCutsceneStringsEn[gCutsceneMsgIndex]);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    // if the timing variable is less than 5, increment
    // the fade until we are at full opacity.
    if (gCutsceneMsgTimer < 5) {
        gCutsceneMsgFade += 50;
    }

    // if the cutscene frame length + the fade-in counter is
    // less than the timer, it means we have exceeded the
    // time that the message is supposed to remain on
    // screen. if (message_duration = 50) and (msg_timer = 55)
    // then after the first 5 frames, the message will remain
    // on screen for another 50 frames until it starts fading.
    if (gCutsceneMsgDuration + 5 < gCutsceneMsgTimer) {
        gCutsceneMsgFade -= 50;
    }

    // like the first check, it takes 5 frames to fade out, so
    // perform a + 10 to account for the earlier check (10-5=5).
    if (gCutsceneMsgDuration + 10 < gCutsceneMsgTimer) {
        gCutsceneMsgIndex = -1;
        gCutsceneMsgFade = 0;
        gCutsceneMsgTimer = 0;
        return;
    }

    gCutsceneMsgTimer++;
}

#define PEACH_MESSAGE_TIMER 250
#define STR_X 38
#define STR_Y 142

// "Dear Mario" message handler
void print_peach_letter_message(void) {
    struct DialogEntry *dialog = dialog_table_get(gDialogID);

    const u8* str = sOverrideDialogHookString != NULL ? sOverrideDialogHookString : dialog->str;

    create_dl_translation_matrix(MENU_MTX_PUSH, 97.0f, 118.0f, 0);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCutsceneMsgFade);
    gSPDisplayList(gDisplayListHead++, castle_grounds_seg7_dl_0700EA58);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 20, 20, 20, gCutsceneMsgFade);

    print_generic_string(STR_X, STR_Y, str);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 200, 80, 120, gCutsceneMsgFade);
    gSPDisplayList(gDisplayListHead++, castle_grounds_seg7_us_dl_0700F2E8);

    // at the start/end of message, reset the fade.
    if (gCutsceneMsgTimer == 0) {
        gCutsceneMsgFade = 0;
    }

    // we're less than 20 increments, so increase the fade.
    if (gCutsceneMsgTimer < 20) {
        gCutsceneMsgFade += 10;
    }

    // we're after PEACH_MESSAGE_TIMER increments, so decrease the fade.
    if (gCutsceneMsgTimer > PEACH_MESSAGE_TIMER) {
        gCutsceneMsgFade -= 10;
    }

    // 20 increments after the start of the decrease, we're
    // back where we are, so reset everything at the end.
    if (gCutsceneMsgTimer > (PEACH_MESSAGE_TIMER + 20)) {
        gCutsceneMsgIndex = -1;
        gCutsceneMsgFade = 0; //! uselessly reset since the next execution will just set it to 0 again.
        gDialogID = DIALOG_NONE;
        gCutsceneMsgTimer = 0;
        return; // return to avoid incrementing the timer
    }

    gCutsceneMsgTimer++;
}

/**
 * Renders the cannon reticle when Mario is inside a cannon.
 * Formed by four triangles.
 */
void render_hud_cannon_reticle(void) {
    create_dl_translation_matrix(MENU_MTX_PUSH, 160.0f, 120.0f, 0);

    gDPSetEnvColor(gDisplayListHead++, 50, 50, 50, 180);
    create_dl_translation_matrix(MENU_MTX_PUSH, -20.0f, -8.0f, 0);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, 20.0f, 8.0f, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 180.0f, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, 8.0f, -20.0f, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, DEFAULT_DIALOG_BOX_ANGLE, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, -8.0f, 20.0f, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, -DEFAULT_DIALOG_BOX_ANGLE, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void reset_red_coins_collected(void) {
}

void change_dialog_camera_angle(void) {
    if (cam_select_alt_mode(0) == CAM_SELECTION_MARIO) {
        gDialogCameraAngleIndex = CAM_SELECTION_MARIO;
    } else {
        gDialogCameraAngleIndex = CAM_SELECTION_FIXED;
    }
}

void shade_screen(void) {
    create_dl_translation_matrix(MENU_MTX_PUSH, GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), 240.0f, 0);

    // This is a bit weird. It reuses the dialog text box (width 130, height -80),
    // so scale to at least fit the screen.

    create_dl_scale_matrix(MENU_MTX_NOPUSH,
                           GFX_DIMENSIONS_ASPECT_RATIO * SCREEN_HEIGHT / 125.0f, 3.0f, 1.0f);

    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 110);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void print_animated_red_coin(s16 x, s16 y) {
    s32 timer = gGlobalTimer;

    create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 0.2f, 0.2f, 1.0f);
    gDPSetRenderMode(gDisplayListHead++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);

    switch (timer & 6) {
        case 0:
            gSPDisplayList(gDisplayListHead++, coin_seg3_dl_03007940);
            break;
        case 2:
            gSPDisplayList(gDisplayListHead++, coin_seg3_dl_03007968);
            break;
        case 4:
            gSPDisplayList(gDisplayListHead++, coin_seg3_dl_03007990);
            break;
        case 6:
            gSPDisplayList(gDisplayListHead++, coin_seg3_dl_030079B8);
            break;
    }

    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

static inline void red_coins_print_glyph(s16 *x, u8 glyph, u8 width) {
    u8 text[] = { glyph, GLYPH_SPACE };
    print_hud_lut_string(HUD_LUT_GLOBAL, *x, SCREEN_HEIGHT - 35, text);
    *x += width;
}

void render_pause_red_coins(void) {
    if (gCurrentArea->numRedCoins == 8) {
        u8 collected = gCurrentArea->numRedCoins - count_objects_with_behavior(bhvRedCoin);
        for (s32 x = 0; x < collected; x++) {
            print_animated_red_coin(GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(30) - x * 20, 16);
        }
        return;
    }

    if (gCurrentArea->numRedCoins > 0) {
        u8 collected = gCurrentArea->numRedCoins - count_objects_with_behavior(bhvRedCoin);
        s16 x = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(38);
        print_animated_red_coin(x - 8, 20);
        gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
        red_coins_print_glyph(&x, GLYPH_MULTIPLY, 16);
        if (collected >= 100) red_coins_print_glyph(&x, (collected / 100) % 10, 12);
        if (collected >= 10) red_coins_print_glyph(&x, (collected / 10) % 10, 12);
        red_coins_print_glyph(&x, collected % 10, 15);
        red_coins_print_glyph(&x, GLYPH_SLASH, 15);
        if (gCurrentArea->numRedCoins >= 100) red_coins_print_glyph(&x, (gCurrentArea->numRedCoins / 100) % 10, 12);
        if (gCurrentArea->numRedCoins >= 10) red_coins_print_glyph(&x, (gCurrentArea->numRedCoins / 10) % 10, 12);
        red_coins_print_glyph(&x, gCurrentArea->numRedCoins % 10, 15);
        gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    }
}

#define CRS_NUM_X1 100
#define TXT_STAR_X 98
#define ACT_NAME_X 116
#define LVL_NAME_X 117
#define MYSCORE_X  62

void render_pause_my_score_coins(void) {
    // sanity check
    if (!COURSE_IS_VALID_COURSE(gCurrCourseNum)) { return; }

    INGAME_TEXT_COPY(textCourse, TEXT_COURSE);
    INGAME_TEXT_COPY(textMyScore, TEXT_MY_SCORE);
    INGAME_TEXT_COPY(textStar, TEXT_STAR);
    INGAME_TEXT_COPY(textUnfilledStar, TEXT_UNFILLED_STAR);

    u8 strCourseNum[4];
    u8 courseIndex = gCurrCourseNum - 1;
    u8 starIndex = clamp(gDialogCourseActNum, 1, MAX_ACTS);
    u8 *courseName = (u8*) get_level_name_sm64(gCurrCourseNum, gCurrLevelNum, gCurrAreaIndex, 1);
    u8 *actName = (u8*) get_star_name_sm64(gCurrCourseNum, starIndex, 1);
    u8 starFlags;

    starFlags = save_file_get_star_flags(gCurrSaveFileNum - 1, gCurrCourseNum - 1);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    if (courseIndex < COURSE_STAGES_COUNT) {
        print_hud_my_score_coins(1, gCurrSaveFileNum - 1, courseIndex, 178, 103);
        print_hud_my_score_stars(gCurrSaveFileNum - 1, courseIndex, 118, 103);
    }

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    if (courseIndex < COURSE_STAGES_COUNT && save_file_get_course_star_count(gCurrSaveFileNum - 1, courseIndex) != 0) {
        print_generic_string(MYSCORE_X, 121, textMyScore);
    }

    if (courseIndex < COURSE_STAGES_COUNT) {
        print_generic_string(63, 157, textCourse);
        int_to_str(gCurrCourseNum, strCourseNum);
        print_generic_string(CRS_NUM_X1, 157, strCourseNum);

        if (starFlags & (1 << (starIndex - 1))) {
            print_generic_string(TXT_STAR_X, 140, textStar);
        } else {
            print_generic_string(TXT_STAR_X, 140, textUnfilledStar);
        }
        if (actName != NULL) {
            print_generic_string(ACT_NAME_X, 140, actName);
        }
        print_generic_string(LVL_NAME_X, 157, &courseName[3]);
    }
    else {
        print_generic_string(94, 157, &courseName[3]);
    }
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

#define TXT1_X 3
#define TXT2_X 119
#define Y_VAL7 2

void render_pause_camera_options(s16 x, s16 y, s8 *index, s16 xIndex) {
    INGAME_TEXT_COPY(textLakituMario, TEXT_LAKITU_MARIO);
    INGAME_TEXT_COPY(textLakituStop, TEXT_LAKITU_STOP);
    INGAME_TEXT_COPY(textNormalUpClose, TEXT_NORMAL_UPCLOSE);
    INGAME_TEXT_COPY(textNormalFixed, TEXT_NORMAL_FIXED);

    handle_menu_scrolling(MENU_SCROLL_HORIZONTAL, index, 1, 2);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    print_generic_string(x + 14, y + 2, textLakituMario);
    print_generic_string(x + TXT1_X, y - 13, textNormalUpClose);
    print_generic_string(x + 124, y + 2, textLakituStop);
    print_generic_string(x + TXT2_X, y - 13, textNormalFixed);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    create_dl_translation_matrix(MENU_MTX_PUSH, ((index[0] - 1) * xIndex) + x, y + Y_VAL7, 0);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    switch (index[0]) {
        case 1:
            cam_select_alt_mode(1);
            break;
        case 2:
            cam_select_alt_mode(2);
            break;
    }
}

#define X_VAL8 4
#define Y_VAL8 2

void render_pause_course_options(s16 x, s16 y, s8 *index, s16 yIndex) {
    u8 TEXT_EXIT_TO_CASTLE[16] = { DIALOG_CHAR_TERMINATOR };
    convert_string_ascii_to_sm64(TEXT_EXIT_TO_CASTLE, "EXIT TO CASTLE", false);

    INGAME_TEXT_COPY(textContinue, TEXT_CONTINUE);
    INGAME_TEXT_COPY(textExitCourse, TEXT_EXIT_COURSE);
    INGAME_TEXT_COPY(textCameraAngleR, TEXT_CAMERA_ANGLE_R);

    handle_menu_scrolling(MENU_SCROLL_VERTICAL, index, 1, 4);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    print_generic_string(x + 10, y - 2, textContinue);
    print_generic_string(x + 10, y - 17, textExitCourse);
    print_generic_string(x + 10, y - 32, TEXT_EXIT_TO_CASTLE);

    if (index[0] != 4) {
        print_generic_string(x + 10, y - 47, textCameraAngleR);
        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

        create_dl_translation_matrix(MENU_MTX_PUSH, x - X_VAL8, (y - ((index[0] - 1) * yIndex)) - Y_VAL8, 0);

        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
        gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
        gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    } else {
        render_pause_camera_options(x - 42, y - 57, &gDialogCameraAngleIndex, 110);
    }
}

void render_pause_castle_menu_box(s16 x, s16 y) {
    create_dl_translation_matrix(MENU_MTX_PUSH, x - 78, y - 32, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.2f, 0.8f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 105);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, x + 6, y - 28, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, DEFAULT_DIALOG_BOX_ANGLE, 0, 0, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, x - 9, y - 101, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 270.0f, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void render_pause_castle_menu_box_extended(s16 x, s16 y) {
    create_dl_translation_matrix(MENU_MTX_PUSH, x - 98, y - 32, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.5f, 0.8f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 105);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, x + 6, y - 28, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, DEFAULT_DIALOG_BOX_ANGLE, 0, 0, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, x - 9, y - 101, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 270.0f, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void highlight_last_course_complete_stars(void) {
    u8 courseDone;

    if (gLastCompletedCourseNum == COURSE_NONE) {
        courseDone = 0;
    } else {
        courseDone = gLastCompletedCourseNum - 1;

        if (courseDone >= COURSE_STAGES_COUNT) {
            courseDone = COURSE_STAGES_COUNT;
        }
    }

    gDialogLineNum = courseDone;
}

void print_hud_pause_colorful_str(void) {
    INGAME_TEXT_COPY(textPause, TEXT_PAUSE);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    print_hud_lut_string(HUD_LUT_GLOBAL, 123, 81, textPause);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

void render_pause_castle_course_stars(s16 x, s16 y, s16 fileNum, s16 courseNum) {
    u8 str[COURSE_STAGES_COUNT * 2];

    INGAME_TEXT_COPY(textStar, TEXT_STAR);

    u8 starFlags;

    starFlags = save_file_get_star_flags(fileNum, courseNum);

    if (starFlags & 0x40) {
        str[0] = DIALOG_CHAR_STAR_FILLED;
    } else {
        str[0] = DIALOG_CHAR_STAR_OPEN;
    }

    str[1] = DIALOG_CHAR_SPACE;

    for (s8 i = 1; i < 6; i++) {
        if (starFlags & (1 << i)) {
            str[i * 2] = DIALOG_CHAR_STAR_FILLED;
        } else {
            str[i * 2] = DIALOG_CHAR_STAR_OPEN;
        }

        str[i * 2 + 1] = DIALOG_CHAR_SPACE;
    }

    str[12] = DIALOG_CHAR_TERMINATOR;

    print_generic_string(x + 14, y + 13, str);
}

void render_pause_castle_main_strings(s16 x, s16 y) {
    INGAME_TEXT_COPY(textCoin, TEXT_COIN_X);

    s8 prevIndex = gDialogLineNum;
    handle_menu_scrolling(MENU_SCROLL_VERTICAL, &gDialogLineNum, -1, COURSE_STAGES_COUNT + 1);
    s8 scrollDir = (gDialogLineNum >= prevIndex ? +1 : -1);
    if (gDialogLineNum >= (COURSE_STAGES_COUNT + 1)) {
        gDialogLineNum = 0;
        scrollDir = +1;
    } else if (gDialogLineNum <= -1) {
        gDialogLineNum = COURSE_STAGES_COUNT;
        scrollDir = -1;
    }

    if (gDialogLineNum < COURSE_STAGES_COUNT) {
        // Skip courses with 0 star collected
        s16 tries = 0;
        while (tries++ < COURSE_STAGES_COUNT) {
            if (save_file_get_star_flags(gCurrSaveFileNum - 1, gDialogLineNum) != 0) {
                break;
            }
            if (scrollDir > 0) {
                gDialogLineNum++;
                if (gDialogLineNum >= COURSE_STAGES_COUNT) {
                    gDialogLineNum = 0;
                }
            } else {
                gDialogLineNum--;
                if (gDialogLineNum < 0) {
                    gDialogLineNum = COURSE_STAGES_COUNT - 1;
                }
            }
        }
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    u8 strVal[8];
    UNUSED u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
    UNUSED u16 unused;
    UNUSED u8 textSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

    if (gDialogLineNum < COURSE_STAGES_COUNT) {
        u8 courseNum = gDialogLineNum + 1;
        const u8 *courseName = get_level_name_sm64(courseNum, get_level_num_from_course_num(courseNum), 1, 1);
        pause_capitalize_string_sm64((u8 *) courseName);

        render_pause_castle_course_stars(x, y, gCurrSaveFileNum - 1, gDialogLineNum);
        print_generic_string(x + 34, y - 5, textCoin);
        int_to_str(save_file_get_course_coin_score(gCurrSaveFileNum - 1, gDialogLineNum), strVal);
        print_generic_string(x + 54, y - 5, strVal);
        print_generic_string(x - 9, y + 30, courseName);
    } else if (gDialogLineNum == COURSE_STAGES_COUNT) {
        const u8 *courseName = ((const u8 **) get_course_name_table())[COURSE_MAX]; // Castle secret stars

        INGAME_TEXT_COPY(textStarX, TEXT_STAR_X);
        print_generic_string(x + 40, y + 13, textStarX);
        int_to_str(save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_BONUS_STAGES - 1, COURSE_MAX - 1), strVal);
        print_generic_string(x + 60, y + 13, strVal);
        print_generic_string(x - 9, y + 30, courseName);
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

#define INDEX_CASTLE_STARS (COURSE_COUNT)
#define INDEX_FLAGS (COURSE_COUNT + 1)
#define INDEX_MIN (-1)
#define INDEX_MAX (INDEX_FLAGS + 1)

static u32 pause_castle_get_stars(s32 index) {

    // Main courses (0-14), Secret courses (15-24)
    if (index >= 0 && index < INDEX_CASTLE_STARS) {
        return save_file_get_star_flags(gCurrSaveFileNum - 1, index);
    }

    // Castle stars (25)
    if (index == INDEX_CASTLE_STARS) {
        return save_file_get_star_flags(gCurrSaveFileNum - 1, -1);
    }

    // Flags (26)
    if (index == INDEX_FLAGS) {
        return save_file_get_flags();
    }

    return 0;
}

static void render_pause_castle_course_name(const u8 *courseName, s16 x, s16 y) {
    s16 width = 0;
    for (const u8 *c = courseName; *c != DIALOG_CHAR_TERMINATOR; c++) {
        width += gDialogCharWidths[*c];
    }
    print_generic_string(x - width / 2, y, courseName);
}

static void render_pause_castle_flag_icon(const u8 *texture, s16 texW, s16 texH, s16 x, s16 y, s16 w, s16 h) {
    gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, texW, texH, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
    gSPTextureRectangle(gDisplayListHead++, (x) << 2, (SCREEN_HEIGHT - h - y) << 2, (x + w) << 2, (SCREEN_HEIGHT - y) << 2, G_TX_RENDERTILE, 0, 0, ((0x400 * texW) / w), ((0x400 * texH) / h));
}

static void render_pause_castle_flag(s16 x, s16 y, u32 flag) {
    if (save_file_get_flags() & flag) {
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    } else {
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gDialogTextAlpha / 3);
    }
    switch (flag) {
        case SAVE_FLAG_HAVE_WING_CAP:
            render_pause_castle_flag_icon(exclamation_box_seg8_texture_08015E28, 32, 32, x, y, 12, 12);
            break;

        case SAVE_FLAG_HAVE_METAL_CAP:
            render_pause_castle_flag_icon(exclamation_box_seg8_texture_08014628, 32, 32, x, y, 12, 12);
            break;

        case SAVE_FLAG_HAVE_VANISH_CAP:
            render_pause_castle_flag_icon(exclamation_box_seg8_texture_08012E28, 32, 32, x, y, 12, 12);
            break;

        case SAVE_FLAG_HAVE_KEY_1 | SAVE_FLAG_UNLOCKED_BASEMENT_DOOR:
        case SAVE_FLAG_HAVE_KEY_2 | SAVE_FLAG_UNLOCKED_UPSTAIRS_DOOR:
            render_pause_castle_flag_icon(bowser_key_left_texture, 32, 64, x, y, 6, 12);
            render_pause_castle_flag_icon(bowser_key_right_texture, 32, 64, x + 6, y, 6, 12);
            break;
    }
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
}

static void render_pause_castle_course_stars_extended(s16 x, s16 y) {
    bool isMainCourse = COURSE_IS_MAIN_COURSE(gDialogLineNum + 1);
    u32 stars = pause_castle_get_stars(gDialogLineNum);
    u8 str[32];

    // Build the stars string
    s32 lastCollectedStar = 0;
    for (s32 i = 0; i != (isMainCourse ? 6 : 7); ++i) {
        if (stars & (1 << i)) {
            str[2 * i] = DIALOG_CHAR_STAR_FILLED;
            lastCollectedStar = i + 1;
        } else {
            str[2 * i] = DIALOG_CHAR_STAR_OPEN;
        }
        str[2 * i + 1] = DIALOG_CHAR_SPACE;
        str[2 * i + 2] = DIALOG_CHAR_TERMINATOR;
    }

    // Hide the not collected ones after the last collected for secret courses
    if (!isMainCourse) {
        str[2 * lastCollectedStar] = DIALOG_CHAR_TERMINATOR;
    }

    // Render the 100 coins star next to the coin counter for main courses
    if (isMainCourse && (stars & 0x40)) {
        INGAME_TEXT_COPY(textStar, TEXT_STAR);
        print_generic_string(x + 89, y - 5, textStar);
    }

    // Render the stars
    print_generic_string(x + 14, y + 13, str);
}

void render_pause_castle_main_strings_extended(s16 x, s16 y) {

    // Main courses (0-14), Secret courses (15-24), Castle stars (25), Flags (26)
    // Indices -1 and 26 are used to loop back respectively to Flags and Course 1
    s8 prevIndex = gDialogLineNum;
    handle_menu_scrolling(MENU_SCROLL_VERTICAL, &gDialogLineNum, INDEX_MIN, INDEX_MAX);
    s8 scrollDir = (gDialogLineNum >= prevIndex ? +1 : -1);
    if (gDialogLineNum >= INDEX_MAX) {
        gDialogLineNum = 0;
        scrollDir = +1;
    } else if (gDialogLineNum <= INDEX_MIN) {
        gDialogLineNum = INDEX_FLAGS;
        scrollDir = -1;
    }

    // Skip courses with 0 star collected
    if (gDialogLineNum < INDEX_CASTLE_STARS) {
        while (!pause_castle_get_stars(gDialogLineNum)) {
            gDialogLineNum += scrollDir;
            if (gDialogLineNum >= INDEX_CASTLE_STARS) {
                gDialogLineNum = INDEX_CASTLE_STARS;
                break;
            }
            if (gDialogLineNum <= INDEX_MIN) {
                gDialogLineNum = INDEX_FLAGS;
                break;
            }
        }
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    u8 courseNum = gDialogLineNum + 1;
    const u8 *courseName = (
        gDialogLineNum >= INDEX_CASTLE_STARS ?
        ((const u8 **) get_course_name_table())[COURSE_MAX] : // Castle secret stars
        get_level_name_sm64(courseNum, get_level_num_from_course_num(courseNum), 1, 1)
    );

    // Main courses (0-14)
    if (gDialogLineNum < COURSE_STAGES_COUNT) {
        INGAME_TEXT_COPY(textCoin, TEXT_COIN_X);
        u8 textCoinCount[8];
        render_pause_castle_course_name(courseName, 160, y + 30);
        render_pause_castle_course_stars_extended(x + 20, y);
        print_generic_string(x + 54, y - 5, textCoin);
        int_to_str(save_file_get_course_coin_score(gCurrSaveFileNum - 1, gDialogLineNum), textCoinCount);
        print_generic_string(x + 94, y - 5, textCoinCount);
    }

    // Secret courses (15-24)
    else if (gDialogLineNum >= COURSE_STAGES_COUNT && gDialogLineNum < INDEX_CASTLE_STARS) {
        render_pause_castle_course_name(courseName + 3, 160, y + 30);
        render_pause_castle_course_stars_extended(x + 20, y);
    }

    // Castle stars (25)
    else if (gDialogLineNum == INDEX_CASTLE_STARS) {
        INGAME_TEXT_COPY(textStar, TEXT_STAR_X);
        u8 textStarCount[8];
        render_pause_castle_course_name(courseName + 3, 160, y + 30);
        print_generic_string(x + 60, y + 13, textStar);
        int_to_str(save_file_get_course_star_count(gCurrSaveFileNum - 1, -1), textStarCount);
        print_generic_string(x + 80, y + 13, textStarCount);
    }

    // Flags (26)
    else if (gDialogLineNum == INDEX_FLAGS) {
        const u8 textFlags[] = { ASCII_TO_DIALOG('P'), ASCII_TO_DIALOG('R'), ASCII_TO_DIALOG('O'), ASCII_TO_DIALOG('G'), ASCII_TO_DIALOG('R'), ASCII_TO_DIALOG('E'), ASCII_TO_DIALOG('S'), ASCII_TO_DIALOG('S'), DIALOG_CHAR_TERMINATOR };
        const u8 textCaps[] = { ASCII_TO_DIALOG('C'), ASCII_TO_DIALOG('A'), ASCII_TO_DIALOG('P'), ASCII_TO_DIALOG('S'), 0xE6, DIALOG_CHAR_TERMINATOR };
        const u8 textKeys[] = { ASCII_TO_DIALOG('K'), ASCII_TO_DIALOG('E'), ASCII_TO_DIALOG('Y'), ASCII_TO_DIALOG('S'), 0xE6, DIALOG_CHAR_TERMINATOR };
        render_pause_castle_course_name(textFlags, 160, y + 30);
        print_generic_string(x + 45, y + 13, textCaps);
        render_pause_castle_flag(x + 80, y + 15, SAVE_FLAG_HAVE_WING_CAP);
        render_pause_castle_flag(x + 96, y + 15, SAVE_FLAG_HAVE_METAL_CAP);
        render_pause_castle_flag(x + 112, y + 15, SAVE_FLAG_HAVE_VANISH_CAP);
        print_generic_string(x + 45, y - 5, textKeys);
        render_pause_castle_flag(x + 80, y - 3, SAVE_FLAG_HAVE_KEY_1 | SAVE_FLAG_UNLOCKED_BASEMENT_DOOR);
        render_pause_castle_flag(x + 96, y - 3, SAVE_FLAG_HAVE_KEY_2 | SAVE_FLAG_UNLOCKED_UPSTAIRS_DOOR);
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

s8 gCourseCompleteCoinsEqual = 0;
s32 gCourseDoneMenuTimer = 0;
s32 gCourseCompleteCoins = 0;
s8 gHudFlash = 0;

s16 render_pause_courses_and_castle(void) {
    s16 num;
    switch (gDialogBoxState) {
        case DIALOG_STATE_OPENING:
            gDialogLineNum = 1;
            gDialogTextAlpha = 0;
            //level_set_transition(-1, NULL);
            play_sound(SOUND_MENU_PAUSE_HIGHPRIO, gGlobalSoundSource);

            if (COURSE_IS_VALID_COURSE(gCurrCourseNum)) {
                change_dialog_camera_angle();
                gDialogBoxState = DIALOG_STATE_VERTICAL;
            } else {
                highlight_last_course_complete_stars();
                gDialogBoxState = DIALOG_STATE_HORIZONTAL;
            }
            break;
        case DIALOG_STATE_VERTICAL:
            if (!gDjuiPanelPauseCreated && !gPauseMenuHidden) {
                shade_screen();
                render_pause_my_score_coins();
                render_pause_red_coins();

                /* Always allow exiting from course */
                if (gLevelValues.pauseExitAnywhere
                    || (gMarioStates[0].action & ACT_FLAG_PAUSE_EXIT)) {
                    render_pause_course_options(99, 93, &gDialogLineNum, 15);
                }

                if (gPlayer1Controller->buttonPressed & A_BUTTON
                 || gPlayer1Controller->buttonPressed & START_BUTTON)
                {
                    bool allowExit = true;
                    if (gDialogLineNum == 2) {
                        // Exit Course
                        allowExit = (gLevelValues.pauseExitAnywhere || (gMarioStates[0].action & ACT_FLAG_PAUSE_EXIT));
                    } else if (gDialogLineNum == 3) {
                        // Exit to Castle
                        allowExit = (gLevelValues.pauseExitAnywhere || (gMarioStates[0].action & ACT_FLAG_PAUSE_EXIT))
                            && (gLevelValues.exitCastleLevel != LEVEL_NONE);
                    }

                    if (gDialogLineNum == 2 || gDialogLineNum == 3) {
                        smlua_call_event_hooks(HOOK_ON_PAUSE_EXIT, gDialogLineNum == 3, &allowExit);
                    }
                    if (allowExit) {
                        level_set_transition(0, NULL);
                        play_sound(SOUND_MENU_PAUSE_2, gGlobalSoundSource);
                        gDialogBoxState = DIALOG_STATE_OPENING;
                        gMenuMode = -1;

                        if (gDialogLineNum == 2 || gDialogLineNum == 3) {
                            num = gDialogLineNum;
                        } else {
                            num = 1;
                        }

                        return num;
                    } else {
                        play_sound(SOUND_MENU_CAMERA_BUZZ | (0xFF << 8), gGlobalSoundSource);
                    }
                }
            }
            break;
        case DIALOG_STATE_HORIZONTAL:
            if (!gDjuiPanelPauseCreated && !gPauseMenuHidden) {
                shade_screen();
                print_hud_pause_colorful_str();

                if (gLevelValues.extendedPauseDisplay) {
                    render_pause_castle_menu_box_extended(160, 143);
                    render_pause_castle_main_strings_extended(84, 60);
                } else {
                    render_pause_castle_menu_box(160, 143);
                    render_pause_castle_main_strings(104, 60);
                }
                if (gPlayer1Controller->buttonPressed & A_BUTTON
                 || gPlayer1Controller->buttonPressed & START_BUTTON)
                {
                    level_set_transition(0, NULL);
                    play_sound(SOUND_MENU_PAUSE_2, gGlobalSoundSource);
                    gMenuMode = -1;
                    gDialogBoxState = DIALOG_STATE_OPENING;

                    return 1;
                }
          }
          break;
    }

    if (gDialogTextAlpha < 250) {
        gDialogTextAlpha += 25;
    }

    if ((gPlayer1Controller->buttonPressed & L_TRIG) && configModDevMode) {
        for (int i = 0; i < gLocalMods.entryCount; i++) {
            struct Mod* mod = gLocalMods.entries[i];
            if (mod->enabled) {
                mod_refresh_files(mod);
            }
        }
        djui_lua_error_clear();
    }

    return 0;
}

#define TXT_HISCORE_X 109
#define TXT_HISCORE_Y 36
#define TXT_CONGRATS_X 70

#define HUD_PRINT_HISCORE         0
#define HUD_PRINT_CONGRATULATIONS 1

void print_hud_course_complete_string(s8 str) {
    INGAME_TEXT_COPY(textHiScore, TEXT_HUD_HI_SCORE);
    INGAME_TEXT_COPY(textCongratulations, TEXT_HUD_CONGRATULATIONS);

    u8 colorFade = sins(gDialogColorFadeTimer) * 50.0f + 200.0f;

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, colorFade, colorFade, colorFade, 255);

    if (str == HUD_PRINT_HISCORE) {
        print_hud_lut_string(HUD_LUT_GLOBAL, TXT_HISCORE_X, TXT_HISCORE_Y, textHiScore);
    } else { // HUD_PRINT_CONGRATULATIONS
        print_hud_lut_string(HUD_LUT_GLOBAL, TXT_CONGRATS_X, 67, textCongratulations);
    }

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

void print_hud_course_complete_coins(s16 x, s16 y) {
    u8 courseCompleteCoinsStr[4];
    u8 hudTextSymCoin[] = { GLYPH_COIN, GLYPH_SPACE };
    u8 hudTextSymX[] = { GLYPH_MULTIPLY, GLYPH_SPACE };

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);

    print_hud_lut_string(HUD_LUT_GLOBAL, x, y, hudTextSymCoin);
    print_hud_lut_string(HUD_LUT_GLOBAL, x + 16, y, hudTextSymX);

    int_to_str(gCourseCompleteCoins, courseCompleteCoinsStr);
    print_hud_lut_string(HUD_LUT_GLOBAL, x + 32, y, courseCompleteCoinsStr);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (gCourseCompleteCoins >= gHudDisplay.coins) {
        gCourseCompleteCoinsEqual = 1;
        gCourseCompleteCoins = gHudDisplay.coins;

        if (gGotFileCoinHiScore) {
            print_hud_course_complete_string(HUD_PRINT_HISCORE);
        }
    } else {
        if ((gCourseDoneMenuTimer & 1) || gHudDisplay.coins > 70) {
            gCourseCompleteCoins++;
            play_sound(SOUND_MENU_YOSHI_GAIN_LIVES, gGlobalSoundSource);

            // retain vanilla behavior
            if (gLevelValues.numCoinsToLife == 50) {
                if (gCourseCompleteCoins == 50 || gCourseCompleteCoins == 100 || gCourseCompleteCoins == 150) {
                    play_sound(SOUND_GENERAL_COLLECT_1UP, gGlobalSoundSource);
                    gMarioStates[0].numLives++;
                }
            } else {
                if (gLevelValues.numCoinsToLife != 0 && gCourseCompleteCoins % gLevelValues.numCoinsToLife == 0 && gCourseCompleteCoins > 0) {
                    play_sound(SOUND_GENERAL_COLLECT_1UP, gGlobalSoundSource);
                    gMarioStates[0].numLives++;
                }
            }
        }

        if (gHudDisplay.coins == gCourseCompleteCoins && gGotFileCoinHiScore) {
            play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gGlobalSoundSource);
        }
    }
}

void play_star_fanfare_and_flash_hud(s32 arg, u8 starNum) {
    if (gHudDisplay.coins == gCourseCompleteCoins && (gCurrCourseStarFlags & starNum) == 0 && gHudFlash == 0) {
        gCurrCourseStarFlags |= starNum; // SM74 was spamming fanfare without this line
        if (gFanFareDebounce <= 0) {
            gFanFareDebounce = 30 * 5;
            play_star_fanfare();
        }
        gHudFlash = arg;
    }
}

#define TXT_NAME_X1 71
#define TXT_NAME_X2 69
#define CRS_NUM_X2 104
#define CRS_NUM_X3 102
#define TXT_CLEAR_X1 get_string_width(name) + 81
#define TXT_CLEAR_X2 get_string_width(name) + 79

void render_course_complete_lvl_info_and_hud_str(void) {
    INGAME_TEXT_COPY(textCourse, TEXT_COURSE);
    INGAME_TEXT_COPY(textCatch, TEXT_CATCH); // unused in US
    INGAME_TEXT_COPY(textClear, TEXT_CLEAR);
    u8 textSymStar[] = { GLYPH_STAR, GLYPH_SPACE };
    u8 *name;

    u8 strCourseNum[4];

    if (gLastCompletedCourseNum <= COURSE_STAGES_MAX) {
        print_hud_course_complete_coins(118, 103);
        play_star_fanfare_and_flash_hud(1, 1 << (gLastCompletedStarNum - 1));

        name = (u8*) get_star_name_sm64(gLastCompletedCourseNum, gLastCompletedStarNum, 1);

        gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
        int_to_str(gLastCompletedCourseNum, strCourseNum);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gDialogTextAlpha);
        print_generic_string(65, 165, textCourse);
        print_generic_string(CRS_NUM_X2, 165, strCourseNum);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
        print_generic_string(63, 167, textCourse);
        print_generic_string(CRS_NUM_X3, 167, strCourseNum);
        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    } else if ((gLastCompletedCourseNum == COURSE_BITDW || gLastCompletedCourseNum == COURSE_BITFS) && gLastCollectedStarOrKey == 1) {
        name = (u8*) get_level_name_sm64(gLastCompletedCourseNum, gCurrLevelNum, gCurrAreaIndex, 1) + 3;
        gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gDialogTextAlpha);
        print_generic_string(TXT_NAME_X1, 130, name);
        print_generic_string(TXT_CLEAR_X1, 130, textClear);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
        print_generic_string(TXT_NAME_X2, 132, name);
        print_generic_string(TXT_CLEAR_X2, 132, textClear);
        gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
        print_hud_course_complete_coins(118, 111);
        play_star_fanfare_and_flash_hud(2, 0); //! 2 isn't defined, originally for key hud?
        return;
    } else {
        name = (u8*) get_level_name_sm64(gLastCompletedCourseNum, gLastCompletedStarNum, gCurrAreaIndex, 1) + 3;
        print_hud_course_complete_coins(118, 103);
        play_star_fanfare_and_flash_hud(1, 1 << (gLastCompletedStarNum - 1));
    }

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    print_hud_lut_string(HUD_LUT_GLOBAL, 55, 77, textSymStar);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, gDialogTextAlpha);
    print_generic_string(76, 145, name);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    print_generic_string(74, 147, name);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

#define TXT_SAVEOPTIONS_X x + 12
#define TXT_SAVECONT_Y 0
//#define TXT_SAVEQUIT_Y 20
//#define TXT_SAVE_EXIT_GAME_Y 40
#define TXT_CONTNOSAVE_Y 20

#define X_VAL9 x
void render_save_confirmation(s16 x, s16 y, s8 *index, s16 sp6e)
{
    INGAME_TEXT_COPY(textSaveAndContinue, TEXT_SAVE_AND_CONTINUE);
    //u8 textSaveAndQuit[] = { TEXT_SAVE_AND_QUIT };
    //u8 textSaveExitGame[] = { TEXT_SAVE_EXIT_GAME };
    INGAME_TEXT_COPY(textContinueWithoutSave, TEXT_CONTINUE_WITHOUT_SAVING);

    handle_menu_scrolling(MENU_SCROLL_VERTICAL, index, 1, 2); // decreased to '2' to prevent Exit Game

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

    print_generic_string(TXT_SAVEOPTIONS_X, y + TXT_SAVECONT_Y, textSaveAndContinue);
    //print_generic_string(TXT_SAVEOPTIONS_X, y - TXT_SAVEQUIT_Y, textSaveAndQuit);
    //print_generic_string(TXT_SAVEOPTIONS_X, y - TXT_SAVE_EXIT_GAME_Y, textSaveExitGame);
    print_generic_string(TXT_SAVEOPTIONS_X, y - TXT_CONTNOSAVE_Y, textContinueWithoutSave);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    create_dl_translation_matrix(MENU_MTX_PUSH, X_VAL9, y - ((index[0] - 1) * sp6e), 0);

    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);

    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

s16 render_course_complete_screen(void) {
    s16 num;

    switch (gDialogBoxState) {
        case DIALOG_STATE_OPENING:
            render_course_complete_lvl_info_and_hud_str();
            if (gCourseDoneMenuTimer > 100 && gCourseCompleteCoinsEqual == 1) {
                gDialogBoxState = DIALOG_STATE_VERTICAL;
                //level_set_transition(-1, NULL);
                gDialogTextAlpha = 0;
                gDialogLineNum = 1;
            }
            break;
        case DIALOG_STATE_VERTICAL:
            shade_screen();
            render_course_complete_lvl_info_and_hud_str();
            render_save_confirmation(100, 86, &gDialogLineNum, 20);
            if (gCourseDoneMenuTimer > 110
                && (gPlayer1Controller->buttonPressed & A_BUTTON
                 || gPlayer1Controller->buttonPressed & START_BUTTON
                )) {
                level_set_transition(0, NULL);
                play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundSource);
                gDialogBoxState = DIALOG_STATE_OPENING;
                gMenuMode = -1;
                num = gDialogLineNum;
                gCourseDoneMenuTimer = 0;
                gCourseCompleteCoins = 0;
                gCourseCompleteCoinsEqual = 0;
                gHudFlash = 0;

                return num;
            }
            break;
    }

    if (gDialogTextAlpha < 250) {
        gDialogTextAlpha += 25;
    }

    gCourseDoneMenuTimer++;

    return 0;
}

s16 render_menus_and_dialogs(void) {
    s16 mode = 0;

    create_dl_ortho_matrix();

    if (gMenuMode != -1) {
        gPrevMenuMode = gMenuMode;
        switch (gMenuMode) {
            case 0:
                mode = render_pause_courses_and_castle();
                break;
            case 1:
                mode = render_pause_courses_and_castle();
                break;
            case 2:
                mode = render_course_complete_screen();
                break;
            case 3:
                mode = render_course_complete_screen();
                break;
        }

        gDialogColorFadeTimer = (s16) gDialogColorFadeTimer + 0x1000;
    } else if (gDialogID != DIALOG_NONE) {
        // The Peach "Dear Mario" message needs to be repositioned separately
        if (gDialogID == DIALOG_020) {
            print_peach_letter_message();
            return mode;
        }

        render_dialog_entries();
        gDialogColorFadeTimer = (s16) gDialogColorFadeTimer + 0x1000;
    }
    return mode;
}

void set_min_dialog_width(s16 width) {
    gDialogMinWidth = width;
}

void set_dialog_override_pos(s16 x, s16 y) {
    gOverrideDialogPos = 1;
    gDialogOverrideX = x;
    gDialogOverrideY = y;
}

void reset_dialog_override_pos(void) {
    gOverrideDialogPos = 0;
}

void set_dialog_override_color(u8 bgR, u8 bgG, u8 bgB, u8 bgA, u8 textR, u8 textG, u8 textB, u8 textA) {
    gOverrideDialogColor = 1;
    gDialogBgColorR = bgR;
    gDialogBgColorG = bgG;
    gDialogBgColorB = bgB;
    gDialogBgColorA = bgA;
    gDialogTextColorR = textR;
    gDialogTextColorG = textG;
    gDialogTextColorB = textB;
    gDialogTextColorA = textA;
}

void reset_dialog_override_color(void) {
    gOverrideDialogColor = 0;
}

void set_dialog_box_state(u8 state) {
    if (state > DIALOG_STATE_CLOSING) { return; }
    gDialogBoxState = state;
}
