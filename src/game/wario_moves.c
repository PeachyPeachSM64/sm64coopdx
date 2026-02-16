#include "wario_moves.h"

#include "sm64.h"

#include "behavior_data.h"
#include "camera.h"
#include "audio/external.h"
#include "engine/math_util.h"
#include "interaction.h"
#include "mario.h"
#include "mario_actions_moving.h"
#include "mario_step.h"
#include "obj_behaviors.h"
#include "object_helpers.h"
#include "pc/lua/smlua.h"
#include "rumble_init.h"

#include "characters.h"

extern void update_air_without_turn(struct MarioState *m);
extern u32 common_air_action_step(struct MarioState *m, u32 landAction, s32 animation, u32 stepArg);
extern void obj_spawn_yellow_coins(struct Object *obj, s8 nCoins);
extern void animated_stationary_ground_step(struct MarioState *m, s32 animation, u32 endAction);

static bool sWarioWalkSpin = false;
static s32 sWarioSpinCount = 0;
static s32 sWarioChargeCount = 0;

static bool is_wario(struct MarioState *m) {
    if (m == NULL) { return false; }
    return (get_character(m)->type == CT_WARIO);
}

static const void *get_interact_behavior_script(struct MarioState *m) {
    if (m == NULL || m->interactObj == NULL) { return NULL; }
    return virtual_to_segmented(0x13, m->interactObj->behavior);
}

static bool wario_interact_is_behavior(struct MarioState *m, const BehaviorScript *behavior) {
    if (m == NULL || m->interactObj == NULL || behavior == NULL) { return false; }
    const void *script = get_interact_behavior_script(m);
    return (script == smlua_override_behavior(behavior));
}

s32 check_wario_pile_driver_jump_cancel(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (!is_wario(m)) { return FALSE; }
    if (!(m->input & INPUT_Z_PRESSED)) { return FALSE; }
    if (m->interactObj == NULL) { return FALSE; }

    if (wario_interact_is_behavior(m, bhvGoomba)
        || wario_interact_is_behavior(m, bhvBreakableBoxSmall)
        || wario_interact_is_behavior(m, bhvSmallPenguin)
        || wario_interact_is_behavior(m, bhvMacroUkiki)
        || wario_interact_is_behavior(m, bhvMips)) {
        return set_mario_action(m, ACT_WARIO_PILE_DRIVER, 0);
    }

    return FALSE;
}

s32 check_wario_spin_light_idle_cancel(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (!is_wario(m)) { return FALSE; }
    if (!(m->input & INPUT_Z_DOWN)) { return FALSE; }
    if (m->interactObj == NULL) { return FALSE; }

    if (wario_interact_is_behavior(m, bhvGoomba)) {
        return set_mario_action(m, ACT_PICKING_UP_ENEMIES, 0);
    }

    return FALSE;
}

s32 check_wario_spin_heavy_idle_cancel(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (!is_wario(m)) { return FALSE; }
    if (!(m->input & INPUT_Z_DOWN)) { return FALSE; }
    if (m->interactObj == NULL) { return FALSE; }

    if (wario_interact_is_behavior(m, bhvHeaveHo)
        || wario_interact_is_behavior(m, bhvChuckya)) {
        return set_mario_action(m, ACT_PICKING_UP_ENEMIES, 0);
    }

    return FALSE;
}

