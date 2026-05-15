#ifndef __INPUT_H__
#define __INPUT_H__

#include "libretro.h"
#include "mednafen/state.h"

/* input.cpp -> input.c (this conversion): the input_* entry points
 * are defined in C now, but libretro.cpp (C++) calls them.  Force
 * C linkage on both sides via the extern "C" wrap so the C++ caller
 * doesn't name-mangle the references and the linker sees the same
 * unmangled symbols input.c emits.  Same pattern as ss.h / smpc.h /
 * sound.h after the SS-core C-compat omnibus.  Without this, an
 * LTO+mingw build fails with undefined-reference errors -- the
 * a7fa45b class of bug. */
#ifdef __cplusplus
extern "C" {
#endif

// These input routines tell libretro about Saturn peripherals
// and map input from the abstract 'retropad' into Saturn land.

void input_init_env( retro_environment_t environ_cb );

void input_init(void);

void input_set_geometry( unsigned width, unsigned height );

void input_set_env( retro_environment_t environ_cb );

void input_set_deadzone_stick( int percent );
void input_set_deadzone_trigger( int percent );
void input_set_mouse_sensitivity( int percent );

void input_update( retro_input_state_t input_state_cb);
void input_update_with_bitmasks( retro_input_state_t input_state_cb );

// save state function for input
int input_StateAction( StateMem* sm, const unsigned load, const bool data_only );

void input_multitap( int port, bool enabled );

#ifdef __cplusplus
}
#endif

#endif
