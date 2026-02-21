---------------------
----- # Poses # -----
---------------------

-- Register Pose Animations
require('poses/render_pose_1')

-- Ground Poses
photo_mode_pose_register_custom_anim(false, 'Render Pose 1', 'render_pose_1', 1)

---------------------
-- # Face States # --
---------------------

-- Register Eye States
photo_mode_eye_state_reset(CT_MARIO)
photo_mode_eye_state_register(CT_MARIO, 'Open', 0)
photo_mode_eye_state_register(CT_MARIO, 'Half Closed', 1)
photo_mode_eye_state_register(CT_MARIO, 'Closed', 2)
photo_mode_eye_state_register(CT_MARIO, 'Dead', 7)
photo_mode_eye_state_register(CT_MARIO, 'Down', 3)
photo_mode_eye_state_register(CT_MARIO, 'Angry', 4)
photo_mode_eye_state_register(CT_MARIO, 'Happy', 5)
photo_mode_eye_state_register(CT_MARIO, 'Exhuasted', 6)
photo_mode_eye_state_register(CT_MARIO, 'Hurt', 8)

-- Register Mouth States
photo_mode_mouth_state_reset(CT_MARIO)
photo_mode_mouth_state_register(CT_MARIO, 'Default', 0)
photo_mode_mouth_state_register(CT_MARIO, 'Happy', 3)
photo_mode_mouth_state_register(CT_MARIO, 'Angry', 4)
photo_mode_mouth_state_register(CT_MARIO, 'Open', 5)

-- Todo: Utilize face states in normal game
-- Burning: Open Mouth, Dead Eyes
-- Dead: Hurt Eyes, Normal Mouth
-- Tip Toe: Down Eyes, Normal Mouth
-- Punching State: Angry Eyes, Angry Mouth
-- Collect Star: Happy Mouth, Open Eyes
-- ect.