s32 act_wario_pile_driver(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (!is_wario(m)) { return set_mario_action(m, ACT_FREEFALL, 0); }

    u32 stepResult;
    f32 yOffset;

    m->twirlYaw = m->intendedYaw;
    m->angleVel[1] = 0x2000;
    m->faceAngle[1] += m->angleVel[1];
    queue_rumble_data_mario(m, 4, 20);
    play_sound(SOUND_OBJ_BOWSER_SPINNING, m->marioObj->header.gfx.cameraToObject);

    if (m->actionState == 0) {
        if (m->actionTimer < 10) {
            yOffset = 20 - 2 * m->actionTimer;
            if (m->pos[1] + yOffset + 160.0f < m->ceilHeight) {
                m->pos[1] += yOffset;
                m->peakHeight = m->pos[1];
                vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
            }
        }

        mario_set_forward_vel(m, 0.0f);

        set_character_animation(m, CHAR_ANIM_FORWARD_FLIP);
        if (m->actionTimer == 0) {
            play_sound(SOUND_ACTION_SPIN, m->marioObj->header.gfx.cameraToObject);
        }

        m->actionTimer++;
        if (is_anim_at_end(m)) {
            m->actionState = 1;
        }
    } else {
        set_character_animation(m, CHAR_ANIM_FORWARD_FLIP);

        stepResult = perform_air_step(m, 0);
        if (stepResult == AIR_STEP_LANDED) {
            play_mario_heavy_landing_sound(m, SOUND_ACTION_TERRAIN_HEAVY_LANDING);
            m->particleFlags |= PARTICLE_MIST_CIRCLE | PARTICLE_HORIZONTAL_STAR;
            set_camera_shake_from_hit(SHAKE_GROUND_POUND);
            set_mario_action(m, ACT_WARIO_PILE_DRIVER_LAND, 0);
        } else if (stepResult == AIR_STEP_HIT_WALL) {
            mario_set_forward_vel(m, -16.0f);
            if (m->vel[1] > 0.0f) {
                m->vel[1] = 0.0f;
            }
            set_mario_particle_flags(m, PARTICLE_VERTICAL_STAR, FALSE);
            set_mario_action(m, ACT_BACKWARD_AIR_KB, 0);
        }
    }

    return FALSE;
}

static s32 wario_landing_step(struct MarioState *m, s32 animation, u32 nextAction) {
    stationary_ground_step(m);
    set_character_animation(m, animation);
    if (is_anim_at_end(m)) {
        return set_mario_action(m, nextAction, 0);
    }
    return FALSE;
}

s32 act_wario_pile_driver_land(struct MarioState *m) {
    if (!m) { return FALSE; }

    m->actionState = 1;
    queue_rumble_data_mario(m, 4, 50);
    mario_drop_held_object(m);

    if (m->input & INPUT_UNKNOWN_10) {
        return drop_and_set_mario_action(m, ACT_SHOCKWAVE_BOUNCE, 0);
    }

    if (m->input & INPUT_OFF_FLOOR) {
        return set_mario_action(m, ACT_FREEFALL, 0);
    }

    if (m->input & INPUT_ABOVE_SLIDE) {
        return set_mario_action(m, ACT_BUTT_SLIDE, 0);
    }

    // Coin attraction is not present in this codebase; omit.
    wario_landing_step(m, CHAR_ANIM_GROUND_POUND_LANDING, ACT_BUTT_SLIDE_STOP);
    return FALSE;
}

s32 act_wario_charge(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (!is_wario(m)) { return set_mario_action(m, ACT_WALKING, 0); }

    if (m->input & INPUT_A_PRESSED) {
        sWarioChargeCount = 0;
        return set_mario_action(m, ACT_TRIPLE_JUMP, 0);
    }

    if (sWarioChargeCount == 0) {
        if (!(m->flags & MARIO_MARIO_SOUND_PLAYED)) {
            play_character_sound_offset(m, CHAR_SOUND_HELLO, ((random_u16() % 3U) << 16));
            m->flags |= MARIO_MARIO_SOUND_PLAYED;
        }
    }
    sWarioChargeCount++;

    if (sWarioChargeCount < 60) {
        // Use a shell-like max speed feel without depending on private helpers.
        if (m->forwardVel < 48.0f) {
            mario_set_forward_vel(m, m->forwardVel + 4.0f);
        }
        if (m->forwardVel > 48.0f) {
            mario_set_forward_vel(m, 48.0f);
        }

        set_character_anim_with_accel(m, CHAR_ANIM_RUNNING_UNUSED, 0x000C0000);
        play_step_sound(m, 9, 45);

        switch (perform_ground_step(m)) {
            case GROUND_STEP_LEFT_GROUND:
                sWarioChargeCount = 0;
                set_mario_action(m, ACT_FREEFALL, 0);
                set_character_animation(m, CHAR_ANIM_GENERAL_FALL);
                break;

            case GROUND_STEP_HIT_WALL:
                sWarioChargeCount = 0;
                mario_stop_riding_object(m);
                play_sound((m->flags & MARIO_METAL_CAP) ? SOUND_ACTION_METAL_BONK : SOUND_ACTION_BONK,
                           m->marioObj->header.gfx.cameraToObject);
                set_mario_particle_flags(m, PARTICLE_VERTICAL_STAR, FALSE);
                set_mario_action(m, ACT_BACKWARD_GROUND_KB, 0);
                break;

            case GROUND_STEP_NONE:
                m->flags |= MARIO_KICKING;
                set_mario_particle_flags(m, PARTICLE_DUST, FALSE);
                break;
        }

        adjust_sound_for_speed(m);
        reset_rumble_timers(m);
    } else {
        sWarioChargeCount = 0;
        set_mario_action(m, ACT_WALKING, 0);
    }

    return FALSE;
}

