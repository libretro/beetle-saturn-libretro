
#include "libretro.h"
#include "libretro_settings.h"
#include "mednafen/mednafen-types.h"
/* log_cb is the only symbol this TU uses from mednafen/git.h.
 * Forward-declare directly so this TU's include surface stays
 * minimal: retro_log_printf_t comes from libretro.h (above) and
 * the defining TU for log_cb is libretro.c.  Pulling in git.h
 * here would also drag in emuspec.h / mdfn_gameinfo.h / state.h
 * which input.c doesn't otherwise need. */
extern retro_log_printf_t log_cb;

#include "mednafen/ss/ss.h"
#include "mednafen/ss/smpc.h"
#include "mednafen/ss/cart.h"   /* CART_STV (ActiveCartType compare for ST-V coin wireup) */
#include "mednafen/ss/stvio.h"  /* STVIO_InsertCoin */
#include "mednafen/state.h"
#include <stdio.h>

//------------------------------------------------------------------------------
// Locals
//------------------------------------------------------------------------------

#define MAX_CONTROLLERS		12 /* 2x 6 player adaptors */

static retro_environment_t environ_cb; /* cached during input_init_env */

static unsigned players				= 2;

static int astick_deadzone			= 0;
static int trigger_deadzone			= 0;
static const int TRIGGER_MAX			= 0xFFFF;
static int32_t mouse_sensitivity_q16		= (1 << 16); /* 1.0 in Q16 */

static unsigned geometry_width			= 0;
static unsigned geometry_height			= 0;

// Pointer/touchscreen state, per-player. Previously these were single
// globals shared across all players, which broke multi-player gun setups
// and caused player 0's hold state to clobber player 1's input.
// The release-frame debounce -- holding the gun position for one frame
// after touch release so that a quick light tap reads as a real shot --
// is implemented purely by `pointer_pressed[iplayer]` going 0->1 on
// press and 1->0 on release, with the release frame writing the latched
// position into INPUT_DATA before the input loop advances.  Upstream
// once supported a multi-frame latch via a `POINTER_PRESSED_CYCLES`
// constant + `pointer_cycles_after_released[]` counter, but the cycle
// count had been pinned at 1 for some time and the >1-frame branch was
// unreachable; both have been removed.
static int pointer_pressed[ MAX_CONTROLLERS ]		= {0};
static int pointer_pressed_last_x[ MAX_CONTROLLERS ]	= {0};
static int pointer_pressed_last_y[ MAX_CONTROLLERS ]	= {0};

typedef union
{
	uint8_t u8[ 32 ];
	uint16_t gun_pos[ 2 ];
	uint16_t buttons;
} INPUT_DATA;

// Controller state buffer (per player)
static INPUT_DATA input_data[ MAX_CONTROLLERS ] = {0};

// Controller type (per player)
static uint32_t input_type[ MAX_CONTROLLERS ]	= {0};


#define INPUT_MODE_3D_PAD_ANALOG		( 1 << 0 ) // Set means analog mode.
#define INPUT_MODE_3D_PAD_PREVIOUS_MASK		( 1 << 1 ) // Edge trigger helper.
#define INPUT_MODE_MISSION_THROTTLE_LATCH	( 1 << 2 ) // Latch throttle enabled?
#define INPUT_MODE_MISSION_THROTTLE_PREV	( 1 << 3 ) // Edge trigger helper.

#define INPUT_MODE_DEFAULT				0
#define INPUT_MODE_DEFAULT_3D_PAD		INPUT_MODE_3D_PAD_ANALOG

// Mode switch for 3D Control Pad (per player)
static uint16_t input_mode[ MAX_CONTROLLERS ] 		= {0};
static int16_t input_throttle_latch[ MAX_CONTROLLERS ]	= {0};

/* ST-V coin-insert edge-detect state.  RETRO_DEVICE_ID_JOYPAD_SELECT
 * doubles as the 3D Pad mode switch on consumer Saturn games (cf.
 * input_map_3d_pad_mode_switch below) and as the coin button on ST-V
 * games -- the two uses can't conflict since ST-V games don't use
 * the 3D Pad.  prev_coin[port] tracks last frame's SELECT bit per
 * port; check_stv_coin_inserts edge-triggers on rising 0 -> 1 to
 * dispatch one STVIO_InsertCoin() per press, gated on ActiveCartType
 * == CART_STV.  Both player 1 and player 2 SELECTs feed the same
 * single CoinPending counter in stvio.c -- the ST-V coin slot is
 * one-per-cabinet, so cumulating presses is the right model. */
static bool prev_coin[ MAX_CONTROLLERS ] = {0};

/* ST-V "misc input" port (port index 12 in SMPC's port-number space).
 *
 * One byte, bit layout (matches mednafen/ss/stvio.c consumers):
 *   bit 0 - SS reset button (stvio's TransformInput clears this every
 *           frame after read, so we only ever set it transiently)
 *   bit 2 - Test    (operator-panel test button)
 *   bit 3 - Service (operator-panel service button -- free credit)
 *   bit 4 - Pause   (operator-panel pause button -- not all games)
 *
 * Polarity is "1 = pressed" on our side; stvio XORs into DataIn[]
 * which starts at 0xFF (active-low to the game) so the game sees
 * 1=unpressed / 0=pressed as expected.
 *
 * Mednafen's standalone wires this up via STVIO_SetInput(12, ...).
 * Our fork's input.c only iterated 0..MAX_CONTROLLERS-1 (= 0..11),
 * never port 12, so DPtr[12] stayed NULL throughout ST-V game
 * lifetime -- Test/Service/Pause were unreachable, and stvio.c
 * needed defensive NULL guards to avoid crashing.  Now we bind
 * this byte at input_init time when ActiveCartType == CART_STV.
 * update_stv_misc_inputs() populates the bits from per-frame poll
 * state; the NULL guards in stvio.c become belt-and-suspenders. */
static uint8_t input_stv_misc_byte = 0;

/* ActiveCartType lives in mednafen/ss/ss.c.  No header exposes it
 * (it's a TU-local global with extern decls inline at the usage
 * sites in ss.c itself), so input.c picks it up the same way. */
extern int ActiveCartType;

//------------------------------------------------------------------------------
// Supported Devices
//------------------------------------------------------------------------------

#define RETRO_DEVICE_SS_PAD       RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)
#define RETRO_DEVICE_SS_3D_PAD    RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 0)
#define RETRO_DEVICE_SS_WHEEL     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 1)
#define RETRO_DEVICE_SS_MISSION   RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 2)
#define RETRO_DEVICE_SS_MISSION2  RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 3)
#define RETRO_DEVICE_SS_TWINSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 4)
#define RETRO_DEVICE_SS_MOUSE     RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 0)
#define RETRO_DEVICE_SS_GUN_JP    RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 0)
#define RETRO_DEVICE_SS_GUN_US    RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 1)
/* Sega Saturn Keyboard (US 101-key PS/2 adapter).  The SMPC-side
 * IODevice already exists in mednafen/ss/smpc_iodevice.c
 * (IODevice_Keyboard_Create).  We expose it to RetroArch as a
 * KEYBOARD subclass so users can select it from the controller
 * type list, and we pump scan-code state into the 18-byte phys[]
 * bitmap that UpdateInput consumes.  Subclass 1 is reserved for
 * the JP keyboard (separate IDII layout) if/when it's exposed. */
#define RETRO_DEVICE_SS_KEYBOARD  RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_KEYBOARD, 0)

enum { INPUT_DEVICE_TYPES_COUNT = 1 /*none*/ + 10 }; /* <-- update me! */

static const struct retro_controller_description input_device_types[ INPUT_DEVICE_TYPES_COUNT ] =
{
	{ "Control Pad", RETRO_DEVICE_JOYPAD },
	{ "3D Control Pad", RETRO_DEVICE_SS_3D_PAD },
	{ "Virtua Gun", RETRO_DEVICE_SS_GUN_JP },
	{ "Stunner", RETRO_DEVICE_SS_GUN_US },
	{ "Mouse", RETRO_DEVICE_SS_MOUSE },
	{ "Arcade Racer", RETRO_DEVICE_SS_WHEEL },
	{ "Mission Stick", RETRO_DEVICE_SS_MISSION },
	{ "Dual Mission Sticks", RETRO_DEVICE_SS_MISSION2 }, /*"Panzer Dragoon Zwei" only!*/
	{ "Twin-Stick", RETRO_DEVICE_SS_TWINSTICK },
	{ "Keyboard", RETRO_DEVICE_SS_KEYBOARD },
	{ NULL, 0 },
};

static const struct retro_controller_info ports_no_6player[ 1 + 1 + 1 ] =
{
	/* port one */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	
	/* port two */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },

	{ 0 },
};

static const struct retro_controller_info ports_left_6player[ 6 + 1 + 1 ] =
{
	/* port one */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },

	/* port two: 6player adaptor */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	
	{ 0 },
};

static const struct retro_controller_info ports_right_6player[ 1 + 6 + 1 ] =
{
	/* port one */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	
	/* port two: 6player adaptor */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },

	{ 0 },
};

static const struct retro_controller_info ports_two_6player[ 6 + 6 + 1 ] =
{
	/* port one: 6player adaptor */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	
	/* port two: 6player adaptor */
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },
	{ input_device_types, INPUT_DEVICE_TYPES_COUNT },

	{ 0 },
};

/* Lookup table to select ports info for all combinations
   of 6player adaptor connection. */
static const struct retro_controller_info* ports_lut[ 4 ] =
{
	ports_no_6player,
	ports_left_6player,
	ports_right_6player,
	ports_two_6player
};


//------------------------------------------------------------------------------
// Mapping Helpers
//------------------------------------------------------------------------------

/* Control Pad (default) */
enum { INPUT_MAP_PAD_SIZE = 12 };
static const unsigned input_map_pad[ INPUT_MAP_PAD_SIZE ] =
{
	// libretro input				 at position	|| maps to Saturn		on bit
	//-----------------------------------------------------------------------------
	RETRO_DEVICE_ID_JOYPAD_L,		// L1			-> Z					0
	RETRO_DEVICE_ID_JOYPAD_X,		// X(top)		-> Y					1
	RETRO_DEVICE_ID_JOYPAD_Y,		// Y(left)		-> X					2
	RETRO_DEVICE_ID_JOYPAD_R2,		// R2			-> R					3
	RETRO_DEVICE_ID_JOYPAD_UP,		// Pad-Up		-> Pad-Up				4
	RETRO_DEVICE_ID_JOYPAD_DOWN,		// Pad-Down		-> Pad-Down				5
	RETRO_DEVICE_ID_JOYPAD_LEFT,		// Pad-Left		-> Pad-Left				6
	RETRO_DEVICE_ID_JOYPAD_RIGHT,		// Pad-Right		-> Pad-Right				7
	RETRO_DEVICE_ID_JOYPAD_A,		// A(right)		-> B					8
	RETRO_DEVICE_ID_JOYPAD_R,		// R1			-> C					9
	RETRO_DEVICE_ID_JOYPAD_B,		// B(down)		-> A					10
	RETRO_DEVICE_ID_JOYPAD_START,		// Start		-> Start				11
};

