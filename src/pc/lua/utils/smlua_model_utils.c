#include "smlua_model_utils.h"
#include "pc/lua/smlua.h"

enum ModelExtendedId smlua_model_util_get_id(const char *name) {
    const GeoLayout *geoLayout = dynos_geolayout_get(name);
    if (!geoLayout) {
        LOG_LUA_LINE("Could not find model: '%s'", name);
        return E_MODEL_ERROR_MODEL;
    }

    struct GraphNode *node = dynos_model_load_geo_layout(E_MODEL_NONE, MODEL_POOL_SESSION, geoLayout, name, false);
    return dynos_model_get_id(node);
}

const char *smlua_model_util_get_name(enum ModelExtendedId modelId) {
    if (modelId == E_MODEL_NONE) {
        return NULL;
    }


}

bool smlua_model_util_forget(enum ModelExtendedId modelId) {
    if (modelId == E_MODEL_NONE) {
        return false;
    }


}

