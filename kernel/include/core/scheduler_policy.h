#ifndef _SCHEDULER_POLICY_H_
#define _SCHEDULER_POLICY_H_

#include "core/cpu.h"

Task *scheduler_policy_pick_next(Cpu *cpu);

#endif // _SCHEDULER_POLICY_H_
