#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "djui_panel_qol_movement.h"
#include "djui_paginated.h"
#include "pc/configfile.h"

static struct DjuiPaginated* sMovementPaginated = NULL;
static struct DjuiThreePanel* sDescriptionPanel = NULL;
static struct DjuiText* sTooltip = NULL;

static const char* djui_qol_movement_get_description(s32 index) {
    switch (index) {
        case 0: return "Fixes limited jumping on non-slippery surfaces caused by some slopes being treated as steep.\n\nHow to test: find a non-slippery slope where jumps/jump kicks feel restricted (Mario acts like he can't jump properly). With this on, jumping should behave normally.";
        case 1: return "Fixes slide actions by shortening the hitbox of the player.\n\nHow to test: crouch-slide into small ledges/steps a bunch of times. With this on, you should pop up less often.";
        case 2: return "When you smack into a wall in the air, Mario's facing/animation should look more consistent (and wall kicks feel less weird).\n\nHow to test: jump into a wall at an angle, then press A to wall kick. With this on, Mario should face the wall more reliably.";
        case 3: return "Fixes some odd bonking when ground pounding near walls.\n\nHow to test: ground pound very close to a wall/pillar. With this on, you should no longer bonk when doing so.";
        case 4: return "Helps prevent some \"standing still\" floor snap/downwarp weirdness.\n\nHow to test: stand still on uneven floors, moving ground, or near edges. With this on, Mario should snap downward less often.";
        case 5: return "Less random bonking when you're braking into a wall.\n\nHow to test: run, let go to brake, and drift into a wall. With this on, you should push/sidle more often instead of going into bonk knockback.";
        case 6: return "Starting to walk feels more consistent (no \"sometimes it barely moved\" first step).\n\nHow to test: from a standstill, lightly tap the stick repeatedly. With this on, the first step speed should be more consistent.";
        case 7: return "Makes slide speed updates a bit more consistent (less sudden speed weirdness).\n\nHow to test: slide down a slope and make small steering inputs. With this on, speed should feel less jittery/spiky.";
        case 8: return "Fixes a rare water entry glitch where you can get snapped upward when entering water oddly.\n\nHow to test: dive/ground pound into water near edges or weird geometry seams. With this on, sudden upward snaps should be less likely.";
        case 9: return "Stops you from grabbing ledges on slopes that are way too steep to make sense.\n\nHow to test: try to ledge-grab near very steep slanted surfaces. With this on, bogus grabs should happen less.";
        case 10: return "Makes buffered inputs on landing feel more reliable (less \"I pressed A and nothing happened\" moments).\n\nHow to test: do repeated jumps/long jumps and press A right as you land. With this on, the buffered jump should trigger more often.";
        case 11: return "Makes special triple jumps a bit more forgiving/consistent around tricky collisions.\n\nHow to test: do special triple jumps near walls/ledges and see if you get fewer weird cancels or wall hits.";
        case 12: return "Gives you a bit more freedom moving around hangable ceilings/surfaces while airborne.\n\nHow to test: jump up near a hangable ceiling and try moving/turning midair close to it. With this on, it should feel less restrictive.";
        case 13: return "Better ceiling hanging controls (faster to start moving, nicer turning, and easier to drop).\n\nHow to test: grab a hangable ceiling. With this on, moving starts sooner, and you can press A or B to drop.";
        case 14: return "Smoother/more controllable flying.\n\nHow to test: use a Wing Cap and try wide turns at different speeds. With this on, steering should feel smoother.";
        case 15: return "Turning adjusts based on your speed: snappier when slow, smoother when fast.\n\nHow to test: walk slowly and do a quick turn (should snap more), then run fast and turn (should be smoother/less instant).\n\n!! This patch causes the cannonless setup to fail !!";
        case 16: return "Long jumps are easier to do reliably (less picky input/timing).\n\nHow to test: repeatedly attempt long jumps from a run. With this on, you should mess them up less.";
        case 17: return "Jump kicks are easier to get consistently.\n\nHow to test: jump and press B at different timings. With this on, jump kicks should register more often.";
        case 18: return "Lets you grab ledges from more midair moves (like rollouts).\n\nHow to test: do a forward/backward rollout toward a ledge and try to grab it. With this on, the ledge grab should work from those actions.";
        case 19: return "Gives you more chances to wall kick instead of just bonking in certain cases.\n\nHow to test: do a low-speed dive/rollout into a wall and press A. With this on, you should get more wall kicks instead of a bonk.";
        case 20: return "Improves the twirl animation when you're holding an object.\n\nHow to test: pick up a light object, then twirl. With this on, the animation should look correct while holding the object.";
        case 21: return "Lets you roll out of slide-kick sliding with B as well as A.\n\nHow to test: slide kick, keep sliding, then press B. With this on, B should trigger the rollout (same as A).";
        case 22: return "Rollouts from dive-sliding feel more consistent/intentional.\n\nHow to test: dive onto the ground to start dive sliding, then press A or B during the slide. With this on, rollouts should work more reliably.";
        case 23: return "Helps avoid some annoying ceiling bonk behavior in tight spaces.\n\nHow to test: jump into a low ceiling repeatedly (tight hallway/low room). With this on, you should get fewer ceiling-bonk knockback/cancels.";
        case 24: return "Cleans up some weirdness in the door + key cutscene.\n\nHow to test: get a key and unlock a door. With this on, the cutscene/transition should feel more consistent.";
        case 25: return "Wind surfaces should trigger more reliably.\n\nHow to test: stand in a windy area and see how consistently Mario gets pushed. With this on, wind should apply more consistently.";
        case 26: return "Lava boost behavior is handled more consistently across different states.\n\nHow to test: touch lava in different ways (falling into it vs landing on it). With this on, the boost behavior should be more consistent.";
        case 27: return "Smoother-looking squish animation (less sudden scaling).\n\nHow to test: get squished by a moving platform/door. With this on, the squash/stretch should look smoother.";
        case 28: return "Less sudden pitch snapping when you hit the floor while swimming.\n\nHow to test: swim down into the pool floor at an angle. With this on, Mario's pitch should ease into the floor angle.";
        case 29: return "Helps prevent some edge-case ledge slip-offs.\n\nHow to test: walk near a platform edge while holding the stick back (especially on non-slippery ground). With this on, you should slip off less easily.";
        case 30: return "Makes certain airborne exits/transitions feel more consistent.\n\nHow to test: try cancelling out of airborne moves near the ground (dives/rollouts/bonks). With this on, transitions should feel less random.";
        default: return "";
    }
}

