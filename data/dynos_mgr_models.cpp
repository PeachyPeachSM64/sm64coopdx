#include <vector>
#include "dynos.cpp.h"

extern "C" {
#include <assert.h>
#include "sm64.h"
#include "pc/debuglog.h"
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

static const char *DynOS_Model_IdName(enum ModelExtendedId modelId) {
#define MODEL_EXTENDED_GEO(_modelId_, ...) { if (modelId == _modelId_) { return #_modelId_; } }
#define MODEL_EXTENDED_DL(_modelId_, ...) { if (modelId == _modelId_) { return #_modelId_; } }
#define MODEL_EXTENDED_LVL(...)
#include "dynos_models_builtin.inl"
#undef MODEL_EXTENDED_GEO
#undef MODEL_EXTENDED_DL
#undef MODEL_EXTENDED_LVL
    if (modelId == E_MODEL_AREA_GEO) { return "E_MODEL_AREA_GEO"; }
    static char modelIdStr[32];
    if (modelId >= E_MODEL__LEVEL_GEO_START_ && modelId < E_MODEL__LEVEL_GEO_END_) {
        snprintf(modelIdStr, sizeof(modelIdStr), "E_MODEL_LEVEL_GEO_%03X", modelId - E_MODEL__LEVEL_GEO_START_);
    } else if (modelId < E_MODEL__CUSTOM_START_) {
        snprintf(modelIdStr, sizeof(modelIdStr), "%04X", modelId);
    } else if (((modelId - E_MODEL_DYNOS_PACK) & 1) == 0) {
        snprintf(modelIdStr, sizeof(modelIdStr), "E_MODEL_DYNOS_PACK_%04X", (modelId - E_MODEL_DYNOS_PACK) / 2);
    } else {
        snprintf(modelIdStr, sizeof(modelIdStr), "E_MODEL_MOD_ACTOR_%04X", (modelId - E_MODEL_MOD_ACTOR) / 2);
    }
    return modelIdStr;
}

enum ModelLoadType { MLT_GEO_LAYOUT, MLT_DISPLAY_LIST, MLT_GRAPH_NODE };

struct ModelExtendedData {
    enum ModelExtendedId modelId;
    std::string name;
    const void *asset;
    u8 layer;
    struct GraphNode *nodes[MODEL_POOL_MAX];

public:
    const char *GetName() const {
        return name.empty() ? NULL : name.c_str();
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
    if (_asset_) { \
        if (!mAssetToId.count((const void *) _asset_)) { \
            mAssetToId[(const void *) _asset_] = _modelId_; \
        } else { \
            mIdReferences[_modelId_] = mAssetToId[(const void *) _asset_]; \
        } \
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
        // Convert reference to original id
        if (mIdReferences.count(aModelId)) {
            aModelId = mIdReferences[aModelId];
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
        } else if (aVanillaId != E_MODEL_NONE && aVanillaId < E_MODEL__CUSTOM_START_) {
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

            // Is that even possible?
            if (aModelId != E_MODEL_DYNOS_PACK && aModelId != E_MODEL_MOD_ACTOR) {
                LOG_ERROR("[Models] LoadData: Trying to load a custom model that is not a DynOS pack or a mod actor: %s (%s)", DynOS_Model_IdName(aModelId), aName);
                return NULL;
            }

            // Custom ids alternate between dynos pack and mod actor, allowing the two
            // of them to grow infinitely while being kept in sync across players
            enum ModelExtendedId customId = aModelId;
            while (mData.find(customId) != mData.end()) {
                customId = (enum ModelExtendedId) ((size_t) customId + 2);
            }

            mData[customId] = {
                .modelId = customId,
                .name = aName ? aName : "",
                .asset = aAsset,
                .layer = aLayer,
            };
            aData = &mData[customId];
            mAssetToId[aAsset] = customId;

            LOG_INFO("[Models] LoadData: Created new entry: %s (%s)", DynOS_Model_IdName(aData->modelId), aData->GetName());
        }

        // Load id in vanilla slot
        LoadVanillaId(aModelId, aData->modelId);

        return aData;
    }

public:
    struct GraphNode *Load(enum ModelLoadType aModelLoadType, enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, const char *aName, u8 aLayer, struct GraphNode *aGraphNode) {
        if (!aAsset) { return NULL; }

        // Sanity check pool
        if (aModelPool >= MODEL_POOL_MAX) { return NULL; }

        // Check pools to see if there already is a graph node for this asset
        // Can only check pools with lower indices, starting from permanent
        // Ignore this for area layouts (they are always reloaded)
        struct ModelExtendedData *data = GetData(aAsset);
        if (data) {
            enum ModelPool modelPoolMax = (enum ModelPool) ((size_t) aModelPool + 1);
            struct GraphNode *node = data->GetGraphNode(modelPoolMax);
            if (node) {
                LoadVanillaId(aModelId, data->modelId);
                return node;
            }
        }

        // Load model data
        data = LoadData(data, aModelId, aAsset, aName, aLayer);
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
        if (aModelId != E_MODEL_AREA_GEO) {
            mNodeToIdPool[node] = { data->modelId, aModelPool };
        }

        LOG_INFO("[Models] Load: Successfully loaded model in %s pool: %s (%s)", ((const char *[]){ "permanent", "session", "level" })[aModelPool], DynOS_Model_IdName(data->modelId), data->GetName());

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
    std::map<enum ModelExtendedId, enum ModelExtendedId> mIdReferences;
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
        [aDataName](const struct ModelExtendedData &data) { return strcmp(data.GetName(), aDataName) == 0; },
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
        return data->GetName();
    }
    return NULL;
}

const GeoLayout *DynOS_Builtin_LvlGeo_GetFromName(const char *aDataName) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aDataName](const struct ModelExtendedData &data) { return strcmp(data.GetName(), aDataName) == 0; },
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
        return data->GetName();
    }
    return NULL;
}

  //////////
 // Load //
//////////

struct GraphNode *DynOS_Model_LoadGeoLayout(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, const char *aName) {
    return sModels.Load(MLT_GEO_LAYOUT, aModelId, aModelPool, aAsset, aName, 0xFF, NULL);
}

struct GraphNode *DynOS_Model_LoadDisplayList(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, u8 aLayer) {
    return sModels.Load(MLT_DISPLAY_LIST, aModelId, aModelPool, aAsset, NULL, aLayer, NULL);
}

struct GraphNode *DynOS_Model_LoadGraphNode(enum ModelExtendedId aModelId, enum ModelPool aModelPool, const void *aAsset, struct GraphNode *aNode) {
    return sModels.Load(MLT_GRAPH_NODE, aModelId, aModelPool, aAsset, NULL, 0xFF, aNode);
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
        LOG_WARNING("DynOS_Model_GetGraphNode: Trying to load model with existing data now: %s", DynOS_Model_IdName(data->modelId));
        return sModels.Load(data->layer != 0xFF ? MLT_DISPLAY_LIST : MLT_GEO_LAYOUT, E_MODEL_NONE, MODEL_POOL_SESSION, data->asset, data->GetName(), data->layer, NULL);
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
        return data->GetName();
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
