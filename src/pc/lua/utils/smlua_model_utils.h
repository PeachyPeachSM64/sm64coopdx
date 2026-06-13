#ifndef SMLUA_MODEL_UTILS_H
#define SMLUA_MODEL_UTILS_H

#include "types.h"

/* |description|Gets the model extended id of a geo layout `name`|descriptionEnd| */
enum ModelExtendedId smlua_model_util_get_id(const char *name);

/* |description|Gets the geo layout name of a `modelId`|descriptionEnd| */
const char *smlua_model_util_get_name(enum ModelExtendedId modelId);

/* |description|Forget about a `modelId`, so its geo layout can be reloaded later. Has no effect on permanent models.|descriptionEnd| */
bool smlua_model_util_forget(enum ModelExtendedId modelId);

#endif