static const unsigned input_map_pad_left_shoulder =
	RETRO_DEVICE_ID_JOYPAD_L2;		// L2			-> L					15

/* 3D Control Pad */
enum { INPUT_MAP_3D_PAD_SIZE = 11 };
static const unsigned input_map_3d_pad[ INPUT_MAP_3D_PAD_SIZE ] =
{
	// libretro input				 at position	|| maps to Saturn		on bit
	//-----------------------------------------------------------------------------
	RETRO_DEVICE_ID_JOYPAD_UP,		// Pad-Up		-> Pad-Up				0
	RETRO_DEVICE_ID_JOYPAD_DOWN,		// Pad-Down		-> Pad-Down				1
	RETRO_DEVICE_ID_JOYPAD_LEFT,		// Pad-Left		-> Pad-Left				2
	RETRO_DEVICE_ID_JOYPAD_RIGHT,		// Pad-Right		-> Pad-Right				3
	RETRO_DEVICE_ID_JOYPAD_A,		// A(right)		-> B					4
	RETRO_DEVICE_ID_JOYPAD_R,		// R1			-> C					5
	RETRO_DEVICE_ID_JOYPAD_B,		// B(down)		-> A					6
	RETRO_DEVICE_ID_JOYPAD_START,		// Start		-> Start				7
	RETRO_DEVICE_ID_JOYPAD_L,		// L1			-> Z					8
	RETRO_DEVICE_ID_JOYPAD_X,		// X(top)		-> Y					9
	RETRO_DEVICE_ID_JOYPAD_Y,		// Y(left)		-> X					10
};

static const unsigned input_map_3d_pad_mode_switch = RETRO_DEVICE_ID_JOYPAD_SELECT;

/* Arcade Racer (wheel) */
enum { INPUT_MAP_WHEEL_BITSHIFT = 4 };
enum { INPUT_MAP_WHEEL_SIZE = 7 };
static const unsigned input_map_wheel[ INPUT_MAP_WHEEL_SIZE ] =
{
	// libretro input				 at position	|| maps to Saturn		on bit
	//-----------------------------------------------------------------------------
	RETRO_DEVICE_ID_JOYPAD_A,		// A(right)		-> B					4
	RETRO_DEVICE_ID_JOYPAD_R,		// R1			-> C					5
	RETRO_DEVICE_ID_JOYPAD_B,		// B(down)		-> A					6
	RETRO_DEVICE_ID_JOYPAD_START,		// Start		-> Start				7
	RETRO_DEVICE_ID_JOYPAD_L,		// L1			-> Z					8
	RETRO_DEVICE_ID_JOYPAD_X,		// X(top)		-> Y					9
	RETRO_DEVICE_ID_JOYPAD_Y,		// Y(left)		-> X					10
};

static const unsigned input_map_wheel_shift_left  = RETRO_DEVICE_ID_JOYPAD_L2;
static const unsigned input_map_wheel_shift_right = RETRO_DEVICE_ID_JOYPAD_R2;

/* Mission Stick */
enum { INPUT_MAP_MISSION_SIZE = 8 };
static const unsigned input_map_mission[ INPUT_MAP_MISSION_SIZE ] =
{
	// libretro input				 at position	|| maps to Saturn		on bit
	//-----------------------------------------------------------------------------
	RETRO_DEVICE_ID_JOYPAD_A,		// A(right)		-> B					0
	RETRO_DEVICE_ID_JOYPAD_R,		// R1			-> C					1
	RETRO_DEVICE_ID_JOYPAD_B,		// B(down)		-> A					2
	RETRO_DEVICE_ID_JOYPAD_START,		// Start		-> Start				3
	RETRO_DEVICE_ID_JOYPAD_L,		// L1			-> Z					4
	RETRO_DEVICE_ID_JOYPAD_X,		// X(top)		-> Y					5
	RETRO_DEVICE_ID_JOYPAD_Y,		// Y(left)		-> X					6
	RETRO_DEVICE_ID_JOYPAD_R2,		// R2			-> R					7
};

static const unsigned input_map_mission_left_shoulder =
	RETRO_DEVICE_ID_JOYPAD_L2;		// L2			-> L					15

static const unsigned input_map_mission_throttle_latch = RETRO_DEVICE_ID_JOYPAD_R3;

/* Twin-Stick */
static const unsigned input_map_twinstick_left_trigger  = RETRO_DEVICE_ID_JOYPAD_L2;
static const unsigned input_map_twinstick_left_button   = RETRO_DEVICE_ID_JOYPAD_L;
static const unsigned input_map_twinstick_right_trigger = RETRO_DEVICE_ID_JOYPAD_R2;
static const unsigned input_map_twinstick_right_button  = RETRO_DEVICE_ID_JOYPAD_R;

/* Sega Saturn US 101-key keyboard scan-code map.
 *
 * Each entry maps a libretro RETROK_ keysym to the Saturn keyboard's
 * 8-bit scan-code value as documented in the IDII table in upstream
 * mednafen/ss/input/keyboard.cpp.  Scan codes 0x00-0x8F are valid
 * (144 slots, of which ~89 are real keys; the rest are IDII Padding
 * slots that don't correspond to any physical key).
 *
 * poll_ss_keyboard() walks this table, queries each RETROK_ via
 * input_state_cb(port, RETRO_DEVICE_KEYBOARD, 0, key), and ORs the
 * corresponding scan-code bit into the 18-byte phys-bitmap buffer
 * that IODevice_Keyboard_UpdateInput consumes (see smpc_iodevice.c).
 *
 * Keys deliberately omitted: Windows / Meta / Super keys, since the
 * PS/2-adapter-emulated keyboard didn't expose them reliably (cf.
 * upstream comment in keyboard.cpp).  Print Screen has no RETROK_
 * equivalent that's commonly mapped, so it's also dropped. */
struct ss_keyboard_map_entry { uint16_t retro_key; uint8_t saturn_sc; };
static const struct ss_keyboard_map_entry ss_keyboard_map[] = {
	{ RETROK_F9,           0x01 },
	{ RETROK_F5,           0x03 },
	{ RETROK_F3,           0x04 },
	{ RETROK_F1,           0x05 },
	{ RETROK_F2,           0x06 },
	{ RETROK_F12,          0x07 },
	{ RETROK_F10,          0x09 },
	{ RETROK_F8,           0x0A },
	{ RETROK_F6,           0x0B },
	{ RETROK_F4,           0x0C },
	{ RETROK_TAB,          0x0D },
	{ RETROK_BACKQUOTE,    0x0E },
	{ RETROK_LALT,         0x11 },
	{ RETROK_LSHIFT,       0x12 },
	{ RETROK_LCTRL,        0x14 },
	{ RETROK_q,            0x15 },
	{ RETROK_1,            0x16 },
	{ RETROK_RALT,         0x17 },
	{ RETROK_RCTRL,        0x18 },
	{ RETROK_KP_ENTER,     0x19 },
	{ RETROK_z,            0x1A },
	{ RETROK_s,            0x1B },
	{ RETROK_a,            0x1C },
	{ RETROK_w,            0x1D },
	{ RETROK_2,            0x1E },
	{ RETROK_c,            0x21 },
	{ RETROK_x,            0x22 },
	{ RETROK_d,            0x23 },
	{ RETROK_e,            0x24 },
	{ RETROK_4,            0x25 },
	{ RETROK_3,            0x26 },
	{ RETROK_SPACE,        0x29 },
	{ RETROK_v,            0x2A },
	{ RETROK_f,            0x2B },
	{ RETROK_t,            0x2C },
	{ RETROK_r,            0x2D },
	{ RETROK_5,            0x2E },
	{ RETROK_n,            0x31 },
	{ RETROK_b,            0x32 },
	{ RETROK_h,            0x33 },
	{ RETROK_g,            0x34 },
	{ RETROK_y,            0x35 },
	{ RETROK_6,            0x36 },
	{ RETROK_m,            0x3A },
	{ RETROK_j,            0x3B },
	{ RETROK_u,            0x3C },
	{ RETROK_7,            0x3D },
	{ RETROK_8,            0x3E },
	{ RETROK_COMMA,        0x41 },
	{ RETROK_k,            0x42 },
	{ RETROK_i,            0x43 },
	{ RETROK_o,            0x44 },
	{ RETROK_0,            0x45 },
	{ RETROK_9,            0x46 },
	{ RETROK_PERIOD,       0x49 },
	{ RETROK_SLASH,        0x4A },
	{ RETROK_l,            0x4B },
	{ RETROK_SEMICOLON,    0x4C },
	{ RETROK_p,            0x4D },
	{ RETROK_MINUS,        0x4E },
	{ RETROK_QUOTE,        0x52 },
	{ RETROK_LEFTBRACKET,  0x54 },
	{ RETROK_EQUALS,       0x55 },
	{ RETROK_CAPSLOCK,     0x58 },
	{ RETROK_RSHIFT,       0x59 },
	{ RETROK_RETURN,       0x5A },
	{ RETROK_RIGHTBRACKET, 0x5B },
	{ RETROK_BACKSLASH,    0x5D },
	{ RETROK_BACKSPACE,    0x66 },
	{ RETROK_KP1,          0x69 },
	{ RETROK_KP4,          0x6B },
	{ RETROK_KP7,          0x6C },
	{ RETROK_KP0,          0x70 },
	{ RETROK_KP_PERIOD,    0x71 },
	{ RETROK_KP2,          0x72 },
	{ RETROK_KP5,          0x73 },
	{ RETROK_KP6,          0x74 },
	{ RETROK_KP8,          0x75 },
	{ RETROK_ESCAPE,       0x76 },
	{ RETROK_NUMLOCK,      0x77 },
	{ RETROK_F11,          0x78 },
	{ RETROK_KP_PLUS,      0x79 },
	{ RETROK_KP3,          0x7A },
	{ RETROK_KP_MINUS,     0x7B },
	{ RETROK_KP_MULTIPLY,  0x7C },
	{ RETROK_KP9,          0x7D },
	{ RETROK_SCROLLOCK,    0x7E },
	{ RETROK_KP_DIVIDE,    0x80 },
	{ RETROK_INSERT,       0x81 },
	{ RETROK_PAUSE,        0x82 },
	{ RETROK_F7,           0x83 },
	{ RETROK_DELETE,       0x85 },
	{ RETROK_LEFT,         0x86 },
	{ RETROK_HOME,         0x87 },
	{ RETROK_END,          0x88 },
	{ RETROK_UP,           0x89 },
	{ RETROK_DOWN,         0x8A },
	{ RETROK_PAGEUP,       0x8B },
	{ RETROK_PAGEDOWN,     0x8C },
	{ RETROK_RIGHT,        0x8D },
};
#define SS_KEYBOARD_MAP_SIZE ( sizeof(ss_keyboard_map) / sizeof(ss_keyboard_map[0]) )

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

