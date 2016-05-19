#include "Emulate.h"

#include <cassert>
#include <functional>
#include <stdint.h>
#include "Memory.h"

class EmulatorPimpl {
public:
    EmulatorPimpl(RegisterFile *registers);

    size_t emulate();

    void set_memory(Memory *mem)
    {
        this->mem = mem;
    }

    void set_io(Memory *io)
    {
        this->io = io;
    }

    bool has_trapped() const
    {
        return executed_int3;
    }

    void reset();
private:
    uint8_t fetch_byte();
    //
    // mov
    //
    void mov88();
    void mov89();
    void mov8a();
    void mov8b();
    void movc6();
    void movc7();
    void movb0_b7();
    void movb8_bf();
    void mova0();
    void mova1();
    void mova2();
    void mova3();
    void mov8e();
    void mov8c();
    //
    // push
    //
    void pushff();
    void push50_57();
    void pushsr();
    //
    // pop
    //
    void pop8f();
    void pop58_5f();
    void popsr();
    //
    // xchg
    //
    void xchg86();
    void xchg87();
    void xchg90_97();
    //
    // in
    //
    void ine4();
    void ine5();
    void inec();
    void ined();
    //
    // out
    //
    void oute6();
    void oute7();
    void outee();
    void outef();
    //
    // xlat
    //
    void xlatd7();
    //
    // lea
    //
    void lea8d();
    //
    // lds
    //
    void ldsc5();
    //
    // les
    //
    void lesc4();
    //
    // lahf
    //
    void lahf9f();
    //
    // sahf
    //
    void sahf9e();
    //
    // pushf
    //
    void pushf9c();
    //
    // popf
    //
    void popf9d();
    //
    // add / adc
    //
    void add_adc_sub_sbb_cmp_80();
    void add_adc_sub_sbb_cmp_81();
    void add_adc_sub_sbb_cmp_82();
    void add_adc_sub_sbb_cmp_83();
    //
    // add
    //
    template <typename T>
    std::pair<uint16_t, T> do_add(uint16_t v1, uint16_t v2,
                                  uint16_t carry_in=0);
    void add00();
    void add01();
    void add02();
    void add03();
    void add04();
    void add05();
    //
    // adc
    //
    void adc10();
    void adc11();
    void adc12();
    void adc13();
    void adc14();
    void adc15();
    //
    // sub
    //
    template <typename T>
    std::pair<uint16_t, T> do_sub(uint16_t v1, uint16_t v2,
                                  uint16_t carry_in=0);
    void sub28();
    void sub29();
    void sub2a();
    void sub2b();
    void sub2c();
    void sub2d();
    //
    //sbb
    //
    void sbb18();
    void sbb19();
    void sbb1a();
    void sbb1b();
    void sbb1c();
    void sbb1d();
    //
    // inc
    //
    void inc_dec_fe();
    void incff();
    void inc40_47();
    //
    // dec
    //
    void decff();
    void dec48_4f();
    //
    // ascii
    //
    void aaa37();
    void daa27();
    void aas3f();
    void das2f();
    void aamd4();
    //
    // neg
    //
    void negf6();
    void negf7();
    //
    // cmp
    //
    void cmp38();
    void cmp39();
    void cmp3a();
    void cmp3b();
    void cmp3c();
    void cmp3d();
    //
    // mul
    //
    void mulf6();
    void mulf7();
    //
    // imul
    //
    void imulf6();
    void imulf7();
    //
    // int
    //
    void intcc();
    //
    // cbw / cwd
    //
    void cbw98();
    void cwd99();
    //
    // jump
    //
    void jmpe9();
    void jmpeb();
    void jmpff_intra();
    void jmpff_inter();
    void jmpea();
    void je74();
    void jl7c();
    void jle7e();
    void jp7a();
    void jb72();
    void jbe76();
    void jo70();
    void js78();
    void jne75();
    void jnl7d();
    void jnle7f();
    void jnb73();
    void jnbe77();
    void jnp7b();
    void jno71();
    //
    // call
    //
    void calle8();
    void callff_intra();
    void callff_inter();
    void call9a();
    //
    // ret
    //
    void retc3();
    void retc2();
    void retcb();
    void retca();
    //
    // iret
    //
    void iretcf();
    //
    // clc
    //
    void clcf8();
    //
    // cmc
    //
    void cmcf5();
    //
    // stc
    //
    void stcf9();
    //
    // cld
    //
    void cldfc();
    template <typename T>
    std::pair<uint16_t, T> do_mul(int32_t v1, int32_t v2);
    // Helpers
    void push_inc_jmp_call_ff();
    void neg_mul_f6();
    void neg_mul_f7();
    void push_word(uint16_t v);
    uint16_t pop_word();
    template <typename T>
    std::pair<uint16_t, T> do_alu(int32_t v1, int32_t v2,
                                  int32_t carry_in,
                                  std::function<uint32_t(uint32_t, uint32_t, uint32_t)> alu_op);
    template <typename T>
        void write_data(T val, bool stack=false);
    template <typename T>
        T read_data(bool stack=false);
    uint16_t fetch_16bit();

