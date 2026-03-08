#include "cheats.h"
#include "pc/controller/controller_api.h"

struct CheatList Cheats;

struct CheatControls CheatsControls = {
    .TimeStopButton = { VK_INVALID, VK_INVALID, VK_INVALID },
    .SpambaControls = { 0x100D, 0x100E, 0x1009 },
};
