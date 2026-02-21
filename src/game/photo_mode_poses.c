#include "photo_mode_poses.h"

#include <stdlib.h>
#include <string.h>

#include "mario.h"
#include "characters.h"
#include "pc/lua/utils/smlua_anim_utils.h"

#define PHOTO_MODE_MAX_CUSTOM_POSES 64
#define PHOTO_MODE_CUSTOM_ANIM_ID_BASE 20000

#define PHOTO_MODE_MAX_CUSTOM_EYE_STATES 64
#define PHOTO_MODE_MAX_CUSTOM_MOUTH_STATES 64

struct PhotoModeCustomState {
    char* name;
    s16 value;
};

static struct PhotoModeCustomState sEyeStatesPerCharacter[CT_MAX][PHOTO_MODE_MAX_CUSTOM_EYE_STATES];
static s32 sEyeStateCountPerCharacter[CT_MAX] = { 0 };

static struct PhotoModeCustomState sMouthStatesPerCharacter[CT_MAX][PHOTO_MODE_MAX_CUSTOM_MOUTH_STATES];
static s32 sMouthStateCountPerCharacter[CT_MAX] = { 0 };

struct PhotoModeCustomPose {
    char* name;
    bool isCustomAnim;
    bool isMarioAnimIndex;
    s32 characterAnimId;
    char* customAnimName;
    s16 frame;
};

static void photo_mode_custom_state_free(struct PhotoModeCustomState* state) {
    if (state == NULL) { return; }
    if (state->name) {
        free(state->name);
        state->name = NULL;
    }
    state->value = 0;
}

static struct PhotoModeCustomPose sGroundCustomPoses[PHOTO_MODE_MAX_CUSTOM_POSES];
static struct PhotoModeCustomPose sAirCustomPoses[PHOTO_MODE_MAX_CUSTOM_POSES];
static s32 sGroundCustomPoseCount = 0;
static s32 sAirCustomPoseCount = 0;

static void photo_mode_custom_pose_free(struct PhotoModeCustomPose* pose) {
    if (pose->name) {
        free(pose->name);
        pose->name = NULL;
    }
    if (pose->customAnimName) {
        free(pose->customAnimName);
        pose->customAnimName = NULL;
    }
    pose->isCustomAnim = false;
    pose->isMarioAnimIndex = false;
    pose->characterAnimId = 0;
    pose->frame = 0;
}

static struct PhotoModeCustomPose* photo_mode_custom_pose_list(bool air, s32* outCount) {
    if (air) {
        if (outCount) { *outCount = sAirCustomPoseCount; }
        return sAirCustomPoses;
    }

    if (outCount) { *outCount = sGroundCustomPoseCount; }
    return sGroundCustomPoses;
}

static void force_anim_frame_obj(struct Object* obj, s16 frame) {
    if (obj == NULL) { return; }
    if (frame <= 0) { frame = 1; }
    struct AnimInfo* a = &obj->header.gfx.animInfo;
    a->animFrame = frame;
    a->animFrameAccelAssist = (frame << 16);
    a->animAccel = 0;
}

static bool set_custom_anim_and_init(struct Object* obj, const char* animName, s16 photoModeAnimId) {
    if (obj == NULL || animName == NULL) { return false; }

    struct AnimInfo* a = &obj->header.gfx.animInfo;
    struct Animation* prevAnim = a->curAnim;

    smlua_anim_util_set_animation(obj, animName);

    const char* currentName = smlua_anim_util_get_current_animation_name(obj);
    if (currentName == NULL || strcmp(currentName, animName) != 0) {
        a->curAnim = prevAnim;
        return false;
    }

    struct Animation* anim = a->curAnim;
    if (anim == NULL) { return false; }

    a->animID = photoModeAnimId;
    a->animAccel = 0x10000;

    if (anim->flags & ANIM_FLAG_2) {
        a->animFrameAccelAssist = (anim->startFrame << 16);
    } else {
        if (anim->flags & ANIM_FLAG_BACKWARD) {
            a->animFrameAccelAssist = (anim->startFrame << 16) + a->animAccel;
        } else {
            a->animFrameAccelAssist = (anim->startFrame << 16) - a->animAccel;
        }
    }

    a->animFrame = (a->animFrameAccelAssist >> 16);
    return true;
}

s32 photo_mode_custom_pose_count(bool air) {
    return air ? sAirCustomPoseCount : sGroundCustomPoseCount;
}

bool photo_mode_custom_pose_get_name(bool air, s32 index, const char** outName) {
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (index < 0 || index >= count || outName == NULL) { return false; }
    *outName = list[index].name;
    return (*outName != NULL);
}

bool photo_mode_custom_pose_get_frame(bool air, s32 index, s16* outFrame) {
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (index < 0 || index >= count || outFrame == NULL) { return false; }
    *outFrame = list[index].frame;
    return true;
}