static void djui_panel_qol_movement_description_create(void) {
    if (sDescriptionPanel != NULL) { return; }
    f32 bodyHeight = 1000;

    struct DjuiThreePanel* panel = djui_three_panel_create(&gDjuiRoot->base, 64, bodyHeight, 0);
    struct DjuiThreePanelTheme theme = gDjuiThemes[configDjuiTheme]->threePanels;

    djui_base_set_alignment(&panel->base, DJUI_HALIGN_RIGHT, DJUI_VALIGN_CENTER);
    djui_base_set_size_type(&panel->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_RELATIVE);
    djui_base_set_size(&panel->base, 442.0f, 1.0f);
    djui_base_set_color(&panel->base, theme.rectColor.r, theme.rectColor.g, theme.rectColor.b, theme.rectColor.a);
    djui_base_set_border_color(&panel->base, theme.borderColor.r, theme.borderColor.g, theme.borderColor.b, theme.borderColor.a);
    djui_base_set_border_width(&panel->base, 8);
    djui_base_set_padding(&panel->base, 16, 16, 16, 16);
    {
        struct DjuiFlowLayout* body = djui_flow_layout_create(&panel->base);
        djui_base_set_alignment(&body->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_CENTER);
        djui_base_set_size_type(&body->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&body->base, 1.0f, 1.0f);
        djui_base_set_color(&body->base, 0, 0, 0, 0);
        djui_flow_layout_set_margin(body, 16);
        djui_flow_layout_set_flow_direction(body, DJUI_FLOW_DIR_DOWN);

        struct DjuiText* description = djui_text_create(&panel->base, "");
        djui_base_set_size_type(&description->base, DJUI_SVT_RELATIVE, DJUI_SVT_RELATIVE);
        djui_base_set_size(&description->base, 1.0f, 1.0f);
        djui_base_set_color(&description->base, 222, 222, 222, 255);
        djui_text_set_alignment(description, DJUI_HALIGN_LEFT, DJUI_VALIGN_CENTER);
        djui_text_set_drop_shadow(description, 64, 64, 64, 100);
        sTooltip = description;
    }
    sDescriptionPanel = panel;
}

static void djui_panel_qol_movement_description_set(const char* text) {
    if (sTooltip == NULL) { return; }
    djui_text_set_text(sTooltip, (text != NULL) ? text : "");
}

static void djui_qol_movement_checkbox_on_hover(struct DjuiBase* base) {
    djui_panel_qol_movement_description_set(djui_qol_movement_get_description((s32)base->tag));
}

static void djui_qol_movement_checkbox_on_hover_end(UNUSED struct DjuiBase* base) {
    djui_panel_qol_movement_description_set("");
}

static void djui_panel_qol_movement_destroy(UNUSED struct DjuiBase* caller) {
    if (sDescriptionPanel != NULL) {
        djui_base_destroy(&sDescriptionPanel->base);
        sDescriptionPanel = NULL;
    }
    sTooltip = NULL;
    sMovementPaginated = NULL;
}

