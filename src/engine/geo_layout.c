#include <ultra64.h>
#include "sm64.h"

#include "geo_layout.h"
#include "math_util.h"
#include "game/memory.h"
#include "graph_node.h"
#include "geo_commands.h"
#include "surface_load.h"
#include "game/level_geo.h"
#include "game/screen_transition.h"
#include "game/geo_misc.h"
#include "game/macro_special_objects.h"
#include "pc/debuglog.h"

#define CUR_GRAPH_NODE_LIST_SIZE 32
#define GEO_LAYOUT_STACK_SIZE 32

#define GEO_CMD_FLAGS_RESET 0
#define GEO_CMD_FLAGS_SET   1
#define GEO_CMD_FLAGS_CLEAR 2

#define cur_geo_cmd_u8(offset) \
    (sGeoLayoutCommand[CMD_PROCESS_OFFSET(offset)])

#define cur_geo_cmd_s16(offset) \
    (*(s16 *) &sGeoLayoutCommand[CMD_PROCESS_OFFSET(offset)])

#define cur_geo_cmd_s32(offset) \
    (*(s32 *) &sGeoLayoutCommand[CMD_PROCESS_OFFSET(offset)])

#define cur_geo_cmd_u32(offset) \
    (*(u32 *) &sGeoLayoutCommand[CMD_PROCESS_OFFSET(offset)])

#define cur_geo_cmd_ptr(offset) \
    (*(void **) &sGeoLayoutCommand[CMD_PROCESS_OFFSET(offset)])

typedef void (*GeoLayoutCommandProc)(void);

static GeoLayoutCommandProc GeoLayoutJumpTable[] = {
    geo_layout_cmd_branch_and_link,
    geo_layout_cmd_end,
    geo_layout_cmd_branch,
    geo_layout_cmd_return,
    geo_layout_cmd_open_node,
    geo_layout_cmd_close_node,
    geo_layout_cmd_assign_as_view,
    geo_layout_cmd_update_node_flags,
    geo_layout_cmd_node_root,
    geo_layout_cmd_node_ortho_projection,
    geo_layout_cmd_node_perspective,
    geo_layout_cmd_node_start,
    geo_layout_cmd_node_master_list,
    geo_layout_cmd_node_level_of_detail,
    geo_layout_cmd_node_switch_case,
    geo_layout_cmd_node_camera,
    geo_layout_cmd_node_translation_rotation,
    geo_layout_cmd_node_translation,
    geo_layout_cmd_node_rotation,
    geo_layout_cmd_node_animated_part,
    geo_layout_cmd_node_billboard,
    geo_layout_cmd_node_display_list,
    geo_layout_cmd_node_shadow,
    geo_layout_cmd_node_object_parent,
    geo_layout_cmd_node_generated,
    geo_layout_cmd_node_background,
    geo_layout_cmd_nop,
    geo_layout_cmd_copy_view,
    geo_layout_cmd_node_held_obj,
    geo_layout_cmd_node_scale,
    geo_layout_cmd_nop2,
    geo_layout_cmd_nop3,
    geo_layout_cmd_node_culling_radius,
    // coop
    geo_layout_cmd_node_background_ext,
    geo_layout_cmd_node_switch_case_ext,
    geo_layout_cmd_node_generated_ext,
    geo_layout_cmd_node_bone,
    geo_layout_cmd_node_water_regions,
};

struct GraphNode gObjParentGraphNode;
struct GraphNode *gCurRootGraphNode = NULL;
struct GraphNode *gCurGraphNodeList[CUR_GRAPH_NODE_LIST_SIZE];
s16 gCurGraphNodeIndex;

/* The sGeoViews array is a mysterious one. Some background:
 *
 * If there are e.g. multiple Goombas, the multiple Goomba objects share one
 * Geo node tree describing the goomba 3D model. Since every node has a single
 * parent field and not a parent array, the parent is dynamically rebinded to
 * each goomba instance just before rendering and set to null afterwards.
 * The same happens for ObjectParentNode, which has as his sharedChild a group
 * of all 240 object nodes. Why does the ObjectParentNode exist at all, if its
 * only purpose is to temporarily bind the actual group with objects? This might
 * be another remnant to Luigi.
 *
 * When creating a root node, room for (2 + cmd+0x02) pointers is allocated in
 * sGeoViews. Except for the title screen, cmd+0x02 is 10. The 2 default ones
 * might be for Mario and Luigi, and the other 10 could be different cameras for
 * different rooms / boss fights. An area might be structured like this:
 *
 * geo_camera mode_player //Mario cam
 * geo_open_node
 *   geo_render_obj
 *   geo_assign_as_view 1   // currently unused geo command
 * geo_close_node
 *
 * geo_camera mode_player //Luigi cam
 * geo_open_node
 *   geo_render_obj
 *   geo_copy_view 1        // currently unused geo command
 *   geo_assign_as_view 2
 * geo_close_node
 *
 * geo_camera mode_boss //boss fight cam
 * geo_assign_as_view 3
 * ...
 *
 * There might also be specific geo nodes for Mario or Luigi only. Or a fixed camera
 * might not have display list nodes of parts of the level that are out of view.
 * In the end Luigi got scrapped and the multiple-camera design did not pan out,
 * so everything was reduced to a single ObjectParent with a single group, and
 * camera switching was all done in one node. End of speculation.
 */
