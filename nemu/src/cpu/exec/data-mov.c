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
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  TODO();

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
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_EAX, 1);
    rtl_sext(&t0, &t0, 1);
    rtl_sr(R_EAX, 2, &t0);
  }
  else {
    rtl_lr(&t0, R_EAX, 2);
    rtl_sext(&t0, &t0, 2);
    rtl_sr(R_EAX, 4, &t0);
  }

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
