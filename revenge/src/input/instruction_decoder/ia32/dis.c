/*
 *  Copyright (C) 2004  The revenge Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/* Intel ia32 instruction format: -
 Instruction-Prefixes (Up to four prefixes of 1-byte each. [optional] )
 Opcode (1-, 2-, or 3-byte opcode)
 ModR/M (1 byte [if required] )
 SIB (Scale-Index-Base:1 byte [if required] )
 Displacement (Address displacement of 1, 2, or 4 bytes or none)
 Immediate (Immediate data of 1, 2, or 4 bytes or none)

 Naming convention taked from Intel Instruction set manual, Appendix A. 25366713.pdf
*/

#include "dis.hpp"

instructions_t instructions;
uint8_t inst[]={0x57,
	0x56,
	0x53,
	0x83, 0xec, 0x18,
	0x8b, 0x74, 0x24, 0x28,
	0x8d, 0x44, 0x24, 0x14,
	0x8b, 0x5c, 0x24, 0x2c,
	0x8d, 0x7e, 0x48,
	0x50,
	0x57,
	0xe8, 0xfc, 0xff, 0xff, 0xff,
	0x58,
	0x5a,
	0x0f, 0xb7, 0x44, 0x24, 0x30,
	0xc1, 0xe3, 0x10,
	0x09, 0xc3,
	0x66, 0x8b, 0x56, 0x08,
	0x89, 0xd8,
	0xef, 
	0x83, 0xc2, 0x04,
	0xed,
	0xff, 0x74, 0x24, 0x14,
	0x57,
	0x89, 0xc3,
	0xe8, 0xfc, 0xff, 0xff, 0xff,
	0x83, 0xc4, 0x20,
	0x89, 0xd8,
	0x5b,
	0x5e,
	0x5f,
	0xc3
};

/* Little endian */
uint32_t getbyte(uint8_t *address) {
	uint32_t result;
	result=*address;
	printf(" 0x%x\n",result);
	return result;
}

uint32_t getdword(uint8_t *address) {
	uint32_t result;
	result=getbyte(address++);
	result=result | getbyte(address++) << 8;
	result=result | getbyte(address++) << 16;
	result=result | getbyte(address++) << 24;
	return result;
}


void split_ModRM(uint8_t byte, uint8_t *reg,  uint8_t *reg_mem, uint8_t *mod) {
  *reg = (byte >> 3) & 0x7; //bits 3-5
  *reg_mem = (byte & 0x7); //bits 0-2
  *mod = (byte >> 6); //bit 6-7
  printf("byte=%02x, reg=%02x, reg_mem=%02x, mod=%02x\n",
    byte,
    *reg,
    *reg_mem,
    *mod);
}
void split_SIB(uint8_t byte, uint8_t *mul,  uint8_t *index, uint8_t *base) {
  *index = (byte >> 3) & 0x7; //bits 3-5
  *base = (byte & 0x7); //bits 0-2
  *mul = (byte >> 6); //bit 6-7
// do the *2 etc. later
//  *mul = 1 << *mul; // convert bits to *1, *2, *4, *8
  printf("byte=%02x, mul=%02x, index=%02x, base=%02x\n",
    byte,
    *mul,
    *index,
    *base);
}

