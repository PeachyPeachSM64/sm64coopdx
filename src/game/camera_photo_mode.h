#ifndef CAMERA_PHOTO_MODE_H
#define CAMERA_PHOTO_MODE_H

#include <PR/ultratypes.h>

void mode_photo_mode_camera(void);
void switch_to_photo_mode(void);
void update_photo_mode_camera(struct GraphNodeCamera *gc);
void sync_photo_mode_camera_to_game_camera(void);

#endif // CAMERA_PHOTO_MODE_H