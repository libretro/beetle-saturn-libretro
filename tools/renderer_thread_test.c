/* Exercise the actual renderer queue with a deliberately tiny capacity.
 * Include the implementation to inspect its private VRAM after draining;
 * link the other core objects, excluding vdp2_render.o. */
#define WQ_SIZE 64u
#include "../mednafen/ss/vdp2_render.c"
#include <stdio.h>
#include <features/features_cpu.h>
static void drain(void)
{
#ifdef WANT_THREADING
   while (VDP2_ATOMIC_LOAD_ACQ(WQ_PopCount) != Prod.PushLocal)
   {
      int key = retro_eventcount_prepare_wait(&DrainEvent);
      /* Queued commands do not wake the renderer on their own; every
       * wait for it has to (see VDP2REND_StateAction). */
      WakeConsumer();
      if (VDP2_ATOMIC_LOAD_ACQ(WQ_PopCount) != Prod.PushLocal)
         retro_eventcount_commit_wait(&DrainEvent, key);
      else retro_eventcount_cancel_wait(&DrainEvent);
   }
#endif
}
int main(void)
{
   unsigned round, i;
   int64_t start = cpu_features_get_time_usec();
   uint16_t words[512];
   for (round = 0; round < 4; round++)
   {
      if (!VDP2REND_Init(false, 0)) return 1;
      for (i = 0; i < 200000; i++)
         VDP2REND_Write16_DB((i & 0x3fff) * 2, (uint16_t)i);
      drain();
      for (i = 0; i < 0x4000; i++)
      {
         unsigned last = i + ((199999 - i) / 0x4000) * 0x4000;
         if (VRAM[i] != (uint16_t)last) return 2;
      }
      for (i = 0; i < 5000; i++)
      {
         unsigned j;
         for (j = 0; j < 512; j++) words[j] = (uint16_t)(i + j);
         VDP2REND_WriteBurst16_DB(0, 512, 1, words);
      }
      drain();
      for (i = 0; i < 512; i++)
         if (VRAM[i] != (uint16_t)(4999 + i)) return 3;
      VDP2REND_Write8_DB(0, 0x1200);
      VDP2REND_Write8_DB(1, 0x0034);
      drain();
      if (VRAM[0] != 0x1234) return 4;
      /* State serialization requires the same full queue drain. */
      {
         StateMem sm;
         uint16_t rr[256], cr[2048], vr[0x40000];
         memset(&sm, 0, sizeof(sm));
         VDP2REND_Write16_DB(0, 0xbeef);
         VDP2REND_StateAction(&sm, 0, false, rr, cr, vr);
         if (VRAM[0] != 0xbeef || VDP2_ATOMIC_LOAD_ACQ(WQ_PopCount) != Prod.PushLocal) return 5;
         free(sm.data);
      }
      /* Actual DRAW_LINE completion and EndFrame synchronization. */
      {
         EmulateSpecStruct spec;
         MDFN_Surface *surface = MDFN_Surface_New(704, 576, 704);
         int32_t widths[576];
         if (!surface) return 6;
         memset(&spec, 0, sizeof(spec));
         memset(widths, 0, sizeof(widths));
         spec.surface = surface;
         spec.LineWidths = widths;
         VDP2REND_Reset(true);
         for (i = 0; i < 20; i++)
         {
            unsigned line;
            VDP2REND_StartFrame(&spec, false, -1);
            for (line = 0; line < 240; line++) VDP2REND_DrawLine(line, line, false);
            VDP2REND_EndFrame();
            if (VDP2_ATOMIC_LOAD_ACQ(DrawFinishCount) != Prod.DrawPushLocal) return 7;
         }
         MDFN_Surface_Delete(surface);
      }
      /* Drain while sleeping, followed by shutdown with queued work. */
      retro_sleep(2);
      for (i = 0; i < 1000; i++) VDP2REND_Write16_DB(0, i);
      VDP2REND_Kill();
      VDP2REND_Kill();
   }
   printf("Renderer PASS: tiny queue back-pressure, 800k writes, 20k bursts, state drain and restart; %.3f s\n",
         (cpu_features_get_time_usec() - start) / 1000000.0);
   return 0;
}
