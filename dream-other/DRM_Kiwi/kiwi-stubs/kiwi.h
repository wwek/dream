/*
--------------------------------------------------------------------------------
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
--------------------------------------------------------------------------------
*/

// Copyright (c) 2015-2025 John Seamons, ZL4VO/KF6VO

#pragma once

#include "types.h"
#include "str.h"
#include "printf.h"
#include "datatypes.h"
#include "coroutines.h"
#include "cfg.h"
#include "non_block.h"

typedef enum { KiwiSDR_1 = 1, KiwiSDR_2 = 2 } model_e;

typedef enum { PLATFORM_BBG_BBB = 0, PLATFORM_BBAI = 1, PLATFORM_BBAI_64 = 2, PLATFORM_BYAI = 3, PLATFORM_RPI = 4 } platform_e;
const char * const platform_s[] = { "beaglebone-black", "bbai", "bbai64", "byai", "rpi" };

typedef enum { DAILY_RESTART_NO = 0, DAILY_RESTART = 1, DAILY_REBOOT = 2} daily_restart_e;

typedef struct {
    model_e model;
    platform_e platform;
    
    int current_nusers;
    bool dbgUs;
    
    #define RESTART_DELAY_30_SEC 1
    #define RESTART_DELAY_MAX 7
    int restart_delay;

    bool hw;
    bool ext_clk;
    bool isWB;
    bool allow_admin_conns;
    bool spectral_inversion, spectral_inversion_lockout;
    bool require_id;
    bool test_marine_mobile;
    bool disable_recent_changes;
    u4_t vr, vc;        // virus detection info
    
    int current_espeed;
    
    float rf_attn_dB;

    bool snr_initial_meas_done, snr_meas_active, snr_disable_filter;
    
    bool RsId;
    
    int CAT_fd, CAT_ch;
    
    // lat/lon returned by ipinfo lookup
	bool ipinfo_ll_valid;
	float ipinfo_lat, ipinfo_lon;
	char *ipinfo_loc;
	#define	LEN_GRID6   (6 + SPACE_FOR_NULL)
	char ipinfo_grid6[LEN_GRID6];
	
	char gps_latlon[32];
	char gps_grid6[LEN_GRID6];
	
	// low-res lat/lon from timezone process
	int lowres_lat, lowres_lon;
	
	daily_restart_e daily_restart;
} kiwi_t;

extern kiwi_t kiwi;

extern int version_maj, version_min;

extern bool background_mode, need_hardware, is_multi_core, any_preempt_autorun, spi_show_stats,
	DUC_enable_start, rev_enable_start, web_nocache, kiwi_reg_debug, cmd_debug, gen_debug,
	gps_e1b_only, disable_led_task, debug_printfs, force_camp, ecpu_stack_check,
	log_local_ip, DRM_enable, have_snd_users, admin_keepalive;

extern int wf_sim, wf_real, wf_time, ev_dump, wf_flip, wf_exit, wf_start, down,
	meas, monitors_max, rx_yield, gps_chans, wf_max, cfg_no_wf, do_gps, do_sdr, wf_olap,
	spi_clkg, spi_speed, spi_mode, spi_delay, spi_no_async, bg, dx_print, snr_meas, wf_full_rate,
	port, print_stats, ecpu_cmds, ecpu_tcmds, serial_number, ip_limit_mins, is_locked, test_flag, n_camp,
	use_spidev, inactivity_timeout_mins, S_meter_cal, waterfall_cal, debian_ver,
	utc_offset, dst_offset, reg_kiwisdr_com_status, kiwi_reg_lo_kHz, kiwi_reg_hi_kHz,
	debian_maj, debian_min, gps_debug, gps_var, gps_lo_gain, gps_cg_gain, use_foptim, web_caching_debug,
	drm_nreg_chans, snr_meas_interval_min,
	snr_all, snr_HF, ant_connected, spi_test, spidev_maj, spidev_min;

extern char **main_argv;

extern u4_t ov_mask, snd_intr_usec;
extern float max_thr;
extern double ui_srate_Hz, ui_srate_kHz;
extern TYPEREAL DC_offset_I, DC_offset_Q;
extern kstr_t *cpu_stats_buf;
extern char *tzone_id, *tzone_name;
extern cfg_t cfg_ipl;
extern lock_t spi_lock;

extern int p0, p1, p2;
extern double p_f[8];
extern int p_i[8];


typedef enum { DOM_SEL_NAM=0, DOM_SEL_DUC=1, DOM_SEL_PUB=2, DOM_SEL_SIP=3, DOM_SEL_REV=4 } dom_sel_e;
const char * const dom_type_s[] = { "NAM", "DUC", "PUB", "SIP", "REV" };

typedef enum { RX4_WF4=0, RX8_WF2=1, RX3_WF3=2, RX14_WF0=3 } firmware_e;

#define	KEEPALIVE_SEC		    60
#define KEEPALIVE_SEC_NO_AUTH   20      // don't hang the rx channel as long if waiting for password entry

// print_stats
#define STATS_GPS       0x01
#define STATS_GPS_SOLN  0x02
#define STATS_TASK      0x04

#define IDENT_LEN_MIN   8
#define IDENT_LEN_NOM   16

void kiwi_restart();

void update_freqs(bool *update_cfg = NULL);
void update_vars_from_config(bool called_at_init = false);
void cfg_adm_transition();

#define DUMP_PANIC true
void dump(bool doPanic = false);

void c2s_sound_init();
void c2s_sound_setup(void *param);
void c2s_sound(void *param);
void c2s_sound_shutdown(void *param);

void c2s_waterfall_once();
void c2s_waterfall_init();
void c2s_waterfall_compression(int rx_chan, bool compression);
void c2s_waterfall_no_sync(int rx_chan, bool no_sync);
void c2s_waterfall_setup(void *param);
void c2s_waterfall(void *param);
void c2s_waterfall_shutdown(void *param);

void c2s_mon_setup(void *param);
void c2s_mon(void *param);

void c2s_admin_setup(void *param);
void c2s_admin_shutdown(void *param);
void c2s_admin(void *param);

void c2s_mfg_setup(void *param);
void c2s_mfg(void *param);

extern bool update_pending, update_in_progress, backup_in_progress;

extern bool sd_copy_in_progress;

void stat_task(void *param);