int rmb(instructions_t *instructions, uint8_t *bytes_base, uint8_t *return_reg) {
	uint8_t reg;
	uint8_t reg_mem;
	uint8_t mod;
	uint8_t mul, index, base;
	instruction_t *instruction;
	split_ModRM(getbyte(&bytes_base[instructions->bytes_used]),&reg, &reg_mem, &mod);
	instructions->bytes_used++;
	*return_reg = reg;
	if (mod == 3) {  // Special case Reg to Reg transfer
/* Fill in half of instruction */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = NOP;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = reg_table[reg_mem].offset;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = reg_table[reg_mem].offset;
		instruction->dstA.size = 4;
		return 1;
	}
	if (reg_mem == 4) {
		split_SIB(getbyte(&bytes_base[instructions->bytes_used]),&mul, &index, &base);
		instructions->bytes_used++;
		if (index != 4) {
			/* Handle scaled index */
			instruction = &instructions->instruction[instructions->instruction_number];	
			instruction->opcode = MOV;
			instruction->srcA.store = 1;
			instruction->srcA.indirect = 0;
			instruction->srcA.value = reg_table[index].offset;
			instruction->srcA.size = reg_table[index].size;
			instruction->dstA.store = 1;
			instruction->dstA.indirect = 0;
			instruction->dstA.value = REG_TMP;
			instruction->dstA.size = 4;
			instructions->instruction_number++;
			if (mul > 0) {
				instruction = &instructions->instruction[instructions->instruction_number];	
				instruction->opcode = MUL;
				instruction->srcA.store = 0;
				instruction->srcA.indirect = 0;
				instruction->srcA.value = 1 << mul;
				instruction->srcA.size = 4;
				instruction->dstA.store = 1;
				instruction->dstA.indirect = 0;
				instruction->dstA.value = REG_TMP;
				instruction->dstA.size = 4;
				instructions->instruction_number++;
			}
		}
		if (base == 5) {
			/* Handle base==5 */
			instruction = &instructions->instruction[instructions->instruction_number];	
			instruction->opcode = (instructions->instruction_number > 0) ? ADD : MOV;
			if (mod==0) {
				instruction->srcA.store = 0;
				instruction->srcA.indirect = 0;
				instruction->srcA.value = getdword(&bytes_base[instructions->bytes_used]); // Means get from rest of instruction
				instructions->bytes_used+=4;
				instruction->srcA.size = 4;
			} else {
				instruction->srcA.store = 1;
				instruction->srcA.indirect = 0;
				instruction->srcA.value = REG_BP;
				instruction->srcA.size = 4;
			}
			instruction->dstA.store = 1;
			instruction->dstA.indirect = 0;
			instruction->dstA.value = REG_TMP;
			instruction->dstA.size = 4;
			instructions->instruction_number++;
		} else {
			instruction = &instructions->instruction[instructions->instruction_number];	
			instruction->opcode = (instructions->instruction_number > 0) ? ADD : MOV;
			instruction->srcA.store = 1;
			instruction->srcA.indirect = 0;
			instruction->srcA.value = reg_table[index].offset;
			instruction->srcA.size = reg_table[index].size;
			instruction->dstA.store = 1;
			instruction->dstA.indirect = 0;
			instruction->dstA.value = REG_TMP;
			instruction->dstA.size = 4;
			instructions->instruction_number++;
		}
	} else if ((reg_mem == 5) && (mod == 0)) {
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = (instructions->instruction_number > 0) ? ADD : MOV;
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = getdword(&bytes_base[instructions->bytes_used]); // Means get from rest of instruction
		instructions->bytes_used+=4;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = REG_TMP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
	} else {
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = (instructions->instruction_number > 0) ? ADD : MOV;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = reg_table[reg_mem].offset;
		instruction->srcA.size = reg_table[reg_mem].size;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = REG_TMP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
	}
	if (mod > 0) {
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = (instructions->instruction_number > 0) ? ADD : MOV;
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		if (mod == 1) {
			instruction->srcA.size = 1;
			instruction->srcA.value = getbyte(&bytes_base[instructions->bytes_used]); // Means get from rest of instruction
			instructions->bytes_used++;
		} else { /* mod == 2 */
			instruction->srcA.size = 4;
			instruction->srcA.value = getdword(&bytes_base[instructions->bytes_used]); // Means get from rest of instruction
			instructions->bytes_used+=4;
		}
	instruction->dstA.store = 1;
	instruction->dstA.indirect = 0;
	instruction->dstA.value = REG_TMP;
	instruction->dstA.size = 4;
	instructions->instruction_number++;
	}
	return 0;
}

void dis_Ex_Gx(int opcode, instructions_t *instructions, uint8_t *inst, uint8_t *reg, int size) {
  int half;
  instruction_t *instruction;

  half = rmb(instructions, inst, reg);
  instruction = &instructions->instruction[instructions->instruction_number];	
  instruction->opcode = opcode;
  instruction->srcA.store = 1;
  instruction->srcA.indirect = 0;
  instruction->srcA.value = reg_table[*reg].offset;
  instruction->srcA.size = size;
  if (!half) {
    instruction->dstA.store = 1;
    instruction->dstA.indirect = 1;
    instruction->dstA.value = REG_TMP;
    instruction->dstA.size = size;
  }
  instructions->instruction_number++;
}