/* ----------------------------------------------------------------------
 *  Deterministic fixed-point analog conditioning
 *
 *  The analog deadzone rescaling fed the emulated 3D Control Pad's axis
 *  state through float (and a polar sqrt/atan2/sin/cos round-trip).
 *  float sqrt/atan2/trig is not bit-reproducible across architectures
 *  or libm implementations, so two netplay peers could derive different
 *  axis bytes from identical stick input and desync.  Everything below
 *  is integer Q16 fixed point.  The polar deadzone in particular reduces
 *  to a plain radial scale: since cos(atan2(y,x)) == x / hypot(x,y) and
 *  sin(atan2(y,x)) == y / hypot(x,y), the new cartesian values are just
 *  (x,y) * (new_radius / radius) - no trig is needed at all, only one
 *  integer square root.
 * -------------------------------------------------------------------- */

#define ANALOG_Q16_SHIFT 16
#define ANALOG_Q16_ONE   (1 << ANALOG_Q16_SHIFT)

/*  64-bit integer square root (binary, no FPU); returns floor(sqrt(x)). */
static uint32_t analog_isqrt64(uint64_t x)
{
	uint64_t res = 0;
	uint64_t bit = (uint64_t)1 << 62;

	while (bit > x)
		bit >>= 2;

	while (bit != 0)
	{
		if (x >= res + bit)
		{
			x   -= res + bit;
			res  = (res >> 1) + bit;
		}
		else
			res >>= 1;
		bit >>= 2;
	}
	return (uint32_t)res;
}

/*  Q16/Q32 product -> int, rounded half away from zero (matches round). */
static INLINE int32_t analog_q16_round(int64_t q16_product)
{
	int64_t half = ANALOG_Q16_ONE / 2;
	if (q16_product >= 0)
		return  (int32_t)(( q16_product + half) >> ANALOG_Q16_SHIFT);
	return    -(int32_t)((-q16_product + half) >> ANALOG_Q16_SHIFT);
}

static void get_analog_axis( retro_input_state_t input_state_cb,
		int player_index,
		int stick,
		int axis,
		int* p_analog )
{
	int analog = input_state_cb( player_index, RETRO_DEVICE_ANALOG, stick, axis );

	// Analog stick deadzone
	if ( astick_deadzone > 0 )
	{
		static const int ASTICK_MAX = 0x8000;
		// scale = ASTICK_MAX / (ASTICK_MAX - deadzone), in Q16
		const int32_t scale_q16 = (int32_t)(((int64_t)ASTICK_MAX << ANALOG_Q16_SHIFT)
		                                     / (ASTICK_MAX - astick_deadzone));

		if ( analog < -astick_deadzone )
		{
			// Re-scale analog stick range
			analog = -analog_q16_round( (int64_t)(-analog - astick_deadzone) * scale_q16 );
			if (analog < -32767)
				analog = -32767;
		}
		else if ( analog > astick_deadzone )
		{
			// Re-scale analog stick range
			analog = analog_q16_round( (int64_t)(analog - astick_deadzone) * scale_q16 );
			if (analog > +32767)
				analog = +32767;
		}
		else
			analog = 0;
	}

	// output
	*p_analog = analog;
}

static void get_analog_stick( retro_input_state_t input_state_cb,
		int player_index,
		int stick,
		int* p_analog_x,
		int* p_analog_y )
{
	int analog_x = input_state_cb( player_index, RETRO_DEVICE_ANALOG, stick, RETRO_DEVICE_ID_ANALOG_X );
	int analog_y = input_state_cb( player_index, RETRO_DEVICE_ANALOG, stick, RETRO_DEVICE_ID_ANALOG_Y );

	// Analog stick deadzone (borrowed code from parallel-n64 core)
	if ( astick_deadzone > 0 )
	{
		static const int ASTICK_MAX = 0x8000;

		// Radius in integer units.  The original code converted to polar
		// (sqrt + atan2), rescaled the radius, then converted back with
		// cos/sin.  Because cos(atan2(y,x)) == x/radius and
		// sin(atan2(y,x)) == y/radius, the round-trip is just a radial
		// scale of (x,y) - no trig, one integer sqrt, fully deterministic.
		uint32_t radius = analog_isqrt64( (uint64_t)((int64_t)analog_x * analog_x)
		                                + (uint64_t)((int64_t)analog_y * analog_y) );

		if ( (int)radius > astick_deadzone )
		{
			// Re-scale radius to negate deadzone (makes slow movements possible):
			//   new_radius = (radius - deadzone) * ASTICK_MAX/(ASTICK_MAX-deadzone)
			// then (x,y) *= new_radius / radius, with round-half-away-from-zero.
			int32_t scale_q16   = (int32_t)(((int64_t)ASTICK_MAX << ANALOG_Q16_SHIFT)
			                                 / (ASTICK_MAX - astick_deadzone));
			int64_t new_radius_q16 = (int64_t)((int)radius - astick_deadzone) * scale_q16;

			analog_x = analog_q16_round( (int64_t)analog_x * new_radius_q16 / radius );
			analog_y = analog_q16_round( (int64_t)analog_y * new_radius_q16 / radius );

			// Clamp to correct range
			if (analog_x > +32767) analog_x = +32767;
			if (analog_x < -32767) analog_x = -32767;
			if (analog_y > +32767) analog_y = +32767;
			if (analog_y < -32767) analog_y = -32767;
		}
		else
		{
			analog_x = 0;
			analog_y = 0;
		}
	}

	// output
	*p_analog_x = analog_x;
	*p_analog_y = analog_y;
}

static uint32_t apply_trigger_deadzone( uint32_t input )
{
	// Scale by two and apply outer deadzone (about 1%)
	input = ( input * 66191 ) / 32768;
	
	// Inner deadzone
	if ( trigger_deadzone > 0 )
	{
		// trigger_deadzone is a signed int (user-configurable from libretro
		// core options) but the > 0 guard above means it's safe to treat
		// as unsigned for the comparison/subtraction against the uint32_t
		// input. The alias also silences a sign-compare warning.
		const uint32_t deadzone = (uint32_t)trigger_deadzone;
		// scale = TRIGGER_MAX / (TRIGGER_MAX - deadzone), in Q16
		const int32_t  scale_q16 = (int32_t)(((int64_t)TRIGGER_MAX << ANALOG_Q16_SHIFT)
		                                      / (TRIGGER_MAX - trigger_deadzone));

		if ( input > deadzone )
		{
			// Re-scale analog range
			input = (uint32_t)analog_q16_round( (int64_t)(input - deadzone) * scale_q16 );
		}
		else
			input        = 0;
	}
	
	// Clamp
	if (input > TRIGGER_MAX)
		input = TRIGGER_MAX;

	return input;
}

static uint16_t get_analog_trigger( retro_input_state_t input_state_cb,
		int player_index,
		int id )
{
	// NOTE: Analog triggers were added Nov 2017. Not all front-ends support this
	// feature (or pre-date it) so we need to handle this in a graceful way.

	// First, try and get an analog value using the new libretro API constant
	uint16_t trigger = input_state_cb( player_index,
			RETRO_DEVICE_ANALOG,
			RETRO_DEVICE_INDEX_ANALOG_BUTTON,
			id );

	if ( trigger == 0 )
	{
		// If we got exactly zero, we're either not pressing the button, or the front-end
		// is not reporting analog values. We need to do a second check using the classic
		// digital API method, to at least get some response - better than nothing.

		// NOTE: If we're really just not holding the trigger, we're still going to get zero.
		if (input_state_cb( player_index,
					RETRO_DEVICE_JOYPAD,
					0,
					id ))
			return  0xFFFF;
		return 0;
	}

	// We got something, which means the front-end can handle analog buttons.
	// So we apply a deadzone to the input and use it.
	// Mednafen wants 0 - 65535 so we scale up from 0 - 32767
	return apply_trigger_deadzone( (unsigned)trigger );
}


//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

void input_init_env( retro_environment_t _environ_cb )
{
#define RETRO_DESCRIPTOR_BLOCK( _user )																				\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "A Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "B Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "C Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "X Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "Y Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "Z Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,    "L Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,    "R Button" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start Button" },							\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Mode Switch / Coin (ST-V)" },							\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,    "Test (ST-V)" },								\
		{ _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,    "Service (ST-V) -- chord with Test for Pause" },								\
		{ _user, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X, "Analog X" },		\
		{ _user, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y, "Analog Y" },		\
		{ _user, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X, "Analog X (Right)" },	\
		{ _user, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y, "Analog Y (Right)" },	\
		{ _user, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER, "Gun Trigger" },						\
		{ _user, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START, "Gun Start" },							\
		{ _user, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD, "Gun Reload" },							\
		{ _user, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT,     "Mouse A" },								\
		{ _user, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT,    "Mouse B" },								\
		{ _user, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE,   "Mouse C" },								\
		{ _user, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_4, "Mouse Start" },								\
		{ _user, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_5, "Mouse Start (Alt)" }

	struct retro_input_descriptor desc[] =
	{
		RETRO_DESCRIPTOR_BLOCK( 0 ),
		RETRO_DESCRIPTOR_BLOCK( 1 ),
		RETRO_DESCRIPTOR_BLOCK( 2 ),
		RETRO_DESCRIPTOR_BLOCK( 3 ),
		RETRO_DESCRIPTOR_BLOCK( 4 ),
		RETRO_DESCRIPTOR_BLOCK( 5 ),
		RETRO_DESCRIPTOR_BLOCK( 6 ),
		RETRO_DESCRIPTOR_BLOCK( 7 ),
		RETRO_DESCRIPTOR_BLOCK( 8 ),
		RETRO_DESCRIPTOR_BLOCK( 9 ),
		RETRO_DESCRIPTOR_BLOCK( 10 ),
		RETRO_DESCRIPTOR_BLOCK( 11 ),

		{ 0 },
	};

#undef RETRO_DESCRIPTOR_BLOCK

	// Cache this
	environ_cb = _environ_cb;

	/* Send to front-end */
	environ_cb( RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc );
}

void input_set_env( retro_environment_t environ_cb )
{
	/* Pick ports selection */
	const struct retro_controller_info* ports = ports_lut[ setting_multitap_port1 + setting_multitap_port2 * 2 ];
	
	/* Send to front-end */
	environ_cb( RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports );
}

