#ifdef TARGET_WII_U

#include <stdint.h>
#include <stdbool.h>

#include <math.h>
#include <string.h>

#include <PR/ultratypes.h>
#include <PR/os_cont.h>

#include <vpad/input.h>
#include <padscore/wpad.h>
#include <padscore/kpad.h>

#include "controller_api.h"
#include "pc/configfile.h"

typedef struct WiiUKeymap {
    uint16_t mask;
    uint32_t vpad;
    uint32_t classic;
    uint32_t pro;
} WiiUKeymap;

#define VB(btn) VPAD_BUTTON_##btn
#define CB(btn) WPAD_CLASSIC_BUTTON_##btn
#define PB(btn) WPAD_PRO_BUTTON_##btn
#define PT(btn) WPAD_PRO_TRIGGER_##btn

#define VE(dir) VPAD_STICK_R_EMULATION_##dir
#define CE(dir) WPAD_CLASSIC_STICK_R_EMULATION_##dir
#define PE(dir) WPAD_PRO_STICK_R_EMULATION_##dir

static WiiUKeymap sMap[] = {
    { B_BUTTON,     VB(B) | VB(Y),       CB(B) | CB(Y),       PB(B) | PB(Y) },
    { A_BUTTON,     VB(A) | VB(X),       CB(A) | CB(X),       PB(A) | PB(X) },
    { START_BUTTON, VB(PLUS),            CB(PLUS),            PB(PLUS) },
    { Z_TRIG,       VB(L) | VB(ZL),      CB(L) | CB(ZL),      PT(L) | PT(ZL) },
    { R_TRIG,       VB(R) | VB(ZR),      CB(R) | CB(ZR),      PT(R) | PT(ZR) },
    { U_CBUTTONS,   VE(UP),              CE(UP),              PE(UP) },
    { R_CBUTTONS,   VE(RIGHT),           CE(RIGHT),           PE(RIGHT) },
    { D_CBUTTONS,   VE(DOWN),            CE(DOWN),            PE(DOWN) },
    { L_CBUTTONS,   VE(LEFT),            CE(LEFT),            PE(LEFT) },
};

static KPADStatus sLastKpad = { 0 };
static int sKpadTimeout = 10;

static void controller_wiiu_init(void) {
    VPADInit();
    KPADInit();
    WPADEnableURCC(1);
    WPADEnableWiiRemote(1);

    if (configN64FaceButtons) {
        sMap[0] = (WiiUKeymap) { B_BUTTON, VB(Y) | VB(X), CB(Y) | CB(X), PB(Y) | PB(X) };
        sMap[1] = (WiiUKeymap) { A_BUTTON, VB(B) | VB(A), CB(B) | CB(A), PB(B) | PB(A) };
    }
}

static void read_vpad(OSContPad *pad) {
    VPADStatus status;
    VPADReadError err;
    VPADRead(VPAD_CHAN_0, &status, 1, &err);

    uint32_t v = status.hold;

    if (err != 0) {
        return;
    }

    for (size_t i = 0; i < (sizeof(sMap) / sizeof(sMap[0])); i++) {
        if (v & sMap[i].vpad) {
            pad->button |= sMap[i].mask;
        }
    }

    if (v & VPAD_BUTTON_LEFT) pad->stick_x = -80;
    if (v & VPAD_BUTTON_RIGHT) pad->stick_x = 80;
    if (v & VPAD_BUTTON_DOWN) pad->stick_y = -80;
    if (v & VPAD_BUTTON_UP) pad->stick_y = 80;

    s8 ax = (s8)round(status.leftStick.x * 80);
    s8 ay = (s8)round(status.leftStick.y * 80);
    if (ax != 0) { pad->stick_x = ax; }
    if (ay != 0) { pad->stick_y = ay; }
}