void dis_Gx_Ex(int opcode, instructions_t *instructions, uint8_t *inst, uint8_t *reg, int size) {
  int half=0;
  instruction_t *instruction;

  half = rmb(instructions, inst, reg);
  instruction = &instructions->instruction[instructions->instruction_number];	
  instruction->opcode = opcode;
  instruction->dstA.store = 1;
  instruction->dstA.indirect = 0;
  instruction->dstA.value = reg_table[*reg].offset;
  instruction->dstA.size = size;
  if (!half) {
    instruction->srcA.store = 1;
    instruction->srcA.indirect = 1;
    instruction->srcA.value = REG_TMP;
    instruction->srcA.size = size;
  }
  instructions->instruction_number++;
}

int disassemble(instructions_t *instructions, uint8_t *inst) {
	uint8_t reg=0;
	int half=0;
	instruction_t *instruction;
	printf("inst[0]=0x%x\n",inst[0]);
	instructions->instruction[instructions->instruction_number].opcode = NOP; /* Un-supported OPCODE */
	switch(inst[instructions->bytes_used++]) {
	case 0x00:												/* ADD Eb,Gb */
		dis_Ex_Gx(ADD, instructions, inst, &reg, 1);
		break;
	case 0x01:												/* ADD Ev,Gv */
		dis_Ex_Gx(ADD, instructions, inst, &reg, 4);
		break;
	case 0x02:												/* ADD Gb,Eb */
		dis_Gx_Ex(ADD, instructions, inst, &reg, 1);
		break;
	case 0x03:												/* ADD Gv,Ev */
		dis_Gx_Ex(ADD, instructions, inst, &reg, 4);
		break;
	case 0x04:												/* ADD AL,Ib */
		break;

	case 0x05:												/* ADD eAX,Iv */
		break;
	case 0x06:												/* PUSH ES */
                /* PUSH -> SP=SP-4 (-2 for word); [SP]=ES; */
		break;
	case 0x07:												/* POP ES */		
                /* POP -> ES=[SP]; SP=SP+4 (+2 for word); */
		break;
	case 0x08:												/* OR Eb,Gb */
		dis_Ex_Gx(OR, instructions, inst, &reg, 1);
		break;
	case 0x09:												/* OR Ev,Gv */
		dis_Ex_Gx(OR, instructions, inst, &reg, 4);
		break;
	case 0x0a:												/* OR Gb,Eb */
		dis_Gx_Ex(OR, instructions, inst, &reg, 1);
		break;
	case 0x0b:												/* OR Gv,Ev */
		dis_Gx_Ex(OR, instructions, inst, &reg, 4);
		break;
	case 0x0c:												/* OR AL,Ib */
		break;
	case 0x0d:												/* OR eAX,Iv */
		break;
	case 0x0e:												/* PUSH CS */		
		break;
	case 0x0f:												/* 2 byte opcodes*/
		prefix_0f(instructions, inst);
		break;
	case 0x10:												/* ADC Eb,Gb */
		break;
	case 0x11:												/* ADC Ev,Gv */
		break;
	case 0x12:												/* ADC Gb,Eb */
		break;
	case 0x13:												/* ADC Gv,Ev */
		break;
	case 0x14:												/* ADC AL,Ib */
		break;
	case 0x15:												/* ADC eAX,Iv */
		break;
	case 0x16:												/* PUSH SS */		
		break;
	case 0x17:												/* POP SS */		
		break;
	case 0x18:												/* SBB Eb,Gb */
		break;
	case 0x19:												/* SBB Ev,Gv */
		break;
	case 0x1a:												/* SBB Gb,Eb */
		break;
	case 0x1b:												/* SBB Gv,Ev */
		break;
	case 0x1c:												/* SBB AL,Ib */
		break;
	case 0x1d:												/* SBB eAX,Iv */
		break;
	case 0x1e:												/* PUSH DS */		
		break;
	case 0x1f:												/* POP DS */
		break;
	case 0x20:												/* AND Eb,Gb */
		break;
	case 0x21:												/* AND Ev,Gv */
		break;
	case 0x22:												/* AND Gb,Eb */
		break;
	case 0x23:												/* AND Gv,Ev */
		break;
	case 0x24:												/* AND AL,Ib */
		break;
	case 0x25:												/* AND eAX,Iv */
		break;
	case 0x26:												/* SEG ES: */
		break;
	case 0x27:												/* DAA */
		break;
	case 0x28:												/* SUB Eb,Gb */
		break;
	case 0x29:												/* SUB Ev,Gv */
		break;
	case 0x2a:												/* SUB Gb,Eb */
		break;
	case 0x2b:												/* SUB Gv,Ev */
		break;
	case 0x2c:												/* SUB AL,Ib */
		break;
	case 0x2d:												/* SUB eAX,Iv */
		break;
		break;
	case 0x2e:												/* SEG CS: */
		break;
	case 0x2f:												/* DAS */
		break;
	case 0x30:												/* XOR Eb,Gb */
		break;
	case 0x31:												/* XOR Ev,Gv */
	case 0x32:												/* XOR Gb,Eb */
	case 0x33:												/* XOR Gv,Ev */
	case 0x34:												/* XOR AL,Ib */
	case 0x35:												/* XOR eAX,Iv */
	case 0x36:												/* SEG SS: */
	case 0x37:												/* AAA */
	case 0x39:												/* CMP Ev,Gv */
	case 0x3a:												/* CMP Gb,Eb */
	case 0x3b:												/* CMP Gv,Ev */
	case 0x3c:												/* CMP AL,Ib */
	case 0x3d:												/* CMP eAX,Iv */
	case 0x3e:												/* SEG DS: */
	case 0x3f:												/* AAS */
	case 0x40:												/* INC eAX */
	case 0x41:												/* INC CX */
	case 0x42:												/* INC DX */
	case 0x43:												/* INC BX */
	case 0x44:												/* INC SP */
	case 0x45:												/* INC BP */
	case 0x46:												/* INC SI */
	case 0x47:												/* INC DI */
	case 0x48:												/* DEC eAX */
	case 0x49:												/* DEC CX */
	case 0x4a:												/* DEC DX */
	case 0x4b:												/* DEC BX */
	case 0x4c:												/* DEC SP */
	case 0x4d:												/* DEC BP */
	case 0x4e:												/* DEC SI */
	case 0x4f:												/* DEC DI */
		break;
/* PUSH reg */
	case 0x50:												/* PUSH eAX */
	case 0x51:												/* PUSH CX */
	case 0x52:												/* PUSH DX */
	case 0x53:												/* PUSH BX */
	case 0x54:												/* PUSH SP */
	case 0x55:												/* PUSH BP */
	case 0x56:												/* PUSH SI */
	case 0x57:												/* PUSH DI */
                /* PUSH -> SP=SP-4 (-2 for word); [SP]=reg; */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = SUB;
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = 4;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = REG_SP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;

		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = MOV;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = reg_table[inst[0] & 0x7].offset;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 1;
		instruction->dstA.value = REG_SP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
		break;
/* POP reg */
	case 0x58:												/* POP eAX */
	case 0x59:												/* POP CX */
	case 0x5a:												/* POP DX */
	case 0x5b:												/* POP BX */
	case 0x5c:												/* POP SP */
	case 0x5d:												/* POP BP */
	case 0x5e:												/* POP SI */
	case 0x5f:												/* POP DI */
                /* POP -> ES=[SP]; SP=SP+4 (+2 for word); */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = MOV;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = reg_table[inst[0] & 0x7].offset;
		instruction->dstA.size = 4;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 1;
		instruction->srcA.value = REG_SP;
		instruction->srcA.size = 4;
		instructions->instruction_number++;

		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = ADD;
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = 4;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = REG_SP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
		break;

	case 0x60:												/* PUSHA */
		break;
	case 0x61:												/* POPA */
		break;
	case 0x62:												/* BOUND */
		break;
	case 0x63:												/* ARPL Ev,Rw */
		break;
	case 0x64:												/* SEG FS: */
	case 0x65:												/* SEG GS: */
	case 0x66:												/* Operand Size Prefix */
	case 0x67:												/* Address Size Prefix */
	case 0x68:												/* PUSH Iv */
	case 0x69:												/* IMUL Gv,Ev,Iv */
		break;
	case 0x6a:												/* PUSH Ib */
		break;
	case 0x6b:												/* IMUL Gv,Ev,Ib */
		break;
	case 0x6c:												/* INSB */
	case 0x6d:												/* INSW */
	case 0x6e:												/* OUTSB */
	case 0x6f:												/* OUTSW */
	case 0x70:												/* JO */
	case 0x71:												/* JNO */
	case 0x72:												/* JB */
	case 0x73:												/* JNB */
	case 0x74:												/* JZ */
	case 0x75:												/* JNZ */
	case 0x76:												/* JBE */
	case 0x77:												/* JNBE */
	case 0x78:												/* JS */
	case 0x79:												/* JNS */
	case 0x7a:												/* JP */
	case 0x7b:												/* JNP */
	case 0x7c:												/* JL */
	case 0x7d:												/* JNL */
	case 0x7e:												/* JLE */
	case 0x7f:												/* JNLE */
	case 0x80:												/* Grpl Eb,Ib */
	case 0x82:												/* Grpl Eb,Ib Mirror instruction */
	case 0x81:												/* Grpl Ev,Iv */
		break;
	case 0x83:												/* Grpl Ev,Ix */
		half = rmb(instructions, inst, &reg);
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = immed_table[reg];
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = getbyte(&inst[instructions->bytes_used]); // Means get from rest of instruction
		instructions->bytes_used++;
		instruction->srcA.size = 4;
		if (!half) {
			instruction->dstA.store = 1;
			instruction->dstA.indirect = 1;
			instruction->dstA.value = REG_TMP;
			instruction->dstA.size = 4;
		}
		instructions->instruction_number++;
		break;
	case 0x84:												/* TEST Eb,Gb */
		break;
	case 0x85:												/* TEST Ev,Gv */
		break;
	case 0x86:												/* XCHG Eb,Gb */
	case 0x87:												/* XCHG Ev,Gv */
		break;
	case 0x88:												/* MOV Eb,Gb */
		dis_Ex_Gx(MOV, instructions, inst, &reg, 1);
		break;
	case 0x89:												/* MOV Ev,Gv */
		dis_Ex_Gx(MOV, instructions, inst, &reg, 4);
		break;
	case 0x8a:												/* MOV Gb,Eb */
		dis_Gx_Ex(MOV, instructions, inst, &reg, 1);
		break;
	case 0x8b:												/* MOV Gv,Ev */
		dis_Gx_Ex(MOV, instructions, inst, &reg, 4);
		break;
	case 0x8c:												/* Mov Ew,Sw */
		break;
	case 0x8d:												/* LEA Gv */
		half = rmb(instructions, inst, &reg);
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = MOV;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = reg_table[reg].offset;
		instruction->dstA.size = 4;
		if (!half) {
			instruction->srcA.store = 1;
			instruction->srcA.indirect = 0;
			instruction->srcA.value = REG_TMP;
			instruction->srcA.size = 4;
		}
		instructions->instruction_number++;
		break;
	case 0x8e:												/* MOV Sw,Ew */
	case 0x8f:												/* POP Ev */
	case 0x90:												/* NOP */
		break;
	case 0x91:												/* XCHG CX,eAX */
		break;
	case 0x92:												/* XCHG DX,eAX */
		break;
	case 0x93:												/* XCHG BX,eAX */
		break;
	case 0x94:												/* XCHG SP,eAX */
		break;
	case 0x95:												/* XCHG BP,eAX */
		break;
	case 0x96:												/* XCHG SI,eAX */
		break;
	case 0x97:												/* XCHG DI,eAX */
		break;
	case 0x98:												/* CBW */
	case 0x99:												/* CWD */
		break;
	case 0x9a:												/* CALL Ap */
	case 0x9b:												/* WAIT */
	case 0x9c:												/* PUSHF */
	case 0x9d:												/* POPF */
		break;
	case 0x9e:												/* SAHF */
		break;
	case 0x9f:												/* LAHF */
		break;
	case 0xa0:												/* MOV AL,Ob */
		break;
	case 0xa1:												/* MOV eAX,Ow */
		break;
	case 0xa2:												/* MOV Ob,AL */
		break;
	case 0xa3:												/* MOV Ow,eAX */
		break;
	case 0xa4:												/* MOVSB */
	case 0xa5:												/* MOVSW */
	case 0xa6:												/* CMPSB */
	case 0xa7:												/* CMPSW */
	case 0xa8:												/* TEST AL,Ib */
	case 0xa9:												/* TEST eAX,Iv */
	case 0xaa:												/* STOSB */
	case 0xab:												/* STOSW */
	case 0xac:												/* LODSB */
	case 0xad:												/* LODSW */
	case 0xae:												/* SCASB */
	case 0xaf:												/* SCASW */
	case 0xb0:												/* MOV AL,Ib */
	case 0xb1:												/* MOV CL,Ib */
	case 0xb2:												/* MOV DL,Ib */
	case 0xb3:												/* MOV BL,Ib */
	case 0xb4:												/* MOV AH,Ib */
	case 0xb5:												/* MOV CH,Ib */
	case 0xb6:												/* MOV DH,Ib */
	case 0xb7:												/* MOV BH,Ib */
	case 0xb8:												/* MOV eAX,Iv */
	case 0xb9:												/* MOV CX,Iv */
	case 0xba:												/* MOV DX,Iv */
	case 0xbb:												/* MOV BX,Iv */
	case 0xbc:												/* MOV SP,Iv */
	case 0xbd:												/* MOV BP.Iv */
	case 0xbe:												/* MOV SI,Iv */
	case 0xbf:												/* MOV DI,Iv */
	case 0xc0:												/* GRP2 Eb,Ib */
	case 0xc1:												/* GRP2 Ev,Ib */
		half = rmb(instructions, inst, &reg);
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = shift2_table[reg];
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = getbyte(&inst[instructions->bytes_used]); // Means get from rest of instruction
		instructions->bytes_used++;
		instruction->srcA.size = 4;
		if (!half) {
			instruction->dstA.store = 1;
			instruction->dstA.indirect = 1;
			instruction->dstA.value = REG_TMP;
			instruction->dstA.size = 4;
		}
		instructions->instruction_number++;
		break;
	case 0xc2:												/* RETN Iv */
	case 0xc3:												/* RETN */
		break;
	case 0xc4:												/* LES */
	case 0xc5:												/* LDS */
	case 0xc6:												/* MOV Eb,Ib */
	case 0xc7:												/* MOV EW,Iv */
	case 0xc8:												/* ENTER Iv,Ib */
	case 0xc9:												/* LEAVE */
		break;
	case 0xca:												/* RETF Iv */
	case 0xcb:												/* RETF */
	case 0xcc:												/* INT3 */
	case 0xcd:												/* INT Ib */	
		break;
	case 0xce:												/* INTO */
		break;
	case 0xcf:												/* IRET */
	case 0xd0:												/* GRP2 Eb,1 */
	case 0xd1:												/* GRP2 Ev,1 */
	case 0xd2:												/* GRP2 Eb,CL */
	case 0xd3:												/* GRP2 Ev,CL */
	case 0xd4:												/* AAM Ib */
	case 0xd5:												/* AAD Ib */
	case 0xd6:												/* SALC */
		break;
	case 0xd7:												/* XLAT */
		break;
	case 0xd8:												/* FPU ESC 0 */
	case 0xd9:												/* FPU ESC 1 */
	case 0xda:												/* FPU ESC 2 */
	case 0xdb:												/* FPU ESC 3 */
	case 0xdc:												/* FPU ESC 4 */
	case 0xdd:												/* FPU ESC 5 */
	case 0xde:												/* FPU ESC 6 */
	case 0xdf:												/* FPU ESC 7 */
		break;
	case 0xe0:												/* LOOPNZ */
		break;
	case 0xe1:												/* LOOPZ */
		break;
	case 0xe2:												/* LOOP */
		break;
	case 0xe3:												/* JCXZ */
	case 0xe4:												/* IN AL,Ib */
		break;
	case 0xe5:												/* IN eAX,Ib */
		break;
	case 0xe6:												/* OUT Ib,AL */
		break;
	case 0xe7:												/* OUT Ib,eAX */
		break;
	case 0xe8:												/* CALL Jw */
                /* PUSH -> SP=SP-4 (-2 for word); [SP]=IP; */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = SUB;
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = 4;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = REG_SP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;

		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = MOV;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = REG_IP;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 1;
		instruction->dstA.value = REG_SP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
		/* Fall through to JMP */	
	case 0xe9:												/* JMP Jw */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = JMP;
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = getdword(&inst[instructions->bytes_used]); // Means get from rest of instruction
		instructions->bytes_used+=4;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 1;
		instruction->dstA.value = REG_IP;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
		break;
	case 0xea:												/* JMP Ap */
	case 0xeb:												/* JMP Jb */
	case 0xec:												/* IN AL,DX */
	case 0xed:												/* IN eAX,DX */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = IN;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 1;
		instruction->srcA.value = REG_DX;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 0;
		instruction->dstA.value = REG_AX;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
		break;
	case 0xee:												/* OUT DX,AL */
	case 0xef:												/* OUT DX,eAX */
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = OUT;
		instruction->srcA.store = 1;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = REG_AX;
		instruction->srcA.size = 4;
		instruction->dstA.store = 1;
		instruction->dstA.indirect = 1;
		instruction->dstA.value = REG_DX;
		instruction->dstA.size = 4;
		instructions->instruction_number++;
		break;
	case 0xf0:												/* LOCK */
		break;
	case 0xf2:												/* REPNZ */
		break;		
	case 0xf3:												/* REPZ */
		break;		
	case 0xf4:												/* HLT */
	case 0xf5:												/* CMC */
		break;
	case 0xf6:												/* GRP3 Eb ,Ib: */
	case 0xf7:												/* GRP3 Ev ,Iv: */
	case 0xf8:												/* CLC */
		break;
	case 0xf9:												/* STC */
		break;
	case 0xfa:												/* CLI */
		break;
	case 0xfb:												/* STI */
		break;
	case 0xfc:												/* CLD */
		break;
	case 0xfd:												/* STD */
		break;
	case 0xfe:												/* GRP4 Eb */
	case 0xff:												/* GRP5 Ev */
#if 0
		half = rmb(instructions, inst, &reg);
		instruction = &instructions->instruction[instructions->instruction_number];	
		instruction->opcode = group5_table[reg];
		instruction->srcA.store = 0;
		instruction->srcA.indirect = 0;
		instruction->srcA.value = getbyte(&inst[instructions->bytes_used]); // Means get from rest of instruction
		instructions->bytes_used++;
		instruction->srcA.size = 4;
		if (!half) {
			instruction->dstA.store = 1;
			instruction->dstA.indirect = 1;
			instruction->dstA.value = REG_TMP;
			instruction->dstA.size = 4;
		}
		instructions->instruction_number++;
#endif
		break;
	}

	return 0;
}



