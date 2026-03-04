local directional = require('directional')

local sDebugMode = false
local sYawDeg = 0
local sPitchDeg = 0
local sIntensity = 0.4
local sInitialized = false

local YAW_SPEED = 5.0
local PITCH_SPEED = 5.0
local INTENSITY_SPEED = 0.02

local function on_mario_update(m)
    if m.playerIndex ~= 0 then return end

    local controller = m.controller
    local pressed = controller.buttonPressed

    if (pressed & L_JPAD) ~= 0 and not sDebugMode then
        sDebugMode = true
        directional.debugModeActive = true

        if not sInitialized then
            local defYaw, defPitch, defIntensity = directional.getCurrentLevelDefaults()
            sYawDeg = defYaw
            sPitchDeg = defPitch
            sIntensity = defIntensity
            sInitialized = true
        end

        directional.debugYawDeg = sYawDeg
        directional.debugPitchDeg = sPitchDeg
        directional.debugIntensity = sIntensity

        play_sound(SOUND_MENU_CLICK_FILE_SELECT, gGlobalSoundSource)
        return
    end

    if not sDebugMode then return end

    if (pressed & L_JPAD) ~= 0 then
        sDebugMode = false
        directional.debugModeActive = false
        play_sound(SOUND_MENU_CLICK_FILE_SELECT, gGlobalSoundSource)
        return
    end

    local held = controller.buttonDown

    if (held & U_JPAD) ~= 0 then
        sPitchDeg = sPitchDeg - PITCH_SPEED
        if sPitchDeg < -90 then sPitchDeg = -90 end
    end
    if (held & D_JPAD) ~= 0 then
        sPitchDeg = sPitchDeg + PITCH_SPEED
        if sPitchDeg > 90 then sPitchDeg = 90 end
    end
    if (held & R_JPAD) ~= 0 then
        sYawDeg = sYawDeg + YAW_SPEED
        if sYawDeg >= 360 then sYawDeg = sYawDeg - 360 end
    end
    if (pressed & L_JPAD) == 0 and (held & L_JPAD) ~= 0 then
        sYawDeg = sYawDeg - YAW_SPEED
        if sYawDeg < 0 then sYawDeg = sYawDeg + 360 end
    end

    if (pressed & R_TRIG) ~= 0 then
        sIntensity = sIntensity + INTENSITY_SPEED
        if sIntensity > 1.0 then sIntensity = 1.0 end
    end
    if (pressed & Z_TRIG) ~= 0 then
        sIntensity = sIntensity - INTENSITY_SPEED
        if sIntensity < 0.0 then sIntensity = 0.0 end
    end

    directional.debugYawDeg = sYawDeg
    directional.debugPitchDeg = sPitchDeg
    directional.debugIntensity = sIntensity
end

local function on_hud_render()
    if not sDebugMode then return end

    djui_hud_set_resolution(RESOLUTION_DJUI)
    djui_hud_set_font(FONT_NORMAL)

    local screenWidth = djui_hud_get_screen_width()
    local x = 20
    local y = 100
    local lineHeight = 32
    local scale = 1.0

    djui_hud_set_color(0, 0, 0, 200)
    djui_hud_render_rect(x - 10, y - 10, 320, 160)

    djui_hud_set_color(255, 255, 0, 255)
    djui_hud_print_text("LIGHTING DEBUG MODE", x, y, scale)

    y = y + lineHeight
    djui_hud_set_color(255, 255, 255, 255)
    djui_hud_print_text(string.format("Yaw: %.1f deg", sYawDeg), x, y, scale)

    y = y + lineHeight
    djui_hud_print_text(string.format("Pitch: %.1f deg", sPitchDeg), x, y, scale)

    y = y + lineHeight
    djui_hud_print_text(string.format("Intensity: %.0f%%", sIntensity * 100), x, y, scale)

    y = y + lineHeight + 10
    djui_hud_set_color(180, 180, 180, 255)
    djui_hud_set_font(FONT_TINY)
    djui_hud_print_text("D-Pad: Rotate | R/Z: Intensity | D-Left: Exit", x, y, 0.8)
end

local function on_level_init()
    sInitialized = false
end

hook_event(HOOK_MARIO_UPDATE, on_mario_update)
hook_event(HOOK_ON_HUD_RENDER, on_hud_render)
hook_event(HOOK_ON_LEVEL_INIT, on_level_init)

return {}