static void read_wpad(OSContPad *pad) {
    WPADExtensionType ext;
    for (int i = 1; i < 4; i++) {
        int res = WPADProbe(i, &ext);
        if (res == 0) {
            WPADDisconnect(i);
        }
    }

    int res = WPADProbe(WPAD_CHAN_0, &ext);
    if (res != 0) {
        return;
    }

    KPADStatus status;
    int err;
    int read = KPADReadEx(WPAD_CHAN_0, &status, 1, &err);
    if (read == 0) {
        sKpadTimeout--;

        if (sKpadTimeout == 0) {
            WPADDisconnect(WPAD_CHAN_0);
            memset(&sLastKpad, 0, sizeof(sLastKpad));
            return;
        }

        status = sLastKpad;
    } else {
        sKpadTimeout = 10;
        sLastKpad = status;
    }

    KPADVec2D stick;
    bool disconnect = false;

    uint32_t wm = status.hold;

    if (wm & WPAD_BUTTON_MINUS) {
        disconnect = true;
    }

    if (status.extensionType == WPAD_EXT_NUNCHUK || status.extensionType == WPAD_EXT_MPLUS_NUNCHUK) {
        uint32_t ext = status.nunchuk.hold;
        stick = status.nunchuk.stick;

        if (wm & WPAD_BUTTON_A) pad->button |= A_BUTTON;
        if (wm & WPAD_BUTTON_B) pad->button |= B_BUTTON;
        if (wm & WPAD_BUTTON_PLUS) pad->button |= START_BUTTON;
        if (wm & WPAD_BUTTON_UP) pad->button |= U_CBUTTONS;
        if (wm & WPAD_BUTTON_DOWN) pad->button |= D_CBUTTONS;
        if (wm & WPAD_BUTTON_LEFT) pad->button |= L_CBUTTONS;
        if (wm & WPAD_BUTTON_RIGHT) pad->button |= R_CBUTTONS;

        if (ext & WPAD_NUNCHUK_BUTTON_C) pad->button |= R_TRIG;
        if (ext & WPAD_NUNCHUK_BUTTON_Z) pad->button |= Z_TRIG;

    } else if (status.extensionType == WPAD_EXT_CLASSIC || status.extensionType == WPAD_EXT_MPLUS_CLASSIC) {
        uint32_t ext = status.classic.hold;
        stick = status.classic.leftStick;

        for (size_t i = 0; i < (sizeof(sMap) / sizeof(sMap[0])); i++) {
            if (ext & sMap[i].classic) {
                pad->button |= sMap[i].mask;
            }
        }

        if (ext & WPAD_CLASSIC_BUTTON_LEFT) pad->stick_x = -80;
        if (ext & WPAD_CLASSIC_BUTTON_RIGHT) pad->stick_x = 80;
        if (ext & WPAD_CLASSIC_BUTTON_DOWN) pad->stick_y = -80;
        if (ext & WPAD_CLASSIC_BUTTON_UP) pad->stick_y = 80;

        if (ext & WPAD_CLASSIC_BUTTON_MINUS) disconnect = true;

    } else if (status.extensionType == WPAD_EXT_PRO_CONTROLLER) {
        uint32_t ext = status.pro.hold;
        stick = status.pro.leftStick;

        for (size_t i = 0; i < (sizeof(sMap) / sizeof(sMap[0])); i++) {
            if (ext & sMap[i].pro) {
                pad->button |= sMap[i].mask;
            }
        }

        if (ext & WPAD_PRO_BUTTON_LEFT) pad->stick_x = -80;
        if (ext & WPAD_PRO_BUTTON_RIGHT) pad->stick_x = 80;
        if (ext & WPAD_PRO_BUTTON_DOWN) pad->stick_y = -80;
        if (ext & WPAD_PRO_BUTTON_UP) pad->stick_y = 80;

        if (ext & WPAD_PRO_BUTTON_MINUS) disconnect = true;
    } else {
        return;
    }

    s8 ax = (s8)round(stick.x * 80);
    s8 ay = (s8)round(stick.y * 80);
    if (ax != 0) { pad->stick_x = ax; }
    if (ay != 0) { pad->stick_y = ay; }

    if (disconnect) {
        WPADDisconnect(WPAD_CHAN_0);
    }
}

static void controller_wiiu_read(OSContPad *pad) {
    OSContPad vpad = { 0 };
    OSContPad wpad = { 0 };

    read_vpad(&vpad);
    read_wpad(&wpad);

    pad->button = vpad.button | wpad.button;

    if (vpad.stick_x != 0 || vpad.stick_y != 0) {
        pad->stick_x = vpad.stick_x;
        pad->stick_y = vpad.stick_y;
    } else {
        pad->stick_x = wpad.stick_x;
        pad->stick_y = wpad.stick_y;
    }
}

static u32 controller_wiiu_rawkey(void) {
    return VK_INVALID;
}

struct ControllerAPI controller_wiiu = {
    0,
    controller_wiiu_init,
    controller_wiiu_read,
    controller_wiiu_rawkey,
    NULL,
    NULL,
    NULL,
    NULL,
};

#endif
