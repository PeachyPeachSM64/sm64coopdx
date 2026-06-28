#include "smlua_model_utils.h"
#include "pc/lua/smlua.h"

enum ModelExtendedId smlua_model_util_get_id(const char *name) {
    enum ModelExtendedId modelType; u8 layer;
    const void *asset = dynos_model_get_asset_from_name(name, &modelType, &layer);
    if (asset == NULL) {
        LOG_LUA_LINE("Could not find model: '%s'", name);
        return E_MODEL_ERROR_MODEL;
    }

    struct GraphNode *node = (
        layer == GEO_LAYOUT_LAYER ?
        dynos_model_load_geo_layout(modelType, MODEL_POOL_SESSION, name, asset) :
        dynos_model_load_display_list(modelType, MODEL_POOL_SESSION, name, asset, layer)
    );
    return dynos_model_get_id(node);
}

const char *smlua_model_util_get_name(enum ModelExtendedId modelId) {
    if (modelId == E_MODEL_NONE) {
        return NULL;
    }

    return dynos_model_get_name(modelId);
}
