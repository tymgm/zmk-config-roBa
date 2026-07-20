/*
 * Runtime-adjustable trackball settings for the PMW3610 driver.
 * Values live in RAM, are used by the driver on every motion event,
 * and are persisted with the Zephyr settings subsystem so they survive
 * power cycles. Added by roBaish Studio.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#if IS_ENABLED(CONFIG_SETTINGS)
#include <zephyr/settings/settings.h>
#endif

#include "pmw3610_runtime.h"

LOG_MODULE_REGISTER(pmw3610_rt, LOG_LEVEL_INF);

/* fall back to sane defaults when the sensor Kconfig options are not present
 * (e.g. on the peripheral half, which has no trackball) */
#ifndef CONFIG_PMW3610_CPI
#define CONFIG_PMW3610_CPI 800
#endif
#ifndef CONFIG_PMW3610_CPI_DIVIDOR
#define CONFIG_PMW3610_CPI_DIVIDOR 1
#endif
#ifndef CONFIG_PMW3610_AUTOMOUSE_TIMEOUT_MS
#define CONFIG_PMW3610_AUTOMOUSE_TIMEOUT_MS 400
#endif
#ifndef CONFIG_PMW3610_SCROLL_TICK
#define CONFIG_PMW3610_SCROLL_TICK 20
#endif

#define CPI_STEP 200
#define CPI_MIN 200
#define CPI_MAX 3200
#define AMTO_STEP 100
#define AMTO_MIN 100
#define AMTO_MAX 5000
#define TICK_STEP 4
#define TICK_MIN 1
#define TICK_MAX 100

static int32_t rt_cpi = CONFIG_PMW3610_CPI;
static int32_t rt_amto = CONFIG_PMW3610_AUTOMOUSE_TIMEOUT_MS;
static int32_t rt_tick = CONFIG_PMW3610_SCROLL_TICK;

int32_t pmw3610_rt_cpi(void) { return rt_cpi; }
int32_t pmw3610_rt_cpi_dividor(void) { return CONFIG_PMW3610_CPI_DIVIDOR; }
int32_t pmw3610_rt_automouse_timeout_ms(void) { return rt_amto; }
int32_t pmw3610_rt_scroll_tick(void) { return rt_tick; }

static int32_t clamp32(int32_t v, int32_t lo, int32_t hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

#if IS_ENABLED(CONFIG_SETTINGS)

static int rt_settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    int32_t val;
    if (len != sizeof(val)) {
        return -EINVAL;
    }
    if (read_cb(cb_arg, &val, sizeof(val)) != sizeof(val)) {
        return -EINVAL;
    }
    if (settings_name_steq(name, "cpi", NULL)) {
        rt_cpi = clamp32(val, CPI_MIN, CPI_MAX);
    } else if (settings_name_steq(name, "amto", NULL)) {
        rt_amto = clamp32(val, AMTO_MIN, AMTO_MAX);
    } else if (settings_name_steq(name, "tick", NULL)) {
        rt_tick = clamp32(val, TICK_MIN, TICK_MAX);
    }
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(pmw3610_rt, "pmw3610rt", NULL, rt_settings_set, NULL, NULL);

static void rt_save(const char *key, int32_t val) {
    char path[32];
    snprintk(path, sizeof(path), "pmw3610rt/%s", key);
    settings_save_one(path, &val, sizeof(val));
}

static void rt_clear_saved(void) {
    settings_delete("pmw3610rt/cpi");
    settings_delete("pmw3610rt/amto");
    settings_delete("pmw3610rt/tick");
}

static int rt_init(void) {
    settings_subsys_init();
    settings_load_subtree("pmw3610rt");
    LOG_INF("runtime trackball settings: cpi=%d amto=%d tick=%d", rt_cpi, rt_amto, rt_tick);
    return 0;
}

SYS_INIT(rt_init, APPLICATION, 99);

#else /* !CONFIG_SETTINGS */

static void rt_save(const char *key, int32_t val) {
    ARG_UNUSED(key);
    ARG_UNUSED(val);
}
static void rt_clear_saved(void) {}

#endif /* CONFIG_SETTINGS */

int pmw3610_rt_adjust(uint32_t action) {
    switch (action) {
    case PMW_RT_RESET:
        rt_cpi = CONFIG_PMW3610_CPI;
        rt_amto = CONFIG_PMW3610_AUTOMOUSE_TIMEOUT_MS;
        rt_tick = CONFIG_PMW3610_SCROLL_TICK;
        rt_clear_saved();
        break;
    case PMW_RT_CPI_UP:
        rt_cpi = clamp32(rt_cpi + CPI_STEP, CPI_MIN, CPI_MAX);
        rt_save("cpi", rt_cpi);
        break;
    case PMW_RT_CPI_DOWN:
        rt_cpi = clamp32(rt_cpi - CPI_STEP, CPI_MIN, CPI_MAX);
        rt_save("cpi", rt_cpi);
        break;
    case PMW_RT_AMTO_UP:
        rt_amto = clamp32(rt_amto + AMTO_STEP, AMTO_MIN, AMTO_MAX);
        rt_save("amto", rt_amto);
        break;
    case PMW_RT_AMTO_DOWN:
        rt_amto = clamp32(rt_amto - AMTO_STEP, AMTO_MIN, AMTO_MAX);
        rt_save("amto", rt_amto);
        break;
    case PMW_RT_SCROLL_SLOWER:
        rt_tick = clamp32(rt_tick + TICK_STEP, TICK_MIN, TICK_MAX);
        rt_save("tick", rt_tick);
        break;
    case PMW_RT_SCROLL_FASTER:
        rt_tick = clamp32(rt_tick - TICK_STEP, TICK_MIN, TICK_MAX);
        rt_save("tick", rt_tick);
        break;
    default:
        LOG_WRN("unknown adjust action %u", action);
        return -EINVAL;
    }
    LOG_INF("trackball settings now: cpi=%d amto=%d tick=%d", rt_cpi, rt_amto, rt_tick);
    return 0;
}
