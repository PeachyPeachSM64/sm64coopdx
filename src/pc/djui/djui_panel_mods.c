#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "pc/network/network.h"
#include "pc/utils/misc.h"
#include "pc/configfile.h"
#include "pc/mods/mods.h"
#include "pc/mods/mods_utils.h"
#include "pc/mods/mod_fs.h"
#include "djui_panel_pause.h"
#include "pc/thread.h"
#include "djui_panel_mods.h"
#include "sm64.h"
#include "pc/lua/smlua.h"
#include "data/dynos.c.h"
#include "game/segment2.h"
#include "game/camera.h"
#include "game/level_update.h"
#include "game/ingame_menu.h"
#include "game/sound_init.h"

extern ALIGNED8 const u8 texture_hud_char_mario_head[];

#define DJUI_MOD_PANEL_WIDTH (410.0f + (16 * 2.0f))
#define MOD_CATEGORY_ALL 0
#define MOD_CATEGORY_MISC 1
#define MOD_CATEGORY_START 2

static struct DjuiFlowLayout* sModLayout = NULL;
static struct DjuiThreePanel* sDescriptionPanel = NULL;
static struct DjuiText* sTooltip = NULL;
static struct DjuiBase* sPreview = NULL;
static struct DjuiBase* sDescriptionBg = NULL;
static struct DjuiPaginated* sModPaginated = NULL;
static struct DjuiButton* sBackButton = NULL;
static struct DjuiButton* sRefreshButton = NULL;
static unsigned int sSelectedCategory = MOD_CATEGORY_ALL;
static bool sWarned = false;
static char sWarnedIconMods[16][SYS_MAX_PATH] = { 0 };
static u8 sWarnedIconModsIndex = 0;

extern f32 gRenderingDelta;

#define DJUI_MOD_PREVIEW_MAX (8)
#define DJUI_MOD_DESCRIPTION_BOX_HEIGHT (170.0f)

struct DjuiModPreview {
    struct DjuiBase base;
    s32 modIndex;
    u8 previewCount;
    struct TextureInfo previews[DJUI_MOD_PREVIEW_MAX];
};

static bool djui_mod_card_file_exists(const char* path) {
    if (path == NULL || path[0] == '\0') { return false; }
    FILE* f = fopen(path, "rb");
    if (f == NULL) { return false; }
    fclose(f);
    return true;
}

static struct DjuiBase* djui_find_tagged_child_recursive(struct DjuiBase* base, s32 tag) {
    if (base == NULL) { return NULL; }
    struct DjuiBaseChild* child = base->child;
    while (child != NULL) {
        if (child->base != NULL) {
            if (child->base->bTag && child->base->tag == tag) {
                return child->base;
            }
            struct DjuiBase* found = djui_find_tagged_child_recursive(child->base, tag);
            if (found != NULL) { return found; }
        }
        child = child->next;
    }
    return NULL;
}

static void djui_mod_card_build_icon_texture_name(const struct Mod* mod, char* outName, size_t outNameSize) {
    if (outName == NULL || outNameSize == 0) { return; }
    outName[0] = '\0';
    if (mod == NULL) { return; }

    // build a stable, sanitized dynos texture name
    // NOTE: dynos texture names are treated like identifiers, so avoid slashes and other punctuation
    snprintf(outName, outNameSize, "mod_icon_%s", mod->relativePath);
    for (size_t i = 0; outName[i] != '\0'; i++) {
        if (!isalnum((unsigned char)outName[i])) {
            outName[i] = '_';
        }
    }
}

static void djui_mod_preview_build_texture_name(const struct Mod* mod, u8 previewIndex, char* outName, size_t outNameSize) {
    if (outName == NULL || outNameSize == 0) { return; }
    outName[0] = '\0';
    if (mod == NULL) { return; }
    snprintf(outName, outNameSize, "mod_preview_%s_%u", mod->relativePath, (unsigned int)previewIndex);
    for (size_t i = 0; outName[i] != '\0'; i++) {
        if (!isalnum((unsigned char)outName[i])) {
            outName[i] = '_';
        }
    }
}

static void djui_mod_preview_clear(struct DjuiModPreview* preview) {
    if (preview == NULL) { return; }
    preview->modIndex = -1;
    preview->previewCount = 0;
    memset(preview->previews, 0, sizeof(preview->previews));
    djui_base_set_visible(&preview->base, false);
}

