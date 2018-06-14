#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = true;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  if(decoding.is_operand_size_16)
    decoding.jmp_eip &= 0xffff;
  decoding.is_jmp = true;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_push(eip, width);
  decoding.is_jmp = true;

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(call_rm) {
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_push(eip, width);
  decoding.jmp_eip = id_dest->val;
  if(decoding.is_operand_size_16)
    decoding.jmp_eip &= 0xffff;
  decoding.is_jmp = true;

  print_asm("call *%s", id_dest->str);
}


make_EHelper(ret) {
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_pop(&decoding.jmp_eip, width);
  if(decoding.is_operand_size_16)
    decoding.jmp_eip &= 0xffff;
  decoding.is_jmp = true;

  print_asm("ret");
}