#ifndef _LIMINE_RESPONSES_H_
#define _LIMINE_RESPONSES_H_

#include "types.h"

/* some responses are allocated by limine, we need to save them in our own
 * memory in order to preserve them. */
void limine_responses_save(void);

volatile u64 *limine_base_revision(void);
struct limine_dtb_response *limine_device_tree(void);
volatile struct limine_memmap_response *limine_memmap(void);
volatile struct limine_hhdm_response *limine_hhdm(void);
volatile struct limine_executable_address_response *limine_kernel_address(void);
volatile struct limine_framebuffer_response *limine_framebuffer(void);
struct limine_mp_response *limine_mp(void);

#endif // _LIMINE_RESPONSES_H_
