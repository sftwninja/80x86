#include <VCore.h>
#include <VerilogDriver.h>

#include "svdpi.h"

#include "Memory.h"
#include "RegisterFile.h"
#include "CPU.h"
#include "RTLCPU.h"

union reg_converter {
    uint16_t v16;
    uint8_t v8[2];
};

template <bool debug_enabled>
RTLCPU<debug_enabled>::RTLCPU()
    : VerilogDriver<VCore, debug_enabled>("core"),
    i_in_progress(false),
    d_in_progress(false),
    mem_latency(0)
{
    this->reset();

    this->periodic(ClockSetup, [&]{ this->data_access(); });
    this->periodic(ClockSetup, [&]{ this->instruction_access(); });
}

template <bool debug_enabled>
void RTLCPU<debug_enabled>::reset()
{
    VerilogDriver<VCore, debug_enabled>::reset();
}

template <bool debug_enabled>
void RTLCPU<debug_enabled>::write_reg(GPR regnum, uint16_t val)
{
    svSetScope(svGetScopeFromName("TOP.Core.regfile"));

    if (regnum < NUM_16BIT_REGS) {
        this->dut.set_gpr(static_cast<int>(regnum), val);
    } else {
        auto regsel = static_cast<int>(regnum - AL) & 0x3;
        reg_converter conv;
        conv.v16 = this->dut.get_gpr(regsel);

        conv.v8[regnum >= AH] = val;

        this->dut.set_gpr(regsel, conv.v16);
    }
}

template <bool debug_enabled>
uint16_t RTLCPU<debug_enabled>::read_reg(GPR regnum)
{
    svSetScope(svGetScopeFromName("TOP.Core.regfile"));

    if (regnum < NUM_16BIT_REGS)
        return this->dut.get_gpr(static_cast<int>(regnum));

    auto regsel = static_cast<int>(regnum - AL) & 0x3;
    reg_converter conv;
    conv.v16 = this->dut.get_gpr(regsel);

    return conv.v8[regnum >= AH];
}

template <bool debug_enabled>
size_t RTLCPU<debug_enabled>::step()
{
    return 0;
}

template <bool debug_enabled>
void RTLCPU<debug_enabled>::write_flags(uint16_t val)
{
    (void)val;
}

template <bool debug_enabled>
uint16_t RTLCPU<debug_enabled>::read_flags()
{
    return 0;
}

template <bool debug_enabled>
bool RTLCPU<debug_enabled>::has_trapped() const
{
    return false;
}

template <bool debug_enabled>
void RTLCPU<debug_enabled>::data_access()
{
    if (this->dut.reset || !this->dut.instr_m_access || i_in_progress)
        return;

    i_in_progress = true;
    this->after_n_cycles(mem_latency, [&]{
        this->dut.instr_m_data_in =
            this->mem.template read<uint16_t>(this->dut.instr_m_addr << 1);
        this->dut.instr_m_ack = 1;
        this->after_n_cycles(1, [&]{
            this->dut.instr_m_ack = 0;
            i_in_progress = false;
        });
    });
}

template <bool debug_enabled>
void RTLCPU<debug_enabled>::instruction_access()
{
    if (this->dut.reset || !this->dut.data_m_access || d_in_progress)
        return;

    d_in_progress = true;
    this->after_n_cycles(mem_latency, [&]{
        auto v =
            this->mem.template read<uint16_t>(this->dut.data_m_addr << 1);
        if (this->dut.data_m_wr_en) {
            if (this->dut.data_m_bytesel & (1 << 0))
                this->mem.template write<uint8_t>(this->dut.data_m_addr << 1,
                                                    this->dut.data_m_data_out & 0xff);
            if (this->dut.data_m_bytesel & (1 << 1))
                this->mem.template write<uint8_t>((this->dut.data_m_addr << 1) + 1,
                                                    (this->dut.data_m_data_out >> 8) & 0xff);
        } else {
            this->dut.data_m_data_in = v;
        }
        this->dut.data_m_ack = 1;
        this->after_n_cycles(1, [&]{
            this->dut.data_m_ack = 0;
            d_in_progress = false;
        });
    });
}

template RTLCPU<verilator_debug_enabled>::RTLCPU();