    Memory *mem;
    Memory *io;
    RegisterFile *registers;
    size_t instr_length = 0;
    std::unique_ptr<ModRMDecoder> modrm_decoder;
    uint8_t opcode;
    bool executed_int3;
};

EmulatorPimpl::EmulatorPimpl(RegisterFile *registers)
    : registers(registers),
    executed_int3(false)
{
    modrm_decoder = std::make_unique<ModRMDecoder>(
        [&]{ return this->fetch_byte(); },
        this->registers
    );

    reset();
}

void EmulatorPimpl::reset()
{
    registers->reset();
    opcode = 0;
    instr_length = 0;
    executed_int3 = false;
}

size_t EmulatorPimpl::emulate()
{
    instr_length = 0;
    executed_int3 = false;
    auto orig_ip = registers->get(IP);

    opcode = fetch_byte();
    switch (opcode) {
    // mov
    case 0x88: mov88(); break;
    case 0x89: mov89(); break;
    case 0x8a: mov8a(); break;
    case 0x8b: mov8b(); break;
    case 0xc6: movc6(); break;
    case 0xc7: movc7(); break;
    case 0xb0 ... 0xb7: movb0_b7(); break;
    case 0xb8 ... 0xbf: movb8_bf(); break;
    case 0xa0: mova0(); break;
    case 0xa1: mova1(); break;
    case 0xa2: mova2(); break;
    case 0xa3: mova3(); break;
    case 0x8e: mov8e(); break;
    case 0x8c: mov8c(); break;
    // push
    case 0xff: push_inc_jmp_call_ff(); break;
    case 0x50 ... 0x57: push50_57(); break;
    case 0x06: // fallthrough
    case 0x0e: // fallthrough
    case 0x16: // fallthrough
    case 0x1e: pushsr(); break;
    // pop
    case 0x8f: pop8f(); break;
    case 0x58 ... 0x5f: pop58_5f(); break;
    case 0x07: // fallthrough
    case 0x0f: // fallthrough
    case 0x17: // fallthrough
    case 0x1f: popsr(); break;
    // xchg
    case 0x86: xchg86(); break;
    case 0x87: xchg87(); break;
    case 0x90 ... 0x97: xchg90_97(); break;
    // in
    case 0xe4: ine4(); break;
    case 0xe5: ine5(); break;
    case 0xec: inec(); break;
    case 0xed: ined(); break;
    // out
    case 0xe6: oute6(); break;
    case 0xe7: oute7(); break;
    case 0xee: outee(); break;
    case 0xef: outef(); break;
    // xlat
    case 0xd7: xlatd7(); break;
    // lea
    case 0x8d: lea8d(); break;
    // lds
    case 0xc5: ldsc5(); break;
    // les
    case 0xc4: lesc4(); break;
    // lahf
    case 0x9f: lahf9f(); break;
    // sahf
    case 0x9e: sahf9e(); break;
    // pushf
    case 0x9c: pushf9c(); break;
    // popf
    case 0x9d: popf9d(); break;
    // add / adc / sub / sbb / cmp
    case 0x80: add_adc_sub_sbb_cmp_80(); break;
    case 0x81: add_adc_sub_sbb_cmp_81(); break;
    case 0x82: add_adc_sub_sbb_cmp_82(); break;
    case 0x83: add_adc_sub_sbb_cmp_83(); break;
    // add
    case 0x00: add00(); break;
    case 0x01: add01(); break;
    case 0x02: add02(); break;
    case 0x03: add03(); break;
    case 0x04: add04(); break;
    case 0x05: add05(); break;
    // adc
    case 0x10: adc10(); break;
    case 0x11: adc11(); break;
    case 0x12: adc12(); break;
    case 0x13: adc13(); break;
    case 0x14: adc14(); break;
    case 0x15: adc15(); break;
    // sub
    case 0x28: sub28(); break;
    case 0x29: sub29(); break;
    case 0x2a: sub2a(); break;
    case 0x2b: sub2b(); break;
    case 0x2c: sub2c(); break;
    case 0x2d: sub2d(); break;
    // sbb
    case 0x18: sbb18(); break;
    case 0x19: sbb19(); break;
    case 0x1a: sbb1a(); break;
    case 0x1b: sbb1b(); break;
    case 0x1c: sbb1c(); break;
    case 0x1d: sbb1d(); break;
    // inc
    case 0xfe: inc_dec_fe(); break;
    case 0x40 ... 0x47: inc40_47(); break;
    // dec
    case 0x48 ... 0x4f: dec48_4f(); break;
    // ascii
    case 0x37: aaa37(); break;
    case 0x27: daa27(); break;
    case 0x2f: das2f(); break;
    case 0x3f: aas3f(); break;
    case 0xd4: aamd4(); break;
    // neg
    case 0xf6: neg_mul_f6(); break;
    case 0xf7: neg_mul_f7(); break;
    // cmp
    case 0x38: cmp38(); break;
    case 0x39: cmp39(); break;
    case 0x3a: cmp3a(); break;
    case 0x3b: cmp3b(); break;
    case 0x3c: cmp3c(); break;
    case 0x3d: cmp3d(); break;
    // int
    case 0xcc:
        intcc();
        executed_int3 = true;
        break;
    // cbw
    case 0x98: cbw98(); break;
    // cwd
    case 0x99: cwd99(); break;
    // jmp
    case 0xe9: jmpe9(); break;
    case 0xea: jmpea(); break;
    case 0xeb: jmpeb(); break;
    case 0x70: jo70(); break;
    case 0x72: jb72(); break;
    case 0x74: je74(); break;
    case 0x76: jbe76(); break;
    case 0x78: js78(); break;
    case 0x7a: jp7a(); break;
    case 0x7c: jl7c(); break;
    case 0x7e: jle7e(); break;
    case 0x75: jne75(); break;
    case 0x7d: jnl7d(); break;
    case 0x7f: jnle7f(); break;
    case 0x73: jnb73(); break;
    case 0x77: jnbe77(); break;
    case 0x7b: jnp7b(); break;
    case 0x71: jno71(); break;
    // call
    case 0xe8: calle8(); break;
    case 0x9a: call9a(); break;
    // ret
    case 0xc3: retc3(); break;
    case 0xc2: retc2(); break;
    case 0xcb: retcb(); break;
    case 0xca: retca(); break;
    // iret
    case 0xcf: iretcf(); break;
    // clc
    case 0xf8: clcf8(); break;
    // cmc
    case 0xf5: cmcf5(); break;
    // stc
    case 0xf9: stcf9(); break;
    // cld
    case 0xfc: cldfc(); break;
    }

    if (registers->get(IP) == orig_ip)
        registers->set(IP, registers->get(IP) + instr_length);

    return instr_length;
}

