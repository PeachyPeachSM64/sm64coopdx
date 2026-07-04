#include <vector>
#include <algorithm>
#include "dynos.cpp.h"

extern "C" {
#include <assert.h>
#include "sm64.h"
#include "geo_commands.h"
#include "pc/debuglog.h"
#include "pc/mods/mod_fs.h"
#include "engine/geo_layout.h"
#include "engine/graph_node.h"
#include "actors/group0.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group3.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group8.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group16.h"
#include "actors/group17.h"
#include "actors/common0.h"
#include "actors/common1.h"
#include "actors/custom0.h"
#include "actors/zcustom0.h"
#include "levels/bob/header.h"
#include "levels/wf/header.h"
#include "levels/jrb/header.h"
#include "levels/ccm/header.h"
#include "levels/bbh/header.h"
#include "levels/hmc/header.h"
#include "levels/lll/header.h"
#include "levels/ssl/header.h"
#include "levels/ddd/header.h"
#include "levels/sl/header.h"
#include "levels/wdw/header.h"
#include "levels/ttm/header.h"
#include "levels/thi/header.h"
#include "levels/ttc/header.h"
#include "levels/rr/header.h"
#include "levels/bitdw/header.h"
#include "levels/bitfs/header.h"
#include "levels/bits/header.h"
#include "levels/bowser_1/header.h"
#include "levels/bowser_2/header.h"
#include "levels/bowser_3/header.h"
#include "levels/pss/header.h"
#include "levels/cotmc/header.h"
#include "levels/totwc/header.h"
#include "levels/vcutm/header.h"
#include "levels/wmotr/header.h"
#include "levels/sa/header.h"
#include "levels/castle_grounds/header.h"
#include "levels/castle_inside/header.h"
#include "levels/castle_courtyard/header.h"
#include "levels/menu/header.h"
}

//
//  //////////////////////
//  // Custom model ids //
//  //////////////////////
//
// There are 4 types of custom ids:
// - DynOS packs
// - Mod actors
// - Level geos
// - ModFS models
//
// However, two of them need separate slots for each player. Why?
// Mod actors loading order is consistent.
// DynOS packs are local.
// But level geos and ModFS models load on demand and thus can easily
// be desynced across clients.
//
// Now, custom model ids, instead of being separated by some fixed
// arbitrary constant, are arranged this way:
// custom id:  0   1    2    3  ...    17   18   19  ...    33
//         0  DP, MA, LG0, LG1, ..., LG15, MF0, MF1, ..., MF15
//         1  DP, MA, LG0, LG1, ..., LG15, MF0, MF1, ..., MF15
// ...and so on.
// This arrangement allows to grow model lists infinitely, without having
// the risk of two lists overlapping at some point.
//

// Make sure all custom types are defined in dynos_models_custom.inl
static const size_t E_MODEL__TYPE_COUNT_ = (0
#define MODEL_TYPE_CUSTOM(...) + 1
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM
);
static_assert(E_MODEL__TYPE_END_ - E_MODEL__TYPE_START_ == E_MODEL__TYPE_COUNT_);

static const size_t E_MODEL__CUSTOM_OFFSET_ = (0
#define MODEL_TYPE_CUSTOM(_modelType_, _size_, ...) + (_size_)
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM
);

static constexpr size_t E_MODEL__CUSTOM_TYPE_SIZE_(size_t modelType) {
    return (0
#define MODEL_TYPE_CUSTOM(_modelType_, _size_, ...) + (modelType == _modelType_) * (_size_)
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM
    );
}

static constexpr size_t E_MODEL__CUSTOM_TYPE_START_(size_t modelType) {
    return E_MODEL__CUSTOM_START_ + (0 *
#define MODEL_TYPE_CUSTOM(_modelType_, _size_, ...) (modelType >= _modelType_) + (_size_) *
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM
    0);
}

static constexpr size_t E_MODEL__CUSTOM_TYPE_END_(size_t modelType) {
    return E_MODEL__CUSTOM_START_ + (0
#define MODEL_TYPE_CUSTOM(_modelType_, _size_, ...) + (modelType >= _modelType_) * (_size_)
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM
    );
}

