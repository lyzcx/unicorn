/* Unicorn Emulator Engine */
/* By Nguyen Anh Quynh <aquynh@gmail.com>, 2015 */

#include "hw/boards.h"
#include "hw/sparc/sparc.h"
#include "sysemu/cpus.h"
#include "unicorn.h"
#include "cpu.h"
#include "unicorn_common.h"


#define READ_QWORD(x) ((uint64)x)
#define READ_DWORD(x) (x & 0xffffffff)
#define READ_WORD(x) (x & 0xffff)
#define READ_BYTE_H(x) ((x & 0xffff) >> 8)
#define READ_BYTE_L(x) (x & 0xff)


static bool sparc_stop_interrupt(int intno)
{
    switch(intno) {
        default:
            return false;
        case TT_ILL_INSN:
            return true;
    }
}

static void sparc_set_pc(struct uc_struct *uc, uint64_t address)
{
    ((CPUSPARCState *)uc->current_cpu->env_ptr)->pc = address;
    ((CPUSPARCState *)uc->current_cpu->env_ptr)->npc = address + 4;
}

void sparc_reg_reset(uch handle)
{
    struct uc_struct *uc = (struct uc_struct *) handle;
    CPUArchState *env;

    env = first_cpu->env_ptr;
    memset(env->gregs, 0, sizeof(env->gregs));
    memset(env->fpr, 0, sizeof(env->fpr));
    memset(env->regbase, 0, sizeof(env->regbase));

    env->pc = 0;
    env->npc = 0;
}

int sparc_reg_read(uch handle, unsigned int regid, void *value)
{
    struct uc_struct *uc = (struct uc_struct *) handle;
    CPUState *mycpu = first_cpu;

    if (regid >= UC_SPARC_REG_G0 && regid <= UC_SPARC_REG_G7)
        *(int32_t *)value = SPARC_CPU(uc, mycpu)->env.gregs[regid - UC_SPARC_REG_G0];
    else {
        switch(regid) {
            default: break;
            case UC_SPARC_REG_PC:
                     *(int32_t *)value = SPARC_CPU(uc, mycpu)->env.pc;
                     break;
        }
    }

    return 0;
}


#define WRITE_DWORD(x, w) (x = (x & ~0xffffffff) | (w & 0xffffffff))
#define WRITE_WORD(x, w) (x = (x & ~0xffff) | (w & 0xffff))
#define WRITE_BYTE_H(x, b) (x = (x & ~0xff00) | (b & 0xff))
#define WRITE_BYTE_L(x, b) (x = (x & ~0xff) | (b & 0xff))

int sparc_reg_write(uch handle, unsigned int regid, const void *value)
{
    struct uc_struct *uc = (struct uc_struct *) handle;
    CPUState *mycpu = first_cpu;

    if (regid >= UC_SPARC_REG_G0 && regid <= UC_SPARC_REG_G7)
        SPARC_CPU(uc, mycpu)->env.gregs[regid - UC_SPARC_REG_G0] = *(int32_t *)value;
    else {
        switch(regid) {
            default: break;
            case UC_SPARC_REG_PC:
                     SPARC_CPU(uc, mycpu)->env.pc = *(uint32_t *)value;
                     SPARC_CPU(uc, mycpu)->env.npc = *(uint32_t *)value + 4;
                     break;
        }
    }


    return 0;
}

__attribute__ ((visibility ("default")))
void sparc_uc_init(struct uc_struct* uc)
{
    register_accel_types(uc);
    sparc_cpu_register_types(uc);
    leon3_machine_init(uc);
    uc->reg_read = sparc_reg_read;
    uc->reg_write = sparc_reg_write;
    uc->reg_reset = sparc_reg_reset;
    uc->set_pc = sparc_set_pc;
    uc->stop_interrupt = sparc_stop_interrupt;
    uc_common_init(uc);
}