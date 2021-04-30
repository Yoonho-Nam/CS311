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
instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

static int jump_bit = 0;
static int load_use_bit = 0;

static void pipeline_IF() {
    if (CURRENT_STATE.PIPE_STALL[0] == TRUE) {
        CURRENT_STATE.PIPE_STALL[1] = TRUE;
        CURRENT_STATE.PIPE_STALL[0] = FALSE;
        if (!jump_bit) CURRENT_STATE.PIPE[0] = 0;
        if (jump_bit) {
            CURRENT_STATE.PIPE[0] = CURRENT_STATE.PC;
            CURRENT_STATE.PC = CURRENT_STATE.JUMP_PC;
            jump_bit = 0;
        }
        CURRENT_STATE.IF_ID_INST = 0;
        CURRENT_STATE.IF_ID_NPC = 0;
        return;
    }

    if (load_use_bit == 1) {
        load_use_bit = 0;
        return;
    }

    if (load_use_bit == 2) load_use_bit--;

    CURRENT_STATE.PIPE[0] = CURRENT_STATE.PC;

    if (CURRENT_STATE.PIPE[0] >= MEM_TEXT_START + NUM_INST * 4) {
        CURRENT_STATE.IF_ID_INST = 0;
        CURRENT_STATE.PIPE[0] = 0;
        return;
    }

    instruction *inst = get_inst_info(CURRENT_STATE.PIPE[0]);
    CURRENT_STATE.IF_ID_INST = (uint32_t) inst;
    CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PIPE[0] + 4;
    CURRENT_STATE.PC += 4;
}

static void pipeline_ID() {
    if (CURRENT_STATE.PIPE_STALL[1] == TRUE) {
        CURRENT_STATE.PIPE_STALL[2] = TRUE;
        CURRENT_STATE.PIPE_STALL[1] = FALSE;
        CURRENT_STATE.PIPE[1] = 0;
    	CURRENT_STATE.ID_EX_NPC = 0;
        CURRENT_STATE.ID_EX_REG1 = 0;
        CURRENT_STATE.ID_EX_REG2 = 0;
        CURRENT_STATE.ID_EX_RS = 0;
        CURRENT_STATE.ID_EX_RT = 0;
        CURRENT_STATE.ID_EX_IMM = 0;
        CURRENT_STATE.ID_EX_DEST = 0;
        CURRENT_STATE.ID_EX_SHAMT = 0;
        CURRENT_STATE.ID_EX_FUNC = 0;
        CURRENT_STATE.ID_EX_OPCODE = 0;
        return;
    }

    if (load_use_bit == 1) return;

    CURRENT_STATE.PIPE[1] = CURRENT_STATE.PIPE[0];
    if (CURRENT_STATE.PIPE[1] == 0) return;

    instruction *inst = (instruction *) CURRENT_STATE.IF_ID_INST;
    CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.IF_ID_NPC;
    CURRENT_STATE.ID_EX_OPCODE = OPCODE(inst);

    switch(OPCODE(inst)) {
        //Type I
        case 0x9:		//(0x001001)ADDIU
        case 0xc:		//(0x001100)ANDI
        case 0xf:		//(0x001111)LUI	
        case 0xd:		//(0x001101)ORI
        case 0xb:		//(0x001011)SLTIU
        case 0x23:		//(0x100011)LW	
        case 0x2b:		//(0x101011)SW
        case 0x4:		//(0x000100)BEQ
        case 0x5:		//(0x000101)BNE
            CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[RS(inst)];
            CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[RT(inst)];
            CURRENT_STATE.ID_EX_RS = RS(inst);
            CURRENT_STATE.ID_EX_RT = RT(inst);
            CURRENT_STATE.ID_EX_IMM = (int32_t) IMM(inst);
            CURRENT_STATE.ID_EX_DEST = RT(inst);
            CURRENT_STATE.ID_EX_SHAMT = 0;
            CURRENT_STATE.ID_EX_FUNC = 0;
            break;

	    //TYPE R
	    case 0x0:
            CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[RS(inst)];
            CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[RT(inst)];
            CURRENT_STATE.ID_EX_RS = RS(inst);
            CURRENT_STATE.ID_EX_RT = RT(inst);
            CURRENT_STATE.ID_EX_DEST = RD(inst);
            CURRENT_STATE.ID_EX_SHAMT = SHAMT(inst);
            CURRENT_STATE.ID_EX_FUNC = FUNC(inst);
            CURRENT_STATE.ID_EX_IMM = 0;
            if (FUNC(inst) == 0x8) {
                CURRENT_STATE.PIPE_STALL[0] = TRUE;
                jump_bit = 1;
                CURRENT_STATE.JUMP_PC = CURRENT_STATE.REGS[RS(inst)];
                CURRENT_STATE.ID_EX_REG1 = 0;
                CURRENT_STATE.ID_EX_REG2 = 0;
                CURRENT_STATE.ID_EX_RS = 0;
                CURRENT_STATE.ID_EX_RT = 0;
                CURRENT_STATE.ID_EX_IMM = 0;
                CURRENT_STATE.ID_EX_DEST = 0;
                CURRENT_STATE.ID_EX_SHAMT = 0;
                CURRENT_STATE.ID_EX_FUNC = 0;
            }
            break;

        //TYPE J
        case 0x2:
            CURRENT_STATE.PIPE_STALL[0] = TRUE;
            jump_bit = 1;
            CURRENT_STATE.JUMP_PC = (TARGET(inst) << 2) | (CURRENT_STATE.IF_ID_NPC & 0xf0000000);
            CURRENT_STATE.ID_EX_REG1 = 0;
            CURRENT_STATE.ID_EX_REG2 = 0;
            CURRENT_STATE.ID_EX_RS = 0;
            CURRENT_STATE.ID_EX_RT = 0;
            CURRENT_STATE.ID_EX_IMM = 0;
            CURRENT_STATE.ID_EX_DEST = 0;
            CURRENT_STATE.ID_EX_SHAMT = 0;
            CURRENT_STATE.ID_EX_FUNC = 0;
            break;
        case 0x3:
            CURRENT_STATE.PIPE_STALL[0] = TRUE;
            jump_bit = 1;
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC;
            CURRENT_STATE.JUMP_PC = (TARGET(inst) << 2) | (CURRENT_STATE.IF_ID_NPC & 0xf0000000);
            CURRENT_STATE.ID_EX_REG1 = 0;
            CURRENT_STATE.ID_EX_REG2 = 0;
            CURRENT_STATE.ID_EX_RS = 0;
            CURRENT_STATE.ID_EX_RT = 0;
            CURRENT_STATE.ID_EX_IMM = 0;
            CURRENT_STATE.ID_EX_DEST = 0;
            CURRENT_STATE.ID_EX_SHAMT = 0;
            CURRENT_STATE.ID_EX_FUNC = 0;
            break;

        default:
            printf("Not available instruction\n");
            assert(0);
    }
}