bool photo_mode_custom_pose_is_custom_anim(bool air, s32 index, bool* outIsCustomAnim) {
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (index < 0 || index >= count || outIsCustomAnim == NULL) { return false; }
    *outIsCustomAnim = list[index].isCustomAnim;
    return true;
}

bool photo_mode_custom_pose_get_character_anim_id(bool air, s32 index, s32* outAnimId) {
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (index < 0 || index >= count || outAnimId == NULL) { return false; }
    *outAnimId = list[index].characterAnimId;
    return !list[index].isCustomAnim;
}

bool photo_mode_custom_pose_get_custom_anim_name(bool air, s32 index, const char** outAnimName) {
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (index < 0 || index >= count || outAnimName == NULL) { return false; }
    *outAnimName = list[index].customAnimName;
    return list[index].isCustomAnim && (*outAnimName != NULL);
}

bool photo_mode_custom_pose_apply(struct MarioState* m, bool air, s32 index) {
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (m == NULL || m->marioObj == NULL || index < 0 || index >= count) { return false; }

    if (list[index].isCustomAnim) {
        if (list[index].customAnimName == NULL) { return false; }
        s16 photoModeAnimId = (s16)(PHOTO_MODE_CUSTOM_ANIM_ID_BASE + (air ? 1000 : 0) + index);
        if (!set_custom_anim_and_init(m->marioObj, list[index].customAnimName, photoModeAnimId)) {
            return false;
        }
        force_anim_frame_obj(m->marioObj, list[index].frame);
        return true;
    }

    if (list[index].isMarioAnimIndex) {
        set_mario_animation(m, list[index].characterAnimId);
        force_anim_frame_obj(m->marioObj, list[index].frame);
        return true;
    }

    set_character_animation(m, (enum CharacterAnimID) list[index].characterAnimId);
    force_anim_frame_obj(m->marioObj, list[index].frame);
    return true;
}

void photo_mode_custom_poses_reset(void) {
    for (s32 i = 0; i < sGroundCustomPoseCount; i++) {
        photo_mode_custom_pose_free(&sGroundCustomPoses[i]);
    }
    for (s32 i = 0; i < sAirCustomPoseCount; i++) {
        photo_mode_custom_pose_free(&sAirCustomPoses[i]);
    }

    memset(sGroundCustomPoses, 0, sizeof(sGroundCustomPoses));
    memset(sAirCustomPoses, 0, sizeof(sAirCustomPoses));
    sGroundCustomPoseCount = 0;
    sAirCustomPoseCount = 0;
}

static bool photo_mode_validate_character_type(s32 characterType) {
    return (characterType >= 0 && characterType < CT_MAX);
}

s32 photo_mode_eye_state_count(s32 characterType) {
    if (!photo_mode_validate_character_type(characterType)) { return 0; }
    return sEyeStateCountPerCharacter[characterType];
}

bool photo_mode_eye_state_get_name(s32 characterType, s32 index, const char** outName) {
    if (!photo_mode_validate_character_type(characterType)) { return false; }
    if (index < 0 || index >= sEyeStateCountPerCharacter[characterType] || outName == NULL) { return false; }
    *outName = sEyeStatesPerCharacter[characterType][index].name;
    return (*outName != NULL);
}

bool photo_mode_eye_state_get_value(s32 characterType, s32 index, s16* outEyeSwitchIndex) {
    if (!photo_mode_validate_character_type(characterType)) { return false; }
    if (index < 0 || index >= sEyeStateCountPerCharacter[characterType] || outEyeSwitchIndex == NULL) { return false; }
    *outEyeSwitchIndex = sEyeStatesPerCharacter[characterType][index].value;
    return true;
}

bool photo_mode_eye_state_register(s32 characterType, const char* name, s16 eyeSwitchIndex) {
    if (!photo_mode_validate_character_type(characterType)) { return false; }
    if (name == NULL) { return false; }
    if (sEyeStateCountPerCharacter[characterType] >= PHOTO_MODE_MAX_CUSTOM_EYE_STATES) { return false; }

    struct PhotoModeCustomState* state = &sEyeStatesPerCharacter[characterType][sEyeStateCountPerCharacter[characterType]];
    memset(state, 0, sizeof(*state));
    state->name = strdup(name);
    if (state->name == NULL) { return false; }
    state->value = eyeSwitchIndex;

    sEyeStateCountPerCharacter[characterType]++;
    return true;
}

