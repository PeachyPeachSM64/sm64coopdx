#ifndef PHOTO_MODE_POSES_H
#define PHOTO_MODE_POSES_H

#include <stdbool.h>
#include <PR/ultratypes.h>

struct MarioState;

s32 photo_mode_custom_pose_count(bool air);
bool photo_mode_custom_pose_get_name(bool air, s32 index, const char** outName);
bool photo_mode_custom_pose_get_frame(bool air, s32 index, s16* outFrame);
bool photo_mode_custom_pose_is_custom_anim(bool air, s32 index, bool* outIsCustomAnim);
bool photo_mode_custom_pose_get_character_anim_id(bool air, s32 index, s32* outAnimId);
bool photo_mode_custom_pose_get_custom_anim_name(bool air, s32 index, const char** outAnimName);
bool photo_mode_custom_pose_apply(struct MarioState* m, bool air, s32 index);
void photo_mode_custom_poses_reset(void);

bool photo_mode_custom_pose_register_character(bool air, const char* name, s32 characterAnimId, s16 frame);
bool photo_mode_custom_pose_register_mario_anim_index(bool air, const char* name, s32 marioAnimIndex, s16 frame);
bool photo_mode_custom_pose_register_custom_anim(bool air, const char* name, const char* animName, s16 frame);

s32 photo_mode_eye_state_count(s32 characterType);
bool photo_mode_eye_state_get_name(s32 characterType, s32 index, const char** outName);
bool photo_mode_eye_state_get_value(s32 characterType, s32 index, s16* outEyeSwitchIndex);
bool photo_mode_eye_state_register(s32 characterType, const char* name, s16 eyeSwitchIndex);
void photo_mode_eye_state_reset(s32 characterType);

s32 photo_mode_mouth_state_count(s32 characterType);
bool photo_mode_mouth_state_get_name(s32 characterType, s32 index, const char** outName);
bool photo_mode_mouth_state_get_value(s32 characterType, s32 index, s16* outFaceSwitchIndex);
bool photo_mode_mouth_state_register(s32 characterType, const char* name, s16 faceSwitchIndex);
void photo_mode_mouth_state_reset(s32 characterType);

#endif
