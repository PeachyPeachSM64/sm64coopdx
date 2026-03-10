#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "djui_panel_qol_bugfixes.h"
#include "pc/configfile.h"

void djui_panel_qol_bugfixes_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(QOL, QOL_BUGFIXES_TITLE), false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        djui_checkbox_create(body, DLANG(QOL, FIX_MAX_LIVES_COINS_OVERFLOW), &configBugfixMaxLives, NULL);
        djui_checkbox_create(body, DLANG(QOL, FIX_KING_BOB_OMB_MUSIC_FADE), &configBugfixKingBobOmbFadeMusic, NULL);
        djui_checkbox_create(body, DLANG(QOL, FIX_KOOPA_RACE_MUSIC_ON_WARP), &configBugfixKoopaRaceMusic, NULL);
        djui_checkbox_create(body, DLANG(QOL, FIX_PIRANHA_PLANT_STATE_RESET), &configBugfixPiranhaPlantStateReset, NULL);
        djui_checkbox_create(body, DLANG(QOL, FIX_SLEEPING_PIRANHA_PLANT_DAMAGE), &configBugfixPiranhaPlantSleepDamage, NULL);
        djui_checkbox_create(body, DLANG(QOL, FIX_BOWSER_KEY_SHOWING_AS_STAR), &configBugfixStarBowserKey, NULL);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }

    djui_panel_add(caller, panel, NULL);
}
