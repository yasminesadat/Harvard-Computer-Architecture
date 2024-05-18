#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

// Global Variables
uint8_t registerFile[64];
uint16_t instructionMemory[1024];
uint8_t dataMemory[2048];
uint16_t storeIndex = 0;
uint16_t PC = 0;
uint8_t SREG;

// Global Variables From Decode Phase
uint8_t opcode;
uint8_t r1;
uint8_t valueR1;
uint8_t r2;
uint8_t valueR2;
uint8_t imm;

int pcValues[2];

// Method Signatures
uint16_t fetch();
void decode(uint16_t instruction);
void execute();
void runProgram();
void endPrints();

void storeInstruction(char line[])
{
    char *instructionSegment[3];
    char *token = strtok(line, " ");
    for (int i = 0; i < 3; i++)
    {
        *(instructionSegment + i) = token;
        token = strtok(NULL, " ");
    }
    // remove the terminating character found in last instruction (end of file)
    instructionSegment[2][strcspn(instructionSegment[2], "\n")] = '\0';
    printf("Loaded Instruction %s %s %s\n", instructionSegment[0], instructionSegment[1], instructionSegment[2]);
    uint16_t instruction;
    char checkOperand = 'I';
    // first field for opcode
    if (strcmp(instructionSegment[0], "ADD") == 0)
    {
        instruction = 0 << 12;
        checkOperand = 'R';
    }
    else if (strcmp(instructionSegment[0], "SUB") == 0)
    {
        instruction = 1 << 12;
        checkOperand = 'R';
    }
    else if (strcmp(instructionSegment[0], "MUL") == 0)
    {
        instruction = 2 << 12;
        checkOperand = 'R';
    }
    else if (strcmp(instructionSegment[0], "LDI") == 0)
        instruction = 3 << 12;
    else if (strcmp(instructionSegment[0], "BEQZ") == 0)
        instruction = 4 << 12;
    else if (strcmp(instructionSegment[0], "AND") == 0)
    {
        instruction = 5 << 12;
        checkOperand = 'R';
    }
    else if (strcmp(instructionSegment[0], "OR") == 0)
    {
        instruction = 6 << 12;
        checkOperand = 'R';
    }
    else if (strcmp(instructionSegment[0], "JR") == 0)
    {
        instruction = 7 << 12;
        checkOperand = 'R';
    }
    else if (strcmp(instructionSegment[0], "SLC") == 0)
        instruction = 8 << 12;
    else if (strcmp(instructionSegment[0], "SRC") == 0)
        instruction = 9 << 12;
    else if (strcmp(instructionSegment[0], "LB") == 0)
        instruction = 10 << 12;
    else if (strcmp(instructionSegment[0], "SB") == 0)
        instruction = 11 << 12;
    uint8_t num = atoi(++(instructionSegment[1]));

    // second field containing register
    instruction += (num << 6);

    // third field containing another register or immediate value
    switch (checkOperand)
    {
    case 'R':
        instruction += atoi(++(instructionSegment[2]));
        break;
    case 'I':
        // check negative case
        if (*instructionSegment[2] == '-')
        {
            uint8_t positive = atoi(++(instructionSegment[2]));
            instruction += (((uint8_t)((~positive) + 1)) & (0b00111111));
        }
        else
        {
            instruction += atoi(instructionSegment[2]);
        }
        break;
    }
    printf("Instruction stored as %i\n\n", instruction);
    instructionMemory[storeIndex++] = instruction;
}

void loadInstructions()
{
    FILE *filePointer;
    char line[20]; // Assuming maximum line length is 20 characters

    // Open the file in read mode
    filePointer = fopen("AssemblyCode.txt", "r");

    // Check if file opened successfully
    if (filePointer == NULL)
    {
        printf("File could not be opened.");
    }

    // Read lines until end of file
    while (fgets(line, sizeof(line), filePointer) != NULL)
    {
        // Store line in instruction memory
        storeInstruction(line);
    }

    // Close the file
    fclose(filePointer);
}

// First Stage: fetch
uint16_t fetch()
{
    printf("PC value to be fetched: %i\n", PC);
    return instructionMemory[PC++];
}