static constexpr size_t E_MODEL__CUSTOM_TYPE_INDEX_(size_t modelType, size_t modelId) {
    size_t modelCustomIndex = (modelId - E_MODEL__CUSTOM_TYPE_START_(modelType));
    size_t modelTypeSize = E_MODEL__CUSTOM_TYPE_SIZE_(modelType);
    size_t modelTypeIndex = modelCustomIndex / E_MODEL__CUSTOM_OFFSET_;
    size_t modelSubIndex = modelCustomIndex % E_MODEL__CUSTOM_OFFSET_;
    return (modelTypeIndex * modelTypeSize) + modelSubIndex;
}

enum ModelExtendedId DynOS_Model_GetType(enum ModelExtendedId aModelId) {

    // Vanilla models
    if (aModelId < E_MODEL__EXTENDED_END_) {
        return E_MODEL_NONE;
    }

    // Special slots
    if (aModelId < E_MODEL__CUSTOM_START_) {
        return aModelId;
    }

    // Custom models
    size_t customStart = E_MODEL__CUSTOM_START_ + ((aModelId - E_MODEL__CUSTOM_START_) % E_MODEL__CUSTOM_OFFSET_);
    for (size_t modelType = E_MODEL__TYPE_START_; modelType < E_MODEL__TYPE_END_; modelType++) {
        if (customStart < E_MODEL__CUSTOM_TYPE_END_(modelType)) {
            return (enum ModelExtendedId) modelType;
        }
    }
    return E_MODEL_NONE;
}

static const char *DynOS_Model_IdName(enum ModelExtendedId modelId) {
#define MODEL_EXTENDED_GEO(_modelId_, ...) { if (modelId == _modelId_) { return #_modelId_; } }
#define MODEL_EXTENDED_DL(_modelId_, ...) { if (modelId == _modelId_) { return #_modelId_; } }
#include "dynos_models_builtin.inl"
#undef MODEL_EXTENDED_GEO
#undef MODEL_EXTENDED_DL
    if (modelId == E_MODEL_AREA_GEO) { return "E_MODEL_AREA_GEO"; }

    // Custom model types
#define MODEL_TYPE_CUSTOM(_modelType_, ...) { if (modelId == _modelType_) { return #_modelType_; } }
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM

    // Custom model ids
    static char modelIdStr[32];
    if (modelId >= E_MODEL__CUSTOM_START_) {
        enum ModelExtendedId modelType = DynOS_Model_GetType(modelId);
        const char *modelTypeName = NULL;
#define MODEL_TYPE_CUSTOM(_modelType_, ...) { if (modelType == _modelType_) { modelTypeName = #_modelType_; } }
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM

        if (modelTypeName != NULL) {
            size_t modelTypeSize = E_MODEL__CUSTOM_TYPE_SIZE_(modelType);
            size_t modelIndex = E_MODEL__CUSTOM_TYPE_INDEX_(modelType, modelId);
            if (modelTypeSize > 1) {
                snprintf(modelIdStr, sizeof(modelIdStr), "%s_%04llu_%02llu", modelTypeName, modelIndex / modelTypeSize, modelIndex % modelTypeSize);
            } else {
                snprintf(modelIdStr, sizeof(modelIdStr), "%s_%04llu", modelTypeName, modelIndex);
            }
            return modelIdStr;
        }
    }

    // Default
    snprintf(modelIdStr, sizeof(modelIdStr), "%03u", modelId);
    return modelIdStr;
}

//
// Special GraphNode root for models
// Embed the model id and pool and allows for graph node hot swapping
//

#undef max
static const size_t GRAPH_NODE_MAX_SIZE = (
#define GRAPH_NODE_TYPE(_name_, _id_, _type_, ...) std::max(sizeof(struct _type_),
#include "src/engine/graph_node_types.inl"
#undef GRAPH_NODE_TYPE
0llu
#define GRAPH_NODE_TYPE(_name_, _id_, _type_, ...) )
#include "src/engine/graph_node_types.inl"
#undef GRAPH_NODE_TYPE
);