static bool djui_mod_preview_try_load_one(struct Mod* mod, u8 previewIndex, struct TextureInfo* outTi) {
    if (mod == NULL || outTi == NULL) { return false; }

    char texName[SYS_MAX_PATH] = { 0 };
    djui_mod_preview_build_texture_name(mod, previewIndex, texName, sizeof(texName));
    if (texName[0] == '\0') { return false; }

    if (dynos_texture_get(texName, outTi)) {
        return true;
    }

    // try .tex first, then .png
    const char* exts[] = { "tex", "png" };
    for (u32 e = 0; e < (sizeof(exts) / sizeof(exts[0])); e++) {
        char fileRel[64] = { 0 };
        snprintf(fileRel, sizeof(fileRel), "textures/preview%u.%s", (unsigned int)previewIndex, exts[e]);

        char previewPath[SYS_MAX_PATH] = { 0 };
        char previewResourcePath[SYS_MAX_PATH] = { 0 };
        char previewModFsPath[SYS_MAX_PATH] = { 0 };

        if (mod->isDirectory) {
            if (!concat_path(previewPath, mod->basePath, fileRel)) { return false; }

            char modsPath[SYS_MAX_PATH] = { 0 };
            snprintf(modsPath, SYS_MAX_PATH, "%s/%s", sys_resource_path(), MOD_DIRECTORY);
            char modFolder[SYS_MAX_PATH] = { 0 };
            if (concat_path(modFolder, modsPath, mod->relativePath)) {
                concat_path(previewResourcePath, modFolder, fileRel);
            }

            if (djui_mod_card_file_exists(previewPath)) {
                if (dynos_add_texture(previewPath, texName) && dynos_texture_get(texName, outTi)) {
                    return true;
                }
                // if it exists but failed to load, don't continue trying other extensions
                return false;
            }

            if (previewResourcePath[0] != '\0' && djui_mod_card_file_exists(previewResourcePath)) {
                if (dynos_add_texture(previewResourcePath, texName) && dynos_texture_get(texName, outTi)) {
                    return true;
                }
                return false;
            }

            continue;
        }

        // packed mod: ModFS URI
        snprintf(previewModFsPath, SYS_MAX_PATH, MOD_FS_URI_FORMAT, mod->relativePath, fileRel);
        if (dynos_add_texture(previewModFsPath, texName) && dynos_texture_get(texName, outTi)) {
            return true;
        }
    }

    return false;
}

static void djui_mod_preview_set_mod_index(struct DjuiModPreview* preview, s32 modIndex) {
    if (preview == NULL) { return; }

    if (modIndex < 0 || modIndex >= gLocalMods.entryCount) {
        djui_mod_preview_clear(preview);
        return;
    }

    if (preview->modIndex == modIndex) {
        return;
    }

    preview->modIndex = modIndex;
    preview->previewCount = 0;
    memset(preview->previews, 0, sizeof(preview->previews));

    struct Mod* mod = gLocalMods.entries[modIndex];
    if (mod == NULL) {
        djui_mod_preview_clear(preview);
        return;
    }

    for (u8 i = 1; i <= DJUI_MOD_PREVIEW_MAX; i++) {
        struct TextureInfo ti = { 0 };
        if (!djui_mod_preview_try_load_one(mod, i, &ti)) {
            break;
        }
        preview->previews[preview->previewCount++] = ti;
    }

    djui_base_set_visible(&preview->base, preview->previewCount > 0);
}

static bool djui_mod_preview_render(struct DjuiBase* base) {
    struct DjuiModPreview* preview = (struct DjuiModPreview*)base;
    struct DjuiBaseRect* comp = &base->comp;

    if (preview == NULL || preview->previewCount == 0) {
        return false;
    }

    // translate position
    f32 translatedX = comp->x;
    f32 translatedY = comp->y;
    djui_gfx_position_translate(&translatedX, &translatedY);
    create_dl_translation_matrix(DJUI_MTX_PUSH, translatedX, translatedY, 0);

    // translate size
    f32 translatedWidth = comp->width;
    f32 translatedHeight = comp->height;
    djui_gfx_scale_translate(&translatedWidth, &translatedHeight);

    if (!djui_gfx_add_clipping(base)) {
        // timing
        const f32 cycleFrames = 300.0f;
        const f32 fadeFrames = 60.0f;
        const f32 panFrames = 2400.0f;

        f32 t = (f32)gGlobalTimer + gRenderingDelta;
        u8 idx = (u8)(((u32)floorf(t / cycleFrames)) % preview->previewCount);
        u8 nextIdx = (u8)((idx + 1) % preview->previewCount);

        f32 phase = fmodf(t, cycleFrames);
        f32 fadeT = 0;
        if (preview->previewCount > 1 && phase >= (cycleFrames - fadeFrames)) {
            fadeT = (f32)(phase - (cycleFrames - fadeFrames)) / (f32)fadeFrames;
        }

        // horizontal pan (triangle wave 0..1..0)
        f32 panPhase = fmodf(t, panFrames);
        f32 panT = (panPhase < (panFrames / 2))
            ? (f32)panPhase / (f32)(panFrames / 2)
            : 1.0f - (f32)(panPhase - (panFrames / 2)) / (f32)(panFrames / 2);

        const struct TextureInfo* a = &preview->previews[idx];
        const struct TextureInfo* b = &preview->previews[nextIdx];
        if (a->texture != NULL) {
            // compute crop region to fill the box
            f32 targetAspect = comp->height > 0 ? (comp->width / comp->height) : 1.0f;
            u32 tileW = (u32)roundf((f32)a->height * targetAspect);
            u32 tileH = a->height;
            if (tileW > a->width) {
                tileW = a->width;
                tileH = (u32)roundf((f32)a->width / targetAspect);
                if (tileH > a->height) { tileH = a->height; }
            }
            u32 maxX = (a->width > tileW) ? (a->width - tileW) : 0;
            u32 maxY = (a->height > tileH) ? (a->height - tileH) : 0;
            u32 tileX = (u32)roundf((f32)maxX * panT);
            u32 tileY = (u32)roundf((f32)maxY * 0.5f);

            f32 aspect = tileH ? ((f32)tileW / (f32)tileH) : 1.0f;
            u8 alphaA = (u8)roundf((f32)base->color.a * (1.0f - fadeT));
            gDPSetEnvColor(gDisplayListHead++, base->color.r, base->color.g, base->color.b, alphaA);
            create_dl_scale_matrix(DJUI_MTX_PUSH, translatedWidth / aspect, translatedHeight, 1.0f);
            djui_gfx_render_texture_tile(a->texture, a->width, a->height, a->format, a->size, tileX, tileY, tileW, tileH, true);
            gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

            if (preview->previewCount > 1 && fadeT > 0 && b->texture != NULL) {
                // compute crop region for next image
                tileW = (u32)roundf((f32)b->height * targetAspect);
                tileH = b->height;
                if (tileW > b->width) {
                    tileW = b->width;
                    tileH = (u32)roundf((f32)b->width / targetAspect);
                    if (tileH > b->height) { tileH = b->height; }
                }
                maxX = (b->width > tileW) ? (b->width - tileW) : 0;
                maxY = (b->height > tileH) ? (b->height - tileH) : 0;
                tileX = (u32)roundf((f32)maxX * panT);
                tileY = (u32)roundf((f32)maxY * 0.5f);
                aspect = tileH ? ((f32)tileW / (f32)tileH) : 1.0f;
                u8 alphaB = (u8)roundf((f32)base->color.a * fadeT);
                gDPSetEnvColor(gDisplayListHead++, base->color.r, base->color.g, base->color.b, alphaB);
                create_dl_scale_matrix(DJUI_MTX_PUSH, translatedWidth / aspect, translatedHeight, 1.0f);
                djui_gfx_render_texture_tile(b->texture, b->width, b->height, b->format, b->size, tileX, tileY, tileW, tileH, true);
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
            }
        }
    }

    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    return true;
}