// Second Stage: decode
void decode(uint16_t instruction)
{
    opcode = (instruction & 0b1111000000000000) >> 12;
    r1 = (instruction & 0b0000111111000000) >> 6;
    r2 = (instruction & 0b0000000000111111);
    imm = (instruction & 0b0000000000111111);
    // negative case
    // append 2 sign bits to unify with number of bits coming from the registers
    if (imm >> 5 == 1)
    {
        imm = imm | 0b11000000;
    }
    printf("~ ~ ~ ~ ~ ~ ~ \n");
    printf("opcode is %i\n", opcode);
    printf("r1 is R%i ", r1);
    valueR1 = registerFile[r1];
    printf("with value %i\n", valueR1);
    printf("r2 is R%i ", r2);
    valueR2 = registerFile[r2];
    printf("with value %i\n", valueR2);
    printf("imm is %i\n", imm);
    printf("~ ~ ~ ~ ~ ~ ~ \n");
}

// Third Stage: execute
void execute()
{
    printf("~ ~ ~ ~ ~ ~ ~ \nExecute Stage Inputs: \n");
    printf("opcode is %i\n", opcode);
    printf("r1 is R%i ", r1);
    printf("with value %i\n", valueR1);
    printf("r2 is R%i ", r2);
    printf("with value %i\n", valueR2);
    printf("imm is %i\n", imm);
    printf("~ ~ ~ ~ ~ ~ ~ \n");
    uint16_t result;
    uint16_t imm16;
    switch (opcode)
    {
    case 0: // ADD
        result = valueR1 + valueR2;
        uint8_t addneg = (result & 0b0000000010000000) >> 7;
        uint8_t addcarry = (result & 0b0000000100000000) >> 8;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        if (addcarry == 1) // Carry Flag Check
        {
            SREG = SREG | 0b00010000;
        }
        if (addneg == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        uint8_t addresbit7 = (result & 0b0000000010000000) >> 7;
        uint8_t addop1bit7 = (valueR1 & 0b10000000) >> 7;
        uint8_t addop2bit7 = (valueR2 & 0b10000000) >> 7;
        uint8_t addxorResFin = addcarry ^ addresbit7 ^ addop1bit7 ^ addop2bit7;
        if (addxorResFin == 1) // Overflow Flag Check
        {
            SREG = SREG | 0b00001000;
        }
        if ((addneg ^ addxorResFin) == 1) // Sign Flag Check
        {
            SREG = SREG | 0b00000010;
        }
        registerFile[r1] = result;
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 1: // SUB
        result = valueR1 - valueR2;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        uint16_t subneg = (result & 0b0000000010000000) >> 7;
        if (subneg == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        uint16_t subresbit8 = (result & 0b0000000100000000) >> 8;
        uint16_t subresbit7 = (result & 0b0000000010000000) >> 7;
        uint8_t subop1bit7 = (valueR1 & 0b10000000) >> 7;
        uint8_t subop2bit7 = (valueR2 & 0b10000000) >> 7;
        uint8_t subxorRes1 = subresbit8 ^ subresbit7;
        uint8_t subxorRes2 = subop1bit7 ^ subop2bit7;
        uint8_t subxorResFin = subxorRes1 ^ subxorRes2;
        if (subxorResFin == 1) // Overflow Flag Check
        {
            SREG = SREG | 0b00001000;
        }
        if ((subneg ^ subxorResFin) == 1) // Sign Flag Check
        {
            SREG = SREG | 0b00000010;
        }
        registerFile[r1] = result;
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 2: // MUL
        result = valueR1 * valueR2;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        if (((result & 0b0000000010000000) >> 7) == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        registerFile[r1] = result;
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 3: // LDI
        registerFile[r1] = imm;
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 4: // BEQZ
        // 16 bit version of immediate
        imm16 = imm;
        if (imm >> 7 == 1)
        {
            imm16 = imm16 | 0b1111111100000000;
        }
        PC = (valueR1 == 0) ? ((uint16_t)pcValues[1]) + 1 + imm16 : PC;
        printf("PC is %i \n", PC);
        break;
    case 5: // AND
        result = valueR1 & valueR2;
        registerFile[r1] = result;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        if (((result & 0b0000000010000000) >> 7) == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 6: // OR
        result = valueR1 | valueR2;
        registerFile[r1] = result;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        if (((result & 0b0000000010000000) >> 7) == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 7: // JR
        result = (valueR1 << 8) + (valueR2);
        PC = result;
        printf("PC is %i \n", PC);
        break;
    case 8: // SLC
        result = (valueR1 << imm) | (valueR1 >> (8 - imm));
        registerFile[r1] = result;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        if (((result & 0b0000000010000000) >> 7) == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 9: // SRC
        result = (valueR1 >> imm) | (valueR1 << (8 - imm));
        registerFile[r1] = result;
        if (result == 0) // Zero Flag Check
        {
            SREG = SREG | 0b00000001;
        }
        if (((result & 0b0000000010000000) >> 7) == 1) // Negative Flag Check
        {
            SREG = SREG | 0b00000100;
        }
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 10: // LB
        registerFile[r1] = dataMemory[imm];
        printf("R%i is %i \n", r1, registerFile[r1]);
        break;
    case 11: // SB
        dataMemory[imm] = valueR1;
        printf("Data Memory of %i is %i \n", imm, dataMemory[imm]);
        break;
    }
    printf("Status Register Value is %i\n", SREG);
    SREG = 0;
}

void runProgram()
{
    printf("Loading Instructions...\n\n");
    loadInstructions();
    if (storeIndex == 0)
    {
        printf("No loaded instructions.");
        return;
    }
    uint16_t fetchedInstruction;
    unsigned int maxClockCycles = 3 + (storeIndex - 1);
    printf("MAX Clock cycles with no jumps or branches: %i\n", maxClockCycles);
    unsigned int cycle = 1;
    // ready for first and second cycle
    pcValues[0] = 1;
    pcValues[1] = 0;
    while (1)
    {
        printf("-----------------\n");
        printf("Start of Cycle %i\n", cycle);
        printf("PC value is %i\n", PC);
        if (cycle == 1)
        {
            fetchedInstruction = fetch();
            printf("Fetching first Instruction: %i\n", fetchedInstruction);
        }
        else if (cycle == 2)
        {
            printf("Decoding Instruction: %i\n", instructionMemory[pcValues[1]]);
            decode(fetchedInstruction);
            if (PC < storeIndex)
            {
                fetchedInstruction = fetch();
                printf("Fetching Instruction: %i\n", fetchedInstruction);
            }
            else
            {
                printf("No more instructions to fetch at this stage\n");
                pcValues[0] = -1;
            }
        }
        else
        {
            printf("Executing Instruction: %i\n", instructionMemory[pcValues[1]]);
            execute();

            // successful jump or branch
            // 2 cycles like the start in the flush logic
            if ((opcode == 4 && valueR1 == 0) || (opcode == 7))
            {
                if (PC >= storeIndex)
                {
                    printf("End of Program\n");
                    endPrints();
                    return;
                }
                printf("-----------------\n");
                printf("Start of Cycle %i\n", ++cycle);
                printf("PC value is %i\n", PC);
                fetchedInstruction = fetch();
                printf("Fetching Instruction: %i\n", fetchedInstruction);
                pcValues[1] = PC - 1;

                printf("-----------------\n");
                printf("Start of Cycle %i\n", ++cycle);
                printf("PC value is %i\n", PC);
                printf("Decoding Instruction: %i\n", instructionMemory[pcValues[1]]);
                decode(fetchedInstruction);
                if (PC < storeIndex)
                {
                    fetchedInstruction = fetch();
                    printf("Fetching Instruction: %i\n", fetchedInstruction);
                    pcValues[0] = PC - 1;
                }
                else
                {
                    printf("No more instructions to fetch at this stage\n");
                    pcValues[0] = -1;
                }
            }
            else
            {
                if (pcValues[0] != -1)
                {
                    printf("Decoding Instruction: %i\n", instructionMemory[pcValues[0]]);
                    decode(fetchedInstruction);
                }
                else
                {
                    printf("No more instructions to decode at this stage\n");
                }
                if (PC < storeIndex)
                {
                    fetchedInstruction = fetch();
                    printf("Fetching Instruction: %i\n", fetchedInstruction);
                    // shift array
                    pcValues[1] = pcValues[0];
                    pcValues[0] = PC - 1;
                }
                else
                {
                    printf("No more instructions to fetch at this stage\n");
                    // shift array
                    pcValues[1] = pcValues[0];
                    pcValues[0] = -1;
                }
            }
        }
        // no instructions to execute
        if (pcValues[1] == -1)
        {
            printf("End of Program\n");
            endPrints();
            return;
        }
        cycle++;
    }
}

void endPrints()
{
    printf("PC value is %i\n", PC);
    printf("Status Register Value is %i\n", SREG);
    printf("-----------------\nREGISTER FILE\n");
    for (int i = 0; i < 64; i++)
    {
        printf("R%i : %i\n", i, registerFile[i]);
    }
    printf("-----------------\nINSTRUCTION MEMORY\n");
    for (int i = 0; i < 1024; i++)
    {
        printf("Address %i : %i\n", i, instructionMemory[i]);
    }
    printf("-----------------\nDATA MEMORY\n");
    for (int i = 0; i < 2048; i++)
    {
        printf("Address %i : %i\n", i, dataMemory[i]);
    }
}

int main()
{
    runProgram();
}