#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_CHARACTERS 60

int first = 1;

void print_hex(uint32_t value){
    if (first) {
        printf("Instructions parsed\n");
        first--;
    }
    printf("0x%08X\n", value);
}

int compare(const char* s1, const char* s2){
    int i;
    for (i = 0; s1[i] != '\0' && s2[i] != '\0'; i++) if (s1[i] != s2[i]) return 0;
    return s1[i] == s2[i];
}

uint32_t check_op(const char *op){
    if (compare(op, "add") || compare(op, "sub") || compare(op, "and") || compare(op, "or")) return 0x33;
    if (compare(op, "addi")) return 0x13;
    if (compare(op, "beq")) return 0x63;
    
    return 0x0;
}

void return_funct(const char *op, uint32_t* funct3, uint32_t* funct7){
    if (compare(op, "add")){
        *funct3 = 0x0;
        *funct7 = 0x0;
    }
    if (compare(op, "sub")){
        *funct3 = 0x0;
        *funct7 = 0x20;
    }
    if (compare(op, "and")){
        *funct3 = 0x7;
        *funct7 = 0x0;
    }
    if (compare(op, "or")){
        *funct3 = 0x6;
        *funct7 = 0x0;
    }
}

void return_funct3(const char *op, uint32_t* funct3){
    if (compare(op, "addi") || compare(op, "beq")) *funct3 = 0x0;
}

uint32_t assemble(const char *instruction){
    uint32_t result = 0x0;
    uint32_t op, rd, rs1, rs2, funct3, funct7;
    int32_t imm_i, imm_s, imm_b;
    
    char op_str[6], rd_str[4], rs1_str[4], rs2_str[4], imm_i_str[5], imm_s_str[11], imm_b_str[11];
    int i, j;

    for (i = 0; instruction[i] != ' ' && instruction[i] != '\0'; i++) op_str[i] = instruction[i];
    op_str[i] = '\0';
    op = check_op(op_str);

    // #### TYPE R
    if (op == 0x33){   
        j = 0;
        while(instruction[i] != 'x') i++;
        i++;
        for (; instruction[i] != ','; i++, j++) rd_str[j] = instruction[i];
        rd_str[j] = '\0';
        rd = atoi(rd_str);
        while(instruction[i] != 'x') i++;
        i++;
        j = 0;

        for (; instruction[i] != ','; i++, j++) rs1_str[j] = instruction[i];
        rs1_str[j] = '\0';
        rs1 = atoi(rs1_str);
        while(instruction[i] != 'x') i++;
        i++;
        j = 0;

        for (; instruction[i] != '\n' && instruction[i] != '\0'; i++, j++) rs2_str[j] = instruction[i];
        rs2_str[j] = '\0';
        rs2 = atoi(rs2_str);

        return_funct(op_str, &funct3, &funct7);

        result |= op;
        result |= (rd << 7);
        result |= (funct3 << 12);
        result |= (rs1 << 15);
        result |= (rs2 << 20);
        result |= (funct7 << 25);
    }
    // #### TYPE I
    if (op == 0x13){    
        j = 0;
        while(instruction[i] != 'x') i++;
        i++;
        for (; instruction[i] != ','; i++, j++) rd_str[j] = instruction[i];
        rd_str[j] = '\0';
        rd = atoi(rd_str);
        while(instruction[i] != 'x') i++;
        i++;
        j = 0;

        for (; instruction[i] != ','; i++, j++) rs1_str[j] = instruction[i];
        rs1_str[j] = '\0';
        rs1 = atoi(rs1_str);
        while(instruction[i] == ' ') i++;
        i++;
        j = 0;

        for (; instruction[i] != '\n' && instruction[i] != '\0'; i++, j++) imm_i_str[j] = instruction[i];
        imm_i = atoi(imm_i_str);
        //printf("imm = 0x%08X\n", imm_i);

        return_funct3(op_str, &funct3);

        result |= op;
        result |= (rd << 7);
        result |= (funct3 << 12);
        result |= (rs1 << 15);
        result |= (imm_i << 20);
    }
    // #### TYPE B
    if (op == 0x63){
        // à fazer;
    }

    //print_hex(result);
    return result;
}

int main(){
    FILE *instructions = fopen("instructions.txt", "r");
    if (instructions == NULL){
        printf("failed to open instructions.txt\n");
        return 1;
    }

    FILE *code = fopen("machine_code.txt", "w");
    if (code == NULL){
        printf("failed to open instructions.txt\n");
        return 2;
    }

    uint32_t inst;
    char str[MAX_CHARACTERS];
    char inst_str[12];

    while(fgets(str, MAX_CHARACTERS, instructions) != NULL){
        inst = assemble(str);
        sprintf(inst_str, "0x%08X\n", inst);
        fputs(inst_str, code);
    }
    
    fclose(instructions);
    fclose(code);

    return 0;
}