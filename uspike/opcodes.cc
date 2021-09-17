/*
  Copyright (c) 2021 Peter Hsu.  All Rights Reserved.  See LICENCE file for details.
*/
#include <stdint.h>
#include <stdio.h>

#include "options.h"
#include "uspike.h"
#include "mmu.h"
#include "cpu.h"

Insn_t decoder(int b, long pc)
{
  Insn_t i(Op_UNKNOWN);	       // recall all registers set to NOREG by default
#define x( lo, len) ((b >> lo) & ((1 << len)-1))
#define xs(lo, len) (b << (32-lo-len) >> (32-len))
  
#include "decoder.h"
  
 opcode_found:
  return i;
}

void redecode(long pc)
{
  if (code.valid(pc))
    code.set(pc, decoder(code.image(pc), pc));
}

#define LABEL_WIDTH  16
#define OFFSET_WIDTH  8
int slabelpc(char* buf, long pc)
{
  long offset;
  const char* label = elf_find_pc(pc, &offset);
  if (label)
    return sprintf(buf, "%*.*s+%*ld %8lx: ", LABEL_WIDTH, LABEL_WIDTH, label, -(OFFSET_WIDTH-1), offset, pc);
  else
    return sprintf(buf, "%*s %8lx: ", LABEL_WIDTH+OFFSET_WIDTH, "<invalid pc>", pc);
}

void labelpc(long pc, FILE* f)
{
  char buffer[1024];
  slabelpc(buffer, pc);
  fprintf(f, "%s", buffer);
}

int sdisasm(char* buf, long pc)
{
  Insn_t i = code.at(pc);
  if (i.opcode() == Op_ZERO)
    i = decoder(code.image(pc), pc);
  uint32_t b = code.image(pc);
  int n = 0;
  if (i.compressed())
    n += sprintf(buf+n, "    %04x  ", b&0xFFFF);
  else
    n += sprintf(buf+n, "%08x  ",     b);
  n += sprintf(buf+n, "%-23s", op_name[i.opcode()]);
  char sep = ' ';
  if (i.rd()  != NOREG) { n += sprintf(buf+n, "%c%s", sep, reg_name[i.rd() ]); sep=','; }
  if (i.rs1() != NOREG) { n += sprintf(buf+n, "%c%s", sep, reg_name[i.rs1()]); sep=','; }
  if (i.longimmed())    { n += sprintf(buf+n, "%c%ld", sep, i.immed()); }
  else {
    if (i.rs2() != NOREG) { n += sprintf(buf+n, "%c%s", sep, reg_name[i.rs2()]); sep=','; }
    if (i.rs3() != NOREG) { n += sprintf(buf+n, "%c%s", sep, reg_name[i.rs3()]); sep=','; }
    n += sprintf(buf+n, "%c%ld", sep, i.immed());
  }
  return n;
}

void disasm(long pc, const char* end, FILE* f)
{
  char buffer[1024];
  sdisasm(buffer, pc);
  fprintf(f, "%s%s", buffer, end);
}

void insnSpace_t::init(long lo, long hi)
{
  _base=lo;
  _limit=hi;
  int n = (hi-lo)/2;
  predecoded=new Insn_t[n];
  memset(predecoded, 0, n*sizeof(Insn_t));
  // Predecode instruction code segment
  long pc = lo;
  while (pc < hi) {
    Insn_t i = code.set(pc, decoder(code.image(pc), pc));
    pc += i.compressed() ? 2 : 4;
  }
  substitute_cas(lo, hi);
}

#include "constants.h"