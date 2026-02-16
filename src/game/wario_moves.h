#ifndef WARIO_MOVES_H
#define WARIO_MOVES_H

#include <PR/ultratypes.h>

struct MarioState;

s32 check_wario_pile_driver_jump_cancel(struct MarioState *m);
s32 check_wario_spin_light_idle_cancel(struct MarioState *m);
s32 check_wario_spin_heavy_idle_cancel(struct MarioState *m);

s32 act_wario_pile_driver(struct MarioState *m);
s32 act_wario_pile_driver_land(struct MarioState *m);
s32 act_wario_charge(struct MarioState *m);
s32 act_wario_triple_jump(struct MarioState *m);

s32 act_picking_up_enemies(struct MarioState *m);
s32 act_holding_enemies(struct MarioState *m);
s32 act_releasing_enemies(struct MarioState *m);

#endif // WARIO_MOVES_H