struct GraphNodeModel {
    struct GraphNode node;
    u8 padding[GRAPH_NODE_MAX_SIZE - sizeof(struct GraphNode)];

    enum ModelExtendedId modelId;
    enum ModelPool modelPool;
    const void *asset;
};

//
// Model manager
//

enum ModelLoadType { MLT_GEO_LAYOUT, MLT_DISPLAY_LIST, MLT_GRAPH_NODE };

struct ModelExtendedData {
    enum ModelExtendedId modelId;
    std::string name;
    const void *asset;
    u8 layer;
    struct GraphNodeModel *model;

public:
    const char *GetName() const {
        return name.empty() ? NULL : name.c_str();
    }

    void UpdateGraphNodeModel(struct GraphNode *aNode, enum ModelPool aModelPool, const void *aAsset) {
        size_t nodeSize = get_graph_node_type_size(aNode->type);
        memset(model, 0, sizeof(*model));
        memcpy(model, aNode, nodeSize);

        // Fix links
        if (model->node.prev) {
            if (model->node.prev == aNode) {
                model->node.prev = &model->node;
            }
            model->node.prev->next = &model->node;
        }
        if (model->node.next) {
            if (model->node.next == aNode) {
                model->node.next = &model->node;
            }
            model->node.next->prev = &model->node;
        }
        if (model->node.parent && model->node.parent->children == aNode) {
            model->node.parent->children = &model->node;
        }
        if (model->node.children) {
            struct GraphNode *child = model->node.children;
            do {
                if (child->parent == aNode) {
                    child->parent = &model->node;
                }
                child = child->next;
            } while (child != model->node.children);
        }

        model->node.isModel = true;
        model->modelId = modelId;
        model->modelPool = aModelPool;
        model->asset = aAsset;
    }
};

class ModelExtendedManager : NoCopy {

public:
    inline ModelExtendedManager() {

#define MODEL_EXTENDED_GEO(_modelId_, _asset_) { \
    mData[_modelId_] = { \
        .modelId = _modelId_, \
        .name    = #_asset_, \
        .asset   = (const void *) _asset_, \
        .layer   = LAYER_GEO_LAYOUT, \
    }; \
}

#define MODEL_EXTENDED_DL(_modelId_, _asset_, _layer_) { \
    mData[_modelId_] = { \
        .modelId = _modelId_, \
        .name    = #_asset_, \
        .asset   = (const void *) _asset_, \
        .layer   = _layer_, \
    }; \
}

#include "dynos_models_builtin.inl"

#undef MODEL_EXTENDED_GEO
#undef MODEL_EXTENDED_DL
    }

public:
    template <typename EqualFunc>
    struct ModelExtendedData *GetData(const EqualFunc& aEqualFunc, enum ModelExtendedId aModelIdStart = E_MODEL_NONE, enum ModelExtendedId aModelIdEnd = E_MODEL_NONE) {
        for (auto &kv : mData) {
            enum ModelExtendedId modelId = kv.first;
            if (aModelIdStart > E_MODEL_NONE && modelId < aModelIdStart) { continue; }
            if (aModelIdEnd > E_MODEL_NONE && modelId >= aModelIdEnd) { break; }

            auto &data = kv.second;
            if (aEqualFunc(data)) {
                return &data;
            }
        }
        return NULL;
    }

    struct ModelExtendedData *GetData(const char *aName, enum ModelExtendedId aModelIdStart = E_MODEL_NONE, enum ModelExtendedId aModelIdEnd = E_MODEL_NONE) {
        return GetData(
            [aName](const struct ModelExtendedData &data) { return data.GetName() != NULL && strcmp(data.GetName(), aName) == 0; },
            aModelIdStart,
            aModelIdEnd
        );
    }

    struct ModelExtendedData *GetData(const void *aAsset, enum ModelExtendedId aModelIdStart = E_MODEL_NONE, enum ModelExtendedId aModelIdEnd = E_MODEL_NONE) {
        return GetData(
            [aAsset](const struct ModelExtendedData &data) { return data.asset == aAsset; },
            aModelIdStart,
            aModelIdEnd
        );
    }

    struct ModelExtendedData *GetData(enum ModelExtendedId aModelId) {
        // Convert vanilla id to extended id
        if (aModelId >= E_MODEL__VANILLA_MIN_ && aModelId <= E_MODEL__VANILLA_MAX_) {
            aModelId = mLoadedModelIds[aModelId];
        }
        auto itData = mData.find(aModelId);
        if (itData != mData.end()) {
            return &itData->second;
        }
        return NULL;
    }