static void pipeline_EX() {
    if (CURRENT_STATE.PIPE_STALL[2] == TRUE) {
        CURRENT_STATE.PIPE_STALL[3] = TRUE;
        CURRENT_STATE.PIPE_STALL[2] = FALSE;
        CURRENT_STATE.PIPE[2] = 0;
        CURRENT_STATE.EX_MEM_NPC = 0;
        CURRENT_STATE.EX_MEM_ALU_OUT = 0;
        CURRENT_STATE.EX_MEM_W_VALUE = 0;
        CURRENT_STATE.EX_MEM_BR_TARGET = 0;
        CURRENT_STATE.EX_MEM_BR_TAKE = 0;
        CURRENT_STATE.EX_MEM_DEST = 0;
        CURRENT_STATE.EX_MEM_OPCODE = 0;
        CURRENT_STATE.EX_MEM_RT = 0;
        return;
    }

    CURRENT_STATE.PIPE[2] = CURRENT_STATE.PIPE[1];
    if (CURRENT_STATE.PIPE[2] == 0) return;

    if (CURRENT_STATE.ID_EX_OPCODE == 0x23 && CURRENT_STATE.IF_ID_INST != 0) {
        instruction *inst = (instruction *) CURRENT_STATE.IF_ID_INST;
        if (CURRENT_STATE.ID_EX_RT == RS(inst) || CURRENT_STATE.ID_EX_RT == RT(inst)) {
            CURRENT_STATE.PIPE_STALL[2] = TRUE;
            load_use_bit = 2;
        }
    }

    CURRENT_STATE.EX_MEM_NPC = CURRENT_STATE.ID_EX_NPC;
    CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_DEST;
    CURRENT_STATE.EX_MEM_OPCODE = CURRENT_STATE.ID_EX_OPCODE;
    CURRENT_STATE.EX_MEM_W_VALUE = 0;
    CURRENT_STATE.EX_MEM_BR_TARGET = 0;
    CURRENT_STATE.EX_MEM_BR_TAKE = 0;
    CURRENT_STATE.EX_MEM_ALU_OUT = 0;
    CURRENT_STATE.EX_MEM_RT = CURRENT_STATE.ID_EX_RT;

    switch(CURRENT_STATE.ID_EX_OPCODE) {
	    //I format
	    case 0x9:		//ADDIU
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
            break;
	    case 0xc:		//ANDI
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 & (CURRENT_STATE.ID_EX_IMM & 0x0000ffff);
            break;
        case 0xf:		//LUI
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_IMM << 16;
            break;
	    case 0xd:		//ORI
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 | (CURRENT_STATE.ID_EX_IMM & 0x0000ffff);
            break;
        case 0xb:		//SLTIU
            CURRENT_STATE.EX_MEM_ALU_OUT = (CURRENT_STATE.ID_EX_REG1 < CURRENT_STATE.ID_EX_IMM)? 1 : 0;
            break;
	    case 0x23:		//LW
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
            break;
	    case 0x2b:		//SW
            CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_IMM;
            CURRENT_STATE.EX_MEM_W_VALUE = CURRENT_STATE.ID_EX_REG2;
            break;
	    case 0x4:		//BEQ
            CURRENT_STATE.EX_MEM_BR_TAKE = (CURRENT_STATE.ID_EX_REG1 == CURRENT_STATE.ID_EX_REG2)? 1 : 0;
            CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.ID_EX_NPC + (CURRENT_STATE.ID_EX_IMM << 2);
            break;
	    case 0x5:		//BNE
            CURRENT_STATE.EX_MEM_BR_TAKE = (CURRENT_STATE.ID_EX_REG1 != CURRENT_STATE.ID_EX_REG2)? 1 : 0;
            CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.ID_EX_NPC + (CURRENT_STATE.ID_EX_IMM << 2);
            break;

    	//R format
	    case 0x0:		//ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
            switch(CURRENT_STATE.ID_EX_FUNC) {
                case 0x21:
                    CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 + CURRENT_STATE.ID_EX_REG2;
                    break;
                case 0x24:
                    CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 & CURRENT_STATE.ID_EX_REG2;
                    break;
                case 0x27:
                    CURRENT_STATE.EX_MEM_ALU_OUT = ~(CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_REG2);
                    break;
                case 0x25:
                    CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 | CURRENT_STATE.ID_EX_REG2;
                    break;
                case 0x2b:
                    CURRENT_STATE.EX_MEM_ALU_OUT = (CURRENT_STATE.ID_EX_REG1 < CURRENT_STATE.ID_EX_REG2) ? 1:0;
                    break;
                case 0x00:
                    CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG2 << CURRENT_STATE.ID_EX_SHAMT;
                    break;
                case 0x02:
                    CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG2 >> CURRENT_STATE.ID_EX_SHAMT;
                    break;
                case 0x23:
                    CURRENT_STATE.EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_REG1 - CURRENT_STATE.ID_EX_REG2;
                    break;
                case 0x8:
                    break;
                default:
		            printf("Not available function code\n");
		            assert(0);
            }
            break;

    	//J format
	    case 0x2:		//J
            break;
	    case 0x3:		//JAL
            break;

	    default:
		printf("Not available instruction\n");
		assert(0);
    }
}

