/*
 * This file is part of the coreboot project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ROMSTAGE_H__
#define __ARCH_ROMSTAGE_H__

#include <arch/cpu.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Support setting up a stack frame consisting of MTRR information
 * for use in bootstrapping the caching attributes after cache-as-ram
 * is torn down.
 */

struct postcar_frame {
	uintptr_t stack;
	uint32_t upper_mask;
	int max_var_mtrrs;
	int num_var_mtrrs;
	int skip_common_mtrr;
};

/*
 * Initialize postcar_frame object allocating stack from cbmem,
 * with stack_size == 0, default 4 KiB is allocated.
 * Returns 0 on success, < 0 on error.
 */
int postcar_frame_init(struct postcar_frame *pcf, size_t stack_size);

/*
 * Add variable MTRR covering the provided range with MTRR type.
 */
void postcar_frame_add_mtrr(struct postcar_frame *pcf,
				uintptr_t addr, size_t size, int type);

/*
 * Add variable MTRR covering the memory-mapped ROM with given MTRR type.
 */
void postcar_frame_add_romcache(struct postcar_frame *pcf, int type);

/*
 * Add a common MTRR setup most platforms will have as a subset.
 */
void postcar_frame_common_mtrrs(struct postcar_frame *pcf);

/*
 * Push used MTRR and Max MTRRs on to the stack
 * and return pointer to stack top.
 */
void *postcar_commit_mtrrs(struct postcar_frame *pcf);

/*
 * Load and run a program that takes control of execution that
 * tears down CAR and loads ramstage. The postcar_frame object
 * indicates how to set up the frame. If caching is enabled at
 * the time of the call it is up to the platform code to handle
 * coherency with dirty lines in the cache using some mechansim
 * such as platform_prog_run() because run_postcar_phase()
 * utilizes prog_run() internally.
 */
void run_postcar_phase(struct postcar_frame *pcf);

/*
 * Systems without a native coreboot cache-as-ram teardown may implement
 * this to use an alternate method.
 */
void late_car_teardown(void);

#endif /* __ARCH_ROMSTAGE_H__ */