private:
    void LoadVanillaId(enum ModelExtendedId aVanillaId, enum ModelExtendedId aModelId) {
        if (aVanillaId >= E_MODEL__VANILLA_MIN_ && aVanillaId <= E_MODEL__VANILLA_MAX_) {
            mLoadedModelIds[aVanillaId] = aModelId;
        } else if (aVanillaId != E_MODEL_NONE && aVanillaId < E_MODEL__TYPE_START_) {
            LOG_WARNING("[Models] LoadVanillaId: Loading an asset with a model id outside of the vanilla model slot range [%u, %u] has no effect: %s", E_MODEL__VANILLA_MIN_, E_MODEL__VANILLA_MAX_, DynOS_Model_IdName(aVanillaId));
        }
    }

    struct ModelExtendedData *LoadData(struct ModelExtendedData *aData, enum ModelExtendedId aModelId, const void *aAsset, const char *aName, u8 aLayer) {

        // Area layout
        // Doesn't need to be loaded internally, we just need a graph node
        if (aModelId == E_MODEL_AREA_GEO) {
            static struct ModelExtendedData sAreaGeoData;
            sAreaGeoData.modelId = E_MODEL_AREA_GEO;
            sAreaGeoData.name = "";
            sAreaGeoData.asset = aAsset;
            sAreaGeoData.layer = aLayer;
            return &sAreaGeoData;
        }

        // No existing data for this asset, create a new entry
        if (!aData) {
            enum ModelExtendedId customId;

            // Resolve model id
#define MODEL_TYPE_CUSTOM(_modelType_, _size_, _offset_) \
            if (aModelId == _modelType_) { \
                customId = (enum ModelExtendedId) (E_MODEL__CUSTOM_TYPE_START_(_modelType_) + _offset_); \
            } else
#include "dynos_models_custom.inl"
#undef MODEL_TYPE_CUSTOM
            if (aModelId >= E_MODEL__VANILLA_MIN_ && aModelId <= E_MODEL__VANILLA_MAX_) {
                customId = (enum ModelExtendedId) (E_MODEL__CUSTOM_TYPE_START_(E_MODEL_TYPE_LEVEL_GEO) + network_global_index_from_local(0));
            } else {
                LOG_ERROR("[Models] LoadData: Trying to load an invalid custom model: %s (%s)", DynOS_Model_IdName(aModelId), aName);
                return NULL;
            }

            // Find the next available slot
            while (mData.find(customId) != mData.end()) {
                customId = (enum ModelExtendedId) ((size_t) customId + E_MODEL__CUSTOM_OFFSET_);
            }

            mData[customId] = {
                .modelId = customId,
                .name = aName ? aName : "",
                .asset = aAsset,
                .layer = aLayer,
            };
            aData = &mData[customId];

            LOG_INFO("[Models] LoadData: Created new entry: %s (%s)", DynOS_Model_IdName(aData->modelId), aData->GetName());
        }

        // Load id in vanilla slot
        LoadVanillaId(aModelId, aData->modelId);

        return aData;
    }

