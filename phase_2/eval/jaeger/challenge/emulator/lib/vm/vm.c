#include "vm.h"

#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "r5/csr.h"
#include "r5/decoder.h"
#include "result.h"
#include "vm_mem.h"

#include "ops/vm_ops.h"

struct vm_device_address VM_DEVICE_ADDRESSES[VM_MAX_DEVICES] = {
    {DEVICE_0_ADDRESS, DEVICE_0_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_1_ADDRESS, DEVICE_1_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_2_ADDRESS, DEVICE_2_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_3_ADDRESS, DEVICE_3_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_4_ADDRESS, DEVICE_4_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_5_ADDRESS, DEVICE_5_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_6_ADDRESS, DEVICE_6_ADDRESS + DEVICE_ADDRESS_OFFSET},
    {DEVICE_7_ADDRESS, DEVICE_7_ADDRESS + DEVICE_ADDRESS_OFFSET}
};


struct vm * vm_create() {
    struct vm * vm = malloc(sizeof(struct vm));

    if (vm == NULL) {
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < R5_NUMBER_GPRS; i++) {
        vm->regs[i] = 0;
    }

    vm->pc = 0;
    
    vm->hardware_pages = hardware_pages_create();
    if (vm->hardware_pages == NULL) {
        free(vm);
        return NULL;
    }

    vm->num_devices = 0;

    for (i = 0; i < VM_MAX_DEVICES; i++) {
        vm->devices[i] = NULL;
    }

    memset(&vm->mstatus, 0, sizeof(struct r5_csr_mstatus));
    memset(&vm->mtvec, 0, sizeof(struct r5_csr_mtvec));
    memset(&vm->mip, 0, sizeof(struct r5_csr_mip));
    memset(&vm->mie, 0, sizeof(struct r5_csr_mie));
    vm->mtime = 0;
    vm->mtimecmp = 0;
    vm->mepc = 0;
    vm->mcause = 0;
    vm->mtval = 0;
    vm->level = VM_LEVEL_MACHINE;

    vm->mtimecmp_written = 0;
    vm->tracing = 0;
    vm->wfi = 0;

    return vm;
}

struct vm * vm_from_elf(const char * path) {
    FILE * fh = fopen(path, "rb");
    if (fh == NULL) {
        return NULL;
    }

    fseek(fh, 0, SEEK_END);
    size_t filesize = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    uint8_t * elf_bytes = malloc(filesize);
    if (elf_bytes == NULL) {
        return NULL;
    }

    if (fread(elf_bytes, 1, filesize, fh) != filesize) {
        free(elf_bytes);
        fclose(fh);
        return NULL;
    }

    fclose(fh);

    struct vm * vm = vm_create();
    if (vm == NULL) {
        free(elf_bytes);
        return NULL;
    }

    if (vm_load_elf(vm, elf_bytes) != OK) {
        vm_delete(vm);
        free(elf_bytes);
        return NULL;
    }

    free(elf_bytes);

    return vm;
}


void vm_delete(struct vm * vm) {
    hardware_pages_delete(vm->hardware_pages);

    free(vm);
}


enum result vm_load_elf(struct vm * vm, const uint8_t * elf) {
    const Elf32_Ehdr * ehdr = (const Elf32_Ehdr *) elf;