void input_init(void)
{
	unsigned i;

	// Initialise to default pad type and bind input buffers to SMPC emulation.
	for ( i = 0; i < MAX_CONTROLLERS; ++i )
	{
		input_type[ i ] = RETRO_DEVICE_JOYPAD;
		input_mode[ i ] = INPUT_MODE_DEFAULT;
		input_throttle_latch[ i ] = 0;

		SS_SetInput( i, "gamepad", (uint8_t*)&input_data[ i ] );
	}

	/* Bind the ST-V misc-input port (test/service/pause/reset bits).
	 * SS_SetInput routes port 12 to BOTH STVIO_SetInput (which sets
	 * DPtr[12] = ptr) and SMPC_SetInput (which sets MiscInputPtr).
	 * Gated on ActiveCartType so consumer Saturn games don't get a
	 * stray misc-input pointer registered with SMPC -- it'd be
	 * harmless there (the byte is always 0), but skipping the call
	 * keeps the SMPC state matching Saturn-only expectations. */
	if ( ActiveCartType == CART_STV )
	{
		input_stv_misc_byte = 0;
		SS_SetInput( 12, "misc", &input_stv_misc_byte );
	}
}

void input_set_geometry(unsigned width, unsigned height)
{
	geometry_width = width;
	geometry_height = height;
}

void input_set_deadzone_stick( int percent )
{
	if ( percent >= 0 && percent <= 100 )
		astick_deadzone = ( percent * 0x8000 ) / 100;
}

void input_set_deadzone_trigger( int percent )
{
	if ( percent >= 0 && percent <= 100 )
		trigger_deadzone = ( percent * TRIGGER_MAX ) / 100;
}

void input_set_mouse_sensitivity( int percent )
{
	if ( percent > 0 && percent <= 200 )
		mouse_sensitivity_q16 = (int32_t)(((int64_t)percent << 16) / 100);
}

/* Rising-edge detect SELECT on each port; on press, queue one ST-V
 * coin via STVIO_InsertCoin.  Called from the tail of both the
 * bitmask and per-key polling paths.  No-op outside ST-V mode so
 * the prev_coin[] state stays valid across cart transitions without
 * resetting -- if a user swaps ST-V game while pressing SELECT, the
 * next press on the new game still edge-triggers (the held-through
 * state stays true, the next 0 -> 1 transition is the next press).
 *
 * The MASK fast path reads all buttons in one input_state_cb call;
 * the non-bitmask path is a per-button query.  Both share this
 * helper, which queries SELECT once per port directly -- one extra
 * per-port input_state_cb call vs. piggy-backing on the existing
 * per-player input_type dispatch, but cheaper than threading the
 * coin-bit through every input-type branch and easier to reason
 * about (input_type-specific transforms don't bleed into the ST-V
 * coin path). */
static void check_stv_coin_inserts( retro_input_state_t input_state_cb )
{
	unsigned port;

	if ( ActiveCartType != CART_STV )
		return;

	for ( port = 0; port < MAX_CONTROLLERS && port < 2; ++port )
	{
		const bool now = !!input_state_cb( port, RETRO_DEVICE_JOYPAD, 0,
		                                   RETRO_DEVICE_ID_JOYPAD_SELECT );
		if ( now && !prev_coin[ port ] )
			STVIO_InsertCoin();
		prev_coin[ port ] = now;
	}
}

/* Per-frame populate of the ST-V misc-input byte (test/service/pause).
 * Bound by input_init via SS_SetInput(12, "misc", &input_stv_misc_byte)
 * when ActiveCartType == CART_STV; stvio.c reads the byte through
 * DPtr[12].
 *
 * Inputs are level-triggered (not edge-triggered like the coin button)
 * because the underlying ST-V misc-input register samples continuously
 * -- holding "test" should keep the test menu navigation responsive,
 * not pulse it once.
 *
 * Source bindings (P1 controller, retropad):
 *   L3 -> Test
 *   R3 -> Service
 *   L3 + R3 chord -> Pause
 *
 * The chord for Pause keeps the binding count down (no fourth button
 * needed) and avoids accidentally bumping Pause when the user just
 * means to hold Test or Service.  Only port 0 (P1's pad) drives these
 * -- the physical ST-V cabinet has a single operator panel, so taking
 * exactly one controller's chord as the source is the closest analogue.
 *
 * Bit 0 (reset button) is left zeroed here -- stvio's TransformInput
 * clears it after each read anyway, and there's no obvious mapping
 * for a "soft-reset the Saturn from the front panel" button at this
 * layer (RetroArch's reset hotkey calls our retro_reset() instead). */
static void update_stv_misc_inputs( retro_input_state_t input_state_cb )
{
	bool l3;
	bool r3;
	uint8_t misc = 0;

	if ( ActiveCartType != CART_STV )
		return;

	l3 = !!input_state_cb( 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3 );
	r3 = !!input_state_cb( 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3 );

	if ( l3 && !r3 ) misc |= 0x04;  /* Test    (bit 2) */
	if ( r3 && !l3 ) misc |= 0x08;  /* Service (bit 3) */
	if ( l3 &&  r3 ) misc |= 0x10;  /* Pause   (bit 4) */

	input_stv_misc_byte = misc;
}

/* Per-frame Saturn keyboard poll.  Translates RETROK_ keysyms (the
 * libretro keyboard event stream surfaced through input_state_cb's
 * RETRO_DEVICE_KEYBOARD path) into the 18-byte Saturn scan-code
 * bitmap that IODevice_Keyboard_UpdateInput consumes via DPtr[port].
 *
 * Bit layout: scan code sc lives at byte (sc >> 3) bit (sc & 7), LSB
 * first within the byte.  Buffer is fully zeroed at the head of each
 * frame because the consumer XORs against its own previous-frame
 * `processed[]` to detect make/break edges -- it relies on the input
 * bitmap being a complete present-state snapshot, not a delta.
 *
 * Called only from the RETRO_DEVICE_SS_KEYBOARD case in the poll-path
 * switch (input_update / input_update_with_bitmasks), so the function
 * itself doesn't have to gate on input_type. */
static void poll_ss_keyboard( retro_input_state_t input_state_cb, unsigned iplayer, uint8_t *buf )
{
	unsigned i;

	memset( buf, 0, 18 );

	for ( i = 0; i < SS_KEYBOARD_MAP_SIZE; ++i )
	{
		const uint8_t sc = ss_keyboard_map[ i ].saturn_sc;
		if ( input_state_cb( iplayer, RETRO_DEVICE_KEYBOARD, 0,
		                     ss_keyboard_map[ i ].retro_key ) )
			buf[ sc >> 3 ] |= (uint8_t)(1u << (sc & 7));
	}
}