public:
    struct GraphNodeModel *Load(enum ModelLoadType aModelLoadType, enum ModelExtendedId aModelId, enum ModelPool aModelPool, const char *aName, const void *aAsset, u8 aLayer, struct GraphNode *aGraphNode) {
        if (!aAsset) { return NULL; }

        // Sanity check pool
        if (aModelPool >= MODEL_POOL_MAX) { return NULL; }

        // Try to find name
        if (!aName) {
            aName = DynOS_Model_GetNameFromAsset(aAsset);
        }

        // Check if there already is a graph node for this asset
        // If the graph node was loaded in a pool with higher priority, return it
        // Ignore this for area layouts (they must be always reloaded)
        struct ModelExtendedData *data = NULL;
        if (aModelId != E_MODEL_AREA_GEO && (data = GetData(aAsset)) != NULL) {
            if (data->model && data->model->modelPool <= aModelPool) {
                if (data->name.empty() && aName != NULL) {
                    data->name = aName;
                }
                LoadVanillaId(aModelId, data->modelId);
                return data->model;
            }
        }

        // Allocate model pool
        if (!mPools[aModelPool]) {
            mPools[aModelPool] = dynamic_pool_init();
            if (!mPools[aModelPool]) {
                LOG_ERROR("[Models] Load: Could not allocate model pool!");
                return NULL;
            }
        }

        // TODO MODELS: this shit
        // // Create fake geo layout for ModFS placeholder
        // if (aAsset == mod_fs_placeholder_geo) {
        //     aAsset = dynamic_pool_alloc(mPools[aModelPool], sizeof(mod_fs_placeholder_geo));
        //     if (!aAsset) {
        //         LOG_ERROR("[Models] Load: Could not allocate data for ModFS model placeholder!");
        //         return NULL;
        //     }
        //     memcpy((void *) aAsset, mod_fs_placeholder_geo, sizeof(mod_fs_placeholder_geo));

        //     // Create entry
        //     mData[aModelId] = {
        //         .modelId = aModelId,
        //         .name = aName ? aName : "",
        //         .asset = aAsset,
        //         .layer = aLayer,
        //     };
        //     data = &mData[aModelId];
        //     mAssetToId[aAsset] = aModelId;
        // }

        // Load model data
        data = LoadData(data, aModelId, aAsset, aName, aLayer);
        if (!data) {
            return NULL;
        }

        // Load graph node
        struct GraphNode *node = NULL;
        switch (aModelLoadType) {
            case MLT_GEO_LAYOUT:
                node = process_geo_layout(mPools[aModelPool], (const GeoLayout *) aAsset, aModelId == E_MODEL_AREA_GEO);
                break;
            case MLT_DISPLAY_LIST:
                node = (struct GraphNode *) init_graph_node_display_list(mPools[aModelPool], NULL, aLayer, (void *) aAsset);
                break;
            case MLT_GRAPH_NODE:
                node = aGraphNode;
                break;
        }
        if (!node) { return NULL; }

        // Create a new graph node model if needed
        if (data->model == NULL || aModelId == E_MODEL_AREA_GEO) {
            data->model = (struct GraphNodeModel *) dynamic_pool_alloc(mPools[aModelPool], sizeof(struct GraphNodeModel));
            if (!data->model) {
                LOG_ERROR("[Models] Load: Could not allocate graph node model!");
                return NULL;
            }
        }
        data->UpdateGraphNodeModel(node, aModelPool, aAsset);

        LOG_INFO("[Models] Load: Successfully loaded model in %s pool: %s (%s)", ((const char *[]){ "permanent", "session", "level" })[aModelPool], DynOS_Model_IdName(data->modelId), data->GetName());

        return data->model;
    }

public:
    void ClearPool(enum ModelPool aModelPool) {
        if (aModelPool >= MODEL_POOL_MAX || !mPools[aModelPool]) {
            return;
        }

        // Schedule pool to be freed
        dynamic_pool_free_pool(mPools[aModelPool]);

        // Clear data
        std::vector<enum ModelExtendedId> modelIdsToRemove;
        for (auto &kv : mData) {
            struct ModelExtendedData &data = kv.second;
            if (data.model && data.model->modelPool == aModelPool) {
                data.model = NULL;

                // Schedule a removal of the whole data entry if it's custom and there is no graph node loaded anymore
                if (data.modelId >= E_MODEL__CUSTOM_START_) {
                    modelIdsToRemove.push_back(data.modelId);
                }
            }
        }

        // Remove ids from data
        for (const auto &modelId : modelIdsToRemove) {
            mData.erase(modelId);
        }
    }

