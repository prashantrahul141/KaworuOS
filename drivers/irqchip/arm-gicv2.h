#ifndef _ARM_GIC_V2_H_
#define _ARM_GIC_V2_H_

#include "common/manager.h"

errno_t armgicv2_probe(Device *device);

errno_t armgicv2_remove(Device *device);

#endif // _ARM_GIC_V2_H_
