/* m68k_ham_instances.inc.h
 *
 * Phase-9d-15: instantiates one struct M68K_HAM_<T>_<AM> +
 * supporting functions per (T, AM) combination actually used in
 * m68k_instr*.inc.  36 combos total.
 *
 * The naming convention is M68K_HAM_<TSIZE>_<AM> where TSIZE is
 * u8/u16/u32/void.  "void" is used for the addressing-mode-only
 * forms (LEA/PEA/JMP/JSR), which never read or write a value --
 * the read/write/rmw functions are not generated for void HAMs.
 */

#define HAM_NAME M68K_HAM_u8_ABS_LONG
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ABS_LONG
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_ABS_SHORT
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ABS_SHORT
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_ADDR_REG_INDIR
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ADDR_REG_INDIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_ADDR_REG_INDIR_DISP
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ADDR_REG_INDIR_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_ADDR_REG_INDIR_INDX
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ADDR_REG_INDIR_INDX
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_ADDR_REG_INDIR_POST
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ADDR_REG_INDIR_POST
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_ADDR_REG_INDIR_PRE
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   ADDR_REG_INDIR_PRE
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_DATA_REG_DIR
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   DATA_REG_DIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_IMMEDIATE
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   IMMEDIATE
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_PC_DISP
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   PC_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u8_PC_INDEX
#define HAM_T    uint8_t
#define HAM_TSIZE 1
#define HAM_AM   PC_INDEX
#include "m68k_ham_body.inc.h"

/* --- u16 --- */
#define HAM_NAME M68K_HAM_u16_ABS_LONG
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ABS_LONG
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ABS_SHORT
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ABS_SHORT
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ADDR_REG_DIR
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ADDR_REG_DIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ADDR_REG_INDIR
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ADDR_REG_INDIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ADDR_REG_INDIR_DISP
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ADDR_REG_INDIR_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ADDR_REG_INDIR_INDX
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ADDR_REG_INDIR_INDX
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ADDR_REG_INDIR_POST
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ADDR_REG_INDIR_POST
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_ADDR_REG_INDIR_PRE
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   ADDR_REG_INDIR_PRE
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_DATA_REG_DIR
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   DATA_REG_DIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_IMMEDIATE
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   IMMEDIATE
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_PC_DISP
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   PC_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u16_PC_INDEX
#define HAM_T    uint16_t
#define HAM_TSIZE 2
#define HAM_AM   PC_INDEX
#include "m68k_ham_body.inc.h"

/* --- u32 --- */
#define HAM_NAME M68K_HAM_u32_ABS_LONG
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ABS_LONG
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ABS_SHORT
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ABS_SHORT
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ADDR_REG_DIR
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ADDR_REG_DIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ADDR_REG_INDIR
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ADDR_REG_INDIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ADDR_REG_INDIR_DISP
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ADDR_REG_INDIR_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ADDR_REG_INDIR_INDX
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ADDR_REG_INDIR_INDX
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ADDR_REG_INDIR_POST
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ADDR_REG_INDIR_POST
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_ADDR_REG_INDIR_PRE
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   ADDR_REG_INDIR_PRE
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_DATA_REG_DIR
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   DATA_REG_DIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_IMMEDIATE
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   IMMEDIATE
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_PC_DISP
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   PC_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_u32_PC_INDEX
#define HAM_T    uint32_t
#define HAM_TSIZE 4
#define HAM_AM   PC_INDEX
#include "m68k_ham_body.inc.h"

/* --- void (addressing-mode only -- LEA/PEA/JMP/JSR) --- */
#define HAM_NAME M68K_HAM_void_ABS_LONG
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   ABS_LONG
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_void_ABS_SHORT
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   ABS_SHORT
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_void_ADDR_REG_INDIR
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   ADDR_REG_INDIR
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_void_ADDR_REG_INDIR_DISP
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   ADDR_REG_INDIR_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_void_ADDR_REG_INDIR_INDX
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   ADDR_REG_INDIR_INDX
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_void_PC_DISP
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   PC_DISP
#include "m68k_ham_body.inc.h"

#define HAM_NAME M68K_HAM_void_PC_INDEX
#define HAM_T    void
#define HAM_TSIZE 0
#define HAM_AM   PC_INDEX
#include "m68k_ham_body.inc.h"