void input_update_with_bitmasks( retro_input_state_t input_state_cb )
{
	unsigned iplayer;

	// For each player (logical controller)
	for ( iplayer = 0; iplayer < players; ++iplayer )
	{
		INPUT_DATA* p_input = &(input_data[ iplayer ]);
		int16_t ret         = input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK );

		// reset input
		p_input->buttons    = 0;

		// What kind of controller is connected?
		switch ( input_type[ iplayer ] )
		{

		case RETRO_DEVICE_JOYPAD:
		case RETRO_DEVICE_SS_PAD:
		{
			int i;

			//
			// -- standard control pad buttons + d-pad

			// input_map_pad is configured to quickly map libretro buttons to the correct bits for the Saturn.
			for ( i = 0; i < INPUT_MAP_PAD_SIZE; ++i )
			{
				if (ret & (1 << input_map_pad[ i ]))
					p_input->buttons |= ( 1 << i );
			}

			// .. the left trigger on the Saturn is a special case since there's a gap in the bits.
			if (ret & (1 << RETRO_DEVICE_ID_JOYPAD_L2))
				p_input->buttons |= ( 1 << 15 );
 
			if (!opposite_directions)
			{
				if ((p_input->buttons & 0x30) == 0x30)
					p_input->buttons &= ~0x30;
				if ((p_input->buttons & 0xC0) == 0xC0)
					p_input->buttons &= ~0xC0;
			}
			break;
		}

		case RETRO_DEVICE_SS_TWINSTICK:

			{
				int analog_lx, analog_ly;
				int analog_rx, analog_ry;
				const int thresh = 16000;

				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog_lx, &analog_ly );
				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_RIGHT, &analog_rx, &analog_ry );

				// left-stick
				if ( analog_ly <= -thresh )
					p_input->buttons |= ( 1 << 4 ); // Up
				if ( analog_lx >= thresh )
					p_input->buttons |= ( 1 << 7 ); // Right
				if ( analog_ly >= thresh )
					p_input->buttons |= ( 1 << 5 ); // Down
				if ( analog_lx <= -thresh )
					p_input->buttons |= ( 1 << 6 ); // Left

				// right-stick
				if ( analog_ry <= -thresh )
					p_input->buttons |= ( 1 << 1 ); // Up <-(Y)
				if ( analog_rx >= thresh )
					p_input->buttons |= ( 1 << 0 ); // Right <-(Z)
				if ( analog_ry >= thresh )
					p_input->buttons |= ( 1 << 8 ); // Down <-(B)
				if ( analog_rx <= -thresh )
					p_input->buttons |= ( 1 << 2 ); // Left <-(X)

				// left trigger
				if (ret & (1 << input_map_twinstick_left_trigger))
					p_input->buttons |= (1 << 15);

				// left button
				if (ret & (1 << input_map_twinstick_left_button))
					p_input->buttons |= ( 1 << 3 );

				// right trigger
				if (ret & (1 << input_map_twinstick_right_trigger))
					p_input->buttons |= ( 1 << 10 );

				// right button
				if (ret & (1 << input_map_twinstick_right_button))
					p_input->buttons |= ( 1 << 9 );

				// start
				if (ret & (1 << RETRO_DEVICE_ID_JOYPAD_START))
					p_input->buttons |= ( 1 << 11 );
			}
			break;

		case RETRO_DEVICE_SS_3D_PAD:

			{
				int i;
				int analog_x, analog_y;
				uint16_t thumb_x, thumb_y;
				uint16_t l_trigger, r_trigger;

				//
				// -- 3d control pad buttons

				// input_map_3d_pad is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_3D_PAD_SIZE; ++i )
					if (ret & (1 << input_map_3d_pad[ i ]))
						p_input->buttons |= ( 1 << i );

				//
				// -- analog stick

				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog_x, &analog_y );

				// mednafen wants 0 - 32767 - 65535
				thumb_x = (uint16_t)(analog_x + 32767);
				thumb_y = (uint16_t)(analog_y + 32767);

				//
				// -- triggers

				// mednafen wants 0 - 65535
				l_trigger = get_analog_trigger( input_state_cb, iplayer, RETRO_DEVICE_ID_JOYPAD_L2 );
				r_trigger = get_analog_trigger( input_state_cb, iplayer, RETRO_DEVICE_ID_JOYPAD_R2 );


				//
				// -- mode switch

				{
					// Handle MODE button as a switch
					uint16_t prev = ( input_mode[iplayer] & INPUT_MODE_3D_PAD_PREVIOUS_MASK );
					uint16_t held = 0;

					if (ret & (1 << input_map_3d_pad_mode_switch))
						held = INPUT_MODE_3D_PAD_PREVIOUS_MASK;

					// Rising edge trigger
					if ( !prev && held )
					{
						char text[ 256 ];
						struct retro_message msg;
						// Toggle 'state' bit: analog/digital mode
						input_mode[ iplayer ] ^= INPUT_MODE_3D_PAD_ANALOG;

						// Tell user
						if ( input_mode[iplayer] & INPUT_MODE_3D_PAD_ANALOG )
							sprintf( text, "Controller %u: Analog Mode", (iplayer+1) );
						else
							sprintf( text, "Controller %u: Digital Mode", (iplayer+1) );
						msg.msg    = text;
						msg.frames = 180;
						environ_cb( RETRO_ENVIRONMENT_SET_MESSAGE, &msg );
					}

					// Store held state in 'previous' bit.
					input_mode[ iplayer ] = ( input_mode[ iplayer ] & ~INPUT_MODE_3D_PAD_PREVIOUS_MASK ) | held;
				}

				//
				// -- format input data

				// Apply analog/digital mode switch bit.
				if ( input_mode[iplayer] & INPUT_MODE_3D_PAD_ANALOG )
					p_input->buttons |= 0x1000; // set bit 12

				p_input->u8[0x2] = ((thumb_x >> 0) & 0xff);
				p_input->u8[0x3] = ((thumb_x >> 8) & 0xff);
				p_input->u8[0x4] = ((thumb_y >> 0) & 0xff);
				p_input->u8[0x5] = ((thumb_y >> 8) & 0xff);
				p_input->u8[0x6] = ((r_trigger >> 0) & 0xff);
				p_input->u8[0x7] = ((r_trigger >> 8) & 0xff);
				p_input->u8[0x8] = ((l_trigger >> 0) & 0xff);
				p_input->u8[0x9] = ((l_trigger >> 8) & 0xff);
			}

			break;

		case RETRO_DEVICE_SS_WHEEL:

			{
				//
				// -- Wheel buttons

				int i;
				int analog_x;
				uint16_t right, left;

				// input_map_wheel is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_WHEEL_SIZE; ++i )
				{
					const uint16_t bit = ( 1 << ( i + INPUT_MAP_WHEEL_BITSHIFT ) );
					if (ret & (1 << input_map_wheel[ i ]))
						p_input->buttons |= bit;
				}

				// shift-paddles
				if (ret & (1 << input_map_wheel_shift_left))
					p_input->buttons |= ( 1 << 0 );
				if (ret & (1 << input_map_wheel_shift_right))
					p_input->buttons |= ( 1 << 1 );

				//
				// -- analog wheel

				get_analog_axis( input_state_cb, iplayer,
					RETRO_DEVICE_INDEX_ANALOG_LEFT,
					RETRO_DEVICE_ID_ANALOG_X, &analog_x );

				//
				// -- format input data

				// Convert analog values into direction values.
				right   = analog_x > 0 ?  analog_x : 0;
				left    = analog_x < 0 ? -analog_x : 0;

				p_input->u8[0x2] = ((left  >> 0) & 0xff);
				p_input->u8[0x3] = ((left  >> 8) & 0xff);
				p_input->u8[0x4] = ((right >> 0) & 0xff);
				p_input->u8[0x5] = ((right >> 8) & 0xff);
			}

			break;

		case RETRO_DEVICE_SS_MOUSE:

			{
				int dx_raw, dy_raw;
				int16_t *delta;

				// mouse buttons
				p_input->u8[0x4] = 0;

				if ( input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT ) )
					p_input->u8[0x4] |= ( 1 << 0 ); // A

				if ( input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT ) )
					p_input->u8[0x4] |= ( 1 << 1 ); // B

				if ( input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE ) )
					p_input->u8[0x4] |= ( 1 << 2 ); // C

				if ( input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START ) ||
					 input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_4 ) ||
					 input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_5 ) ) {
					p_input->u8[0x4] |= ( 1 << 3 ); // Start
				}

				// mouse input
				dx_raw = input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X );
				dy_raw = input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y );

				delta = (int16_t*)p_input;
				delta[ 0 ] = (int16_t)analog_q16_round( (int64_t)dx_raw * mouse_sensitivity_q16 );
				delta[ 1 ] = (int16_t)analog_q16_round( (int64_t)dy_raw * mouse_sensitivity_q16 );
			}

			break;

		case RETRO_DEVICE_SS_MISSION:

			{
				int i;
				int analog_x, analog_y;
				int throttle_real;
				int16_t throttle;
				uint16_t right, left, down, up, th_up, th_dn;

				//
				// -- mission stick buttons

				// input_map_mission is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_MISSION_SIZE; ++i )
					if (ret & (1 << input_map_mission[ i ]))
						p_input->buttons |= ( 1 << i );

				// .. the left trigger is a special case, there's a gap in the bits.
				if (ret & (1 << input_map_mission_left_shoulder))
					p_input->buttons |= ( 1 << 11 );

				//
				// -- analog stick

				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog_x, &analog_y );

				//
				// -- throttle

				get_analog_axis( input_state_cb, iplayer,
					RETRO_DEVICE_INDEX_ANALOG_RIGHT,
					RETRO_DEVICE_ID_ANALOG_Y, &throttle_real );

				if ( input_mode[iplayer] & INPUT_MODE_MISSION_THROTTLE_LATCH ) // Use latched value
					throttle = input_throttle_latch[iplayer];
				else // Direct read
					throttle = throttle_real;


				//
				// -- throttle latch switch

				{
					// Handle MODE button as a switch
					uint16_t prev = ( input_mode[iplayer] & INPUT_MODE_MISSION_THROTTLE_PREV );
					uint16_t held = 0;

					if (ret & (1 << input_map_mission_throttle_latch))
						held  = INPUT_MODE_MISSION_THROTTLE_PREV;

					// Rising edge trigger?
					if ( !prev && held )
					{
						// Toggle 'state' bit: throttle latch enable/disable.
						input_mode[ iplayer ] ^= INPUT_MODE_MISSION_THROTTLE_LATCH;

						// Tell user
						if ( input_mode[iplayer] & INPUT_MODE_MISSION_THROTTLE_LATCH )
							input_throttle_latch[iplayer] = (int16_t)throttle_real;
					}

					// Store held state in 'previous' bit.
					input_mode[ iplayer ] = ( input_mode[ iplayer ] & ~INPUT_MODE_MISSION_THROTTLE_PREV ) | held;
				}


				//
				// -- format input data

				// Convert analog values into direction values.
				right = analog_x > 0 ?  analog_x : 0;
				left  = analog_x < 0 ? -analog_x : 0;
				down  = analog_y > 0 ?  analog_y : 0;
				up    = analog_y < 0 ? -analog_y : 0;
				th_up = throttle > 0 ?  throttle : 0;
				th_dn = throttle < 0 ? -throttle : 0;

				p_input->u8[0x2] = 0; // todo: auto-fire controls.
				p_input->u8[0x3] = ((left  >> 0) & 0xff);
				p_input->u8[0x4] = ((left  >> 8) & 0xff);
				p_input->u8[0x5] = ((right >> 0) & 0xff);
				p_input->u8[0x6] = ((right >> 8) & 0xff);
				p_input->u8[0x7] = ((up    >> 0) & 0xff);
				p_input->u8[0x8] = ((up    >> 8) & 0xff);
				p_input->u8[0x9] = ((down  >> 0) & 0xff);
				p_input->u8[0xa] = ((down  >> 8) & 0xff);
				p_input->u8[0xb] = ((th_up >> 0) & 0xff);
				p_input->u8[0xc] = ((th_up >> 8) & 0xff);
				p_input->u8[0xd] = ((th_dn >> 0) & 0xff);
				p_input->u8[0xe] = ((th_dn >> 8) & 0xff);
			}

			break;

		case RETRO_DEVICE_SS_MISSION2:

			{
				int i;
				int analog1_x, analog1_y;
				int analog2_x, analog2_y;
				uint16_t right1, left1, down1, up1;
				uint16_t right2, left2, down2, up2;

				//
				// -- mission stick buttons

				// input_map_mission is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_MISSION_SIZE; ++i )
				{
					if (ret & (1 << input_map_mission[ i ]))
						p_input->buttons |= ( 1 << i );
				}
				// .. the left trigger is a special case, there's a gap in the bits.
				if (ret & (1 << input_map_mission_left_shoulder))
					p_input->buttons |= ( 1 << 11 );

				//
				// -- analog sticks

				// Default - patent shows first stick on right side, second added on left
				// see: https://segaretro.org/images/a/a1/Patent_EP0745928A2.pdf
				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_RIGHT, &analog1_x, &analog1_y );
				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog2_x, &analog2_y );

				//
				// -- format input data

				// Convert analog values into direction values.
				right1 = analog1_x > 0 ?  analog1_x : 0;
				left1  = analog1_x < 0 ? -analog1_x : 0;
				down1  = analog1_y > 0 ?  analog1_y : 0;
				up1    = analog1_y < 0 ? -analog1_y : 0;

				right2 = analog2_x > 0 ?  analog2_x : 0;
				left2  = analog2_x < 0 ? -analog2_x : 0;
				down2  = analog2_y > 0 ?  analog2_y : 0;
				up2    = analog2_y < 0 ? -analog2_y : 0;

				p_input->u8[ 0x2] = 0; // todo: auto-fire controls.

				p_input->u8[ 0x3] = ((left1  >> 0) & 0xff);
				p_input->u8[ 0x4] = ((left1  >> 8) & 0xff);
				p_input->u8[ 0x5] = ((right1 >> 0) & 0xff);
				p_input->u8[ 0x6] = ((right1 >> 8) & 0xff);
				p_input->u8[ 0x7] = ((up1    >> 0) & 0xff);
				p_input->u8[ 0x8] = ((up1    >> 8) & 0xff);
				p_input->u8[ 0x9] = ((down1  >> 0) & 0xff);
				p_input->u8[ 0xa] = ((down1  >> 8) & 0xff);
				p_input->u8[ 0xb] = 0; // todo: throttle1
				p_input->u8[ 0xc] = 0;
				p_input->u8[ 0xd] = 0;
				p_input->u8[ 0xe] = 0;

				p_input->u8[ 0xf] = ((left2  >> 0) & 0xff);
				p_input->u8[0x10] = ((left2  >> 8) & 0xff);
				p_input->u8[0x11] = ((right2 >> 0) & 0xff);
				p_input->u8[0x12] = ((right2 >> 8) & 0xff);
				p_input->u8[0x13] = ((up2    >> 0) & 0xff);
				p_input->u8[0x14] = ((up2    >> 8) & 0xff);
				p_input->u8[0x15] = ((down2  >> 0) & 0xff);
				p_input->u8[0x16] = ((down2  >> 8) & 0xff);
				p_input->u8[0x17] = 0; // todo: throttle2
				p_input->u8[0x18] = 0;
				p_input->u8[0x19] = 0;
				p_input->u8[0x1a] = 0;
			}

			break;

		case RETRO_DEVICE_SS_GUN_JP:
		case RETRO_DEVICE_SS_GUN_US:

			{
				if ( setting_gun_input == SETTING_GUN_INPUT_POINTER )
				{
					int gun_x_raw      = input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
					int gun_y_raw      = input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

					// .. scale into screen space:
					// NOTE: the scaling here is empirical-guesswork.
					// Tested at 352x240 (ntsc) and 352x256 (pal)

					const int scale_x  = 21472;
					const int scale_y  = geometry_height;
					const int offset_y = geometry_height - 240;

					int is_offscreen   = 0;

					int gun_x          = ( ( gun_x_raw + 0x7fff ) * scale_x ) / (0x7fff << 1);
					int gun_y          = ( ( gun_y_raw + 0x7fff ) * scale_y ) / (0x7fff << 1) + offset_y;

					int touch_count;

					// Handle offscreen by checking corrected x and y values
					if ( gun_x == 0 || gun_y == 0 )
					{
						is_offscreen = 1;
						gun_x        = -16384; // magic position to disable cross-hair drawing.
						gun_y        = -16384;
					}

					// Touch sensitivity: keep the gun position held for one
					// frame after touch release so a very light tap still
					// reads as a real shot.  Implemented by
					// `pointer_pressed[iplayer]` -- see the global at the
					// top of this TU for the full state-machine.
					// NOTE: this block used 'return' previously, which aborted the entire
					// per-player input update loop and left players 1..N with stale input
					// from the prior frame. It also clobbered pointer state across players.
					// Both are fixed: state is per-player, and we 'break' out of the
					// switch so the for-loop advances to the next player.

					// trigger
					if ( input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED ) )
					{
						pointer_pressed[ iplayer ] = 1;
						pointer_pressed_last_x[ iplayer ] = gun_x;
						pointer_pressed_last_y[ iplayer ] = gun_y;
					} else if ( pointer_pressed[ iplayer ] ) {
						pointer_pressed[ iplayer ] = 0;
						p_input->gun_pos[ 0 ] = pointer_pressed_last_x[ iplayer ];
						p_input->gun_pos[ 1 ] = pointer_pressed_last_y[ iplayer ];
						p_input->u8[4] &= ~0x1;
						break;
					}

					// position
					p_input->gun_pos[ 0 ] = gun_x;
					p_input->gun_pos[ 1 ] = gun_y;

					// buttons
					p_input->u8[ 4 ] = 0;

					// use multi-touch to support different button inputs:
					// 3-finger touch: START button
					// 2-finger touch: Reload
					// offscreen touch: Reload
					touch_count = input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_COUNT );
					if ( touch_count == 3 )
						p_input->u8[ 4 ] |= 0x2;
					else if ( touch_count == 2 )
						p_input->u8[ 4 ] |= 0x4;
					else if ( touch_count == 1 && is_offscreen )
						p_input->u8[ 4 ] |= 0x4;
					else if ( touch_count == 1 )
						p_input->u8[ 4 ] |= 0x1;

				} else {   // Lightgun input is default
					uint8_t shot_type;
					int gun_x, gun_y;
					int forced_reload = input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD );

					// off-screen?
					if ( input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN ) ||
						forced_reload ||
						geometry_height == 0 )
					{
						shot_type = 0x4; // off-screen shot

						gun_x = -16384; // magic position to disable cross-hair drawing.
						gun_y = -16384;
					}
					else
					{
						int gun_x_raw = input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X );
						int gun_y_raw = input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y );

						// .. scale into screen space:
						// NOTE: the scaling here is empirical-guesswork.
						// Tested at 352x240 (ntsc) and 352x256 (pal)

						const int scale_x = 21472;
						const int scale_y = geometry_height;
						const int offset_y = geometry_height - 240;

						shot_type = 0x1; // on-screen shot

						gun_x = ( ( gun_x_raw + 0x7fff ) * scale_x ) / (0x7fff << 1);
						gun_y = ( ( gun_y_raw + 0x7fff ) * scale_y ) / (0x7fff << 1) + offset_y;
					}

					// position
					p_input->gun_pos[ 0 ] = gun_x;
					p_input->gun_pos[ 1 ] = gun_y;

					// buttons
					p_input->u8[ 4 ] = 0;

					// trigger
					if ( input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER ) || forced_reload )
						p_input->u8[ 4 ] |= shot_type;

					// start
					if ( input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START ) )
						p_input->u8[ 4 ] |= 0x2;

				}

			}

			break;

		case RETRO_DEVICE_KEYBOARD:
		case RETRO_DEVICE_SS_KEYBOARD:
			poll_ss_keyboard( input_state_cb, iplayer, p_input->u8 );
			break;

		} // switch ( input_type[ iplayer ] )

	} // for each player

	check_stv_coin_inserts( input_state_cb );
	update_stv_misc_inputs( input_state_cb );
}

