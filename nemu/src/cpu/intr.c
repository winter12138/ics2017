#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
	GateDesc desc;
	uint32_t offset;

	rtl_push(&cpu.eflags, 4);
	rtl_push(&cpu.cs, 4);
	rtl_push(&ret_addr, 4);

	t0 = cpu.idtr.base;
	t0 += sizeof(GateDesc)*NO;
	rtl_lm(&desc.val, &t0, 4);
	t0 += 4;
	rtl_lm(&desc.val+1, &t0, 4);
	offset = desc.offset_15_0 | desc.offset_31_16<<16;
	if(desc.present) {
		decoding.jmp_eip = offset;
		decoding.is_jmp = true;
	}
}

void dev_raise_intr() {
}
