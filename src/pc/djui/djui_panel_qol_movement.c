#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "djui_panel_qol_movement.h"
#include "djui_paginated.h"
#include "pc/configfile.h"

static struct DjuiPaginated* sMovementPaginated = NULL;

static void djui_panel_qol_movement_paginated_pre_render(struct DjuiBase* base, bool* outSkipRender) {
    if (outSkipRender != NULL) {
        *outSkipRender = false;
    }
    struct DjuiPaginated* paginated = (struct DjuiPaginated*)base;
    if (paginated == NULL || paginated->layout == NULL || paginated->base.parent == NULL) { return; }

    // Estimate how many checkboxes can fit in the current body height.
    // Each checkbox is 32px tall and flow layout uses a 16px margin.
    f32 bodyHeight = paginated->base.parent->comp.height;
    f32 itemHeight = 32.0f;
    f32 margin = paginated->layout->margin.value;
    f32 navHeight = itemHeight + margin;
    f32 backButtonHeight = itemHeight + margin;
    f32 usable = bodyHeight - navHeight - backButtonHeight;

    s32 showCount = (s32)(usable / (itemHeight + margin));
    if (showCount < 1) { showCount = 1; }
    if (showCount > 64) { showCount = 64; }

    if (paginated->showCount != showCount) {
        paginated->showCount = showCount;
        djui_paginated_calculate_height(paginated);
    }
}

void djui_panel_qol_movement_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create("QoL - Movement", false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        sMovementPaginated = djui_paginated_create(body, 8);
        sMovementPaginated->showMaxCount = true;
        sMovementPaginated->base.on_render_pre = djui_panel_qol_movement_paginated_pre_render;

        struct DjuiBase* layoutBase = &sMovementPaginated->layout->base;
        djui_checkbox_create(layoutBase, "Fix jump kick not slippery", &configQolFixJumpKickNotSlippery, NULL);
        djui_checkbox_create(layoutBase, "Fix short hitbox slide acts", &configQolFixShortHitboxSlideActs, NULL);
        djui_checkbox_create(layoutBase, "Fix hit wall action", &configQolFixHitWallAction, NULL);
        djui_checkbox_create(layoutBase, "Fix ground pound wall", &configQolFixGroundPoundWall, NULL);
        djui_checkbox_create(layoutBase, "Fix stationary ground steps", &configQolFixStationaryGroundSteps, NULL);
        djui_checkbox_create(layoutBase, "Fix less ground bonks", &configQolFixLessGroundBonks, NULL);
        djui_checkbox_create(layoutBase, "Fix initial walking speed", &configQolFixInitialWalkingSpeed, NULL);
        djui_checkbox_create(layoutBase, "Fix slide vel update sliding", &configQolFixSlideVelUpdateSliding, NULL);
        djui_checkbox_create(layoutBase, "Fix water plunge upwarp", &configQolFixWaterPlungeUpwarp, NULL);
        djui_checkbox_create(layoutBase, "Fix ledge grab steep slopes", &configQolFixLedgeGrabSteepSlopes, NULL);
        djui_checkbox_create(layoutBase, "Fix action land eat input", &configQolFixActionLandEatInput, NULL);

        djui_checkbox_create(layoutBase, "Special triple jump air steps", &configQolSpecialTripleJumpAirSteps, NULL);
        djui_checkbox_create(layoutBase, "Hangable surface air freely", &configQolHangableSurfaceAirFreely, NULL);
        djui_checkbox_create(layoutBase, "Better hanging", &configQolBetterHanging, NULL);
        djui_checkbox_create(layoutBase, "Better flying", &configQolBetterFlying, NULL);
        djui_checkbox_create(layoutBase, "Velocity based turn speed", &configQolVelocityBasedTurnSpeed, NULL);
        djui_checkbox_create(layoutBase, "Easier long jumps", &configQolEasierLongJumps, NULL);
        djui_checkbox_create(layoutBase, "Easier jump kicks", &configQolEasierJumpKicks, NULL);
        djui_checkbox_create(layoutBase, "Ledge grab more actions", &configQolLedgeGrabMoreActions, NULL);
        djui_checkbox_create(layoutBase, "Wall kick more actions", &configQolWallKickMoreActions, NULL);
        djui_checkbox_create(layoutBase, "Twirl with object", &configQolTwirlWithObject, NULL);
        djui_checkbox_create(layoutBase, "Slide kick slide button", &configQolSlideKickSlideButton, NULL);
        djui_checkbox_create(layoutBase, "Dive slide rollout", &configQolDiveSlideRollout, NULL);
        djui_checkbox_create(layoutBase, "Disable ceiling bonks", &configQolDisableCeilingBonks, NULL);

        djui_checkbox_create(layoutBase, "Fix door key cutscene", &configQolFixDoorKeyCutscene, NULL);
        djui_checkbox_create(layoutBase, "Fix surfce wind detection", &configQolFixSurfaceWindDetection, NULL);
        djui_checkbox_create(layoutBase, "Fix lava interaction", &configQolFixLavaInteraction, NULL);
        djui_checkbox_create(layoutBase, "Smooth squish", &configQolSmoothSquish, NULL);
        djui_checkbox_create(layoutBase, "Smooth pitch when hitting floor underwater", &configQolSmoothPitchWhenHittingFloorUnderwater, NULL);
        djui_checkbox_create(layoutBase, "Ledge climb protection", &configQolLedgeClimbProtection, NULL);
        djui_checkbox_create(layoutBase, "Better exit airborne", &configQolBetterExitAirborne, NULL);

        djui_paginated_calculate_height(sMovementPaginated);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }

    djui_panel_add(caller, panel, NULL);
}
