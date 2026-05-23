#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEM_SIZE 1024

uint32_t memory[MEM_SIZE];
uint32_t reg[32];
uint32_t pc = 0;

int32_t sign_extend(int32_t value, int bits) {
    int32_t mask = 1 << (bits - 1);
    return (value ^ mask) - mask;
}

char* abi_table(int reg){
    switch(reg){
        case 0:  return "zero";
        case 1:  return "ra";
        case 2:  return "sp";
        case 3:  return "gp";
        case 4:  return "tp";
        case 5:  return "t0";
        case 6:  return "t1";
        case 7:  return "t2";
        case 8:  return "s0";
        case 9:  return "s1";
        case 10: return "a0";
        case 11: return "a1";
        case 12: return "a2";
        case 13: return "a3";
        case 14: return "a4";
        case 15: return "a5";
        case 16: return "a6";
        case 17: return "a7";
        case 18: return "s2";
        case 19: return "s3";
        case 20: return "s4";
        case 21: return "s5";
        case 22: return "s6";
        case 23: return "s7";
        case 24: return "s8";
        case 25: return "s9";
        case 26: return "s10";
        case 27: return "s11";
        case 28: return "t3";
        case 29: return "t4";
        case 30: return "t5";
        case 31: return "t6";
        default: return "invalid";
    }
}

void print_registers() {
    printf("========= Registers Table =========\n");

    for(int i = 0; i < 16; i++) {
        printf(
            "%-4s (x%-2d) = %-2u  |  %-4s (x%-2d) = %u\n",
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
<<<<<<< HEAD
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
=======
    memory[0] = 0x00A00093;
    memory[1] = 0x01400113;
    memory[2] = 0x002081B3;

    int instructions = 3;
>>>>>>> 1ad59ec9891ad36bab7917bcbcec8767ab47dd75

    while(pc < instructions * 4) {
        uint32_t instruction = memory[pc / 4];
        execute(instruction);
    }

    print_registers();

    return 0;
<<<<<<< HEAD
}
=======
}
>>>>>>> 1ad59ec9891ad36bab7917bcbcec8767ab47dd75
