#include <vector>
#include "dynos.cpp.h"

extern "C" {
#include <assert.h>
#include "sm64.h"
#include "pc/debuglog.h"
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

enum ModelLoadType { MLT_GEO_LAYOUT, MLT_DISPLAY_LIST, MLT_GRAPH_NODE };

struct ModelExtendedData {
    enum ModelExtendedId modelId;
    const char *name;
    const void *asset;
    u8 layer;
    struct GraphNode *nodes[MODEL_POOL_MAX];

public:
    inline ~ModelExtendedData() {
        if (modelId >= E_MODEL__CUSTOM_START_) {
            free((void *) name);
        }
    }

    struct GraphNode *GetGraphNode(enum ModelPool aModelPoolMax = MODEL_POOL_MAX) {
        for (size_t poolIndex = 0; poolIndex < (size_t) aModelPoolMax; ++poolIndex) {
            if (nodes[poolIndex]) {
                return nodes[poolIndex];
            }
        }
        return NULL;
    }
};

class ModelExtendedManager : NoCopy {

public:
    inline ModelExtendedManager() {
        size_t lvlGeoIndex = E_MODEL__LEVEL_GEO_START_;

#define ASSET_TO_ID(_asset_, _modelId_) { \
    if (_asset_ && !mAssetToId.count((const void *) _asset_)) { \
        mAssetToId[(const void *) _asset_] = _modelId_; \
    } \
}

#define MODEL_EXTENDED_GEO(_modelId_, _vanillaId_, _asset_) { \
    mData[_modelId_] = { \
        .modelId = _modelId_, \
        .name    = #_asset_, \
        .asset   = (const void *) _asset_, \
        .layer   = 0xFF, \
    }; \
    ASSET_TO_ID(_asset_, _modelId_); \
}

#define MODEL_EXTENDED_DL(_modelId_, _vanillaId_, _asset_, _layer_) { \
    mData[_modelId_] = { \
        .modelId = _modelId_, \
        .name    = #_asset_, \
        .asset   = (const void *) _asset_, \
        .layer   = _layer_, \
    }; \
    ASSET_TO_ID(_asset_, _modelId_); \
}

#define MODEL_EXTENDED_LVL(_asset_) { \
    mData[(enum ModelExtendedId) lvlGeoIndex] = { \
        .modelId = (enum ModelExtendedId) lvlGeoIndex, \
        .name    = #_asset_, \
        .asset   = (const void *) _asset_, \
        .layer   = 0xFF, \
    }; \
    ASSET_TO_ID(_asset_, (enum ModelExtendedId) lvlGeoIndex); \
    lvlGeoIndex++; \
}

#include "dynos_models_builtin.inl"

#undef ASSET_TO_ID
#undef MODEL_EXTENDED_GEO
#undef MODEL_EXTENDED_DL
#undef MODEL_EXTENDED_LVL

        assert(lvlGeoIndex <= E_MODEL__LEVEL_GEO_END_);
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

    struct ModelExtendedData *GetData(const void *aAsset) {
        auto itId = mAssetToId.find(aAsset);
        if (itId != mAssetToId.end()) {
            return GetData(itId->second);
        }
        return NULL;
    }

    struct ModelExtendedData *GetData(struct GraphNode *aNode) {
        auto itId = mNodeToIdPool.find(aNode);
        if (itId != mNodeToIdPool.end()) {
            return GetData(itId->second.first);
        }
        return NULL;
    }

    enum ModelPool GetModelPool(struct GraphNode *aNode) {
        auto itPool = mNodeToIdPool.find(aNode);
        if (itPool != mNodeToIdPool.end()) {
            return itPool->second.second;
        }
        return MODEL_POOL_MAX;
    }