void EmulatorPimpl::push_inc_jmp_call_ff()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 6)
        pushff();
    else if (modrm_decoder->raw_reg() == 0)
        incff();
    else if (modrm_decoder->raw_reg() == 1)
        decff();
    else if (modrm_decoder->raw_reg() == 2)
        callff_intra();
    else if (modrm_decoder->raw_reg() == 3)
        callff_inter();
    else if (modrm_decoder->raw_reg() == 4)
        jmpff_intra();
    else if (modrm_decoder->raw_reg() == 5)
        jmpff_inter();
}

template <typename T>
std::pair<uint16_t, T>
EmulatorPimpl::do_alu(int32_t v1, int32_t v2, int32_t carry,
                      std::function<uint32_t(uint32_t, uint32_t, uint32_t)> alu_op)
{
    uint16_t flags = registers->get_flags();

    flags &= ~(AF | CF | OF | PF | SF | ZF);

    uint32_t result32 = alu_op(static_cast<uint32_t>(v1),
                               static_cast<uint32_t>(v2),
                               static_cast<uint32_t>(carry));
    bool af = !!(alu_op(v1 & 0xf, v2 & 0xf, 0) & (1 << 4));

    auto sign_bit = (8 * sizeof(T)) - 1;
    auto carry_bit = (8 * sizeof(T));

    if (af)
        flags |= AF;
    if (result32 & (1 << carry_bit))
        flags |= CF;
    if (result32 & (1 << sign_bit))
        flags |= SF;
    if ((result32 & static_cast<T>(-1)) == 0)
        flags |= ZF;
    if (!__builtin_parity(result32 & static_cast<uint8_t>(-1)))
        flags |= PF;
    bool carry_in = !!(alu_op(static_cast<uint32_t>(v1) & ~(1 << sign_bit),
                              static_cast<uint32_t>(v2) & ~(1 << sign_bit),
                              static_cast<uint32_t>(carry)) & (1 << sign_bit));
    if (carry_in ^ !!(flags & CF))
        flags |= OF;

    return std::make_pair(flags, static_cast<T>(result32));
}

