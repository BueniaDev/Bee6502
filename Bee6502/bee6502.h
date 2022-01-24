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

#ifndef BEE6502_H
#define BEE6502_H

#include <iostream>
#include <sstream>
#include <cstdint>
using namespace std;

namespace bee6502
{
    class Bee6502Interface
    {
	public:
	    Bee6502Interface();
	    ~Bee6502Interface();

	    virtual uint8_t readByte(uint16_t addr)
	    {
		cout << "Reading value from address of " << hex << int(addr) << endl;
		exit(0);
		return 0;
	    }

	    virtual void writeByte(uint16_t addr, uint8_t data)
	    {
		cout << "Writing value of " << hex << int(data) << " to address of " << hex << int(addr) << endl;
		exit(0);
	    }
    };

    class Bee6502
    {
	#define get_opcode_cycle(op, cycle)  ((op << 3) | (cycle & 7))
	public:
	    Bee6502();
	    ~Bee6502();

	    struct Status
	    {
		uint8_t rega;
		uint8_t regx;
		uint8_t regy;
		uint8_t regstatus;
		uint16_t sp;
		uint16_t pc;
		uint8_t ir;
	    };

	    void init();
	    void shutdown();

	    void setinterface(Bee6502Interface *cb);
	    void runcycle();

	    void setBCD(bool is_enabled);

	    Status getStatus();

	    int runinstruction()	
	    {
		do
		{
		    runcycle();
		} while (!is_inst_fetch);

		return inst_cycles;
	    }
	    
	    void debugoutput(bool print_disassembly = true);

	    size_t disassembleinstr(ostream &stream, size_t pc);

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }

	    template<typename T>
	    T setbit(T reg, int bit)
	    {
		return (reg | (1 << bit));
	    }

	    template<typename T>
	    T resetbit(T reg, int bit)
	    {
		return (reg & ~(1 << bit));
	    }

	    template<typename T>
	    T changebit(T reg, int bit, bool is_set)
	    {
		if (is_set)
		{
		    return setbit(reg, bit);
		}
		else
		{
		    return resetbit(reg, bit);
		}
	    }

	    size_t singlebit(int bit)
	    {
		return (1 << bit);
	    }

	    uint8_t readByte(uint16_t addr);
	    void writeByte(uint16_t addr, uint8_t data);

	    void exec_opcode();

	    uint8_t adc_internal();
	    void cmp_internal(uint8_t reg);

	    void unrecognized_instr();

	    uint8_t regaccum = 0;
	    uint8_t regx = 0;
	    uint8_t regy = 0;
	    uint8_t status_reg = 0;
	    uint8_t sp = 0;
	    uint16_t pc = 0;
	    uint8_t ir = 0;

	    uint16_t addr_val = 0;
	    uint8_t data_val0 = 0;
	    uint8_t data_val1 = 0;

	    bool is_rw = false;
	    bool is_inst_fetch = false;
	    bool is_reset = false;
	    int cycle_count = 0;
	    int inst_cycles = 0;

	    int brk_flags = 0;

	    const int reset_brk = 2;

	    bool is_bcd = false;

	    Status main_status;
	    void update_status();

	    Bee6502Interface *inter = NULL;

	    uint8_t current_instr = 0;

	    void set_nz(uint8_t value);

	    bool is_sign()
	    {
		return testbit(status_reg, 7);
	    }

	    void set_sign(bool val)
	    {
		status_reg = changebit(status_reg, 7, val);
	    }

	    void set_overflow(bool val)
	    {
		status_reg = changebit(status_reg, 6, val);
	    }

	    bool is_decimal()
	    {
		return testbit(status_reg, 3);
	    }

	    void set_decimal(bool val)
	    {
		status_reg = changebit(status_reg, 3, val);
	    }

	    bool is_zero()
	    {
		return testbit(status_reg, 1);
	    }

	    void set_zero(bool val)
	    {
		status_reg = changebit(status_reg, 1, val);
	    }

	    bool is_carry()
	    {
		return testbit(status_reg, 0);
	    }

	    void set_carry(bool val)
	    {
		status_reg = changebit(status_reg, 0, val);
	    }
    };
};


#endif // BEE6502_H