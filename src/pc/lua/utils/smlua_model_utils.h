#ifndef SMLUA_MODEL_UTILS_H
#define SMLUA_MODEL_UTILS_H

#include "types.h"

/* |description|Gets the model id of the geo layout `name`|descriptionEnd| */
enum ModelExtendedId smlua_model_util_get_id(const char *name);

/* |description|Loads the geo layout `name` as `modelId`|descriptionEnd| */
// bool smlua_model_util_load_id(enum ModelExtendedId modelId, const char *name);
// TODO

/* |description|Gets the geo layout name corresponding to a `modelId`|descriptionEnd| */
const char *smlua_model_util_get_name(enum ModelExtendedId modelId);

#endif