private:
    std::map<enum ModelExtendedId, struct ModelExtendedData> mData;
    enum ModelExtendedId mLoadedModelIds[E_MODEL__VANILLA_END_];
    struct DynamicPool *mPools[MODEL_POOL_MAX];
};

static ModelExtendedManager sModels;

  //////////////
 // Built-in //
//////////////

const void *DynOS_Model_GetBuiltinAssetFromName(const char *aName, u8 *outLayer) {
    const struct ModelExtendedData *data = sModels.GetData(aName, E_MODEL__EXTENDED_START_, E_MODEL__EXTENDED_END_);
    if (data) {
        if (outLayer) { *outLayer = data->layer; }
        return data->asset;
    }
    return NULL;
}

const char *DynOS_Model_GetNameFromBuiltinAsset(const void *aAsset) {
    const struct ModelExtendedData *data = sModels.GetData(aAsset, E_MODEL__EXTENDED_START_, E_MODEL__EXTENDED_END_);
    if (data) {
        return data->GetName();
    }
    return NULL;
}

  //////////
 // Load //
//////////

struct GraphNode *DynOS_Model_LoadGeoLayout(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const char *aName, const void *aAsset) {
    return (struct GraphNode *) sModels.Load(MLT_GEO_LAYOUT, aModelId, aModelPool, aName, aAsset, LAYER_GEO_LAYOUT, NULL);
}

struct GraphNode *DynOS_Model_LoadDisplayList(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const char *aName, const void *aAsset, u8 aLayer) {
    return (struct GraphNode *) sModels.Load(MLT_DISPLAY_LIST, aModelId, aModelPool, aName, aAsset, aLayer, NULL);
}

struct GraphNode *DynOS_Model_LoadGraphNode(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const char *aName, const void *aAsset, u8 aLayer, struct GraphNode *aGraphNode) {
    return (struct GraphNode *) sModels.Load(MLT_GRAPH_NODE, aModelId, aModelPool, aName, aAsset, aLayer, aGraphNode);
}

  /////////
 // Get //
/////////

static struct GraphNode *DynOS_Model_GetErrorModel() {
    struct ModelExtendedData *data = sModels.GetData(E_MODEL_ERROR_MODEL);
    if (data) {
        return (struct GraphNode *) sModels.Load(MLT_GEO_LAYOUT, E_MODEL_NONE, MODEL_POOL_PERMANENT, data->GetName(), data->asset, LAYER_GEO_LAYOUT, NULL);
    }
    return NULL;
}

struct GraphNode *DynOS_Model_GetGraphNode(enum ModelExtendedId aModelId) {
    if (aModelId == E_MODEL_NONE) {
        return NULL;
    }

    struct ModelExtendedData *data = sModels.GetData(aModelId);
    if (data) {
        if (data->model) {
            return (struct GraphNode *) data->model;
        }

        // Try to load it
        LOG_WARNING("DynOS_Model_GetGraphNode: Trying to load model with existing data now: %s", DynOS_Model_IdName(data->modelId));
        return (struct GraphNode *) sModels.Load(data->layer == LAYER_GEO_LAYOUT ? MLT_GEO_LAYOUT : MLT_DISPLAY_LIST, E_MODEL_NONE, MODEL_POOL_SESSION, data->GetName(), data->asset, data->layer, NULL);
    }

    // TODO MODELS: this shit
    // // ModFS special case
    // // These models are meant to be easily replaced with `smlua_model_util_load_id`
    // // To keep their id, allocate space with the error model
    // if (DynOS_Model_GetType(aModelId) == E_MODEL_TYPE_MOD_FS) {
    //     LOG_INFO("DynOS_Model_GetGraphNode: Allocating space for ModFS model: %s", DynOS_Model_IdName(aModelId));
    //     return sModels.Load(MLT_GEO_LAYOUT, aModelId, MODEL_POOL_SESSION, NULL, mod_fs_placeholder_geo, LAYER_GEO_LAYOUT, NULL);
    // }

