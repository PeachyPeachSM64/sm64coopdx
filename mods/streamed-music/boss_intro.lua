local M = {}

local BOSS_INTRO_SEQ_ID = 126

local function is_boss_start_dialog(dialogId)
    return dialogId == DIALOG_017
        or dialogId == DIALOG_114
        or dialogId == DIALOG_128
        or dialogId == DIALOG_117
        or dialogId == DIALOG_150
end

function M.new(opts)
    local enabled = false
    if type(opts.mod_storage_exists) == 'function' and type(opts.mod_storage_load_bool) == 'function' then
        if opts.mod_storage_exists("bossIntro") then
            enabled = opts.mod_storage_load_bool("bossIntro")
        end
    end

    local active = false
    local preOpenFrames = 0

    if type(opts.hook_mod_menu_checkbox) == 'function' then
        opts.hook_mod_menu_checkbox("Boss Intro Music", enabled, function(index, value)
            enabled = (value == true)
            if type(opts.mod_storage_save_bool) == 'function' then
                opts.mod_storage_save_bool("bossIntro", enabled)
            end
        end)
    end

    local function update(set_forced_seq_id, refresh_track)
        if not enabled then
            if active then
                active = false
                preOpenFrames = 0
                if type(set_forced_seq_id) == 'function' then
                    set_forced_seq_id(nil)
                end
                refresh_track()
            end
            return nil
        end

        if type(opts.get_dialog_box_state) ~= 'function' or type(opts.get_dialog_id) ~= 'function' then
            return nil
        end

        local ds = opts.get_dialog_box_state() or 0
        local did = opts.get_dialog_id() or -1

        if did ~= -1 and is_boss_start_dialog(did) and ds == 0 then
            preOpenFrames = 15
        end

        if preOpenFrames > 0 then
            preOpenFrames = preOpenFrames - 1
            if preOpenFrames < 0 then preOpenFrames = 0 end
        end

        if (ds ~= 0 or preOpenFrames > 0) and is_boss_start_dialog(did) then
            if not active then
                active = true
                if type(set_forced_seq_id) == 'function' then
                    set_forced_seq_id(BOSS_INTRO_SEQ_ID)
                end
                refresh_track()
            end
            return BOSS_INTRO_SEQ_ID
        end

        if active then
            active = false
            preOpenFrames = 0
            if type(set_forced_seq_id) == 'function' then
                set_forced_seq_id(nil)
            end
            refresh_track()
        end

        return nil
    end

    return {
        update = update,
    }
end

return M
