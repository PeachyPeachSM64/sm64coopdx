#include "mario_cheats.h"

#include <stdlib.h>
#include <string.h>

#include "sm64.h"
#include "audio/external.h"
#include "behavior_data.h"
#include "engine/math_util.h"
#include "game_init.h"
#include "gfx_dimensions.h"
#include "interaction.h"
#include "level_update.h"
#include "mario.h"
#include "object_helpers.h"
#include "object_list_processor.h"
#include "print.h"
#include "save_file.h"
#include "pc/cheats.h"
#include "pc/controller/controller_api.h"

static const s32 sModifierValues[] = { 1, 2, 3, 4, 5 };
static const f32 sSizeValues[] = { 1.f, 2.f, 3.f, 4.f, 0.f, 0.25f, 0.5f, 0.75f };

u32 gKeyPressed = VK_INVALID;

void cheats_update(struct MarioState* m) {
    (void)m;
    gKeyPressed = controller_get_cached_raw_key();
}

s32 cheats_moon_jump(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.MoonJump) {
        if (m->controller && (m->controller->buttonDown & L_TRIG)) {
            m->vel[1] = 25.0f;
            return TRUE;
        }
    }
    return FALSE;
}

s32 cheats_moon_gravity(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.MoonGravity) {
        if ((m->action & ACT_GROUP_MASK) == ACT_GROUP_AIRBORNE) {
            if (m->action == ACT_FREEFALL || m->action == ACT_LONG_JUMP) {
                m->vel[1] += 1.0f;
            } else {
                m->vel[1] += 2.0f;
            }
            return TRUE;
        }
    }
    return FALSE;
}

s32 cheats_super_copter(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.SuperCopter) {
        if (m->controller && (m->controller->buttonDown & A_BUTTON)) {
            m->vel[1] = 30.0f;
            set_mario_action(m, ACT_TWIRLING, 0);
            return TRUE;
        }

        if (m->controller && m->action == ACT_TWIRLING && (m->controller->buttonPressed & Z_TRIG)) {
            set_mario_action(m, ACT_GROUND_POUND, 0);
        }
    }
    return FALSE;
}

s32 cheats_debug_move(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.DebugMove) {
        set_mario_action(m, ACT_DEBUG_FREE_MOVE, 0);
        Cheats.DebugMove = false;
        return TRUE;
    }
    return FALSE;
}

s32 cheats_god_mode(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.GodMode) {
        m->health = 0x880;
        m->healCounter = 0;
        m->hurtCounter = 0;
        return TRUE;
    }
    return FALSE;
}

s32 cheats_infinite_lives(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.InfiniteLives) {
        m->numLives = 99;
    }
    return FALSE;
}

