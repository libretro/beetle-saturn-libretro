 /* Plain enums (not 11 `enum : type {}`).  The latter is rejected
  * by MSVC C89 mode and by strict ISO C.  enum constants are always
  * `int` in C; the bool / unsigned typed-enum forms in 11 only
  * affect the constant's underlying storage type, never its value --
  * so the downstream `if(EmulateICache)` / `case which:` macro
  * expansions get the same compile-time fold either way. */
 enum { which = 1 };
 enum { EmulateICache = 1 /* true */ };
 enum { CacheBypassHack = 0 /* false */ };

 if(MDFN_UNLIKELY(z->timestamp >= bound_timestamp))
  return;

 if(z->resume_id)
 {
  const uint16_t id = z->resume_id;
  z->resume_id = 0;
  switch (id)
  {
   #include "sh7095s_ctable.inc"
   default: MDFN_UNREACHABLE;
  }
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
