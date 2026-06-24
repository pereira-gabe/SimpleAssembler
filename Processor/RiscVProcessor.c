#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_INSTRUCTIONS 1024

uint32_t program[MAX_INSTRUCTIONS];
uint32_t reg[32];
uint32_t pc = 0;

static const char* reg_names[32] = {
    "zero", "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

int32_t sign_extend(int32_t value, int bits) {
    int32_t mask = 1 << (bits - 1);
    return (value ^ mask) - mask;
}

void print_registers() {
    printf("============ Registers Table ============\n");

    for (int i = 0; i < 16; i++) {
        printf(
            "%-4s (x%-2d) = %-10d | %-4s (x%-2d) = %d\n",
            reg_names[i],
            i,
            reg[i],
            reg_names[i + 16],
            i + 16,
            reg[i + 16]
        );
    }

    printf("=========================================\n");
}

void execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    uint32_t rd     = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1    = (instruction >> 15) & 0x1F;
    uint32_t rs2    = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    int32_t imm_i = sign_extend((instruction >> 20) & 0xFFF, 12);

    int32_t imm_b = sign_extend(
        (((instruction >> 31) & 0x1) << 12) |
        (((instruction >> 7)  & 0x1) << 11) |
        (((instruction >> 25) & 0x3F) << 5) |
        (((instruction >> 8)  & 0xF) << 1),
        13
    );

    switch (opcode) {

        case 0x33:
            switch (funct3) {

                case 0x0:
                    if (funct7 == 0x00)
                        reg[rd] = reg[rs1] + reg[rs2];
                    else if (funct7 == 0x20)
                        reg[rd] = reg[rs1] - reg[rs2];
                    break;

                case 0x7:
                    reg[rd] = reg[rs1] & reg[rs2];
                    break;

                case 0x6:
                    reg[rd] = reg[rs1] | reg[rs2];
                    break;
            }

            pc += 4;
            break;

        case 0x13:
            reg[rd] = reg[rs1] + imm_i;
            pc += 4;
            break;

        case 0x63:
            if (reg[rs1] == reg[rs2])
                pc += imm_b;
            else
                pc += 4;
            break;

        default:
            printf("unsupported instruction: 0x%08X\n", instruction);
            exit(1);
    }

    reg[0] = 0;
}

int main() {
    FILE *file = fopen("machine_code.txt", "r");

    if (file == NULL) {
        printf("error opening machine_code.txt\n");
        return 1;
    }

    int instruction_count = 0;

    while (
        instruction_count < MAX_INSTRUCTIONS &&
        fscanf(file, "%x", &program[instruction_count]) == 1
    ) {
        instruction_count++;
    }

    fclose(file);

    while (pc < instruction_count * 4) {
        execute(program[pc / 4]);
    }

    print_registers();

    return 0;
}
