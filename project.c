#include "spimcore.h"

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned* ALUresult, char* Zero)
{
    switch (ALUControl)
    {
    case 0: // sum
        *ALUresult = A + B;
        if (*ALUresult == 0)
        {
            *Zero = 1;
        }
        else
        {
            *Zero = 0;
        }
        break;
    case 1: // sub
        *ALUresult = A - B;
        if (*ALUresult == 0)
        {
            *Zero = 1;
        }
        else
        {
            *Zero = 0;
        }
        break;
    case 2: // A < B
        if (A < B)
        {
            *Zero = 1;
            *ALUresult = 1;
        }
        else
            *Zero = 0;
        *ALUresult = 0;
        break;
    case 3: // A < B (unsigned)
        if ((unsigned)A < (unsigned)B)
        {
            *Zero = 1;
            *ALUresult = 1;
        }
        else
            *Zero = 0;
        *ALUresult = 0;
        break;
    case 4: // and
        *ALUresult = A & B;
        *Zero = 0;
        break;
    case 5: // or
        *ALUresult = A | B;
        *Zero = 0;
        break;
    case 6: // shift left by 16 bits
        *ALUresult = B << 16;
        *Zero = 0;
        break;
    case 7: // not
        *ALUresult = ~A;
        *Zero = 0;
        break;
    }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned* Mem, unsigned* instruction)
{
    if (PC % 4 != 0)
    {
        return 1; // Halt: PC not aligned
    }

    if (PC < 0 || PC >= (65536 >> 2))
    {
        return 1; // Halt: PC out of range
    }

    *instruction = Mem[PC >> 2]; // fetch instruction from memory

    return 0; // No Halt
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned* op, unsigned* r1, unsigned* r2, unsigned* r3, unsigned* funct, unsigned* offset, unsigned* jsec)
{
    *op = (instruction >> 26) & 0x3F; // get opcode (6 bits) [bits 31-26], 0x3F = 111111 which keeps the last 6 bits
    *r1 = (instruction >> 21) & 0x1F; // get rs (5 bits) [bits 25-21], 0x1F = 11111 which keeps the last 5 bits
    *r2 = (instruction >> 16) & 0x1F; // get rt (5 bits) [bits 20-16], 0x1F = 11111 which keeps the last 5 bits
    *r3 = (instruction >> 11) & 0x1F; // get rd (5 bits) [bits 15-11], 0x1F = 11111 which keeps the last 5 bits
    *funct = instruction & 0x3F;      // get funct (6 bits) [bits 5-0], 0x3F = 111111 which keeps the last 6 bits
    *offset = instruction & 0xFFFF;   // get offset (16 bits) [bits 15-0], 0xFFFF = 1111111111111111 which keeps the last 16 bits
    *jsec = instruction & 0x03FFFFFF; // get jump address (26 bits) [bits 25-0], 0x03FFFFFF = 000000000000000000000000001111111111111111 which keeps the last 26 bits
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls* controls)
{
    // Initialize control signals based on the opcode
    switch (op)
    {
    case 0: // R-type instruction
        controls->RegDst = 1;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 7; // 111 for R-type
        controls->MemWrite = 0;
        controls->ALUSrc = 0;
        controls->RegWrite = 1;
        break;

    case 2: // J-type instruction (Jump)
        controls->RegDst = 2; // Don't care
        controls->Jump = 1;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 2; // Don't care
        controls->ALUOp = 2;    // Don't care
        controls->MemWrite = 0;
        controls->ALUSrc = 2; // Don't care
        controls->RegWrite = 0;
        break;

    case 4: // BEQ instruction
        controls->RegDst = 2; // Don't care
        controls->Jump = 0;
        controls->Branch = 1;
        controls->MemRead = 0;
        controls->MemtoReg = 2; // Don't care
        controls->ALUOp = 1;    // 001 for subtraction
        controls->MemWrite = 0;
        controls->ALUSrc = 0;
        controls->RegWrite = 0;
        break;

    case 8: // addi instruction
        controls->RegDst = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 0; // ALU performs addition
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
        break;

    case 10: // slti instruction
        controls->RegDst = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 2; // ALU performs set less than
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
        break;

    case 11: // sltiu instruction
        controls->RegDst = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 3; // ALU performs set less than unsigned
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
        break;

    case 15: // lui instruction
        controls->RegDst = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 0;
        controls->ALUOp = 6; // ALU performs shift left by 16 bits
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
        break;

    case 35: // lw instruction
        controls->RegDst = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 1;
        controls->MemtoReg = 1; // Load data from memory to register
        controls->ALUOp = 0;    // ALU performs addition
        controls->MemWrite = 0;
        controls->ALUSrc = 1;
        controls->RegWrite = 1;
        break;

    case 43:// sw instruction
        controls->RegDst = 2; // Don't care
        controls->Jump = 0;
        controls->Branch = 0;
        controls->MemRead = 0;
        controls->MemtoReg = 2; // Don't care
        controls->ALUOp = 0;    // ALU performs addition
        controls->MemWrite = 1; // Write to memory
        controls->ALUSrc = 1;
        controls->RegWrite = 0;
        break;

    default:
        return 1; // Invalid opcode, halt the program
    }

    return 0; // No halt
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned* Reg, unsigned* data1, unsigned* data2)
{
    *data1 = Reg[r1]; // Read data from registers r1 and r2
    *data2 = Reg[r2]; // Read data from registers r1 and r2
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned* extended_value)
{
    // Assign the sign-extended value of offset to extended_value
    // Get msb, if 1 extend with negative values (1s) and 0s if positive
    if (offset & (1 << 15))
    {
        *extended_value = offset | 0xFFFF0000; // Extend with 1s
    }
    else
    {
        *extended_value = offset | 0x0000FFFF; // Extend with 0s
    }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned* ALUresult, char* Zero)
{
    // Determining whether to use data2 or extended value
    unsigned operand = (ALUSrc == 1) ? extended_value : data2;

    char operation;
    if (ALUOp == 7) // R-type
    {
        switch (funct)
        {
        case 32: operation = 0; break; // add
        case 34: operation = 1; break; // sub
        case 42: operation = 2; break; // slt
        case 43: operation = 3; break; // sltu
        case 36: operation = 4; break; // and
        case 37: operation = 5; break; // or
        default:
            return 1; // Invalid funct code, halt the program
        }

    }
    else {
        switch (ALUOp) // I-type
        {
        case 0: operation = 0; break; // add (e.g., lw, sw, addi)
        case 1: operation = 1; break; // sub (e.g., beq)
        case 2: operation = 2; break; // slt (e.g., slti)
        case 3: operation = 3; break; // sltu (e.g., sltiu)
        case 6: operation = 6; break; // lui
        default: return 1; //  Invalid ALUOp, halt the program
        }
    }

    ALU(data1, operand, operation, ALUresult, Zero);

    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned* memdata, unsigned* Mem)
{
    if(MemWrite == 1 || MemRead == 1){
        if((ALUresult % 4) != 0){
            return 1;
        }
        if(MemWrite == 1){
            memdata = data2;
        }
        else{
            memdata = Mem;
        }
    }
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned* Reg)
{
    if((RegWrite == 1) && (MemtoReg == 1)){
        Reg[RegDst] = memdata;
    }
    else if((RegWrite == 1) && (MemtoReg == 0)){
        Reg[RegDst] = ALUresult;
    }

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned* PC)
{
    PC += 4;
    if(Jump == 1){
        jsec<<2;
    }
}
