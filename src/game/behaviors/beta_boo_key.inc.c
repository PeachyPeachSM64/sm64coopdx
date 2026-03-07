/**
 * Behavior for bhvAlphaBooKey and bhvBetaBooKey (Luigi Keys).
 * Repurposed from the beta boo key mechanic for Render96 collectibles.
 */

#include "game/save_file.h"

static void validate_key(void) {
    /*if (save_file_taken_key(gCurrSaveFileNum - 1, o->oBehParams2ndByte)) {
        cur_obj_become_intangible();
        cur_obj_disable_rendering();
        obj_mark_for_deletion(o);
    }*/
}

void bhv_key_init(void) {
    o->oPosY += 80;
    validate_key();
}

/**
 * Update function for bhvAlphaBooKey.
 * It rotates the key, and deletes it when collected.
 */
void bhv_alpha_boo_key_loop(void) {
    // Rotate the key
    o->oFaceAngleRoll += 0x200;
    o->oFaceAngleYaw += 0x200;

    if (obj_check_if_collided_with_object(o, gMarioObject)) {
        if (o->parentObj) {
            o->parentObj->oBooDeathStatus = BOO_DEATH_STATUS_DYING;
        }

        // Delete the object and spawn sparkles
        obj_mark_for_deletion(o);
        spawn_object(o, MODEL_SPARKLES, bhvGoldenCoinSparkles);
    }
}

// For some reason, the action functions for the beta boo key
// are written in reverse order.

/**
 * Continue to make the key fall, and handle collection.
 */
static void beta_boo_key_dropped_loop(void) {
    // Apply standard physics to the key
    cur_obj_update_floor_and_walls();
    cur_obj_move_standard(78);

    // Slowly increase the Y offset to make the model aligned correctly.
    // This is spread out over 13 frames so that it's not noticable.
    if (o->oGraphYOffset < 26.0f) {
        o->oGraphYOffset += 2.0f;
    }

    // Transition from rotating in both the yaw and the roll axes
    // to just in the yaw axis. This is done by truncating the key's roll
    // to the nearest multiple of 0x800, then continuously adding 0x800
    // until it reaches a multiple of 0x10000, at which point &-ing with
    // 0xFFFF returns 0 and the key stops rotating in the roll direction.
    if (o->oFaceAngleRoll & 0xFFFF) {
        o->oFaceAngleRoll &= 0xF800;
        o->oFaceAngleRoll += 0x800;
    }

    // Once the key stops bouncing, stop its horizontal movement on the ground.
    if (o->oMoveFlags & OBJ_MOVE_ON_GROUND) {
        o->oVelX = 0.0f;
        o->oVelZ = 0.0f;
    }

    // Rotate the key
    o->oFaceAngleYaw += 0x800;

    // If the key hits the floor or 90 frames have elapsed since it was dropped,
    // become tangible and handle collision.
    if (o->oTimer > 90 || o->oMoveFlags & OBJ_MOVE_LANDED) {
        cur_obj_become_tangible();

        if (obj_check_if_collided_with_object(o, gMarioObject)) {
            // This interaction status is 0x01, the first interaction status flag.
            // It was only used for Hoot in the final game, but it seems it could've
            // done something else or held some special meaning in beta.
            // Earlier, in beta_boo_key_drop (called when the parent boo is killed),
            // o->parentObj is set to the parent boo's parentObj. This means that
            // here, the parentObj is actually the parent of the old parent boo.
            // One theory about this code is that there was a boo spawner, which
            // spawned "false" boos and one "true" boo with the key, and the player
            // was intended to find the one with the key to progress.
            if (o->parentObj) { o->parentObj->oInteractStatus = INT_STATUS_HOOT_GRABBED_BY_MARIO; }

            // Delete the object and spawn sparkles
            obj_mark_for_deletion(o);
            spawn_object(o, MODEL_SPARKLES, bhvGoldenCoinSparkles);
        }
    }
}

