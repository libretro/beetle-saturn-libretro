 enum : unsigned { which = 1 };
 enum : bool { EmulateICache = true };
 enum : bool { DebugMode = SH7095_DEBUG_MODE };
 enum : bool { CacheBypassHack = false };

 if(MDFN_UNLIKELY(z->timestamp >= bound_timestamp))
  return;

 if(z->ResumePoint)
 {
  const void* const tmp = z->ResumePoint;
  z->ResumePoint = nullptr;
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
