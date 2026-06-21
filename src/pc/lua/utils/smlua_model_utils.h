#ifndef SMLUA_MODEL_UTILS_H
#define SMLUA_MODEL_UTILS_H

#include "types.h"

/* |description|Gets the model id of the geo layout `name`|descriptionEnd| */
enum ModelExtendedId smlua_model_util_get_id(const char *name);

/* |description|Gets the geo layout name corresponding to a `modelId`|descriptionEnd| */
const char *smlua_model_util_get_name(enum ModelExtendedId modelId);

#endif