void input_update( retro_input_state_t input_state_cb )
{
	unsigned iplayer;

	// For each player (logical controller)
	for ( iplayer = 0; iplayer < players; ++iplayer )
	{
		INPUT_DATA* p_input = &(input_data[ iplayer ]);

		// reset input
		p_input->buttons = 0;

		// What kind of controller is connected?
		switch ( input_type[ iplayer ] )
		{

		case RETRO_DEVICE_JOYPAD:
		case RETRO_DEVICE_SS_PAD:
		{
			int i;

			//
			// -- standard control pad buttons + d-pad

			// input_map_pad is configured to quickly map libretro buttons to the correct bits for the Saturn.
			for ( i = 0; i < INPUT_MAP_PAD_SIZE; ++i )
				p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_pad[ i ] ) ? ( 1 << i ) : 0;
			// .. the left trigger on the Saturn is a special case since there's a gap in the bits.
			p_input->buttons |=
				input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_pad_left_shoulder ) ? ( 1 << 15 ) : 0;
 
			if (!opposite_directions)
			{
				if ((p_input->buttons & 0x30) == 0x30)
					p_input->buttons &= ~0x30;
				if ((p_input->buttons & 0xC0) == 0xC0)
					p_input->buttons &= ~0xC0;
			}
			break;
		}

		case RETRO_DEVICE_SS_TWINSTICK:

			{
				int analog_lx, analog_ly;
				int analog_rx, analog_ry;
				const int thresh = 16000;

				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog_lx, &analog_ly );
				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_RIGHT, &analog_rx, &analog_ry );

				// left-stick
				if ( analog_ly <= -thresh )
					p_input->buttons |= ( 1 << 4 ); // Up
				if ( analog_lx >= thresh )
					p_input->buttons |= ( 1 << 7 ); // Right
				if ( analog_ly >= thresh )
					p_input->buttons |= ( 1 << 5 ); // Down
				if ( analog_lx <= -thresh )
					p_input->buttons |= ( 1 << 6 ); // Left

				// right-stick
				if ( analog_ry <= -thresh )
					p_input->buttons |= ( 1 << 1 ); // Up <-(Y)
				if ( analog_rx >= thresh )
					p_input->buttons |= ( 1 << 0 ); // Right <-(Z)
				if ( analog_ry >= thresh )
					p_input->buttons |= ( 1 << 8 ); // Down <-(B)
				if ( analog_rx <= -thresh )
					p_input->buttons |= ( 1 << 2 ); // Left <-(X)

				// left trigger
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_twinstick_left_trigger ) ? ( 1 << 15 ) : 0;

				// left button
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_twinstick_left_button ) ? ( 1 << 3 ) : 0;

				// right trigger
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_twinstick_right_trigger ) ? ( 1 << 10 ) : 0;

				// right button
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_twinstick_right_button ) ? ( 1 << 9 ) : 0;

				// start
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START ) ? ( 1 << 11 ) : 0;
			}
			break;

		case RETRO_DEVICE_SS_3D_PAD:

			{
				int i;
				int analog_x, analog_y;
				// mednafen wants 0 - 32767 - 65535
				uint16_t thumb_x, thumb_y;
				// mednafen wants 0 - 65535
				uint16_t l_trigger, r_trigger;

				//
				// -- 3d control pad buttons

				// input_map_3d_pad is configured to quickly map libretro buttons to the correct bits for the Saturn.

				for ( i = 0; i < INPUT_MAP_3D_PAD_SIZE; ++i )
					p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_3d_pad[ i ] ) ? ( 1 << i ) : 0;

				//
				// -- analog stick

				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog_x, &analog_y );

				thumb_x = (uint16_t)(analog_x + 32767);
				thumb_y = (uint16_t)(analog_y + 32767);

				//
				// -- triggers

				l_trigger = get_analog_trigger( input_state_cb, iplayer, RETRO_DEVICE_ID_JOYPAD_L2 );
				r_trigger = get_analog_trigger( input_state_cb, iplayer, RETRO_DEVICE_ID_JOYPAD_R2 );


				//
				// -- mode switch

				{
					// Handle MODE button as a switch
					uint16_t prev = ( input_mode[iplayer] & INPUT_MODE_3D_PAD_PREVIOUS_MASK );
					uint16_t held = 0;

					if (input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_3d_pad_mode_switch ))
						held = INPUT_MODE_3D_PAD_PREVIOUS_MASK;

					// Rising edge trigger
					if ( !prev && held )
					{
						char text[ 256 ];
						struct retro_message msg;
						// Toggle 'state' bit: analog/digital mode
						input_mode[ iplayer ] ^= INPUT_MODE_3D_PAD_ANALOG;

						// Tell user
						if ( input_mode[iplayer] & INPUT_MODE_3D_PAD_ANALOG )
							sprintf( text, "Controller %u: Analog Mode", (iplayer+1) );
						else
							sprintf( text, "Controller %u: Digital Mode", (iplayer+1) );
						msg.msg    = text;
						msg.frames = 180;
						environ_cb( RETRO_ENVIRONMENT_SET_MESSAGE, &msg );
					}

					// Store held state in 'previous' bit.
					input_mode[ iplayer ] = ( input_mode[ iplayer ] & ~INPUT_MODE_3D_PAD_PREVIOUS_MASK ) | held;
				}

				//
				// -- format input data

				// Apply analog/digital mode switch bit.
				if ( input_mode[iplayer] & INPUT_MODE_3D_PAD_ANALOG ) {
					p_input->buttons |= 0x1000; // set bit 12
				}

				p_input->u8[0x2] = ((thumb_x >> 0) & 0xff);
				p_input->u8[0x3] = ((thumb_x >> 8) & 0xff);
				p_input->u8[0x4] = ((thumb_y >> 0) & 0xff);
				p_input->u8[0x5] = ((thumb_y >> 8) & 0xff);
				p_input->u8[0x6] = ((r_trigger >> 0) & 0xff);
				p_input->u8[0x7] = ((r_trigger >> 8) & 0xff);
				p_input->u8[0x8] = ((l_trigger >> 0) & 0xff);
				p_input->u8[0x9] = ((l_trigger >> 8) & 0xff);
			}

			break;

		case RETRO_DEVICE_SS_WHEEL:

			{
				//
				// -- Wheel buttons

				int i;
				int analog_x;
				uint16_t right, left;

				// input_map_wheel is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_WHEEL_SIZE; ++i ) {
					const uint16_t bit = ( 1 << ( i + INPUT_MAP_WHEEL_BITSHIFT ) );
					p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_wheel[ i ] ) ? bit : 0;
				}

				// shift-paddles
				p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_wheel_shift_left ) ? ( 1 << 0 ) : 0;
				p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_wheel_shift_right ) ? ( 1 << 1 ) : 0;

				//
				// -- analog wheel

				get_analog_axis( input_state_cb, iplayer,
					RETRO_DEVICE_INDEX_ANALOG_LEFT,
					RETRO_DEVICE_ID_ANALOG_X, &analog_x );

				//
				// -- format input data

				// Convert analog values into direction values.
				right = analog_x > 0 ?  analog_x : 0;
				left  = analog_x < 0 ? -analog_x : 0;

				p_input->u8[0x2] = ((left  >> 0) & 0xff);
				p_input->u8[0x3] = ((left  >> 8) & 0xff);
				p_input->u8[0x4] = ((right >> 0) & 0xff);
				p_input->u8[0x5] = ((right >> 8) & 0xff);
			}

			break;

		case RETRO_DEVICE_SS_MOUSE:

			{
				int dx_raw, dy_raw;
				int16_t *delta;

				// mouse buttons
				p_input->u8[0x4] = 0;

				if ( input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT ) ) {
					p_input->u8[0x4] |= ( 1 << 0 ); // A
				}

				if ( input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT ) ) {
					p_input->u8[0x4] |= ( 1 << 1 ); // B
				}

				if ( input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE ) ) {
					p_input->u8[0x4] |= ( 1 << 2 ); // C
				}

				if ( input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START ) ||
					 input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_4 ) ||
					 input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_BUTTON_5 ) ) {
					p_input->u8[0x4] |= ( 1 << 3 ); // Start
				}

				// mouse input
				dx_raw = input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X );
				dy_raw = input_state_cb( iplayer, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y );

				delta = (int16_t*)p_input;
				delta[ 0 ] = (int16_t)analog_q16_round( (int64_t)dx_raw * mouse_sensitivity_q16 );
				delta[ 1 ] = (int16_t)analog_q16_round( (int64_t)dy_raw * mouse_sensitivity_q16 );
			}

			break;

		case RETRO_DEVICE_SS_MISSION:

			{
				int i;
				int analog_x, analog_y;
				int throttle_real;
				int16_t throttle;
				uint16_t right, left, down, up, th_up, th_dn;

				//
				// -- mission stick buttons

				// input_map_mission is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_MISSION_SIZE; ++i )
					p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_mission[ i ] ) ? ( 1 << i ) : 0;
				// .. the left trigger is a special case, there's a gap in the bits.
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_mission_left_shoulder ) ? ( 1 << 11 ) : 0;

				//
				// -- analog stick

				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog_x, &analog_y );

				//
				// -- throttle

				get_analog_axis( input_state_cb, iplayer,
					RETRO_DEVICE_INDEX_ANALOG_RIGHT,
					RETRO_DEVICE_ID_ANALOG_Y, &throttle_real );

				if ( input_mode[iplayer] & INPUT_MODE_MISSION_THROTTLE_LATCH )
				{
					// Use latched value
					throttle = input_throttle_latch[iplayer];
				}
				else
				{
					// Direct read
					throttle = throttle_real;
				}


				//
				// -- throttle latch switch

				{
					// Handle MODE button as a switch
					uint16_t prev = ( input_mode[iplayer] & INPUT_MODE_MISSION_THROTTLE_PREV );
					uint16_t held = 0;

					if (input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_mission_throttle_latch))
						held = INPUT_MODE_MISSION_THROTTLE_PREV;

					// Rising edge trigger?
					if ( !prev && held )
					{
						// Toggle 'state' bit: throttle latch enable/disable.
						input_mode[ iplayer ] ^= INPUT_MODE_MISSION_THROTTLE_LATCH;

						// Tell user
						if ( input_mode[iplayer] & INPUT_MODE_MISSION_THROTTLE_LATCH ) {
							log_cb( RETRO_LOG_INFO, "Controller %u: Latched Throttle at %d\n", (iplayer+1), throttle_real );
							input_throttle_latch[iplayer] = (int16_t)throttle_real;
						} else {
							log_cb( RETRO_LOG_INFO, "Controller %u: Remove Throttle Latch\n", (iplayer+1) );
						}
					}

					// Store held state in 'previous' bit.
					input_mode[ iplayer ] = ( input_mode[ iplayer ] & ~INPUT_MODE_MISSION_THROTTLE_PREV ) | held;
				}


				//
				// -- format input data

				// Convert analog values into direction values.
				right = analog_x > 0 ?  analog_x : 0;
				left  = analog_x < 0 ? -analog_x : 0;
				down  = analog_y > 0 ?  analog_y : 0;
				up    = analog_y < 0 ? -analog_y : 0;
				th_up = throttle > 0 ?  throttle : 0;
				th_dn = throttle < 0 ? -throttle : 0;

				p_input->u8[0x2] = 0; // todo: auto-fire controls.
				p_input->u8[0x3] = ((left  >> 0) & 0xff);
				p_input->u8[0x4] = ((left  >> 8) & 0xff);
				p_input->u8[0x5] = ((right >> 0) & 0xff);
				p_input->u8[0x6] = ((right >> 8) & 0xff);
				p_input->u8[0x7] = ((up    >> 0) & 0xff);
				p_input->u8[0x8] = ((up    >> 8) & 0xff);
				p_input->u8[0x9] = ((down  >> 0) & 0xff);
				p_input->u8[0xa] = ((down  >> 8) & 0xff);
				p_input->u8[0xb] = ((th_up >> 0) & 0xff);
				p_input->u8[0xc] = ((th_up >> 8) & 0xff);
				p_input->u8[0xd] = ((th_dn >> 0) & 0xff);
				p_input->u8[0xe] = ((th_dn >> 8) & 0xff);
			}

			break;

		case RETRO_DEVICE_SS_MISSION2:

			{
				int i;
				int analog1_x, analog1_y;
				int analog2_x, analog2_y;
				uint16_t right1, left1, down1, up1;
				uint16_t right2, left2, down2, up2;

				//
				// -- mission stick buttons

				// input_map_mission is configured to quickly map libretro buttons to the correct bits for the Saturn.
				for ( i = 0; i < INPUT_MAP_MISSION_SIZE; ++i )
					p_input->buttons |= input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_mission[ i ] ) ? ( 1 << i ) : 0;
				// .. the left trigger is a special case, there's a gap in the bits.
				p_input->buttons |=
					input_state_cb( iplayer, RETRO_DEVICE_JOYPAD, 0, input_map_mission_left_shoulder ) ? ( 1 << 11 ) : 0;

				//
				// -- analog sticks

				// Default - patent shows first stick on right side, second added on left
				// see: https://segaretro.org/images/a/a1/Patent_EP0745928A2.pdf
				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_RIGHT, &analog1_x, &analog1_y );
				get_analog_stick( input_state_cb, iplayer, RETRO_DEVICE_INDEX_ANALOG_LEFT, &analog2_x, &analog2_y );

				//
				// -- format input data

				// Convert analog values into direction values.
				right1 = analog1_x > 0 ?  analog1_x : 0;
				left1  = analog1_x < 0 ? -analog1_x : 0;
				down1  = analog1_y > 0 ?  analog1_y : 0;
				up1    = analog1_y < 0 ? -analog1_y : 0;

				right2 = analog2_x > 0 ?  analog2_x : 0;
				left2  = analog2_x < 0 ? -analog2_x : 0;
				down2  = analog2_y > 0 ?  analog2_y : 0;
				up2    = analog2_y < 0 ? -analog2_y : 0;

				p_input->u8[ 0x2] = 0; // todo: auto-fire controls.

				p_input->u8[ 0x3] = ((left1  >> 0) & 0xff);
				p_input->u8[ 0x4] = ((left1  >> 8) & 0xff);
				p_input->u8[ 0x5] = ((right1 >> 0) & 0xff);
				p_input->u8[ 0x6] = ((right1 >> 8) & 0xff);
				p_input->u8[ 0x7] = ((up1    >> 0) & 0xff);
				p_input->u8[ 0x8] = ((up1    >> 8) & 0xff);
				p_input->u8[ 0x9] = ((down1  >> 0) & 0xff);
				p_input->u8[ 0xa] = ((down1  >> 8) & 0xff);
				p_input->u8[ 0xb] = 0; // todo: throttle1
				p_input->u8[ 0xc] = 0;
				p_input->u8[ 0xd] = 0;
				p_input->u8[ 0xe] = 0;

				p_input->u8[ 0xf] = ((left2  >> 0) & 0xff);
				p_input->u8[0x10] = ((left2  >> 8) & 0xff);
				p_input->u8[0x11] = ((right2 >> 0) & 0xff);
				p_input->u8[0x12] = ((right2 >> 8) & 0xff);
				p_input->u8[0x13] = ((up2    >> 0) & 0xff);
				p_input->u8[0x14] = ((up2    >> 8) & 0xff);
				p_input->u8[0x15] = ((down2  >> 0) & 0xff);
				p_input->u8[0x16] = ((down2  >> 8) & 0xff);
				p_input->u8[0x17] = 0; // todo: throttle2
				p_input->u8[0x18] = 0;
				p_input->u8[0x19] = 0;
				p_input->u8[0x1a] = 0;
			}

			break;

		case RETRO_DEVICE_SS_GUN_JP:
		case RETRO_DEVICE_SS_GUN_US:

			{
				if ( setting_gun_input == SETTING_GUN_INPUT_POINTER ) {
					int gun_x, gun_y;
					int gun_x_raw, gun_y_raw;
					const int scale_x = 21472;
					const int scale_y = geometry_height;
					const int offset_y = geometry_height - 240;
					int is_offscreen = 0;
					int touch_count;

					gun_x_raw = input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
					gun_y_raw = input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

					// .. scale into screen space:
					// NOTE: the scaling here is empirical-guesswork.
					// Tested at 352x240 (ntsc) and 352x256 (pal)

					gun_x = ( ( gun_x_raw + 0x7fff ) * scale_x ) / (0x7fff << 1);
					gun_y = ( ( gun_y_raw + 0x7fff ) * scale_y ) / (0x7fff << 1) + offset_y;

					// Handle offscreen by checking corrected x and y values
					if ( gun_x == 0 || gun_y == 0 )
					{
						is_offscreen = 1;
						gun_x = -16384; // magic position to disable cross-hair drawing.
						gun_y = -16384;
					}

					// Touch sensitivity: keep the gun position held for one
					// frame after touch release so a very light tap still
					// reads as a real shot.  Implemented by
					// `pointer_pressed[iplayer]` -- see the global at the
					// top of this TU for the full state-machine.
					// NOTE: this block used 'return' previously, which aborted the entire
					// per-player input update loop and left players 1..N with stale input
					// from the prior frame. It also clobbered pointer state across players.
					// Both are fixed: state is per-player, and we 'break' out of the
					// switch so the for-loop advances to the next player.

					// trigger
					if ( input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED ) )
					{
						pointer_pressed[ iplayer ] = 1;
						pointer_pressed_last_x[ iplayer ] = gun_x;
						pointer_pressed_last_y[ iplayer ] = gun_y;
					} else if ( pointer_pressed[ iplayer ] ) {
						pointer_pressed[ iplayer ] = 0;
						p_input->gun_pos[ 0 ] = pointer_pressed_last_x[ iplayer ];
						p_input->gun_pos[ 1 ] = pointer_pressed_last_y[ iplayer ];
						p_input->u8[4] &= ~0x1;
						break;
					}

					// position
					p_input->gun_pos[ 0 ] = gun_x;
					p_input->gun_pos[ 1 ] = gun_y;

					// buttons
					p_input->u8[ 4 ] = 0;

					// use multi-touch to support different button inputs:
					// 3-finger touch: START button
					// 2-finger touch: Reload
					// offscreen touch: Reload
					touch_count = input_state_cb( iplayer, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_COUNT );
					if ( touch_count == 3 )
						p_input->u8[ 4 ] |= 0x2;
					else if ( touch_count == 2 )
						p_input->u8[ 4 ] |= 0x4;
					else if ( touch_count == 1 && is_offscreen )
						p_input->u8[ 4 ] |= 0x4;
					else if ( touch_count == 1 )
						p_input->u8[ 4 ] |= 0x1;

				} else {   // Lightgun input is default
					uint8_t shot_type;
					int gun_x, gun_y;
					int forced_reload = input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD );

					// off-screen?
					if ( input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN ) ||
						forced_reload ||
						geometry_height == 0 )
					{
						shot_type = 0x4; // off-screen shot

						gun_x = -16384; // magic position to disable cross-hair drawing.
						gun_y = -16384;
					}
					else
					{
						int gun_x_raw = input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X );
						int gun_y_raw = input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y );

						// .. scale into screen space:
						// NOTE: the scaling here is empirical-guesswork.
						// Tested at 352x240 (ntsc) and 352x256 (pal)

						const int scale_x = 21472;
						const int scale_y = geometry_height;
						const int offset_y = geometry_height - 240;

						shot_type = 0x1; // on-screen shot

						gun_x = ( ( gun_x_raw + 0x7fff ) * scale_x ) / (0x7fff << 1);
						gun_y = ( ( gun_y_raw + 0x7fff ) * scale_y ) / (0x7fff << 1) + offset_y;
					}

					// position
					p_input->gun_pos[ 0 ] = gun_x;
					p_input->gun_pos[ 1 ] = gun_y;

					// buttons
					p_input->u8[ 4 ] = 0;

					// trigger
					if ( input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER ) || forced_reload )
						p_input->u8[ 4 ] |= shot_type;

					// start
					if ( input_state_cb( iplayer, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START ) )
						p_input->u8[ 4 ] |= 0x2;

				}

			}

			break;

		case RETRO_DEVICE_KEYBOARD:
		case RETRO_DEVICE_SS_KEYBOARD:
			poll_ss_keyboard( input_state_cb, iplayer, p_input->u8 );
			break;

		} // switch ( input_type[ iplayer ] )

	} // for each player

	check_stv_coin_inserts( input_state_cb );
	update_stv_misc_inputs( input_state_cb );
}