s32 act_wario_triple_jump(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (!is_wario(m)) { return set_mario_action(m, ACT_TRIPLE_JUMP, 0); }

    if (m->input & INPUT_B_PRESSED) {
        return set_mario_action(m, ACT_DIVE, 0);
    }

    if (m->input & INPUT_Z_PRESSED) {
        return set_mario_action(m, ACT_GROUND_POUND, 0);
    }

    if (m->actionTimer == 0) {
        play_mario_jump_sound(m);
        play_mario_sound(m, SOUND_ACTION_TERRAIN_JUMP, 0);
    }
    m->actionTimer++;

    update_air_without_turn(m);

    common_air_action_step(m, ACT_TRIPLE_JUMP_LAND, CHAR_ANIM_FORWARD_SPINNING, 0);
    if (m->marioObj->header.gfx.animInfo.animFrame == 1) {
        play_sound(SOUND_ACTION_SPIN, m->marioObj->header.gfx.cameraToObject);
    }

    return FALSE;
}

static void update_wario_spin_walk_speed(struct MarioState *m) {
    s16 intendedDYaw;
    f32 intendedMag;
    f32 faceAngleYaw = m->faceAngle[1];

    if (m->input & INPUT_NONZERO_ANALOG) {
        intendedDYaw = m->intendedYaw - m->faceAngle[1];
        intendedMag = m->intendedMag / 32.0f;

        m->forwardVel += coss(intendedDYaw) * intendedMag * 100.0f;
        faceAngleYaw = m->faceAngle[1] + (s16)(sins(intendedDYaw) * intendedMag * 1024.0f);
        if (m->forwardVel < 0.0f) {
            faceAngleYaw += intendedDYaw;
            m->forwardVel *= -1.0f;
        }

        if (m->forwardVel > 32.0f) {
            m->forwardVel = 32.0f;
        }
    }

    m->vel[0] = m->slideVelX = m->forwardVel * sins((s16)faceAngleYaw);
    m->vel[2] = m->slideVelZ = m->forwardVel * coss((s16)faceAngleYaw);
}

static s32 act_walking_wario_spin(struct MarioState *m) {
    update_wario_spin_walk_speed(m);
    switch (perform_ground_step(m)) {
        case GROUND_STEP_LEFT_GROUND:
            sWarioWalkSpin = false;
            sWarioSpinCount = 0;
            set_mario_action(m, ACT_FREEFALL, 0);
            set_character_animation(m, CHAR_ANIM_GENERAL_FALL);
            break;
    }

    return FALSE;
}

s32 act_picking_up_enemies(struct MarioState *m) {
    if (!m) { return FALSE; }

    if (m->actionState == 0) {
        m->actionState = 1;
        m->angleVel[1] = 0;
        m->marioBodyState->grabPos = GRAB_POS_LIGHT_OBJ;
        mario_grab_used_object(m);
        if (m->heldObj != NULL) {
            queue_rumble_data_mario(m, 5, 80);
            play_character_sound_if_no_flag(m, CHAR_SOUND_HRMM, MARIO_MARIO_SOUND_PLAYED);
        }
    }

    m->marioBodyState->grabPos = GRAB_POS_LIGHT_OBJ_SPIN;
    set_character_animation(m, CHAR_ANIM_GRAB_BOWSER);
    if (is_anim_at_end(m)) {
        set_mario_action(m, ACT_HOLDING_ENEMIES, 0);
    }

    stationary_ground_step(m);
    return FALSE;
}

