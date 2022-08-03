/* Generated automatically by the program `genattr-common'
   from the machine description file `md'.  */

#ifndef GCC_INSN_ATTR_COMMON_H
#define GCC_INSN_ATTR_COMMON_H

enum attr_got {GOT_UNSET, GOT_XGOT_HIGH, GOT_LOAD};
enum attr_move_type {MOVE_TYPE_UNKNOWN, MOVE_TYPE_LOAD, MOVE_TYPE_FPLOAD, MOVE_TYPE_STORE, MOVE_TYPE_FPSTORE, MOVE_TYPE_MTC, MOVE_TYPE_MFC, MOVE_TYPE_MOVE, MOVE_TYPE_FMOVE, MOVE_TYPE_CONST, MOVE_TYPE_LOGICAL, MOVE_TYPE_ARITH, MOVE_TYPE_ANDI, MOVE_TYPE_SHIFT_SHIFT};
enum attr_mode {MODE_UNKNOWN, MODE_NONE, MODE_QI, MODE_HI, MODE_SI, MODE_DI, MODE_TI, MODE_HF, MODE_SF, MODE_DF, MODE_TF};
enum attr_dword_mode {DWORD_MODE_NO, DWORD_MODE_YES};
enum attr_type {TYPE_UNKNOWN, TYPE_BRANCH, TYPE_JUMP, TYPE_CALL, TYPE_LOAD, TYPE_FPLOAD, TYPE_STORE, TYPE_FPSTORE, TYPE_MTC, TYPE_MFC, TYPE_CONST, TYPE_ARITH, TYPE_LOGICAL, TYPE_SHIFT, TYPE_SLT, TYPE_IMUL, TYPE_IDIV, TYPE_MOVE, TYPE_FMOVE, TYPE_FADD, TYPE_FMUL, TYPE_FMADD, TYPE_FDIV, TYPE_FCMP, TYPE_FCVT, TYPE_FSQRT, TYPE_MULTI, TYPE_AUIPC, TYPE_SFB_ALU, TYPE_NOP, TYPE_GHOST, TYPE_FCVT_I, TYPE_FSGNJ, TYPE_EBREAK, TYPE_VSET, TYPE_VADD, TYPE_VCMP, TYPE_VSHIFT, TYPE_VLOGICAL, TYPE_VMOVE, TYPE_VRED_BIT, TYPE_VMUL, TYPE_VMADD, TYPE_VRED_SUM, TYPE_VRED_MIN, TYPE_VRED_MAX, TYPE_VDIV, TYPE_VREM, TYPE_VLOAD, TYPE_VSTORE, TYPE_VFADD, TYPE_VFSGNJ, TYPE_VFMUL, TYPE_VFWMUL, TYPE_VFMADD, TYPE_VFWMADD, TYPE_VFRED, TYPE_VFMOVE, TYPE_VFCMP, TYPE_VFCVT, TYPE_VFDIV, TYPE_VFSQRT};
enum attr_cannot_copy {CANNOT_COPY_NO, CANNOT_COPY_YES};
enum attr_tune {TUNE_GENERIC, TUNE_SIFIVE_7, TUNE_C906V};
enum attr_emode {EMODE_UNKNOWN, EMODE_QI, EMODE_HI, EMODE_SI, EMODE_DI};
#define INSN_SCHEDULING
#define DELAY_SLOTS 0

#endif /* GCC_INSN_ATTR_COMMON_H */