    if (ehdr->e_machine != EM_RISCV) {
        LOG_WARN("ehdr->e_machine != EM_RISCV");
        return ERROR_ELF_RISCV;
    }
    else if (ehdr->e_type != ET_EXEC) {
        LOG_WARN("ehdr->e_type != ET_EXEC");
        return ERROR_ELF_EXEC;
    }
    else if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
        LOG_WARN("ehdr->e_ident[EI_CLASS] != ELFCLASS32");
        return ERROR_ELF_CLASS32;
    }
    else if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        LOG_WARN("ehdr->e_ident[EI_DATA] != ELFDATA2LSB");
        return ERROR_ELF_LITTLE_ENDIAN;
    }

    unsigned int i;
    for (i = 0; i < ehdr->e_phnum; i++) {
        const Elf32_Phdr * phdr = (const Elf32_Phdr *)
            &(elf[ehdr->e_phoff + (i * ehdr->e_phentsize)]);
        /* Create the necessary hardware pages */
        uint32_t address;
        for (
            address = phdr->p_vaddr;
            address < phdr->p_vaddr + phdr->p_memsz;
            address += R5_HARDWARE_PAGE_SIZE
        ) {
            if (hardware_pages_create_page(vm->hardware_pages, address)) {
                return ERROR_ELF_CREATE_PAGE_FAILED;
            }
        }

        /* Copy over the data */
        uint32_t offset;
        for (offset = 0; offset < phdr->p_filesz; offset++) {
            uint8_t byte = elf[phdr->p_offset + offset];
            enum result result;
            if ((result = vm_set_u8(vm, phdr->p_vaddr + offset, byte)) != OK) {
                return result;
            }
        }
    }

    /* Gp must be set to value of __global_pointer$ symbol */
    uint32_t gp = 0;
    int gp_found = 0;

    for (i = 0; i < ehdr->e_shnum; i++) {
        const Elf32_Shdr * shdr = (const Elf32_Shdr *)
            &(elf[ehdr->e_shoff + (i * ehdr->e_shentsize)]);

        if ((shdr->sh_type != SHT_DYNSYM) && (shdr->sh_type != SHT_SYMTAB)) {
            continue;
        }

        const Elf32_Shdr * strtab = (const Elf32_Shdr *)
            &(elf[ehdr->e_shoff + (shdr->sh_link * ehdr->e_shentsize)]);

        unsigned int sym_i;
        for (sym_i = 0; sym_i < shdr->sh_size / shdr->sh_entsize; sym_i++) {
            const Elf32_Sym * sym = (const Elf32_Sym *)
                &(elf[shdr->sh_offset + (sym_i * shdr->sh_entsize)]);
            
            const char * sym_name =
                (const char *) &(elf[strtab->sh_offset + sym->st_name]);

            if (strcmp("__global_pointer$", sym_name) == 0) {
                gp = sym->st_value;
                gp_found = 1;
                break;
            }
        }

        if (gp_found) {
            break;
        }
    }

    if (!gp_found) {
        /* This is probably ok. */
        // return ERROR_ELF_GP_NOT_FOUND;
    }

    vm->regs[R5_GP] = gp;

    return OK;
}


int vm_add_device(struct vm * vm, struct device * device) {
    if (vm->num_devices == VM_MAX_DEVICES) {
        return ERROR_TOO_MANY_DEVICES;
    }
    vm->devices[vm->num_devices++] = device;
    return OK;
}


int vm_run_devices(struct vm * vm) {
    unsigned int i;

    for (i = 0; i < VM_MAX_DEVICES; i++) {
        if (vm->devices[i] == NULL) {
            continue;
        }

        vm->devices[i]->run(vm->devices[i], vm);
    }

    return 0;
}


int vm_get_register(struct vm * vm, enum r5_register reg, uint32_t * value) {
    if (reg > R5_REGISTER_MAX) {
        return ERROR_INVALID_REGISTER;
    }

    if (vm->regs == R5_ZERO) {
        *value = 0;
    }
    else {
        *value = vm->regs[reg];
    }

    return 0;
}


int vm_set_register(struct vm * vm, enum r5_register reg, uint32_t value) {
    if (reg > R5_REGISTER_MAX) {
        return ERROR_INVALID_REGISTER;
    }
    else if (reg != R5_ZERO) {
        vm->regs[reg] = value;
    }

    return 0;
}

/* Short, unchecked version of vm_get_register */
uint32_t vm_getr(struct vm * vm, enum r5_register reg) {
    if (reg == R5_ZERO) {
        return 0;
    }
    else {
        return vm->regs[reg];
    }
}

/* Short, unchecked version of vm_set_register */
void vm_setr(struct vm * vm, enum r5_register reg, uint32_t value) {
    vm->regs[reg] = value;
}


int vm_get_pc(struct vm * vm, uint32_t * value) {
    *value = vm->pc;
    return OK;
}


int vm_set_pc(struct vm * vm, uint32_t value) {
    vm->pc = value;
    return OK;
}


