#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "djui_panel_pause.h"
#include "pc/cheats.h"

static void djui_panel_cheats_toggle_save(UNUSED struct DjuiBase* base) {
    // no-op for now
}

static void djui_panel_cheats_classic_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(CHEATS, CHEATS_CLASSIC_TITLE), false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        djui_checkbox_create(body, DLANG(CHEATS, MOON_JUMP), &Cheats.MoonJump, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, GOD_MODE), &Cheats.GodMode, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, INFINITE_LIVES), &Cheats.InfiniteLives, djui_panel_cheats_toggle_save);

        djui_checkbox_create(body, DLANG(CHEATS, MOON_GRAVITY), &Cheats.MoonGravity, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, DEBUG_MOVE), &Cheats.DebugMove, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, SUPER_COPTER), &Cheats.SuperCopter, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, AUTO_WALL_KICK), &Cheats.AutoWallKick, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, NO_HOLD_HEAVY), &Cheats.NoHoldHeavy, djui_panel_cheats_toggle_save);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}

static void djui_panel_cheats_modifiers_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(CHEATS, CHEATS_MODIFIERS_TITLE), false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        char* modifierChoices[5] = { "x1", "x2", "x3", "x4", "x5" };
        djui_selectionbox_create(body, DLANG(CHEATS, SPEED_MODIFIER), modifierChoices, 5, (unsigned int*)&Cheats.SpeedModifier, NULL);
        djui_selectionbox_create(body, DLANG(CHEATS, JUMP_MODIFIER), modifierChoices, 5, (unsigned int*)&Cheats.JumpModifier, NULL);
        djui_selectionbox_create(body, DLANG(CHEATS, SWIM_MODIFIER), modifierChoices, 5, (unsigned int*)&Cheats.SwimModifier, NULL);
        djui_checkbox_create(body, DLANG(CHEATS, SPEED_DISPLAY), &Cheats.SpeedDisplay, djui_panel_cheats_toggle_save);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}

static void djui_panel_cheats_time_space_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(CHEATS, CHEATS_TIME_SPACE_TITLE), false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        char* bljChoices[13] = { "Disabled", "x1", "x2", "x3", "x4", "x5", "x6", "Hold x1", "Hold x2", "Hold x3", "Hold x4", "Hold x5", "Hold x6" };
        djui_selectionbox_create(body, DLANG(CHEATS, BLJ_ANYWHERE), bljChoices, 13, (unsigned int*)&Cheats.BLJAnywhere, NULL);

        djui_checkbox_create(body, DLANG(CHEATS, SWIM_ANYWHERE), &Cheats.SwimAnywhere, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, WALK_ON_HAZARDS), &Cheats.WalkOnHazards, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, COINS_MAGNET), &Cheats.CoinsMagnet, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, TIME_STOP), &Cheats.TimeStop, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, DLANG(CHEATS, QUICK_ENDING), &Cheats.QuickEnding, djui_panel_cheats_toggle_save);

        char* waterChoices[3] = { "Default", "Drained", "Flooded" };
        djui_selectionbox_create(body, DLANG(CHEATS, WATER_LEVEL), waterChoices, 3, (unsigned int*)&Cheats.WaterLevel, NULL);

        char* hurtChoices[8] = { "Disabled", "Knockback", "Shock", "Burn", "Lava Boost", "Squish", "Stuck", "1 HP" };
        djui_selectionbox_create(body, DLANG(CHEATS, HURT_MARIO), hurtChoices, 8, (unsigned int*)&Cheats.HurtMario, NULL);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}

void djui_panel_cheats_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(CHEATS, CHEATS_TITLE), false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        djui_checkbox_create(body, DLANG(CHEATS, ENABLE_CHEATS), &Cheats.EnableCheats, djui_panel_cheats_toggle_save);

        djui_button_create(body, DLANG(CHEATS, CLASSIC), DJUI_BUTTON_STYLE_NORMAL, djui_panel_cheats_classic_create);
        djui_button_create(body, DLANG(CHEATS, MODIFIERS), DJUI_BUTTON_STYLE_NORMAL, djui_panel_cheats_modifiers_create);
        djui_button_create(body, DLANG(CHEATS, TIME_SPACE), DJUI_BUTTON_STYLE_NORMAL, djui_panel_cheats_time_space_create);

        djui_button_create(body, DLANG(CHEATS, WARP_TO_LEVEL), DJUI_BUTTON_STYLE_NORMAL, djui_panel_pause_warp_to_level_create);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}
