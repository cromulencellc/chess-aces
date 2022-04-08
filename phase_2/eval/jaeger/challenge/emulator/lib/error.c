#include "result.h"

#include <stdlib.h>

struct result_to_string {
    enum result result;
    const char * string;
};

struct result_to_string result_strings[] = {
    {OK, "Ok"},
    {OK_WAITING_FOR_INTERRUPT, "Waiting for Interrupt"},
    {ERROR_INVALID_INSTRUCTION, "Invalid instruction"},
    {ERROR_OOM, "Out of memory"},
    {ERROR_NONALIGNED_PAGE, "Non-aligned page"},
    {ERROR_PAGE_EXISTS, "Page exists"},
    {ERROR_PAGE_DOES_NOT_EXIST, "Page does not exist"},
    {ERROR_INVALID_REGISTER, "Invalid register"},
    {ERROR_TOO_MANY_DEVICES, "Too many devices"},
    {ERROR_UNSUPPORTED_CSR, "Unsupported CSR"},
    {ERROR_ELF_RISCV, "Elf not RISC-V"},
    {ERROR_ELF_EXEC, "Elf type not EXEC"},
    {ERROR_ELF_CLASS32, "Elf not 32-bit"},
    {ERROR_ELF_LITTLE_ENDIAN, "Elf not little-endian"},
    {ERROR_ELF_CREATE_PAGE_FAILED, "Failed to create page while loading elf"},
    {ERROR_ELF_GP_NOT_FOUND, "Failed to find symbol for GP register"},
    {ERROR_R5_INSTRUCTION_SNPRINTF, "Failed to snprintf risc-v instruction"},
    {ERROR_INVALID_ADDRESS, "Invalid address"}
};

const char * result_string(enum result result) {
    unsigned int i;

    for(
        i = 0;
        i < sizeof(result_strings) / sizeof(struct result_to_string);
        i++
    ) {
        if (result_strings[i].result == result) {
            return result_strings[i].string;
        }
    }

    return NULL;
}