enum result vm_exception(struct vm * vm, uint32_t mcause, uint32_t mtval) {
    /* Is this an interrupt */
    if (mcause & R5_CSR_MCAUSE_INTERRUPT) {
        /* If interrupts are not enabled, return */
        if (! vm->mstatus.mie) {
            return OK;
        }
    }

    if (vm->tracing) {
        printf(
            "vm_exception mcause=0x%08x mtval=0x%08x\n",
            mcause,
            mtval
        );
    }

    vm->mcause = mcause;
    vm->mtval = mtval;
    vm->mepc = vm->pc;
    
    switch (vm->level) {
    case VM_LEVEL_USER: vm->mstatus.mpp = R5_CSR_MSTATUS_MPP_U; break;
    case VM_LEVEL_SUPERVISOR: vm->mstatus.mpp = R5_CSR_MSTATUS_MPP_S; break;
    case VM_LEVEL_RESERVED: return -1;
    case VM_LEVEL_MACHINE: vm->mstatus.mpp = R5_CSR_MSTATUS_MPP_M; break;
    }
    
    /* Set MPIE to MIE, and then set MIE to disable interrupts */
    vm->mstatus.mpie = vm->mstatus.mie;
    vm->mstatus.mie = 0;

    switch (vm->mtvec.mode) {
    case R5_MTVEC_DIRECT:
        vm->pc = vm->mtvec.base;
        break;
    case R5_MTVEC_VECTORED: {
        vm->pc = vm->mtvec.base;
        vm->pc += (mcause & 0x7fffffff) * 4;
        break;
    }
    }

    /* We are in an interrupt, so we're no longer waiting for interrupt */
    vm->wfi = 0;

    return OK;
}


enum result vm_timer_tick(struct vm * vm) {
    vm->mtime++;

    if (vm->tracing) {
        printf(
            "vm_timer_tick mtime=%016lx mtimecmp=%016lx mtimecmp_written=%u mstatus.mie=%u mie.mtie=%u\n",
            vm->mtime,
            vm->mtimecmp,
            vm->mtimecmp_written,
            vm->mstatus.mie,
            vm->mie.mtie
        );
    }

            /* We have written to mtimecmp since the last timer interrupt */
    if (    (vm->mtimecmp_written)
            /* We have passed the threshold for the timer interrupt */
         && (vm->mtime >= vm->mtimecmp)
            /* Machine interrupts are enabled */
         && (vm->mstatus.mie)
            /* The machine timer interrupt is enabled */
         && (vm->mie.mtie)) {
        
        if (vm->tracing) {
            printf("Executing timer interrupt\n");
        }
        return vm_exception(vm, R5_CSR_MCAUSE_MACHINE_TIMER_INTERRUPT, 0);
    }

    return OK;
}