static void pipeline_MEM() {
    if (CURRENT_STATE.PIPE_STALL[3] == TRUE) {
        CURRENT_STATE.PIPE_STALL[4] = TRUE;
        CURRENT_STATE.PIPE_STALL[3] = FALSE;
        CURRENT_STATE.PIPE[3] = 0;
        CURRENT_STATE.MEM_WB_ALU_OUT = 0;
        CURRENT_STATE.MEM_WB_MEM_OUT = 0;
        CURRENT_STATE.MEM_WB_DEST = 0;
        CURRENT_STATE.MEM_WB_OPCODE = 0;
        CURRENT_STATE.MEM_WB_RT = 0;
        return;
    }

    CURRENT_STATE.PIPE[3] = CURRENT_STATE.PIPE[2];
    if (CURRENT_STATE.PIPE[3] == 0) return;

    if (CURRENT_STATE.EX_MEM_OPCODE == 0x4 || CURRENT_STATE.EX_MEM_OPCODE == 0x5) {
        CURRENT_STATE.MEM_WB_ALU_OUT = 0;
        CURRENT_STATE.MEM_WB_MEM_OUT = 0;
        CURRENT_STATE.MEM_WB_DEST = 0;
        CURRENT_STATE.MEM_WB_RT = 0;
        CURRENT_STATE.MEM_WB_OPCODE = CURRENT_STATE.EX_MEM_OPCODE;
        if (CURRENT_STATE.EX_MEM_BR_TAKE) {
            CURRENT_STATE.PC = CURRENT_STATE.EX_MEM_BR_TARGET;
            CURRENT_STATE.PIPE_STALL[0] = TRUE;
            CURRENT_STATE.PIPE_STALL[1] = TRUE;
            CURRENT_STATE.PIPE_STALL[2] = TRUE;
        }
        return;
    }

    CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
    CURRENT_STATE.MEM_WB_MEM_OUT = 0;
    CURRENT_STATE.MEM_WB_OPCODE = CURRENT_STATE.EX_MEM_OPCODE;
    CURRENT_STATE.MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.MEM_WB_RT = CURRENT_STATE.EX_MEM_RT;

    if (CURRENT_STATE.EX_MEM_OPCODE == 0x2b) mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.EX_MEM_W_VALUE);
    if (CURRENT_STATE.EX_MEM_OPCODE == 0x23) CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);

    switch (CURRENT_STATE.MEM_WB_OPCODE) {
        case 0x0:
        case 0x9:		//(0x001001)ADDIU
        case 0xc:		//(0x001100)ANDI
        case 0xf:		//(0x001111)LUI	
        case 0xd:		//(0x001101)ORI
        case 0xb:       //(0x001011)SLTIU
            if (CURRENT_STATE.EX_MEM_DEST != 0 && CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_RS)
                CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_ALU_OUT;
            if (CURRENT_STATE.EX_MEM_DEST != 0 && CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.ID_EX_RT)
                CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_ALU_OUT;
            break;
    }
}