s32 cheats_hurt_mario(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (!Cheats.EnableCheats) { return FALSE; }
    if (!Cheats.HurtMario) { return FALSE; }
    if (!m->controller) { return FALSE; }

    if ((m->controller->buttonDown & L_TRIG) && (m->controller->buttonPressed & A_BUTTON)) {
        switch (Cheats.HurtMario) {
            case 1: {
                spawn_object(m->marioObj, MODEL_EXPLOSION, bhvExplosion);
                play_sound(SOUND_GENERAL2_BOBOMB_EXPLOSION | 0xFF00, m->marioObj->header.gfx.cameraToObject);
                if (m->action & ACT_FLAG_AIR) {
                    drop_and_set_mario_action(m, (random_u16() & 1) ? ACT_HARD_FORWARD_AIR_KB : ACT_HARD_BACKWARD_AIR_KB, 1);
                } else if (m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER)) {
                    drop_and_set_mario_action(m, (random_u16() & 1) ? ACT_FORWARD_WATER_KB : ACT_BACKWARD_WATER_KB, 1);
                } else {
                    drop_and_set_mario_action(m, (random_u16() & 1) ? ACT_HARD_FORWARD_GROUND_KB : ACT_HARD_BACKWARD_GROUND_KB, 1);
                }
            } break;

            case 2: {
                if (m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER)) {
                    drop_and_set_mario_action(m, ACT_WATER_SHOCKED, 0);
                } else {
                    u32 actionArg = (m->action & (ACT_FLAG_AIR | ACT_FLAG_ON_POLE | ACT_FLAG_HANGING)) == 0;
                    drop_and_set_mario_action(m, ACT_SHOCKED, actionArg);
                }
            } break;

            case 3: {
                if (!(m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER))) {
                    m->marioObj->oMarioBurnTimer = 0;
                    update_mario_sound_and_camera(m);
                    if ((m->action & ACT_FLAG_AIR) && m->vel[1] <= 0.0f) {
                        drop_and_set_mario_action(m, ACT_BURNING_FALL, 1);
                    } else {
                        drop_and_set_mario_action(m, ACT_BURNING_JUMP, 1);
                    }
                }
            } break;

            case 4: {
                if (!(m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER))) {
                    drop_and_set_mario_action(m, ACT_LAVA_BOOST, 0);
                }
            } break;

            case 5: {
                drop_and_set_mario_action(m, ACT_SQUISHED, 0);
                vec3f_set(m->marioObj->header.gfx.scale, 1.8f, 0.05f, 1.8f);
                m->particleFlags |= PARTICLE_MIST_CIRCLE;
                m->squishTimer = 0xFF;
                m->actionState = 1;
            } break;

            case 6: {
                switch (random_u16() % 3) {
                    case 0: drop_and_set_mario_action(m, ACT_HEAD_STUCK_IN_GROUND, 0); break;
                    case 1: drop_and_set_mario_action(m, ACT_BUTT_STUCK_IN_GROUND, 0); break;
                    case 2: drop_and_set_mario_action(m, ACT_FEET_STUCK_IN_GROUND, 0); break;
                }
                m->particleFlags |= PARTICLE_MIST_CIRCLE;
            } break;

            case 7: {
                m->health = 0x180;
                m->healCounter = 0;
                m->hurtCounter = 0;
            } break;
        }
    }
    return TRUE;
}

s32 cheats_blj_anywhere(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (!Cheats.EnableCheats) { return FALSE; }
    if (!Cheats.BLJAnywhere) { return FALSE; }
    if (!m->controller) { return FALSE; }

    if (m->forwardVel < 1.0f) {
        if ((m->action == ACT_LONG_JUMP_LAND || m->action == ACT_LONG_JUMP_LAND_STOP) && Cheats.BLJAnywhere >= 7 && (m->controller->buttonDown & A_BUTTON)) {
            set_jumping_action(m, ACT_LONG_JUMP, 0);
        } else if (m->action == ACT_LONG_JUMP && m->pos[1] - 50.0f < m->floorHeight) {
            if (Cheats.BLJAnywhere < 7) {
                if (m->input & INPUT_A_PRESSED) {
                    m->forwardVel -= (Cheats.BLJAnywhere - 1) * 2.5f;
                    m->vel[1] = -50.0f;
                }
            } else if (m->controller->buttonDown & A_BUTTON) {
                m->forwardVel -= (Cheats.BLJAnywhere - 7) * 2.5f;
                m->vel[1] = -50.0f;
            }
        }
        return TRUE;
    }
    return FALSE;
}

s32 cheats_swim_anywhere(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.SwimAnywhere) {
        m->waterLevel = m->pos[1] + 300.0f;
        return TRUE;
    }
    return FALSE;
}

s32 cheats_no_hold_heavy(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.NoHoldHeavy) {
        if (m->action == ACT_HOLD_HEAVY_IDLE) {
            set_mario_action(m, ACT_HOLD_IDLE, 0);
        } else if (m->action == ACT_HOLD_HEAVY_WALKING) {
            set_mario_action(m, ACT_HOLD_WALKING, 0);
        }
        return TRUE;
    }
    return FALSE;
}

