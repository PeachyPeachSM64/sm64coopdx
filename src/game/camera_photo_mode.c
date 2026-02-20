#include <ultra64.h>

#define INCLUDED_FROM_CAMERA_C

#include "sm64.h"
#include "camera.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "level_update.h"
#include "pc/configfile.h"
#include "pc/controller/controller_mouse.h"

extern struct LakituState gLakituState;
extern struct Camera *gCamera;
extern struct PlayerCameraState *sMarioCamState;
extern int photoModeClosed;
u8 sPreviousCameraMode = 0;

Vec3f freeCamPos;
Vec3f freeCamFocus;
Vec3f initialForward;
f32 initialDistance;
f32 sFreeCamYaw;
f32 sFreeCamPitch;
f32 sFreeCamRoll;

#define MAX_CAMERA_DISTANCE 2000.0f
#define powf(x, y) ((x) * (x))

void photo_mode_camera_dist_limit(void) {
    float distanceToMario = sqrtf(powf(freeCamPos[0] - sMarioCamState->pos[0], 2) +
                                  powf(freeCamPos[1] - sMarioCamState->pos[1], 2) +
                                  powf(freeCamPos[2] - sMarioCamState->pos[2], 2));

    if (distanceToMario > MAX_CAMERA_DISTANCE) {
        float scale = MAX_CAMERA_DISTANCE / distanceToMario;
        freeCamPos[0] = sMarioCamState->pos[0] + (freeCamPos[0] - sMarioCamState->pos[0]) * scale;
        freeCamPos[1] = sMarioCamState->pos[1] + (freeCamPos[1] - sMarioCamState->pos[1]) * scale;
        freeCamPos[2] = sMarioCamState->pos[2] + (freeCamPos[2] - sMarioCamState->pos[2]) * scale;
    }
}

int gInitialPosSet = FALSE;
int gPrecisionOn = FALSE;
int gZBoostActive = FALSE;

f32 camSpeedChanger;

void set_initial_photo_mode_camera_position(void) {
    vec3f_copy(freeCamPos, gCamera->pos);
    vec3f_copy(freeCamFocus, gCamera->focus);
    
    Vec3f temp;
    vec3f_dif(temp, freeCamFocus, freeCamPos);
    initialDistance = sqrtf(temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2]);
    if (initialDistance > 0.0f) {
        f32 scale = 1.0f / initialDistance;
        temp[0] *= scale;
        temp[1] *= scale;
        temp[2] *= scale;
    }
    vec3f_copy(initialForward, temp);
    
    // Calculate initial yaw
    sFreeCamYaw = atan2f(-temp[0], -temp[2]) * (180.0f / M_PI);
    // Calculate initial pitch
    f32 horizontalDistance = sqrtf(temp[0] * temp[0] + temp[2] * temp[2]);
    sFreeCamPitch = atan2f(horizontalDistance, temp[1]) * (180.0f / M_PI);
    
    sFreeCamPitch = clamp(sFreeCamPitch, -89.0f, 89.0f);
    sFreeCamYaw = fmodf(sFreeCamYaw + 180.0f, 360.0f);
}

void update_photo_mode_camera(struct GraphNodeCamera *gc) {
    vec3f_copy(gc->pos, freeCamPos);
    
    gc->focus[0] = freeCamPos[0] + initialForward[0] * initialDistance;
    gc->focus[1] = freeCamPos[1] + initialForward[1] * initialDistance;
    gc->focus[2] = freeCamPos[2] + initialForward[2] * initialDistance;
    
    vec3f_copy(freeCamFocus, gc->focus);
}

/**
 * Synchronizes the photo mode camera (freeCamPos, freeCamFocus) to the game camera (gCamera)
 * This ensures the rendered view uses the photo mode camera values
 */
void sync_photo_mode_camera_to_game_camera(void) {
    if (gCamera && gLakituState.mode == CAMERA_MODE_PHOTO_MODE && !photoModeClosed) {
        vec3f_copy(gCamera->pos, freeCamPos);
        vec3f_copy(gCamera->focus, freeCamFocus);
    }
}

