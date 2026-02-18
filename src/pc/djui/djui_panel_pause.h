#pragma once
#include "djui.h"

extern bool gDjuiPanelPauseCreated;
extern bool gDjuiSecretWarpUnlocked;

void djui_panel_pause_quit_yes(UNUSED struct DjuiBase* caller);

void djui_panel_pause_create(struct DjuiBase* caller);
