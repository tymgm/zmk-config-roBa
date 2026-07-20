/*
 * Runtime-adjustable trackball settings (CPI / automouse timeout / scroll tick)
 * for the PMW3610 driver. Added by roBaish Studio.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

/* actions usable as the parameter of the zmk,behavior-pmw3610-config behavior */
#define PMW_RT_RESET 0        /* forget saved values, back to firmware defaults */
#define PMW_RT_CPI_UP 1       /* cursor speed +200 CPI */
#define PMW_RT_CPI_DOWN 2     /* cursor speed -200 CPI */
#define PMW_RT_AMTO_UP 3      /* automouse layer timeout +100 ms */
#define PMW_RT_AMTO_DOWN 4    /* automouse layer timeout -100 ms */
#define PMW_RT_SCROLL_SLOWER 5 /* more ball motion needed per scroll step */
#define PMW_RT_SCROLL_FASTER 6 /* less ball motion needed per scroll step */

int32_t pmw3610_rt_cpi(void);
int32_t pmw3610_rt_cpi_dividor(void);
int32_t pmw3610_rt_automouse_timeout_ms(void);
int32_t pmw3610_rt_scroll_tick(void);

/* apply one PMW_RT_* action; returns 0 on success */
int pmw3610_rt_adjust(uint32_t action);