int vm_step(struct vm * vm) {
    uint32_t op_word;
    struct r5_instruction ins;

    if (vm->wfi) {
        if (vm->tracing) {
            printf("wfi\n");
        }
        return OK_WAITING_FOR_INTERRUPT;
    }

    if (vm_get_u32(vm, vm->pc, &op_word)) {
        return vm_exception(vm, R5_CSR_MCAUSE_LOAD_PAGE_FAULT, vm->pc);
    }

    if (r5_decode(&ins, op_word)) {
        if (vm->tracing) {
            printf("INVALID INSTRUCTION @ 0x%08x\n", vm->pc);
        }
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    if (vm->tracing) {
        char buf[64];
        if (r5_instruction_snprintf(buf, 64, &ins) < 0) {
            return ERROR_R5_INSTRUCTION_SNPRINTF;
        }

        printf("0x%08x (%08x): %s\n", vm->pc, op_word, buf);

        printf(
            "sp=0x%08x ra=0x%08x\n",
            vm->regs[R5_SP], vm->regs[R5_RA]
        );
        printf(
            "a0=0x%08x a1=0x%08x a2=0x%08x a3=0x%08x\n",
            vm->regs[R5_A0], vm->regs[R5_A1], vm->regs[R5_A2], vm->regs[R5_A3]
        );
        printf(
            "a4=0x%08x a5=0x%08x a6=0x%08x a7=0x%08x\n",
            vm->regs[R5_A4], vm->regs[R5_A5], vm->regs[R5_A6], vm->regs[R5_A7]
        );
    }

    switch (ins.op) {
    case R5_ADD: return vm_add(vm, &ins);
    case R5_ADDI: return vm_addi(vm, &ins);
    case R5_AND: return vm_and(vm, &ins);
    case R5_ANDI: return vm_andi(vm, &ins);
    case R5_AUIPC: return vm_auipc(vm, &ins);
    case R5_BEQ: return vm_beq(vm, &ins);
    case R5_BGE: return vm_bge(vm, &ins);
    case R5_BGEU: return vm_bgeu(vm, &ins);
    case R5_BLT: return vm_blt(vm, &ins);
    case R5_BLTU: return vm_bltu(vm, &ins);
    case R5_BNE: return vm_bne(vm, &ins);
    case R5_CSRRC: return vm_csrrc(vm, &ins);
    case R5_CSRRCI: return vm_csrrci(vm, &ins);
    case R5_CSRRS: return vm_csrrs(vm, &ins);
    case R5_CSRRSI: return vm_csrrsi(vm, &ins);
    case R5_CSRRW: return vm_csrrw(vm, &ins);
    case R5_CSRRWI: return vm_csrrwi(vm, &ins);
    case R5_DIV: return vm_div(vm, &ins);
    case R5_DIVU: return vm_divu(vm, &ins);
    case R5_EBREAK: return vm_ebreak(vm, &ins);
    case R5_JAL: return vm_jal(vm, &ins);
    case R5_JALR: return vm_jalr(vm, &ins);
    case R5_LB: return vm_lb(vm, &ins);
    case R5_LBU: return vm_lbu(vm, &ins);
    case R5_LH: return vm_lh(vm, &ins);
    case R5_LHU: return vm_lhu(vm, &ins);
    case R5_LUI: return vm_lui(vm, &ins);
    case R5_LW: return vm_lw(vm, &ins);
    case R5_MRET: return vm_mret(vm, &ins);
    case R5_MUL: return vm_mul(vm, &ins);
    case R5_MULH: return vm_mulh(vm, &ins);
    case R5_MULHU: return vm_mulhu(vm, &ins);
    case R5_MULHSU: return vm_mulhsu(vm, &ins);
    case R5_OR: return vm_or(vm, &ins);
    case R5_ORI: return vm_ori(vm, &ins);
    case R5_REM: return vm_rem(vm, &ins);
    case R5_REMU: return vm_remu(vm, &ins);
    case R5_SB: return vm_sb(vm, &ins);
    case R5_SH: return vm_sh(vm, &ins);
    case R5_SW: return vm_sw(vm, &ins);
    case R5_SLL: return vm_sll(vm, &ins);
    case R5_SLLI: return vm_slli(vm, &ins);
    case R5_SLT: return vm_slt(vm, &ins);
    case R5_SLTI: return vm_slti(vm, &ins);
    case R5_SLTU: return vm_sltu(vm, &ins);
    case R5_SLTIU: return vm_sltiu(vm, &ins);
    case R5_SRA: return vm_sra(vm, &ins);
    case R5_SRAI: return vm_srai(vm, &ins);
    case R5_SRL: return vm_srl(vm, &ins);
    case R5_SRLI: return vm_srli(vm, &ins);
    case R5_SUB: return vm_sub(vm, &ins);
    case R5_WFI: return vm_wfi(vm, &ins);
    case R5_XOR: return vm_xor(vm, &ins);
    case R5_XORI: return vm_xori(vm, &ins);
    case R5_ECALL:
    case R5_FENCE:
    case R5_FENCEI:
    case R5_RDCYCLE:
    case R5_RDCYCLEH:
    case R5_RDTIME:
    case R5_RDTIMEH:
    case R5_RDINSTRET:
    case R5_RDINSTRETH:
    case R5_SRET:
    case R5_URET:
    case R5_INVALID_OP:
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    vm->pc += 4;

    return OK;
}