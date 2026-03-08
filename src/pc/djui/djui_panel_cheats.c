#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "djui_panel_pause.h"
#include "pc/cheats.h"

static void djui_panel_cheats_toggle_save(UNUSED struct DjuiBase* base) {
    // no-op for now
}

static void djui_panel_cheats_classic_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create("Cheats - Classic", false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        djui_checkbox_create(body, "Moon Jump", &Cheats.MoonJump, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "God Mode", &Cheats.GodMode, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Infinite Lives", &Cheats.InfiniteLives, djui_panel_cheats_toggle_save);

        djui_checkbox_create(body, "Moon Gravity", &Cheats.MoonGravity, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Debug Move", &Cheats.DebugMove, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Super Copter", &Cheats.SuperCopter, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Auto Wall Kick", &Cheats.AutoWallKick, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "No Hold Heavy", &Cheats.NoHoldHeavy, djui_panel_cheats_toggle_save);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}

static void djui_panel_cheats_modifiers_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create("Cheats - Modifiers", false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        char* modifierChoices[5] = { "x1", "x2", "x3", "x4", "x5" };
        djui_selectionbox_create(body, "Speed Modifier", modifierChoices, 5, (unsigned int*)&Cheats.SpeedModifier, NULL);
        djui_selectionbox_create(body, "Jump Modifier", modifierChoices, 5, (unsigned int*)&Cheats.JumpModifier, NULL);
        djui_selectionbox_create(body, "Swim Modifier", modifierChoices, 5, (unsigned int*)&Cheats.SwimModifier, NULL);
        djui_checkbox_create(body, "Speed Display", &Cheats.SpeedDisplay, djui_panel_cheats_toggle_save);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}

static void djui_panel_cheats_time_space_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create("Cheats - Time-Space", false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        char* bljChoices[13] = { "Disabled", "x1", "x2", "x3", "x4", "x5", "x6", "Hold x1", "Hold x2", "Hold x3", "Hold x4", "Hold x5", "Hold x6" };
        djui_selectionbox_create(body, "BLJ Anywhere", bljChoices, 13, (unsigned int*)&Cheats.BLJAnywhere, NULL);

        djui_checkbox_create(body, "Swim Anywhere", &Cheats.SwimAnywhere, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Walk On Hazards", &Cheats.WalkOnHazards, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Coins Magnet", &Cheats.CoinsMagnet, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Time Stop", &Cheats.TimeStop, djui_panel_cheats_toggle_save);
        djui_checkbox_create(body, "Quick Ending", &Cheats.QuickEnding, djui_panel_cheats_toggle_save);

        char* waterChoices[3] = { "Default", "Drained", "Flooded" };
        djui_selectionbox_create(body, "Water Level", waterChoices, 3, (unsigned int*)&Cheats.WaterLevel, NULL);

        char* hurtChoices[8] = { "Disabled", "Knockback", "Shock", "Burn", "Lava Boost", "Squish", "Stuck", "1 HP" };
        djui_selectionbox_create(body, "Hurt Mario", hurtChoices, 8, (unsigned int*)&Cheats.HurtMario, NULL);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}

void djui_panel_cheats_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create("Cheats", false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        djui_checkbox_create(body, "Enable Cheats", &Cheats.EnableCheats, djui_panel_cheats_toggle_save);

        djui_button_create(body, "Classic", DJUI_BUTTON_STYLE_NORMAL, djui_panel_cheats_classic_create);
        djui_button_create(body, "Modifiers", DJUI_BUTTON_STYLE_NORMAL, djui_panel_cheats_modifiers_create);
        djui_button_create(body, "Time-Space", DJUI_BUTTON_STYLE_NORMAL, djui_panel_cheats_time_space_create);

        djui_button_create(body, "Warp To Level", DJUI_BUTTON_STYLE_NORMAL, djui_panel_pause_warp_to_level_create);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }
    djui_panel_add(caller, panel, NULL);
}
