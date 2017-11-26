#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_push(&id_dest->val, id_dest->width);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;

  rtl_lr(&t0, R_ESP, width);
  rtl_push(&cpu.eax, width);
  rtl_push(&cpu.ecx, width);
  rtl_push(&cpu.edx, width);
  rtl_push(&cpu.ebx, width);
  rtl_push(&t0, width);
  rtl_push(&cpu.ebp, width);
  rtl_push(&cpu.esi, width);
  rtl_push(&cpu.edi, width);

  print_asm("pusha");
}

make_EHelper(popa) {
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;
  
  rtl_pop(&cpu.edi, width);
  rtl_pop(&cpu.esi, width);
  rtl_pop(&cpu.ebp, width);
  rtl_pop(&t0, width);
  rtl_pop(&cpu.ebx, width);
  rtl_pop(&cpu.edx, width);
  rtl_pop(&cpu.ecx, width);
  rtl_pop(&cpu.eax, width);

  print_asm("popa");
}

make_EHelper(leave) {
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;
  
  rtl_lr(&t0, R_EBP, width);
  rtl_sr(R_ESP, width, &t0);
  rtl_pop(&t0, width);
  rtl_sr(R_EBP, width, &t0);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_EAX, 2);
    rtl_sext(&t0, &t0, 2);
    rtl_sr(R_EAX, 4, &t0);
  }
  else {
    rtl_lr(&t0, R_EAX, 4);
    rtl_msb(&t0, &t0, 4);
    if(0== t0) {
      rtl_sr(R_EDX, 4, &tzero);
    } else {
      rtl_xori(&t0, &tzero, 0xffffffff);
      rtl_sr(R_EDX, 4, &t0);
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  int width;
  width = decoding.is_operand_size_16 ? 2 : 4;

  rtl_lr(&t0, R_EAX, width>>1);
  rtl_sext(&t0, &t0, width>>1);
  rtl_sr(R_EAX, width, &t0);

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

make_EHelper(xchg) {
  rtl_mv(&t0, &id_dest->val);
  operand_write(id_dest, &id_src->val);
  operand_write(id_src, &t0);
}
