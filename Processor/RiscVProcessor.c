#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEM_SIZE 1024

uint32_t memory[MEM_SIZE];
uint32_t reg[32];
uint32_t pc = 0;

char** abi_convention = NULL;

int32_t sign_extend(int32_t value, int bits) {
    int32_t mask = 1 << (bits - 1);
    return (value ^ mask) - mask;
}

char* abi_table(int reg){
    return abi_convention[reg];
}

void print_registers() {
    printf("============ Registers Table ============\n");

    for(int i = 0; i < 16; i++) {
        printf(
            "%-4s (x%-2d) = %-2d     |     %-4s (x%-2d) = %d\n",
            abi_table(i),
            i,
            reg[i],
            abi_table(i + 16),
            i + 16,
            reg[i + 16]
        );
    }
}

void execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F; // máscara com os 7 últimos bits ligados
    uint32_t rd     = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1    = (instruction >> 15) & 0x1F;
    uint32_t rs2    = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    int32_t imm_i = sign_extend((instruction >> 20) & 0xFFF, 12);
    int32_t imm_s = sign_extend((((instruction >> 25) & 0x7F) << 5) |((instruction >> 7) & 0x1F),12);
    int32_t imm_b = sign_extend((((instruction >> 31) & 0x1) << 12) |(((instruction >> 7) & 0x1) << 11) |(((instruction >> 25) & 0x3F) << 5) |(((instruction >> 8) & 0xF) << 1),13);

    switch(opcode) {
        case 0x33:     // R-Type
            switch(funct3) {
                case 0x0:
                    if(funct7 == 0x00) {        // ADD
                        reg[rd] = reg[rs1] + reg[rs2];
                    }

                    else if(funct7 == 0x20) {   // SUB
                        reg[rd] = reg[rs1] - reg[rs2];
                    }

                    break;
                case 0x7:                       // AND
                    reg[rd] = reg[rs1] & reg[rs2];
                    break;
                case 0x6:                       // OR
                    reg[rd] = reg[rs1] | reg[rs2];
                    break;
            }
            pc += 4;
            break;

        case 0x13:     // I-Type
            if(funct3 == 0x0) {                 // ADDI
                reg[rd] = reg[rs1] + imm_i;
            }
            pc += 4;
            break;

        case 0x03:     // LOAD
            if(funct3 == 0x2) {                 // LW
                reg[rd] = memory[(reg[rs1] + imm_i) / 4];
            }
            pc += 4;
            break;

        case 0x23:     // STORE
            if(funct3 == 0x2) {                 // SW
                memory[(reg[rs1] + imm_s) / 4] = reg[rs2];
            }
            pc += 4;
            break;

        case 0x63:     // BRANCH
            if(funct3 == 0x0) {                 // BEQ
                if(reg[rs1] == reg[rs2]) {
                    pc += imm_b;
                }
                else {
                    pc += 4;
                }
            }
            break;

        default:
            printf("unsuported instruction\n");
            exit(1);
    }

    reg[0] = 0;
}

int main() {
    abi_convention = malloc(32 * sizeof(char*));
    abi_convention[0]  = "zero";
    abi_convention[1]  = "ra";
    abi_convention[2]  = "sp";
    abi_convention[3]  = "gp";
    abi_convention[4]  = "tp";
    abi_convention[5]  = "t0";
    abi_convention[6]  = "t1";
    abi_convention[7]  = "t2";
    abi_convention[8]  = "s0";
    abi_convention[9]  = "s1";
    abi_convention[10] = "a0";
    abi_convention[11] = "a1";
    abi_convention[12] = "a2";
    abi_convention[13] = "a3";
    abi_convention[14] = "a4";
    abi_convention[15] = "a5";
    abi_convention[16] = "a6";
    abi_convention[17] = "a7";
    abi_convention[18] = "s2";
    abi_convention[19] = "s3";
    abi_convention[20] = "s4";
    abi_convention[21] = "s5";
    abi_convention[22] = "s6";
    abi_convention[23] = "s7";
    abi_convention[24] = "s8";
    abi_convention[25] = "s9";
    abi_convention[26] = "s10";
    abi_convention[27] = "s11";
    abi_convention[28] = "t3";
    abi_convention[29] = "t4";
    abi_convention[30] = "t5";
    abi_convention[31] = "t6";

    FILE *file = fopen("../machine_code.txt", "r");

    if (file == NULL) {
        printf("error opening machine_code.txt\n");
        return 1;
    }

    int instructions = 0;

    while (instructions < MEM_SIZE && fscanf(file, "%x", &memory[instructions]) == 1) instructions++;

    fclose(file);

    if (instructions > MEM_SIZE / 8){
        printf("instructions limit exceeded.\nMax instructions supported: %d", MEM_SIZE);
        return 2;
    }

    while(pc < instructions * 4) {
        uint32_t instruction = memory[pc / 4];
        execute(instruction);
    }

    print_registers();

    free(abi_convention);

    return 0;
}