void photo_mode_eye_state_reset(s32 characterType) {
    if (!photo_mode_validate_character_type(characterType)) { return; }
    for (s32 i = 0; i < sEyeStateCountPerCharacter[characterType]; i++) {
        photo_mode_custom_state_free(&sEyeStatesPerCharacter[characterType][i]);
    }
    memset(sEyeStatesPerCharacter[characterType], 0, sizeof(sEyeStatesPerCharacter[characterType]));
    sEyeStateCountPerCharacter[characterType] = 0;
}

s32 photo_mode_mouth_state_count(s32 characterType) {
    if (!photo_mode_validate_character_type(characterType)) { return 0; }
    return sMouthStateCountPerCharacter[characterType];
}

bool photo_mode_mouth_state_get_name(s32 characterType, s32 index, const char** outName) {
    if (!photo_mode_validate_character_type(characterType)) { return false; }
    if (index < 0 || index >= sMouthStateCountPerCharacter[characterType] || outName == NULL) { return false; }
    *outName = sMouthStatesPerCharacter[characterType][index].name;
    return (*outName != NULL);
}

bool photo_mode_mouth_state_get_value(s32 characterType, s32 index, s16* outFaceSwitchIndex) {
    if (!photo_mode_validate_character_type(characterType)) { return false; }
    if (index < 0 || index >= sMouthStateCountPerCharacter[characterType] || outFaceSwitchIndex == NULL) { return false; }
    *outFaceSwitchIndex = sMouthStatesPerCharacter[characterType][index].value;
    return true;
}

bool photo_mode_mouth_state_register(s32 characterType, const char* name, s16 faceSwitchIndex) {
    if (!photo_mode_validate_character_type(characterType)) { return false; }
    if (name == NULL) { return false; }
    if (sMouthStateCountPerCharacter[characterType] >= PHOTO_MODE_MAX_CUSTOM_MOUTH_STATES) { return false; }

    struct PhotoModeCustomState* state = &sMouthStatesPerCharacter[characterType][sMouthStateCountPerCharacter[characterType]];
    memset(state, 0, sizeof(*state));
    state->name = strdup(name);
    if (state->name == NULL) { return false; }
    state->value = faceSwitchIndex;

    sMouthStateCountPerCharacter[characterType]++;
    return true;
}

void photo_mode_mouth_state_reset(s32 characterType) {
    if (!photo_mode_validate_character_type(characterType)) { return; }
    for (s32 i = 0; i < sMouthStateCountPerCharacter[characterType]; i++) {
        photo_mode_custom_state_free(&sMouthStatesPerCharacter[characterType][i]);
    }
    memset(sMouthStatesPerCharacter[characterType], 0, sizeof(sMouthStatesPerCharacter[characterType]));
    sMouthStateCountPerCharacter[characterType] = 0;
}

static bool photo_mode_custom_pose_register_internal(bool air, const char* name, bool isCustomAnim, s32 characterAnimId,
                                                    const char* customAnimName, s16 frame) {
    if (name == NULL) { return false; }
    if (frame <= 0) { frame = 1; }

    s32* count = air ? &sAirCustomPoseCount : &sGroundCustomPoseCount;
    struct PhotoModeCustomPose* list = air ? sAirCustomPoses : sGroundCustomPoses;

    if (*count >= PHOTO_MODE_MAX_CUSTOM_POSES) { return false; }

    struct PhotoModeCustomPose* pose = &list[*count];
    memset(pose, 0, sizeof(*pose));

    pose->name = strdup(name);
    if (pose->name == NULL) { return false; }

    pose->isCustomAnim = isCustomAnim;
    pose->isMarioAnimIndex = false;
    pose->characterAnimId = characterAnimId;
    pose->frame = frame;

    if (isCustomAnim) {
        if (customAnimName == NULL) {
            photo_mode_custom_pose_free(pose);
            return false;
        }
        pose->customAnimName = strdup(customAnimName);
        if (pose->customAnimName == NULL) {
            photo_mode_custom_pose_free(pose);
            return false;
        }
    }

    (*count)++;
    return true;
}

bool photo_mode_custom_pose_register_character(bool air, const char* name, s32 characterAnimId, s16 frame) {
    return photo_mode_custom_pose_register_internal(air, name, false, characterAnimId, NULL, frame);
}

bool photo_mode_custom_pose_register_mario_anim_index(bool air, const char* name, s32 marioAnimIndex, s16 frame) {
    if (!photo_mode_custom_pose_register_internal(air, name, false, marioAnimIndex, NULL, frame)) {
        return false;
    }
    s32 count = 0;
    struct PhotoModeCustomPose* list = photo_mode_custom_pose_list(air, &count);
    if (count <= 0) { return false; }
    list[count - 1].isMarioAnimIndex = true;
    return true;
}

bool photo_mode_custom_pose_register_custom_anim(bool air, const char* name, const char* animName, s16 frame) {
    return photo_mode_custom_pose_register_internal(air, name, true, 0, animName, frame);
}
