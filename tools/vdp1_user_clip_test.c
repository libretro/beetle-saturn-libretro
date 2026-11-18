/* Draw with the user clip enabled before any user clip command has run
 * (Shining Wisdom's opening movie does this). Include the implementation
 * to start drawing and read the framebuffer; link the other core objects,
 * excluding vdp1.o. */
#include "../mednafen/ss/vdp1.c"
#include <stdio.h>
static void put_command(unsigned slot, const uint16_t *words)
{
   memcpy(&VDP1_VRAM[slot * 0x10], words, 0x10 * sizeof(uint16_t));
}
int main(void)
{
   /* System clip 319x223, local coordinate 0,0, a 16x16 RGB polygon that
    * draws only inside the user clip (CMDPMOD bit 10 set, bit 9 clear),
    * end. No user clip command. */
   static const uint16_t sysclip[0x10] = { 0x0009, 0, 0, 0, 0, 0, 0, 0, 0, 0, 319, 223 };
   static const uint16_t localc[0x10] = { 0x000A, 0, 0, 0, 0, 0, 0, 0 };
   static const uint16_t polygon[0x10] = { 0x0004, 0, 0x04C0 | (5 << 3), 0x801F, 0, 0,
                                           0, 0, 15, 0, 15, 15, 0, 15 };
   static const uint16_t end[0x10] = { 0x8000 };
   sscpu_timestamp_t ts = 0;
   unsigned x, y, drawn = 0, loops = 0;
   VDP1_Init();
   VDP1_Reset(true);
   put_command(0, sysclip);
   put_command(1, localc);
   put_command(2, polygon);
   put_command(3, end);
   StartDrawing();
   while (DrawingActive)
   {
      ts += 1000;
      VDP1_Update(ts);
      if (++loops > 100000) return 2;
   }
   for (y = 0; y < 16; y++)
      for (x = 0; x < 16; x++)
         if (VDP1_FB[FBDrawWhich][(y << 9) + x] == 0x801F) drawn++;
   if (drawn != 256)
   {
      fprintf(stderr, "vdp1_user_clip_test: %u of 256 pixels drawn\n", drawn);
      return 1;
   }
   return 0;
}