private:
    void LoadVanillaId(enum ModelExtendedId aVanillaId, enum ModelExtendedId aModelId) {
        if (aVanillaId >= E_MODEL__VANILLA_MIN_ && aVanillaId <= E_MODEL__VANILLA_MAX_) {
            mLoadedModelIds[aVanillaId] = aModelId;
        } else if (aVanillaId != E_MODEL_NONE) {
            LOG_WARNING("[Models] LoadVanillaId: Loading an asset with a model id outside of the vanilla model slot range [%u, %u] has no effect: %u", E_MODEL__VANILLA_MIN_, E_MODEL__VANILLA_MAX_, aVanillaId);
        }
    }

    struct ModelExtendedData *LoadData(struct ModelExtendedData *aData, enum ModelExtendedId aModelId, const void *aAsset, const char *aName, u8 aLayer, bool isAreaLayout) {

        // Area layout
        // Always load in specific ids
        if (isAreaLayout) {
            if (aModelId >= E_MODEL__LEVEL_AREA_START_ && aModelId < E_MODEL__LEVEL_AREA_END_) {
                return &mData[aModelId];
            }
            LOG_ERROR("[Models] LoadData: Trying to load an area geo layout with a model id outside of the range [%u, %u]: %u", E_MODEL__LEVEL_AREA_START_, E_MODEL__LEVEL_AREA_END_ - 1, aModelId);
            return NULL;
        }

        // No existing data for this asset, create a new entry
        if (!aData) {

            // Custom ids alternate between dynos pack and mod actor, allowing the two
            // of them to grow infinitely while being kept in sync across players
            enum ModelExtendedId customId = (aModelId == E_MODEL_DYNOS_PACK) ? E_MODEL__DYNOS_PACK_START_ : E_MODEL__MOD_ACTOR_START_;
            while (mData.find(customId) != mData.end()) {
                customId = (enum ModelExtendedId) ((size_t) customId + 2);
            }

            mData[customId] = {
                .modelId = customId,
                .name = aName ? strdup(aName) : NULL,
                .asset = aAsset,
                .layer = aLayer,
            };
            aData = &mData[customId];

            LOG_INFO("[Models] LoadData: Created new %s entry [ ID: %04u | ASSET: %016llX | NAME: %s ]",
                ((aModelId == E_MODEL_DYNOS_PACK) ? "DynOS pack" : "mod actor"),
                aData->modelId,
                (uintptr_t) aData->asset,
                aData->name
            );
        }

        // Load id in vanilla slot
        LoadVanillaId(aModelId, aData->modelId);

        return aData;
    }

public:
    struct GraphNode *Load(enum ModelLoadType aModelLoadType, enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, const char *aName, u8 aLayer, struct GraphNode *aGraphNode, bool isAreaLayout) {
        if (!aAsset) { return NULL; }

        // Sanity check pool
        if (aModelPool >= MODEL_POOL_MAX) { return NULL; }

        // Check pools to see if there already is a graph node for this asset
        // Can only check pools with lower indices, starting from permanent
        // Ignore this for area layouts (they are always reloaded)
        struct ModelExtendedData *data = GetData(aAsset);
        if (data && !isAreaLayout) {
            enum ModelPool modelPoolMax = (enum ModelPool) ((size_t) aModelPool + 1);
            struct GraphNode *node = data->GetGraphNode(modelPoolMax);
            if (node) {
                LoadVanillaId(aModelId, data->modelId);
                return node;
            }
        }

        // Load model data
        data = LoadData(data, aModelId, aAsset, aName, aLayer, isAreaLayout);
        if (!data) {
            return NULL;
        }

        // Allocate model pool
        if (!mPools[aModelPool]) {
            mPools[aModelPool] = dynamic_pool_init();
        }

        // Load graph node
        struct GraphNode *node = NULL;
        switch (aModelLoadType) {
            case MLT_GEO_LAYOUT:
                node = process_geo_layout(mPools[aModelPool], (void *) aAsset);
                break;
            case MLT_DISPLAY_LIST:
                node = (struct GraphNode *) init_graph_node_display_list(mPools[aModelPool], NULL, aLayer, (void *) aAsset);
                break;
            case MLT_GRAPH_NODE:
                node = aGraphNode;
                break;
        }
        if (!node) { return NULL; }

        // Store graph node
        data->nodes[aModelPool] = node;
        mNodeToIdPool[node] = { data->modelId, aModelPool };

        LOG_INFO("[Models] Load: Successfully loaded model [ %016llX | ID: %04u | POOL: %c | ASSET: %016llX | NAME: %s ]",
            (uintptr_t) node,
            data->modelId,
            ((char[]){ 'P', 'S', 'L' })[aModelPool],
            (uintptr_t) data->asset,
            data->name
        );

        return node;
    }

public:
    void ClearPool(enum ModelPool aModelPool) {
        if (aModelPool >= MODEL_POOL_MAX || !mPools[aModelPool]) {
            return;
        }

        // Schedule pool to be freed
        dynamic_pool_free_pool(mPools[aModelPool]);

        // Clear maps
        std::vector<enum ModelExtendedId> modelIdsToRemove;
        std::vector<const void *> assetsToRemove;
        for (auto &kv : mData) {
            struct ModelExtendedData &data = kv.second;
            struct GraphNode *node = data.nodes[aModelPool];
            if (node) {

                // Remove reference from node to id/pool look up
                mNodeToIdPool.erase(node);

                // Remove graph node pointer from data
                data.nodes[aModelPool] = NULL;

                // Schedule a removal of the whole data entry if it's custom and there is no graph node loaded anymore
                if (data.modelId >= E_MODEL__CUSTOM_START_ && !data.GetGraphNode()) {
                    modelIdsToRemove.push_back(data.modelId);
                    for (const auto &assetId : mAssetToId) {
                        if (assetId.second == data.modelId) {
                            assetsToRemove.push_back(assetId.first);
                        }
                    }
                }
            }
        }

        // Remove ids from data
        // Remove all asset to id entries for these ids
        for (const auto &modelId : modelIdsToRemove) { mData.erase(modelId); }
        for (const auto &asset : assetsToRemove) { mAssetToId.erase(asset); }
    }

