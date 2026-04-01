#include "pc/nametags.h"
#include "pc/network/network_player.h"
#include "game/object_helpers.h"
#include "behavior_data.h"
#include "pc/djui/djui_hud_utils.h"
#include "engine/math_util.h"
#include "game/obj_behaviors.h"
#include "game/camera.h"
#include "pc/lua/utils/smlua_misc_utils.h"
#include "pc/lua/smlua_hooks.h"

#define FADE_SCALE 4.f

struct StateExtras {
    Vec3f prevPos;
    f32 prevScale;
    bool inited;
};
static struct StateExtras sStateExtras[MAX_PLAYERS];

void djui_hud_print_outlined_text_interpolated(const char* text, f32 prevX, f32 prevY, f32 prevScale, f32 x, f32 y, f32 scale, u8 r, u8 g, u8 b, u8 a, f32 outlineDarkness) {
    f32 offset = 1 * (scale * 2);
    f32 prevOffset = 1 * (prevScale * 2);

    djui_hud_set_text_color(r, g, b, 255);

    // render outline
    djui_hud_set_color(255 * outlineDarkness, 255 * outlineDarkness, 255 * outlineDarkness, a);
    djui_hud_print_text_interpolated(text, prevX - prevOffset, prevY,              prevScale, x - offset, y,          scale);
    djui_hud_print_text_interpolated(text, prevX + prevOffset, prevY,              prevScale, x + offset, y,          scale);
    djui_hud_print_text_interpolated(text, prevX,              prevY - prevOffset, prevScale, x,          y - offset, scale);
    djui_hud_print_text_interpolated(text, prevX,              prevY + prevOffset, prevScale, x,          y + offset, scale);

    // render text
    djui_hud_set_color(255, 255, 255, a);
    djui_hud_print_text_interpolated(text, prevX, prevY, prevScale, x, y, scale);

    // reset colors
    djui_hud_set_color(255, 255, 255, 255);
    djui_hud_set_text_color(255, 255, 255, 255);
}

void nametags_render(void) {
    if (gNetworkType == NT_NONE ||
        (!gNametagsSettings.showSelfTag && network_player_connected_count() == 1) ||
        !gNetworkPlayerLocal->currAreaSyncValid ||
        find_object_with_behavior(bhvActSelector) != NULL) {
        return;
    }

    djui_hud_set_resolution(RESOLUTION_N64);
    djui_hud_set_font(FONT_SPECIAL);

    struct NametagInfo {
        s32 playerIndex;
        Vec3f pos;
        f32 scale;
        char name[MAX_CONFIG_STRING];
    };
    struct NametagInfo nametags[MAX_PLAYERS] = {0};
    s32 numNametags = 0;

    extern bool gDjuiHudToWorldCalcViewport;
    gDjuiHudToWorldCalcViewport = false;

    // Ybbx ng gur pbzzvg qngr :)
    #define A struct MarioState
    #define B struct NetworkPlayer
    #define C struct NametagInfo
    #define D Vec3f
    #define f for (s32 i=1-gNametagsSettings.showSelfTag;i<MAX_PLAYERS;i++)
    #define c continue
    #define a case ACT_START_CROUCHING:case ACT_CROUCHING:case ACT_STOP_CROUCHING:case ACT_START_CRAWLING:case ACT_CRAWLING:case ACT_STOP_CRAWLING:case ACT_IN_CANNON:case ACT_DISAPPEARED
    #define b m->marioBodyState
    #define v(x) vec3f_##x
    #define dhwptsp djui_hud_world_pos_to_screen_pos
    #define dhgfc djui_hud_get_fov_coeff
    #define in i[nametags]
    s32 n[MAX_PLAYERS]={0};f{A*m=&i[gMarioStates];B*q=&i[gNetworkPlayers];if(!is_player_active(m)+!q->currAreaSyncValid)c;switch(m->action){a:c;}if(b->mirrorMario||b->updateHeadPosTime-gGlobalTimer)c;D p,o;v(add)(v(set)(p,0,'d',0),b->headPos);if((i||m->action-ACT_FIRST_PERSON)*dhwptsp(p,o)){char g[MAX_CONFIG_STRING];const char*z=NULL;smlua_call_event_hooks(HOOK_ON_NAMETAGS_RENDER,i,p,&z);snprintf(g,MAX_CONFIG_STRING,"%s",z?z:q->name);if (!dhwptsp(p, o))c;f32 y=-'x'*'x'/'<'/'<'*'K'/2[o]*dhgfc();s32 j=0;for(;j<numNametags;++j){C*u=&(j[n])[nametags];if(y<u->scale){memmove(n+j+1,j+n,sizeof(s32)*(numNametags-j));break;}};j[n]=i;in.playerIndex=i;v(copy)(in.pos,o);in.scale=y;memcpy(in.name,g,sizeof(g));numNametags++;}}

    gDjuiHudToWorldCalcViewport = true;

    // render nametags
    for (s32 k = 0; k < numNametags; ++k) {
        struct NametagInfo *nametag = &nametags[n[k]];
        struct MarioState *m = &gMarioStates[nametag->playerIndex];
        struct NetworkPlayer *np = &gNetworkPlayers[nametag->playerIndex];

        u8* color = network_get_player_text_color(m->playerIndex);
        f32 measure = djui_hud_measure_text(nametag->name) * nametag->scale * 0.5f;
        nametag->pos[1] -= 16 * nametag->scale;

        u8 alpha = (nametag->playerIndex == 0 ? 255 : MIN(np->fadeOpacity << 3, 255)) * clamp(FADE_SCALE - nametag->scale, 0.f, 1.f);

        struct StateExtras* e = &sStateExtras[nametag->playerIndex];
        if (!e->inited) {
            vec3f_copy(e->prevPos, nametag->pos);
            e->prevScale = nametag->scale;
            e->inited = true;
        }

        // Apply viewport for credits
        extern Vp *gViewportOverride;
        extern Vp *gViewportClip;
        extern Vp gViewportFullscreen;
        Vp *viewport = gViewportOverride == NULL ? gViewportClip : gViewportOverride;
        if (viewport) {
            make_viewport_clip_rect(viewport);
            gSPViewport(gDisplayListHead++, viewport);
        }

        // render name
        djui_hud_print_outlined_text_interpolated(nametag->name,
              e->prevPos[0] - measure,   e->prevPos[1],   e->prevScale,
            nametag->pos[0] - measure, nametag->pos[1], nametag->scale,
            color[0], color[1], color[2], alpha, 0.25);

        // render power meter
        if (nametag->playerIndex != 0 && gNametagsSettings.showHealth) {
            djui_hud_set_color(255, 255, 255, alpha);
            f32 healthScale = 90 * nametag->scale;
            f32 prevHealthScale = 90 * e->prevScale;
            hud_render_power_meter_interpolated(m->health,
                  e->prevPos[0] - (prevHealthScale * 0.5f),   e->prevPos[1] - 72 * nametag->scale, prevHealthScale, prevHealthScale,
                nametag->pos[0] - (    healthScale * 0.5f), nametag->pos[1] - 72 * nametag->scale,     healthScale,     healthScale
            );
        }

        // Reset viewport
        if (viewport) {
            gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, BORDER_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - BORDER_HEIGHT);
            gSPViewport(gDisplayListHead++, &gViewportFullscreen);
        }

        vec3f_copy(e->prevPos, nametag->pos);
        e->prevScale = nametag->scale;
    }
}

void nametags_reset(void) {
    for (u8 i = 0; i < MAX_PLAYERS; i++) {
        sStateExtras[i].inited = false;
    }
}