static struct DjuiCheckbox* djui_checkbox_create_with_description(struct DjuiBase* parent, const char* label,
                                                                 bool* value, void (*onValueChange)(struct DjuiBase*), s32 descIndex) {
    struct DjuiCheckbox* checkbox = djui_checkbox_create(parent, label, value, onValueChange);
    checkbox->base.tag = descIndex;
    djui_interactable_hook_hover(&checkbox->base, djui_qol_movement_checkbox_on_hover, djui_qol_movement_checkbox_on_hover_end);
    return checkbox;
}

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
    djui_panel_qol_movement_description_create();
    struct DjuiThreePanel* panel = djui_panel_menu_create("QoL - Movement", true);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        sMovementPaginated = djui_paginated_create(body, 8);
        sMovementPaginated->showMaxCount = true;
        sMovementPaginated->base.on_render_pre = djui_panel_qol_movement_paginated_pre_render;

        struct DjuiBase* layoutBase = &sMovementPaginated->layout->base;

        djui_checkbox_create_with_description(layoutBase, "Fix jump kick not slippery", &configQolFixJumpKickNotSlippery, NULL, 0);
        djui_checkbox_create_with_description(layoutBase, "Fix short hitbox slide acts", &configQolFixShortHitboxSlideActs, NULL, 1);
        djui_checkbox_create_with_description(layoutBase, "Fix hit wall action", &configQolFixHitWallAction, NULL, 2);
        djui_checkbox_create_with_description(layoutBase, "Fix ground pound wall", &configQolFixGroundPoundWall, NULL, 3);
        djui_checkbox_create_with_description(layoutBase, "Fix stationary ground steps", &configQolFixStationaryGroundSteps, NULL, 4);
        djui_checkbox_create_with_description(layoutBase, "Fix less ground bonks", &configQolFixLessGroundBonks, NULL, 5);
        djui_checkbox_create_with_description(layoutBase, "Fix initial walking speed", &configQolFixInitialWalkingSpeed, NULL, 6);
        djui_checkbox_create_with_description(layoutBase, "Fix slide vel update sliding", &configQolFixSlideVelUpdateSliding, NULL, 7);
        djui_checkbox_create_with_description(layoutBase, "Fix water plunge upwarp", &configQolFixWaterPlungeUpwarp, NULL, 8);
        djui_checkbox_create_with_description(layoutBase, "Fix ledge grab steep slopes", &configQolFixLedgeGrabSteepSlopes, NULL, 9);
        djui_checkbox_create_with_description(layoutBase, "Fix action land eat input", &configQolFixActionLandEatInput, NULL, 10);

        djui_checkbox_create_with_description(layoutBase, "Special triple jump air steps", &configQolSpecialTripleJumpAirSteps, NULL, 11);
        djui_checkbox_create_with_description(layoutBase, "Hangable surface air freely", &configQolHangableSurfaceAirFreely, NULL, 12);
        djui_checkbox_create_with_description(layoutBase, "Better hanging", &configQolBetterHanging, NULL, 13);
        djui_checkbox_create_with_description(layoutBase, "Better flying", &configQolBetterFlying, NULL, 14);
        djui_checkbox_create_with_description(layoutBase, "Velocity based turn speed", &configQolVelocityBasedTurnSpeed, NULL, 15);
        djui_checkbox_create_with_description(layoutBase, "Easier long jumps", &configQolEasierLongJumps, NULL, 16);
        djui_checkbox_create_with_description(layoutBase, "Easier jump kicks", &configQolEasierJumpKicks, NULL, 17);
        djui_checkbox_create_with_description(layoutBase, "Ledge grab more actions", &configQolLedgeGrabMoreActions, NULL, 18);
        djui_checkbox_create_with_description(layoutBase, "Wall kick more actions", &configQolWallKickMoreActions, NULL, 19);
        djui_checkbox_create_with_description(layoutBase, "Twirl with object", &configQolTwirlWithObject, NULL, 20);
        djui_checkbox_create_with_description(layoutBase, "Slide kick slide button", &configQolSlideKickSlideButton, NULL, 21);
        djui_checkbox_create_with_description(layoutBase, "Dive slide rollout", &configQolDiveSlideRollout, NULL, 22);
        djui_checkbox_create_with_description(layoutBase, "Disable ceiling bonks", &configQolDisableCeilingBonks, NULL, 23);

        djui_checkbox_create_with_description(layoutBase, "Fix door key cutscene", &configQolFixDoorKeyCutscene, NULL, 24);
        djui_checkbox_create_with_description(layoutBase, "Fix surfce wind detection", &configQolFixSurfaceWindDetection, NULL, 25);
        djui_checkbox_create_with_description(layoutBase, "Fix lava interaction", &configQolFixLavaInteraction, NULL, 26);
        djui_checkbox_create_with_description(layoutBase, "Smooth squish", &configQolSmoothSquish, NULL, 27);
        djui_checkbox_create_with_description(layoutBase, "Smooth pitch when hitting floor underwater", &configQolSmoothPitchWhenHittingFloorUnderwater, NULL, 28);
        djui_checkbox_create_with_description(layoutBase, "Ledge climb protection", &configQolLedgeClimbProtection, NULL, 29);
        djui_checkbox_create_with_description(layoutBase, "Better exit airborne", &configQolBetterExitAirborne, NULL, 30);

        djui_paginated_calculate_height(sMovementPaginated);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }

    struct DjuiPanel* p = djui_panel_add(caller, panel, NULL);
    if (p != NULL) {
        p->on_panel_destroy = djui_panel_qol_movement_destroy;
    }
}
