/*
    This file is part of the Bee6502 engine.
    Copyright (C) 2021 BueniaDev.

    Bee6502 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bee6502 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bee6502.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "bee6502.h"
using namespace bee6502;

namespace bee6502
{
    Bee6502Interface::Bee6502Interface()
    {

    }

    Bee6502Interface::~Bee6502Interface()
    {

    }

    Bee6502::Bee6502()
    {

    }

    Bee6502::~Bee6502()
    {

    }

    void Bee6502::init()
    {
	is_inst_fetch = true;
	is_reset = true;
	regaccum = 0;
	regx = 0x80;
	regy = 0;
	status_reg = 0;
	ir = 0;
	set_zero(true);
	cout << "Bee6502::Initialized" << endl;
    }

    void Bee6502::shutdown()
    {
	cout << "Bee6502::Shutting down...." << endl;
    }

    void Bee6502::runcycle()
    {
	if (is_inst_fetch || is_reset)
	{
	    if (is_inst_fetch)
	    {
		ir = readByte(pc);
		is_inst_fetch = false;

		if (is_reset)
		{
		    brk_flags = setbit(brk_flags, reset_brk);
		}

		if (brk_flags)
		{
		    ir = 0;
		    is_reset = false;
		}
		else
		{
		    pc += 1;
		}
	    }
	}

	exec_opcode();

	cycle_count += 1;

	if (is_inst_fetch)
	{
	    inst_cycles = cycle_count;
	    cycle_count = 0;
	}

	update_status();
    }

    void Bee6502::exec_opcode()
    {
	switch (get_opcode_cycle(ir, cycle_count))
	{
	    // BRK
	    case get_opcode_cycle(0x00, 0): break;
	    case get_opcode_cycle(0x00, 1):
	    {
		if ((brk_flags & 0x3) == 0)
		{
		    pc += 1;
		}

		addr_val = (0x100 | sp);

		if (!testbit(brk_flags, reset_brk))
		{
		    cout << "Pushing upper PC onto stack" << endl;
		    writeByte(addr_val, (pc >> 8));
		}
	    }
	    break;
	    case get_opcode_cycle(0x00, 2):
	    {
		addr_val = (0x100 | (sp - 1));
		if (!testbit(brk_flags, reset_brk))
		{
		    cout << "Pushing lower PC onto stack" << endl;
		    writeByte(addr_val, (pc & 0xFF));
		}
	    }
	    break;
	    case get_opcode_cycle(0x00, 3):
	    {
		addr_val = (0x100 | (sp - 2));
		sp -= 3;
		if (testbit(brk_flags, reset_brk))
		{
		    addr_val = 0xFFFC;
		}
		else
		{
		    cout << "Pushing flags onto stack" << endl;
		    writeByte(addr_val, (status_reg | 0x20));

		    if (testbit(brk_flags, 1))
		    {
			addr_val = 0xFFFA;
		    }
		    else
		    {
			addr_val = 0xFFFE;
		    }
		}
	    }
	    break;
	    case get_opcode_cycle(0x00, 4):
	    {
		data_val0 = readByte(addr_val++);
		status_reg |= 0x14;
		brk_flags = 0;
	    }
	    break;
	    case get_opcode_cycle(0x00, 5):
	    {
		data_val1 = readByte(addr_val);
	    }
	    break;
	    case get_opcode_cycle(0x00, 6):
	    {
		pc = ((data_val1 << 8) | data_val0);
		is_inst_fetch = true;
	    }
	    break;
	    // ADC zp
	    case get_opcode_cycle(0x65, 0):
	    {
		addr_val = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x65, 1):
	    {
		data_val0 = readByte(addr_val);
	    }
	    break;
	    case get_opcode_cycle(0x65, 2):
	    {
		regaccum = adc_internal();
		is_inst_fetch = true;
	    }
	    break;
	    // ADC #imm
	    case get_opcode_cycle(0x69, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x69, 1):
	    {
		regaccum = adc_internal();
		is_inst_fetch = true;
	    }
	    break;
	    // STA zp
	    case get_opcode_cycle(0x85, 0):
	    {
		addr_val = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x85, 1):
	    {
		writeByte(addr_val, regaccum);
	    }
	    break;
	    case get_opcode_cycle(0x85, 2):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    // TXA
	    case get_opcode_cycle(0x8A, 0): break;
	    case get_opcode_cycle(0x8A, 1):
	    {
		regaccum = regx;
		set_nz(regaccum);
		is_inst_fetch = true;
	    }
	    break;
	    // STA abs
	    case get_opcode_cycle(0x8D, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x8D, 1):
	    {
		data_val1 = readByte(pc++);
		addr_val = ((data_val1 << 8) | data_val0);
	    }
	    break;
	    case get_opcode_cycle(0x8D, 2):
	    {
		writeByte(addr_val, regaccum);
	    }
	    break;
	    case get_opcode_cycle(0x8D, 3):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    // STX abs
	    case get_opcode_cycle(0x8E, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x8E, 1):
	    {
		data_val1 = readByte(pc++);
		addr_val = ((data_val1 << 8) | data_val0);
	    }
	    break;
	    case get_opcode_cycle(0x8E, 2):
	    {
		writeByte(addr_val, regx);
	    }
	    break;
	    case get_opcode_cycle(0x8E, 3):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    // STA zp,X
	    case get_opcode_cycle(0x95, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x95, 1):
	    {
		addr_val = ((data_val0 + regx) & 0xFF);
	    }
	    break;
	    case get_opcode_cycle(0x95, 2):
	    {
		writeByte(addr_val, regaccum);
	    }
	    break;
	    case get_opcode_cycle(0x95, 3):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    // STA abs,Y
	    case get_opcode_cycle(0x99, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x99, 1):
	    {
		data_val1 = readByte(pc++);
		addr_val = ((data_val1 << 8) | data_val0);
	    }
	    break;
	    case get_opcode_cycle(0x99, 2):
	    {
		data_val1 = (addr_val >> 8);
		data_val0 = ((addr_val + regy) & 0xFF);
	    }
	    break;
	    case get_opcode_cycle(0x99, 3):
	    {
		addr_val += regy;
		writeByte(addr_val, regaccum);
	    }
	    break;
	    case get_opcode_cycle(0x99, 4):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    // TXS
	    case get_opcode_cycle(0x9A, 0): break;
	    case get_opcode_cycle(0x9A, 1):
	    {
		sp = regx;
		is_inst_fetch = true;
	    }
	    break;
	    // STA abs,X
	    case get_opcode_cycle(0x9D, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0x9D, 1):
	    {
		data_val1 = readByte(pc++);
		addr_val = ((data_val1 << 8) | data_val0);
	    }
	    break;
	    case get_opcode_cycle(0x9D, 2):
	    {
		data_val1 = (addr_val >> 8);
		data_val0 = ((addr_val + regx) & 0xFF);
	    }
	    break;
	    case get_opcode_cycle(0x9D, 3):
	    {
		addr_val += regx;
		writeByte(addr_val, regaccum);
	    }
	    break;
	    case get_opcode_cycle(0x9D, 4):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    // LDX #imm
	    case get_opcode_cycle(0xA2, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0xA2, 1):
	    {
		regx = data_val0;
		set_nz(regx);
		is_inst_fetch = true;
	    }
	    break;
	    // LDA #imm
	    case get_opcode_cycle(0xA9, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0xA9, 1):
	    {
		regaccum = data_val0;
		set_nz(regaccum);
		is_inst_fetch = true;
	    }
	    break;
	    // TAX
	    case get_opcode_cycle(0xAA, 0): break;
	    case get_opcode_cycle(0xAA, 1):
	    {
		regx = regaccum;
		set_nz(regx);
		is_inst_fetch = true;
	    }
	    break;
	    // DEX
	    case get_opcode_cycle(0xCA, 0): break;
	    case get_opcode_cycle(0xCA, 1):
	    {
		regx -= 1;
		set_nz(regx);
		is_inst_fetch = true;
	    }
	    break;
	    // BNE rel
	    case get_opcode_cycle(0xD0, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0xD0, 1):
	    {
		addr_val = (pc + int8_t(data_val0));

		if (is_zero())
		{
		    is_inst_fetch = true;
		}
	    }
	    break;
	    case get_opcode_cycle(0xD0, 2):
	    {
		if ((addr_val & 0xFF00) == (pc & 0xFF00))
		{
		    pc = addr_val;
		    is_inst_fetch = true;
		}
	    }
	    break;
	    case get_opcode_cycle(0xD0, 3):
	    {
		pc = addr_val;
		is_inst_fetch = true;
	    }
	    break;
	    // CLD
	    case get_opcode_cycle(0xD8, 0): break;
	    case get_opcode_cycle(0xD8, 1):
	    {
		set_decimal(false);
		is_inst_fetch = true;
	    }
	    break;
	    // CPX #imm
	    case get_opcode_cycle(0xE0, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    case get_opcode_cycle(0xE0, 1):
	    {
		cmp_internal(regx);
		is_inst_fetch = true;
	    }
	    break;
	    // INX
	    case get_opcode_cycle(0xE8, 0): break;
	    case get_opcode_cycle(0xE8, 1):
	    {
		regx += 1;
		set_nz(regx);
		is_inst_fetch = true;
	    }
	    break;
	    // SBC #imm
	    case get_opcode_cycle(0xE9, 0):
	    {
		data_val0 = readByte(pc++);
	    }
	    break;
	    // NOP
	    case get_opcode_cycle(0xEA, 0): break;
	    case get_opcode_cycle(0xEA, 1):
	    {
		is_inst_fetch = true;
	    }
	    break;
	    default: unrecognized_instr(); break;
	}
    }

    void Bee6502::set_nz(uint8_t value)
    {
	set_sign(testbit(value, 7));
	set_zero(value == 0);
    }

    void Bee6502::setBCD(bool is_enabled)
    {
	is_bcd = is_enabled;
    }

    void Bee6502::unrecognized_instr()
    {
	cout << "Unrecognized instruction of " << hex << int(ir) << ", cycle of " << dec << int(cycle_count) << endl;
	exit(1);
    }

    uint8_t Bee6502::adc_internal()
    {
	uint8_t value = 0;
	if (is_bcd && is_decimal())
	{
	    cout << "Decimal mode" << endl;
	    exit(0);
	}
	else
	{
	    uint16_t sum = (regaccum + data_val0 + is_carry());
	    set_nz((sum & 0xFF));

	    uint16_t overflow_res = ((regaccum ^ sum) & (data_val0 ^ sum));
	    set_overflow(testbit(overflow_res, 7));
	    set_carry((sum & 0xFF00) != 0);
	    value = (sum & 0xFF);
	}

	return value;
    }

    void Bee6502::cmp_internal(uint8_t reg)
    {
	uint16_t result = (reg - data_val0);

	set_nz((result & 0xFF));
	set_carry((result & 0xFF00) == 0);
    }

    void Bee6502::setinterface(Bee6502Interface *cb)
    {
	inter = cb;
    }

    void Bee6502::update_status()
    {
	main_status.rega = regaccum;
	main_status.regx = regx;
	main_status.regy = regy;
	main_status.regstatus = (status_reg | 0x20);
	main_status.sp = (0x100 | sp);
	main_status.pc = pc;
	main_status.ir = ir;
    }

    Bee6502::Status Bee6502::getStatus()
    {
	return main_status;
    }

    void Bee6502::debugoutput(bool print_disassembly)
    {
	cout << "PC: " << hex << int(main_status.pc) << endl;
	cout << "A: " << hex << int(main_status.rega) << endl;
	cout << "X: " << hex << int(main_status.regx) << endl;
	cout << "Y: " << hex << int(main_status.regy) << endl;
	cout << "P: " << hex << int(main_status.regstatus) << endl;
	cout << "SP: " << hex << int(main_status.sp) << endl;
	cout << "IR: " << hex << int(main_status.ir) << endl;
    }

    uint8_t Bee6502::readByte(uint16_t addr)
    {
	if (inter != NULL)
	{
	    return inter->readByte(addr);
	}

	return 0xEA; // NOP
    }

    void Bee6502::writeByte(uint16_t addr, uint8_t data)
    {
	if (inter != NULL)
	{
	    inter->writeByte(addr, data);
	}
    }
};