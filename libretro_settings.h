#ifndef __LIBRETRO_SETTINGS_HDR__
#define __LIBRETRO_SETTINGS_HDR__

#include <boolean.h>

enum
{
	SETTING_GUN_CROSSHAIR_OFF,
	SETTING_GUN_CROSSHAIR_CROSS,
	SETTING_GUN_CROSSHAIR_DOT,

	SETTING_GUN_CROSSHAIR_LAST
};

enum
{
	SETTING_GUN_INPUT_LIGHTGUN,
	SETTING_GUN_INPUT_POINTER
};

extern int setting_region;
extern int setting_cart;
extern bool setting_smpc_autortc;
extern int setting_smpc_autortc_lang;
extern int setting_initial_scanline;
extern int setting_initial_scanline_pal;
extern int setting_last_scanline;
extern int setting_last_scanline_pal;
extern int setting_gun_crosshair;
extern int setting_gun_input;
extern bool setting_disc_test;
extern bool setting_multitap_port1;
extern bool setting_multitap_port2;
extern bool opposite_directions;
extern bool setting_midsync;
extern bool setting_mpeg_card;
extern bool setting_jit_scu;
extern bool setting_jit_scsp;
extern bool setting_sh2_jit;
extern int  setting_sh2_interleave;   /* 0 = exact (per instruction); else cycles the master may run ahead of the slave */

#endif
