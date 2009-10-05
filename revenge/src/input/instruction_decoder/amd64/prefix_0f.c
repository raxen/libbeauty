#include <dis.h>
#include "internal.h"

int prefix_0f(struct rev_eng *handle, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset) {
	int half;
	uint8_t reg=0;
	uint64_t size = 8;
	instruction_t *instruction;
	switch (base_address[offset + dis_instructions->bytes_used++]) {
	case 0x00:												/* GRP 6 Exxx */
	case 0x01:												/* Group 7 Ev */
	case 0x02:												/* LAR Gv,Ev */
	case 0x03:												/* LSL Gv,Ev */
	case 0x06:												/* CLTS */
		break;
	case 0x20:												/* MOV Rd.CRx */
		break;
	case 0x21:												/* MOV Rd,DRx */
		break;
	case 0x22:												/* MOV CRx,Rd */
		break;
	case 0x23:												/* MOV DRx,Rd */
		break;
	case 0x80:												/* JO */
	case 0x81:												/* JNO */
	case 0x82:												/* JB */
	case 0x83:												/* JNB */
	case 0x84:												/* JZ */
	case 0x85:												/* JNZ */
	case 0x86:												/* JBE */
	case 0x87:												/* JNBE */
	case 0x88:												/* JS */
	case 0x89:												/* JNS */
	case 0x8a:												/* JP */
	case 0x8b:												/* JNP */
	case 0x8c:												/* JL */
	case 0x8d:												/* JNL */
	case 0x8e:												/* JLE */
	case 0x8f:												/* JNLE */
	case 0x90:												/* SETO */
	case 0x91:												/* SETNO */
	case 0x92:												/* SETB */
	case 0x93:												/* SETNB */
	case 0x94:												/* SETZ */
	case 0x95:												/* SETNZ */
	case 0x96:												/* SETBE */
	case 0x97:												/* SETNBE */
	case 0x98:												/* SETS */
	case 0x99:												/* SETNS */
	case 0x9a:												/* SETP */
	case 0x9b:												/* SETNP */
	case 0x9c:												/* SETL */
	case 0x9d:												/* SETNL */
	case 0x9e:												/* SETLE */
	case 0x9f:												/* SETNLE */
	case 0xa0:												/* PUSH FS */		
	case 0xa1:												/* POP FS */		
	case 0xa2:												/* CPUID */
	case 0xa3:												/* BT Ev,Gv */
	case 0xa4:												/* SHLD Ev,Gv,Ib */
	case 0xa5:												/* SHLD Ev,Gv,CL */
	case 0xa8:												/* PUSH GS */		
	case 0xa9:												/* POP GS */		
	case 0xab:												/* BTS Ev,Gv */
	case 0xac:												/* SHRD Ev,Gv,Ib */
		break;
	case 0xad:												/* SHRD Ev,Gv,CL */
		break;
	case 0xaf:												/* IMUL Gv,Ev */
		break;
	case 0xb2:												/* LSS Ev */
	case 0xb3:												/* BTR Ev,Gv */
	case 0xb4:												/* LFS Ev */
	case 0xb5:												/* LGS Ev */
	case 0xb6:												/* MOVZX Gv,Eb */
		half = rmb(handle, dis_instructions, base_address, offset, size, &reg);
		instruction = &dis_instructions->instruction[dis_instructions->instruction_number];	
		instruction->opcode = MOV;
		instruction->dstA.store = STORE_REG;
		instruction->dstA.indirect = 0;
		instruction->dstA.indirect_size = 8;
		instruction->dstA.index = reg_table[reg].offset;
		instruction->dstA.value_size = size;
		if (!half) {
			instruction->srcA.store = STORE_REG;
			instruction->srcA.indirect = 1;
			instruction->srcA.indirect_size = 8;
			instruction->srcA.index = REG_TMP1;
		}
		instruction->srcA.value_size = 1;
		dis_instructions->instruction_number++;
		break;
	case 0xb7:												/* MOVZX Gv,Ev */
		half = rmb(handle, dis_instructions, base_address, offset, size, &reg);
		instruction = &dis_instructions->instruction[dis_instructions->instruction_number];	
		instruction->opcode = MOV;
		instruction->dstA.store = STORE_REG;
		instruction->dstA.indirect = 0;
		instruction->dstA.indirect_size = 8;
		instruction->dstA.index = reg_table[reg].offset;
		instruction->dstA.value_size = size;
		if (!half) {
			instruction->srcA.store = STORE_REG;
			instruction->srcA.indirect = 1;
			instruction->srcA.indirect_size = 8;
			instruction->srcA.index = REG_TMP1;
		}
		instruction->srcA.value_size = 2;
		dis_instructions->instruction_number++;
		break;


	case 0xbf:												/* MOVSX Gv,Ev */
	case 0xba:												/* GRP8 Ev,Ib */
	case 0xbb:												/* BTC Ev,Gv */
	case 0xbc:												/* BSF Gv,Ev */
	case 0xbd:												/* BSR Gv,Ev */
	case 0xbe:												/* MOVSX Gv,Eb */
	case 0xc8:												/* BSWAP EAX */
	case 0xc9:												/* BSWAP ECX */
	case 0xca:												/* BSWAP EDX */
	case 0xcb:												/* BSWAP EBX */
	case 0xcc:												/* BSWAP ESP */
	case 0xcd:												/* BSWAP EBP */
	case 0xce:												/* BSWAP ESI */
	case 0xcf:												/* BSWAP EDI */
		break;

	};
	return 0;
}
		
