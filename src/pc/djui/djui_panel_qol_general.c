#include "djui.h"
#include "djui_panel.h"
#include "djui_panel_menu.h"
#include "djui_panel_qol_general.h"
#include "pc/configfile.h"

void djui_panel_qol_general_create(struct DjuiBase* caller) {
    struct DjuiThreePanel* panel = djui_panel_menu_create(DLANG(QOL, QOL_GENERAL_TITLE), false);
    struct DjuiBase* body = djui_three_panel_get_body(panel);
    {
        char* stayInLevelChoices[3] = {
            (char*)DLANG(QOL, CHOICE_DISABLED),
            (char*)DLANG(QOL, CHOICE_ENABLED_NORMAL),
            (char*)DLANG(QOL, CHOICE_ENABLED_NONSTOP),
        };
        djui_selectionbox_create(body, DLANG(QOL, STAY_IN_LEVEL_AFTER_STAR), stayInLevelChoices, 3, &configStayInLevelAfterStar, NULL);
        djui_checkbox_create(body, DLANG(QOL, BETTER_GODDARD_MATERIAL_HANDLING), &configQolBetterGoddardMaterial, NULL);

        djui_button_create(body, DLANG(MENU, BACK), DJUI_BUTTON_STYLE_BACK, djui_panel_menu_back);
    }

    djui_panel_add(caller, panel, NULL);
}
