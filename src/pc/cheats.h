#ifndef _CHEATS_H
#define _CHEATS_H

#include <stdbool.h>
typedef unsigned int uint;

struct CheatList {
    bool EnableCheats;

    bool MoonJump;
    bool GodMode;
    bool InfiniteLives;
    bool MoonGravity;
    bool DebugMove;
    bool SuperCopter;
    bool AutoWallKick;
    bool NoHoldHeavy;

    uint SpeedModifier;
    uint JumpModifier;
    uint SwimModifier;
    uint PlayAs;
    bool SpeedDisplay;

    uint BLJAnywhere;
    bool SwimAnywhere;
    bool WalkOnHazards;
    bool NoDeathBarrier;
    uint WaterLevel;
    bool CoinsMagnet;
    bool TimeStop;
    bool QuickEnding;
    uint HurtMario;

    bool Spamba;
    uint SpambaIndex;

    bool ChaosMode;
    uint Chaos[64];
};

struct CheatControls {
    uint TimeStopButton[3];
    uint SpambaControls[3];
};

extern struct CheatList Cheats;
extern struct CheatControls CheatsControls;

#endif // _CHEATS_H
