#ifndef DYNOS_MODELS_H
#define DYNOS_MODELS_H

#define MAX_BUILTIN_LEVEL_GEOS 1000

// E_MODEL__XXX_ are delimiters, not actual model ids
enum ModelExtendedId {
    E_MODEL_NONE = 0,
    E_MODEL__VANILLA_MIN_ = 0x01,
    E_MODEL__VANILLA_MAX_ = 0xFF,

#define MODEL_EXTENDED_GEO(_modelId_, ...) _modelId_,
#define MODEL_EXTENDED_DL(_modelId_, ...) _modelId_,
#define MODEL_EXTENDED_LVL(...)
#include "data/dynos_models_builtin.inl"
#undef MODEL_EXTENDED_GEO
#undef MODEL_EXTENDED_DL
#undef MODEL_EXTENDED_LVL

    E_MODEL_MAX,
    E_MODEL__EXTENDED_END_      = E_MODEL_MAX,
    E_MODEL__VANILLA_END_       = E_MODEL__VANILLA_MAX_ + 1,
    E_MODEL__EXTENDED_START_    = E_MODEL__VANILLA_END_,

    E_MODEL__LEVEL_GEO_START_   = E_MODEL__EXTENDED_END_,
    E_MODEL__LEVEL_GEO_END_     = E_MODEL__LEVEL_GEO_START_ + MAX_BUILTIN_LEVEL_GEOS,

    E_MODEL__CUSTOM_START_      = E_MODEL__LEVEL_GEO_END_ + 1,

    // Special slots
    E_MODEL_AREA_GEO            = E_MODEL__CUSTOM_START_ - 1,
    E_MODEL_DYNOS_PACK          = E_MODEL__CUSTOM_START_ + 0,
    E_MODEL_MOD_ACTOR           = E_MODEL__CUSTOM_START_ + 1,
};

// The lower the index, the higher the lifetime
enum ModelPool {
    MODEL_POOL_PERMANENT,
    MODEL_POOL_SESSION,
    MODEL_POOL_LEVEL,

    MODEL_POOL_MAX,
};

#endif
