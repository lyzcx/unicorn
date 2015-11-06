/* Unicorn Emulator Engine */
/* By Nguyen Anh Quynh <aquynh@gmail.com>, 2015 */

#include "hw/boards.h"
#include "hw/mips/mips.h"
#include "sysemu/cpus.h"
#include "unicorn.h"
#include "cpu.h"

#include "unicorn_common.h"


#define READ_QWORD(x) ((uint64)x)
#define READ_DWORD(x) (x & 0xffffffff)
#define READ_WORD(x) (x & 0xffff)
#define READ_BYTE_H(x) ((x & 0xffff) >> 8)
#define READ_BYTE_L(x) (x & 0xff)


static uint64_t mips_mem_redirect(uint64_t address)
{
    // kseg0 range masks off high address bit
    if (address >= 0x80000000 && address <= 0x9fffffff)
        return address & 0x7fffffff;

    // kseg1 range masks off top 3 address bits
    if (address >= 0xa0000000 && address <= 0xbfffffff) {
        return address & 0x1fffffff;
    }

    // no redirect
    return address;
}

static void mips_set_pc(struct uc_struct *uc, uint64_t address)
{
    ((CPUMIPSState *)uc->current_cpu->env_ptr)->active_tc.PC = address;
}

void mips_reg_reset(struct uc_struct *uc)
{
    (void)uc;
    CPUArchState *env = first_cpu->env_ptr;
    memset(env->active_tc.gpr, 0, sizeof(env->active_tc.gpr));

    env->active_tc.PC = 0;
}

int mips_reg_read(struct uc_struct *uc, unsigned int regid, void *value)
{
    CPUState *mycpu = first_cpu;

    if (regid >= UC_MIPS_REG_0 && regid <= UC_MIPS_REG_31)
        *(int32_t *)value = MIPS_CPU(uc, mycpu)->env.active_tc.gpr[regid - UC_MIPS_REG_0];
    else {
        switch(regid) {
            default: break;
            case UC_MIPS_REG_PC:
                     *(int32_t *)value = MIPS_CPU(uc, mycpu)->env.active_tc.PC;
                     break;
        }
    }

    return 0;
}


#define WRITE_DWORD(x, w) (x = (x & ~0xffffffff) | (w & 0xffffffff))
#define WRITE_WORD(x, w) (x = (x & ~0xffff) | (w & 0xffff))
#define WRITE_BYTE_H(x, b) (x = (x & ~0xff00) | (b & 0xff))
#define WRITE_BYTE_L(x, b) (x = (x & ~0xff) | (b & 0xff))

int mips_reg_write(struct uc_struct *uc, unsigned int regid, const void *value)
{
    CPUState *mycpu = first_cpu;

    if (regid >= UC_MIPS_REG_0 && regid <= UC_MIPS_REG_31)
        MIPS_CPU(uc, mycpu)->env.active_tc.gpr[regid - UC_MIPS_REG_0] = *(uint32_t *)value;
    else {
        switch(regid) {
            default: break;
            case UC_MIPS_REG_PC:
                     MIPS_CPU(uc, mycpu)->env.active_tc.PC = *(uint32_t *)value;
                     break;
        }
    }


    return 0;
}

__attribute__ ((visibility ("default")))
#ifdef TARGET_MIPS64
#ifdef TARGET_WORDS_BIGENDIAN
  void mips64_uc_init(struct uc_struct* uc)
#else
  void mips64el_uc_init(struct uc_struct* uc)
#endif
#else // if TARGET_MIPS
#ifdef TARGET_WORDS_BIGENDIAN
  void mips_uc_init(struct uc_struct* uc)
#else
  void mipsel_uc_init(struct uc_struct* uc)
#endif
#endif
{
    register_accel_types(uc);
    mips_cpu_register_types(uc);
    mips_machine_init(uc);
    uc->reg_read = mips_reg_read;
    uc->reg_write = mips_reg_write;
    uc->reg_reset = mips_reg_reset;
    uc->set_pc = mips_set_pc;
    uc->mem_redirect = mips_mem_redirect;
    uc_common_init(uc);
}