template <typename T>
std::pair<uint16_t, T> EmulatorPimpl::do_add(uint16_t v1, uint16_t v2,
                                             uint16_t carry_in)
{
    return do_alu<T>(v1, v2, carry_in,
        [](uint32_t a, uint32_t b, uint32_t c) -> uint32_t {
            return a + b + c;
        });
}

template <typename T>
std::pair<uint16_t, T> EmulatorPimpl::do_sub(uint16_t v1, uint16_t v2,
                                             uint16_t carry_in)
{
    return do_alu<T>(v1, v2, carry_in,
        [](uint32_t a, uint32_t b, uint32_t c) -> uint32_t {
            return a - b - c;
        });
}

template <typename T>
std::pair<uint16_t, T> EmulatorPimpl::do_mul(int32_t v1, int32_t v2)
{
    return do_alu<T>(v1, v2, 0,
        [](uint32_t a, uint32_t b, uint32_t __attribute__((unused)) c) -> uint32_t {
            return a * b;
        });
}

void EmulatorPimpl::add_adc_sub_sbb_cmp_80()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2 &&
        modrm_decoder->raw_reg() != 5 &&
        modrm_decoder->raw_reg() != 3 &&
        modrm_decoder->raw_reg() != 7)
        return;

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = fetch_byte();
    bool carry_in = modrm_decoder->raw_reg() == 2 || modrm_decoder->raw_reg() == 3 ?
        !!(registers->get_flags() & CF) : 0;

    uint8_t result;
    uint16_t flags;
    if (modrm_decoder->raw_reg() == 0 ||
        modrm_decoder->raw_reg() == 2)
        std::tie(flags, result) = do_add<uint8_t>(v1, v2, carry_in);
    else
        std::tie(flags, result) = do_sub<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    // cmp doesn't write the result
    if (modrm_decoder->raw_reg() != 7)
        write_data<uint8_t>(result & 0xff);
}

void EmulatorPimpl::add_adc_sub_sbb_cmp_82()
{
    // The 's' bit has no effect for 8-bit add immediate.
    add_adc_sub_sbb_cmp_80();
}

// add r/m, immediate, 16-bit
void EmulatorPimpl::add_adc_sub_sbb_cmp_81()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2 &&
        modrm_decoder->raw_reg() != 5 &&
        modrm_decoder->raw_reg() != 3 &&
        modrm_decoder->raw_reg() != 7)
        return;

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = fetch_16bit();
    bool carry_in = modrm_decoder->raw_reg() == 2 || modrm_decoder->raw_reg() == 3 ?
        !!(registers->get_flags() & CF) : 0;

    uint16_t result;
    uint16_t flags;
    if (modrm_decoder->raw_reg() == 0 ||
        modrm_decoder->raw_reg() == 2)
        std::tie(flags, result) = do_add<uint16_t>(v1, v2, carry_in);
    else
        std::tie(flags, result) = do_sub<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    // cmp doesn't write the result
    if (modrm_decoder->raw_reg() != 7)
        write_data<uint16_t>(result & 0xffff);
}

// add r/m, immediate, 8-bit, sign-extended
void EmulatorPimpl::add_adc_sub_sbb_cmp_83()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2 &&
        modrm_decoder->raw_reg() != 5 &&
        modrm_decoder->raw_reg() != 3 &&
        modrm_decoder->raw_reg() != 7)
        return;

    uint16_t v1 = read_data<uint16_t>();
    int16_t immed = fetch_byte();
    immed <<= 8;
    immed >>= 8;
    bool carry_in = modrm_decoder->raw_reg() == 2 || modrm_decoder->raw_reg() == 3 ?
        !!(registers->get_flags() & CF) : 0;

    uint16_t result;
    uint16_t flags;
    if (modrm_decoder->raw_reg() == 0 ||
        modrm_decoder->raw_reg() == 2)
        std::tie(flags, result) = do_add<uint16_t>(v1, immed, carry_in);
    else
        std::tie(flags, result) = do_sub<uint16_t>(v1, immed, carry_in);

    registers->set_flags(flags);
    // cmp doesn't write the result
    if (modrm_decoder->raw_reg() != 7)
        write_data<uint16_t>(result & 0xffff);
}

