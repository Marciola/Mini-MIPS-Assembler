#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <search.h>

struct Label {
    char name[50];
    int address;
};

// Calculates and prints current PC
void printPC(int PC);

void printInstruction(long answer1);

// Returns the following:
// 0: not directive
// 1: .data or .text
// 2: .space
// 3: .word
int isDirective(char* instruction);

int isLabel(char* instruction);

// Returns the following:
// 0: not instruction
// 1: R-type
// 2: I-type
// 3: J-type
// 4: la instruction
int isInstruction(char* token);

int isRegister(char* token);

int getLength(char* instruction);

// Returns decimal representation of symbolic register 
int getRegRep(char* token);

long getImmed(long dest, long branch);

void buildLA(char* code, int loadRegister, char* address);

long rType(long rs, long rt, long rd, long shamt, long funct);

long iType(long op, long rs, long rt, long immed);

long jType(long op, long currentPC, long targaddr);

long getUpper(long address);

long getLower(long address);

int saveLabel(struct Label* arr, int LabelSize, char* token, int pc);

long getLabel(struct Label* arr, int LabelSize, char* token);

main()
{

    char instruction[100];
    char code[1000] = "";
    char* space = " ";
    char* hexPC = NULL;
    char* delims = " \n\t,()";
    int PC = 0;
    int tempPC = 0;
    int i = 0;
    char* nullToken = NULL;
    char* token = "";
    char* oldToken = "";
    char* EOFTest = "";
    // Used in char* to int conversion
    char* junk;
    struct Label Labels[100];
    int LabelsSize = 0;
    
    // First assembler walk through
    do {

        // Get instruction
        EOFTest = fgets (instruction, 100, stdin);
        
        if (EOFTest != NULL) {
            // Initialize token
            token = strtok(instruction, delims);
        }
        
        while(token != NULL) {

            if (isLabel(token)) {
                saveLabel(Labels, LabelsSize, token, PC);
                LabelsSize++;
            }

            if (isRegister(token)) {
                sprintf(token, "%d", getRegRep(token));
            }

            // Checks for 'la' - a pseudo-instruction
            if (isInstruction(token) == 4) {
                char* loadReg = "";
                char* loadLabel = "";

                loadReg = strtok(nullToken, delims);
                loadLabel = strtok(nullToken, delims);

                buildLA(code, getRegRep(loadReg), loadLabel);
            } else {
                strcat(code, token);
                strcat(code, space);
            }
            
            oldToken = token;
            token = strtok(nullToken, delims);
    
            if (isInstruction(oldToken) > 0) {
                if (isInstruction(oldToken) == 4) {
                    PC += 8;
                } else {
                    PC += 4;
                }
            } else if (isDirective(oldToken) == 3) {
                PC += 4;
            } else if (isDirective(oldToken) == 2) {
                tempPC = strtol(token, &junk, 10);
                if (tempPC > 4) {   
                    PC += tempPC;
                } else {
                    PC += 4;
                }
            }
        }
    } while(EOFTest != NULL);

    PC = 0;
    token = strtok(code, " ");

    // Second assembler walk through
    long rs;
    long rt;
    long rd;
    long label = 0;
    long shamt;
    long immed;
    long funct;
    long answer = -1;
    char* need = ":";
    char temp[100];
    char command[100];

    while (token != NULL) {

        strcpy(command, token);

        if (isInstruction(token) == 0) {

        } else if (isInstruction(token) == 1) {
            if (strcmp(token, "add") == 0) {
                token = strtok(nullToken, " ");
                rd = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                answer = rType(rs, rt, rd, 0, 32);
            } else if (strcmp(token, "nor") == 0) {
                token = strtok(nullToken, " ");
                rd = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                answer = rType(rs, rt, rd, 0, 39);
            } else if (strcmp(token, "sll") == 0) {
                token = strtok(nullToken, " ");
                rd = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                shamt = strtol(token, &junk, 10);

                answer = rType(0, rt, rd, shamt, 0);
            }
        } else if (isInstruction(token) == 2) {
            if (strcmp(token, "addi") == 0) {
                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                immed = strtol(token, &junk, 10);

                answer = iType(8, rs, rt, immed);
            } else if (strcmp(token, "ori") == 0) {
                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                immed = strtol(token, &junk, 10);

                if (getLabel(Labels, LabelsSize, token) > 0) {
                    immed = getLower(getLabel(Labels, LabelsSize, token));
                }

                answer = iType(13, rs, rt, immed);
            } else if (strcmp(token, "lui") == 0) {
                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                immed = strtol(token, &junk, 10);
                
                if (getLabel(Labels, LabelsSize, token) > 0) {
                    immed = getUpper(getLabel(Labels, LabelsSize, token));
                }

                answer = iType(15, 0, rt, immed);
            } else if (strcmp(token, "sw") == 0) {
                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                immed = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);

                answer = iType(43, rs, rt, immed);
            } else if (strcmp(token, "lw") == 0) {
                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                immed = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);
                
                answer = iType(35, rs, rt, immed);
            } else if (strcmp(token, "bne") == 0) {
                token = strtok(nullToken, " ");
                rs = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                rt = strtol(token, &junk, 10);

                token = strtok(nullToken, " ");
                strcpy(temp, token);
                label = getLabel(Labels, LabelsSize, temp);
                
                immed = getImmed(label, PC);

                answer = iType(5, rs, rt, immed);
            }
        } else if (isInstruction(token) == 3) {
            token = strtok(nullToken, " ");
            strcpy(temp, token);
            label = getLabel(Labels, LabelsSize, temp);
            answer = jType(2, PC, label);
        }

        oldToken = token;
        token = strtok(nullToken, " ");
        
        if (answer != -1) {
            printPC(PC);
            printInstruction(answer);
            answer = -1;
        }
       
        if (isInstruction(command) > 0) {
            if (isInstruction(command) == 4) {
                PC += 8;
            } else {
                PC += 4;
            }
        } else if (isDirective(command) == 3) {
            PC += 4;
        } else if (isDirective(command) == 2) {
            tempPC = strtol(token, &junk, 10);
            if (tempPC > 4) {   
                PC += tempPC;
            } else {
                PC += 4;
            }
        }
        
    }
}

