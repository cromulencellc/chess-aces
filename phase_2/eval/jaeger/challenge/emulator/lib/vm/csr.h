#ifndef vm_csr_HEADER
#define vm_csr_HEADER

#include "r5/instruction.h"

struct vm;

/** Handler for csrrc instruction */
int vm_csrrc(struct vm * vm, const struct r5_instruction * ins);

/** Handler for csrrs instruction */
int vm_csrrs(struct vm * vm, const struct r5_instruction * ins);

/** Handler for csrrw instruction */
int vm_csrrw(struct vm * vm, const struct r5_instruction * ins);

/** Handler for csrrci instruction */
int vm_csrrci(struct vm * vm, const struct r5_instruction * ins);

/** Handler for csrrsi instruction */
int vm_csrrsi(struct vm * vm, const struct r5_instruction * ins);

/** Handler for csrrwi instruction */
int vm_csrrwi(struct vm * vm, const struct r5_instruction * ins);



enum r5_mstatus_mpp {
    R5_MSTATUS_MPP_U,
    R5_MSTATUS_MPP_S,
    R5_MSTATUS_MPP_RESERVED,
    R5_MSTATUS_MPP_M
};

enum r5_mstatus_fs {
    R5_MSTATUS_FS_OFF,
    R5_MSTATUS_FS_INITIAL,
    R5_MSTATUS_FS_CLEAN,
    R5_MSTATUS_FS_DIRTY
};

enum r5_mstatus_xs {
    R5_MSTATUS_XS_OFF,
    R5_MSTATUS_XS_INITIAL,
    R5_MSTATUS_XS_CLEAN,
    R5_MSTATUS_XS_DIRTY
};

struct r5_csr_mstatus {
    uint8_t uie;
    uint8_t sie;
    uint8_t mie;
    uint8_t upie;
    uint8_t spie;
    uint8_t mpie;
    uint8_t spp;
    enum r5_mstatus_mpp mpp;
    enum r5_mstatus_fs fs;
    enum r5_mstatus_xs xs;
    uint8_t mprv;
    uint8_t sum;
    uint8_t mxr;
    uint8_t tvm;
    uint8_t tw;
    uint8_t tsr;
    uint8_t sd;
};

enum r5_csr_mtvec_mode {
   R5_MTVEC_DIRECT,
   R5_MTVEC_VECTORED
};

struct r5_csr_mtvec {
   uint32_t base;
   enum r5_csr_mtvec_mode mode;
};

struct r5_csr_mip {
   uint8_t usip;
   uint8_t ssip;
   uint8_t msip;
   uint8_t utip;
   uint8_t stip;
   uint8_t mtip;
   uint8_t ueip;
   uint8_t seip;
   uint8_t meip;
};

struct r5_csr_mie {
   uint8_t usie;
   uint8_t ssie;
   uint8_t msie;
   uint8_t utie;
   uint8_t stie;
   uint8_t mtie;
   uint8_t ueie;
   uint8_t seie;
   uint8_t meie;
};

#endif