/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2013 Google Inc.
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

#ifndef _BAYTRAIL_ROMSTAGE_H_
#define _BAYTRAIL_ROMSTAGE_H_

#if !defined(__PRE_RAM__)
#error "Don't include romstage.h from a ramstage compilation unit!"
#endif

#include <stdint.h>
#include <arch/cpu.h>
#include <soc/mrc_wrapper.h>

struct romstage_params {
	struct mrc_params *mrc_params;
};

void mainboard_romstage_entry_rp(struct romstage_params *params);
void romstage_common(struct romstage_params *params);

void raminit(struct mrc_params *mp, int prev_sleep_state);
void gfx_init(void);
void tco_disable(void);
void punit_init(void);
void byt_config_com1_and_enable(void);

#endif /* _BAYTRAIL_ROMSTAGE_H_ */