// save state function for input
int input_StateAction( StateMem* sm, const unsigned load, const bool data_only )
{
	int success;

	SFORMAT StateRegs[] =
	{
		SFPTR16N( input_mode, MAX_CONTROLLERS, "pad-mode" ),
		SFPTR16N( input_throttle_latch, MAX_CONTROLLERS, "throttle-latch" ),
		SFEND
	};

	success = MDFNSS_StateAction( sm, load, data_only, StateRegs, "LIBRETRO-INPUT", false);

	// ok?
	return success;
}

int input_StateActionDevices( StateMem* sm, const unsigned load, const bool data_only )
{
	uint32_t saved_input_type[ MAX_CONTROLLERS ];
	int success;
	SFORMAT StateRegs[] =
	{
		SFPTR32N( saved_input_type, MAX_CONTROLLERS, "input-type" ),
		SFEND
	};

	/* On save, snapshot the live per-port device type into the
	 * SFPTR target so MDFNSS_StateAction writes it out.  On load
	 * this is the buffer MDFNSS_StateAction fills from the state
	 * stream. */
	/* Seed from live unconditionally.  On save this is the
	 * snapshot MDFNSS_StateAction writes out.  On load it makes
	 * the absent-optional-section path a no-op: the call below
	 * returns success=1 without touching the buffer when the
	 * chunk isn't present (states written by pre-fix builds),
	 * the loop then sees saved == live and skips every port.
	 * Without this seed the LOAD path read stack-uninitialised
	 * bytes and called retro_set_controller_port_device with
	 * garbage -- visible in hashlog as 4 run-to-run-varying
	 * bytes inside the "LIBRETRO-INPUT-DEVICES" chunk. */
	memcpy( saved_input_type, input_type, sizeof( saved_input_type ) );

	success = MDFNSS_StateAction( sm, load, data_only, StateRegs,
		"LIBRETRO-INPUT-DEVICES", true /* optional */ );

	if ( load && success )
	{
		unsigned i;
		for ( i = 0; i < MAX_CONTROLLERS; ++i )
		{
			/* If the saved state was written with a different
			 * IOPort device on this port than the live core has
			 * currently assigned, swap the IOPort pointer back to
			 * what the state was written under so the upcoming
			 * SMPC_StateAction's IODevice_*_StateAction call finds
			 * its named section in the state buffer rather than
			 * Power()-cycling the (mismatched) current device. */
			if ( saved_input_type[ i ] != input_type[ i ] )
				retro_set_controller_port_device( i, saved_input_type[ i ] );
		}
	}

	return success;
}