static struct GraphNode **sGeoViews;
static u16 sGeoNumViews; // length of sGeoViews array

static struct DynamicPool *sGraphNodePool = NULL;

static uintptr_t sGeoLayoutStack[GEO_LAYOUT_STACK_SIZE];
static s16 sGeoLayoutStackIndex; // similar to SP register in MIPS
static s16 sGeoLayoutReturnIndex; // similar to RA register in MIPS
static u8 *sGeoLayoutCommand;
static const GeoLayout *sGeoLayout;
static bool sIsAreaCommand;

/*
  0x00: Branch and store return address
   cmd+0x04: void *branchTarget
*/
void geo_layout_cmd_branch_and_link(void) {
    if (sGeoLayoutStackIndex >= GEO_LAYOUT_STACK_SIZE - 1) {
        LOG_ERROR("Geo layout stack size reached maximum! Geo layout processing will end.");
        sGeoLayoutCommand = NULL;
        return;
    }

    sGeoLayoutStack[sGeoLayoutStackIndex++] = (uintptr_t) (sGeoLayoutCommand + CMD_PROCESS_OFFSET(8));
    sGeoLayoutStack[sGeoLayoutStackIndex++] = (gCurGraphNodeIndex << 16) + sGeoLayoutReturnIndex;
    sGeoLayoutReturnIndex = sGeoLayoutStackIndex;
    sGeoLayoutCommand = segmented_to_virtual(cur_geo_cmd_ptr(0x04));
}

// 0x01: Terminate geo layout
void geo_layout_cmd_end(void) {
    if (sGeoLayoutStackIndex < 2) {
        LOG_ERROR("Geo layout stack is empty! Geo layout processing will end.");
        sGeoLayoutCommand = NULL;
        return;
    }

    sGeoLayoutStackIndex = sGeoLayoutReturnIndex;
    sGeoLayoutReturnIndex = sGeoLayoutStack[--sGeoLayoutStackIndex] & 0xFFFF;
    gCurGraphNodeIndex = sGeoLayoutStack[sGeoLayoutStackIndex] >> 16;
    sGeoLayoutCommand = (u8 *) sGeoLayoutStack[--sGeoLayoutStackIndex];
}

/*
  0x02: Branch
   cmd+0x04: void *branchTarget
*/
void geo_layout_cmd_branch(void) {
    if (cur_geo_cmd_u8(0x01) == 1) {
        if (sGeoLayoutStackIndex >= GEO_LAYOUT_STACK_SIZE) {
            LOG_ERROR("Geo layout stack size reached maximum! Geo layout processing will end.");
            sGeoLayoutCommand = NULL;
            return;
        }

        sGeoLayoutStack[sGeoLayoutStackIndex++] = (uintptr_t) (sGeoLayoutCommand + CMD_PROCESS_OFFSET(8));
    }

    sGeoLayoutCommand = segmented_to_virtual(cur_geo_cmd_ptr(0x04));
}

// 0x03: Return from branch
void geo_layout_cmd_return(void) {
    if (sGeoLayoutStackIndex < 1) {
        LOG_ERROR("Geo layout stack is empty! Geo layout processing will end.");
        sGeoLayoutCommand = NULL;
        return;
    }

    sGeoLayoutCommand = (u8 *) sGeoLayoutStack[--sGeoLayoutStackIndex];
}