static void djui_mod_preview_destroy(struct DjuiBase* base) {
    struct DjuiModPreview* preview = (struct DjuiModPreview*)base;
    free(preview);
}

static struct DjuiModPreview* djui_mod_preview_create(struct DjuiBase* parent) {
    struct DjuiModPreview* preview = calloc(1, sizeof(struct DjuiModPreview));
    struct DjuiBase* base = &preview->base;
    preview->modIndex = -1;
    preview->previewCount = 0;
    djui_base_init(parent, base, djui_mod_preview_render, djui_mod_preview_destroy);
    djui_base_set_color(base, 255, 255, 255, 255);
    djui_base_set_visible(base, false);
    return preview;
}

static bool djui_mod_card_try_load_icon_png(struct Mod* mod, struct TextureInfo* outTi) {
    if (mod == NULL || outTi == NULL) { return false; }

    static const char* sIconFileTextures = "textures/icon.tex";

    char iconPathTextures[SYS_MAX_PATH] = { 0 };
    char iconPathResourceTextures[SYS_MAX_PATH] = { 0 };
    char iconPathModFsTextures[SYS_MAX_PATH] = { 0 };

    char texName[SYS_MAX_PATH] = { 0 };
    djui_mod_card_build_icon_texture_name(mod, texName, sizeof(texName));
    if (texName[0] == '\0') { return false; }

    // if already loaded, use it
    if (dynos_texture_get(texName, outTi)) {
        return true;
    }

    // attempt to load, then query again
    bool iconPathTexturesExists = false;
    bool iconPathTexturesAdded = false;
    bool iconPathResourceTexturesExists = false;
    bool iconPathResourceTexturesAdded = false;
    bool iconPathModFsTexturesAdded = false;

    if (mod->isDirectory) {
        if (!concat_path(iconPathTextures, mod->basePath, sIconFileTextures)) { return false; }

        // also try the default resource mods path (useful when the runtime resource dir differs from the repo tree)
        char modsPath[SYS_MAX_PATH] = { 0 };
        snprintf(modsPath, SYS_MAX_PATH, "%s/%s", sys_resource_path(), MOD_DIRECTORY);
        char modFolder[SYS_MAX_PATH] = { 0 };
        if (concat_path(modFolder, modsPath, mod->relativePath)) {
            concat_path(iconPathResourceTextures, modFolder, sIconFileTextures);
        }
    } else {
        snprintf(iconPathModFsTextures, SYS_MAX_PATH, MOD_FS_URI_FORMAT, mod->relativePath, sIconFileTextures);
    }

    if (iconPathTextures[0] != '\0') {
        iconPathTexturesExists = djui_mod_card_file_exists(iconPathTextures);
        if (iconPathTexturesExists) {
            iconPathTexturesAdded = dynos_add_texture(iconPathTextures, texName);
            if (iconPathTexturesAdded) {
                if (dynos_texture_get(texName, outTi)) {
                    LOG_INFO("Loaded mod icon '%s' from '%s'", texName, iconPathTextures);
                    return true;
                }
            }
        }
    }

    if (iconPathResourceTextures[0] != '\0') {
        iconPathResourceTexturesExists = djui_mod_card_file_exists(iconPathResourceTextures);
        if (iconPathResourceTexturesExists) {
            iconPathResourceTexturesAdded = dynos_add_texture(iconPathResourceTextures, texName);
            if (iconPathResourceTexturesAdded) {
                if (dynos_texture_get(texName, outTi)) {
                    LOG_INFO("Loaded mod icon '%s' from '%s'", texName, iconPathResourceTextures);
                    return true;
                }
            }
        }
    }

    if (iconPathModFsTextures[0] != '\0') {
        iconPathModFsTexturesAdded = dynos_add_texture(iconPathModFsTextures, texName);
        if (iconPathModFsTexturesAdded) {
            if (dynos_texture_get(texName, outTi)) {
                LOG_INFO("Loaded mod icon '%s' from '%s'", texName, iconPathModFsTextures);
                return true;
            }
        }
    }

    bool alreadyWarned = false;
    for (u32 i = 0; i < 16; i++) {
        if (sWarnedIconMods[i][0] != '\0' && strcmp(sWarnedIconMods[i], mod->relativePath) == 0) {
            alreadyWarned = true;
            break;
        }
    }
    if (!alreadyWarned) {
        snprintf(sWarnedIconMods[sWarnedIconModsIndex], SYS_MAX_PATH, "%s", mod->relativePath);
        sWarnedIconModsIndex = (sWarnedIconModsIndex + 1) & 15;

        LOG_ERROR("Failed to load mod icon (textures/icon.tex) for '%s'.", mod->relativePath);
        if (iconPathTextures[0] != '\0') {
            LOG_ERROR("  texturesPath='%s' exists=%d add=%d", iconPathTextures, iconPathTexturesExists, iconPathTexturesAdded);
        }
        if (iconPathResourceTextures[0] != '\0') {
            LOG_ERROR("  resourceTexturesPath='%s' exists=%d add=%d", iconPathResourceTextures, iconPathResourceTexturesExists, iconPathResourceTexturesAdded);
        }
        if (iconPathModFsTextures[0] != '\0') {
            LOG_ERROR("  modfsTexturesPath='%s' add=%d", iconPathModFsTextures, iconPathModFsTexturesAdded);
        }
    }
    return false;
}

