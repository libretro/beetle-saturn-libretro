 /* Plain enums (not C++11 `enum : type {}`).  The latter is rejected
  * by MSVC C89 mode and by strict ISO C.  enum constants are always
  * `int` in C; the bool / unsigned typed-enum forms in C++11 only
  * affect the constant's underlying storage type, never its value --
  * so the downstream `if(EmulateICache)` / `case which:` /
  * `if(DebugMode)` macro expansions get the same compile-time fold
  * either way. */
 enum { which = 1 };
 enum { EmulateICache = 1 /* true */ };
 enum { DebugMode = SH7095_DEBUG_MODE };
 enum { CacheBypassHack = 0 /* false */ };

 if(MDFN_UNLIKELY(z->timestamp >= bound_timestamp))
  return;

 if(z->ResumePoint)
 {
  const void* const tmp = z->ResumePoint;
  z->ResumePoint = NULL;
  goto *tmp;
 }

 do
 {
  //
  // Ideally, we would place SPEPRecover: after the z->FRT event check, but doing
  // so causes gcc(multiple versions) to produce inconceivably awful code under certain conditions
  // (such as disabling all the SS_DBG stuff at compile-time) because it thinks it's an important loop
  // or something?(even with all our branch hinting!)
  //
  SPEPRecover:;

  if(MDFN_UNLIKELY(z->timestamp >= z->FRT_WDT_NextTS))
  {
   SH7095_FRT_WDT_Update(z);
   SH7095_FRT_WDT_Recalc_NET(z);
  }

  instr = (uint16_t)z->Pipe_ID;
  OP_PAIR_TICK(1);
  //
  #include "sh7095_ops.inc"
  //
 } while(z->timestamp < bound_timestamp);
