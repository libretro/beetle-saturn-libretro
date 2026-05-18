switch(z->Pipe_ID >> 24)
{
 #include "sh7095_opdefs.inc"

 #define PART_OP_NORMIDIF { DoIDIF(false); }
 
 #define BEGIN_OP(x)		OP_##x { PART_OP_NORMIDIF
 #define BEGIN_OP_FUDGEIF(x)	OP_##x { if(EmulateICache) { z->UCRead_IF_Kludge = false; } DoIDIF(false); if(EmulateICache) { z->timestamp -= z->UCRead_IF_Kludge; }
 #define BEGIN_OP_DLYIDIF(x)	OP_##x {

 // "Interrupt-disabled" instruction(blocks interrupt from being taken for next instruction).
 // Use with BEGIN_OP_DLYIDIF()
 #define PART_OP_INTDIS { DoIDIF(true); }

 #define END_OP } break;

 // Pair fusion: at the end of a setter-of-T's handler, after its DoIDIF has
 // populated z->Pipe_ID with the next op-id, take the inlined cond-branch path
 // when the next op is the matching BF/BT (and no exception is pending --
 // z->EPending ORs 0xFF000000 into z->Pipe_ID, so the equality fails closed).
 // The extra z->PC += 2 compensates for the post-switch z->PC += 2 only firing
 // once for the two folded instructions; cycle accounting matches the
 // unfused path (one fetch in z op's DoIDIF, one in the inlined branch's).
 // Skipped in the slave resume path (SH7095_RunSlaveUntil_TBD_) because DoIDIF can yield
 // via CHECK_EXIT_RESUME and br_instr is a stack local that won't survive.
 #if !defined(SH7095_OP_PAIR_PROFILE) && !defined(SH7095_NEED_RESUME_TABLE_INIT) && defined(_LOW_ACCURACY_)
 #define FUSE_COND_BRANCH(next_op_id, cond_expr)				\
  if(MDFN_LIKELY((z->Pipe_ID >> 24) == (next_op_id)))			\
  {									\
   const uint16_t br_instr = (uint16_t)z->Pipe_ID;				\
   z->PC += 2;								\
   if(EmulateICache) { z->UCRead_IF_Kludge = false; }			\
   DoIDIF(false);							\
   if(EmulateICache) { z->timestamp -= z->UCRead_IF_Kludge; }			\
   CondRelBranch((cond_expr), (uint32_t)(int8_t)br_instr << 1);		\
  }
 // Delay-slot fusion: collapse a NOP delay slot into its preceding
 // delay-branch handler.  After DelayBranch(), z->Pipe_ID's high byte is
 // (InstrDecodeTab[delay_slot] | 0x80) -- so for a NOP delay slot it's
 // 0xF5.  We inline the NOP's empty-body dispatch as z->PC += 2 + DoIDIF,
 // saving one switch round-trip when z fires.  Hot for the slave's
 // interrupt-storm cycle (RTE -> NOP delay -> PSEUDO_EPENDING -> RTE),
 // where RTE -> NOP transitions are 100% in profiling.
 #define FUSE_DELAY_SLOT_NOP()						\
  if(MDFN_LIKELY((z->Pipe_ID >> 24) == (OP_NOP_ID | 0x80)))		\
  {									\
   z->PC += 2;								\
   DoIDIF(false);							\
  }
 // Triplet fusion: load -> cmp/tst -> conditional branch.  At the load's
 // tail, z->Pipe_ID has the cmp/tst (decoded by the load's BEGIN_OP DoIDIF)
 // and z->Pipe_IF has the branch instruction word (raw uint16_t, op-id NOT yet
 // computed -- DoID decodes during rotation).  Detection therefore needs an
 // explicit InstrDecodeTab lookup on z->Pipe_IF; the high byte of z->Pipe_IF is
 // always zero (FetchIF only writes a uint16_t) so testing it would fail
 // closed.  alu_set_t_expr reads alu_instr (uint16_t) as the cmp/tst word.
 // The two DoIDIF_INLINE calls model the cmp/tst's and the branch's fetch
 // slots; z->PC interleaves +=2 between them so each fetches the right address.
 // The branch's slot keeps its FUDGEIF kludge wrap.  Total z->PC advance =
 // 2+2 inside macro + 2 post-switch = 6 bytes for 3 folded instructions.
 #define FUSE_LOAD_CMPTST_BRANCH(alu_op_id, br_op_id, alu_set_t_expr, br_cond_expr)	\
  if(MDFN_LIKELY((z->Pipe_ID >> 24) == (alu_op_id) &&					\
                 InstrDecodeTab[(uint16_t)z->Pipe_IF] == (br_op_id)))			\
  {											\
   const uint16_t alu_instr = (uint16_t)z->Pipe_ID;						\
   const uint16_t br_instr = (uint16_t)z->Pipe_IF;						\
   WB_EX_CHECK(0)									\
   alu_set_t_expr;									\
   z->PC += 2;										\
   { if(EmulateICache) SH7095_DoIDIF_INLINE_C1_I0(z); else SH7095_DoIDIF_INLINE_C0_I0(z); }						\
   z->PC += 2;										\
   if(EmulateICache) { z->UCRead_IF_Kludge = false; }					\
   { if(EmulateICache) SH7095_DoIDIF_INLINE_C1_I0(z); else SH7095_DoIDIF_INLINE_C0_I0(z); }						\
   if(EmulateICache) { z->timestamp -= z->UCRead_IF_Kludge; }					\
   CondRelBranch((br_cond_expr), (uint32_t)(int8_t)br_instr << 1);				\
  }
 #else
 #define FUSE_COND_BRANCH(next_op_id, cond_expr) do {} while(0)
 #define FUSE_LOAD_CMPTST_BRANCH(alu_op_id, br_op_id, alu_set_t_expr, br_cond_expr) do {} while(0)
 #define FUSE_DELAY_SLOT_NOP() do {} while(0)
 #endif

 #define WB_EX_CHECK(r) { if(z->timestamp < z->WB_until[(r)]) z->timestamp = z->WB_until[(r)]; }
 #define WB_WRITE(r, v) { z->R[(r)] = (v); z->WB_until[(r)] = z->MA_until + 1; }
 #define WB_READ8(r, A) { uint8_t memrv; MemRead8(A, memrv); WB_WRITE((r), (int8_t)memrv); }
 #define WB_READ16(r, A) { uint16_t memrv; MemRead16(A, memrv); WB_WRITE((r), (int16_t)memrv); }
 #define WB_READ32(r, A) { uint32_t memrv; MemRead32(A, memrv); WB_WRITE((r), memrv); }
 #define WB_READ16_I(r, A) { uint16_t memrv; MemRead16_I(A, memrv); WB_WRITE((r), (int16_t)memrv); }
 #define WB_READ32_I(r, A) { uint32_t memrv; MemRead32_I(A, memrv); WB_WRITE((r), memrv); }
 //
 //
 //
 //
 // MOV #imm,Rn
 //
 BEGIN_OP(MOV_IMM_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(uint32_t, imm) = (int8_t)instr;

	WB_EX_CHECK(n)

	z->R[n] = imm;
 END_OP


 //
 // MOV.W @(disp,z->PC),Rn
 //
 BEGIN_OP(MOV_W_PCREL_REG)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->PC + (d << 1);

	WB_READ16_I(instr_nyb2, ea);
 END_OP


 //
 // MOV.L @(disp,z->PC),Rn
 //
 BEGIN_OP(MOV_L_PCREL_REG)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = (z->PC &~ 0x3) + (d << 2);

	WB_READ32_I(instr_nyb2, ea);
 END_OP


 //
 // MOV Rm,Rn
 //
 BEGIN_OP(MOV_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = z->R[m];
 END_OP


 //
 // MOV.B Rm,@Rn
 //
 BEGIN_OP(MOV_B_REG_REGINDIR)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, val) = z->R[m];
	CONST_VAR(uint32_t, ea) = z->R[n];

	MemWrite8(ea, val);
 END_OP


 //
 // MOV.W Rm,@Rn
 //
 BEGIN_OP(MOV_W_REG_REGINDIR)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, val) = z->R[m];
	CONST_VAR(uint32_t, ea) = z->R[n];

	MemWrite16(ea, val);
 END_OP


 //
 // MOV.L Rm,@Rn
 //
 BEGIN_OP(MOV_L_REG_REGINDIR)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, val) = z->R[m];
	CONST_VAR(uint32_t, ea) = z->R[n];

	MemWrite32(ea, val);
 END_OP


 //
 // MOV.B @Rm,Rn
 //
 BEGIN_OP(MOV_B_REGINDIR_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[m];

	WB_READ8(instr_nyb2, ea);

	FUSE_LOAD_CMPTST_BRANCH(OP_CMP_EQ_IMM_REG0_ID, OP_BF_ID,
		SH7095_SetT(z, (int32_t)(int8_t)alu_instr == (int32_t)z->R[0]), !SH7095_GetT(z));
 END_OP


 //
 // MOV.W @Rm,Rn
 //
 BEGIN_OP(MOV_W_REGINDIR_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[m];

	WB_READ16(instr_nyb2, ea);
 END_OP


 //
 // MOV.L @Rm,Rn
 //
 BEGIN_OP(MOV_L_REGINDIR_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[m];

	WB_READ32(instr_nyb2, ea);
 END_OP


 //
 // MOV.B Rm,@-Rn
 //
 BEGIN_OP(MOV_B_REG_REGINDIRPD)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, val) = z->R[m];
	uint32_t ea;

	z->R[n] -= 1;
	ea = z->R[n];

	MemWrite8(ea, val);
 END_OP


 //
 // MOV.W Rm,@-Rn
 //
 BEGIN_OP(MOV_W_REG_REGINDIRPD)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, val) = z->R[m];
	uint32_t ea;

	z->R[n] -= 2;
	ea = z->R[n];

	MemWrite16(ea, val);
 END_OP


 //
 // MOV.L Rm,@-Rn
 //
 BEGIN_OP(MOV_L_REG_REGINDIRPD)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, val) = z->R[m];
	uint32_t ea;

	z->R[n] -= 4;
	ea = z->R[n];

	MemWrite32(ea, val);
 END_OP


 //
 // MOV.B @Rm+,Rn
 //
 BEGIN_OP(MOV_B_REGINDIRPI_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	uint32_t ea;

	ea = z->R[m];
	z->R[m] += 1;

	WB_READ8(instr_nyb2, ea);
 END_OP


 //
 // MOV.W @Rm+,Rn
 //
 BEGIN_OP(MOV_W_REGINDIRPI_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	uint32_t ea;

	ea = z->R[m];
	z->R[m] += 2;

	WB_READ16(instr_nyb2, ea);
 END_OP


 //
 // MOV.L @Rm+,Rn
 //
 BEGIN_OP(MOV_L_REGINDIRPI_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	uint32_t ea;

	ea = z->R[m];
	z->R[m] += 4;

	WB_READ32(instr_nyb2, ea);
 END_OP


 //
 // MOV.B R0,@(disp,Rn)
 //
 BEGIN_OP(MOV_B_REG0_REGINDIRDISP)
	CONST_VAR(unsigned, n) = instr_nyb1;
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xf;
	CONST_VAR(uint32_t, ea) = z->R[n] + (d << 0);

	MemWrite8(ea, z->R[0]);
 END_OP


 //
 // MOV.W R0,@(disp,Rn)
 //
 BEGIN_OP(MOV_W_REG0_REGINDIRDISP)
	CONST_VAR(unsigned, n) = instr_nyb1;
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xf;
	CONST_VAR(uint32_t, ea) = z->R[n] + (d << 1);

	MemWrite16(ea, z->R[0]);
 END_OP


 //
 // MOV.L Rm,@(disp,Rn)
 //
 BEGIN_OP(MOV_L_REG_REGINDIRDISP)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xf;
	CONST_VAR(uint32_t, ea) = z->R[n] + (d << 2);

	MemWrite32(ea, z->R[m]);
 END_OP


 //
 // MOV.B @(disp,Rm),R0
 //
 BEGIN_OP(MOV_B_REGINDIRDISP_REG0)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xf;
	CONST_VAR(uint32_t, ea) = z->R[m] + (d << 0);

	WB_READ8(0, ea);

	FUSE_LOAD_CMPTST_BRANCH(OP_TST_IMM_REG0_ID, OP_BT_ID,
		SH7095_SetT(z, !(z->R[0] & (uint8_t)alu_instr)), SH7095_GetT(z));
 END_OP


 //
 // MOV.W @(disp,Rm),R0
 //
 BEGIN_OP(MOV_W_REGINDIRDISP_REG0)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xf;
	CONST_VAR(uint32_t, ea) = z->R[m] + (d << 1);

	WB_READ16(0, ea);
 END_OP


 //
 // MOV.L @(disp,Rm),Rn
 //
 BEGIN_OP(MOV_L_REGINDIRDISP_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xf;
	CONST_VAR(uint32_t, ea) = z->R[m] + (d << 2);

	WB_READ32(instr_nyb2, ea);
 END_OP


 //
 // MOV.B Rm,@(R0,Rn)
 //
 BEGIN_OP(MOV_B_REG_IDXREGINDIR)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[0] + z->R[n];

	MemWrite8(ea, z->R[m]);
 END_OP


 //
 // MOV.W Rm,@(R0,Rn)
 //
 BEGIN_OP(MOV_W_REG_IDXREGINDIR)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[0] + z->R[n];

	MemWrite16(ea, z->R[m]);
 END_OP


 //
 // MOV.L Rm,@(R0,Rn)
 //
 BEGIN_OP(MOV_L_REG_IDXREGINDIR)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[0] + z->R[n];

	MemWrite32(ea, z->R[m]);
 END_OP


 //
 // MOV.B @(R0,Rm),Rn
 //
 BEGIN_OP(MOV_B_IDXREGINDIR_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[0] + z->R[m];

	WB_READ8(instr_nyb2, ea);
 END_OP


 //
 // MOV.W @(R0,Rm),Rn
 //
 BEGIN_OP(MOV_W_IDXREGINDIR_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[0] + z->R[m];

	WB_READ16(instr_nyb2, ea);
 END_OP


 //
 // MOV.L @(R0,Rm),Rn
 //
 BEGIN_OP(MOV_L_IDXREGINDIR_REG)
	CONST_VAR(unsigned, m) = instr_nyb1;
	CONST_VAR(uint32_t, ea) = z->R[0] + z->R[m];

	WB_READ32(instr_nyb2, ea);
 END_OP


 //
 // MOV.B R0,@(disp,z->GBR)
 //
 BEGIN_OP(MOV_B_REG0_GBRINDIRDISP)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->GBR + (d << 0);

	MemWrite8(ea, z->R[0]);
 END_OP


 //
 // MOV.W R0,@(disp,z->GBR)
 //
 BEGIN_OP(MOV_W_REG0_GBRINDIRDISP)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->GBR + (d << 1);

	MemWrite16(ea, z->R[0]);
 END_OP


 //
 // MOV.L R0,@(disp,z->GBR)
 //
 BEGIN_OP(MOV_L_REG0_GBRINDIRDISP)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->GBR + (d << 2);

	MemWrite32(ea, z->R[0]);
 END_OP


 //
 // MOV.B @(disp,z->GBR),R0
 //
 BEGIN_OP(MOV_B_GBRINDIRDISP_REG0)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->GBR + (d << 0);

	WB_READ8(0, ea);
 END_OP


 //
 // MOV.W @(disp,z->GBR),R0
 //
 BEGIN_OP(MOV_W_GBRINDIRDISP_REG0)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->GBR + (d << 1);

	WB_READ16(0, ea);
 END_OP


 //
 // MOV.L @(disp,z->GBR),R0
 //
 BEGIN_OP(MOV_L_GBRINDIRDISP_REG0)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = z->GBR + (d << 2);

	WB_READ32(0, ea);
 END_OP


 //
 // MOVA @(disp,z->PC),R0
 //
 BEGIN_OP(MOVA_PCREL_REG0)
	CONST_VAR(unsigned, d) = (instr >> 0) & 0xff;
	CONST_VAR(uint32_t, ea) = (z->PC &~ 0x3) + (d << 2);

	WB_EX_CHECK(0)

	z->R[0] = ea;
 END_OP


 //
 // MOVT Rn
 //
 BEGIN_OP(MOVT_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] = SH7095_GetT(z);
 END_OP


 //
 // SWAP.B Rm,Rn
 //
 BEGIN_OP(SWAP_B_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (z->R[m] & 0xFFFF0000) | ((z->R[m] << 8) & 0xFF00) | ((z->R[m] >> 8) & 0x00FF);
 END_OP


 //
 // SWAP.W Rm,Rn
 //
 BEGIN_OP(SWAP_W_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (z->R[m] << 16) | (z->R[m] >> 16);
 END_OP


 //
 // XTRCT Rm,Rn
 //
 BEGIN_OP(XTRCT_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (z->R[n] >> 16) | (z->R[m] << 16);
 END_OP


 //
 // ADD Rm,Rn
 //
 BEGIN_OP(ADD_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = z->R[n] + z->R[m];
 END_OP


 //
 // ADD #imm,Rn
 //
 BEGIN_OP(ADD_IMM_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(uint32_t, imm) = (int8_t)instr;

	WB_EX_CHECK(n)

	z->R[n] = z->R[n] + imm;
 END_OP


 //
 // ADDC Rm,Rn
 //
 BEGIN_OP(ADDC_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint64_t result = (uint64_t)z->R[n] + z->R[m] + SH7095_GetT(z);

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = result;
	SH7095_SetT(z, (result >> 32) & 1);
 END_OP


 //
 // ADDV Rm,Rn
 //
 BEGIN_OP(ADDV_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint32_t result = z->R[n] + z->R[m];

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, ((~(z->R[n] ^ z->R[m])) & (z->R[n] ^ result)) >> 31);
	z->R[n] = result;
 END_OP


 //
 // CMP/EQ #imm,R0
 //
 BEGIN_OP(CMP_EQ_IMM_REG0)
	CONST_VAR(uint32_t, imm) = (int8_t)instr;

	WB_EX_CHECK(0)

	SH7095_SetT(z, imm == z->R[0]);

	FUSE_COND_BRANCH(OP_BF_ID, !SH7095_GetT(z));
 END_OP


 //
 // CMP/EQ Rm,Rn
 //
 BEGIN_OP(CMP_EQ_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, z->R[n] == z->R[m]);
 END_OP


 //
 // CMP/HS Rm,Rn
 //
 BEGIN_OP(CMP_HS_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, z->R[n] >= z->R[m]);
 END_OP


 //
 // CMP/GE Rm,Rn
 //
 BEGIN_OP(CMP_GE_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, (int32_t)z->R[n] >= (int32_t)z->R[m]);
 END_OP


 //
 // CMP/HI Rm,Rn
 //
 BEGIN_OP(CMP_HI_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, z->R[n] > z->R[m]);
 END_OP


 //
 // CMP/GT Rm,Rn
 //
 BEGIN_OP(CMP_GT_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, (int32_t)z->R[n] > (int32_t)z->R[m]);

	FUSE_COND_BRANCH(OP_BT_ID, SH7095_GetT(z));
 END_OP


 //
 // CMP/PZ Rn
 //
 BEGIN_OP(CMP_PZ_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	SH7095_SetT(z, (int32_t)z->R[n] >= 0);
 END_OP


 //
 // CMP/PL Rn
 //
 BEGIN_OP(CMP_PL_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	SH7095_SetT(z, (int32_t)z->R[n] > 0);
 END_OP


 //
 // CMP/STR Rm,Rn
 //
 BEGIN_OP(CMP_STR_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint32_t tmp = z->R[n] ^ z->R[m];
	unsigned new_T = 0;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	if(!(tmp & 0x000000FF)) new_T = 1;
	if(!(tmp & 0x0000FF00)) new_T = 1;
	if(!(tmp & 0x00FF0000)) new_T = 1;
	if(!(tmp & 0xFF000000)) new_T = 1;

	SH7095_SetT(z, new_T);
 END_OP


 //
 // DIV1 Rm,Rn
 //
 BEGIN_OP(DIV1_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	uint32_t tmp;
	bool new_Q;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	new_Q = z->R[n] >> 31;
	z->R[n] <<= 1;
	z->R[n] |= SH7095_GetT(z);

	tmp = z->R[n];
	new_Q ^= SH7095_GetM(z);

	if(!(SH7095_GetQ(z) ^ SH7095_GetM(z)))
	{
	 z->R[n] -= z->R[m];
	 new_Q ^= (z->R[n] > tmp);
	}
	else
	{
	 z->R[n] += z->R[m];
	 new_Q ^= (z->R[n] < tmp);
	}

	SH7095_SetQ(z, new_Q);
	SH7095_SetT(z, new_Q == SH7095_GetM(z));
 END_OP


 //
 // DIV0S Rm,Rn
 //
 BEGIN_OP(DIV0S_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const unsigned new_Q = z->R[n] >> 31;
	const unsigned new_M = z->R[m] >> 31;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetQ(z, new_Q);
	SH7095_SetM(z, new_M);
	SH7095_SetT(z, new_Q != new_M);
 END_OP


 //
 // DIV0U 
 //
 BEGIN_OP(DIV0U)
	SH7095_SetQ(z, false);
	SH7095_SetM(z, false);
	SH7095_SetT(z, false);
 END_OP


 //
 // DT 
 //
 BEGIN_OP(DT)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n]--;
	SH7095_SetT(z, !z->R[n]);

	FUSE_COND_BRANCH(OP_BF_ID, !SH7095_GetT(z));
 END_OP


 //
 // EXTS.B Rm,Rn
 //
 BEGIN_OP(EXTS_B_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (int8_t)z->R[m];
 END_OP


 //
 // EXTS.W Rm,Rn
 //
 BEGIN_OP(EXTS_W_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (int16_t)z->R[m];
 END_OP


 //
 // EXTU.B Rm,Rn
 //
 BEGIN_OP(EXTU_B_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (uint8_t)z->R[m];
 END_OP


 //
 // EXTU.W Rm,Rn
 //
 BEGIN_OP(EXTU_W_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = (uint16_t)z->R[m];
 END_OP


 //
 // MAC.L @Rm+,@Rn+
 //
 //  Pipeline: page 188(not implemented right here)
 BEGIN_OP(MAC_L)
	RESUME_VAR(int32_t, opexec_MAC_L_m0);
	RESUME_VAR(int32_t, opexec_MAC_L_m1);
	{
	 // Order confirmed.
	 MemRead32(z->R[instr_nyb1], opexec_MAC_L_m0);
	 z->R[instr_nyb1] += 4;
	 MemRead32(z->R[instr_nyb2], opexec_MAC_L_m1);
	 z->R[instr_nyb2] += 4;
	}
	uint64_t a, b, sum;

	a = SH7095_GetMAC64(z);
	b = (int64_t)opexec_MAC_L_m0 * opexec_MAC_L_m1;
	sum = a + b;

	if(SH7095_GetS(z) && sum > 0x00007FFFFFFFFFFFULL && sum < 0xFFFF800000000000ULL)
	{
	 if((int32_t)(opexec_MAC_L_m0 ^ opexec_MAC_L_m1) < 0)
	  sum = 0xFFFF800000000000ULL;
	 else
	  sum = 0x00007FFFFFFFFFFFULL;
	}

	SH7095_SetMAC64(z, sum);

	z->timestamp++;
 END_OP


 //
 // MAC.W @Rm+,@Rn+
 //
 //  Pipeline: page 180(not implemented right here)
 BEGIN_OP(MAC_W)
	RESUME_VAR(int16_t, opexec_MAC_W_m0);
	RESUME_VAR(int16_t, opexec_MAC_W_m1);
	{
	 // Order confirmed.
	 MemRead16(z->R[instr_nyb1], opexec_MAC_W_m0);
	 z->R[instr_nyb1] += 2;
         MemRead16(z->R[instr_nyb2], opexec_MAC_W_m1);
	 z->R[instr_nyb2] += 2;
	}

	if(SH7095_GetS(z))
	{
	 int32_t b = (int32_t)opexec_MAC_W_m0 * opexec_MAC_W_m1;
	 uint64_t sum = (int64_t)(int32_t)z->MACL + b;

	 if(sum > 0x000000007FFFFFFFULL && sum < 0xFFFFFFFF80000000ULL)
	 {
	  z->MACH |= 1;

	  if(b < 0)
	   sum = 0x80000000ULL;
	  else
	   sum = 0x7FFFFFFFULL;
	 }

	 z->MACL = sum;
	}
	else
	 SH7095_SetMAC64(z, SH7095_GetMAC64(z) + (int64_t)opexec_MAC_W_m0 * opexec_MAC_W_m1);

	z->timestamp++;
 END_OP


 //
 // DMULS.L Rm,Rn
 //
 //  Pipeline: page 215 (not implemented here totally correctly)
 BEGIN_OP_DLYIDIF(DMULS_L_REG_REG)
	{
	 z->timestamp++;

	 { sscpu_timestamp_t t__ = z->timestamp + 1; if(z->MM_until > z->MA_until) z->MA_until = z->MM_until; if(t__ > z->MA_until) z->MA_until = t__; }
	 z->MM_until = z->MA_until + 4;
	 PART_OP_NORMIDIF
	}
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint64_t result = (int64_t)(int32_t)z->R[n] * (int32_t)z->R[m];

	z->MACL = result >> 0;
	z->MACH = result >> 32;
 END_OP


 //
 // DMULU.L Rm,Rn
 //
 //  Pipeline: page 215 (not implemented here totally correctly)
 BEGIN_OP_DLYIDIF(DMULU_L_REG_REG)
	{
	 z->timestamp++;

	 { sscpu_timestamp_t t__ = z->timestamp + 1; if(z->MM_until > z->MA_until) z->MA_until = z->MM_until; if(t__ > z->MA_until) z->MA_until = t__; }
	 z->MM_until = z->MA_until + 4;
	 PART_OP_NORMIDIF
	}
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint64_t result = (uint64_t)z->R[n] * z->R[m];

	z->MACL = result >> 0;
	z->MACH = result >> 32;
 END_OP


 //
 // MUL.L Rm,Rn
 //
 //  Pipeline: page 215 (not implemented here totally correctly)
 BEGIN_OP_DLYIDIF(MUL_L_REG_REG)
	{
	 z->timestamp++;

	 { sscpu_timestamp_t t__ = z->timestamp + 1; if(z->MM_until > z->MA_until) z->MA_until = z->MM_until; if(t__ > z->MA_until) z->MA_until = t__; }
	 z->MM_until = z->MA_until + 4;
	 PART_OP_NORMIDIF
	}
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	z->MACL = z->R[n] * z->R[m];
 END_OP


 //
 // MULS.W Rm,Rn
 //
 //  Pipeline: page 207
 BEGIN_OP(MULS_W_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	{ sscpu_timestamp_t t__ = z->timestamp + 1; if(z->MM_until > z->MA_until) z->MA_until = z->MM_until; if(t__ > z->MA_until) z->MA_until = t__; }
	z->MM_until = z->MA_until + 2;

	z->MACL = (int16_t)z->R[n] * (int16_t)z->R[m];
 END_OP


 //
 // MULU.W Rm,Rn
 //
 //  Pipeline: page 207
 BEGIN_OP(MULU_W_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	{ sscpu_timestamp_t t__ = z->timestamp + 1; if(z->MM_until > z->MA_until) z->MA_until = z->MM_until; if(t__ > z->MA_until) z->MA_until = t__; }
	z->MM_until = z->MA_until + 2;

	z->MACL = (uint32_t)(uint16_t)z->R[n] * (uint32_t)(uint16_t)z->R[m];
 END_OP


 //
 // NEG Rm,Rn
 //
 BEGIN_OP(NEG_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = -z->R[m];
 END_OP


 //
 // NEGC Rm,Rn
 //
 BEGIN_OP(NEGC_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint64_t result = (uint64_t)0 - z->R[m] - SH7095_GetT(z);

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = result;
	SH7095_SetT(z, (result >> 32) & 1);
 END_OP


 //
 // SUB Rm,Rn
 //
 BEGIN_OP(SUB_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = z->R[n] - z->R[m];
 END_OP


 //
 // SUBC Rm,Rn
 //
 BEGIN_OP(SUBC_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint64_t result = (uint64_t)z->R[n] - z->R[m] - SH7095_GetT(z);

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = result;
	SH7095_SetT(z, (result >> 32) & 1);
 END_OP


 //
 // SUBV Rm,Rn
 //
 BEGIN_OP(SUBV_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;
	const uint32_t result = z->R[n] - z->R[m];

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, (((z->R[n] ^ z->R[m])) & (z->R[n] ^ result)) >> 31);
	z->R[n] = result;
 END_OP


 //
 // AND Rm,Rn
 //
 BEGIN_OP(AND_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = z->R[n] & z->R[m];
 END_OP


 //
 // AND #imm,R0
 //
 BEGIN_OP(AND_IMM_REG0)
	const unsigned imm = (uint8_t)instr;

	WB_EX_CHECK(0)

	z->R[0] = z->R[0] & imm;
 END_OP


 //
 // AND.B #imm,@(R0,z->GBR)
 //
 BEGIN_OP(AND_B_IMM_IDXGBRINDIR)
	RESUME_VAR(uint32_t, opexec_ea);
	opexec_ea = z->R[0] + z->GBR;
	uint32_t tmp;

	MemRead8(opexec_ea, tmp);
	z->timestamp++;
	tmp &= (uint8_t)instr;
	MemWrite8(opexec_ea, tmp);
	z->timestamp++;
 END_OP


 //
 // NOT Rm,Rn
 //
 BEGIN_OP(NOT_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = ~z->R[m];
 END_OP


 //
 // OR Rm,Rn
 //
 BEGIN_OP(OR_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] |= z->R[m];
 END_OP


 //
 // OR #imm,R0
 //
 BEGIN_OP(OR_IMM_REG0)
	const unsigned imm = (uint8_t)instr;

	WB_EX_CHECK(0)

	z->R[0] |= imm;
 END_OP


 //
 // OR.B #imm,@(R0,z->GBR)
 //
 BEGIN_OP(OR_B_IMM_IDXGBRINDIR)
	RESUME_VAR(uint32_t, opexec_ea);
	opexec_ea = z->R[0] + z->GBR;
	uint32_t tmp;

	MemRead8(opexec_ea, tmp);
	z->timestamp++;
	tmp |= (uint8_t)instr;
	MemWrite8(opexec_ea, tmp);
	z->timestamp++;
 END_OP


 //
 // TAS.B @Rn
 //
 BEGIN_OP(TAS_B_REGINDIR)
	uint8_t tmp;
	RESUME_VAR(uint32_t, opexec_ea);

	opexec_ea = z->R[instr_nyb2];

	CHECK_EXIT_RESUME();

	SH7095_BusLock++;
	tmp = ExtBusRead(uint8_t, false, opexec_ea);	// FIXME: Address error on invalid address(>= 0x40000000 ?).
	z->timestamp = SH7095_mem_timestamp;

	SH7095_SetT(z, !tmp);

	tmp |= 0x80;

	MemWrite8(opexec_ea, tmp);
	SH7095_BusLock--;

	z->timestamp += 3;
 END_OP


 //
 // TST Rm,Rn
 //
 BEGIN_OP(TST_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	SH7095_SetT(z, !(z->R[n] & z->R[m]));

	FUSE_COND_BRANCH(OP_BF_ID, !SH7095_GetT(z));
	FUSE_COND_BRANCH(OP_BT_ID, SH7095_GetT(z));
 END_OP


 //
 // TST #imm,R0
 //
 BEGIN_OP(TST_IMM_REG0)
	const unsigned imm = (uint8_t)instr;

	WB_EX_CHECK(0)

	SH7095_SetT(z, !(z->R[0] & imm));

	FUSE_COND_BRANCH(OP_BT_ID, SH7095_GetT(z));
 END_OP


 //
 // TST.B #imm,@(R0,z->GBR)
 //
 BEGIN_OP(TST_B_IMM_IDXGBRINDIR)
	CONST_VAR(uint32_t, ea) = z->R[0] + z->GBR;
	uint32_t tmp;

	MemRead8(ea, tmp);
	z->timestamp++;
	SH7095_SetT(z, !(tmp & (uint8_t)instr));
	z->timestamp++;
 END_OP


 //
 // XOR Rm,Rn
 //
 BEGIN_OP(XOR_REG_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	CONST_VAR(unsigned, m) = instr_nyb1;

	WB_EX_CHECK(n)
	WB_EX_CHECK(m)

	z->R[n] = z->R[n] ^ z->R[m];
 END_OP


 //
 // XOR #imm,R0
 //
 BEGIN_OP(XOR_IMM_REG0)
	const unsigned imm = (uint8_t)instr;

	WB_EX_CHECK(0)

	z->R[0] = z->R[0] ^ imm;
 END_OP


 //
 // XOR.B #imm,@(R0,z->GBR)
 //
 BEGIN_OP(XOR_B_IMM_IDXGBRINDIR)
	RESUME_VAR(uint32_t, opexec_ea);
	opexec_ea = z->R[0] + z->GBR;
	uint32_t tmp;

	MemRead8(opexec_ea, tmp);
	z->timestamp++;
	tmp ^= (uint8_t)instr;
	MemWrite8(opexec_ea, tmp);
	z->timestamp++;
 END_OP


 //
 // ROTL Rn
 //
 BEGIN_OP(ROTL_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned rotbit = z->R[n] >> 31;

	WB_EX_CHECK(n)

	z->R[n] = (z->R[n] << 1) | rotbit;
	SH7095_SetT(z, rotbit);
 END_OP


 //
 // ROTR Rn
 //
 BEGIN_OP(ROTR_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned rotbit = z->R[n] & 1;

	WB_EX_CHECK(n)

	z->R[n] = (z->R[n] >> 1) | (rotbit << 31);
	SH7095_SetT(z, rotbit);
 END_OP


 //
 // ROTCL Rn
 //
 BEGIN_OP(ROTCL_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned rotbit = z->R[n] >> 31;

	WB_EX_CHECK(n)

	z->R[n] = (z->R[n] << 1) | SH7095_GetT(z);
	SH7095_SetT(z, rotbit);
 END_OP


 //
 // ROTCR Rn
 //
 BEGIN_OP(ROTCR_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned rotbit = z->R[n] & 1;

	WB_EX_CHECK(n)

	z->R[n] = (z->R[n] >> 1) | (SH7095_GetT(z) << 31);
	SH7095_SetT(z, rotbit);
 END_OP


 //
 // SHAR Rn
 //
 BEGIN_OP(SHAR_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned shbit = z->R[n] & 1;

	WB_EX_CHECK(n)

	z->R[n] = (int32_t)z->R[n] >> 1;
	SH7095_SetT(z, shbit);
 END_OP


 //
 // SHLL Rn
 //
 BEGIN_OP(SHLL_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned shbit = z->R[n] >> 31;

	WB_EX_CHECK(n)

	z->R[n] <<= 1;
	SH7095_SetT(z, shbit);
 END_OP


 //
 // SHLR Rn
 //
 BEGIN_OP(SHLR_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;
	const unsigned shbit = z->R[n] & 1;

	WB_EX_CHECK(n)

	z->R[n] >>= 1;
	SH7095_SetT(z, shbit);
 END_OP


 //
 // SHLL2 Rn
 //
 BEGIN_OP(SHLL2_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] <<= 2;
 END_OP


 //
 // SHLR2 Rn
 //
 BEGIN_OP(SHLR2_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] >>= 2;
 END_OP


 //
 // SHLL8 Rn
 //
 BEGIN_OP(SHLL8_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] <<= 8;
 END_OP


 //
 // SHLR8 Rn
 //
 BEGIN_OP(SHLR8_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] >>= 8;
 END_OP


 //
 // SHLL16 Rn
 //
 BEGIN_OP(SHLL16_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] <<= 16;
 END_OP


 //
 // SHLR16 Rn
 //
 BEGIN_OP(SHLR16_REG)
	CONST_VAR(unsigned, n) = instr_nyb2;

	WB_EX_CHECK(n)

	z->R[n] >>= 16;
 END_OP


 //
 // BF 
 //
 BEGIN_OP_FUDGEIF(BF)
	CondRelBranch(!SH7095_GetT(z), (uint32_t)(int8_t)instr << 1);
 END_OP


 //
 // BF/S 
 //
 BEGIN_OP_FUDGEIF(BF_S)
	CondRelDelayBranch(!SH7095_GetT(z), (uint32_t)(int8_t)instr << 1);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // BT 
 //
 BEGIN_OP_FUDGEIF(BT)
	CondRelBranch(SH7095_GetT(z), (uint32_t)(int8_t)instr << 1);
 END_OP


 //
 // BT/S 
 //
 BEGIN_OP_FUDGEIF(BT_S)
	CondRelDelayBranch(SH7095_GetT(z), (uint32_t)(int8_t)instr << 1);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // BRA 
 //
 BEGIN_OP_DLYIDIF(BRA)
	UCRelDelayBranch((uint32_t)sign_x_to_s32(12, instr) << 1);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // BRAF Rm
 //
 BEGIN_OP_DLYIDIF(BRAF_REG)
	UCRelDelayBranch(z->R[instr_nyb2]);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // BSR 
 //
 BEGIN_OP_DLYIDIF(BSR)
	z->PR = z->PC;

	UCRelDelayBranch((uint32_t)sign_x_to_s32(12, instr) << 1);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // BSRF Rm
 //
 BEGIN_OP_DLYIDIF(BSRF_REG)
	z->PR = z->PC;

	UCRelDelayBranch(z->R[instr_nyb2]);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // JMP @Rm
 //
 BEGIN_OP_DLYIDIF(JMP_REGINDIR)
	UCDelayBranch(z->R[instr_nyb2]);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // JSR @Rm
 //
 BEGIN_OP_DLYIDIF(JSR_REGINDIR)
	z->PR = z->PC;

	UCDelayBranch(z->R[instr_nyb2]);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // RTS 
 //
 BEGIN_OP_DLYIDIF(RTS)
	UCDelayBranch(z->PR);
	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // CLRT 
 //
 BEGIN_OP(CLRT)
	SH7095_SetT(z, false);
 END_OP


 //
 // CLRMAC 
 //
 BEGIN_OP(CLRMAC)
	z->MACH = 0;
	z->MACL = 0;
 END_OP


 //
 // LDC 
 //
 BEGIN_OP_DLYIDIF(LDC)
	{
	 CONST_VAR(unsigned, m) = instr_nyb2;
	 const unsigned cri = (instr >> 4) & 0x3;

	 z->CtrlRegs[cri] = z->R[m];
	 if(cri == 0)
	 {
	  z->SR &= 0x3F3;
	  SH7095_RecalcPendingIntPEX(z);
	 }
	}
	PART_OP_INTDIS
 END_OP

 //
 // LDC.L 
 //
 // Pipeline: page 233
 //
 BEGIN_OP_DLYIDIF(LDC_L)
	{
	 uint32_t ea;
	 ea = z->R[instr_nyb2];
	 z->R[instr_nyb2] += 4;

	 z->timestamp++;
	 MemRead32(ea, z->CtrlRegs[(instr >> 4) & 0x3]);
	 if(((instr >> 4) & 0x3) == 0)
	 {
	  z->SR &= 0x3F3;
	  SH7095_RecalcPendingIntPEX(z);
	 }

	 z->timestamp++;
	}
	PART_OP_INTDIS
 END_OP


 //
 // LDS 
 //
 BEGIN_OP_DLYIDIF(LDS)
	{
	 CONST_VAR(unsigned, m) = instr_nyb2;
	 const unsigned sri = (instr >> 4) & 0x3;

	 z->SysRegs[sri] = z->R[m];
	}
	PART_OP_INTDIS
 END_OP


 //
 // LDS.L 
 //
 BEGIN_OP_DLYIDIF(LDS_L)	// Pipeline same as ordinary load instruction
	{
	 uint32_t ea;

	 ea = z->R[instr_nyb2];
	 z->R[instr_nyb2] += 4;

	 MemRead32(ea, z->SysRegs[(instr >> 4) & 0x3]);
	}
	PART_OP_INTDIS
 END_OP


 //
 // NOP 
 //
 BEGIN_OP(NOP)
 END_OP


 //
 // RTE 
 //
 // Pipeline: page 241
 //
 BEGIN_OP(RTE)
	RESUME_VAR(uint32_t, opexec_new_PC);
	RESUME_VAR(uint32_t, opexec_new_SR);

	MemRead32(z->R[15], opexec_new_PC);
	z->R[15] += 4;

	MemRead32(z->R[15], opexec_new_SR)
	z->R[15] += 4;


	z->SR = opexec_new_SR & 0x3F3;
	SH7095_RecalcPendingIntPEX(z);

	z->timestamp++;

	DelayBranch(opexec_new_PC);

	FUSE_DELAY_SLOT_NOP();
 END_OP


 //
 // SETT 
 //
 BEGIN_OP(SETT)
	SH7095_SetT(z, true);
 END_OP


 //
 // SLEEP 
 //
 BEGIN_OP_DLYIDIF(SLEEP)
	//
	// z->Standby mode time yay?
	//
	if(MDFN_UNLIKELY(z->SBYCR & 0x80))
	{
	 for(unsigned ch = 0; ch < 2; ch++)
	 {
	  z->DMACH[ch].CHCR = 0x00;
	  z->DMACH[ch].CHCRM = 0x00;
	 }
	 z->DMAOR = 0x00;
	 SH7095_DMA_RecalcRunning(z);
	 SH7095_RecalcPendingIntPEX(z);
	 //
	 //
	 SH7095_FRT_Reset(z);
	 //
	 //
	 SH7095_WDT_StandbyReset(z);
	 //
	 //
	 SH7095_SCI_Reset(z);
	 //
	 //

	 z->timestamp++;
	 z->Pipe_ID = (z->Pipe_ID & 0x00FFFFFF) | (0x7E << 24);

	 z->Standby = true;

	 goto SkipPCInc;
	}

	if(MDFN_LIKELY(!z->EPending))
	{
	 z->timestamp += 3;
	 goto SkipPCInc;
	}
	else
	{
	 DoID(false);
	 FetchIF();
	}
 END_OP

 BEGIN_OP_DLYIDIF(PSEUDO_STANDBY)
	if(MDFN_LIKELY(z->Standby))
	{
	 z->timestamp += 7;
	 goto SkipPCInc;
	}
	else 
	{
	 SH7095_FRT_Reset(z);	// SH7095_Reset_TBD_ again(for the counter) because we didn't stop clocking it.

	 DoID(false);
	 FetchIF();
	}
 END_OP

 //
 // STC 
 //
 BEGIN_OP_DLYIDIF(STC)
	{
	 CONST_VAR(unsigned, n) = instr_nyb2;
	 const unsigned cri = (instr >> 4) & 0x3;

	 z->R[n] = z->CtrlRegs[cri];
	}
	PART_OP_INTDIS
 END_OP


 //
 // STC.L 
 //
 // pipeline: page 234
 BEGIN_OP_DLYIDIF(STC_L)
	{
	 CONST_VAR(unsigned, n) = instr_nyb2;
	 CONST_VAR(unsigned, cri) = (instr >> 4) & 0x3;
	 uint32_t ea;

	 z->R[n] -= 4;
	 ea = z->R[n];

	 MemWrite32(ea, z->CtrlRegs[cri]);
	 z->timestamp++;
	}
	PART_OP_INTDIS
 END_OP


 //
 // STS 
 //
 BEGIN_OP_DLYIDIF(STS)
	{
	 CONST_VAR(unsigned, n) = instr_nyb2;
	 const unsigned sri = (instr >> 4) & 0x3;

	 z->R[n] = z->SysRegs[sri];
	}
	PART_OP_INTDIS
 END_OP


 //
 // STS.L 
 //
 BEGIN_OP_DLYIDIF(STS_L)	// Pipeline same as ordinary store instruction
	{
	 CONST_VAR(unsigned, n) = instr_nyb2;
	 CONST_VAR(unsigned, sri) = (instr >> 4) & 0x3;
	 uint32_t ea;

	 z->R[n] -= 4;
	 ea = z->R[n];

	 MemWrite32(ea, z->SysRegs[sri]);
	}
	PART_OP_INTDIS
 END_OP


 //
 // TRAPA #imm
 //
 // Saved z->PC is the address of the instruction after the TRAPA instruction
 //
 BEGIN_OP_DLYIDIF(TRAPA)
	RESUME_VAR(uint32_t, opexec_new_PC);

	z->PC -= 2;
	Exception((uint8_t)instr, opexec_new_PC)
	Branch(opexec_new_PC);
 END_OP


 /*
 **
 **
 */
 //
 // Illegal Instruction
 //
 // Saved z->PC is the address of the illegal instruction.
 //
 BEGIN_OP_DLYIDIF(ILLEGAL)
	RESUME_VAR(uint32_t, opexec_new_PC);

	z->PC -= 4;
	Exception(SH7095_VECNUM_ILLINSTR, opexec_new_PC);
	Branch(opexec_new_PC);
 END_OP

 //
 // Illegal Slot Instruction
 //
 // Saved z->PC is the effective target address of the jump.
 //
 BEGIN_OP_DLYIDIF(SLOT_ILLEGAL)
	RESUME_VAR(uint32_t, opexec_new_PC);

	z->PC -= 2;
	Exception(SH7095_VECNUM_ILLSLOT, opexec_new_PC);
	Branch(opexec_new_PC);
 END_OP

 //
 // Pending exception(address error/interrupt)
 //
 BEGIN_OP_DLYIDIF(PSEUDO_EPENDING)
	RESUME_VAR(uint32_t, opexec_new_PC);
	RESUME_VAR(uint8_t, opexec_vecnum);

	opexec_new_PC = 0;

	//
	// Priority here(listed highest to lowest):
	//  External halt
	//   Power
	//    SH7095_Reset_TBD_
	//     Pseudo DMA burst(hacky abusey thing to stall the CPU, should be above everything but reset/power and ext halt otherwise kaboom!).
	//      CPU address error
	//       DMA address error
	//        NMI
	//         Interrupt (final else, may be called quasi-spuriously)
	//
	#define TPP(x) (z->Pipe_ID & (1U << ((x) + SH7095_EPENDING_PEXBITS_SHIFT)))
	//
	// Test against z->Pipe_ID, reset bits in z->EPending(if appropriate).
	//
	if(MDFN_UNLIKELY(TPP(SH7095_PEX_PSEUDO_EXTHALT)))
	{
	 z->Pipe_ID = (z->Pipe_ID & 0x00FFFFFF) | (0xFEU << 24);
	 goto SkipPCInc;
	}
	else if(MDFN_UNLIKELY(TPP(SH7095_PEX_POWERON) || TPP(SH7095_PEX_RESET)))
	{
	 z->EPending = 0;
	 opexec_vecnum = SH7095_VECNUM_POWERON;

	 if(TPP(SH7095_PEX_RESET))
	  opexec_vecnum = SH7095_VECNUM_RESET;

	 Exception_ResPow(opexec_vecnum, opexec_new_PC);
	}
	else if(MDFN_UNLIKELY(TPP(SH7095_PEX_PSEUDO_DMABURST)))
	{
	 z->Pipe_ID = (z->Pipe_ID & 0x00FFFFFF) | (0xFEU << 24);
	 goto SkipPCInc;
	}
	else if(MDFN_UNLIKELY(TPP(SH7095_PEX_CPUADDR)))
	{
	 z->PC -= 4;
	 SH7095_ClearPEX(z, SH7095_PEX_CPUADDR);
	 Exception(SH7095_VECNUM_CPUADDR, opexec_new_PC);
	 SH7095_ClearPEX(z, SH7095_PEX_CPUADDR);	// Infinite recursive address errors are not good(stack wraparound could clobber backup memory).
	}
	else if(MDFN_UNLIKELY(TPP(SH7095_PEX_DMAADDR)))
	{
	 z->PC -= 4;
	 SH7095_ClearPEX(z, SH7095_PEX_DMAADDR);
	 Exception(SH7095_VECNUM_DMAADDR, opexec_new_PC);
	}
	else if(TPP(SH7095_PEX_NMI))
	{
	 z->PC -= 4;
	 SH7095_ClearPEX(z, SH7095_PEX_NMI);
	 Exception(SH7095_VECNUM_NMI, opexec_new_PC);
	 //
	 //
	 //
	 z->SR |= 0xF << 4;
	 SH7095_RecalcPendingIntPEX(z);
	}
	else	// Int
	{
	 RESUME_VAR(uint8_t, opexec_ipr);

	 opexec_ipr = SH7095_GetPendingInt(z, NULL);

	 if(MDFN_LIKELY(opexec_ipr > ((z->SR >> 4) & 0xF)))
	 {
	  // Note: SH7095_GetPendingInt(z) may call z->ExIVecFetch(), which may call SH7095_SetIRL_TBD_ with a new value, so be
	  // careful to use the "old" value here.
	  SH7095_GetPendingInt(z, &opexec_vecnum);

	  z->PC -= 4;
	  Exception(opexec_vecnum, opexec_new_PC);
	  //
	  //
	  //
	  z->SR &= ~(0xF << 4);
	  z->SR |= opexec_ipr << 4;
	  SH7095_RecalcPendingIntPEX(z);
	 }
	 else
	 {
	  // 
	  // Can happen like so(note for future testing):
	  //
	  // (z->WDT interval timer IRQ pending here)
	  //
	  // WTCSR_R;
	  // asm volatile(
	  //	"ldc %0,z->SR\n\t"
	  //	"mov.w %2, @%1\n\t"
	  //	:
	  //	:"r"(0), "r"(0xFFFFFE88), "r"(0xA500 | 0x00)
	  // 	: "memory");
	  //
	  z->Pipe_ID = (uint16_t)z->Pipe_ID;
	  z->Pipe_ID |= InstrDecodeTab[z->Pipe_ID] << 24;
	  goto SPEPRecover;
	 }
	}
	#undef TPP
	//
	//
	//
	Branch(opexec_new_PC);
 END_OP

 BEGIN_OP_DLYIDIF(PSEUDO_DMABURST)
	if(MDFN_LIKELY(SH7095_DMA_InBurst(z) || z->ExtHalt))
	{
	 z->timestamp += 7;
	 goto SkipPCInc;
	}
	else
	{
	 SH7095_ClearPEX(z, SH7095_PEX_PSEUDO_DMABURST);
	 SH7095_ClearPEX(z, SH7095_PEX_PSEUDO_EXTHALT);

	 //
	 // Recover z->Pipe_ID opcode field; only use z->Pipe_ID for z, not z->EPending, otherwise
	 // we may accidentally allow an interrupt to occur immediately after an interrupt-disabled instruction.
	 //
	 z->Pipe_ID &= 0x00FFFFFF;
	 z->Pipe_ID &= ~(1U << (SH7095_PEX_PSEUDO_DMABURST + SH7095_EPENDING_PEXBITS_SHIFT));
	 z->Pipe_ID &= ~(1U << (SH7095_PEX_PSEUDO_EXTHALT + SH7095_EPENDING_PEXBITS_SHIFT));
	 z->Pipe_ID |= InstrDecodeTab[(uint16_t)z->Pipe_ID] << 24;
	 if(z->Pipe_ID & (0xFF << SH7095_EPENDING_PEXBITS_SHIFT))
	  z->Pipe_ID |= SH7095_EPENDING_OP_OR;

	 goto SkipPCInc;
	}
 END_OP
}

z->PC += 2;
SkipPCInc:;

#ifdef SH7095_NEED_RESUME_TABLE_INIT
MDFN_STATIC_ASSERT(__COUNTER__ == (5000 + 393), "Unexpected __COUNTER__");
#endif

#undef PART_OP_NORMIDIF
#undef BEGIN_OP
#undef BEGIN_OP_FUDGEIF
#undef BEGIN_OP_DLYIDIF
#undef PART_OP_INTDIS
#undef END_OP
#undef FUSE_COND_BRANCH
#undef FUSE_LOAD_CMPTST_BRANCH
#undef FUSE_DELAY_SLOT_NOP
#undef WB_EX_CHECK
#undef WB_WRITE
#undef WB_READ8
#undef WB_READ16
#undef WB_READ32
#undef WB_READ16_I
#undef WB_READ32_I