static int count = 0;

static void pipeline_WB() {
    if (CURRENT_STATE.PIPE_STALL[4] == TRUE) {
        CURRENT_STATE.PIPE_STALL[4] = FALSE;
        CURRENT_STATE.PIPE[4] = 0;
        return;
    }

    CURRENT_STATE.PIPE[4] = CURRENT_STATE.PIPE[3];
    if (CURRENT_STATE.PIPE[4] == 0) return;

    switch (CURRENT_STATE.MEM_WB_OPCODE) {
        case 0x0:
        case 0x9:		//(0x001001)ADDIU
        case 0xc:		//(0x001100)ANDI
        case 0xf:		//(0x001111)LUI	
        case 0xd:		//(0x001101)ORI
        case 0xb:		//(0x001011)SLTIU
            CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_ALU_OUT;
            break;
        case 0x23:
            CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_MEM_OUT;
            break;
    }

    if (CURRENT_STATE.MEM_WB_OPCODE == 0x23 && CURRENT_STATE.EX_MEM_OPCODE == 0x2b && CURRENT_STATE.MEM_WB_RT == CURRENT_STATE.EX_MEM_RT)
        CURRENT_STATE.EX_MEM_W_VALUE = CURRENT_STATE.MEM_WB_MEM_OUT;

    switch (CURRENT_STATE.MEM_WB_OPCODE) {
        case 0x0:
        case 0x9:		//(0x001001)ADDIU
        case 0xc:		//(0x001100)ANDI
        case 0xf:		//(0x001111)LUI	
        case 0xd:		//(0x001101)ORI
        case 0xb:       //(0x001011)SLTIU
        case 0x23:
            if (CURRENT_STATE.MEM_WB_DEST != 0 && CURRENT_STATE.EX_MEM_DEST != CURRENT_STATE.ID_EX_RS && CURRENT_STATE.MEM_WB_DEST == CURRENT_STATE.ID_EX_RS) {
                CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_ALU_OUT;
                if (CURRENT_STATE.MEM_WB_OPCODE == 0x23) CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_MEM_OUT;
            }
            if (CURRENT_STATE.MEM_WB_DEST != 0 && CURRENT_STATE.EX_MEM_DEST != CURRENT_STATE.ID_EX_RT && CURRENT_STATE.MEM_WB_DEST == CURRENT_STATE.ID_EX_RT) {
                CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_ALU_OUT;
                if (CURRENT_STATE.MEM_WB_OPCODE == 0x23) CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_MEM_OUT;
            }
            break;
    }

    if (CURRENT_STATE.PIPE[4] + 4 >= MEM_TEXT_START + NUM_INST * 4) RUN_BIT = FALSE;
    count++;
    if (count == 100) RUN_BIT = FALSE;
}


/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	/** Your implementation here */
    pipeline_WB();
    pipeline_MEM();
    pipeline_EX();
    pipeline_ID();
    pipeline_IF();
}