void mode_photo_mode_camera(void) {
    photo_mode_camera_dist_limit();
    resolve_geometry_collisions(freeCamPos, freeCamPos);

    if (gPlayer1Controller->buttonDown & Z_TRIG) {
        gZBoostActive = TRUE;
    } else {
        gZBoostActive = FALSE;
    }

    // Determine camera speed based on conditions
    if (gPrecisionOn) {
        if (gZBoostActive) {
            camSpeedChanger = 1.0f;
        } else {
            if (gFOVState.fov < 50) {
                camSpeedChanger = 0.1f - ((50.0f - gFOVState.fov) / 50.0f) * (0.1f - 0.01f);
            } else if (gFOVState.fov > 50) {
                camSpeedChanger = 0.1f + ((gFOVState.fov - 50.0f) / 50.0f) * (0.1f - 0.01f);
            } else if (gFOVState.fov == 50) {
                camSpeedChanger = 0.1f;
            }
        }
    } else if (gZBoostActive) {
        camSpeedChanger = 2.5f;
    } else {
        if (gFOVState.fov < 50) {
            camSpeedChanger = 1.0f - ((50.0f - gFOVState.fov) / 50.0f) * (1.0f - 0.5f);
        } else if (gFOVState.fov > 50) {
            camSpeedChanger = 1.0f + ((gFOVState.fov - 50.0f) / 50.0f) * (1.0f - 0.5f);
        } else if (gFOVState.fov == 50) {
            camSpeedChanger = 1.0f;
        }
    }

    // Move camera up and down
    if (gPlayer1Controller->buttonDown & L_TRIG) {
        freeCamPos[1] -= 10.0f * camSpeedChanger;
    }
    if (gPlayer1Controller->buttonDown & R_TRIG) {
        freeCamPos[1] += 10.0f * camSpeedChanger;
    }

    // Rotate left and right
    if (gPlayer1Controller->buttonDown & L_CBUTTONS) {
        sFreeCamYaw -= 2.0f * camSpeedChanger;
    }
    if (gPlayer1Controller->buttonDown & R_CBUTTONS) {
        sFreeCamYaw += 2.0f * camSpeedChanger;
    }

    // Rotate up and down
    if (gPlayer1Controller->buttonDown & U_CBUTTONS) {
        sFreeCamPitch += 2.0f * camSpeedChanger;
    }
    if (gPlayer1Controller->buttonDown & D_CBUTTONS) {
        sFreeCamPitch -= 2.0f * camSpeedChanger;
    }

    if (configFreeCameraMouse && mouse_relative_enabled) {
        const f32 sensX = (f32) configFreeCameraXSens;
        const f32 sensY = (f32) configFreeCameraYSens;
        const f32 invX = configCameraInvertX ? 1.0f : -1.0f;
        const f32 invY = configCameraInvertY ? 1.0f : -1.0f;

        // bettercamera stores angles in s16 units where 65536 = 360 degrees,
        // while photo mode uses degrees, so convert to degrees here.
        const f32 angToDeg = 360.0f / 65536.0f;
        const f32 deltaYawDeg = invX * (f32) mouse_x * 16.0f * (sensX / 125.0f) * angToDeg;
        const f32 deltaPitchDeg = invY * (f32) mouse_y * 16.0f * (sensY / 125.0f) * angToDeg;

        // Invert yaw sign to match the perceived left/right direction of C-buttons in photo mode.
        sFreeCamYaw -= deltaYawDeg;
        sFreeCamPitch += deltaPitchDeg;
    }

    // Clamp the pitch to avoid flipping the camera
    sFreeCamPitch = clamp(sFreeCamPitch, -89.0f, 89.0f);
    sFreeCamYaw = fmodf(sFreeCamYaw, 360.0f); // Wrap yaw to avoid overflow

    // Convert yaw, pitch, and roll to radians
    float yawRad = sFreeCamYaw * (M_PI / 180.0f);
    float pitchRad = sFreeCamPitch * (M_PI / 180.0f);

    Vec3f forward = {
        cosf(pitchRad) * cosf(yawRad),
        sinf(pitchRad),
        cosf(pitchRad) * sinf(yawRad)
    };

    Vec3f right = {
        -sinf(yawRad),
        0.0f,
        cosf(yawRad)
    };

    freeCamPos[0] += right[0] * (gPlayer1Controller->stickX * 0.2f * camSpeedChanger);
    freeCamPos[2] += right[2] * (gPlayer1Controller->stickX * 0.2f * camSpeedChanger);

    freeCamPos[0] += forward[0] * (gPlayer1Controller->stickY * 0.2f * camSpeedChanger);
    freeCamPos[2] += forward[2] * (gPlayer1Controller->stickY * 0.2f * camSpeedChanger);
    freeCamPos[1] += forward[1] * (gPlayer1Controller->stickY * 0.2f * camSpeedChanger);

    // Update the initialForward vector when camera rotates
    if ((gPlayer1Controller->buttonDown & (L_CBUTTONS | R_CBUTTONS | U_CBUTTONS | D_CBUTTONS))
        || (configFreeCameraMouse && mouse_relative_enabled && (mouse_x != 0 || mouse_y != 0))) {
        vec3f_copy(initialForward, forward);
    }
    
    // Calculate the camera's focus point based on current position and forward vector
    freeCamFocus[0] = freeCamPos[0] + forward[0] * 100.0f;
    freeCamFocus[1] = freeCamPos[1] + forward[1] * 100.0f;
    freeCamFocus[2] = freeCamPos[2] + forward[2] * 100.0f;
}

void switch_to_photo_mode(void) {
    if (!photoModeClosed) {
        // Save the current gameplay camera mode once, then force photo mode.
        if (!gInitialPosSet) {
            if (gLakituState.mode != CAMERA_MODE_PHOTO_MODE) {
                sPreviousCameraMode = gLakituState.mode;
            }
            set_initial_photo_mode_camera_position();
            gInitialPosSet = TRUE;
        }
        gLakituState.mode = CAMERA_MODE_PHOTO_MODE;
        return;
    }

    // Photo mode is closed: restore gameplay camera mode and reset state.
    if (gLakituState.mode == CAMERA_MODE_PHOTO_MODE) {
        gLakituState.mode = sPreviousCameraMode;
    }
    gInitialPosSet = FALSE;
}