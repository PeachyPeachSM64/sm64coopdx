#include <ultra64.h>

#include "area.h"
#include "audio/external.h"
#include "engine/math_util.h"
#include "game_init.h"
#include "ingame_menu.h"
#include "photo_mode.h"
#include "level_update.h"
#include "level_info.h"
#include "print.h"
#include "sound_init.h"
#include "sm64.h"
#include "mario.h"
#include "main.h"
#include "behavior_data.h"
#include "object_list_processor.h"
#include "camera.h"
#include "camera_photo_mode.h"
#include "segment2.h"
#include "engine/surface_collision.h"
#include "object_helpers.h"
#include "gfx_dimensions.h"
#include "pc/configfile.h"

struct MarioState *marioState = &gMarioStates[0];

static s8 sSavedAllowPartRotation = FALSE;
static Vec3s sSavedHeadAngle;
static s16 sSavedFovValue = 45;

int photoModeClosed = true;
extern int gPrecisionOn;

static s8 maxSections[] = { /* Pose Options */   6, 
                            /* Body Options */   4, 
                            /* Body Rotation */  6, 
                            /* Camera Options */ 3, 
                            /* World Options */  5};

static s16 gCurrentMenuSection = 0;     // Global section tracker
static s16 gPageIndex = 0;              // Current page index

// Indexes
static s16 gGroundPoseIndex = 0;
static s16 gAirPoseIndex = 0;
static s16 gHandStateIndex = 0;
static s16 gCapStateIndex = 0;
static s16 gEyeStateIndex = 0;
static s16 gPowerupIndex = 0;
static s16 gPlayerVisIndex = 0;
static s16 gShadowVisIndex = 0;
static s16 gFrameIndex = 0;
static s16 gCoinVisibilityIndex = 0;
static s16 gEnemyVisibilityIndex = 0;
static s16 gFriendVisibilityIndex = 0;
static s16 gEffectVisibilityIndex = 0;
static s16 gPrecisionIndex = 0;

static s16 previousMenuSection = -1;

int isPlayerShadowVisible = true;

#define PAGE_Y 130
#define OPTION_1_Y 110
#define OPTION_2_Y 90
#define OPTION_3_Y 70
#define OPTION_4_Y 50
#define OPTION_5_Y 30
#define OPTION_6_Y 10

#define PAGE_X 371
#define LARGE_X 405
#define SMALL_X 425

#define OPTION_X 290
#define OPTION_ARROW_X 363
#define OPTION_BAR_X 350
#define OPTION_BAR_VALUE_X 435

const BehaviorScript *coinbehaviors[] = {
    bhvYellowCoin,
    bhvRedCoin,
    bhvMovingYellowCoin,
    bhvOneCoin,
    bhvTemporaryYellowCoin,
    bhvSingleCoinGetsSpawned,
    bhvHiddenBlueCoin,
    NULL
};

const BehaviorScript *enemyBehaviors[] = {
    bhvGoomba,
    bhvKoopa,
    bhvBobomb,
    bhvBobombAnchorMario,
    bhvKingBobomb,
    bhvWaterBomb,
    bhvWaterBombShadow,
    bhvChainChomp,
    bhvChainChompChainPart,
    NULL
};

const BehaviorScript *friendBehaviors[] = {
    bhvBobombBuddy,
    bhvBobombBuddyOpensCannon,
    bhvMips,
    NULL
};

const BehaviorScript *effectBehaviors[] = {
    bhvButterfly,
    bhvBird,
    bhvFish,
    bhvBlueFish,
    bhvSmallParticle,
    bhvSmallParticleSnow,
    bhvSmallParticleBubbles,
    bhvWallTinyStarParticle,
    bhvPoundTinyStarParticle,
    bhvWhitePuff1,
    bhvWhitePuff2,
    bhvWhitePuffExplosion,
    bhvWhitePuffSmoke,
    bhvWhitePuffSmoke2,
    bhvExplosion,
    bhvBubbleMaybe,
    bhvBubbleSplash,
    bhvPlungeBubble,
    bhvMistCircParticleSpawner,
    bhvMistParticleSpawner,
    bhvWaterMist,
    bhvWaterMist2,
    bhvBobombFuseSmoke,
    bhvSparkle,
    bhvSparkleParticleSpawner,
    bhvSparkleSpawn,
    bhvCoinSparkles,
    bhvCelebrationStarSparkle,
    bhvLeafParticleSpawner,
    bhvTreeLeaf,
    bhvTreeSnow,
    bhvBreakBoxTriangle,
    bhvOrangeNumber,
    bhvWaveTrail,
    bhvIdleWaterWave,
    bhvWaterSplash,
    bhvShallowWaterSplash,
    bhvShallowWaterWave,
    bhvTriangleParticleSpawner,
    NULL
};

struct Object **find_all_objects_with_behavior(const BehaviorScript *behavior, s32 *count) {
    #define MAX_OBJECTS 1000
    static struct Object *foundObjects[MAX_OBJECTS];
    uintptr_t *behaviorAddr = segmented_to_virtual(behavior);
    struct ObjectNode *listHead = &gObjectLists[get_object_list_from_behavior(behaviorAddr)];
    struct Object *obj = (struct Object *) listHead->next;
    s32 objCount = 0;
    
    // Reset the count
    *count = 0;
    
    // Clear the static array
    for (s32 i = 0; i < MAX_OBJECTS; i++) {
        foundObjects[i] = NULL;
    }
    
    // Find all objects with the given behavior
    while (obj != (struct Object *) listHead && objCount < MAX_OBJECTS) {
        if (obj->behavior == behaviorAddr && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED) {
            foundObjects[objCount++] = obj;
        }
        obj = (struct Object *) obj->header.next;
    }
    
    *count = objCount;
    return foundObjects;
}

void set_photo_mode_object_visibility(const BehaviorScript *behaviors[], s8 visibility) {
    if (!behaviors) return;
    
    // Iterate until we hit the NULL terminator
    for (s32 i = 0; behaviors[i] != NULL; i++) {
        // Find all objects with this behavior
        s32 numObjects = 0;
        struct Object **objects = find_all_objects_with_behavior(behaviors[i], &numObjects);
        
        // Update visibility for all found objects
        for (s32 j = 0; j < numObjects; j++) {
            if (objects[j]) {
                if (visibility) {
                    objects[j]->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
                } else {
                    objects[j]->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
                }
            }
        }
    }
}

s32 is_mario_on_ground(void) {
    struct Surface *floor;
    f32 floorHeight = find_floor(marioState->pos[0], marioState->pos[1], marioState->pos[2], &floor);
    
    // Check if Mario's current position is on the floor
    int isOnFloor = (marioState->pos[1] == floorHeight);
    
    // Check if the surface normal is mostly vertical (i.e., close to 1.0)
    int isSurfaceNormalVertical = (floor != NULL && floor->normal.y >= 0.86602540f);

    // Check if Mario's vertical speed is zero (indicating he's not falling or jumping)
    int isVerticalSpeedZero = (marioState->vel[1] == 0.0f);

    // Mario is considered on the ground if he is on the floor, the surface is mostly flat, and his vertical speed is zero
    return isOnFloor && isSurfaceNormalVertical && isVerticalSpeedZero;
}

void print_number_value(s16 x, s16 y, s32 number) {
    // Normalize the number to be within the range 0-360
    number = number % 360;
    if (number < 0) {
        number += 360;
    }

    // Buffer to hold the digits of the number
    u8 numberStr[4];  // Buffer for 3 digits + terminator
    int len = 0;

    // Adjust x based on the number of digits
    if (number >= 100) {
        x -= 2;  // Adjust position for hundreds
    } else if (number <= 9 && number >= 0) {
        x += 1;  // Adjust position for ones
    }

    // Populate the numberStr array
    if (number < 10) {
        numberStr[len++] = (u8) (0 + number);
    } else if (number < 100) {
        numberStr[len++] = (u8) (0 + (number / 10));
        numberStr[len++] = (u8) (0 + (number % 10));
    } else {
        numberStr[len++] = (u8) (0 + (number / 100));
        numberStr[len++] = (u8) (0 + ((number / 10) % 10));
        numberStr[len++] = (u8) (0 + (number % 10));
    }

    // Add terminator
    numberStr[len] = DIALOG_CHAR_TERMINATOR;

    // Call print_generic_string to render the number
    print_generic_string(x, y, numberStr);
}

static void (*sSm64PrintGenericString)(s16 x, s16 y, const u8 *str) = print_generic_string;

static void photo_mode_print_generic_string(s16 x, s16 y, const u8 *str) {
    sSm64PrintGenericString(x, y, str);
}

#define print_generic_string photo_mode_print_generic_string

static void photo_mode_print_number_value(s16 x, s16 y, s32 number) {
    print_number_value(x, y, number);
}

f32 centeredX(f32 x, u8 text[]) {
    f32 textWidth = 0;
    
    for (int i = 0; text[i] != DIALOG_CHAR_TERMINATOR; i++) {
        if (text[i] == DIALOG_CHAR_SPACE) {
            textWidth += gDialogCharWidths[DIALOG_CHAR_SPACE];
        } else {
            textWidth += gDialogCharWidths[text[i]];
        }
    }

    f32 finalX = x - (textWidth / 2);
    
    return finalX;
}

// Helper macro to create and initialize a string
#define CREATE_STRING(varName, str) \
    static u8 varName[sizeof(str) + 1] = { 0 }; \
    if (varName[0] == 0) { \
        convert_string_ascii_to_sm64(varName, str, false); \
        photo_mode_capitalize_string_sm64(varName); \
    }

static void photo_mode_capitalize_string_sm64(u8 *str64) {
    if (!str64) { return; }
    for (; *str64 != DIALOG_CHAR_TERMINATOR; str64++) {
        if (*str64 >= 0x24 && *str64 <= 0x3D) {
            *str64 -= 26;
        }
    }
}

// Helper function to set text alpha based on current menu section and option
void set_option_alpha(u8 optionNumber) {
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, (gCurrentMenuSection == optionNumber) ? 255 : 120);
}

// Variables to track button hold state
static int rightButtonHoldCounter = 0; // Tracks frames holding right D-pad
static int leftButtonHoldCounter = 0;  // Tracks frames holding left D-pad

// Constants for how long the button must be held before scrolling
#define HOLD_THRESHOLD 20
#define HOLD_DELAY 2

#define FADE_SPEED 0.2f

