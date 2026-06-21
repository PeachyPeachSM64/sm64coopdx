#include "macro_presets.h"

struct MacroPreset MacroObjectPresets[] = {
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvOneCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvMovingBlueCoin, E_MODEL_BLUE_COIN, 0},
    {bhvBlueCoinSliding, E_MODEL_BLUE_COIN, 0}, // unused
    {bhvRedCoin, E_MODEL_RED_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvCoinFormation, E_MODEL_NONE, 0},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_RING},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_ARROW},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_FLYING},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_FLYING | COIN_FORMATION_FLAG_VERTICAL},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_FLYING | COIN_FORMATION_FLAG_RING},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_FLYING | COIN_FORMATION_FLAG_RING | COIN_FORMATION_FLAG_VERTICAL},
    {bhvCoinFormation, E_MODEL_NONE, COIN_FORMATION_FLAG_FLYING | COIN_FORMATION_FLAG_ARROW}, // unused
    {bhvHiddenStarTrigger, E_MODEL_NONE, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvUnusedFakeStar, E_MODEL_STAR, 0}, // unused
    {bhvMessagePanel, E_MODEL_WOODEN_SIGNPOST, 0},
    {bhvCannonClosed, E_MODEL_DL_CANNON_LID, 0},
    {bhvBobombBuddyOpensCannon, E_MODEL_BOBOMB_BUDDY, 0},
    {bhvButterfly, E_MODEL_BUTTERFLY, 0}, // unused
    {bhvBouncingFireball, E_MODEL_NONE, 0}, // unused
    {bhvFishSpawner, E_MODEL_NONE, 0}, // unused
    {bhvFishSpawner, E_MODEL_NONE, 1},
    {bhvBetaFishSplashSpawner, E_MODEL_NONE, 0},
    {bhvHidden1upInPoleSpawner, E_MODEL_NONE, 0},
    {bhvGoomba, E_MODEL_GOOMBA, 1},
    {bhvGoomba, E_MODEL_GOOMBA, 2},
    {bhvGoombaTripletSpawner, E_MODEL_NONE, 0},
    {bhvGoombaTripletSpawner, E_MODEL_NONE, 8}, // unused
    {bhvSignOnWall, E_MODEL_NONE, 0},
    {bhvChuckya, E_MODEL_CHUCKYA, 0},
    {bhvCannon, E_MODEL_CANNON_BASE, 0},
    {bhvGoomba, E_MODEL_GOOMBA, 0},
    {bhvHomingAmp, E_MODEL_AMP, 0},
    {bhvCirclingAmp, E_MODEL_AMP, 0},
    {bhvCarrySomething1, (enum ModelExtendedId) MODEL_UNKNOWN_7D, 0}, // unused
    {bhvBetaTrampolineTop, E_MODEL_TRAMPOLINE, 0}, // unused
    {bhvFreeBowlingBall, E_MODEL_BOWLING_BALL, 0}, // unused
    {bhvSnufit, E_MODEL_SNUFIT, 0},
    {bhvRecoveryHeart, E_MODEL_HEART, 0},
    {bhv1upSliding, E_MODEL_1UP, 0},
    {bhv1Up, E_MODEL_1UP, 0},
    {bhv1upJumpOnApproach, E_MODEL_1UP, 0}, // unused
    {bhvHidden1up, E_MODEL_1UP, 0},
    {bhvHidden1upTrigger, E_MODEL_NONE, 0},
    {bhv1Up, E_MODEL_1UP, 1},
    {bhv1Up, E_MODEL_1UP, 2},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvBlueCoinSwitch, E_MODEL_BLUE_COIN_SWITCH, 0},
    {bhvHiddenBlueCoin, E_MODEL_BLUE_COIN, 0},
    {bhvCapSwitch, E_MODEL_CAP_SWITCH, 0}, // unused
    {bhvCapSwitch, E_MODEL_CAP_SWITCH, 1}, // unused
    {bhvCapSwitch, E_MODEL_CAP_SWITCH, 2}, // unused
    {bhvCapSwitch, E_MODEL_CAP_SWITCH, 3}, // unused
    {bhvWaterLevelDiamond, E_MODEL_BREAKABLE_BOX, 0}, // unused
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 0},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 1},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 2},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 3},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 4}, // unused
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 5},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 6},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 7},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 8},
    {bhvBreakableBox, E_MODEL_BREAKABLE_BOX, 0},
    {bhvBreakableBox, E_MODEL_BREAKABLE_BOX, 1},
    {bhvPushableMetalBox, E_MODEL_METAL_BOX, 0},
    {bhvBreakableBoxSmall, E_MODEL_BREAKABLE_BOX_SMALL, 0},
    {bhvFloorSwitchHiddenObjects, E_MODEL_PURPLE_SWITCH, 0},
    {bhvHiddenObject, E_MODEL_BREAKABLE_BOX, 0},
    {bhvHiddenObject, E_MODEL_BREAKABLE_BOX, 1}, // unused
    {bhvHiddenObject, E_MODEL_BREAKABLE_BOX, 2}, // unused
    {bhvBreakableBox, E_MODEL_BREAKABLE_BOX, 3},
    {bhvKoopaShellUnderwater, E_MODEL_KOOPA_SHELL, 0},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 9},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvBulletBill, E_MODEL_BULLET_BILL, 0}, // unused
    {bhvHeaveHo, E_MODEL_HEAVE_HO, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvThwomp2, E_MODEL_THWOMP, 0}, // unused
    {bhvFireSpitter, E_MODEL_BOWLING_BALL, 0},
    {bhvFlyGuy, E_MODEL_FLYGUY, 1},
    {bhvJumpingBox, E_MODEL_BREAKABLE_BOX, 0},
    {bhvTripletButterfly, E_MODEL_BUTTERFLY, 0},
    {bhvTripletButterfly, E_MODEL_BUTTERFLY, 4},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvSmallBully, E_MODEL_BULLY, 0},
    {bhvSmallBully, E_MODEL_BULLY_BOSS, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvStub1D0C, (enum ModelExtendedId) MODEL_UNKNOWN_58, 0}, // unused
    {bhvBouncingFireball, E_MODEL_NONE, 0},
    {bhvFlamethrower, E_MODEL_NONE, 4},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvWoodenPost, E_MODEL_WOODEN_POST, 0},
    {bhvWaterBombSpawner, E_MODEL_NONE, 0},
    {bhvEnemyLakitu, E_MODEL_ENEMY_LAKITU, 0},
    {bhvKoopa, E_MODEL_KOOPA_WITH_SHELL, 2}, // unused
    {bhvKoopaRaceEndpoint, E_MODEL_NONE, 0}, // unused
    {bhvBobomb, E_MODEL_BLACK_BOBOMB, 0},
    {bhvWaterBombCannon, E_MODEL_CANNON_BASE, 0}, // unused
    {bhvBobombBuddyOpensCannon, E_MODEL_BOBOMB_BUDDY, 0}, // unused
    {bhvWaterBombCannon, E_MODEL_CANNON_BASE, 0},
    {bhvBobomb, E_MODEL_BLACK_BOBOMB, 1},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvUnusedFakeStar, (enum ModelExtendedId) MODEL_UNKNOWN_54, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvUnagi, E_MODEL_UNAGI, 0}, // unused
    {bhvSushiShark, E_MODEL_SUSHI, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvStaticObject, E_MODEL_KLEPTO, 0}, // unused
    {bhvTweester, E_MODEL_TWEESTER, 0}, // unused
    {bhvPokey, E_MODEL_NONE, 0},
    {bhvPokey, E_MODEL_NONE, 0}, // unused
    {bhvToxBox, E_MODEL_SSL_TOX_BOX, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvMontyMole, E_MODEL_MONTY_MOLE, 0}, // unused
    {bhvMontyMole, E_MODEL_MONTY_MOLE, 1},
    {bhvMontyMoleHole, E_MODEL_DL_MONTY_MOLE_HOLE, 0},
    {bhvFlyGuy, E_MODEL_FLYGUY, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvWigglerHead, E_MODEL_WIGGLER_HEAD, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvSpindrift, E_MODEL_SPINDRIFT, 0},
    {bhvMrBlizzard, E_MODEL_MR_BLIZZARD_HIDDEN, 0},
    {bhvMrBlizzard, E_MODEL_MR_BLIZZARD_HIDDEN, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvSmallPenguin, E_MODEL_PENGUIN, 0}, // unused
    {bhvTuxiesMother, E_MODEL_PENGUIN, 0}, // unused
    {bhvTuxiesMother, E_MODEL_PENGUIN, 0}, // unused
    {bhvMrBlizzard, E_MODEL_MR_BLIZZARD_HIDDEN, 1}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvHauntedChair, E_MODEL_HAUNTED_CHAIR, 0}, // unused
    {bhvHauntedChair, E_MODEL_HAUNTED_CHAIR, 0},
    {bhvHauntedChair, E_MODEL_HAUNTED_CHAIR, 0}, // unused
    {bhvGhostHuntBoo, E_MODEL_BOO, 0}, // unused
    {bhvGhostHuntBoo, E_MODEL_BOO, 0}, // unused
    {bhvCourtyardBooTriplet, E_MODEL_BOO, 0}, // unused
    {bhvBooWithCage, E_MODEL_BOO, 0}, // unused
    {bhvAlphaBooKey, E_MODEL_BETA_BOO_KEY, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvChirpChirp, E_MODEL_NONE, 0},
    {bhvSeaweedBundle, E_MODEL_NONE, 0},
    {bhvBetaChestBottom, E_MODEL_TREASURE_CHEST_BASE, 0}, // unused
    {bhvBowserBomb, E_MODEL_WATER_MINE, 0}, // unused
    {bhvFishSpawner, E_MODEL_NONE, 2}, // unused
    {bhvFishSpawner, E_MODEL_NONE, 3},
    {bhvJetStreamRingSpawner, E_MODEL_WATER_RING, 0}, // unused
    {bhvJetStreamRingSpawner, E_MODEL_WATER_RING, 0}, // unused
    {bhvSkeeter, E_MODEL_SKEETER, 0},
    {bhvClamShell, E_MODEL_CLAM_SHELL, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvMacroUkiki, E_MODEL_UKIKI, 0}, // unused
    {bhvMacroUkiki, E_MODEL_UKIKI, 1}, // unused
    {bhvPiranhaPlant, E_MODEL_PIRANHA_PLANT, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvSmallWhomp, E_MODEL_WHOMP, 0},
    {bhvChainChomp, E_MODEL_CHAIN_CHOMP, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvKoopa, E_MODEL_KOOPA_WITH_SHELL, 1},
    {bhvKoopa, E_MODEL_KOOPA_WITHOUT_SHELL, 0}, // unused
    {bhvWoodenPost, E_MODEL_WOODEN_POST, 0}, // unused
    {bhvFirePiranhaPlant, E_MODEL_PIRANHA_PLANT, 0},
    {bhvFirePiranhaPlant, E_MODEL_PIRANHA_PLANT, 1}, // unused
    {bhvKoopa, E_MODEL_KOOPA_WITH_SHELL, 4},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvMoneybagHidden, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvSwoop, E_MODEL_SWOOP, 0},
    {bhvSwoop, E_MODEL_SWOOP, 1},
    {bhvMrI, E_MODEL_NONE, 0},
    {bhvScuttlebugSpawn, E_MODEL_NONE, 0},
    {bhvScuttlebug, E_MODEL_SCUTTLEBUG, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, (enum ModelExtendedId) MODEL_UNKNOWN_54, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvTTCRotatingSolid, E_MODEL_TTC_ROTATING_CUBE, 0},
    {bhvTTCRotatingSolid, E_MODEL_TTC_ROTATING_PRISM, 1},
    {bhvTTCPendulum, E_MODEL_TTC_PENDULUM, 0},
    {bhvTTCTreadmill, E_MODEL_TTC_LARGE_TREADMILL, 0},
    {bhvTTCTreadmill, E_MODEL_TTC_SMALL_TREADMILL, 1},
    {bhvTTCMovingBar, E_MODEL_TTC_PUSH_BLOCK, 0},
    {bhvTTCCog, E_MODEL_TTC_ROTATING_HEXAGON, 0},
    {bhvTTCCog, E_MODEL_TTC_ROTATING_TRIANGLE, 2},
    {bhvTTCPitBlock, E_MODEL_TTC_PIT_BLOCK, 0},
    {bhvTTCPitBlock, E_MODEL_TTC_PIT_BLOCK_UNUSED, 1}, // unused
    {bhvTTCElevator, E_MODEL_TTC_ELEVATOR_PLATFORM, 0},
    {bhvTTC2DRotator, E_MODEL_TTC_CLOCK_HAND, 0},
    {bhvTTCSpinner, E_MODEL_TTC_SPINNER, 0},
    {bhvTTC2DRotator, E_MODEL_TTC_SMALL_GEAR, 1},
    {bhvTTC2DRotator, E_MODEL_TTC_LARGE_GEAR, 1},
    {bhvTTCTreadmill, E_MODEL_TTC_LARGE_TREADMILL, 2},
    {bhvTTCTreadmill, E_MODEL_TTC_SMALL_TREADMILL, 3},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 10},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 11},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 12},
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 13}, // unused
    {bhvExclamationBox, E_MODEL_EXCLAMATION_BOX, 14},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvSlidingPlatform2, E_MODEL_BITS_SLIDING_PLATFORM, 0}, // unused
    {bhvSlidingPlatform2, E_MODEL_BITS_TWIN_SLIDING_PLATFORMS, 0}, // unused
    {bhvAnotherTiltingPlatform, E_MODEL_BITDW_SLIDING_PLATFORM, 0}, // unused
    {bhvOctagonalPlatformRotating, E_MODEL_BITS_OCTAGONAL_PLATFORM, 0}, // unused
    {bhvAnimatesOnFloorSwitchPress, E_MODEL_BITS_STAIRCASE, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvFerrisWheelAxle, E_MODEL_BITS_FERRIS_WHEEL_AXLE, 0}, // unused
    {bhvActivatedBackAndForthPlatform, E_MODEL_BITS_ARROW_PLATFORM, 0}, // unused
    {bhvSeesawPlatform, E_MODEL_BITS_SEESAW_PLATFORM, 0}, // unused
    {bhvSeesawPlatform, E_MODEL_BITS_TILTING_W_PLATFORM, 0}, // unused
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0},
    {bhvYellowCoin, E_MODEL_YELLOW_COIN, 0}
};
