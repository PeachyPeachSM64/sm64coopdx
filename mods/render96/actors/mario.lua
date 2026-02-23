-- Mario Script
-- Made by ExcellentGamer, Handles face expressions, eye expressions, and pose animations
-- Controls Mario's 
---------------------
----- # Poses # -----
---------------------

-- Register Pose Animations
require('poses/render_pose_1')

-- Ground Poses
photo_mode_pose_register_custom_anim(false, 'Render Pose 1', 'render_pose_1', 1)

---------------------
-- # Face States # --
---------------------

local EYES_OPEN = 0
local EYES_HALF_CLOSED = 1
local EYES_CLOSED = 2
local EYES_DOWN = 3
local EYES_ANGRY = 4
local EYES_HAPPY = 5
local EYES_EXHAUSTED = 6
local EYES_DEAD = 7
local EYES_HURT = 8

local MOUTH_DEFAULT = 0
local MOUTH_HAPPY = 3
local MOUTH_ANGRY = 4
local MOUTH_OPEN = 5

local DEFAULT_EYE_SWITCH_INDEX = -1

-- Register Eye States
photo_mode_eye_state_reset(CT_MARIO)
photo_mode_eye_state_register(CT_MARIO, 'Open', EYES_OPEN)
photo_mode_eye_state_register(CT_MARIO, 'Half Closed', EYES_HALF_CLOSED)
photo_mode_eye_state_register(CT_MARIO, 'Closed', EYES_CLOSED)
photo_mode_eye_state_register(CT_MARIO, 'Dead', EYES_DEAD)
photo_mode_eye_state_register(CT_MARIO, 'Down', EYES_DOWN)
photo_mode_eye_state_register(CT_MARIO, 'Angry', EYES_ANGRY)
photo_mode_eye_state_register(CT_MARIO, 'Happy', EYES_HAPPY)
photo_mode_eye_state_register(CT_MARIO, 'Exhuasted', EYES_EXHAUSTED)
photo_mode_eye_state_register(CT_MARIO, 'Hurt', EYES_HURT)

-- Register Mouth States
photo_mode_mouth_state_reset(CT_MARIO)
photo_mode_mouth_state_register(CT_MARIO, 'Default', MOUTH_DEFAULT)
photo_mode_mouth_state_register(CT_MARIO, 'Happy', MOUTH_HAPPY)
photo_mode_mouth_state_register(CT_MARIO, 'Angry', MOUTH_ANGRY)
photo_mode_mouth_state_register(CT_MARIO, 'Open', MOUTH_OPEN)

local sLastAppliedEyeSwitchIndex = DEFAULT_EYE_SWITCH_INDEX
local sLastAppliedFaceSwitchIndex = MOUTH_DEFAULT

local KICK_OPEN_MOUTH_DURATION_TICKS = 15
local sKickOpenMouthUntil = 0
local sWasInKick = false

local sDoubleJumpOpenMouthUntil = 0
local sWasInDoubleJump = false

local LAND_HAPPY_SMILE_DURATION_TICKS = 20
local sLandHappySmileUntil = 0
local sWasInLandHappySmile = false

local function render96_is_mod_active(modRelativePath)
    if gActiveMods == nil or modRelativePath == nil then return false end
    local i = 0
    while gActiveMods[i] ~= nil do
        local mod = gActiveMods[i]
        if mod ~= nil and mod.relativePath == modRelativePath then
            return true
        end
        i = i + 1
    end
    return false
end

local function render96_in_battle_stance_animation(m)
    if m == nil or m.marioObj == nil then return false end
    local animName = smlua_anim_util_get_current_animation_name(m.marioObj)
    return animName == "MARIO_ANIM_BATTLE_STANCE"
end

local function render96_should_avoid_overriding_external_expression(playerIndex)
    local curEye = mario_get_eye_switch_index(playerIndex)
    local curFace = mario_get_face_switch_index(playerIndex)

    if curEye ~= DEFAULT_EYE_SWITCH_INDEX and curEye ~= sLastAppliedEyeSwitchIndex then
        return true
    end

    if curFace ~= MOUTH_DEFAULT and curFace ~= sLastAppliedFaceSwitchIndex then
        return true
    end

    return false
end

local function render96_in_double_jump_state(m)
    if not m then return false end
    return m.action == ACT_DOUBLE_JUMP or m.action == ACT_DOUBLE_JUMP_LAND or m.action == ACT_DOUBLE_JUMP_LAND_STOP
end

local function render96_in_land_happy_smile_state(m)
    if not m then return false end
    return m.action == ACT_BACKFLIP_LAND or m.action == ACT_BACKFLIP_LAND_STOP or m.action == ACT_TRIPLE_JUMP_LAND or m.action == ACT_TRIPLE_JUMP_LAND_STOP
end

local function render96_apply_expression(playerIndex, eyeSwitchIndex, faceSwitchIndex)
    mario_set_eye_switch_index(playerIndex, eyeSwitchIndex)
    mario_set_face_switch_index(playerIndex, faceSwitchIndex)
    sLastAppliedEyeSwitchIndex = eyeSwitchIndex
    sLastAppliedFaceSwitchIndex = faceSwitchIndex
end

