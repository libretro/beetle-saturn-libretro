#ifndef __INPUT_H__
#define __INPUT_H__

#include "libretro.h"
#include "mednafen/state.h"

/* input.c's entry points are C functions; the surrounding
 * extern "C" guard is defensive header hygiene for any future
 * C++ consumer.  All current callers (libretro.c, ss.c) are C
 * and consume these through normal C linkage; the guard is a
 * no-op at every current compile. */
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

/* Persist the per-port libretro device type (input_type[]) in the
 * save-state so a cross-device rewind / load can re-establish the
 * IOPort device mapping *before* SMPC_StateAction tries to load
 * each port's IODevice fields.  Without this, loading a state
 * saved under one device type while the live core has assigned a
 * different one Power()-cycles the current device and the saved
 * device's IODevice section in the state buffer is ignored.
 * Called from LibRetro_StateAction before SMPC_StateAction.  See
 * issue #21 for the original rewind failure mode. */
int input_StateActionDevices( StateMem* sm, const unsigned load, const bool data_only );

void input_multitap( int port, bool enabled );

#ifdef __cplusplus
}
#endif

#endif
