#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_CHARACTERS 60
#define DEBUG 0
#define LABEL_FOUND 0xFFFFFFFF
#define INVALID_INSTRUCTION 0xFFFFFFFE 

static int first = 1;
static uint32_t beg_offset = 0; // current offset from the first instruction of the segment

typedef struct Label{
    char* name;
    int32_t offset;
} Label;

static uint32_t label_list_size = 0;
Label* label_list = NULL;

FILE *instructions = NULL; 
FILE *code = NULL; 

static const char* abi_convention[32] = {
    "zero", "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

void initialize(){
    instructions = fopen("instructions.txt", "r");
    code = fopen("machine_code.txt", "w");
}

void print_hex(uint32_t value){
    if (first) {
        printf("Instructions parsed:\n");
        first--;
    }
    printf("0x%08X\n", value);
}

void print_labels(){
    for (int i = 0; i < label_list_size; i++){
        printf("%s, %d\n", label_list[i].name, label_list[i].offset);
    }
}

int contains(const char* s, char target){
    int i = 0;
    while (s[i] != '\n' && s[i] != '\0'){
        if (s[i] == target) return 1;
        i++;
    }
    return 0;
}

uint32_t check_op(const char *op){
    if (strcmp(op, "add") == 0 || strcmp(op, "sub") == 0 || strcmp(op, "and") == 0 || strcmp(op, "or") == 0) return 0x33;
    if (strcmp(op, "addi") == 0) return 0x13;
    if (strcmp(op, "beq") == 0) return 0x63;
    
    return 0x0;
}

void return_funct(const char *op, uint32_t* funct3, uint32_t* funct7){
    if (strcmp(op, "add") == 0){
        *funct3 = 0x0;
        *funct7 = 0x0;
    }
    if (strcmp(op, "sub") == 0){
        *funct3 = 0x0;
        *funct7 = 0x20;
    }
    if (strcmp(op, "and") == 0){
        *funct3 = 0x7;
        *funct7 = 0x0;
    }
    if (strcmp(op, "or") == 0){
        *funct3 = 0x6;
        *funct7 = 0x0;
    }
}

void return_funct3(const char *op, uint32_t* funct3){
    if (strcmp(op, "addi") == 0 || strcmp(op, "beq") == 0) *funct3 = 0x0;
}

void replace_substring(char *s1, const char *old, const char *s2) {
    char *pos = strstr(s1, old);

    if (pos == NULL) return;

    size_t old_len = strlen(old);
    size_t new_len = strlen(s2);

    memmove(pos + new_len, pos + old_len, strlen(pos + old_len) + 1);
    memcpy(pos, s2, new_len);
}

void replace_registers(char *inst) {
    char old[8];
    char new_reg[8];

    for (int reg = 0; reg < 32; reg++) {
        char *p = inst;

        while ((p = strstr(p, abi_convention[reg])) != NULL) {
            size_t len = strlen(abi_convention[reg]);

            char before = (p == inst) ? '\0' : p[-1];
            char after  = p[len];

            int left_ok =
                before == '\0' ||
                before == ' '  ||
                before == ','  ||
                before == '('  ||
                before == ')'  ||
                before == '\t' ||
                before == '\n';

            int right_ok =
                after == '\0' ||
                after == ' '  ||
                after == ','  ||
                after == '('  ||
                after == ')'  ||
                after == '\t' ||
                after == '\n';

            if (left_ok && right_ok) {
                strcpy(old, abi_convention[reg]);
                sprintf(new_reg, "x%d", reg);

                replace_substring(inst, old, new_reg);

                p = inst; 
            } else p++;
        }
    }
}

void first_pass(){  // the translation of the abi reg names is done on the second pass
    int offset = 0;
    char str[MAX_CHARACTERS];
    while(fgets(str, MAX_CHARACTERS, instructions) != NULL){
        if (contains(str, ':')){
            char* colon = strchr(str, ':');
            *colon = '\0';
            label_list = realloc(label_list, (label_list_size + 1) * sizeof(*label_list));
            label_list[label_list_size].name = malloc(MAX_CHARACTERS);
            strcpy(label_list[label_list_size].name, str);
            label_list[label_list_size].offset = offset;
            label_list_size++;
        }
        else offset += 4;   
    }
}

int return_label_offset(char* label, int offset){
    for (int i = 0; i < label_list_size; i++) {
        if (strcmp(label_list[i].name, label) == 0) return label_list[i].offset - offset;
    }
    printf("label on line %d not found\n", (beg_offset / 4));
    exit(2);
}

uint32_t assemble(char *instruction){
    uint32_t result = 0x0;
    uint32_t op, rd, rs1, rs2, funct3, funct7;
    int32_t imm_i, imm_s;
    
    if (contains(instruction, ':')){   // if the line represents a label
        return LABEL_FOUND;
    }

    replace_registers(instruction);

    char op_str[MAX_CHARACTERS], rd_str[10], rs1_str[10], rs2_str[10], imm_i_str[10], imm_s_str[20];
    int i, j;

    for (i = 0; instruction[i] != ' ' && instruction[i] != '\0'; i++) op_str[i] = instruction[i];
    op_str[i] = '\0';

    uint32_t current_pc = beg_offset;
    beg_offset += 4;    // labels don't change the offset
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
    else if (op == 0x13){    
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
        imm_i_str[j] = '\0';
        imm_i = atoi(imm_i_str);
        //if(DEBUG) printf("imm = 0x%08X\n", imm_i);

        return_funct3(op_str, &funct3);

        result |= op;
        result |= (rd << 7);
        result |= (funct3 << 12);
        result |= (rs1 << 15);
        result |= ((imm_i & 0xFFF) << 20);
    }
    // #### TYPE B  
    else if (op == 0x63) {
        j = 0;
        while(instruction[i] != 'x') i++;
        i++;
        for (; instruction[i] != ','; i++, j++) rs1_str[j] = instruction[i];  
        rs1_str[j] = '\0';
        rs1 = atoi(rs1_str);
        j = 0;
        
        while(instruction[i] != 'x') i++;
        i++;
        for (; instruction[i] != ','; i++, j++) rs2_str[j] = instruction[i];  
        rs2_str[j] = '\0';
        rs2 = atoi(rs2_str);
        j = 0;
        
        while (instruction[i] == ' ' || instruction[i] == ',') i++;
        char label[MAX_CHARACTERS];
        for (; instruction[i] != '\n' && instruction[i] != '\0'; i++, j++) label[j] = instruction[i];
        label[j] = '\0'; 
        int imm =  return_label_offset(label, current_pc);
        return_funct3(op_str, &funct3);

        result |= (((imm >> 12) & 0x01) << 31); // imm[12]
        result |= (((imm >> 5)  & 0x3F) << 25); // imm[10:5]
        result |= (((imm >> 1)  & 0x0F) << 8);  // imm[4:1]
        result |= (((imm >> 11) & 0x01) << 7);  // imm[11]

        result |= op;
        result |= (funct3 << 12);
        result |= (rs1 << 15);
        result |= (rs2 << 20);
    }
    else{
        printf("unsupported instruction\n");
        return INVALID_INSTRUCTION;
    }

    return result;
}

int main(){
    initialize();

    if (instructions == NULL){
        printf("failed to open instructions.txt\n");
        return 1;
    } 

    if (code == NULL){
        printf("failed to open machine_code.txt\n");
        return 2;
    }

    first_pass();
    rewind(instructions); // reset the instruction pointer to the beginning of the file


    uint32_t inst;
    char str[MAX_CHARACTERS];
    char inst_str[32];
    int line = 0;

    while(fgets(str, MAX_CHARACTERS, instructions) != NULL){
        inst = assemble(str);
        line++;
        if (inst == LABEL_FOUND){
            if(DEBUG) printf("label\n");
            continue;
        }
        if (inst == INVALID_INSTRUCTION){
            printf("invalid instruction on line %d\n", line);
            exit(1);
        }
        sprintf(inst_str, "0x%08X\n", inst);
        fputs(inst_str, code);
        if (DEBUG) print_hex(inst);
    }
    if (DEBUG) print_labels();
    
    fclose(instructions);
    fclose(code);
    for (int i = 0; i < label_list_size; i++) free(label_list[i].name);
    free(label_list);

    return 0;
}