// 0x04: Open node
void geo_layout_cmd_open_node(void) {
    if (gCurGraphNodeIndex >= CUR_GRAPH_NODE_LIST_SIZE) {
        LOG_ERROR("Graph node depth reached maximum! Geo layout processing will end.");
        sGeoLayoutCommand = NULL;
        return;
    }

    if (gCurGraphNodeList[gCurGraphNodeIndex] != NULL) {
        gCurGraphNodeList[gCurGraphNodeIndex + 1] = gCurGraphNodeList[gCurGraphNodeIndex];
        gCurGraphNodeIndex++;
    }
    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

// 0x05: Close node
void geo_layout_cmd_close_node(void) {
    if (gCurGraphNodeIndex > 0) {
        gCurGraphNodeIndex--;
    }
    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x06: Register the current node as a view
   cmd+0x02: index

  Register the current node in the sGeoViews array at the given index
*/
void geo_layout_cmd_assign_as_view(void) {
    if (sIsAreaCommand) {
        u16 index = cur_geo_cmd_s16(0x02);

        if (index < sGeoNumViews) {
            sGeoViews[index] = gCurGraphNodeList[gCurGraphNodeIndex];
        }
    }

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x07: Update current scene graph node flags
   cmd+0x01: u8 operation (0 = reset, 1 = set, 2 = clear)
   cmd+0x02: s16 bits
*/
void geo_layout_cmd_update_node_flags(void) {
    u16 operation = cur_geo_cmd_u8(0x01);
    u16 flagBits = cur_geo_cmd_s16(0x02);

    switch (operation) {
        case GEO_CMD_FLAGS_RESET:
            gCurGraphNodeList[gCurGraphNodeIndex]->flags = flagBits;
            break;
        case GEO_CMD_FLAGS_SET:
            gCurGraphNodeList[gCurGraphNodeIndex]->flags |= flagBits;
            break;
        case GEO_CMD_FLAGS_CLEAR:
            gCurGraphNodeList[gCurGraphNodeIndex]->flags &= ~flagBits;
            break;
    }

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x08: Create a scene graph root node that specifies the viewport
   cmd+0x02: s16 num entries (+2) to allocate for sGeoViews
   cmd+0x04: s16 x
   cmd+0x06: s16 y
   cmd+0x08: s16 width
   cmd+0x0A: s16 height
*/
void geo_layout_cmd_node_root(void) {
    if (sIsAreaCommand) {
        struct GraphNodeRoot *graphNode;

        s16 x = cur_geo_cmd_s16(0x04);
        s16 y = cur_geo_cmd_s16(0x06);
        s16 width = cur_geo_cmd_s16(0x08);
        s16 height = cur_geo_cmd_s16(0x0A);

        // number of entries to allocate for sGeoViews array
        // at least 2 are allocated by default
        // cmd+0x02 = 0x00: Mario face, 0x0A: all other levels
        sGeoNumViews = cur_geo_cmd_s16(0x02) + 2;

        graphNode = init_graph_node_root(sGraphNodePool, NULL, 0, x, y, width, height);

        // TODO: check type
        sGeoViews = dynamic_pool_alloc(sGraphNodePool, sGeoNumViews * sizeof(struct GraphNode *));

        graphNode->views = sGeoViews;
        graphNode->numViews = sGeoNumViews;

        for (s32 i = 0; i < sGeoNumViews; i++) {
            sGeoViews[i] = NULL;
        }

        register_scene_graph_node(&graphNode->node);

    } else {
        // Graph node still needs a root
        struct GraphNodeStart *graphNode;

        graphNode = init_graph_node_start(sGraphNodePool, NULL);

        register_scene_graph_node(&graphNode->node);
    }

    sGeoLayoutCommand += 0x0C << CMD_SIZE_SHIFT;
}

/*
  0x09: Create orthographic projection scene graph node
   cmd+0x02: s16 scale as a percentage (usually it's 100)
*/
void geo_layout_cmd_node_ortho_projection(void) {
    if (sIsAreaCommand) {
        struct GraphNodeOrthoProjection *graphNode;
        f32 scale = (f32) cur_geo_cmd_s16(0x02) / 100.0f;

        graphNode = init_graph_node_ortho_projection(sGraphNodePool, NULL, scale);

        register_scene_graph_node(&graphNode->node);
    }

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x0A: Create camera frustum scene graph node
   cmd+0x01: u8  if nonzero, enable frustumFunc field
   cmd+0x02: s16 field of view
   cmd+0x04: s16 near
   cmd+0x06: s16 far
   [cmd+0x08: GraphNodeFunc frustumFunc]
*/
void geo_layout_cmd_node_perspective(void) {
    GraphNodeFunc frustumFunc = NULL;
    s16 fov = cur_geo_cmd_s16(0x02);
    s16 near = cur_geo_cmd_s16(0x04);
    s16 far = cur_geo_cmd_s16(0x06);

    if (cur_geo_cmd_u8(0x01) != 0) {
        // optional asm function
        frustumFunc = (GraphNodeFunc) cur_geo_cmd_ptr(0x08);
        sGeoLayoutCommand += 4 << CMD_SIZE_SHIFT;
    }

    if (sIsAreaCommand) {
        struct GraphNodePerspective *graphNode;

        graphNode = init_graph_node_perspective(sGraphNodePool, NULL, (f32) fov, near, far, frustumFunc, 0);

        register_scene_graph_node(&graphNode->fnNode.node);
    }

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x0B: Create a scene graph node that groups other nodes without any
  additional functionality
*/
void geo_layout_cmd_node_start(void) {
    struct GraphNodeStart *graphNode;

    graphNode = init_graph_node_start(sGraphNodePool, NULL);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

// 0x1F: No operation
void geo_layout_cmd_nop3(void) {
    sGeoLayoutCommand += 0x10 << CMD_SIZE_SHIFT;
}

/*
  0x0C: Create zbuffer-toggling scene graph node
   cmd+0x01: u8 enableZBuffer (1 = on, 0 = off)
*/
void geo_layout_cmd_node_master_list(void) {
    if (sIsAreaCommand) {
        struct GraphNodeMasterList *graphNode;

        graphNode = init_graph_node_master_list(sGraphNodePool, NULL, cur_geo_cmd_u8(0x01));

        register_scene_graph_node(&graphNode->node);
    }

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x0D: Create a level of detail graph node, which only renders at a certain
  distance interval from the camera.
   cmd+0x04: s16 minDistance
   cmd+0x06: s16 maxDistance
*/
void geo_layout_cmd_node_level_of_detail(void) {
    struct GraphNodeLevelOfDetail *graphNode;
    s16 minDistance = cur_geo_cmd_s16(0x04);
    s16 maxDistance = cur_geo_cmd_s16(0x06);

    graphNode = init_graph_node_render_range(sGraphNodePool, NULL, minDistance, maxDistance);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x0E: Create switch-case scene graph node
   cmd+0x02: s16 initialSelectedCase
   cmd+0x04: GraphNodeFunc caseSelectorFunc

  caseSelectorFunc returns an index which is used to select the child node to render.
  Used for animating coins, blinking, color selection, etc.
*/
void geo_layout_cmd_node_switch_case(void) {
    GraphNodeFunc graphNodeFunc = (GraphNodeFunc) cur_geo_cmd_ptr(0x04);

    if (sIsAreaCommand || graphNodeFunc != geo_switch_area) {
        struct GraphNodeSwitchCase *graphNode;

        graphNode = init_graph_node_switch_case(sGraphNodePool, NULL, cur_geo_cmd_s16(0x02), 0, graphNodeFunc, 0);

        register_scene_graph_node(&graphNode->fnNode.node);
    }

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
 0x0F: Create a camera scene graph node (GraphNodeCamera). The focus sets the Camera's areaCen position.
  cmd+0x02: s16 camera type (changes from course to course)
  cmd+0x04: s16 posX
  cmd+0x06: s16 posY
  cmd+0x08: s16 posZ
  cmd+0x0A: s16 focusX
  cmd+0x0C: s16 focusY
  cmd+0x0E: s16 focusZ
  cmd+0x10: GraphNodeFunc func
*/
void geo_layout_cmd_node_camera(void) {
    if (sIsAreaCommand) {
        struct GraphNodeCamera *graphNode;
        s16 *cmdPos = (s16 *) &sGeoLayoutCommand[4];

        Vec3f pos, focus;

        cmdPos = read_vec3s_to_vec3f(pos, cmdPos);
        cmdPos = read_vec3s_to_vec3f(focus, cmdPos);

        graphNode = init_graph_node_camera(sGraphNodePool, NULL, pos, focus, (GraphNodeFunc) cur_geo_cmd_ptr(0x10), cur_geo_cmd_s16(0x02));

        register_scene_graph_node(&graphNode->fnNode.node);

        sGeoViews[0] = &graphNode->fnNode.node;
    }

    sGeoLayoutCommand += 0x14 << CMD_SIZE_SHIFT;
}

/*
  0x10: Create translation & rotation scene graph node with optional display list
   cmd+0x01: u8 params
     (params & 0x80): if set, enable displayList field and drawingLayer
     ((params & 0x70)>>4): fieldLayout
     (params & 0x0F): drawingLayer

   fieldLayout == 0:
    cmd+0x04: s16 xTranslation
    cmd+0x06: s16 yTranslation
    cmd+0x08: s16 zTranslation
    cmd+0x0A: s16 xRotation
    cmd+0x0C: s16 yRotation
    cmd+0x0E: s16 zRotation

   fieldLayout == 1:
    cmd+0x02: s16 xTranslation
    cmd+0x04: s16 yTranslation
    cmd+0x06: s16 zTranslation
    (rotation gets copied from gVec3sZero)

   fieldLayout == 2:
    cmd+0x02: s16 xRotation
    cmd+0x04: s16 yRotation
    cmd+0x06: s16 zRotation
    (translation gets copied from gVec3sZero)

   fieldLayout == 3:
    cmd+0x02: s16 yRotation
    (translation gets copied from gVec3sZero)
    (x and z translation are set to 0)

   [cmd+var: void *displayList]
*/
void geo_layout_cmd_node_translation_rotation(void) {
    struct GraphNodeTranslationRotation *graphNode;

    Vec3s translation, rotation;

    void *displayList = NULL;
    s16 drawingLayer = 0;

    s16 params = cur_geo_cmd_u8(0x01);
    s16 *cmdPos = (s16 *) sGeoLayoutCommand;

    switch ((params & 0x70) >> 4) {
        case 0:
            cmdPos = read_vec3s(translation, &cmdPos[2]);
            cmdPos = read_vec3s_angle(rotation, cmdPos);
            break;
        case 1:
            cmdPos = read_vec3s(translation, &cmdPos[1]);
            vec3s_copy(rotation, gVec3sZero);
            break;
        case 2:
            cmdPos = read_vec3s_angle(rotation, &cmdPos[1]);
            vec3s_copy(translation, gVec3sZero);
            break;
        case 3:
            vec3s_copy(translation, gVec3sZero);
            vec3s_set(rotation, 0, (cmdPos[1] << 15) / 180, 0);
            cmdPos += 2 << CMD_SIZE_SHIFT;
            break;
    }

    if (params & 0x80) {
        displayList = *(void **) &cmdPos[0];
        drawingLayer = params & 0x0F;
        cmdPos += 2 << CMD_SIZE_SHIFT;
    }

    graphNode = init_graph_node_translation_rotation(sGraphNodePool, NULL, drawingLayer, displayList,
                                                     translation, rotation);
    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand = (u8 *) cmdPos;
}

/*
  0x11: Create translation scene graph node with optional display list
   cmd+0x01: u8 params
     (params & 0x80): if set, enable displayList field and drawingLayer
     (params & 0x0F): drawingLayer
   cmd+0x02: s16 xTranslation
   cmd+0x04: s16 yTranslation
   cmd+0x06: s16 zTranslation
  [cmd+0x08: void *displayList]
*/
void geo_layout_cmd_node_translation(void) {
    struct GraphNodeTranslation *graphNode;

    Vec3s translation;

    s16 drawingLayer = 0;
    s16 params = cur_geo_cmd_u8(0x01);
    s16 *cmdPos = (s16 *) sGeoLayoutCommand;
    void *displayList = NULL;

    cmdPos = read_vec3s(translation, &cmdPos[1]);

    if (params & 0x80) {
        displayList = *(void **) &cmdPos[0];
        drawingLayer = params & 0x0F;
        cmdPos += 2 << CMD_SIZE_SHIFT;
    }

    graphNode =
        init_graph_node_translation(sGraphNodePool, NULL, drawingLayer, displayList, translation);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand = (u8 *) cmdPos;
}

/*
  0x12: Create ? scene graph node
   cmd+0x01: u8 params
     (params & 0x80): if set, enable displayList field and drawingLayer
     (params & 0x0F): drawingLayer
   cmd+0x02: s16 unkX
   cmd+0x04: s16 unkY
   cmd+0x06: s16 unkZ
  [cmd+0x08: void *displayList]
*/
void geo_layout_cmd_node_rotation(void) {
    struct GraphNodeRotation *graphNode;

    Vec3s sp2c;

    s16 drawingLayer = 0;
    s16 params = cur_geo_cmd_u8(0x01);
    s16 *cmdPos = (s16 *) sGeoLayoutCommand;
    void *displayList = NULL;

    cmdPos = read_vec3s_angle(sp2c, &cmdPos[1]);

    if (params & 0x80) {
        displayList = *(void **) &cmdPos[0];
        drawingLayer = params & 0x0F;
        cmdPos += 2 << CMD_SIZE_SHIFT;
    }

    graphNode = init_graph_node_rotation(sGraphNodePool, NULL, drawingLayer, displayList, sp2c);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand = (u8 *) cmdPos;
}

/*
  0x1D: Create scale scene graph node with optional display list
   cmd+0x01: u8 params
     (params & 0x80): if set, enable displayList field and drawingLayer
     (params & 0x40): if set, enable scale XYZ
     (params & 0x0F): drawingLayer
   cmd+0x04: u32 scale (0x10000 = 1.0)
     or
   cmd+0x04: u32 scale X (0x10000 = 1.0)
   cmd+0x08: u32 scale Y (0x10000 = 1.0)
   cmd+0x0C: u32 scale Z (0x10000 = 1.0)
  [cmd+0x08/0x10: void *displayList]
*/
void geo_layout_cmd_node_scale(void) {
    s16 drawingLayer = 0;
    s16 params = cur_geo_cmd_u8(0x01);
    Vec3f scale;
    void *displayList = NULL;
    bool isScaleXYZ = (params & 0x40) != 0;

    if (isScaleXYZ) {
        scale[0] = cur_geo_cmd_u32(0x04) / 65536.0f;
        scale[1] = cur_geo_cmd_u32(0x08) / 65536.0f;
        scale[2] = cur_geo_cmd_u32(0x0C) / 65536.0f;
        sGeoLayoutCommand += 0x10 << CMD_SIZE_SHIFT;
    } else {
        scale[0] = cur_geo_cmd_u32(0x04) / 65536.0f;
        sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
    }

    if (params & 0x80) {
        displayList = cur_geo_cmd_ptr(0x00);
        drawingLayer = params & 0x0F;
        sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
    }

    struct GraphNode *graphNode = (
        isScaleXYZ ?
        (struct GraphNode *) init_graph_node_scale_xyz(sGraphNodePool, NULL, drawingLayer, displayList, scale) :
        (struct GraphNode *) init_graph_node_scale(sGraphNodePool, NULL, drawingLayer, displayList, scale[0])
    );

    register_scene_graph_node(graphNode);
}

// 0x1E: No operation
void geo_layout_cmd_nop2(void) {
    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x13: Create a scene graph node that is rotated by the object's animation.
   cmd+0x01: u8 drawingLayer
   cmd+0x02: s16 xTranslation
   cmd+0x04: s16 yTranslation
   cmd+0x06: s16 zTranslation
   cmd+0x08: void *displayList
*/
void geo_layout_cmd_node_animated_part(void) {
    struct GraphNodeAnimatedPart *graphNode;
    Vec3s translation;
    s32 drawingLayer = cur_geo_cmd_u8(0x01);
    void *displayList = cur_geo_cmd_ptr(0x08);
    s16 *cmdPos = (s16 *) sGeoLayoutCommand;

    read_vec3s(translation, &cmdPos[1]);

    graphNode =
        init_graph_node_animated_part(sGraphNodePool, NULL, drawingLayer, displayList, translation);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand += 0x0C << CMD_SIZE_SHIFT;
}

/*
  0x14: Create billboarding node with optional display list
   cmd+0x01: u8 params
     (params & 0x80): if set, enable displayList field and drawingLayer
     (params & 0x0F): drawingLayer
   cmd+0x02: s16 xTranslation
   cmd+0x04: s16 yTranslation
   cmd+0x06: s16 zTranslation
  [cmd+0x08: void *displayList]
*/
void geo_layout_cmd_node_billboard(void) {
    struct GraphNodeBillboard *graphNode;
    Vec3s translation;
    s16 drawingLayer = 0;
    s16 params = cur_geo_cmd_u8(0x01);
    s16 *cmdPos = (s16 *) sGeoLayoutCommand;
    void *displayList = NULL;

    cmdPos = read_vec3s(translation, &cmdPos[1]);

    if (params & 0x80) {
        displayList = *(void **) &cmdPos[0];
        drawingLayer = params & 0x0F;
        cmdPos += 2 << CMD_SIZE_SHIFT;
    }

    graphNode = init_graph_node_billboard(sGraphNodePool, NULL, drawingLayer, displayList, translation);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand = (u8 *) cmdPos;
}

/*
  0x15: Create plain display list scene graph node
   cmd+0x01: u8 drawingLayer
   cmd+0x04: void *displayList
*/
void geo_layout_cmd_node_display_list(void) {
    struct GraphNodeDisplayList *graphNode;
    s32 drawingLayer = cur_geo_cmd_u8(0x01);
    void *displayList = cur_geo_cmd_ptr(0x04);

    graphNode = init_graph_node_display_list(sGraphNodePool, NULL, drawingLayer, displayList);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x16: Create shadow scene graph node
   cmd+0x02: s16 shadowType
   cmd+0x04: s16 shadowSolidity
   cmd+0x06: s16 shadowScale
*/
void geo_layout_cmd_node_shadow(void) {
    struct GraphNodeShadow *graphNode;
    u8 shadowType = cur_geo_cmd_s16(0x02);
    u8 shadowSolidity = cur_geo_cmd_s16(0x04);
    s16 shadowScale = cur_geo_cmd_s16(0x06);

    graphNode = init_graph_node_shadow(sGraphNodePool, NULL, shadowScale, shadowSolidity, shadowType);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

// 0x17: Create scene graph node that manages the group of all object nodes
void geo_layout_cmd_node_object_parent(void) {
    if (sIsAreaCommand) {
        struct GraphNodeObjectParent *graphNode;

        graphNode = init_graph_node_object_parent(sGraphNodePool, NULL, &gObjParentGraphNode);

        register_scene_graph_node(&graphNode->node);
    }

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x18: Create dynamically generated displaylist scene graph node
   cmd+0x02: s16 parameter
   cmd+0x04: GraphNodeFunc func
*/
void geo_layout_cmd_node_generated(void) {
    GraphNodeFunc graphNodeFunc = (GraphNodeFunc) cur_geo_cmd_ptr(0x04);

    if (sIsAreaCommand || (
        graphNodeFunc != (GraphNodeFunc) geo_envfx_main &&
        graphNodeFunc != (GraphNodeFunc) geo_skybox_main &&
        graphNodeFunc != (GraphNodeFunc) geo_cannon_circle_base &&
        graphNodeFunc != (GraphNodeFunc) geo_render_mirror_mario &&
        graphNodeFunc != (GraphNodeFunc) geo_wdw_set_initial_water_level &&
        graphNodeFunc != (GraphNodeFunc) geo_exec_flying_carpet_timer_update
        // TODO MODELS: remove if it doesn't cause any issue
        // graphNodeFunc != (GraphNodeFunc) geo_painting_update &&
        // graphNodeFunc != (GraphNodeFunc) geo_painting_draw &&
        // graphNodeFunc != (GraphNodeFunc) geo_movtex_draw_colored_no_update &&
        // graphNodeFunc != (GraphNodeFunc) geo_movtex_draw_nocolor &&
    )) {
        // Try to create a water regions node if `geo_movtex_draw_water_regions` is detected and it's not an area layout
        if (!sIsAreaCommand && (
            graphNodeFunc == (GraphNodeFunc) geo_movtex_draw_water_regions ||
            graphNodeFunc == (GraphNodeFunc) geo_movtex_draw_water_regions_ext
        )) {
            geo_layout_cmd_node_water_regions();
        }

        struct GraphNodeGenerated *graphNode;

        graphNode = init_graph_node_generated(sGraphNodePool, NULL, graphNodeFunc, cur_geo_cmd_s16(0x02));

        register_scene_graph_node(&graphNode->fnNode.node);
    }

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x19: Create background scene graph node
   cmd+0x02: s16 background // background ID, or RGBA5551 color if backgroundFunc is null
   cmd+0x04: GraphNodeFunc backgroundFunc
*/
void geo_layout_cmd_node_background(void) {
    if (sIsAreaCommand) {
        struct GraphNodeBackground *graphNode;
        s16 backgroundIdOrColor = cur_geo_cmd_s16(0x02);
        graphNode = init_graph_node_background(
            sGraphNodePool, NULL,
            backgroundIdOrColor, // background ID, or RGBA5551 color if asm function is null
            (GraphNodeFunc) cur_geo_cmd_ptr(0x04), // asm function
            0);

        register_scene_graph_node(&graphNode->fnNode.node);
    }

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

// 0x1A: No operation
void geo_layout_cmd_nop(void) {
    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x1B: Copy the shared children from the object parent from a specific view
  to a newly created object parent node.
   cmd+0x02: s16 index (of sGeoViews)
*/
void geo_layout_cmd_copy_view(void) {
    if (sIsAreaCommand) {
        struct GraphNodeObjectParent *graphNode;
        struct GraphNode *node = NULL;
        s16 index = cur_geo_cmd_s16(0x02);

        if (index >= 0) {
            node = sGeoViews[index];

            if (node->type == GRAPH_NODE_TYPE_OBJECT_PARENT) {
                node = ((struct GraphNodeObjectParent *) node)->sharedChild;
            } else {
                node = NULL;
            }
        }

        graphNode = init_graph_node_object_parent(sGraphNodePool, NULL, node);

        register_scene_graph_node(&graphNode->node);
    }

    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x1C: Create a held object scene graph node
   cmd+0x01: u8 unused
   cmd+0x02: s16 offsetX
   cmd+0x04: s16 offsetY
   cmd+0x06: s16 offsetZ
   cmd+0x08: GraphNodeFunc nodeFunc
*/
void geo_layout_cmd_node_held_obj(void) {
    struct GraphNodeHeldObject *graphNode;
    Vec3s offset;

    read_vec3s(offset, (s16 *) &sGeoLayoutCommand[0x02]);

    graphNode = init_graph_node_held_object(
        sGraphNodePool, NULL, NULL, offset, (GraphNodeFunc) cur_geo_cmd_ptr(0x08), cur_geo_cmd_u8(0x01));

    register_scene_graph_node(&graphNode->fnNode.node);

    sGeoLayoutCommand += 0x0C << CMD_SIZE_SHIFT;
}

/*
  0x20: Create a scene graph node that specifies for an object the radius that
   is used for frustum culling.
   cmd+0x02: s16 cullingRadius
*/
void geo_layout_cmd_node_culling_radius(void) {
    struct GraphNodeCullingRadius *graphNode;
    graphNode = init_graph_node_culling_radius(sGraphNodePool, NULL, cur_geo_cmd_s16(0x02));
    register_scene_graph_node(&graphNode->node);
    sGeoLayoutCommand += 0x04 << CMD_SIZE_SHIFT;
}

/*
  0x21: Create custom background scene graph node
*/
void geo_layout_cmd_node_background_ext(void) {
    if (sIsAreaCommand) {
        struct GraphNodeBackground *graphNode;

        void* bgPtr = cur_geo_cmd_ptr(0x04);
        dynos_level_load_background(bgPtr);

        graphNode = init_graph_node_background(
            sGraphNodePool, NULL,
            BACKGROUND_CUSTOM, // background ID, or RGBA5551 color if asm function is null
            (GraphNodeFunc) cur_geo_cmd_ptr(0x08), // asm function
            1);

        register_scene_graph_node(&graphNode->fnNode.node);
    }

    sGeoLayoutCommand += 0x0C << CMD_SIZE_SHIFT;
}

/*
  0x22: Create switch-case scene graph node with a custom Lua callback
*/
void geo_layout_cmd_node_switch_case_ext(void) {
    struct GraphNodeSwitchCase *graphNode;

    graphNode = init_graph_node_switch_case(
        sGraphNodePool, NULL,
        cur_geo_cmd_s16(0x02), // parameter used by switch func
        0,
        (GraphNodeFunc) geo_process_lua_function,
        0
    );
    graphNode->fnNode.luaTokenIndex = cur_geo_cmd_u32(0x04);

    register_scene_graph_node(&graphNode->fnNode.node);

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x23: Create dynamically generated displaylist scene graph node with a custom Lua callback
*/
void geo_layout_cmd_node_generated_ext(void) {
    struct GraphNodeGenerated *graphNode;

    graphNode = init_graph_node_generated(
        sGraphNodePool, NULL,
        (GraphNodeFunc) geo_process_lua_function,
        cur_geo_cmd_s16(0x02) // parameter
    );
    graphNode->fnNode.luaTokenIndex = cur_geo_cmd_u32(0x04);

    register_scene_graph_node(&graphNode->fnNode.node);

    sGeoLayoutCommand += 0x08 << CMD_SIZE_SHIFT;
}

/*
  0x24: Create a scene graph node that is rotated by the object's animation + an initial rotation.
*/
void geo_layout_cmd_node_bone(void) {
    struct GraphNodeBone *graphNode;
    Vec3s translation;
    Vec3s rotation;
    s32 params = cur_geo_cmd_u8(0x01);
    s32 drawingLayer = params;
    Vec3f scale;
    vec3f_copy(scale, gVec3fOne);

    void *displayList;
    s16 *cmdPos = (s16 *) sGeoLayoutCommand;

    cmdPos = read_vec3s(translation, &cmdPos[2]);
    cmdPos = read_vec3s(rotation, &cmdPos[0]);
    if (params & 0x80) {
        drawingLayer &= 0x0F;

        vec3f_set(scale,
            cur_geo_cmd_u32(0x10) / 65536.0f,
            cur_geo_cmd_u32(0x14) / 65536.0f,
            cur_geo_cmd_u32(0x18) / 65536.0f
        );
        cmdPos += 6 << CMD_SIZE_SHIFT;
    }
    displayList = *(void **) &cmdPos[0];
    cmdPos += 2 << CMD_SIZE_SHIFT;

    graphNode = init_graph_node_bone(
        sGraphNodePool, NULL,
        drawingLayer, displayList,
        translation, rotation,
        scale);

    register_scene_graph_node(&graphNode->node);

    sGeoLayoutCommand = (u8 *) cmdPos;
}

//
// Find water regions and fill `regions` with the water boxes data.
// This is tricky.
// From a geo layout, the goal is to find the water boxes that are normally used in `geo_movtex_draw_water_regions`.
// For that, we need to find in which level and area this geo layout is used as an area layout.
// Then, find the collision associated to that area.
// And finally, parse the collision to retrieve the water boxes.
//

static struct WaterRegion *sRegions;
static s16 sNumRegions;
static bool sAreaFound;

static bool find_water_regions_in_collision(Collision *data) {
    while (true) {
        s16 terrainLoadType = *data++;
        switch (terrainLoadType) {
            case TERRAIN_LOAD_VERTICES: {
                s16 numVertices = *data++;
                data += 3 * numVertices;
            } break;

            case TERRAIN_LOAD_OBJECTS: {
                data += get_special_objects_size(data);
            } break;

            case TERRAIN_LOAD_ENVIRONMENT: {
                sNumRegions = *data++;
                sNumRegions = min(sNumRegions, MAX_WATER_REGIONS);
                memcpy(sRegions, data, sizeof(*sRegions) * sNumRegions);
            } return true;

            case TERRAIN_LOAD_CONTINUE: {
            } continue;

            case TERRAIN_LOAD_END: {
            } return false;

            default: {
                s16 numSurfaces = *data++;
                data += (3 + surface_has_force(terrainLoadType)) * numSurfaces;
            } break;
        }
    }
}

static s32 find_water_regions_in_level_script(u8 type, void *cmd) {
    switch (type) {

        // AREA
        case 0x1F: {
            const GeoLayout *geoLayout = (const GeoLayout *) dynos_level_cmd_get(cmd, 4);
            if (geoLayout == sGeoLayout) {
                sAreaFound = true;
            }
        } break;

        // TERRAIN
        case 0x2E: {
            if (sAreaFound) {
                Collision *data = (Collision *) dynos_level_cmd_get(cmd, 4);
                if (find_water_regions_in_collision(data)) {
                    return 3; // Stop parsing
                }
                sAreaFound = false;
            }
        } break;
    }
    return 0;
}

static s16 find_water_regions(struct WaterRegion *regions) {
    sNumRegions = -1;
    sRegions = regions;
    sAreaFound = false;

    // Vanilla levels
    extern const LevelScript level_wf_entry[];
    extern const LevelScript level_jrb_entry[];
    extern const LevelScript level_ccm_entry[];
    extern const LevelScript level_bbh_entry[];
    extern const LevelScript level_hmc_entry[];
    extern const LevelScript level_lll_entry[];
    extern const LevelScript level_ssl_entry[];
    extern const LevelScript level_ddd_entry[];
    extern const LevelScript level_sl_entry[];
    extern const LevelScript level_wdw_entry[];
    extern const LevelScript level_ttm_entry[];
    extern const LevelScript level_thi_entry[];
    extern const LevelScript level_castle_grounds_entry[];
    extern const LevelScript level_castle_inside_entry[];
    extern const LevelScript level_castle_courtyard_entry[];
    static const LevelScript *sVanillaLevelScriptsWithWaterRegions[] = {
        level_wf_entry, // WF
        level_jrb_entry, // JRB
        level_ccm_entry, // CCM
        level_bbh_entry, // BBH
        level_hmc_entry, // HMC
        level_lll_entry, // LLL
        level_ssl_entry, // SSL
        level_ddd_entry, // DDD
        level_sl_entry, // SL
        level_wdw_entry, // WDW
        level_ttm_entry, // TTM
        level_thi_entry, // THI
        level_castle_grounds_entry, // Castle grounds
        level_castle_inside_entry, // Castle inside
        level_castle_courtyard_entry, // Castle courtyard
    };
    for (s32 i = 0; i < ARRAY_COUNT(sVanillaLevelScriptsWithWaterRegions); ++i) {
        dynos_level_parse_script(sVanillaLevelScriptsWithWaterRegions[i], find_water_regions_in_level_script);
        if (sAreaFound) {
            return sNumRegions;
        }
    }

    // Custom levels
    for (u32 i = 0; i < dynos_level_get_array_count(); ++i) {
        dynos_level_parse_script(dynos_level_get_array_script(i), find_water_regions_in_level_script);
        if (sAreaFound) {
            return sNumRegions;
        }
    }

    return sNumRegions;
}

/*
  Not a real command.
  Called by `geo_layout_cmd_node_generated` if `geo_movtex_draw_water_regions` is detected.
*/
void geo_layout_cmd_node_water_regions(void) {
    if (!sIsAreaCommand) {
        struct WaterRegion regions[MAX_WATER_REGIONS] = {0};
        s16 numRegions = find_water_regions(regions);
        if (numRegions <= 0) {
            return;
        }

        struct GraphNodeWaterRegions *graphNode;

        graphNode = init_graph_node_water_regions(sGraphNodePool, NULL, numRegions, regions);

        register_scene_graph_node(&graphNode->node);
    }
}

struct GraphNode *process_geo_layout(struct DynamicPool *pool, const GeoLayout *geoLayout, bool isAreaCommand) {
    // set by register_scene_graph_node when gCurGraphNodeIndex is 0
    // and gCurRootGraphNode is NULL
    gCurRootGraphNode = NULL;

    sGeoNumViews = 0; // number of entries in sGeoViews

    memset(gCurGraphNodeList, 0, sizeof(gCurGraphNodeList));
    gCurGraphNodeIndex = 0; // incremented by cmd_open_node, decremented by cmd_close_node

    memset(sGeoLayoutStack, 0, sizeof(sGeoLayoutStack));
    sGeoLayoutStackIndex = 2;
    sGeoLayoutReturnIndex = 2; // stack index is often copied here?

    sGeoLayout = geoLayout;
    sGeoLayoutCommand = (u8 *) geoLayout;
    sIsAreaCommand = isAreaCommand;

    sGraphNodePool = pool;

    while (sGeoLayoutCommand != NULL) {
        GeoLayoutJumpTable[sGeoLayoutCommand[0x00]]();
    }

    if (gCurRootGraphNode) {
        gCurRootGraphNode->georef = (const void *) geoLayout;
    }
    return gCurRootGraphNode;
}
