/*
   CD block <-> MPEG card regression test.

   Includes the real cdb.c and drives it through its registers, with the
   MPEG card and the rest of the Saturn stubbed.  Checks:

   - 0xE2 (Get MPEG ROM) raises BFUL when the buffers run out;
   - MPEG_Pull is held off only by a host Get Sector Data that reads the
     buffers the card would take, not by a transfer from another
     partition, and the card still takes its sectors in order.

   Run by tools/check_lockfree.py with the core's CFLAGS.  By hand, from
   the core's root:
     cc -std=gnu99 -O1 -I. -Imednafen -Imednafen/include -Ilibretro-common/include \
        -D__LIBRETRO__ -DNEED_CD tools/cdb_mpeg_test.c -o cdb_mpeg_test -lm
*/
#include "mednafen/ss/cdb.c"

#include <stdio.h>

/* ---- Saturn stubs ------------------------------------------------ */
event_list_entry events[SS_EVENT__SIMD_COUNT];
int32_t SH7095_mem_timestamp;
retro_log_printf_t log_cb;

void SS_SetEventNT(event_list_entry *e, const sscpu_timestamp_t t) { e->event_time = t; }
void SCU_SetInt(unsigned which, bool active) { (void)which; (void)active; }
bool MDFN_GetSettingB(const char *n) { (void)n; return false; }
const char *MDFN_GetSettingS(const char *n) { (void)n; return ""; }
char *MDFN_MakeFName(char *b, size_t l, MakeFName_Type t, int i, const char *c)
{ (void)l; (void)t; (void)i; (void)c; b[0] = 0; return b; }
int MDFNSS_StateAction(void *st, int load, int data_only, SFORMAT *sf, const char *name, bool optional)
{ (void)st; (void)load; (void)data_only; (void)sf; (void)name; (void)optional; return 1; }
RFILE *filestream_open(const char *p, unsigned m, unsigned h) { (void)p; (void)m; (void)h; return NULL; }
int filestream_close(RFILE *f) { (void)f; return 0; }
bool subq_check_checksum(const uint8_t *s) { (void)s; return true; }
void CDIF_ReadTOC(CDIF *c, TOC *o) { (void)c; memset(o, 0, sizeof(*o)); }
void CDIF_HintReadSector(CDIF *c, int32_t l) { (void)c; (void)l; }
bool CDIF_ReadRawSector(CDIF *c, uint8_t *b, int32_t l) { (void)c; (void)b; (void)l; return false; }
bool CDIF_ReadRawSectorPWOnly(CDIF *c, uint8_t *b, int32_t l, bool h) { (void)c; (void)b; (void)l; (void)h; return false; }

/* ---- MPEG card stubs --------------------------------------------- */
static uint8_t  Prog[64 * 2048];
static uint8_t  SrcPart[2]  = { 0xFF, 0xFF };
static unsigned Fed;
static uint8_t  FedFirstByte[64];

bool MPEG_Init(RFILE *f) { (void)f; return true; }
void MPEG_Kill(void) {}
void MPEG_Reset(bool p) { (void)p; }
void MPEG_StateAction(StateMem *sm, const unsigned l, const bool d) { (void)sm; (void)l; (void)d; }
bool MPEG_IsPresent(void) { return true; }
void MPEG_Auth(void) {}
void MPEG_ProgramRead(void) {}
uint16_t MPEG_GetAuth(void) { return 2; }
const uint8_t *MPEG_GetProgram(uint32_t *size) { *size = sizeof(Prog); return Prog; }
bool MPEG_Command(uint8_t c, const uint16_t cd[4], uint8_t b, uint16_t out[4], uint16_t *h)
{ (void)c; (void)cd; (void)b; memset(out, 0, 8); (void)h; return false; }
void MPEG_FeedSector(const uint8_t *d, uint32_t l, uint8_t s, uint8_t p)
{ (void)l; (void)s; (void)p; if(Fed < 64) FedFirstByte[Fed] = d[0]; Fed++; }
uint16_t MPEG_TakePendingHIRQ(void) { return 0; }
void MPEG_Update(int64_t c) { (void)c; }
bool MPEG_GetAudioSample(uint16_t *o) { o[0] = o[1] = 0; return false; }
bool MPEG_WantsPartition(uint8_t p) { return p == SrcPart[0] || p == SrcPart[1]; }
bool MPEG_ConsumesPartition(uint8_t p) { return MPEG_WantsPartition(p); }
bool MPEG_WantsSector(uint8_t p) { return MPEG_ConsumesPartition(p); }
void MPEG_GetSourcePartitions(uint8_t s[2]) { s[0] = SrcPart[0]; s[1] = SrcPart[1]; }

/* ---- harness ----------------------------------------------------- */
static int checks, failures;
#define EXPECT(what, cond) do { checks++; if(!(cond)) { failures++; printf("  FAIL %s\n", what); } } while(0)

static void Run(int32_t clocks)
{
   int32_t t;
   for(t = 0; t < clocks; t += 64)
   {
      SH7095_mem_timestamp += 64;
      CDB_Update(SH7095_mem_timestamp);
   }
}