// Draw selection box at the current selection
void draw_selection_box(s16 x, s16 y) {
    static f32 fadeProgress = 0.0f; // Tracks the fade animation progress
    fadeProgress += FADE_SPEED;

    // Calculate the fade factor using sine for smooth in-and-out animation
    f32 fadeFactor = (sinf(fadeProgress) + 1.0f) / 2.0f; // Normalize to range [0, 1]

    // Interpolate alpha between 120 and 255
    u8 alpha = (u8)((1.0f - fadeFactor) * 120 + fadeFactor * 255);

    // Draw the selection box with fading alpha
    create_dl_translation_matrix(MENU_MTX_PUSH, x, y + 8.0f, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.20f, 0.20f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, alpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void draw_option_bar(s16 y, s16 barRange, s16 scaleRef, s8 index) {
    s16 rangeSub = (barRange == 90) ? 10 : 0; 

    // Calculate scaleFactor
    float scaleFactor = ((float)(scaleRef - rangeSub) / (float)barRange) * 0.59f;

    // Clamp scaleFactor to prevent issues
    if (scaleFactor < 0.f) {  
        scaleFactor = 0.f;
    } 
    if (scaleFactor > 0.59f) {
        scaleFactor = 0.59f;
    }

    // Draw shaded background bar (gray/blue)
    create_dl_translation_matrix(MENU_MTX_PUSH, OPTION_BAR_X, y + 13, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 0.59f, 0.1f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 50, 50, 50, gCurrentMenuSection == index ? 120 : 80);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Draw yellow bar with dynamic scale
    create_dl_translation_matrix(MENU_MTX_PUSH, OPTION_BAR_X, y + 13, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, scaleFactor, 0.1f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCurrentMenuSection == index ? 255 : 120);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

static const Vtx vertex_triangle_inverse[] = {
    {{{     0,     16,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{    -8,      8,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
    {{{     0,      0,      0}, 0, {     0,      0}, {0xff, 0xff, 0xff, 0xff}}},
};

const Gfx dl_draw_triangle_inverse[] = {
    gsSPClearGeometryMode(G_LIGHTING),
    gsDPSetCombineMode(G_CC_FADE, G_CC_FADE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsDPSetTextureFilter(G_TF_POINT),
    gsSPVertex(vertex_triangle_inverse, 3, 0),
    gsSP1Triangle( 0,  1,  2, 0x0),
    gsSPSetGeometryMode(G_LIGHTING),
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// Animation variables for option arrows
static s16 sLeftArrowNudgeTimer = 0;
static s16 sRightArrowNudgeTimer = 0;
static s8 sLeftArrowNudgeIndex = -1;
static s8 sRightArrowNudgeIndex = -1;

// Arrow animation constants
#define ARROW_NUDGE_DISTANCE 4
#define ARROW_NUDGE_DURATION 15

void draw_animated_option_arrows(s16 x, s16 y, s16 secArrDist, s8 arrowIndex) {
    // Animate timers
    if (sLeftArrowNudgeTimer > 0) {
        sLeftArrowNudgeTimer--;
        if (sLeftArrowNudgeTimer == 0) sLeftArrowNudgeIndex = -1;
    }
    if (sRightArrowNudgeTimer > 0) {
        sRightArrowNudgeTimer--;
        if (sRightArrowNudgeTimer == 0) sRightArrowNudgeIndex = -1;
    }

    // Calculate positions with animation
    s16 leftX = x;
    s16 rightX = x + secArrDist;

    // Apply left arrow nudge if active for this index
    if (sLeftArrowNudgeIndex == arrowIndex && sLeftArrowNudgeTimer > 0) {
        leftX -= (ARROW_NUDGE_DISTANCE * sLeftArrowNudgeTimer) / ARROW_NUDGE_DURATION;
    }

    // Apply right arrow nudge if active for this index
    if (sRightArrowNudgeIndex == arrowIndex && sRightArrowNudgeTimer > 0) {
        rightX += (ARROW_NUDGE_DISTANCE * sRightArrowNudgeTimer) / ARROW_NUDGE_DURATION;
    }

    // Draw left arrow (yellow)
    create_dl_translation_matrix(MENU_MTX_PUSH, leftX, y + 3, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 0.7f, 0.7f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCurrentMenuSection == arrowIndex ? 255 : 120); // Yellow color
    gSPDisplayList (gDisplayListHead++, dl_draw_triangle_inverse);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    
    // Draw right arrow (yellow)
    create_dl_translation_matrix(MENU_MTX_PUSH, rightX, y + 3, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 0.7f, 0.7f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gCurrentMenuSection == arrowIndex ? 255 : 120); // Yellow color
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void nudge_left_arrow_photo_mode(s8 index) {
    sLeftArrowNudgeIndex = index;
    sLeftArrowNudgeTimer = ARROW_NUDGE_DURATION;
}

void nudge_right_arrow_photo_mode(s8 index) {
    sRightArrowNudgeIndex = index;
    sRightArrowNudgeTimer = ARROW_NUDGE_DURATION;
}

s16 gRotationAmount = 0;
s16 gXPosition = 0;
s16 gYPosition = 0;
s16 gZPosition = 0;
s16 gHeadRotationX = 0;
s16 gHeadRotationY = 0;
s16 gHeadRotationZ = 0;
s16 gTorsoRotationX = 0;
s16 gTorsoRotationY = 0;
s16 gTorsoRotationZ = 0;

void handle_section_navigation_sound(s16 *value, s16 min, s16 max) {
    static s16 previousValue[360] = { -1 };

    if (*value >= min && *value < max) {
        if (*value != previousValue[gCurrentMenuSection]) {
            play_sound(SOUND_MENU_MESSAGE_NEXT_PAGE, gGlobalSoundSource);
            previousValue[gCurrentMenuSection] = *value;
        }
    }
}

void section_navigation(s16 *sectionIndex, s8 maxCases, int enableWrap, int enableHold) {
    // Handle right D-pad button (scroll right)
    if (gPlayer1Controller->buttonPressed & R_JPAD) {
        s8 oldIndex = *sectionIndex;
        (*sectionIndex)++;
        if (enableWrap) {
            if (*sectionIndex >= maxCases) {
                *sectionIndex = 0;  // Loop back to the first case if beyond max
            }
        } else {
            if (*sectionIndex >= maxCases) {
                *sectionIndex = maxCases - 1;  // Stop at the last valid index
            }
        }
        if (*sectionIndex != oldIndex) {
            nudge_right_arrow_photo_mode(gCurrentMenuSection);
        }
        rightButtonHoldCounter = 0;  // Reset the hold counter when the button is pressed
    }

    // Handle left D-pad button (scroll left)
    if (gPlayer1Controller->buttonPressed & L_JPAD) {
        s8 oldIndex = *sectionIndex;
        (*sectionIndex)--;
        if (enableWrap) {
            if (*sectionIndex < 0) {
                *sectionIndex = maxCases - 1;  // Loop back to the last case if less than 0
            }
        } else {
            if (*sectionIndex < 0) {
                *sectionIndex = 0;  // Stop at the first valid index
            }
        }
        if (*sectionIndex != oldIndex) {
            nudge_left_arrow_photo_mode(gCurrentMenuSection);
        }
        leftButtonHoldCounter = 0;  // Reset the hold counter when the button is pressed
    }

    // Handle button held for scrolling right
    if (enableHold && (gPlayer1Controller->buttonDown & R_JPAD)) {
        rightButtonHoldCounter++;
        if (rightButtonHoldCounter >= HOLD_THRESHOLD) {
            if (rightButtonHoldCounter % HOLD_DELAY == 0) {  // Scroll every few frames
                s8 oldIndex = *sectionIndex;
                (*sectionIndex)++;
                if (enableWrap) {
                    if (*sectionIndex >= maxCases) {
                        *sectionIndex = 0;  // Loop back to the first case
                    }
                } else {
                    if (*sectionIndex >= maxCases) {
                        *sectionIndex = maxCases - 1;  // Stop at the last valid index
                    }
                }
                if (*sectionIndex != oldIndex) {
                    nudge_right_arrow_photo_mode(gCurrentMenuSection);
                }
            }
        }
    } else {
        rightButtonHoldCounter = 0;
    }

    // Handle button held for scrolling left
    if (enableHold && (gPlayer1Controller->buttonDown & L_JPAD)) {
        leftButtonHoldCounter++;
        if (leftButtonHoldCounter >= HOLD_THRESHOLD) {
            if (leftButtonHoldCounter % HOLD_DELAY == 0) {  // Scroll every few frames
                s8 oldIndex = *sectionIndex;
                (*sectionIndex)--;
                if (enableWrap) {
                    if (*sectionIndex < 0) {
                        *sectionIndex = maxCases - 1;  // Loop back to the last case
                    }
                } else {
                    if (*sectionIndex < 0) {
                        *sectionIndex = 0;  // Stop at the first valid index
                    }
                }
                if (*sectionIndex != oldIndex) {
                    nudge_left_arrow_photo_mode(gCurrentMenuSection);
                }
            }
        }
    } else {
        leftButtonHoldCounter = 0;
    }
}

static s16 gInitialCapState;
static int gIsInitialCapStateSaved = false;
s8 initialCapStateIndex;

void save_initial_cap_state(void) {
    if (!gIsInitialCapStateSaved) {
        // Detect the current cap state and set the default option accordingly
        if (marioState->marioBodyState->capState == MARIO_HAS_DEFAULT_CAP_ON) {
            gInitialCapState = MARIO_HAS_DEFAULT_CAP_ON;
            initialCapStateIndex = 0;
        } else if (marioState->marioBodyState->capState == MARIO_HAS_DEFAULT_CAP_OFF) {
            gInitialCapState = MARIO_HAS_DEFAULT_CAP_OFF;
            initialCapStateIndex = 1;
        } else if (marioState->marioBodyState->capState == MARIO_HAS_WING_CAP_ON) {
            gInitialCapState = MARIO_HAS_WING_CAP_ON;
            initialCapStateIndex = 2;
        }
        gCapStateIndex = initialCapStateIndex;
        gIsInitialCapStateSaved = true;
    }
}

void restore_initial_cap_state(void) {
    if (gIsInitialCapStateSaved) {
        marioState->marioBodyState->capState = gInitialCapState;
    }
}

static s16 gInitialHandState;
static int gIsInitialHandStateSaved = false;
s8 initialHandStateIndex;

void save_initial_hand_state(void) {
    if (!gIsInitialHandStateSaved) {
        // Detect the current hand state and set the default option accordingly
        if (marioState->marioBodyState->handState == MARIO_HAND_FISTS) {
            gInitialHandState = MARIO_HAND_FISTS;
            initialHandStateIndex = 0;
        } else if (marioState->marioBodyState->handState == MARIO_HAND_PEACE_SIGN) {
            gInitialHandState = MARIO_HAND_PEACE_SIGN;
            initialHandStateIndex = 1;
        } else if (marioState->marioBodyState->handState == MARIO_HAND_OPEN) {
            gInitialHandState = MARIO_HAND_OPEN;
            initialHandStateIndex = 2;
        } else if (marioState->marioBodyState->handState == MARIO_HAND_RIGHT_OPEN) {
            gInitialHandState = MARIO_HAND_RIGHT_OPEN;
            initialHandStateIndex = 3;
        } else if (marioState->marioBodyState->handState == MARIO_HAND_HOLDING_CAP) {
            gInitialHandState = MARIO_HAND_HOLDING_CAP;
            initialHandStateIndex = 4;
        } else if (marioState->marioBodyState->handState == MARIO_HAND_HOLDING_WING_CAP) {
            gInitialHandState = MARIO_HAND_HOLDING_WING_CAP;
            initialHandStateIndex = 5;
        }
        gHandStateIndex = initialHandStateIndex;
        gIsInitialHandStateSaved = true;
    }
}

void restore_initial_hand_state(void) {
    if (gIsInitialHandStateSaved) {
        marioState->marioBodyState->handState = gInitialHandState;
    }
}

static s16 gInitialEyeState;
static int gIsInitialEyeStateSaved = false;
s8 initialEyeStateIndex;

void save_initial_eye_state(void) {
    if (!gIsInitialEyeStateSaved) {
        // Detect the current eye state and set the default option accordingly
        if (marioState->marioBodyState->eyeState == MARIO_EYES_OPEN) {
            gInitialEyeState = MARIO_EYES_OPEN;
            initialEyeStateIndex = 0;
        } else if (marioState->marioBodyState->eyeState == MARIO_EYES_HALF_CLOSED) {
            gInitialEyeState = MARIO_EYES_HALF_CLOSED;
            initialEyeStateIndex = 1;
        } else if (marioState->marioBodyState->eyeState == MARIO_EYES_CLOSED) {
            gInitialEyeState = MARIO_EYES_CLOSED;
            initialEyeStateIndex = 2;
        } else if (marioState->marioBodyState->eyeState == MARIO_EYES_DEAD) {
            gInitialEyeState = MARIO_EYES_DEAD;
            initialEyeStateIndex = 3;
        } else { // Normal state set to open as default
            gInitialEyeState = MARIO_EYES_OPEN;
            initialEyeStateIndex = 0;
        }
        gEyeStateIndex = initialEyeStateIndex;
        gIsInitialEyeStateSaved = true;
    }
}

void restore_initial_eye_state(void) {
    if (gIsInitialEyeStateSaved) {
        marioState->marioBodyState->eyeState = gInitialEyeState;
    }
}

static s16 gInitialPowerup;
static int gIsInitialPowerupSaved = false;
s8 initialPowerupIndex;

void save_initial_powerup(void) {
    if (!gIsInitialPowerupSaved) {
        // Detect the current powerup state and set the default option accordingly
        if (marioState->marioBodyState->modelState == 0) {
            gInitialPowerup = 0;
            initialPowerupIndex = 0;
        } else if (marioState->marioBodyState->modelState == MODEL_STATE_NOISE_ALPHA) {
            gInitialPowerup = MODEL_STATE_NOISE_ALPHA;
            initialPowerupIndex = 1;
        } else if (marioState->marioBodyState->modelState == MODEL_STATE_METAL) {
            gInitialPowerup = MODEL_STATE_METAL;
            initialPowerupIndex = 2;
        }
        gPowerupIndex = initialPowerupIndex;
        gIsInitialPowerupSaved = true;
    }
}

void restore_initial_powerup(void) {
    if (gIsInitialPowerupSaved) {
        marioState->marioBodyState->modelState = gInitialPowerup;
    }
}

static void force_anim_frame(s16 frame) {
    struct AnimInfo *a = &marioState->marioObj->header.gfx.animInfo;

    a->animFrame = frame;
    a->animFrameAccelAssist = (frame << 16);
    a->animAccel = 0;
}

static s16 gInitialPoseAnimID;
static s16 gInitialPoseFrame;
static int gIsInitialPoseSaved = false;

void save_initial_pose(void) {
    if (!gIsInitialPoseSaved) {
        if (marioState->marioObj->header.gfx.animInfo.animFrame <= 0) {
            marioState->marioObj->header.gfx.animInfo.animFrame = 1;
        }
        gInitialPoseAnimID = marioState->marioObj->header.gfx.animInfo.animID;
        gInitialPoseFrame = marioState->marioObj->header.gfx.animInfo.animFrame;
        gIsInitialPoseSaved = true;
    }
}

void restore_initial_pose(void) {
    if (gIsInitialPoseSaved) {
        set_character_animation(marioState, gInitialPoseAnimID);
        force_anim_frame(gInitialPoseFrame);
    }
}

static s16 gInitialRotation;
static int gIsInitialRotationSaved = false;

#define UPDATE_INCREMENT 1 // Adjust this value to increase rotation speed

#define MULTIPLIER_THRESHOLD 45

void save_initial_rotation(void) {
    if (!gIsInitialRotationSaved) {
        gInitialRotation = marioState->marioObj->header.gfx.angle[1] / (0x10000 / 360);
        gRotationAmount = 180;
        gIsInitialRotationSaved = true;
    }
}

void restore_initial_rotation(void) {
    if (gIsInitialRotationSaved) {
        marioState->marioObj->header.gfx.angle[1] = gInitialRotation * (0x10000 / 360);
        gRotationAmount = 180;
    }
}

void rotation_updater(void) {
    // Increment rotation when right D-pad is pressed
    if (gPlayer1Controller->buttonPressed & R_JPAD) {
        gRotationAmount = (gRotationAmount - UPDATE_INCREMENT + 360) % 360;
        rightButtonHoldCounter = 0; // Reset hold counter
    }

    // Decrement rotation when left D-pad is pressed
    if (gPlayer1Controller->buttonPressed & L_JPAD) {
        gRotationAmount = (gRotationAmount + UPDATE_INCREMENT) % 360;
        leftButtonHoldCounter = 0; // Reset hold counter
    }

    // Handle continuous rotation increase when right D-pad is held
    if (gPlayer1Controller->buttonDown & R_JPAD) {
        rightButtonHoldCounter++;
        s16 decrement = (rightButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (rightButtonHoldCounter >= HOLD_THRESHOLD && rightButtonHoldCounter % HOLD_DELAY == 0) {
            gRotationAmount = (gRotationAmount - decrement + 360) % 360;
        }
    } else {
        rightButtonHoldCounter = 0;
    }

    // Handle continuous rotation decrease when left D-pad is held
    if (gPlayer1Controller->buttonDown & L_JPAD) {
        leftButtonHoldCounter++;
        s16 increment = (leftButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (leftButtonHoldCounter >= HOLD_THRESHOLD && leftButtonHoldCounter % HOLD_DELAY == 0) {
            gRotationAmount = (gRotationAmount + increment) % 360;
        }
    } else {
        leftButtonHoldCounter = 0;
    }

    // Update Mario's rotation based on the adjusted rotation amount
    marioState->marioObj->header.gfx.angle[1] = ((gInitialRotation + gRotationAmount) * (0x10000 / 360)) - 0x8000;
}

static s16 gInitialXPosition;
static s16 gInitialYPosition;
static s16 gInitialZPosition;
static int gIsInitialXPositionSaved = false;
static int gIsInitialYPositionSaved = false;
static int gIsInitialZPositionSaved = false;

void save_initial_position(void) {
    if (!gIsInitialXPositionSaved) {
        gInitialXPosition = marioState->marioObj->header.gfx.pos[0];
        gXPosition = 100;
        gIsInitialXPositionSaved = true;
    }
    if (!gIsInitialYPositionSaved) {
        gInitialYPosition = marioState->marioObj->header.gfx.pos[2];
        gYPosition = 100;
        gIsInitialYPositionSaved = true;
    }
    if (!gIsInitialZPositionSaved) {
        gInitialZPosition = marioState->marioObj->header.gfx.pos[1];
        gZPosition = 100;
        gIsInitialZPositionSaved = true;
    }
}

void restore_initial_position(void) {
    if (gIsInitialXPositionSaved) {
        marioState->marioObj->header.gfx.pos[0] = gInitialXPosition;
        gXPosition = 100;
    }

    if (gIsInitialYPositionSaved) {
        marioState->marioObj->header.gfx.pos[2] = gInitialYPosition;
        gYPosition = 100;
    }

    if (gIsInitialZPositionSaved) {
        marioState->marioObj->header.gfx.pos[1] = gInitialZPosition;
        gZPosition = 100;
    }
}

void mario_position_updater(s16 *position, s16 initialPosition, u8 positionAxis) {
    if (gPlayer1Controller->buttonPressed & R_JPAD) {
        if (*position < 200) {
            *position = (*position + UPDATE_INCREMENT);
        }
    }

    if (gPlayer1Controller->buttonPressed & L_JPAD) {
        if (*position > 0) {
            *position = (*position - UPDATE_INCREMENT);
        }
    }

    // Handle continuous movement when right D-pad is held
    if (gPlayer1Controller->buttonDown & R_JPAD) {
        rightButtonHoldCounter++;
        s16 increment = (rightButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (rightButtonHoldCounter >= HOLD_THRESHOLD && rightButtonHoldCounter % HOLD_DELAY == 0) {
            if (*position < 200) {
                *position = (*position + increment);
            } else {
                *position = 200;
            }
        }
    } else {
        rightButtonHoldCounter = 0;
    }

    // Handle continuous movement when left D-pad is held
    if (gPlayer1Controller->buttonDown & L_JPAD) {
        leftButtonHoldCounter++;
        s16 decrement = (leftButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (leftButtonHoldCounter >= HOLD_THRESHOLD && leftButtonHoldCounter % HOLD_DELAY == 0) {
            if (*position > 0) {
                *position = (*position - decrement);
            } else {
                *position = 0;
            }
        }
    } else {
        leftButtonHoldCounter = 0;
    }
    
    // Update Mario's position
    s16 offset = (*position - 100) * 2; // Convert to -100 to +100 range
    marioState->marioObj->header.gfx.pos[positionAxis] = initialPosition + offset;
}

static s16 gFOVValue = 45; // Initial FOV value, starts at the middle
struct CameraFOVStatus sFOVState;

void fov_updater(void) {
    static s8 previousFOVValue = -1;

    // Handle right D-pad button (increase FOV)
    if (gPlayer1Controller->buttonPressed & R_JPAD) {
        if (gFOVValue < 100) {
            gFOVValue++;
        }
        rightButtonHoldCounter = 0;  // Reset the hold counter when the button is pressed
    }

    // Handle left D-pad button (decrease FOV)
    if (gPlayer1Controller->buttonPressed & L_JPAD) {
        if (gFOVValue > 10) {
            gFOVValue--;
        }
        leftButtonHoldCounter = 0;  // Reset the hold counter when the button is pressed
    }

    // Handle button held for scrolling right
    if (gPlayer1Controller->buttonDown & R_JPAD) {
        rightButtonHoldCounter++;
        if (rightButtonHoldCounter >= HOLD_THRESHOLD) {
            if (rightButtonHoldCounter % HOLD_DELAY == 0) {  // Scroll every few frames
                if (gFOVValue < 100) {
                    gFOVValue++;
                }
            }
        }
    }

    // Handle button held for scrolling left
    if (gPlayer1Controller->buttonDown & L_JPAD) {
        leftButtonHoldCounter++;
        if (leftButtonHoldCounter >= HOLD_THRESHOLD) {
            if (leftButtonHoldCounter % HOLD_DELAY == 0) {  // Scroll every few frames
                if (gFOVValue > 10) {
                    gFOVValue--;
                }
            }
        }
    }
    
    if (gFOVValue != previousFOVValue) {
        play_sound(SOUND_MENU_MESSAGE_NEXT_PAGE, gGlobalSoundSource);
        previousFOVValue = gFOVValue;
    }

    gFOVState.fov = gFOVValue;  // Update the actual FOV
}

static bool sSavedForce4By3 = false;
static bool sSavedForce21By9 = false;

void photo_mode_apply_frame_override(void) {
    switch (gFrameIndex) {
        default:
        case 0: // None
            configForce4By3 = sSavedForce4By3;
            configForce21By9 = sSavedForce21By9;
            break;
        case 1: // Cinematic
            configForce4By3 = false;
            configForce21By9 = true;
            break;
        case 2: // 4:3
            configForce4By3 = true;
            configForce21By9 = false;
            break;
    }
}

void pose_options(void) {
    CREATE_STRING(textDefault, "Default");

    CREATE_STRING(textPoseOptions, "Pose Options");

    CREATE_STRING(textPose, "Pose");
    CREATE_STRING(textHandState, "Hands");
    CREATE_STRING(textRotation, "Rotation");
    CREATE_STRING(textXPosition, "X Position");
    CREATE_STRING(textYPosition, "Y Position");
    CREATE_STRING(textZPosition, "Z Position");

    CREATE_STRING(textPoseStanding, "Standing");
    CREATE_STRING(textPoseTiptoe, "Tiptoe");
    CREATE_STRING(textPoseWalking, "Walking");
    CREATE_STRING(textPoseRunning, "Running");
    CREATE_STRING(textPosePunch, "Punch");
    CREATE_STRING(textPoseDiving, "Diving");
    CREATE_STRING(textPoseSlideKick, "Slide Kick");
    CREATE_STRING(textPoseSkid, "Skid");
    CREATE_STRING(textPoseKneel, "Kneel");
    CREATE_STRING(textPoseZombieWalk, "Zombie Walk");
    CREATE_STRING(textPoseZombieRun, "Zombie Run");
    CREATE_STRING(textPoseSquat, "Squat");
    CREATE_STRING(textPoseScared, "Scared");
    CREATE_STRING(textPoseWaving, "Waving");
    CREATE_STRING(textPoseLookLeft, "Look Left");
    CREATE_STRING(textPoseLookRight, "Look Right");
    CREATE_STRING(textPosePointUp, "Point Up");
    CREATE_STRING(textPoseLookUp, "Look Up");
    CREATE_STRING(textPoseLookBehind, "Look Behind");
    CREATE_STRING(textPoseTipCap, "Tip Cap");
    CREATE_STRING(textPoseDead1, "Dead 1");
    CREATE_STRING(textPoseDead2, "Dead 2");
    CREATE_STRING(textPoseDead3, "Dead 3");
    CREATE_STRING(textPoseChoking, "Choking");
    CREATE_STRING(textPoseCoughing, "Coughing");
    CREATE_STRING(textPoseDrowsy, "Drowsy");
    CREATE_STRING(textPoseSlip, "Slip");
    CREATE_STRING(textPoseGrabCap, "Grab Cap");
    CREATE_STRING(textPoseTalking, "Talking");
    CREATE_STRING(textPoseDriving, "Driving");
    CREATE_STRING(textPoseShrug, "Shrug");
    CREATE_STRING(textPosePitch1, "Pitch 1");
    CREATE_STRING(textPosePitch2, "Pitch 2");
    CREATE_STRING(textPoseVault, "Vault");
    CREATE_STRING(textPoseYawning, "Yawning");
    CREATE_STRING(textPoseTired, "Tired");
    CREATE_STRING(textPoseSitting, "Sitting");
    CREATE_STRING(textPoseLaying, "Laying");
    CREATE_STRING(textPoseCrouch, "Crouch");
    CREATE_STRING(textPoseCrawling, "Crawling");
    CREATE_STRING(textPoseArmsOut, "Arms Out");
    CREATE_STRING(textPoseStarDance1, "Star Dance 1");

    CREATE_STRING(textSingleJump, "Single Jump");
    CREATE_STRING(textHop, "Hop");
    CREATE_STRING(textDoubleJump, "Double Jump");
    CREATE_STRING(textHighJump, "High Jump");
    CREATE_STRING(textJumpKick, "Jump Kick");
    CREATE_STRING(textPunchJump, "Punch Jump");
    CREATE_STRING(textLongJump1, "Long Jump 1");
    CREATE_STRING(textLongJump2, "Long Jump 2");
    CREATE_STRING(textLongJump3, "Long Jump 3");
    CREATE_STRING(textBurningAir, "Burning Air");
    CREATE_STRING(textFlying, "Flying");
    CREATE_STRING(textHanging, "Hanging");
    CREATE_STRING(textGroundPound, "Ground Pound");
    CREATE_STRING(textCannonball, "Cannonball");
    CREATE_STRING(textSlamDunk, "Slam Dunk");
    CREATE_STRING(textFreefall, "Freefall");
    CREATE_STRING(textFalling, "Falling");
    CREATE_STRING(textAscent, "Ascent");
    CREATE_STRING(textBackwardFall, "Backward Fall");
    CREATE_STRING(textTwirl, "Twirl");
    CREATE_STRING(textSwimming, "Swimming");
    CREATE_STRING(textStarDance2, "Star Dance 2");

    CREATE_STRING(textHandFists, "Fists");
    CREATE_STRING(textHandPeaceSign, "Peace");
    CREATE_STRING(textOpen, "Open");
    CREATE_STRING(textHandRightOpen, "Right Open");
    CREATE_STRING(textHandHoldingCap, "Holding Cap");
    CREATE_STRING(textWingCap, "Wing Cap");

    int poseOptions = 0;

    if (is_mario_on_ground()) {
        poseOptions = 0;
    } else {
        poseOptions = 1;
    }

    switch (gCurrentMenuSection) {
        case 1:
            if (poseOptions == 0) {
                section_navigation(&gGroundPoseIndex, 43, true, true);
                handle_section_navigation_sound(&gGroundPoseIndex, 0, 43);
            } else {
                section_navigation(&gAirPoseIndex, 23, true, true);
                handle_section_navigation_sound(&gAirPoseIndex, 0, 23);
            }
            break;
        case 2:
            section_navigation(&gHandStateIndex, 6, true, true);
            handle_section_navigation_sound(&gHandStateIndex, 0, 6);
            break;
        case 3:
            rotation_updater();
            handle_section_navigation_sound(&gRotationAmount, 0, 360);
            break;
        case 4:
            mario_position_updater(&gXPosition, gInitialXPosition, 0);
            handle_section_navigation_sound(&gXPosition, 0, 200);
            break;
        case 5:
            mario_position_updater(&gYPosition, gInitialYPosition, 2);
            handle_section_navigation_sound(&gYPosition, 0, 200);
            break;
        case 6:
            mario_position_updater(&gZPosition, gInitialZPosition, 1);
            handle_section_navigation_sound(&gZPosition, 0, 200);
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    set_option_alpha(0);
    
    print_generic_string(centeredX(PAGE_X, textPoseOptions), PAGE_Y, textPoseOptions);

    set_option_alpha(1);
    print_generic_string(OPTION_X, OPTION_1_Y, textPose);

    if (is_mario_on_ground()) {
        switch (gGroundPoseIndex) {
            case 0:
                print_generic_string(centeredX(LARGE_X, textDefault), OPTION_1_Y, textDefault);
                restore_initial_pose();
                break;
            case 1:
                print_generic_string(centeredX(LARGE_X, textPoseStanding), OPTION_1_Y, textPoseStanding);
                set_character_animation(marioState, CHAR_ANIM_IDLE_HEAD_CENTER);
                force_anim_frame(29);
                break;
            case 2:
                print_generic_string(centeredX(LARGE_X, textPoseTiptoe), OPTION_1_Y, textPoseTiptoe);
                set_character_animation(marioState, CHAR_ANIM_TIPTOE);
                force_anim_frame(65);
                break;
            case 3:
                print_generic_string(centeredX(LARGE_X, textPoseWalking), OPTION_1_Y, textPoseWalking);
                set_character_animation(marioState, CHAR_ANIM_WALKING);
                force_anim_frame(31);
                break;
            case 4:
                print_generic_string(centeredX(LARGE_X, textPoseRunning), OPTION_1_Y, textPoseRunning);
                set_character_animation(marioState, CHAR_ANIM_RUNNING);
                force_anim_frame(30);
                break;
            case 5:
                print_generic_string(centeredX(LARGE_X, textPosePunch), OPTION_1_Y, textPosePunch);
                set_character_animation(marioState, CHAR_ANIM_FIRST_PUNCH);
                force_anim_frame(4);
                break;
            case 6:
                print_generic_string(centeredX(LARGE_X, textPoseDiving), OPTION_1_Y, textPoseDiving);
                set_character_animation(marioState, CHAR_ANIM_SLIDE_DIVE);
                force_anim_frame(19);
                break;
            case 7:
                print_generic_string(centeredX(LARGE_X, textPoseSlideKick), OPTION_1_Y, textPoseSlideKick);
                set_character_animation(marioState, CHAR_ANIM_FALL_FROM_SLIDE_KICK);
                force_anim_frame(1);
                break;
            case 8:
                print_generic_string(centeredX(LARGE_X, textPoseSkid), OPTION_1_Y, textPoseSkid);
                set_character_animation(marioState, CHAR_ANIM_SKID_ON_GROUND);
                force_anim_frame(1);
                break;
            case 9:
                print_generic_string(centeredX(LARGE_X, textPoseKneel), OPTION_1_Y, textPoseKneel);
                set_character_animation(marioState, CHAR_ANIM_STOP_SKID);
                force_anim_frame(8);
                break;
            case 10:
                print_generic_string(centeredX(LARGE_X, textPoseZombieWalk), OPTION_1_Y, textPoseZombieWalk);
                set_character_animation(marioState, CHAR_ANIM_WALK_WITH_LIGHT_OBJ);
                force_anim_frame(70);
                break;
            case 11:
                print_generic_string(centeredX(LARGE_X, textPoseZombieRun), OPTION_1_Y, textPoseZombieRun);
                set_character_animation(marioState, CHAR_ANIM_RUN_WITH_LIGHT_OBJ);
                force_anim_frame(48);
                break;
            case 12:
                print_generic_string(centeredX(LARGE_X, textPoseSquat), OPTION_1_Y, textPoseSquat);
                set_character_animation(marioState, CHAR_ANIM_SHIVERING_WARMING_HAND);
                force_anim_frame(37);
                break;
            case 13:
                print_generic_string(centeredX(LARGE_X, textPoseScared), OPTION_1_Y, textPoseScared);
                set_character_animation(marioState, CHAR_ANIM_SHIVERING_WARMING_HAND);
                force_anim_frame(63);
                break;
            case 14:
                print_generic_string(centeredX(LARGE_X, textPoseWaving), OPTION_1_Y, textPoseWaving);
                set_character_animation(marioState, CHAR_ANIM_CREDITS_WAVING);
                force_anim_frame(9);
                break;
            case 15:
                print_generic_string(centeredX(LARGE_X, textPoseLookLeft), OPTION_1_Y, textPoseLookLeft);
                set_character_animation(marioState, CHAR_ANIM_CREDITS_RAISE_HAND);
                force_anim_frame(22);
                break;
            case 16:
                print_generic_string(centeredX(LARGE_X, textPoseLookRight), OPTION_1_Y, textPoseLookRight);
                set_character_animation(marioState, CHAR_ANIM_CREDITS_RAISE_HAND);
                force_anim_frame(45);
                break;
            case 17:
                print_generic_string(centeredX(LARGE_X, textPosePointUp), OPTION_1_Y, textPosePointUp);
                set_character_animation(marioState, CHAR_ANIM_CREDITS_RAISE_HAND);
                force_anim_frame(95);
                break;
            case 18:
                print_generic_string(centeredX(LARGE_X, textPoseLookUp), OPTION_1_Y, textPoseLookUp);
                set_character_animation(marioState, CHAR_ANIM_CREDITS_LOWER_HAND);
                force_anim_frame(34);
                break;
            case 19:
                print_generic_string(centeredX(LARGE_X, textPoseLookBehind), OPTION_1_Y, textPoseLookBehind);
                set_character_animation(marioState, CHAR_ANIM_MISSING_CAP);
                force_anim_frame(74);
                break;
            case 20:
                print_generic_string(centeredX(LARGE_X, textPoseTipCap), OPTION_1_Y, textPoseTipCap);
                set_character_animation(marioState, CHAR_ANIM_CREDITS_TAKE_OFF_CAP);
                force_anim_frame(9);
                break;
            case 21:
                print_generic_string(centeredX(LARGE_X, textPoseDead1), OPTION_1_Y, textPoseDead1);
                set_character_animation(marioState, CHAR_ANIM_DYING_ON_BACK);
                force_anim_frame(46);
                break;
            case 22:
                print_generic_string(centeredX(LARGE_X, textPoseDead2), OPTION_1_Y, textPoseDead2);
                set_character_animation(marioState, CHAR_ANIM_DYING_ON_STOMACH);
                force_anim_frame(63);
                break;
            case 23:
                print_generic_string(centeredX(LARGE_X, textPoseDead3), OPTION_1_Y, textPoseDead3);
                set_character_animation(marioState, CHAR_ANIM_ELECTROCUTION);
                force_anim_frame(52);
                break;
            case 24:
                print_generic_string(centeredX(LARGE_X, textPoseChoking), OPTION_1_Y, textPoseChoking);
                set_character_animation(marioState, CHAR_ANIM_SUFFOCATING);
                force_anim_frame(45);
                break;
            case 25:
                print_generic_string(centeredX(LARGE_X, textPoseCoughing), OPTION_1_Y, textPoseCoughing);
                set_character_animation(marioState, CHAR_ANIM_COUGHING);
                force_anim_frame(69);
                break;
            case 26:
                print_generic_string(centeredX(LARGE_X, textPoseDrowsy), OPTION_1_Y, textPoseDrowsy);
                set_character_animation(marioState, CHAR_ANIM_DYING_FALL_OVER);
                force_anim_frame(11);
                break;
            case 27:
                print_generic_string(centeredX(LARGE_X, textPoseSlip), OPTION_1_Y, textPoseSlip);
                set_character_animation(marioState, CHAR_ANIM_DYING_FALL_OVER);
                force_anim_frame(71);
                break;
            case 28:
                print_generic_string(centeredX(LARGE_X, textPoseGrabCap), OPTION_1_Y, textPoseGrabCap);
                set_character_animation(marioState, CHAR_ANIM_PUT_CAP_ON);
                force_anim_frame(31);
                break;
            case 29:
                print_generic_string(centeredX(LARGE_X, textPoseTalking), OPTION_1_Y, textPoseTalking);
                set_character_animation(marioState, CHAR_ANIM_TAKE_CAP_OFF_THEN_ON);
                force_anim_frame(19);
                break;
            case 30:
                print_generic_string(centeredX(LARGE_X, textPoseDriving), OPTION_1_Y, textPoseDriving);
                set_character_animation(marioState, CHAR_ANIM_SLIDING_ON_BOTTOM_WITH_LIGHT_OBJ);
                force_anim_frame(1);
                break;
            case 31:
                print_generic_string(centeredX(LARGE_X, textPoseShrug), OPTION_1_Y, textPoseShrug);
                set_character_animation(marioState, CHAR_ANIM_MISSING_CAP);
                force_anim_frame(123);
                break;
            case 32:
                print_generic_string(centeredX(LARGE_X, textPosePitch1), OPTION_1_Y, textPosePitch1);
                set_character_animation(marioState, CHAR_ANIM_GROUND_THROW);
                force_anim_frame(1);
                break;
            case 33:
                print_generic_string(centeredX(LARGE_X, textPosePitch2), OPTION_1_Y, textPosePitch2);
                set_character_animation(marioState, CHAR_ANIM_GROUND_THROW);
                force_anim_frame(9);
                break;
            case 34:
                print_generic_string(centeredX(LARGE_X, textPoseVault), OPTION_1_Y, textPoseVault);
                set_character_animation(marioState, CHAR_ANIM_BREAKDANCE);
                force_anim_frame(8);
                break;
            case 35:
                print_generic_string(centeredX(LARGE_X, textPoseYawning), OPTION_1_Y, textPoseYawning);
                set_character_animation(marioState, CHAR_ANIM_START_SLEEP_YAWN);
                force_anim_frame(34);
                break;
            case 36:
                print_generic_string(centeredX(LARGE_X, textPoseTired), OPTION_1_Y, textPoseTired);
                set_character_animation(marioState, CHAR_ANIM_WALK_PANTING);
                force_anim_frame(9);
                break;
            case 37:
                print_generic_string(centeredX(LARGE_X, textPoseSitting), OPTION_1_Y, textPoseSitting);
                set_character_animation(marioState, CHAR_ANIM_START_SLEEP_SITTING);
                force_anim_frame(33);
                break;
            case 38:
                print_generic_string(centeredX(LARGE_X, textPoseLaying), OPTION_1_Y, textPoseLaying);
                set_character_animation(marioState, CHAR_ANIM_SLEEP_START_LYING);
                force_anim_frame(39);
                break;
            case 39:
                print_generic_string(centeredX(LARGE_X, textPoseCrouch), OPTION_1_Y, textPoseCrouch);
                set_character_animation(marioState, CHAR_ANIM_CROUCHING);
                force_anim_frame(1);
                break;
            case 40:
                print_generic_string(centeredX(LARGE_X, textPoseCrawling), OPTION_1_Y, textPoseCrawling);
                set_character_animation(marioState, CHAR_ANIM_CRAWLING);
                force_anim_frame(50);
                break;
            case 41:
                print_generic_string(centeredX(LARGE_X, textPoseArmsOut), OPTION_1_Y, textPoseArmsOut);
                set_character_animation(marioState, CHAR_ANIM_TRIPLE_JUMP_LAND);
                force_anim_frame(12);
                break;
            case 42:
                print_generic_string(centeredX(LARGE_X, textPoseStarDance1), OPTION_1_Y, textPoseStarDance1);
                set_character_animation(marioState, CHAR_ANIM_STAR_DANCE);
                force_anim_frame(62);
                break;
        }
    } else {
        switch (gAirPoseIndex) {
            case 0:
                print_generic_string(centeredX(LARGE_X, textDefault), OPTION_1_Y, textDefault);
                restore_initial_pose();
                break;
            case 1:
                print_generic_string(centeredX(LARGE_X, textSingleJump), OPTION_1_Y, textSingleJump);
                set_character_animation(marioState, CHAR_ANIM_SINGLE_JUMP);
                force_anim_frame(12);
                break;
            case 2:
                print_generic_string(centeredX(LARGE_X, textHop), OPTION_1_Y, textHop);
                set_character_animation(marioState, CHAR_ANIM_START_FORWARD_SPINNING);
                force_anim_frame(1);
                break;
            case 3:
                print_generic_string(centeredX(LARGE_X, textDoubleJump), OPTION_1_Y, textDoubleJump);
                set_character_animation(marioState, CHAR_ANIM_DOUBLE_JUMP_RISE);
                force_anim_frame(6);
                break;
            case 4:
                print_generic_string(centeredX(LARGE_X, textHighJump), OPTION_1_Y, textHighJump);
                set_character_animation(marioState, CHAR_ANIM_FINAL_BOWSER_WING_CAP_TAKE_OFF);
                force_anim_frame(17);
                break;
            case 5:
                print_generic_string(centeredX(LARGE_X, textJumpKick), OPTION_1_Y, textJumpKick);
                set_character_animation(marioState, CHAR_ANIM_GROUND_KICK);
                force_anim_frame(6);
                break;
            case 6:
                print_generic_string(centeredX(LARGE_X, textPunchJump), OPTION_1_Y, textPunchJump);
                set_character_animation(marioState, CHAR_ANIM_FAST_LONGJUMP);
                force_anim_frame(4);
                break;
            case 7:
                print_generic_string(centeredX(LARGE_X, textLongJump1), OPTION_1_Y, textLongJump1);
                set_character_animation(marioState, CHAR_ANIM_FAST_LONGJUMP);
                force_anim_frame(23);
                break;
            case 8:
                print_generic_string(centeredX(LARGE_X, textLongJump2), OPTION_1_Y, textLongJump2);
                set_character_animation(marioState, CHAR_ANIM_SLOW_LONGJUMP);
                force_anim_frame(5);
                break;
            case 9:
                print_generic_string(centeredX(LARGE_X, textLongJump3), OPTION_1_Y, textLongJump3);
                set_character_animation(marioState, CHAR_ANIM_SLOW_LONGJUMP);
                force_anim_frame(15);
                break;
            case 10:
                print_generic_string(centeredX(LARGE_X, textBurningAir), OPTION_1_Y, textBurningAir);
                set_character_animation(marioState, CHAR_ANIM_FIRE_LAVA_BURN);
                force_anim_frame(6);
                break;
            case 11:
                print_generic_string(centeredX(LARGE_X, textFlying), OPTION_1_Y, textFlying);
                set_character_animation(marioState, CHAR_ANIM_WING_CAP_FLY);
                force_anim_frame(15);
                break;
            case 12:
                print_generic_string(centeredX(LARGE_X, textHanging), OPTION_1_Y, textHanging);
                set_character_animation(marioState, CHAR_ANIM_HANG_ON_OWL);
                force_anim_frame(12);
                break;
            case 13:
                print_generic_string(centeredX(LARGE_X, textGroundPound), OPTION_1_Y, textGroundPound);
                set_character_animation(marioState, CHAR_ANIM_GROUND_POUND);
                force_anim_frame(11);
                break;
            case 14:
                print_generic_string(centeredX(LARGE_X, textCannonball), OPTION_1_Y, textCannonball);
                set_character_animation(marioState, CHAR_ANIM_FORWARD_SPINNING);
                force_anim_frame(9);
                break;
            case 15:
                print_generic_string(centeredX(LARGE_X, textSlamDunk), OPTION_1_Y, textSlamDunk);
                set_character_animation(marioState, CHAR_ANIM_THROW_LIGHT_OBJECT);
                force_anim_frame(1);
                break;
            case 16:
                print_generic_string(centeredX(LARGE_X, textFreefall), OPTION_1_Y, textFreefall);
                set_character_animation(marioState, CHAR_ANIM_GENERAL_FALL);
                force_anim_frame(1);
                break;
            case 17:
                print_generic_string(centeredX(LARGE_X, textFalling), OPTION_1_Y, textFalling);
                set_character_animation(marioState, CHAR_ANIM_BACKFLIP);
                force_anim_frame(31);
                break;
            case 18:
                print_generic_string(centeredX(LARGE_X, textAscent), OPTION_1_Y, textAscent);
                set_character_animation(marioState, CHAR_ANIM_BACKFLIP);
                force_anim_frame(1);
                break;
            case 19:
                print_generic_string(centeredX(LARGE_X, textBackwardFall), OPTION_1_Y, textBackwardFall);
                set_character_animation(marioState, CHAR_ANIM_BEING_GRABBED);
                force_anim_frame(20);
                break;
            case 20:
                print_generic_string(centeredX(LARGE_X, textTwirl), OPTION_1_Y, textTwirl);
                set_character_animation(marioState, CHAR_ANIM_TWIRL);
                force_anim_frame(1);
                break;
            case 21:
                print_generic_string(centeredX(LARGE_X, textSwimming), OPTION_1_Y, textSwimming);
                set_character_animation(marioState, CHAR_ANIM_SWIM_PART1);
                force_anim_frame(13);
                break;
            case 22:
                print_generic_string(centeredX(LARGE_X, textStarDance2), OPTION_1_Y, textStarDance2);
                set_character_animation(marioState, CHAR_ANIM_WATER_STAR_DANCE);
                force_anim_frame(83);
                break;
        }
    }

    set_option_alpha(2);
    print_generic_string(OPTION_X, OPTION_2_Y, textHandState);

    switch (gHandStateIndex) {
        case 0:
            print_generic_string(centeredX(LARGE_X, textHandFists), OPTION_2_Y, textHandFists);
            marioState->marioBodyState->handState = MARIO_HAND_FISTS;
            break;
        case 1:
            print_generic_string(centeredX(LARGE_X, textHandPeaceSign), OPTION_2_Y, textHandPeaceSign);
            marioState->marioBodyState->handState = MARIO_HAND_PEACE_SIGN;
            break;
        case 2:
            print_generic_string(centeredX(LARGE_X, textOpen), OPTION_2_Y, textOpen);
            marioState->marioBodyState->handState = MARIO_HAND_OPEN;
            break;
        case 3:
            print_generic_string(centeredX(LARGE_X, textHandRightOpen), OPTION_2_Y, textHandRightOpen);
            marioState->marioBodyState->handState = MARIO_HAND_RIGHT_OPEN;
            break;
        case 4:
            print_generic_string(centeredX(LARGE_X, textHandHoldingCap), OPTION_2_Y, textHandHoldingCap);
            marioState->marioBodyState->handState = MARIO_HAND_HOLDING_CAP;
            break;
        case 5:
            print_generic_string(centeredX(LARGE_X, textWingCap), OPTION_2_Y, textWingCap);
            marioState->marioBodyState->handState = MARIO_HAND_HOLDING_WING_CAP;
            break;
    }

    // Player rotation
    set_option_alpha(3);
    print_generic_string(OPTION_X, OPTION_3_Y, textRotation);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_3_Y, -gRotationAmount);
    
    // Player X position
    set_option_alpha(4);
    print_generic_string(OPTION_X, OPTION_4_Y, textXPosition);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_4_Y, gXPosition);

    // Player Y position
    set_option_alpha(5);
    print_generic_string(OPTION_X, OPTION_5_Y, textYPosition);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_5_Y, gYPosition);

    // Player Z position
    set_option_alpha(6);
    print_generic_string(OPTION_X, OPTION_6_Y, textZPosition);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_6_Y, gZPosition);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    if (is_mario_on_ground()) {
        draw_animated_option_arrows(OPTION_ARROW_X, OPTION_1_Y, 85, 1);
    } else {
        draw_animated_option_arrows(OPTION_ARROW_X, OPTION_1_Y, 85, 1);
    }

    draw_animated_option_arrows(OPTION_ARROW_X, OPTION_2_Y, 85, 2);
    // Draw inverted rotation bar (full width - current position)
    draw_option_bar(OPTION_3_Y, 359, (360 - gRotationAmount) % 360, 3);
    draw_option_bar(OPTION_4_Y, 200, gXPosition, 4);
    draw_option_bar(OPTION_5_Y, 200, gYPosition, 5);
    draw_option_bar(OPTION_6_Y, 200, gZPosition, 6);
}

void body_options(void) {
    CREATE_STRING(textBodyOptions, "Body Options");

    CREATE_STRING(textCapState, "Caps");
    CREATE_STRING(textEyeState, "Eyes");
    CREATE_STRING(textPowerup, "Powerup");
    CREATE_STRING(textShadowVisibility, "Shadow Visibility");

    CREATE_STRING(textOn, "On");
    CREATE_STRING(textOff, "Off");
    CREATE_STRING(textWingCap, "Wing Cap");

    CREATE_STRING(textOpen, "Open");
    CREATE_STRING(textEyeHalfClosed, "Half Closed");
    CREATE_STRING(textEyeClosed, "Closed");
    CREATE_STRING(textEyeDead, "Dead");

    CREATE_STRING(textNone, "None");
    CREATE_STRING(textPowerupVanish, "Vanish");
    CREATE_STRING(textPowerupMetal, "Metal");

    u8 *textCapStateIndex = NULL;
    u8 *textEyeStateIndex = NULL;
    u8 *textPowerupIndex = NULL;

    switch (gCurrentMenuSection) {
        case 1:
            section_navigation(&gCapStateIndex, 3, true, true);
            handle_section_navigation_sound(&gCapStateIndex, 0, 3);
            break;
        case 2:
            section_navigation(&gEyeStateIndex, 4, true, true);
            handle_section_navigation_sound(&gEyeStateIndex, 0, 4);
            break;
        case 3:
            section_navigation(&gPowerupIndex, 3, true, true);
            handle_section_navigation_sound(&gPowerupIndex, 0, 3);
            break;
        case 4:
            section_navigation(&gShadowVisIndex, 2, false, false);
            handle_section_navigation_sound(&gShadowVisIndex, 0, 2);
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    set_option_alpha(0);

    print_generic_string(centeredX(PAGE_X, textBodyOptions), PAGE_Y, textBodyOptions);

    set_option_alpha(1);
    print_generic_string(OPTION_X, OPTION_1_Y, textCapState);

    switch (gCapStateIndex) {
        case 0:
            textCapStateIndex = (u8 *)&textOn;
            marioState->marioBodyState->capState = MARIO_HAS_DEFAULT_CAP_ON;
            break;
        case 1:
            textCapStateIndex = (u8 *)&textOff;
            marioState->marioBodyState->capState = MARIO_HAS_DEFAULT_CAP_OFF;
            break;
        case 2:
            textCapStateIndex = (u8 *)&textWingCap;
            marioState->marioBodyState->capState = MARIO_HAS_WING_CAP_ON;
            break;
    }
    print_generic_string(centeredX(LARGE_X, textCapStateIndex), OPTION_1_Y, textCapStateIndex);

    set_option_alpha(2);
    print_generic_string(OPTION_X, OPTION_2_Y, textEyeState);

    switch (gEyeStateIndex) {
        case 0:
            textEyeStateIndex = (u8 *)&textOpen;
            marioState->marioBodyState->eyeState = MARIO_EYES_OPEN;
            break;
        case 1:
            textEyeStateIndex = (u8 *)&textEyeHalfClosed;
            marioState->marioBodyState->eyeState = MARIO_EYES_HALF_CLOSED;
            break;
        case 2:
            textEyeStateIndex = (u8 *)&textEyeClosed;
            marioState->marioBodyState->eyeState = MARIO_EYES_CLOSED;
            break;
        case 3:
            textEyeStateIndex = (u8 *)&textEyeDead;
            marioState->marioBodyState->eyeState = MARIO_EYES_DEAD;
            break;
    }
    print_generic_string(centeredX(LARGE_X, textEyeStateIndex), OPTION_2_Y, textEyeStateIndex);

    set_option_alpha(3);
    print_generic_string(OPTION_X, OPTION_3_Y, textPowerup);

    switch (gPowerupIndex) {
        case 0:
            textPowerupIndex = (u8 *)&textNone;
            marioState->marioBodyState->modelState = 0;
            break;
        case 1:
            textPowerupIndex = (u8 *)&textPowerupVanish;
            marioState->marioBodyState->modelState = MODEL_STATE_NOISE_ALPHA;
            break;
        case 2:
            textPowerupIndex = (u8 *)&textPowerupMetal;
            marioState->marioBodyState->modelState = MODEL_STATE_METAL;
            break;
    }
    print_generic_string(centeredX(LARGE_X, textPowerupIndex), OPTION_3_Y, textPowerupIndex);

    set_option_alpha(4);
    print_generic_string(OPTION_X, OPTION_4_Y, textShadowVisibility);

    switch (gShadowVisIndex) {
        case 0:
            print_generic_string(centeredX(SMALL_X, textOn), OPTION_4_Y, textOn);
            isPlayerShadowVisible = true;
            marioState->marioObj->header.gfx.shadowInvisible = FALSE;
            break;
        case 1:
            print_generic_string(centeredX(SMALL_X, textOff), OPTION_4_Y, textOff);
            isPlayerShadowVisible = false;
            marioState->marioObj->header.gfx.shadowInvisible = TRUE;
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    draw_animated_option_arrows(OPTION_ARROW_X, OPTION_1_Y, 85, 1);
    draw_animated_option_arrows(OPTION_ARROW_X, OPTION_2_Y, 85, 2);
    draw_animated_option_arrows(OPTION_ARROW_X, OPTION_3_Y, 85, 3);
    draw_animated_option_arrows(OPTION_ARROW_X + 40, OPTION_4_Y, 45, 6);
}

static s16 gInitialHeadRotationX;
static int gIsInitialHeadRotationXSaved = false;

static s16 gInitialHeadRotationY;
static int gIsInitialHeadRotationYSaved = false;

static s16 gInitialHeadRotationZ;
static int gIsInitialHeadRotationZSaved = false;

void save_initial_head_rotation() {
    if (!gIsInitialHeadRotationXSaved) {
        gInitialHeadRotationX = gPlayerCameraState->headRotation[1];
        gHeadRotationX = 180;
        gIsInitialHeadRotationXSaved = true;
    }

    if (!gIsInitialHeadRotationYSaved) {
        gInitialHeadRotationY = gPlayerCameraState->headRotation[0];
        gHeadRotationY = 180;
        gIsInitialHeadRotationYSaved = true;
    }

    if (!gIsInitialHeadRotationZSaved) {
        gInitialHeadRotationZ = gPlayerCameraState->headRotation[2];
        gHeadRotationZ = 180;
        gIsInitialHeadRotationZSaved = true;
    }
}

void restore_initial_head_rotation() {
    if (gIsInitialHeadRotationXSaved) {
        gPlayerCameraState->headRotation[1] = gInitialHeadRotationX;
        gHeadRotationX = 180;
    }

    if (gIsInitialHeadRotationYSaved) {
        gPlayerCameraState->headRotation[0] = gInitialHeadRotationY;
        gHeadRotationY = 180;
    }

    if (gIsInitialHeadRotationZSaved) {
        gPlayerCameraState->headRotation[2] = gInitialHeadRotationZ;
        gHeadRotationZ = 180;
    }
}

void head_rotation_global_updater(s16 *axis, s16 initialAxis, u8 rotationAxis) {
    // Increment rotation when right D-pad is pressed
    if (gPlayer1Controller->buttonPressed & R_JPAD) {
        *axis = (*axis - UPDATE_INCREMENT + 360) % 360;
        rightButtonHoldCounter = 0; // Reset hold counter
    }

    // Decrement rotation when left D-pad is pressed
    if (gPlayer1Controller->buttonPressed & L_JPAD) {
        *axis = (*axis + UPDATE_INCREMENT) % 360;
        leftButtonHoldCounter = 0; // Reset hold counter
    }

    // Handle continuous rotation increase when right D-pad is held
    if (gPlayer1Controller->buttonDown & R_JPAD) {
        rightButtonHoldCounter++;
        s16 decrement = (rightButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (rightButtonHoldCounter >= HOLD_THRESHOLD && rightButtonHoldCounter % HOLD_DELAY == 0) {
            *axis = (*axis - decrement + 360) % 360;
        }
    } else {
        rightButtonHoldCounter = 0;
    }

    // Handle continuous rotation decrease when left D-pad is held
    if (gPlayer1Controller->buttonDown & L_JPAD) {
        leftButtonHoldCounter++;
        s16 increment = (leftButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (leftButtonHoldCounter >= HOLD_THRESHOLD && leftButtonHoldCounter % HOLD_DELAY == 0) {
            *axis = (*axis + increment) % 360;
        }
    } else {
        leftButtonHoldCounter = 0;
    }

    // Update Mario's head rotation based on the adjusted rotation amount
    // UI uses 0..359 degrees, with 180 as neutral.
    const s16 deltaDeg = (s16)(*axis - 180);
    const s16 deltaAng = (s16)(deltaDeg * (0x10000 / 360));
    const s16 angle = (s16)(initialAxis + deltaAng);
    gPlayerCameraState->headRotation[rotationAxis] = angle;
    marioState->marioBodyState->headAngle[rotationAxis] = angle;
}

static s32 gInitialTorsoRotationX;
static int gIsInitialTorsoRotationXSaved = false;

static s32 gInitialTorsoRotationY;
static int gIsInitialTorsoRotationYSaved = false;

static s32 gInitialTorsoRotationZ;
static int gIsInitialTorsoRotationZSaved = false;

void save_initial_torso_rotation() {
    if (!gIsInitialTorsoRotationXSaved) {
        gInitialTorsoRotationX = marioState->marioBodyState->torsoAngle[1];
        gTorsoRotationX = 180;
        gIsInitialTorsoRotationXSaved = true;
    }

    if (!gIsInitialTorsoRotationYSaved) {
        gInitialTorsoRotationY = marioState->marioBodyState->torsoAngle[2];
        gTorsoRotationY = 180;
        gIsInitialTorsoRotationYSaved = true;
    }

    if (!gIsInitialTorsoRotationZSaved) {
        gInitialTorsoRotationZ = marioState->marioBodyState->torsoAngle[0];
        gTorsoRotationZ = 180;
        gIsInitialTorsoRotationZSaved = true;
    }
}

void restore_initial_torso_rotation() {
    if (gIsInitialTorsoRotationXSaved) {
        marioState->marioBodyState->torsoAngle[1] = gInitialTorsoRotationX;
        gTorsoRotationX = 180;
    }

    if (gIsInitialTorsoRotationYSaved) {
        marioState->marioBodyState->torsoAngle[2] = gInitialTorsoRotationY;
        gTorsoRotationY = 180;
    }

    if (gIsInitialTorsoRotationZSaved) {
        marioState->marioBodyState->torsoAngle[0] = gInitialTorsoRotationZ;
        gTorsoRotationZ = 180;
    }
}

void torso_rotation_global_updater(s16 *axis, s16 initialAxis, u8 rotationAxis) {
    // Increment rotation when right D-pad is pressed
    if (gPlayer1Controller->buttonPressed & R_JPAD) {
        *axis = (*axis - UPDATE_INCREMENT + 360) % 360;
        rightButtonHoldCounter = 0; // Reset hold counter
    }

    // Decrement rotation when left D-pad is pressed
    if (gPlayer1Controller->buttonPressed & L_JPAD) {
        *axis = (*axis + UPDATE_INCREMENT) % 360;
        leftButtonHoldCounter = 0; // Reset hold counter
    }

    // Handle continuous rotation increase when right D-pad is held
    if (gPlayer1Controller->buttonDown & R_JPAD) {
        rightButtonHoldCounter++;
        s16 decrement = (rightButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (rightButtonHoldCounter >= HOLD_THRESHOLD && rightButtonHoldCounter % HOLD_DELAY == 0) {
            *axis = (*axis - decrement + 360) % 360;
        }
    } else {
        rightButtonHoldCounter = 0;
    }

    // Handle continuous rotation decrease when left D-pad is held
    if (gPlayer1Controller->buttonDown & L_JPAD) {
        leftButtonHoldCounter++;
        s16 increment = (leftButtonHoldCounter >= MULTIPLIER_THRESHOLD) ? UPDATE_INCREMENT * 3 : UPDATE_INCREMENT;

        if (leftButtonHoldCounter >= HOLD_THRESHOLD && leftButtonHoldCounter % HOLD_DELAY == 0) {
            *axis = (*axis + increment) % 360;
        }
    } else {
        leftButtonHoldCounter = 0;
    }

    // Update Mario's torso rotation based on the adjusted rotation amount
    // UI uses 0..359 degrees, with 180 as neutral.
    const s16 deltaDeg = (s16)(*axis - 180);
    const s16 deltaAng = (s16)(deltaDeg * (0x10000 / 360));
    marioState->marioBodyState->torsoAngle[rotationAxis] = (s16)(initialAxis + deltaAng);
}

void body_rotation(void) {
    CREATE_STRING(textBodyRotation, "Body Rotation");
    CREATE_STRING(textHeadRotationX, "Head X");
    CREATE_STRING(textHeadRotationY, "Head Y");
    CREATE_STRING(textHeadRotationZ, "Head Z");
    CREATE_STRING(textTorsoRotationX, "Torso X");
    CREATE_STRING(textTorsoRotationY, "Torso Y");
    CREATE_STRING(textTorsoRotationZ, "Torso Z");

    switch (gCurrentMenuSection) {
        case 1:
            head_rotation_global_updater(&gHeadRotationX, gInitialHeadRotationX, 1);
            handle_section_navigation_sound(&gHeadRotationX, 0, 360);
            break;
        case 2:
            head_rotation_global_updater(&gHeadRotationY, gInitialHeadRotationY, 0);
            handle_section_navigation_sound(&gHeadRotationY, 0, 360);
            break;
        case 3:
            head_rotation_global_updater(&gHeadRotationZ, gInitialHeadRotationZ, 2);
            handle_section_navigation_sound(&gHeadRotationZ, 0, 360);
            break;
        case 4:
            torso_rotation_global_updater(&gTorsoRotationX, gInitialTorsoRotationX, 1);
            handle_section_navigation_sound(&gTorsoRotationX, 0, 360);
            break;
        case 5:
            torso_rotation_global_updater(&gTorsoRotationY, gInitialTorsoRotationY, 2);
            handle_section_navigation_sound(&gTorsoRotationY, 0, 360);
            break;
        case 6:
            torso_rotation_global_updater(&gTorsoRotationZ, gInitialTorsoRotationZ, 0);
            handle_section_navigation_sound(&gTorsoRotationZ, 0, 360);
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    set_option_alpha(0);

    print_generic_string(centeredX(PAGE_X, textBodyRotation), PAGE_Y, textBodyRotation);

    set_option_alpha(1);
    print_generic_string(OPTION_X, OPTION_1_Y, textHeadRotationX);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_1_Y, -gHeadRotationX);

    set_option_alpha(2);
    print_generic_string(OPTION_X, OPTION_2_Y, textHeadRotationY);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_2_Y, -gHeadRotationY);

    set_option_alpha(3);
    print_generic_string(OPTION_X, OPTION_3_Y, textHeadRotationZ);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_3_Y, -gHeadRotationZ);

    set_option_alpha(4);
    print_generic_string(OPTION_X, OPTION_4_Y, textTorsoRotationX);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_4_Y, -gTorsoRotationX);

    set_option_alpha(5);
    print_generic_string(OPTION_X, OPTION_5_Y, textTorsoRotationY);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_5_Y, -gTorsoRotationY);

    set_option_alpha(6);
    print_generic_string(OPTION_X, OPTION_6_Y, textTorsoRotationZ);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_6_Y, -gTorsoRotationZ);
    
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    draw_option_bar(OPTION_1_Y, 359, (360 - gHeadRotationX) % 360, 1);
    draw_option_bar(OPTION_2_Y, 359, (360 - gHeadRotationY) % 360, 2);
    draw_option_bar(OPTION_3_Y, 359, (360 - gHeadRotationZ) % 360, 3);
    draw_option_bar(OPTION_4_Y, 359, (360 - gTorsoRotationX) % 360, 4);
    draw_option_bar(OPTION_5_Y, 359, (360 - gTorsoRotationY) % 360, 5);
    draw_option_bar(OPTION_6_Y, 359, (360 - gTorsoRotationZ) % 360, 6);
}

void camera_options(void) {
    CREATE_STRING(textCameraOptions, "Camera Options");
    CREATE_STRING(textFov, "FOV");
    CREATE_STRING(textPrecision, "Precision");
    CREATE_STRING(textFrame, "Frame");
    CREATE_STRING(textCinematic, "Cinematic");
    CREATE_STRING(text4By3, "4:3");
    CREATE_STRING(textOn, "On");
    CREATE_STRING(textOff, "Off");
    CREATE_STRING(textNone, "None");

    switch (gCurrentMenuSection) {
        case 1:
            fov_updater();
            handle_section_navigation_sound(&gFOVValue, 10, 90);
            break;
        case 2:
            section_navigation(&gPrecisionIndex, 2, false, false);
            handle_section_navigation_sound(&gPrecisionIndex, 0, 2);
            break;
        case 3:
            section_navigation(&gFrameIndex, 3, true, false);
            handle_section_navigation_sound(&gFrameIndex, 0, 3);
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    set_option_alpha(0);

    print_generic_string(centeredX(PAGE_X, textCameraOptions), PAGE_Y, textCameraOptions);

    set_option_alpha(2);
    print_generic_string(OPTION_X, OPTION_2_Y, textPrecision);

    switch (gPrecisionIndex) {
        case 0:
            print_generic_string(centeredX(LARGE_X, textOff), OPTION_2_Y, textOff);
            gPrecisionOn = false;
            break;
        case 1:
            print_generic_string(centeredX(LARGE_X, textOn), OPTION_2_Y, textOn);
            gPrecisionOn = true;
            break;
    }

    set_option_alpha(3);
    print_generic_string(OPTION_X, OPTION_3_Y, textFrame);

    switch (gFrameIndex) {
        case 0:
            print_generic_string(centeredX(LARGE_X, textNone), OPTION_3_Y, textNone);
            break;
        case 1:
            print_generic_string(centeredX(LARGE_X, textCinematic), OPTION_3_Y, textCinematic);
            break;
        case 2:
            print_generic_string(centeredX(LARGE_X, text4By3), OPTION_3_Y, text4By3);
            break;
    }

    set_option_alpha(1);
    print_generic_string(OPTION_X, OPTION_1_Y, textFov);
    print_number_value(OPTION_BAR_VALUE_X, OPTION_1_Y, gFOVValue);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    draw_option_bar(OPTION_1_Y, 90, gFOVValue, 1);
    draw_animated_option_arrows(OPTION_ARROW_X, OPTION_2_Y, 85, 2);
    draw_animated_option_arrows(OPTION_ARROW_X, OPTION_3_Y, 85, 3);
}

void world_options(void) {
    CREATE_STRING(textWorldOptions, "World Options");
    CREATE_STRING(textCoinVisibility, "Coin Visibility");
    CREATE_STRING(textEnemyVisibility, "Enemy Visibility");
    CREATE_STRING(textFriendVisibility, "Friend Visibility");
    CREATE_STRING(textEffectVisibility, "Effect Visibility");
    CREATE_STRING(textPlayerVisibility, "Player Visibility");
    CREATE_STRING(textOn, "On");
    CREATE_STRING(textOff, "Off");
    CREATE_STRING(textNone, "None");

    switch (gCurrentMenuSection) {
        case 1:
            section_navigation(&gCoinVisibilityIndex, 2, false, false);
            handle_section_navigation_sound(&gCoinVisibilityIndex, 0, 2);
            break;
        case 2:
            section_navigation(&gEnemyVisibilityIndex, 2, false, false);
            handle_section_navigation_sound(&gEnemyVisibilityIndex, 0, 2);
            break;
        case 3:
            section_navigation(&gFriendVisibilityIndex, 2, false, false);
            handle_section_navigation_sound(&gFriendVisibilityIndex, 0, 2);
            break;
        case 4:
            section_navigation(&gEffectVisibilityIndex, 2, false, false);
            handle_section_navigation_sound(&gEffectVisibilityIndex, 0, 2);
            break;
        case 5:
            section_navigation(&gPlayerVisIndex, 2, false, false);
            handle_section_navigation_sound(&gPlayerVisIndex, 0, 2);
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    set_option_alpha(0);

    print_generic_string(centeredX(PAGE_X, textWorldOptions), PAGE_Y, textWorldOptions);

    set_option_alpha(1);
    print_generic_string(OPTION_X, OPTION_1_Y, textCoinVisibility);

    switch (gCoinVisibilityIndex) {
        case 0:
            print_generic_string(centeredX(SMALL_X, textOn), OPTION_1_Y, textOn);
            set_photo_mode_object_visibility(coinbehaviors, true);
            break;
        case 1:
            print_generic_string(centeredX(SMALL_X, textOff), OPTION_1_Y, textOff);
            set_photo_mode_object_visibility(coinbehaviors, false);
            break;
    }

    set_option_alpha(2);
    print_generic_string(OPTION_X, OPTION_2_Y, textEnemyVisibility);

    switch (gEnemyVisibilityIndex) {
        case 0:
            print_generic_string(centeredX(SMALL_X, textOn), OPTION_2_Y, textOn);
            set_photo_mode_object_visibility(enemyBehaviors, true);
            break;
        case 1:
            print_generic_string(centeredX(SMALL_X, textOff), OPTION_2_Y, textOff);
            set_photo_mode_object_visibility(enemyBehaviors, false);
            break;
    }

    set_option_alpha(3);
    print_generic_string(OPTION_X, OPTION_3_Y, textFriendVisibility);

    switch (gFriendVisibilityIndex) {
        case 0:
            print_generic_string(centeredX(SMALL_X, textOn), OPTION_3_Y, textOn);
            set_photo_mode_object_visibility(friendBehaviors, true);
            break;
        case 1:
            print_generic_string(centeredX(SMALL_X, textOff), OPTION_3_Y, textOff);
            set_photo_mode_object_visibility(friendBehaviors, false);
            break;
    }

    set_option_alpha(4);
    print_generic_string(OPTION_X, OPTION_4_Y, textEffectVisibility);

    switch (gEffectVisibilityIndex) {
        case 0:
            print_generic_string(centeredX(SMALL_X, textOn), OPTION_4_Y, textOn);
            set_photo_mode_object_visibility(effectBehaviors, true);
            break;
        case 1:
            print_generic_string(centeredX(SMALL_X, textOff), OPTION_4_Y, textOff);
            set_photo_mode_object_visibility(effectBehaviors, false);
            break;
    }

    set_option_alpha(5);
    print_generic_string(OPTION_X, OPTION_5_Y, textPlayerVisibility);

    switch (gPlayerVisIndex) {
        case 0:
            print_generic_string(centeredX(SMALL_X, textOn), OPTION_5_Y, textOn);
            marioState->marioObj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
            break;
        case 1:
            print_generic_string(centeredX(SMALL_X, textOff), OPTION_5_Y, textOff);
            marioState->marioObj->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
            break;
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    draw_animated_option_arrows(OPTION_ARROW_X + 40, OPTION_1_Y, 45, 1);
    draw_animated_option_arrows(OPTION_ARROW_X + 40, OPTION_2_Y, 45, 2);
    draw_animated_option_arrows(OPTION_ARROW_X + 40, OPTION_3_Y, 45, 3);
    draw_animated_option_arrows(OPTION_ARROW_X + 40, OPTION_4_Y, 45, 4);
    draw_animated_option_arrows(OPTION_ARROW_X + 40, OPTION_5_Y, 45, 5);
}

void manage_photo_mode_pages(void) {
    if (gCurrentMenuSection == 0) {
        section_navigation(&gPageIndex, 5, true, true);
        handle_section_navigation_sound(&gPageIndex, 0, 5);
    }

    // Vertical menu scrolling (up/down) for the current page
    if (gPageIndex >= 0 && gPageIndex < (s8)(sizeof(maxSections) / sizeof(maxSections[0]))) {
        s8 maxSection = maxSections[gPageIndex];
        s16 *currentSection = &gCurrentMenuSection;

        if (gPlayer1Controller->buttonPressed & D_JPAD) {
            if (*currentSection < maxSection) {
                (*currentSection)++;
            }
        }
        if (gPlayer1Controller->buttonPressed & U_JPAD) {
            if (*currentSection > 0) {
                (*currentSection)--;
            }
        }

        if (*currentSection != previousMenuSection) {
            play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundSource);
            previousMenuSection = *currentSection;  // Update the previous menu section
        }
    }

    // Print the appropriate text and set page based on the current index
    switch (gPageIndex) {
        case 0:
            pose_options();
            break;
        case 1:
            body_options();
            break;
        case 2:
            body_rotation();
            break;
        case 3:
            camera_options();
            break;
        case 4:
            world_options();
            break;
    }

    if (photoModeClosed) {
        gPageIndex = 0;
        gCurrentMenuSection = 0;
    }
}

void close_photo_mode(void) {
    restore_initial_cap_state();
    gIsInitialCapStateSaved = FALSE;
    gCapStateIndex = initialCapStateIndex;

    restore_initial_hand_state();
    gIsInitialHandStateSaved = FALSE;
    gHandStateIndex = initialHandStateIndex;

    restore_initial_eye_state();
    gIsInitialEyeStateSaved = FALSE;
    gEyeStateIndex = initialEyeStateIndex;

    restore_initial_powerup();
    gIsInitialPowerupSaved = FALSE;
    gPowerupIndex = initialPowerupIndex;

    restore_initial_pose();
    gIsInitialPoseSaved = FALSE;
    gGroundPoseIndex = 0;
    gAirPoseIndex = 0;

    restore_initial_rotation();
    gIsInitialRotationSaved = FALSE;
    gRotationAmount = gInitialRotation;

    restore_initial_position();
    gIsInitialXPositionSaved = FALSE;
    gXPosition = gInitialXPosition;
    gIsInitialYPositionSaved = FALSE;
    gYPosition = gInitialYPosition;
    gIsInitialZPositionSaved = FALSE;
    gZPosition = gInitialZPosition;

    gFOVValue = sSavedFovValue;
    gFOVState.fov = sSavedFovValue;

    gFrameIndex = 0;
    configForce4By3 = sSavedForce4By3;
    configForce21By9 = sSavedForce21By9;

    gPlayerVisIndex = 0;
    marioState->marioObj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;

    gShadowVisIndex = 0;
    isPlayerShadowVisible = TRUE;
    marioState->marioObj->header.gfx.shadowInvisible = FALSE;

    gCoinVisibilityIndex = 0;
    set_photo_mode_object_visibility(coinbehaviors, TRUE);

    gEnemyVisibilityIndex = 0;
    set_photo_mode_object_visibility(enemyBehaviors, TRUE);

    gFriendVisibilityIndex = 0;
    set_photo_mode_object_visibility(friendBehaviors, TRUE);

    gEffectVisibilityIndex = 0;
    set_photo_mode_object_visibility(effectBehaviors, TRUE);

    gPrecisionIndex = 0;
    gPrecisionOn = FALSE;

    restore_initial_head_rotation();
    gIsInitialHeadRotationXSaved = FALSE;
    gIsInitialHeadRotationYSaved = FALSE;
    gIsInitialHeadRotationZSaved = FALSE;
    gHeadRotationX = gInitialHeadRotationX;
    gHeadRotationY = gInitialHeadRotationY;
    gHeadRotationZ = gInitialHeadRotationZ;

    restore_initial_torso_rotation();
    gIsInitialTorsoRotationXSaved = FALSE;
    gIsInitialTorsoRotationYSaved = FALSE;
    gIsInitialTorsoRotationZSaved = FALSE;
    gTorsoRotationX = gInitialTorsoRotationX;
    gTorsoRotationY = gInitialTorsoRotationY;
    gTorsoRotationZ = gInitialTorsoRotationZ;

    marioState->marioBodyState->torsoAngle[1] = gInitialTorsoRotationX;
    marioState->marioBodyState->torsoAngle[2] = gInitialTorsoRotationY;
    marioState->marioBodyState->torsoAngle[0] = gInitialTorsoRotationZ;

    marioState->marioBodyState->allowPartRotation = sSavedAllowPartRotation;
    vec3s_copy(marioState->marioBodyState->headAngle, sSavedHeadAngle);

    photoModeClosed = true;
    switch_to_photo_mode();

    extern s16 gPauseScreenMode;
    gPauseScreenMode = 0;
    gMenuMode = 0;
    set_play_mode(PLAY_MODE_PAUSED);
    play_sound(SOUND_MENU_MESSAGE_DISAPPEAR, gGlobalSoundSource);
}

void show_photo_mode_options_box(void) {
    const s16 boxCenterX = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(153);
    const s16 boxCenterY = (s16)(SCREEN_HEIGHT - 120);
    const s16 uiOffsetX = (s16)(boxCenterX - 220);
    const s16 uiOffsetY = (s16)(boxCenterY - 120);

    create_dl_translation_matrix(MENU_MTX_PUSH, boxCenterX, boxCenterY, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.3f, 1.8f, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 110);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    create_dl_translation_matrix(MENU_MTX_PUSH, uiOffsetX, uiOffsetY, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 0.8f, 0.8f, 1.0f);
    manage_photo_mode_pages();
    draw_animated_option_arrows(PAGE_X - 45, PAGE_Y, 90, 0);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

void hide_photo_mode_options_box(void) {
    return;
}

static int isPhotoModeOptionsBoxVisible = true;

s32 activate_photo_mode(void) {
    marioState = gMarioState;
    gFOVState.fov = gFOVValue;
    create_dl_ortho_matrix();

    photo_mode_apply_frame_override();
    if (gPlayer1Controller->buttonPressed & START_BUTTON) {
        isPhotoModeOptionsBoxVisible = !isPhotoModeOptionsBoxVisible;

        if (isPhotoModeOptionsBoxVisible) {
            play_sound(SOUND_MENU_MESSAGE_APPEAR, gGlobalSoundSource);
        } else {
            play_sound(SOUND_MENU_MESSAGE_DISAPPEAR, gGlobalSoundSource);
        }
    }

    if (gPlayer1Controller->buttonPressed & B_BUTTON) {
        close_photo_mode();
    }
    
    if (isPhotoModeOptionsBoxVisible) {
        show_photo_mode_options_box();
    } else {
        hide_photo_mode_options_box();
    }

    photoModeClosed = false;

    return 0;
}

void open_photo_mode(void) {
    isPhotoModeOptionsBoxVisible = true;

    sSavedForce4By3 = configForce4By3;
    sSavedForce21By9 = configForce21By9;

    sSavedFovValue = gFOVState.fov;
    gFOVValue = gFOVState.fov;

    save_initial_cap_state();
    save_initial_hand_state();
    save_initial_eye_state();
    save_initial_powerup();
    save_initial_pose();
    save_initial_rotation();
    save_initial_position();
    save_initial_head_rotation();
    save_initial_torso_rotation();

    sSavedAllowPartRotation = marioState->marioBodyState->allowPartRotation;
    vec3s_copy(sSavedHeadAngle, marioState->marioBodyState->headAngle);
    marioState->marioBodyState->allowPartRotation = TRUE;
}