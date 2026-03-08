#ifndef MARIO_CHEATS_H
#define MARIO_CHEATS_H

#include "types.h"

struct MarioState;

void cheats_update(struct MarioState* m);

s32 cheats_moon_jump(struct MarioState* m);
s32 cheats_moon_gravity(struct MarioState* m);
s32 cheats_super_copter(struct MarioState* m);
s32 cheats_debug_move(struct MarioState* m);
s32 cheats_god_mode(struct MarioState* m);
s32 cheats_infinite_lives(struct MarioState* m);
s32 cheats_hurt_mario(struct MarioState* m);
s32 cheats_blj_anywhere(struct MarioState* m);
s32 cheats_swim_anywhere(struct MarioState* m);
s32 cheats_no_hold_heavy(struct MarioState* m);
s32 cheats_auto_wall_kick(struct MarioState* m);
s32 cheats_coins_magnet(struct MarioState* m);
s32 cheats_time_stop(struct MarioState* m);
s32 cheats_quick_ending(struct MarioState* m);
s32 cheats_water_control(struct MarioState* m);
s32 cheats_speed_display(struct MarioState* m);
s32 cheats_size_modifier(struct MarioState* m);

#endif // MARIO_CHEATS_H