static void Command(uint16_t c0, uint16_t c1, uint16_t c2, uint16_t c3)
{
   CDB_Write_DBM(0x2, (uint16_t)~(HIRQ_CMOK | HIRQ_CSCT | HIRQ_BFUL | HIRQ_MPED), 0xFFFF);
   CDB_Write_DBM(0x6, c0, 0xFFFF);
   CDB_Write_DBM(0x7, c1, 0xFFFF);
   CDB_Write_DBM(0x8, c2, 0xFFFF);
   CDB_Write_DBM(0x9, c3, 0xFFFF);
   Run(200000);
}

/* Fill a partition with sectors whose first payload byte (offset 24,
   Mode 2) is tag, tag+1, ...  */
static void FillPartition(unsigned pnum, unsigned n, uint8_t tag)
{
   unsigned i;
   for(i = 0; i < n; i++)
   {
      const uint8_t b = Buffer_Allocate(true);
      Buffers[b].Data[15] = 0x2;
      Buffers[b].Data[24] = (uint8_t)(tag + i);
      Partition_LinkBuffer(pnum, b);
   }
}

static void Fresh(void)
{
   unsigned i;

   /* NTSC: 28.63636 MHz master clock, divisor 1 */
   CDB_SetClockRatio((uint32_t)(((1ULL << 32) * 11289600ULL) / 28636363ULL));
   CDB_Init();
   CDB_Reset(true);
   CDB_SetDisc(true, NULL);
   /* Run until the block has come up: its first software reset sets up
      the buffers. */
   for(i = 0; i < 2000 && FreeBufferCount != NumBuffers; i++)
      Run(100000);
   if(FreeBufferCount != NumBuffers)
   {
      printf("  CD block did not come up\n");
      exit(1);
   }
   SrcPart[0] = SrcPart[1] = 0xFF;
   Fed = 0;
}

int main(void)
{
   unsigned i;

   for(i = 0; i < sizeof(Prog); i++)
      Prog[i] = (uint8_t)i;

   /* --- 0xE2 with too few free buffers raises BFUL --------------- */
   printf("[0xE2 buffer full]\n");
   {
      Fresh();
      /* leave 2 free buffers, then ask for 4 sectors into filter 0 */
      FillPartition(5, FreeBufferCount - 2, 0);
      EXPECT("two buffers free", FreeBufferCount == 2);
      Command(0xE200, 0x0000, 0x0000, 0x0004);
      EXPECT("two sectors stored", Partitions[0].Count == 2);
      EXPECT("CSCT raised", (CDB_Read(0x2) & HIRQ_CSCT) != 0);
      EXPECT("BFUL raised", (CDB_Read(0x2) & HIRQ_BFUL) != 0);
   }
   {
      Fresh();
      Command(0xE200, 0x0000, 0x0000, 0x0004);
      EXPECT("four sectors stored", Partitions[0].Count == 4);
      EXPECT("no BFUL with room left", (CDB_Read(0x2) & HIRQ_BFUL) == 0);
   }

   /* --- MPEG_Pull hold-off --------------------------------------- */
   printf("[card pull vs host reads]\n");

   /* Host reads another partition: the card still takes its own. */
   {
      Fresh();
      FillPartition(1, 3, 10);
      FillPartition(2, 2, 50);
      Command(0x6100, 0x0000, 0x0200, 0x0002);   /* Get Sector Data, part 2, offs 0, 2 */
      EXPECT("host transfer active", DT.Active && !DT.Writing);
      Fed = 0;
      SrcPart[0] = 1;
      MPEG_Pull();
      EXPECT("card took all 3 of its sectors", Fed == 3 && Partitions[1].Count == 0);
      EXPECT("host partition untouched", Partitions[2].Count == 2);
   }

   /* Host reads the card's partition from sector 1: the card takes the
      sector ahead of it, then waits. */
   {
      Fresh();
      FillPartition(1, 3, 10);
      Command(0x6100, 0x0001, 0x0100, 0x0002);   /* part 1, offs 1, 2 sectors */
      EXPECT("host transfer active", DT.Active && !DT.Writing);
      Fed = 0;
      SrcPart[0] = 1;
      MPEG_Pull();
      EXPECT("card took only the unread sector", Fed == 1 && FedFirstByte[0] == 10);
      EXPECT("sectors being read stay", Partitions[1].Count == 2);

      /* Host finishes: End Data Transfer, then the card takes the rest
         in order. */
      Command(0x0600, 0, 0, 0);
      EXPECT("transfer ended", !DT.Active);
      MPEG_Pull();
      EXPECT("card took the rest", Fed == 3 && Partitions[1].Count == 0);
      EXPECT("in order", FedFirstByte[1] == 11 && FedFirstByte[2] == 12);
   }

   /* Host reads the card's partition from sector 0: nothing taken. */
   {
      Fresh();
      FillPartition(1, 3, 10);
      Command(0x6100, 0x0000, 0x0100, 0x0003);
      Fed = 0;
      SrcPart[0] = 1;
      MPEG_Pull();
      EXPECT("nothing taken while all are read", Fed == 0 && Partitions[1].Count == 3);
   }

   /* Get Then Delete unlinks what it reads: the card can't collide. */
   {
      Fresh();
      FillPartition(1, 3, 10);
      Command(0x6300, 0x0000, 0x0100, 0x0001);   /* Get Then Delete, part 1, offs 0, 1 */
      Fed = 0;
      SrcPart[0] = 1;
      MPEG_Pull();
      EXPECT("card takes what the host did not", Fed == 2 && FedFirstByte[0] == 11);
   }

   printf("\n%d checks, %d failures\n", checks, failures);
   return failures != 0;
}