void printPC(int PC)
{
    int temp,k,j,i = 1;
    char hexadecimal[6];

    while(PC != 0) 
    {
        temp = PC % 16;

        if (temp < 10) {
            temp = temp + 48;
        } else {
            temp = temp + 55;
        }

        hexadecimal[i++] = temp;
        PC = PC / 16;
    }
    
    printf("0x");
    for(k = 8 - i; k >= 0; k--) {
        printf("0");
    }

    for(j = i - 1; j > 0; j--) {
        printf("%c", hexadecimal[j]);
    }
    printf(": ");
}

int isLabel(char* instruction) 
{
    int i = 0;
    char temp = *instruction;

    while(temp != '\0') {
        if(temp == ':') {
            return 1;
        }
        i++;
        temp = *(instruction + i);
    }

    return 0;
}

int isDirective(char* instruction) 
{
    int i = 0;
    char temp = *instruction;

    if (temp == '.') {
        if (strcmp(instruction, ".space") == 0) {
            return 2;
        } else if (strcmp(instruction, ".word") == 0) {
            return 3;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

int getLength(char* instruction)
{
    int count = 0;
    char temp = *instruction;

    while (temp != '\0' && temp != '\n') {
        count++;
        temp = *(instruction + count);
    }

    return count;
}

int getRegRep(char* token) 
{
    if (token[1] == '0') {
        return 0;
    } else if (token[1] == 't') {
        return ((token[2] - '0') + 8);
    } else if (token[1] == 's') {
        return ((token[2] - '0') + 16);
    }
}

int isRegister(char* token) 
{
    int i = 0;
    char temp = *token;

    if (temp == '$') {
        return 1;
    } else {
        return 0;
    }
}

void buildLA(char* code, int loadRegister, char* address)
{
    char* space = " ";
    char loadReg[10];
    
    strcat(code, "lui");
    strcat(code, space);

    // Converting int to char*
    sprintf(loadReg, "%d", loadRegister);
    strcat(code, loadReg);
    strcat(code, space);

    strcat(code, address);
    strcat(code, space);

    strcat(code, "ori");
    strcat(code, space);

    strcat(code, loadReg);
    strcat(code, space);

    strcat(code, loadReg);
    strcat(code, space);

    strcat(code, address);
    strcat(code, space);
}

long rType(long rs, long rt, long rd, long shamt, long funct)
{
    long rsShift = 21;
    long rtShift = 16;
    long rdShift = 11;
    long shamtShift = 6;
    long final = 0;

    rs = rs << rsShift;
    rt = rt << rtShift;
    rd = rd << rdShift;
    shamt = shamt << shamtShift;

    final = rs | rt;
    final = final | rd;
    final = final | shamt;
    final = final | funct;

    return final;
}

int isInstruction(char* token) 
{
    if (strcmp(token, "add") == 0) {
        return 1;
    } else if (strcmp(token, "addi") == 0) {
        return 2;
    } else if (strcmp(token, "nor") == 0) {
        return 1;
    } else if (strcmp(token, "ori") == 0) {
        return 2;
    } else if (strcmp(token, "sll") == 0) {
        return 1;
    } else if (strcmp(token, "lui") == 0) {
        return 2;
    } else if (strcmp(token, "sw") == 0) {
        return 2;
    } else if (strcmp(token, "lw") == 0) {
        return 2;
    } else if (strcmp(token, "bne") == 0) {
        return 2;
    } else if (strcmp(token, "j") == 0) {
        return 3;
    } else if (strcmp(token, "la") == 0) {
        return 4;
    } else {
        return 0;
    }
}

void printInstruction(long answer1)
{
    int k,j,i = 1;
    char hexadecimal[9];

    unsigned int temp;
    unsigned int answer = answer1;

    while(answer != 0) 
    {
        temp = answer % 16;

        if (temp < 10) {
            temp = temp + 48;
        } else {
            temp = temp + 55;
        }

        hexadecimal[i++] = temp;
        answer = answer / 16;
    }
    
    printf("0x");
    for(k = 8 - i; k >= 0; k--) {
        printf("0");
    }

    for(j = i - 1; j > 0; j--) {
        printf("%c", hexadecimal[j]);
    }
    printf("\n");

}

long iType(long op, long rs, long rt, long immed) 
{

    long opShift = 26;
    long rsShift = 21;
    long rtShift = 16;
    long final = 0;
    long immedShift = 65535;

    op = op << opShift;
    rs = rs << rsShift;
    rt = rt << rtShift;
    immed = immed & immedShift;

    final = op | rs;
    final = final | rt;
    final = final | immed;

    return final;
}

long jType(long op, long currentPC, long targaddr)
{
    long addressShift = 67108863;
    long opShift = 26;
    long targShift = 2;
    long address = ((targaddr - currentPC)/4);
    long answer = 0;

    op = op << opShift;
    targaddr = targaddr >> targShift;
    answer = op | targaddr;

    return answer;
}

int saveLabel(struct Label* arr, int LabelSize, char* token, int pc)
{
    strcpy(arr[LabelSize].name, token);
    arr[LabelSize].address = pc;
    return 1;
}

long getLabel(struct Label* arr, int LabelSize, char* token)
{
    char label[100];
    char* colon = ":";
    strcpy(label, token);
    strcat(label, colon); 
    int i = 0;
    
    for(i = 0; i < LabelSize; i++) {
    
        if (strcmp(arr[i].name, label) == 0) {
            return arr[i].address;
        }
        
    }

    return 0;
}

long getImmed(long dest, long branch)
{
    return (dest - branch)/4;
}

long getUpper(long address)
{
    long addressShift = 4294901760;
    long answer = 0;

    answer = address & addressShift;
    answer = address >> 16;

    return answer;
}

long getLower(long address)
{
    long addressShift = 65535;
    long answer = 0;

    answer = address & addressShift;
    
    return answer;
}