    return DynOS_Model_GetErrorModel();
}

enum ModelExtendedId DynOS_Model_GetId(struct GraphNode *aNode) {
    if (!aNode) {
        return E_MODEL_NONE;
    }

    if (aNode->isModel) {
        enum ModelExtendedId modelId = ((struct GraphNodeModel *) aNode)->modelId;
        if (DynOS_Model_GetType(modelId) != E_MODEL_TYPE_DYNOS_PACK) {
            return modelId;
        }
    }

    if (aNode->georef) {
        struct ModelExtendedData *data = sModels.GetData(aNode->georef);
        if (data) {
            return data->modelId;
        }
    }

    return E_MODEL_NONE;
}

bool DynOS_Model_IsSame(struct GraphNode *aNode, enum ModelExtendedId aModelId) {
    if (!aNode) {
        return aModelId == E_MODEL_NONE;
    }

    // Check model id
    if (DynOS_Model_GetId(aNode) == aModelId) {
        return true;
    }

    // Check asset
    if (aNode->isModel) {
        struct ModelExtendedData *data = sModels.GetData(aModelId);
        if (data && data->asset == ((struct GraphNodeModel *) aNode)->asset) {
            return true;
        }
    }

    // Check node and georef
    struct GraphNode *node = DynOS_Model_GetGraphNode(aModelId);
    if (aNode == node || (node && aNode->georef == node->georef)) {
        return true;
    }

    return false;
}

const char *DynOS_Model_GetName(enum ModelExtendedId aModelId) {
    if (aModelId == E_MODEL_NONE) {
        return NULL;
    }

    struct ModelExtendedData *data = sModels.GetData(aModelId);
    if (data) {
        return data->GetName();
    }

    return NULL;
}

const void *DynOS_Model_GetAssetFromName(const char *aName, enum ModelExtendedId *outModelType, u8 *outLayer) {
    if (aName == NULL) { return NULL; }

    // Check levels
    for (const auto &level : DynOS_Lvl_GetArray()) {
        const auto &geoNode = level.second->mGeoLayouts.Find(aName);
        if (geoNode) {
            if (outModelType) { *outModelType = E_MODEL_TYPE_LEVEL_GEO; }
            if (outLayer) { *outLayer = LAYER_GEO_LAYOUT; }
            return (const void *) geoNode->mData;
        }
    }

    // Check managed assets
    struct ModelExtendedData *data = sModels.GetData(aName);
    if (data) {
        if (outModelType) { *outModelType = DynOS_Model_GetType(data->modelId); }
        if (outLayer) { *outLayer = data->layer; }
        return data->asset;
    }

    // Check ModFS file
    if (is_mod_fs_file(aName)) {
        if (DynOS_Actor_AddCustom(gLuaActiveMod->index, -1, aName, aName)) {
            return DynOS_Model_GetAssetFromName(aName, outModelType, outLayer);
        }
    }

    return NULL;
}

const char *DynOS_Model_GetNameFromAsset(const void *aAsset) {
    if (aAsset == NULL) { return NULL; }

    // Check levels
    for (const auto &level : DynOS_Lvl_GetArray()) {
        for (const auto &geoNode : level.second->mGeoLayouts) {
            if ((const void *) geoNode->mData == aAsset) {
                return geoNode->mName.begin();
            }
        }
    }

    // Check managed assets
    struct ModelExtendedData *data = sModels.GetData(aAsset);
    if (data) {
        return data->GetName();
    }

    return NULL;
}

enum ModelPool DynOS_Model_GetModelPool(struct GraphNode *aNode) {
    if (!aNode) {
        return MODEL_POOL_MAX;
    }

    if (aNode->isModel) {
        return ((struct GraphNodeModel *) aNode)->modelPool;
    }

    return MODEL_POOL_MAX;
}

void DynOS_Model_ClearPool(enum ModelPool aModelPool) {
    sModels.ClearPool(aModelPool);
}
