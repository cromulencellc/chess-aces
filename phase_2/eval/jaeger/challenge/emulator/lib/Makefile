OBJS=\
	error.o \
	rc.o \
	system.o \
	bus/bus.o \
	devices/bus_device.o \
	devices/debug_device.o \
	devices/dummy_device.o \
	devices/file_storage_device.o \
	devices/serial_tcp_device.o \
	devices/test_device.o \
	r5/decoder.o \
	r5/instruction.o \
	vm/csr.o \
	vm/hardware_pages.o \
	vm/interrupt_timer.o \
	vm/vm.o \
	vm/vm_mem.o \
	vm/ops/vm_add.o \
	vm/ops/vm_addi.o \
	vm/ops/vm_and.o \
	vm/ops/vm_andi.o \
	vm/ops/vm_auipc.o \
	vm/ops/vm_beq.o \
	vm/ops/vm_bge.o \
	vm/ops/vm_bgeu.o \
	vm/ops/vm_blt.o \
	vm/ops/vm_bltu.o \
	vm/ops/vm_bne.o \
	vm/ops/vm_div.o \
	vm/ops/vm_divu.o \
	vm/ops/vm_ebreak.o \
	vm/ops/vm_jal.o \
	vm/ops/vm_jalr.o \
	vm/ops/vm_lb.o \
	vm/ops/vm_lbu.o \
	vm/ops/vm_lh.o \
	vm/ops/vm_lhu.o \
	vm/ops/vm_lui.o \
	vm/ops/vm_lw.o \
	vm/ops/vm_mret.o \
	vm/ops/vm_mul.o \
	vm/ops/vm_mulh.o \
	vm/ops/vm_mulhsu.o \
	vm/ops/vm_mulhu.o \
	vm/ops/vm_or.o \
	vm/ops/vm_ori.o \
	vm/ops/vm_rem.o \
	vm/ops/vm_remu.o \
	vm/ops/vm_sb.o \
	vm/ops/vm_sh.o \
	vm/ops/vm_sw.o \
	vm/ops/vm_sll.o \
	vm/ops/vm_slli.o \
	vm/ops/vm_slt.o \
	vm/ops/vm_slti.o \
	vm/ops/vm_sltu.o \
	vm/ops/vm_sltiu.o \
	vm/ops/vm_sra.o \
	vm/ops/vm_srai.o \
	vm/ops/vm_srl.o \
	vm/ops/vm_srli.o \
	vm/ops/vm_sub.o \
	vm/ops/vm_wfi.o \
	vm/ops/vm_xor.o \
	vm/ops/vm_xori.o
	

TARGET = ./libemulator.a

CFLAGS=-Wall -g -Werror -O2 -DLOG_LEVEL_INFO
LIB=
INCLUDE=-I./

$(TARGET) : $(OBJS)
	ar r libemulator.a $(OBJS)
	pmccabe -Tv *.c r5/*.c vm/*.c vm/ops/*.c bus/*.c devices/*.c

run: $(TARGET)
	$(TARGET)

%.o : %.c
	$(CC) -fPIC -c -o $@ $< $(INCLUDE) $(CFLAGS)

clean :
	rm -f *.o
	rm -rf bus/*.o
	rm -rf devices/*.o
	rm -rf r5/*.o
	rm -rf vm/*.o
	rm -rf vm/ops/*.o
	rm -rf *dSYM
	rm -f libemulator.a