local function render96_in_kick_state(m)
    if not m then return false end

    local action = m.action

    if action == ACT_JUMP_KICK or action == ACT_SLIDE_KICK or action == ACT_SLIDE_KICK_SLIDE or action == ACT_SLIDE_KICK_SLIDE_STOP then
        return true
    end

    if (action == ACT_PUNCHING or action == ACT_MOVE_PUNCHING) and m.actionArg == 6 then
        return true
    end

    return false
end

---@param m MarioState
local function render96_normal_game_face_states(m)
    if m.playerIndex ~= 0 then return end

    if render96_is_mod_active("sharen-game-enhancer") and render96_in_battle_stance_animation(m) then
        render96_apply_expression(0, EYES_ANGRY, MOUTH_ANGRY)
        return
    end

    if render96_should_avoid_overriding_external_expression(0) then
        return
    end

    local action = m.action
    local hurtCounter = m.hurtCounter
    local health = m.health
    local bodyState = m.marioBodyState
    local now = get_area_update_counter()

    if action == ACT_BURNING_GROUND or action == ACT_BURNING_JUMP or action == ACT_BURNING_FALL then
        render96_apply_expression(0, EYES_DEAD, MOUTH_OPEN)
        return
    end

    local inDoubleJump = render96_in_double_jump_state(m)
    if inDoubleJump and not sWasInDoubleJump then
        sDoubleJumpOpenMouthUntil = now + KICK_OPEN_MOUTH_DURATION_TICKS
    end
    sWasInDoubleJump = inDoubleJump

    if inDoubleJump then
        if now < sDoubleJumpOpenMouthUntil then
            render96_apply_expression(0, DEFAULT_EYE_SWITCH_INDEX, MOUTH_OPEN)
        else
            render96_apply_expression(0, DEFAULT_EYE_SWITCH_INDEX, MOUTH_DEFAULT)
        end
        return
    end

    local inKick = render96_in_kick_state(m)
    if inKick and not sWasInKick then
        sKickOpenMouthUntil = now + KICK_OPEN_MOUTH_DURATION_TICKS
    end
    sWasInKick = inKick

    if inKick then
        if now < sKickOpenMouthUntil then
            render96_apply_expression(0, EYES_ANGRY, MOUTH_OPEN)
        else
            render96_apply_expression(0, EYES_ANGRY, MOUTH_ANGRY)
        end
        return
    end

    if action == ACT_PUNCHING or action == ACT_MOVE_PUNCHING or action == ACT_WATER_PUNCH then
        render96_apply_expression(0, EYES_ANGRY, MOUTH_ANGRY)
        return
    end

    if (action & ACT_FLAG_SWIMMING) ~= 0 then
        render96_apply_expression(0, EYES_ANGRY, MOUTH_DEFAULT)
        return
    end

    local inLandHappySmile = render96_in_land_happy_smile_state(m)
    if inLandHappySmile and not sWasInLandHappySmile then
        sLandHappySmileUntil = now + LAND_HAPPY_SMILE_DURATION_TICKS
    end
    sWasInLandHappySmile = inLandHappySmile

    if inLandHappySmile then
        if now < sLandHappySmileUntil then
            render96_apply_expression(0, EYES_HAPPY, MOUTH_HAPPY)
        else
            render96_apply_expression(0, DEFAULT_EYE_SWITCH_INDEX, MOUTH_DEFAULT)
        end
        return
    end

    if action == ACT_CRAWLING then
        render96_apply_expression(0, EYES_DOWN, MOUTH_DEFAULT)
        return
    end

    if action == ACT_WALKING or action == ACT_HOLD_WALKING or action == ACT_HOLD_HEAVY_WALKING then
        local speed = 0
        if m.forwardVel ~= nil then
            speed = math.abs(m.forwardVel)
        end

        if speed < 16 then
            render96_apply_expression(0, EYES_DOWN, MOUTH_DEFAULT)
        else
            render96_apply_expression(0, DEFAULT_EYE_SWITCH_INDEX, MOUTH_DEFAULT)
        end
        return
    end

    if action == ACT_DEATH_EXIT or action == ACT_DEATH_EXIT_LAND or action == ACT_DEATH_ON_STOMACH or action == ACT_DEATH_ON_BACK
        or action == ACT_QUICKSAND_DEATH or action == ACT_ELECTROCUTION or action == ACT_SUFFOCATION then
        render96_apply_expression(0, EYES_HURT, MOUTH_DEFAULT)
        return
    end

    if hurtCounter ~= nil and hurtCounter > 0 then
        render96_apply_expression(0, EYES_HURT, MOUTH_ANGRY)
        return
    end

    if health ~= nil and health <= 0xFF then
        render96_apply_expression(0, EYES_HURT, MOUTH_ANGRY)
        return
    end

    if bodyState ~= nil then
        local stepSfx = m.terrainSoundAddend
        if stepSfx == SOUND_ACTION_TERRAIN_STEP_TIPTOE or stepSfx == SOUND_ACTION_METAL_STEP_TIPTOE then
            render96_apply_expression(0, EYES_DOWN, MOUTH_DEFAULT)
            return
        end
    end

    render96_apply_expression(0, DEFAULT_EYE_SWITCH_INDEX, MOUTH_DEFAULT)
end

hook_event(HOOK_MARIO_UPDATE, render96_normal_game_face_states)