private:
    std::map<enum ModelExtendedId, struct ModelExtendedData> mData;
    std::map<const void *, enum ModelExtendedId> mAssetToId;
    std::map<struct GraphNode *, std::pair<enum ModelExtendedId, enum ModelPool>> mNodeToIdPool;
    enum ModelExtendedId mLoadedModelIds[E_MODEL__VANILLA_END_];
    struct DynamicPool *mPools[MODEL_POOL_MAX];
};

static ModelExtendedManager sModels;

  //////////////
 // Built-in //
//////////////

const GeoLayout *DynOS_Builtin_Actor_GetFromName(const char *aDataName) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aDataName](const struct ModelExtendedData &data) { return strcmp(data.name, aDataName) == 0; },
        E_MODEL__EXTENDED_START_,
        E_MODEL__EXTENDED_END_
    );
    if (data) {
        return (const GeoLayout *) data->asset;
    }
    return NULL;
}

const char *DynOS_Builtin_Actor_GetFromData(const GeoLayout *aData) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aData](const struct ModelExtendedData &data) { return data.asset == (const void *) aData; },
        E_MODEL__EXTENDED_START_,
        E_MODEL__EXTENDED_END_
    );
    if (data) {
        return data->name;
    }
    return NULL;
}

const GeoLayout *DynOS_Builtin_LvlGeo_GetFromName(const char *aDataName) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aDataName](const struct ModelExtendedData &data) { return strcmp(data.name, aDataName) == 0; },
        E_MODEL__LEVEL_GEO_START_,
        E_MODEL__LEVEL_GEO_END_
    );
    if (data) {
        return (const GeoLayout *) data->asset;
    }
    return NULL;
}

const char *DynOS_Builtin_LvlGeo_GetFromData(const GeoLayout *aData) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aData](const struct ModelExtendedData &data) { return data.asset == (const void *) aData; },
        E_MODEL__LEVEL_GEO_START_,
        E_MODEL__LEVEL_GEO_END_
    );
    if (data) {
        return data->name;
    }
    return NULL;
}

  //////////
 // Load //
//////////

struct GraphNode *DynOS_Model_LoadGeoLayout(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, const char *aName, bool isAreaLayout) {
    return sModels.Load(MLT_GEO_LAYOUT, aModelId, aModelPool, aAsset, aName, 0xFF, NULL, isAreaLayout);
}

struct GraphNode *DynOS_Model_LoadDisplayList(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, u8 aLayer) {
    return sModels.Load(MLT_DISPLAY_LIST, aModelId, aModelPool, aAsset, NULL, aLayer, NULL, false);
}

struct GraphNode *DynOS_Model_LoadGraphNode(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, struct GraphNode *aNode) {
    return sModels.Load(MLT_GRAPH_NODE, aModelId, aModelPool, aAsset, NULL, 0xFF, aNode, false);
}

  /////////
 // Get //
/////////

static struct GraphNode *DynOS_Model_GetErrorModel() {
    struct ModelExtendedData *data = sModels.GetData(E_MODEL_ERROR_MODEL);
    if (data) {
        return data->GetGraphNode();
    }
    return NULL;
}

struct GraphNode *DynOS_Model_GetGraphNode(enum ModelExtendedId aModelId) {
    if (aModelId == E_MODEL_NONE) {
        return NULL;
    }

    struct ModelExtendedData *data = sModels.GetData(aModelId);
    if (data) {
        struct GraphNode *node = data->GetGraphNode();
        if (node) {
            return node;
        }

        // Try to load it
        LOG_WARNING("DynOS_Model_GetGraphNode: Trying to load model with existing data now: %u", data->modelId);
        return sModels.Load(data->layer != 0xFF ? MLT_DISPLAY_LIST : MLT_GEO_LAYOUT, E_MODEL_NONE, MODEL_POOL_SESSION, data->asset, data->name, data->layer, NULL, false);
    }

    // Default: valid id but no model loaded
    return DynOS_Model_GetErrorModel();
}

enum ModelExtendedId DynOS_Model_GetId(struct GraphNode *aNode) {
    if (!aNode) {
        return E_MODEL_NONE;
    }

    struct ModelExtendedData *data = (
        aNode->georef ?
        sModels.GetData(aNode->georef) :
        sModels.GetData(aNode)
    );
    if (data) {
        return data->modelId;
    }

    return E_MODEL_NONE;
}

const char *DynOS_Model_GetName(enum ModelExtendedId aModelId) {
    if (aModelId == E_MODEL_NONE) {
        return NULL;
    }

    struct ModelExtendedData *data = sModels.GetData(aModelId);
    if (data) {
        return data->name;
    }

    return NULL;
}

enum ModelPool DynOS_Model_GetModelPool(struct GraphNode *aNode) {
    if (!aNode) {
        return MODEL_POOL_MAX;
    }

    return sModels.GetModelPool(aNode);
}

void DynOS_Model_ClearPool(enum ModelPool aModelPool) {
    sModels.ClearPool(aModelPool);
}
