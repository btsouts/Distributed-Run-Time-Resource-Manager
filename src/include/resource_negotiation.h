#ifndef __RESOURCE_NEGOTIATION_H__
#define __RESOURCE_NEGOTIATION_H__

#include "structs.h"

int offer_cores(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id);
#endif