s32 act_holding_enemies(struct MarioState *m) {
    if (!m) { return FALSE; }
    s16 spin;

    if (m->input & INPUT_B_PRESSED) {
        play_character_sound_if_no_flag(m, CHAR_SOUND_HERE_WE_GO, MARIO_MARIO_SOUND_PLAYED);
        sWarioWalkSpin = false;
        sWarioSpinCount = 0;
        return set_mario_action(m, ACT_RELEASING_ENEMIES, 0);
    }

    if (m->angleVel[1] == 0) {
        if (m->actionTimer++ > 120) {
            return set_mario_action(m, ACT_RELEASING_ENEMIES, 1);
        }
        set_character_animation(m, CHAR_ANIM_HOLDING_BOWSER);
    } else {
        m->actionTimer = 0;
        set_character_animation(m, CHAR_ANIM_SWINGING_BOWSER);
    }

    if (m->intendedMag > 20.0f) {
        spin = (s16)(m->intendedYaw - m->twirlYaw) / 0x10;

        if (spin < -0x100) { spin = -0x100; }
        if (spin > 0x100) { spin = 0x100; }

        m->twirlYaw = m->intendedYaw;
        m->angleVel[1] += spin;

        if (m->angleVel[1] > 0x2000) { m->angleVel[1] = 0x2000; }
        if (m->angleVel[1] < -0x2000) { m->angleVel[1] = -0x2000; }
    } else {
        m->actionArg = 0;
        m->angleVel[1] = approach_s32(m->angleVel[1], 0, 64, 64);
    }

    if (!sWarioWalkSpin) {
        if (m->angleVel[1] <= -0xE00 || m->angleVel[1] >= 0xE00) {
            sWarioWalkSpin = true;
        } else {
            stationary_ground_step(m);
        }
    }

    if (sWarioWalkSpin) {
        if (m->angleVel[1] <= -0xE00) { m->angleVel[1] = -0x1800; }
        if (m->angleVel[1] >= 0xE00) { m->angleVel[1] = 0x1800; }

        act_walking_wario_spin(m);
        sWarioSpinCount++;

        if (sWarioSpinCount == 50) { obj_spawn_yellow_coins(m->marioObj, 1); }
        if (sWarioSpinCount == 60) { obj_spawn_yellow_coins(m->marioObj, 2); }
        if (sWarioSpinCount == 70) { obj_spawn_yellow_coins(m->marioObj, 1); }

        if (sWarioSpinCount >= 120) {
            play_character_sound_if_no_flag(m, CHAR_SOUND_SO_LONGA_BOWSER, MARIO_MARIO_SOUND_PLAYED);
            sWarioWalkSpin = false;
            sWarioSpinCount = 0;
            return set_mario_action(m, ACT_RELEASING_ENEMIES, 0);
        }
    }

    spin = m->faceAngle[1];
    m->faceAngle[1] += m->angleVel[1];

    if (m->angleVel[1] <= -0x100 && spin < m->faceAngle[1]) {
        queue_rumble_data_mario(m, 4, 20);
        play_sound(SOUND_OBJ_BOWSER_SPINNING, m->marioObj->header.gfx.cameraToObject);
    }
    if (m->angleVel[1] >= 0x100 && spin > m->faceAngle[1]) {
        queue_rumble_data_mario(m, 4, 20);
        play_sound(SOUND_OBJ_BOWSER_SPINNING, m->marioObj->header.gfx.cameraToObject);
    }

    return FALSE;
}

s32 act_releasing_enemies(struct MarioState *m) {
    if (!m) { return FALSE; }
    if (++m->actionTimer == 1) {
        if (m->actionArg == 0) {
            queue_rumble_data_mario(m, 4, 50);
            mario_throw_held_object(m);
        } else {
            queue_rumble_data_mario(m, 4, 50);
            mario_drop_held_object(m);
        }
    }
    m->angleVel[1] = 0;
    animated_stationary_ground_step(m, CHAR_ANIM_RELEASE_BOWSER, ACT_IDLE);
    return FALSE;
}