//------------------------------------------------------------------------------
// Libretro Interface
//------------------------------------------------------------------------------

void retro_set_controller_port_device( unsigned in_port, unsigned device )
{
	if ( in_port < MAX_CONTROLLERS )
	{
		// Store input type
		input_type[ in_port ] = device;
		input_mode[ in_port ] = INPUT_MODE_DEFAULT;

		switch ( device )
		{

		case RETRO_DEVICE_NONE:
			log_cb( RETRO_LOG_INFO, "Controller %u: Unplugged\n", (in_port+1) );
			SS_SetInput( in_port, "none", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_JOYPAD:
		case RETRO_DEVICE_SS_PAD:
			log_cb( RETRO_LOG_INFO, "Controller %u: Control Pad\n", (in_port+1) );
			SS_SetInput( in_port, "gamepad", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_3D_PAD:
			log_cb( RETRO_LOG_INFO, "Controller %u: 3D Control Pad\n", (in_port+1) );
			SS_SetInput( in_port, "3dpad", (uint8_t*)&input_data[ in_port ] );
			input_mode[ in_port ] = INPUT_MODE_DEFAULT_3D_PAD;
			break;

		case RETRO_DEVICE_SS_WHEEL:
			log_cb( RETRO_LOG_INFO, "Controller %u: Arcade Racer\n", (in_port+1) );
			SS_SetInput( in_port, "wheel", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_MISSION:
			log_cb( RETRO_LOG_INFO, "Controller %u: Mission Stick\n", (in_port+1) );
			SS_SetInput( in_port, "mission", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_MISSION2:
			log_cb( RETRO_LOG_INFO, "Controller %u: Dual Mission Sticks\n", (in_port+1) );
			SS_SetInput( in_port, "dmission", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_MOUSE:
			log_cb( RETRO_LOG_INFO, "Controller %u: Mouse\n", (in_port+1) );
			SS_SetInput( in_port, "mouse", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_GUN_US:
			log_cb( RETRO_LOG_INFO, "Controller %u: Stunner\n", (in_port+1) );
			SS_SetInput( in_port, "gun", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_GUN_JP:
			log_cb( RETRO_LOG_INFO, "Controller %u: Virtua Gun\n", (in_port+1) );
			SS_SetInput( in_port, "gun", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_SS_TWINSTICK:
			log_cb( RETRO_LOG_INFO, "Controller %u: Twin-Stick\n", (in_port+1) );
			SS_SetInput( in_port, "gamepad", (uint8_t*)&input_data[ in_port ] );
			break;

		case RETRO_DEVICE_KEYBOARD:
		case RETRO_DEVICE_SS_KEYBOARD:
			log_cb( RETRO_LOG_INFO, "Controller %u: Keyboard\n", (in_port+1) );
			/* Buffer is the 32-byte INPUT_DATA.u8 union member; the
			 * Saturn keyboard IODevice reads bytes 0x00..0x11 (18
			 * bytes) as a scan-code bitmap.  poll_ss_keyboard() writes
			 * that bitmap each frame from the per-port poll-path
			 * RETRO_DEVICE_SS_KEYBOARD case. */
			SS_SetInput( in_port, "keyboard", (uint8_t*)&input_data[ in_port ] );
			break;

		default:
			log_cb( RETRO_LOG_WARN, "Controller %u: Unsupported Device (%u)\n", (in_port+1), device );
			SS_SetInput( in_port, "none", (uint8_t*)&input_data[ in_port ] );
			break;

		}; // switch ( device )

	}; // valid port?
}

void input_multitap( int port, bool enabled )
{
	switch ( port )
	{
		case 1: // PORT 1
			if ( enabled != setting_multitap_port1 ) {
				setting_multitap_port1 = enabled;
				if ( setting_multitap_port1 ) {
					log_cb( RETRO_LOG_INFO, "Connected 6Player Adaptor to Port 1\n" );
				} else {
					log_cb( RETRO_LOG_INFO, "Removed 6Player Adaptor from Port 1\n" );
				}
				SMPC_SetMultitap( 0, setting_multitap_port1 );
			}
			break;

		case 2: // PORT 2
			if ( enabled != setting_multitap_port2 ) {
				setting_multitap_port2 = enabled;
				if ( setting_multitap_port2 ) {
					log_cb( RETRO_LOG_INFO, "Connected 6Player Adaptor to Port 2\n" );
				} else {
					log_cb( RETRO_LOG_INFO, "Removed 6Player Adaptor from Port 2\n" );
				}
				SMPC_SetMultitap( 1, setting_multitap_port2 );
			}
			break;

	}; // switch ( port )

	// Update players count
	players = 2;
	if ( setting_multitap_port1 ) {
		players += 5;
	}
	if ( setting_multitap_port2 ) {
		players += 5;
	}
	
	/*Tell front-end*/
	input_set_env( environ_cb );
}

//==============================================================================