uint8_t EmulatorPimpl::fetch_byte()
{
    return mem->read<uint8_t>(get_phys_addr(registers->get(CS),
                                            registers->get(IP) + instr_length++));
}

void EmulatorPimpl::neg_mul_f6()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 0x3)
        negf6();
    else if (modrm_decoder->raw_reg() == 0x4)
        mulf6();
    else if (modrm_decoder->raw_reg() == 0x5)
        imulf6();
}

void EmulatorPimpl::neg_mul_f7()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 0x3)
        negf7();
    else if (modrm_decoder->raw_reg() == 0x4)
        mulf7();
    else if (modrm_decoder->raw_reg() == 0x5)
        imulf7();
}

template <typename Out, typename In>
static inline Out sign_extend(In v)
{
    static_assert(sizeof(Out) > sizeof(In), "May only sign extend to larger types");

    size_t bit_shift = (sizeof(Out) - sizeof(In)) * 8;
    Out o = static_cast<Out>(v);

    o <<= bit_shift;
    o >>= bit_shift;

    return o;
}

#include "instructions/mov.cpp"
#include "instructions/push.cpp"
#include "instructions/pop.cpp"
#include "instructions/xchg.cpp"
#include "instructions/io.cpp"
#include "instructions/xlat.cpp"
#include "instructions/lea.cpp"
#include "instructions/lahf_sahf.cpp"
#include "instructions/add.cpp"
#include "instructions/adc.cpp"
#include "instructions/sub.cpp"
#include "instructions/sbb.cpp"
#include "instructions/inc_dec.cpp"
#include "instructions/aaa.cpp"
#include "instructions/daa.cpp"
#include "instructions/aas.cpp"
#include "instructions/aam.cpp"
#include "instructions/das.cpp"
#include "instructions/neg.cpp"
#include "instructions/cmp.cpp"
#include "instructions/mul.cpp"
#include "instructions/imul.cpp"
#include "instructions/int.cpp"
#include "instructions/cbw.cpp"
#include "instructions/cwd.cpp"
#include "instructions/jmp.cpp"
#include "instructions/call.cpp"
#include "instructions/ret.cpp"
#include "instructions/clc.cpp"
#include "instructions/cmc.cpp"
#include "instructions/stc.cpp"
#include "instructions/cld.cpp"

void EmulatorPimpl::push_word(uint16_t v)
{
    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, v);
}

uint16_t EmulatorPimpl::pop_word()
{
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto v = mem->read<uint16_t>(addr);
    registers->set(SP, registers->get(SP) + 2);

    return v;
}

uint16_t EmulatorPimpl::fetch_16bit()
{
    uint16_t immed = (static_cast<uint16_t>(fetch_byte()) |
                      (static_cast<uint16_t>(fetch_byte()) << 8));
    return immed;
}

template <typename T>
void EmulatorPimpl::write_data(T val, bool stack)
{
    if (modrm_decoder->rm_type() == OP_REG) {
        auto dest = modrm_decoder->rm_reg();
        registers->set(dest, val);
    } else {
        auto ea = modrm_decoder->effective_address();
        auto segment = modrm_decoder->uses_bp_as_base() || stack ? SS : DS;
        auto addr = get_phys_addr(registers->get(segment), ea);
        mem->write<T>(addr, val);
    }
}

template <typename T>
T EmulatorPimpl::read_data(bool stack)
{
    if (modrm_decoder->rm_type() == OP_MEM) {
        auto displacement = modrm_decoder->effective_address();
        auto segment = modrm_decoder->uses_bp_as_base() || stack ? SS : DS;
        auto addr = get_phys_addr(registers->get(segment), displacement);

        return mem->read<T>(addr);
    } else {
        auto source = modrm_decoder->rm_reg();
        return registers->get(source);
    }
}

Emulator::Emulator(RegisterFile *registers)
    : pimpl(std::make_unique<EmulatorPimpl>(registers))
{
}

Emulator::~Emulator()
{
}

size_t Emulator::emulate()
{
    return pimpl->emulate();
}

void Emulator::set_memory(Memory *mem)
{
    pimpl->set_memory(mem);
}

void Emulator::set_io(Memory *io)
{
    pimpl->set_io(io);
}

bool Emulator::has_trapped() const
{
    return pimpl->has_trapped();
}

void Emulator::reset()
{
    pimpl->reset();
}
