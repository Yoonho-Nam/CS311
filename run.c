/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	instruction* instr_curr = get_inst_info(CURRENT_STATE.PC);
    CURRENT_STATE.PC += 4;
    switch(OPCODE(instr_curr)) {
	    //I format
	    case 0x9:		//ADDIU
            CURRENT_STATE.REGS[RT(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] + (int32_t)IMM(instr_curr);
            break;
	    case 0xc:		//ANDI
            CURRENT_STATE.REGS[RT(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] & ((int32_t)IMM(instr_curr) & 0x0000ffff);
            break;
        case 0xf:		//LUI
            CURRENT_STATE.REGS[RT(instr_curr)] = (int32_t)IMM(instr_curr) << 16;
            break;
	    case 0xd:		//ORI
            CURRENT_STATE.REGS[RT(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] | ((int32_t)IMM(instr_curr) & 0x0000ffff);
            break;
        case 0xb:		//SLTIU
            CURRENT_STATE.REGS[RT(instr_curr)] = (CURRENT_STATE.REGS[RS(instr_curr)] < (int32_t)IMM(instr_curr))? 1 : 0;
            break;
	    case 0x23:		//LW	
            CURRENT_STATE.REGS[RT(instr_curr)] = mem_read_32(CURRENT_STATE.REGS[RS(instr_curr)] + (int32_t)IMM(instr_curr));
            break;
	    case 0x2b:		//SW
            mem_write_32(CURRENT_STATE.REGS[RS(instr_curr)] + (int32_t)IMM(instr_curr), CURRENT_STATE.REGS[RT(instr_curr)]);
            break;
	    case 0x4:		//BEQ
            if (CURRENT_STATE.REGS[RT(instr_curr)] == CURRENT_STATE.REGS[RS(instr_curr)])
                CURRENT_STATE.PC += (int32_t)IMM(instr_curr) << 2;
            break;
	    case 0x5:		//BNE
            if (CURRENT_STATE.REGS[RT(instr_curr)] != CURRENT_STATE.REGS[RS(instr_curr)])
                CURRENT_STATE.PC += (int32_t)IMM(instr_curr) << 2;
            break;

    	//R format
	    case 0x0:		//ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
            switch(FUNC(instr_curr)) {
                case 0x21:
                    CURRENT_STATE.REGS[RD(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] + CURRENT_STATE.REGS[RT(instr_curr)];
                    break;
                case 0x24:
                    CURRENT_STATE.REGS[RD(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] & CURRENT_STATE.REGS[RT(instr_curr)];
                    break;
                case 0x27:
                    CURRENT_STATE.REGS[RD(instr_curr)] = ~(CURRENT_STATE.REGS[RS(instr_curr)] | CURRENT_STATE.REGS[RT(instr_curr)]);
                    break;
                case 0x25:
                    CURRENT_STATE.REGS[RD(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] | CURRENT_STATE.REGS[RT(instr_curr)];
                    break;
                case 0x2b:
                    CURRENT_STATE.REGS[RD(instr_curr)] = (CURRENT_STATE.REGS[RS(instr_curr)] < CURRENT_STATE.REGS[RT(instr_curr)]) ? 1:0;
                    break;
                case 0x00:
                    CURRENT_STATE.REGS[RD(instr_curr)] = CURRENT_STATE.REGS[RT(instr_curr)] << SHAMT(instr_curr);
                    break;
                case 0x02:
                    CURRENT_STATE.REGS[RD(instr_curr)] = CURRENT_STATE.REGS[RT(instr_curr)] >> SHAMT(instr_curr);
                    break;
                case 0x23:
                    CURRENT_STATE.REGS[RD(instr_curr)] = CURRENT_STATE.REGS[RS(instr_curr)] - CURRENT_STATE.REGS[RT(instr_curr)];
                    break;
                case 0x8:
                    CURRENT_STATE.PC = CURRENT_STATE.REGS[RS(instr_curr)];
                    break;
                default:
		            printf("Not available function code\n");
		            assert(0);
            }
            break;

    	//J format
	    case 0x2:		//J
            CURRENT_STATE.PC = (TARGET(instr_curr) << 2) | (CURRENT_STATE.PC & 0xf0000000);
            break;
	    case 0x3:		//JAL
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            CURRENT_STATE.PC = (TARGET(instr_curr) << 2) | (CURRENT_STATE.PC & 0xf0000000);
            break;

	    default:
		printf("Not available instruction\n");
		assert(0);
    }

    if (CURRENT_STATE.PC >= MEM_TEXT_START + NUM_INST * 4) RUN_BIT = FALSE;
}
