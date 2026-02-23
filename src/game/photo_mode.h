#ifndef PHOTO_MODE_H
#define PHOTO_MODE_H

#include <stdbool.h>
#include <PR/ultratypes.h>

s32 activate_photo_mode(void);
void open_photo_mode(void);
void photo_mode_set_opened_via_shortcut(bool openedViaShortcut);
void close_photo_mode_to_gameplay(void);
void close_photo_mode_to_djui_pause_menu(void);

#endif // PHOTO_MODE_H