s32 cheats_auto_wall_kick(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.AutoWallKick) {
        if (m->action == ACT_AIR_HIT_WALL) {
            m->vel[1] = 52.0f;
            m->faceAngle[1] += 0x8000;
            m->wallKickTimer = 0;
            set_mario_action(m, ACT_WALL_KICK_AIR, 0);
            set_mario_animation(m, MARIO_ANIM_START_WALLKICK);
            return TRUE;
        }
    }
    return FALSE;
}

static void mario_attract_nearby_coins(struct MarioState* m, f32 range) {
    if (!m) { return; }
    if (gTimeStopState & TIME_STOP_ENABLED) { return; }

    static const s32 sCoinLists[] = { OBJ_LIST_GENACTOR, OBJ_LIST_LEVEL, -1 };
    for (const s32* list = sCoinLists; *list != -1; list++) {
        struct Object* head = (struct Object*)&gObjectLists[*list];
        for (struct Object* obj = (struct Object*)head->header.next; obj != head; obj = (struct Object*)obj->header.next) {
            if (obj->oIntangibleTimer == 0 && obj->oInteractType == INTERACT_COIN) {
                Vec3f dv = {
                    obj->oPosX - m->pos[0],
                    obj->oPosY - m->pos[1] - 60.0f,
                    obj->oPosZ - m->pos[2],
                };
                f32 distToObj = vec3f_length(dv);
                if (distToObj > 0.0f && distToObj < range) {
                    vec3f_normalize(dv);
                    obj->oPosX -= dv[0] * 40.0f;
                    obj->oPosY -= dv[1] * 40.0f;
                    obj->oPosZ -= dv[2] * 40.0f;
                }
            }
        }
    }
}

s32 cheats_coins_magnet(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.CoinsMagnet) {
        if (!Cheats.TimeStop) {
            mario_attract_nearby_coins(m, 1000.0f);
        }
        return TRUE;
    }
    return FALSE;
}

s32 cheats_time_stop(struct MarioState* m) {
    (void)m;
    static u32 sPrevTimeStopState = 0;
    static bool sPrevTimeStopEnabled = false;

    if (gKeyPressed != VK_INVALID && (
        gKeyPressed == CheatsControls.TimeStopButton[0] ||
        gKeyPressed == CheatsControls.TimeStopButton[1] ||
        gKeyPressed == CheatsControls.TimeStopButton[2])) {
        Cheats.TimeStop = !Cheats.TimeStop;
    }

    if (Cheats.EnableCheats && Cheats.TimeStop) {
        if (!sPrevTimeStopEnabled) {
            sPrevTimeStopEnabled = true;
            sPrevTimeStopState = gTimeStopState;
        }
        gTimeStopState |= TIME_STOP_ENABLED;
        return TRUE;
    }

    if (sPrevTimeStopEnabled) {
        sPrevTimeStopEnabled = false;
        gTimeStopState = sPrevTimeStopState;
        sPrevTimeStopState = 0;
    }
    return FALSE;
}

s32 cheats_quick_ending(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.QuickEnding) {
        if (m->controller) {
            m->controller->stickX = 0;
            m->controller->stickY = 0;
            m->controller->stickMag = 0;
        }
        level_trigger_warp(m, WARP_OP_CREDITS_START);
        Cheats.QuickEnding = false;
        save_file_do_save(gCurrSaveFileNum - 1, FALSE);
        return TRUE;
    }
    return FALSE;
}

