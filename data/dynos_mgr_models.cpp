#include <vector>
#include "dynos.cpp.h"

extern "C" {
#include "pc/debuglog.h"
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
        if (modelId >= E_MODEL_CUSTOM_START) {
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

#include "dynos_models_builtin.inl"

class ModelExtendedManager : NoCopy {

public:
    inline ModelExtendedManager() {
        DynOS_Model_InitModelsData(mData, mAssetToId);
    }

public:
    template <typename EqualFunc>
    struct ModelExtendedData *GetData(const EqualFunc& aEqualFunc, enum ModelExtendedId aModelIdStart, enum ModelExtendedId aModelIdEnd) {
        for (auto &kv : mData) {
            enum ModelExtendedId modelId = kv.first;
            if (modelId < aModelIdStart) { continue; }
            if (modelId >= aModelIdEnd) { break; }

            auto &data = kv.second;
            if (aEqualFunc(data)) {
                return &data;
            }
        }
        return NULL;
    }

    struct ModelExtendedData *GetData(enum ModelExtendedId aModelId) {
        // Convert vanilla id to extended id
        if (aModelId > E_MODEL_NONE && aModelId < E_MODEL_VANILLA_END) {
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
        if (aVanillaId > E_MODEL_NONE && aVanillaId < E_MODEL_VANILLA_END) {
            mLoadedModelIds[aVanillaId] = aModelId;
        } else if (aVanillaId != E_MODEL_NONE) {
            LOG_WARNING("[Models] LoadVanillaId: Loading an asset with a model extended id outside of the vanilla model slot range [%u, %u] has no effect: %u", E_MODEL_NONE + 1, E_MODEL_VANILLA_END - 1, aVanillaId);
        }
    }

    struct ModelExtendedData *LoadData(struct ModelExtendedData *aData, enum ModelExtendedId aModelId, const void *aAsset, const char *aName, u8 aLayer, bool isAreaLayout) {

        // Area layout
        // Always load in specific ids
        if (isAreaLayout) {
            if (aModelId >= E_MODEL_LEVEL_AREA_START && aModelId < E_MODEL_LEVEL_AREA_END) {
                return &mData[aModelId];
            }
            LOG_ERROR("[Models] LoadData: Trying to load an area geo layout with a model extended id outside of the range [%u, %u]: %u", E_MODEL_LEVEL_AREA_START, E_MODEL_LEVEL_AREA_END - 1, aModelId);
            return NULL;
        }

        // No existing data for this asset, create a new entry
        if (!aData) {
            enum ModelExtendedId customId = E_MODEL_CUSTOM_START;
            while (mData.find(customId) != mData.end()) {
                customId = (enum ModelExtendedId) ((size_t) customId + 1);
            }

            mData[customId] = {
                .modelId = customId,
                .name = aName ? strdup(aName) : NULL,
                .asset = aAsset,
                .layer = aLayer,
            };
            aData = &mData[customId];

            LOG_INFO("[Models] LoadData: Created new entry [ ID: %04u | ASSET: %016llX | NAME: %s ]",
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
                if (data.modelId >= E_MODEL_CUSTOM_START && !data.GetGraphNode()) {
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
    enum ModelExtendedId mLoadedModelIds[E_MODEL_VANILLA_END];
    struct DynamicPool *mPools[MODEL_POOL_MAX];
};

static ModelExtendedManager sModels;

  //////////////
 // Built-in //
//////////////

const GeoLayout *DynOS_Builtin_Actor_GetFromName(const char *aDataName) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aDataName](const struct ModelExtendedData &data) { return strcmp(data.name, aDataName) == 0; },
        E_MODEL_EXTENDED_START,
        E_MODEL_EXTENDED_END
    );
    if (data) {
        return (const GeoLayout *) data->asset;
    }
    return NULL;
}

const char *DynOS_Builtin_Actor_GetFromData(const GeoLayout *aData) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aData](const struct ModelExtendedData &data) { return data.asset == (const void *) aData; },
        E_MODEL_EXTENDED_START,
        E_MODEL_EXTENDED_END
    );
    if (data) {
        return data->name;
    }
    return NULL;
}

const GeoLayout *DynOS_Builtin_LvlGeo_GetFromName(const char *aDataName) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aDataName](const struct ModelExtendedData &data) { return strcmp(data.name, aDataName) == 0; },
        E_MODEL_LEVEL_GEO_START,
        E_MODEL_LEVEL_GEO_END
    );
    if (data) {
        return (const GeoLayout *) data->asset;
    }
    return NULL;
}

const char *DynOS_Builtin_LvlGeo_GetFromData(const GeoLayout *aData) {
    const struct ModelExtendedData *data = sModels.GetData(
        [aData](const struct ModelExtendedData &data) { return data.asset == (const void *) aData; },
        E_MODEL_LEVEL_GEO_START,
        E_MODEL_LEVEL_GEO_END
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

enum ModelPool DynOS_Model_GetModelPool(struct GraphNode *aNode) {
    if (!aNode) {
        return MODEL_POOL_MAX;
    }

    return sModels.GetModelPool(aNode);
}

void DynOS_Model_ClearPool(enum ModelPool aModelPool) {
    sModels.ClearPool(aModelPool);
}
