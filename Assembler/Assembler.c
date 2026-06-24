#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_LINE 128
#define N_INSTRUCTIONS 6 // instruction set size

typedef struct{
    char* name;
    int32_t offset;
} Label;

typedef struct{
    FILE* instructions;
    FILE* machine_code;
    Label* labels;
    int nlabels;
    uint32_t offset;
} Assembler;

typedef struct{
    char mnemonic[16];

    uint32_t opcode;
    uint32_t funct3;
    uint32_t funct7;

    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;

    int32_t imm;
} Instruction;

static const char* registers[32] = {    // TODO: hash table implementation of reg table
    "zero", "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

Instruction instruction_set[] = {
    {.mnemonic="sub",  .opcode=0x33, .funct3=0x0, .funct7=0x20},
    {.mnemonic="add",  .opcode=0x33, .funct3=0x0, .funct7=0x00},
    {.mnemonic="and",  .opcode=0x33, .funct3=0x7, .funct7=0x00},
    {.mnemonic="or",   .opcode=0x33, .funct3=0x6, .funct7=0x00},
    {.mnemonic="addi", .opcode=0x13, .funct3=0x0},
    {.mnemonic="beq",  .opcode=0x63, .funct3=0x0}
};

void initialize(Assembler* as, const char* inst_file, const char* out_file){ // TODO: proper error handling
    as->instructions = fopen(inst_file, "r");
    if (as->instructions == NULL){
        printf("failed to open instructions file\n");
        exit(10);
    }
    as->machine_code = fopen(out_file, "w");
    if (as->machine_code == NULL){
        printf("failed to open instructions file\n");
        exit(10);
    }
    as->labels = NULL;
    as->nlabels = 0;
    as->offset = 0;
}

void print_labels(Assembler* as){
    for (int i = 0; i < as->nlabels; i++){
        printf("%s, %d\n", as->labels[i].name, as->labels[i].offset);
    }
}

/* swapped for reg intepretation approach
void replace_substring(char *s1, const char *old, const char *s2) {
    char *pos = strstr(s1, old);

    if (pos == NULL) return;

    size_t old_len = strlen(old);
    size_t new_len = strlen(s2);

    memmove(pos + new_len, pos + old_len, strlen(pos + old_len) + 1);
    memcpy(pos, s2, new_len);
}*/

void first_pass(Assembler* as){
    uint32_t offset = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), as->instructions) != NULL){
        char* comment = strchr(line, '#');

        if (comment != NULL)
            *comment = '\0';

        if (strspn(line, " \t\r\n") == strlen(line)) // skip blank lines
            continue;

        if (strchr(line, ':') != NULL){ // label: instruction is ignored
            char* label = strtok(line, ":");

            as->nlabels++;
            as->labels = realloc(
                as->labels,
                as->nlabels * sizeof(*as->labels)
            );

            as->labels[as->nlabels - 1].name = strdup(label);
            as->labels[as->nlabels - 1].offset = offset;
        }
        else{
            offset += 4;
        }
    }

    rewind(as->instructions);
}

int32_t return_label_offset(Assembler* as, char* label, int offset){
    for (int i = 0; i < as->nlabels; i++) {
        if (strcmp(as->labels[i].name, label) == 0)
            return as->labels[i].offset - offset;
    }
    printf("label on line %d not found\n", (as->offset / 4));
    exit(3);
}

Instruction* find_instruction(const char *mnemonic){
    for (int i = 0; i < N_INSTRUCTIONS; i++){
        if (strcmp(mnemonic, instruction_set[i].mnemonic) == 0)
            return &instruction_set[i];
    }

    return NULL;
}

int parse_register(const char *reg){
    if (reg[0] == 'x'){
        int n = atoi(reg + 1);

        if (n >= 0 && n <= 31)
            return n;
    }

    for (int i = 0; i < 32; i++){
        if (strcmp(reg, registers[i]) == 0)
            return i;
    }

    printf("Invalid register %s\n", reg);
    exit(1);
}

void second_pass(Assembler* as){
    char line[MAX_LINE];

    while(fgets(line, sizeof(line), as->instructions) != NULL){
        uint32_t result = 0x0;

        if (strchr(line, ':') != NULL) // label: instruction is ignored
            continue;

        char* comment = strchr(line, '#');

        if (comment != NULL)
            *comment = '\0';

        char *tokens[10];
        int n_tokens = 0;

        char *tok = strtok(line, " ,\t\n");

        while (tok != NULL && n_tokens < 8){
            tokens[n_tokens++] = tok;
            tok = strtok(NULL, " ,\t\n");
        }

        if (n_tokens == 0)
            continue;

        Instruction *inst = find_instruction(tokens[0]);

        if (inst == NULL){
            printf("Unsupported instruction %s on line %u\n", tokens[0], (as->offset / 4) + 1);
            exit(1);
        }

        if (inst->opcode == 0x33){ // TYPE R
            int rd  = parse_register(tokens[1]);
            int rs1 = parse_register(tokens[2]);
            int rs2 = parse_register(tokens[3]);

            result =
                inst->opcode |
                (rd << 7) |
                (inst->funct3 << 12) |
                (rs1 << 15) |
                (rs2 << 20) |
                (inst->funct7 << 25);
        }

        else if (inst->opcode == 0x13){ // TYPE I
            int rd  = parse_register(tokens[1]);
            int rs1 = parse_register(tokens[2]);
            int imm = atoi(tokens[3]);

            result =
                inst->opcode |
                (rd << 7) |
                (inst->funct3 << 12) |
                (rs1 << 15) |
                ((imm & 0xFFF) << 20);
        }

        else if (inst->opcode == 0x63){ // TYPE B
            int rs1 = parse_register(tokens[1]);
            int rs2 = parse_register(tokens[2]);

            int imm = return_label_offset(
                as,
                tokens[3],
                as->offset
            );

            result =
                inst->opcode |
                (inst->funct3 << 12) |
                (rs1 << 15) |
                (rs2 << 20);

            result |= ((imm >> 12) & 0x1) << 31;
            result |= ((imm >> 5)  & 0x3F) << 25;
            result |= ((imm >> 1)  & 0xF) << 8;
            result |= ((imm >> 11) & 0x1) << 7;
        }

        as->offset += 4;
        char inst_str[12];
        sprintf(inst_str, "0x%08X\n", result);
        //printf("%s", inst_str);
        fputs(inst_str, as->machine_code);
    }
}

void close(Assembler* as){
    fclose(as->instructions);
    fclose(as->machine_code);
    for (int i = 0; i < as->nlabels; i++)
        free(as->labels[i].name);
    free(as->labels);
}

int main(){
    Assembler as;

    initialize(&as, "instructions.txt", "machine_code.txt");
    first_pass(&as);
    second_pass(&as);
    close(&as);

    return 0;
}
