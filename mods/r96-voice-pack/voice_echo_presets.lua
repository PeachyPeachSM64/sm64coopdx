local M = {}

M.DEFAULT = {
    strength = 0.35,
    delayFrames = 5,
    repeats = 2,
    decay = 0.65,
    vanillaCurveExp = 2.0,
    areas = {},
}

M.PRESETS = {
    [LEVEL_CASTLE] = {
        strength = 0.35,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_CASTLE_COURTYARD] = {
        strength = 0.15,
        delayFrames = 4,
        repeats = 1,
        decay = 0.55,
        vanillaCurveExp = 2.5,
        areas = {},
    },

    [LEVEL_CASTLE_GROUNDS] = {
        strength = 0.05,
        delayFrames = 2,
        repeats = 0,
        decay = 0.1,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_BOB] = {
        strength = 0.2,
        delayFrames = 2,
        repeats = 0,
        decay = 0.1,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_WF] = {
        strength = 0.2,
        delayFrames = 2,
        repeats = 0,
        decay = 0.1,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_JRB] = {
        strength = 0.25,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_CCM] = {
        strength = 0.10,
        delayFrames = 3,
        repeats = 1,
        decay = 0.45,
        vanillaCurveExp = 3.0,
        areas = {
            [2] = {
                strength = 0.55,
                delayFrames = 3,
                repeats = 5,
                decay = 0.65,
                vanillaCurveExp = 1.0,
            },
        },
    },

    [LEVEL_SSL] = {
        strength = 0.10,
        delayFrames = 3,
        repeats = 1,
        decay = 0.45,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_SL] = {
        strength = 0.08,
        delayFrames = 3,
        repeats = 1,
        decay = 0.45,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_WDW] = {
        strength = 0.10,
        delayFrames = 3,
        repeats = 1,
        decay = 0.5,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_THI] = {
        strength = 0.08,
        delayFrames = 3,
        repeats = 1,
        decay = 0.45,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_TTM] = {
        strength = 0.08,
        delayFrames = 3,
        repeats = 1,
        decay = 0.45,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_RR] = {
        strength = 0.08,
        delayFrames = 3,
        repeats = 1,
        decay = 0.45,
        vanillaCurveExp = 3.0,
        areas = {},
    },

    [LEVEL_BBH] = {
        strength = 0.35,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_HMC] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_LLL] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_DDD] = {
        strength = 0.25,
        delayFrames = 5,
        repeats = 2,
        decay = 0.6,
        vanillaCurveExp = 1.2,
        areas = {},
    },

    [LEVEL_TTC] = {
        strength = 0.25,
        delayFrames = 4,
        repeats = 2,
        decay = 0.6,
        vanillaCurveExp = 1.5,
        areas = {},
    },

    [LEVEL_BITS] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_BITDW] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_BITFS] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_BOWSER_1] = {
        strength = 0.35,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_BOWSER_2] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_BOWSER_3] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_PSS] = {
        strength = 0.5,
        delayFrames = 3,
        repeats = 3,
        decay = 0.75,
        vanillaCurveExp = 1.0,
        areas = {},
    },

    [LEVEL_COTMC] = {
        strength = 0.18,
        delayFrames = 4,
        repeats = 1,
        decay = 0.55,
        vanillaCurveExp = 2.0,
        areas = {},
    },

    [LEVEL_TOTWC] = {
        strength = 0.18,
        delayFrames = 4,
        repeats = 1,
        decay = 0.55,
        vanillaCurveExp = 2.0,
        areas = {},
    },

    [LEVEL_VCUTM] = {
        strength = 0.22,
        delayFrames = 4,
        repeats = 2,
        decay = 0.55,
        vanillaCurveExp = 2.0,
        areas = {},
    },

    [LEVEL_WMOTR] = {
        strength = 0.22,
        delayFrames = 4,
        repeats = 2,
        decay = 0.55,
        vanillaCurveExp = 2.0,
        areas = {},
    },

    [LEVEL_SA] = {
        strength = 0.30,
        delayFrames = 5,
        repeats = 2,
        decay = 0.65,
        vanillaCurveExp = 1.0,
        areas = {},
    },
}

return M