int main(int argc, char *argv[]) {
//8b 74 24 28
	int n=0;
	int offset=0;
	instruction_t *instruction;
	printf("sizeof inst=%d\n",sizeof(inst));
	for (n=0;n<sizeof(inst);n++) {
		printf(" 0x%02x",inst[n]);
	}
	printf("\n");
	instructions.bytes_used=0;
	for(offset=0;offset<sizeof(inst);offset+=instructions.bytes_used) {
		instructions.instruction_number=0;
		instructions.bytes_used=0;
	        disassemble(&instructions, inst+offset);
		printf("bytes used=%d\n",instructions.bytes_used);
		for (n=0;n<instructions.bytes_used;n++) {
			printf(" 0x%02x",inst[n+offset]);
		}
		printf("\n");
		printf("instruction_number=%d\n",instructions.instruction_number);
		for (n=0;n<instructions.instruction_number;n++) {
			instruction = &instructions.instruction[n];	
			printf("Instruction %d:%s",
				n,
				opcode_table[instruction->opcode]);
			if (instruction->srcA.indirect) {
				printf(" %s[%s0x%x],",
					size_table[instruction->srcA.size],
					store_table[instruction->srcA.store],
					instruction->srcA.value);
			} else {
				printf(" %s%s0x%x,",
				size_table[instruction->srcA.size],
				store_table[instruction->srcA.store],
				instruction->srcA.value);
			}
			if (instruction->dstA.indirect) {
				printf(" %s[%s0x%x]\n",
					size_table[instruction->dstA.size],
					store_table[instruction->dstA.store],
					instruction->dstA.value);
			} else {
				printf(" %s%s0x%x\n",
				size_table[instruction->dstA.size],
				store_table[instruction->dstA.store],
				instruction->dstA.value);
			}
		}
	}

	printf("test1\n");
	return 0;
}
