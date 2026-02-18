local function bhv_goomba_force_shocked_when_jumping(o)
    if o == nil then return end

    if o.oAction == GOOMBA_ACT_JUMP then
        cur_obj_init_animation_with_accel_and_sound(1, 1.0)
    end
end

hook_behavior(id_bhvGoomba, OBJ_LIST_PUSHABLE, false, bhv_goomba_force_shocked_when_jumping, nil)