/**
 * Drop the key. This function is run once, the frame after the boo dies;
 * It immediately sets the action to BETA_BOO_KEY_ACT_DROPPED.
 */
static void beta_boo_key_drop(void) {
    s16 velocityDirection;
    f32 velocityMagnitude;

    // Update the key to be inside the boo
    struct Object *parent = o->parentObj;
    obj_copy_pos(o, parent);

    // This if statement to only run this code on the first frame
    // is redundant, since it instantly sets the action to BETA_BOO_KEY_ACT_DROPPED
    // which stops this function from running again.
    if (o->oTimer == 0) {
        // Separate from the parent boo
        if (parent) {
            o->parentObj = parent->parentObj;
        }

        o->oAction = BETA_BOO_KEY_ACT_DROPPED;

        // Make the key move laterally away from Mario at 3 units/frame
        // (as if he transferred kinetic energy to it)
        velocityDirection = gMarioObject ? gMarioObject->oMoveAngleYaw : 0;
        velocityMagnitude = 3.0f;

        o->oVelX = sins(velocityDirection) * velocityMagnitude;
        o->oVelZ = coss(velocityDirection) * velocityMagnitude;

        // Give it an initial Y velocity of 40 units/frame
        o->oVelY = 40.0f;
    }

    // Rotate the key
    o->oFaceAngleYaw += 0x200;
    o->oFaceAngleRoll += 0x200;
}

/**
 * Update the key to be inside its parent boo, and handle the boo dying.
 */
static void beta_boo_key_inside_boo_loop(void) {
    // Update the key to be inside the boo at all times
    struct Object *parent = o->parentObj;
    obj_copy_pos(o, parent);

    // Use a Y offset of 40 to make the key model aligned correctly.
    // (Why didn't they use oGraphYOffset?)
    o->oPosY += 40.0f;

    // If the boo is dying/dead, set the action to BETA_BOO_KEY_ACT_DROPPING.
    if (parent && parent->oBooDeathStatus != BOO_DEATH_STATUS_ALIVE) {
        o->oAction = BETA_BOO_KEY_ACT_DROPPING;
    }

    // Rotate the key
    o->oFaceAngleRoll += 0x200;
    o->oFaceAngleYaw += 0x200;
}

static void (*sBetaBooKeyActions[])(void) = { beta_boo_key_inside_boo_loop, beta_boo_key_drop,
                                              beta_boo_key_dropped_loop };

/**
 * Update function for bhvBetaBooKey (Luigi Key collectible).
 */
void bhv_beta_boo_key_loop(void) {
    o->oFaceAngleYaw += 0x700;
    o->oPosY += sins(o->oFaceAngleYaw / (20 * 1000)) * 2;
    
    if (obj_check_if_collided_with_object(o, gMarioObject)) {
        obj_mark_for_deletion(o);
        spawn_object(o, MODEL_SPARKLES, bhvGoldenCoinSparkles);
        play_sound(SOUND_GENERAL_COIN, gGlobalSoundSource);
        gMarioState->numKeys++;
        //save_file_register_key(gCurrSaveFileNum - 1, o->oBehParams2ndByte);
    }
}

// Wario Coin functions
static void validate_wario_coin(void) {
    /*if (save_file_taken_wario_coin(gCurrSaveFileNum - 1, o->oBehParams2ndByte)) {
        cur_obj_become_intangible();
        cur_obj_disable_rendering();
        obj_mark_for_deletion(o);
    }*/
}

void bhv_wario_coin_init(void) {
    cur_obj_scale(2.0f);
    validate_wario_coin();
}

void bhv_wario_coin_loop(void) {
    if (obj_check_if_collided_with_object(o, gMarioObject)) {
        obj_mark_for_deletion(o);
        spawn_object(o, MODEL_SPARKLES, bhvGoldenCoinSparkles);
        play_sound(SOUND_GENERAL_COIN, gGlobalSoundSource);
        gMarioState->numWarioCoins++;
        //save_file_register_wario_coin(gCurrSaveFileNum - 1, o->oBehParams2ndByte);
    }
}
