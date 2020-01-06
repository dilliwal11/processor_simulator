#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */

enum
{
  F,
  DRF,
  STATES,
  INT_FU1,
  INT_FU2,
  MUL_FU,
  WB,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128];	// Operation Code
  int rd;		    // Destination Register Address
  int rs1;		    // Source-1 Register Address
  int rs2;
  int rs3;		    // Source-2 Register Address
  int imm;		    // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;		    // Program Counter
  char opcode[128];	// Operation Code
  int rs1;		    // Source-1 Register Address
  int rs2;
  int rs3;		    // Source-2 Register Address
  int rd;		    // Destination Register Address
  int imm;		    // Literal Value
  int rs1_value;	// Source-1 Register Value
  int rs2_value;
  int rd_value;	// Source-2 Register Value
  int buffer;		// Latch to hold some value
  int mem_address;	// Computed Memory Address
  int busy;		    // Flag to indicate, stage is performing some action
  int stalled;		// Flag to indicate, stage is stalled
} CPU_Stage;




typedef struct IQ{

int iq_pc;
char opcode[128];
int rs1;
int rs1_value;
int rs1Ready;		    // Source-1 Register Address
int rs2;
int rs2_value;
int rs2Ready;
int rs3;		    // Source-2 Register Address
int rd;		    // Destination Register Address
int imm;
int valid;

} IQ;

typedef struct ROB{

int rob_pc;
char opcode[128];

// int rs1;
// int rs1Ready;		    // Source-1 Register Address
// int rs2;
// int rs2Ready;
//int rs3;		    // Source-2 Register Address
int rd;
int rd_value;
int rd_ready;
int valid;	    // Destination Register Address
//int imm;

} ROB;





















/* Model of APEX CPU */
typedef struct APEX_CPU
{
  /* Clock cycles elasped */
  int clock;

  /* Current program counter */
  int pc;

  /* Integer register file */
  int regs[32];
  int regs_valid[32];

  /* Array of 5 CPU_stage */
  CPU_Stage stage[7];


//  IQ iq[80];

  /* Code Memory where instructions are stored */
  APEX_Instruction* code_memory;
  int code_memory_size;

  /* Data Memory */
  int data_memory[4096];

  /* Some stats */
  int ins_completed;

} APEX_CPU;

APEX_Instruction*
create_code_memory(const char* filename, int* size);

APEX_CPU*
APEX_cpu_init(const char* filename);

int
APEX_cpu_run(APEX_CPU* cpu,char const* first,char  const* second);

void
APEX_cpu_stop(APEX_CPU* cpu);

int
fetch(APEX_CPU* cpu);

int
decode(APEX_CPU* cpu);

int
mulFU(APEX_CPU* cpu);

int
execute1(APEX_CPU* cpu);

int
execute2(APEX_CPU* cpu);

int
memory1(APEX_CPU* cpu);

int
memory2(APEX_CPU* cpu);

int
writeback(APEX_CPU* cpu);

#endif