s32 cheats_water_control(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (!Cheats.EnableCheats) { return FALSE; }

    static s16* sDefaultEnv = NULL;
    static s32 sDefaultEnvLen = 0;
    static s16 sDefaultLevelNum = -1;
    static s8 sDefaultAreaIndex = -1;
    static u32 sPrevCheatWaterLevel = 0;

    if (gEnvironmentRegions == NULL || gEnvironmentRegionsLength <= 0) {
        return FALSE;
    }

    if (sDefaultLevelNum != gCurrLevelNum || sDefaultAreaIndex != gCurrAreaIndex || sDefaultEnvLen != gEnvironmentRegionsLength) {
        free(sDefaultEnv);
        sDefaultEnvLen = gEnvironmentRegionsLength;
        sDefaultEnv = calloc((size_t)sDefaultEnvLen, sizeof(s16));
        if (sDefaultEnv != NULL) {
            memcpy(sDefaultEnv, gEnvironmentRegions, (size_t)sDefaultEnvLen * sizeof(s16));
        }
        sDefaultLevelNum = gCurrLevelNum;
        sDefaultAreaIndex = gCurrAreaIndex;
        sPrevCheatWaterLevel = 0;
    }

    s16* p = gEnvironmentRegions;
    if (p != NULL && sDefaultEnv != NULL) {
        s32 n = p[0];
        for (s32 i = 0; i != n && i < 8; ++i) {
            s32 idx = 1 + (i * 6);
            if (idx + 5 >= gEnvironmentRegionsLength) { break; }
            s16* env = &p[idx];
            s16* def = &sDefaultEnv[idx];

            switch (Cheats.WaterLevel) {
                case 0: {
                    if (sPrevCheatWaterLevel != 0) {
                        memcpy(env, def, 6 * sizeof(s16));
                    } else {
                        memcpy(def, env, 6 * sizeof(s16));
                    }
                } break;

                case 1: {
                    if (env[0] < 50 && sPrevCheatWaterLevel != 1) {
                        set_mario_action(m, ACT_FREEFALL, 0);
                        m->waterLevel = -0x3FFF;
                        m->prevAction = m->action;
                    }
                    env[1] = -0x7FFF;
                    env[2] = -0x7FFF;
                    env[3] = +0x7FFF;
                    env[4] = +0x7FFF;
                    env[5] = -0x3FFF;
                } break;

                case 2: {
                    if (env[0] < 50 && sPrevCheatWaterLevel != 2) {
                        set_mario_action(m, ACT_WATER_IDLE, 0);
                        m->waterLevel = +0x3FFF;
                        m->prevAction = m->action;
                    }
                    env[1] = -0x7FFF;
                    env[2] = -0x7FFF;
                    env[3] = +0x7FFF;
                    env[4] = +0x7FFF;
                    env[5] = +0x3FFF;
                } break;
            }
        }
    }

    sPrevCheatWaterLevel = Cheats.WaterLevel;
    return FALSE;
}

s32 cheats_speed_display(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (Cheats.EnableCheats && Cheats.SpeedDisplay) {
        f32 hSpeed = m->forwardVel;
        f32 ySpeed = m->vel[1];
        if ((m->action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) {
            hSpeed *= (f32)sModifierValues[Cheats.SwimModifier];
            ySpeed *= (f32)sModifierValues[Cheats.SwimModifier];
        } else {
            hSpeed *= (f32)sModifierValues[Cheats.SpeedModifier];
            if (ySpeed > 0.0f) {
                ySpeed *= (f32)sModifierValues[Cheats.JumpModifier];
            }
        }
        print_text_fmt_int(GFX_DIMENSIONS_FROM_LEFT_EDGE(8), 44, "H SPD %d", (s32)hSpeed);
        print_text_fmt_int(GFX_DIMENSIONS_FROM_LEFT_EDGE(8), 28, "Y SPD %d", (s32)ySpeed);
        return TRUE;
    }
    return FALSE;
}

s32 cheats_size_modifier(struct MarioState* m) {
    if (!m) { return FALSE; }
    if (!m->marioObj) { return FALSE; }

    return FALSE;
}

static s32 cheats_speed_modifier(struct MarioState* m) {
    (void)m;
    if (Cheats.EnableCheats) {
        return sModifierValues[Cheats.SpeedModifier];
    }
    return 1;
}