struct ThreadHandle gModRefreshThread = { 0 };

static struct ModCategory sCategories[] = {
    // lang key, mod category
    { "ALL", NULL },
    { "MISC", NULL },
    { "ROMHACKS", "romhack" },
    { "GAMEMODES", "gamemode" },
    { "MOVESETS", "moveset" },
    { "CHARACTER_SELECT", "cs" },
    { "AUDIO", "audio" },
};
static const int sNumCategories = sizeof(sCategories) / sizeof(sCategories[0]);

static void djui_panel_mods_description_create(void) {
    struct DjuiThreePanel* panel = djui_three_panel_create(&gDjuiRoot->base, 0, 1.0f, 0);
    djui_three_panel_set_body_size_type(panel, DJUI_SVT_RELATIVE);
    struct DjuiThreePanelTheme theme = gDjuiThemes[configDjuiTheme]->threePanels;

    djui_base_set_alignment(&panel->base, DJUI_HALIGN_RIGHT, DJUI_VALIGN_CENTER);
    djui_base_set_size_type(&panel->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_RELATIVE);
    djui_base_set_size(&panel->base, DJUI_MOD_PANEL_WIDTH, 1.0f);
    djui_base_set_color(&panel->base, theme.rectColor.r, theme.rectColor.g, theme.rectColor.b, theme.rectColor.a);
    djui_base_set_border_color(&panel->base, theme.borderColor.r, theme.borderColor.g, theme.borderColor.b, theme.borderColor.a);
    djui_base_set_border_width(&panel->base, 8);
    djui_base_set_padding(&panel->base, 16, 16, 16, 16);
    {
        // IMPORTANT: DjuiThreePanel expects header/body/footer as the first 3 children.
        // Create a transparent header spacer so our real body becomes the second child.
        struct DjuiRect* headerSpacer = djui_rect_create(&panel->base);
        djui_base_set_size_type(&headerSpacer->base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
        djui_base_set_size(&headerSpacer->base, 1.0f, 0.0f);
        djui_base_set_color(&headerSpacer->base, 0, 0, 0, 0);

        // body container (absolute layering)
        struct DjuiRect* body = djui_rect_create(&panel->base);
        djui_base_set_alignment(&body->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);
        djui_base_set_size_type(&body->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&body->base, 1.0f, 1.0f);
        djui_base_set_color(&body->base, 0, 0, 0, 0);
        djui_base_set_padding(&body->base, 0, 0, 0, 0);

        // preview fills the entire right panel body (large rectangle)
        struct DjuiModPreview* preview = djui_mod_preview_create(&body->base);
        djui_base_set_size_type(&preview->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&preview->base, 1.0f, 1.0f);
        djui_base_set_alignment(&preview->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);
        sPreview = &preview->base;

        // description overlay fills same box as preview (full area)
        struct DjuiRect* descriptionOverlay = djui_rect_create(&body->base);
        djui_base_set_size_type(&descriptionOverlay->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&descriptionOverlay->base, 1.0f, 1.0f);
        djui_base_set_color(&descriptionOverlay->base, 0, 0, 0, 0);
        djui_base_set_border_width(&descriptionOverlay->base, 0);
        djui_base_set_padding(&descriptionOverlay->base, 0, 0, 0, 0);

        // top text background (only behind the text)
        struct DjuiRect* descriptionBg = djui_rect_create(&descriptionOverlay->base);
        djui_base_set_size_type(&descriptionBg->base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
        djui_base_set_size(&descriptionBg->base, 1.0f, DJUI_MOD_DESCRIPTION_BOX_HEIGHT);
        djui_base_set_color(&descriptionBg->base, 0, 0, 0, 110);
        djui_base_set_border_width(&descriptionBg->base, 2);
        djui_base_set_border_color(&descriptionBg->base, theme.borderColor.r, theme.borderColor.g, theme.borderColor.b, theme.borderColor.a);
        djui_base_set_padding(&descriptionBg->base, 12, 12, 12, 12);
        djui_base_set_alignment(&descriptionBg->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_TOP);
        djui_base_set_location_type(&descriptionBg->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
        djui_base_set_location(&descriptionBg->base, 0, 0);
        sDescriptionBg = &descriptionBg->base;

        struct DjuiText* description = djui_text_create(&descriptionBg->base, "");
        djui_base_set_size_type(&description->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&description->base, 1.0f, 1.0f);
        djui_base_set_color(&description->base, 222, 222, 222, 255);
        djui_text_set_alignment(description, DJUI_HALIGN_LEFT, DJUI_VALIGN_TOP);
        djui_text_set_drop_shadow(description, 64, 64, 64, 100);
        sTooltip = description;
    }
    sDescriptionPanel = panel;
}

static void djui_mod_checkbox_on_hover(struct DjuiBase* base) {
    char* description = "";
    if (base->tag >= 0 && base->tag < gLocalMods.entryCount) {
        struct Mod* mod = gLocalMods.entries[base->tag];
        char* d = mod->description;
        if (d != NULL) {
            description = mod->description;
        }
    }
    djui_text_set_text(sTooltip, description);

    if (sDescriptionBg != NULL && sTooltip != NULL && sDescriptionPanel != NULL) {
        // measure and resize description background for long descriptions
        djui_base_compute_tree(&sDescriptionPanel->base);

        // compute line count using the tooltip's current computed width
        u16 maxLines = 64;
        u16 lines = (u16)djui_text_count_lines(sTooltip, maxLines);
        f32 lineHeight = (f32)sTooltip->font->lineHeight * sTooltip->fontScale;
        f32 textHeight = lines * lineHeight;
        f32 padding = 12.0f * 2.0f;
        f32 desired = textHeight + padding;

        // clamp to a portion of the right panel height
        f32 panelHeight = sDescriptionPanel->base.comp.height;
        f32 maxH = fmaxf(DJUI_MOD_DESCRIPTION_BOX_HEIGHT, panelHeight * 0.55f);
        f32 minH = DJUI_MOD_DESCRIPTION_BOX_HEIGHT;
        desired = fminf(fmaxf(desired, minH), maxH);

        djui_base_set_size_type(sDescriptionBg, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
        djui_base_set_size(sDescriptionBg, 1.0f, desired);
    }

    if (sPreview != NULL) {
        djui_mod_preview_set_mod_index((struct DjuiModPreview*)sPreview, base->tag);
    }
}

static void djui_mod_checkbox_on_hover_end(UNUSED struct DjuiBase* base) {
    djui_text_set_text(sTooltip, "");

    if (sDescriptionBg != NULL) {
        djui_base_set_size_type(sDescriptionBg, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
        djui_base_set_size(sDescriptionBg, 1.0f, DJUI_MOD_DESCRIPTION_BOX_HEIGHT);
    }

    if (sPreview != NULL) {
        djui_mod_preview_clear((struct DjuiModPreview*)sPreview);
    }
}

static void djui_mod_checkbox_on_value_change(struct DjuiBase* base);

static void djui_mod_card_update_style(struct DjuiBase* base) {
    if (base == NULL) { return; }
    if (base->tag < 0 || base->tag >= gLocalMods.entryCount) { return; }
    struct Mod* mod = gLocalMods.entries[base->tag];
    if (mod == NULL) { return; }

    struct DjuiTheme* theme = gDjuiThemes[configDjuiTheme];
    struct DjuiColor tcEnabled = theme->interactables.textColor;

    struct DjuiRect* checkboxRect = (struct DjuiRect*)djui_find_tagged_child_recursive(base, 1);
    struct DjuiRect* checkboxValueRect = (struct DjuiRect*)djui_find_tagged_child_recursive(base, 2);
    struct DjuiText* nameText = (struct DjuiText*)djui_find_tagged_child_recursive(base, 3);
    struct DjuiText* authorText = (struct DjuiText*)djui_find_tagged_child_recursive(base, 4);
    struct DjuiRect* bgRect = (struct DjuiRect*)djui_find_tagged_child_recursive(base, 5);

    if (!base->enabled) {
        struct DjuiColor bc = djui_theme_shade_color(theme->interactables.defaultBorderColor, 0.6f);
        struct DjuiColor rc = djui_theme_shade_color(theme->interactables.defaultRectColor, 0.6f);
        struct DjuiColor tc = djui_theme_shade_color(theme->interactables.textColor, 0.6f);
        djui_base_set_border_color(base, bc.r, bc.g, bc.b, bc.a);
        if (bgRect) { djui_base_set_color(&bgRect->base, rc.r, rc.g, rc.b, rc.a); }
        if (nameText) { djui_base_set_color(&nameText->base, tc.r, tc.g, tc.b, tc.a); }
        if (authorText) { djui_base_set_color(&authorText->base, tc.r, tc.g, tc.b, tc.a); }
        if (checkboxRect) {
            djui_base_set_border_color(&checkboxRect->base, bc.r, bc.g, bc.b, bc.a);
            djui_base_set_color(&checkboxRect->base, 0, 0, 0, 0);
        }
        if (checkboxValueRect) { djui_base_set_color(&checkboxValueRect->base, tc.r, tc.g, tc.b, tc.a); }
    } else if (gDjuiCursorDownOn == base) {
        struct DjuiColor bc = theme->interactables.cursorDownBorderColor;
        struct DjuiColor rc = theme->interactables.cursorDownRectColor;
        djui_base_set_border_color(base, bc.r, bc.g, bc.b, bc.a);
        if (bgRect) { djui_base_set_color(&bgRect->base, rc.r, rc.g, rc.b, rc.a); }
        if (nameText) { djui_base_set_color(&nameText->base, tcEnabled.r, tcEnabled.g, tcEnabled.b, tcEnabled.a); }
        if (authorText) { djui_base_set_color(&authorText->base, tcEnabled.r, tcEnabled.g, tcEnabled.b, tcEnabled.a); }
        if (checkboxRect) { djui_base_set_border_color(&checkboxRect->base, bc.r, bc.g, bc.b, bc.a); }
        if (checkboxValueRect) { djui_base_set_color(&checkboxValueRect->base, 255, 255, 255, 255); }
    } else if (gDjuiHovered == base) {
        struct DjuiColor bc = theme->interactables.hoveredBorderColor;
        struct DjuiColor rc = theme->interactables.hoveredRectColor;
        djui_base_set_border_color(base, bc.r, bc.g, bc.b, bc.a);
        if (bgRect) { djui_base_set_color(&bgRect->base, rc.r, rc.g, rc.b, rc.a); }
        if (nameText) { djui_base_set_color(&nameText->base, tcEnabled.r, tcEnabled.g, tcEnabled.b, tcEnabled.a); }
        if (authorText) { djui_base_set_color(&authorText->base, tcEnabled.r, tcEnabled.g, tcEnabled.b, tcEnabled.a); }
        if (checkboxRect) { djui_base_set_border_color(&checkboxRect->base, bc.r, bc.g, bc.b, bc.a); }
        if (checkboxValueRect) { djui_base_set_color(&checkboxValueRect->base, 229, 241, 251, 255); }
    } else {
        struct DjuiColor bc = theme->interactables.defaultBorderColor;
        struct DjuiColor rc = theme->interactables.defaultRectColor;
        djui_base_set_border_color(base, bc.r, bc.g, bc.b, bc.a);
        if (bgRect) { djui_base_set_color(&bgRect->base, rc.r, rc.g, rc.b, rc.a); }
        if (nameText) { djui_base_set_color(&nameText->base, tcEnabled.r, tcEnabled.g, tcEnabled.b, tcEnabled.a); }
        if (authorText) { djui_base_set_color(&authorText->base, tcEnabled.r, tcEnabled.g, tcEnabled.b, tcEnabled.a); }
        if (checkboxRect) { djui_base_set_border_color(&checkboxRect->base, bc.r, bc.g, bc.b, bc.a); }
        if (checkboxValueRect) { djui_base_set_color(&checkboxValueRect->base, 220, 220, 220, 255); }
    }

    if (checkboxValueRect) {
        djui_base_set_visible(&checkboxValueRect->base, mod->enabled);
    }
}

static void djui_mod_card_on_cursor_down_begin(struct DjuiBase* base, UNUSED bool inputCursor) {
    if (base == NULL) { return; }
    if (base->tag < 0 || base->tag >= gLocalMods.entryCount) { return; }
    struct Mod* mod = gLocalMods.entries[base->tag];
    if (mod == NULL) { return; }

    mod->enabled = !mod->enabled;
    djui_mod_checkbox_on_value_change(base);
}

static struct DjuiBase* djui_mod_card_create(struct DjuiBase* parent, struct Mod* mod, int modIndex) {
    struct DjuiRect* card = djui_rect_create(parent);
    struct DjuiBase* base = &card->base;
    base->tag = modIndex;

    struct DjuiTheme* theme = gDjuiThemes[configDjuiTheme];

    djui_base_set_size_type(base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(base, 1.0f, 64);
    djui_base_set_padding(base, 0, 0, 0, 0);
    djui_base_set_border_width(base, 2);
    djui_base_set_gradient(base, false);
    djui_base_set_color(base, 0, 0, 0, 0);

    djui_interactable_create(base, djui_mod_card_update_style);
    djui_interactable_hook_cursor_down(base, djui_mod_card_on_cursor_down_begin, NULL, NULL);
    djui_interactable_hook_hover(base, djui_mod_checkbox_on_hover, djui_mod_checkbox_on_hover_end);

    struct DjuiRect* bg = djui_rect_create(base);
    bg->base.bTag = true;
    bg->base.tag = 5;
    djui_base_set_size_type(&bg->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
    djui_base_set_size(&bg->base, 1.0f, 1.0f);
    djui_base_set_alignment(&bg->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);
    djui_base_set_gradient(&bg->base, configDjuiThemeGradients);
    djui_base_set_color(&bg->base, theme->interactables.defaultRectColor.r, theme->interactables.defaultRectColor.g, theme->interactables.defaultRectColor.b, theme->interactables.defaultRectColor.a);
    djui_base_set_padding(&bg->base, 12, 12, 12, 12);

    // icon
    struct TextureInfo ti = { 0 };
    bool hasIcon = false;
    if (mod != NULL && mod->icon != NULL && mod->icon[0] != '\0') {
        hasIcon = dynos_texture_get(mod->icon, &ti);
    }
    if (!hasIcon && mod != NULL) {
        hasIcon = djui_mod_card_try_load_icon_png(mod, &ti);
    }
    if (!hasIcon) {
        ti.texture = texture_hud_char_mario_head;
        ti.width = 16;
        ti.height = 16;
        ti.format = G_IM_FMT_RGBA;
        ti.size = G_IM_SIZ_16b;
    }

    struct DjuiImage* icon = djui_image_create(&bg->base, ti.texture, ti.width, ti.height, ti.format, ti.size);
    djui_base_set_size(&icon->base, 55, 55);
    djui_base_set_alignment(&icon->base, DJUI_HALIGN_LEFT, DJUI_VALIGN_CENTER);
    djui_base_set_location_type(&icon->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_location(&icon->base, -9, 0);

    // text area (sized like DjuiButton to avoid tiny maxLineWidth / ellipses)
    struct DjuiRect* textArea = djui_rect_create(&bg->base);
    djui_base_set_color(&textArea->base, 0, 0, 0, 0);
    djui_base_set_location_type(&textArea->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_location(&textArea->base, 55, 0);
    djui_base_set_size_type(&textArea->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
    djui_base_set_size(&textArea->base, 1.0f, 1.0f);

    const char* name = (mod != NULL) ? mod->name : "";
    const char* author = "";
    if (mod != NULL) {
        if (mod->author != NULL) {
            author = mod->author;
        } else if (mod->shortDescription != NULL) {
            author = mod->shortDescription;
        }
    }

    struct DjuiText* nameText = djui_text_create(&textArea->base, name);
    nameText->base.bTag = true;
    nameText->base.tag = 3;
    djui_base_set_size_type(&nameText->base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(&nameText->base, 1.0f, 28);
    djui_base_set_location_type(&nameText->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_location(&nameText->base, 0, -6);
    djui_base_set_color(&nameText->base, theme->interactables.textColor.r, theme->interactables.textColor.g, theme->interactables.textColor.b, theme->interactables.textColor.a);
    djui_text_set_alignment(nameText, DJUI_HALIGN_LEFT, DJUI_VALIGN_TOP);
    djui_text_set_font_scale(nameText, nameText->font->defaultFontScale * 0.9075f);
    djui_text_set_drop_shadow(nameText, 64, 64, 64, 100);

    struct DjuiText* authorText = djui_text_create(&textArea->base, author);
    authorText->base.bTag = true;
    authorText->base.tag = 4;
    djui_base_set_size_type(&authorText->base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(&authorText->base, 1.0f, 26);
    djui_base_set_color(&authorText->base, theme->interactables.textColor.r, theme->interactables.textColor.g, theme->interactables.textColor.b, theme->interactables.textColor.a);
    djui_base_set_location_type(&authorText->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_location(&authorText->base, 0, 18);
    djui_text_set_alignment(authorText, DJUI_HALIGN_LEFT, DJUI_VALIGN_TOP);
    djui_text_set_font_scale(authorText, authorText->font->defaultFontScale * 0.5665f);
    djui_text_set_drop_shadow(authorText, 64, 64, 64, 100);

    // enable indicator (checkbox)
    struct DjuiRect* cb = djui_rect_create(&bg->base);
    cb->base.bTag = true;
    cb->base.tag = 1;
    djui_base_set_alignment(&cb->base, DJUI_HALIGN_RIGHT, DJUI_VALIGN_CENTER);
    djui_base_set_size_type(&cb->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(&cb->base, 18, 18);
    djui_base_set_border_width(&cb->base, 2);
    djui_base_set_color(&cb->base, 0, 0, 0, 0);

    struct DjuiRect* cbv = djui_rect_create(&cb->base);
    cbv->base.bTag = true;
    cbv->base.tag = 2;
    djui_base_set_alignment(&cbv->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);
    djui_base_set_size_type(&cbv->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(&cbv->base, 10, 10);
    djui_base_set_visible(&cbv->base, mod != NULL && mod->enabled);

    djui_mod_card_update_style(base);
    return base;
}

static void djui_mod_checkbox_on_value_change(UNUSED struct DjuiBase* base) {
    mods_update_selectable();

    if (mods_get_enabled_count() - mods_get_character_select_count() >= 10) {
        if (!sWarned) {
            sWarned = true;
            djui_popup_create(DLANG(HOST_MODS, WARNING), 3);
        }
    } else {
        sWarned = false;
    }

    u16 index = 0;
    struct DjuiBaseChild* node = sModLayout->base.child;
    while (node != NULL) {
        index = node->base->tag;
        if (index >= gLocalMods.entryCount) { break; }
        struct Mod* mod = gLocalMods.entries[index];

        djui_base_set_enabled(node->base, mod->selectable);

        // iterate
        node = node->next;
    }
}

static void djui_panel_mods_destroy(struct DjuiBase* base) {
    struct DjuiThreePanel* threePanel = (struct DjuiThreePanel*)base;
    free(threePanel);

    if (sDescriptionPanel != NULL) {
        djui_base_destroy(&sDescriptionPanel->base);
        sDescriptionPanel = NULL;
    }
    sModLayout = NULL;
    sTooltip = NULL;
}

static void djui_panel_mods_add_mods(struct DjuiBase* layoutBase) {
    bool foundAny = false;
    for (int i = 0; i < gLocalMods.entryCount; i++) {
        struct Mod* mod = gLocalMods.entries[i];
        char* category = mod->category != NULL ? mod->category : mod->incompatible;
        switch (sSelectedCategory) {
            case MOD_CATEGORY_ALL: { break; }
            case MOD_CATEGORY_MISC: {
                bool doContinue = false;
                if (category) {
                    for (int i = MOD_CATEGORY_START; i < sNumCategories; i++) {
                        if (strstr(category, sCategories[i].category)) {
                            doContinue = true;
                            break;
                        }
                    }
                }
                if (doContinue) { continue; }
                break;
            }
            default: {
                if (!category || !strstr(category, sCategories[sSelectedCategory].category)) {
                    continue;
                }
                break;
            }
        }
        struct DjuiBase* card = djui_mod_card_create(layoutBase, mod, i);
        djui_base_set_enabled(card, mod->selectable);
        foundAny = true;
    }
    if (!foundAny) {
        struct DjuiText* text = djui_text_create(layoutBase, DLANG(HOST_MODS, NO_MODS_FOUND));
        djui_base_set_size_type(&text->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&text->base, 1, 1);
        djui_text_set_alignment(text, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);
        djui_text_set_drop_shadow(text, 64, 64, 64, 100);
    }
}

static void djui_panel_on_categories_change(UNUSED struct DjuiBase* caller) {
    if (gModRefreshThread.state == RUNNING) { return; }
    djui_base_destroy_children(&sModLayout->base);
    djui_panel_mods_add_mods(&sModLayout->base);
    djui_paginated_calculate_height(sModPaginated);
}

static void* threaded_mod_refresh(UNUSED void* unused) {
    mods_refresh_local();

    if (gModRefreshThread.state == RUNNING) { join_thread(&gModRefreshThread); }

    mods_update_selectable();
    djui_panel_mods_add_mods(&sModLayout->base);
    djui_paginated_calculate_height(sModPaginated);

    djui_text_set_text(sRefreshButton->text, DLANG(LOBBIES, REFRESH));
    djui_base_set_enabled(&sRefreshButton->base, true);
    djui_base_set_enabled(&sBackButton->base, true);
    gDjuiPanelDisableBack = false;

    return NULL;
}

static void djui_panel_menu_refresh(UNUSED struct DjuiBase* base) {
    djui_base_destroy_children(&sModLayout->base);
    if (init_thread_handle(&gModRefreshThread, threaded_mod_refresh, NULL, NULL, 0) == 0) {
        djui_text_set_text(sRefreshButton->text, DLANG(LOBBIES, REFRESHING));
        djui_base_set_enabled(&sRefreshButton->base, false);
        djui_base_set_enabled(&sBackButton->base, false);
        gDjuiPanelDisableBack = true;
    } else {
        threaded_mod_refresh(NULL);
    }
}

static void djui_panel_menu_restart_game(UNUSED struct DjuiBase* base) {
    if (gMarioStates[0].action == ACT_PUSHING_DOOR || gMarioStates[0].action == ACT_PULLING_DOOR) { return; }

    configfile_save(configfile_name());

    smlua_shutdown();
    mods_activate(&gLocalMods);
    smlua_init();

    extern s16 gPauseScreenMode;
    raise_background_noise(1);
    gCameraMovementFlags &= ~CAM_MOVE_PAUSE_SCREEN;
    gPauseScreenMode = 0;
    set_menu_mode(-1);
    set_play_mode(PLAY_MODE_NORMAL);

    djui_panel_shutdown();
    fade_into_special_warp(SPECIAL_WARP_GODDARD, 0);
}

void djui_panel_mods_create(struct DjuiBase* caller) {
    mods_update_selectable();
    djui_panel_mods_description_create();

    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(HOST_MODS, MODS), true);

    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        // copy category choices from sCategories
        char* categoryChoices[sizeof(sCategories)];

        // loop thru all categories names, and add those to the categoryChoices string array
        for (int i = 0; i < sNumCategories; i++) {
            categoryChoices[i] = djui_language_get("HOST_MOD_CATEGORIES", sCategories[i].langKey);
        }
        djui_selectionbox_create(body, DLANG(HOST_MODS, CATEGORIES), categoryChoices, sNumCategories, &sSelectedCategory, djui_panel_on_categories_change);
        struct DjuiPaginated* paginated = djui_paginated_create(body, 8);
        paginated->showMaxCount = true;
        sModLayout = paginated->layout;
        djui_panel_mods_add_mods(&paginated->layout->base);
        djui_paginated_calculate_height(paginated);
        sModPaginated = paginated;

        djui_button_create(body, "Restart", DJUI_BUTTON_STYLE_NORMAL, djui_panel_menu_restart_game);
        struct DjuiRect* rect1 = djui_rect_container_create(body, 64);
        {
            sBackButton = djui_button_left_create(&rect1->base, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
            sRefreshButton = djui_button_right_create(&rect1->base, DLANG(LOBBIES, REFRESH), DJUI_BUTTON_STYLE_NORMAL, djui_panel_menu_refresh);
        }

        panel->bodySize.value = paginated->base.height.value + 64 + 64 + 64;
    }

    panel->base.destroy = djui_panel_mods_destroy;

    djui_panel_add(caller, panel, NULL);
}
