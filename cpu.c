/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */

  int temp[5];
  int flag = 0, numOfStall =0;int write = 0;
  int pc =0, pcval = 0; int samePc = 0;
  int rd_val = 0;int onetime = 0;int p[20];int i=0;
  IQ iq[8];
  ROB rob[12];int tailROB = 0;int headROB = 0;
int rob_count = 0;
int iq_count = 0;
//IQ iq[8];


APEX_CPU* APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
//  iq->iq_count = 0;
  memset(cpu->regs, 0, sizeof(int) * 32);
  //memset(cpu->iq, 0, sizeof(IQ) * 8);
  memset(p, -1, sizeof(int) * 32);
  //memset(iq, 0, sizeof(int) * 8);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);


  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);



  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2","rs3", "imm");


    /*
    code_memory[] object array have all those values from file in form of object variable.
    cpu->code_memory_size is number of instruction in that file.
    */



    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].rs3,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
  }

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "STR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
  if (strcmp(stage->opcode, "LDR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

   if (strcmp(stage->opcode, "BZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "ADDL") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd,stage->rs1, stage->imm);
  }
  if (strcmp(stage->opcode, "SUBL") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd,stage->rs1, stage->imm);
  }
  if (strcmp(stage->opcode, "ADD") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "AND") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }

   if (strcmp(stage->opcode, "SUB") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "MUL") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
}
/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];

  if (!stage->busy && !stage->stalled) {

    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */

    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];



    strcpy(stage->opcode, current_ins->opcode);

    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;



    /* Update PC for next instruction */
  //  if(flag==0){
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/


    if(strcmp(stage->opcode, "") == 0)
    {
  //  memset(stage, 0, sizeof(CPU_Stage));
    //cpu->stage[DRF] = cpu->stage[F];
    print_stage_content("Fetch", stage);
    return 0;

  }


    if(stage->pc >4)
    cpu->stage[DRF] = cpu->stage[F];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

int decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];


   pc = stage->pc;
   temp[rd_val] = stage->rd;
       /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    for(int i=0;i<rd_val;i++)
      {
	       if(stage->rs1==temp[i] || stage->rs2==temp[i]){
    	      flag = 1;
      }
    }
    }

    if (strcmp(stage->opcode, "STR") == 0) {
    for(int i=0;i<rd_val;i++)
      {
	if(stage->rs1==temp[i] || stage->rs2==temp[i] || stage->rs3==temp[i] ){
    	flag = 1;
      }
    }
    }
/* Read data from register file for store */
    if (strcmp(stage->opcode, "LOAD") == 0) {
    for(int i=0;i<rd_val;i++)
      {
	if(stage->rs1==temp[i]){
    	flag = 1;

      }
    }
    }

 if (strcmp(stage->opcode, "LDR") == 0) {
    for(int i=0;i<rd_val;i++)
      {
	if(stage->rs1==temp[i]  || stage->rs2==temp[i]){
    	flag = 1;

      }
    }
  }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {

    }

    /*if (strcmp(stage->opcode, "BZ") == 0 && onetime ==0) {
    cpu->pc = cpu->pc + stage->imm;
    printf("%d",cpu->pc);
    onetime++;
    }*/

     if (strcmp(stage->opcode, "ADD") == 0) {
    	for(int i=0;i<rd_val;i++)
      	{
		if(stage->rs1==temp[i] || stage->rs2==temp[i] ){
    		flag = 1;

      	}
    	}
    }
     if (strcmp(stage->opcode, "SUB") == 0) {


    	for(int i=0;i<rd_val;i++)
      	{
		if(stage->rs1==temp[i] || stage->rs2==temp[i] ){
    		flag = 1;

      	}
    	}
    }
    if (strcmp(stage->opcode, "AND") == 0) {
    	for(int i=0;i<rd_val;i++)
      	{
		if(stage->rs1==temp[i] || stage->rs2==temp[i] ){
    		flag = 1;

      	}
    	}
    }


     if (strcmp(stage->opcode, "ADDL") == 0) {
      for(int i=0;i<rd_val;i++)
      {
	       if(stage->rs1==temp[i]){
    	      flag = 1;

      }
    }
   }

if (strcmp(stage->opcode, "SUBL") == 0) {
      for(int i=0;i<rd_val;i++)
      {
	       if(stage->rs1==temp[i]){
    	      flag = 1;

      }
    }
   }

   if (strcmp(stage->opcode, "MUL") == 0) {
      for(int i=0;i<rd_val;i++)
      {
	       if(stage->rs1==temp[i] || stage->rs2==temp[i] ){
    	      flag = 1;

      }
    }
   }

	if(flag==1){
	   stage->stalled = 1;

     //return 0;
	}
    /* Copy data from decode latch to execute latch*/
    //if(flag==0)

    cpu->stage[STATES] = cpu->stage[DRF];
    if (ENABLE_DEBUG_MESSAGES ) {
      print_stage_content("Instruction at DECODE_RF_STAGE --->", stage);
      rd_val++;
    }

  return 0;
}
/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */




int stateCall (APEX_CPU* cpu){

CPU_Stage* stage = &cpu->stage[STATES];


if(samePc!=stage->pc){
iq[iq_count].iq_pc = stage->pc;
strcpy(iq[iq_count].opcode, stage->opcode);
iq[iq_count].rd = stage->rd;
iq[iq_count].rs1 = stage->rs1;
iq[iq_count].rs2 = stage->rs2;
//iq[iq_count].rs1_value = stage->rs1;
iq[iq_count].imm = stage->imm;
iq[iq_count].rs1Ready = 0;
iq[iq_count].rs2Ready = 0;
iq[iq_count].valid = 1;
iq_count++;


rob[rob_count].rob_pc = stage->pc;
rob[rob_count].rd = stage->rd;
rob[rob_count].rd_ready = 0;
rob[rob_count].rd_value = 0;
rob[rob_count].valid = 1;
//rob[rob_count].opcode = stage->opcode;
strcpy(rob[rob_count].opcode, stage->opcode);
rob_count++;
samePc = stage->pc;
}
 //printf("IQ (%d)\n",iq_count );
      //print_stage_content("", stage);
     for(int i = 0; i < iq_count; i++){

       if(strcmp(iq[i].opcode,"MOVC")==0)
       printf("IQ-->(%d) %s,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
            iq[i].rd,iq[i].imm);


        if(strcmp(iq[i].opcode,"ADDL")==0)
        printf("IQ-->(%d) %s,R%d,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
             iq[i].rd,iq[i].rs1,iq[i].imm);

        if(strcmp(iq[i].opcode,"SUBL")==0)
        printf("IQ-->(%d) %s,R%d,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].imm);

        if(strcmp(iq[i].opcode,"ADD")==0)
        printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].rs2);

        if(strcmp(iq[i].opcode,"SUB")==0)
        printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].rs2);

     //printf("%d\n",iq[i]->iq_count );
     //print_stage_content("", stage);
   }


   for(int i = 0; i < rob_count; i++){

     if(strcmp(rob[i].opcode,"MOVC")==0)
     printf("ROB-->(%d) %s R%d rd_Ready:%d rd_Value:%d\n",rob[i].rob_pc,rob[i].opcode,rob[i].rd, rob[i].rd_ready, rob[i].rd_value
          );


      if(strcmp(rob[i].opcode,"ADDL")==0)
      printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
           rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

     if(strcmp(rob[i].opcode,"SUBL")==0)
       printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
      rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

      if(strcmp(rob[i].opcode,"ADD")==0)
        printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
       rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

       if(strcmp(rob[i].opcode,"SUB")==0)
         printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
        rob[i].rd, rob[i].rd_ready, rob[i].rd_value);
  }

return 0;
}



void iq_removeItem (int i){


iq[i].iq_pc = 0;
strcpy(iq[i].opcode, "");
iq[i].rs1 = 0;
iq[i].rs1Ready = 0;		    // Source-1 Register Address
iq[i].rs2 =0;
iq[i].rs2Ready= 0;
iq[i].rs3 = 0;		    // Source-2 Register Address
iq[i].rd = 0;		    // Destination Register Address
iq[i].imm = 0;
iq[i].valid = 0;


}






int issueInstruction(APEX_CPU* cpu) {

for(int i = 0; i<iq_count; i++){
if(iq[i].valid) {
  //MOVC
  if(strcmp(iq[i].opcode,"MOVC")==0){
    strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
    cpu->stage[INT_FU1].pc = iq[i].iq_pc;
    cpu->stage[INT_FU1].rd = iq[i].rd;
    cpu->stage[INT_FU1].imm = iq[i].imm;

    iq_removeItem (i);
  }

// ADDL
  if(strcmp(iq[i].opcode,"ADDL")==0){
      if(iq[i].rs1Ready){
      strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
      cpu->stage[INT_FU1].pc = iq[i].iq_pc;
      cpu->stage[INT_FU1].rd = iq[i].rd;
      cpu->stage[INT_FU1].rs1 = iq[i].rs1;
      cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
      cpu->stage[INT_FU1].imm = iq[i].imm;

  iq_removeItem (i);
    }
}
// SUBL
    if(strcmp(iq[i].opcode,"SUBL")==0){
        if(iq[i].rs1Ready){
        strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
        cpu->stage[INT_FU1].pc = iq[i].iq_pc;
        cpu->stage[INT_FU1].rd = iq[i].rd;
        cpu->stage[INT_FU1].rs1 = iq[i].rs1;
        cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
        cpu->stage[INT_FU1].imm = iq[i].imm;

    iq_removeItem (i);
      }
  }
//ADD
  if(strcmp(iq[i].opcode,"ADD")==0){
      if(iq[i].rs1Ready && iq[i].rs2Ready  ){
      strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
      cpu->stage[INT_FU1].pc = iq[i].iq_pc;
      cpu->stage[INT_FU1].rd = iq[i].rd;
      cpu->stage[INT_FU1].rs1 = iq[i].rs1;
      cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
      cpu->stage[INT_FU1].rs2 = iq[i].rs2;
      cpu->stage[INT_FU1].rs2_value = iq[i].rs2_value;

  iq_removeItem (i);
    }
  }
//SUB
  if(strcmp(iq[i].opcode,"SUB")==0){
      if(iq[i].rs1Ready && iq[i].rs2Ready){
      strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
      cpu->stage[INT_FU1].pc = iq[i].iq_pc;
      cpu->stage[INT_FU1].rd = iq[i].rd;
      cpu->stage[INT_FU1].rs1 = iq[i].rs1;
      cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
      cpu->stage[INT_FU1].rs2 = iq[i].rs2;
      cpu->stage[INT_FU1].rs2_value = iq[i].rs2_value;

  iq_removeItem (i);
    }
  }











}

}

return 0;

}


int intFU1(APEX_CPU* cpu)
{

  CPU_Stage* stage = &cpu->stage[INT_FU1];

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
     stage->buffer = cpu->regs[stage->rs2]+stage->imm;


    }
    if (strcmp(stage->opcode, "STR") == 0) {
     stage->buffer = cpu->regs[stage->rs2]+cpu->regs[stage->rs3];


    }


    if (strcmp(stage->opcode, "LOAD") == 0) {
     stage->buffer = cpu->regs[stage->rs1]+stage->imm;


    }
    if (strcmp(stage->opcode, "LDR") == 0) {
     stage->buffer = cpu->regs[stage->rs1]+cpu->regs[stage->rs2];


    }


    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    stage->buffer = stage->imm;

    }


    if (strcmp(stage->opcode, "ADDL") == 0) {
     stage->buffer= stage->rs1_value+stage->imm;
    // printf("%d\n", stage->rs1_value);

    }


    if (strcmp(stage->opcode, "SUBL") == 0) {
      stage->buffer= stage->rs1_value-stage->imm;

    }


    if (strcmp(stage->opcode, "ADD") == 0) {
    stage->buffer = stage->rs1_value+stage->rs2_value;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
    stage->buffer = stage->rs1_value-stage->rs2_value;

    }



     if (strcmp(stage->opcode, "AND") == 0) {
    stage->buffer = cpu->regs[stage->rs1]&cpu->regs[stage->rs2];

    }
    if (strcmp(stage->opcode, "OR") == 0) {
    stage->buffer = cpu->regs[stage->rs1]|cpu->regs[stage->rs2];

    }



    if (strcmp(stage->opcode, "MUL") == 0) {
    stage->buffer = cpu->regs[stage->rs1]*cpu->regs[stage->rs2];
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[INT_FU2] = cpu->stage[INT_FU1];

    if (ENABLE_DEBUG_MESSAGES) {
         print_stage_content("Instruction at INT1_FU_STAGE ---> ", stage);
         }


  return 0;
}
int intFU2(APEX_CPU* cpu)

{
  CPU_Stage* stage = &cpu->stage[INT_FU2];

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->rd_value = stage->buffer;

    }
    if (strcmp(stage->opcode, "ADDL") == 0) {
    stage->rd_value = stage->buffer;

    }

    if (strcmp(stage->opcode, "ADD") == 0) {
    stage->rd_value = stage->buffer;

    }

    if (strcmp(stage->opcode, "SUB") == 0) {
    stage->rd_value = stage->buffer;

    }


    if (strcmp(stage->opcode, "SUBL") == 0) {
    stage->rd_value = stage->buffer;

    }







    // if (strcmp(stage->opcode, "BZ") == 0 && onetime ==0) {
    // cpu->pc = stage->pc + stage->imm;
    // cpu->stage[MEM1] = cpu->stage[INT_FU2];
    //
    // memset(stage, 0, sizeof(CPU_Stage));
    // memset(ex1_stage, 0, sizeof(CPU_Stage));
    // memset(decode_stage, 0, sizeof(CPU_Stage));
    // // memset(decode_stage, 0, sizeof(CPU_Stage));
    // onetime++;
    // }
    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[WB] = cpu->stage[INT_FU2];

    if (ENABLE_DEBUG_MESSAGES) {

      print_stage_content("Instruction at INT2_FU_STAGE --->", stage);

    }



  return 0;

}







/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
/*
int memory1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];



  	if(stage->stalled){
	printf("(STALL)\n");
	cpu->stage[MEM2] = cpu->stage[MEM1];
	}



  if (!stage->busy && !stage->stalled) {


    if (strcmp(stage->opcode, "STORE") == 0) {

    cpu->data_memory[stage->buffer] = cpu->regs[stage->rs1];
  ;

    }


    if (strcmp(stage->opcode, "STR") == 0) {

    cpu->data_memory[stage->buffer] = cpu->regs[stage->rs1];

    }




    if (strcmp(stage->opcode, "LOAD") == 0) {


     cpu->regs[stage->rd] = cpu->data_memory[stage->buffer];


    }

    if (strcmp(stage->opcode, "LDR") == 0) {


     cpu->regs[stage->rd] = cpu->data_memory[stage->buffer];


    }

    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES) {
      //print_stage_content("Memory1", stage);
    }
  }


  return 0;
}
int memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];


  if(stage->stalled){
	printf("(STALL)\n");
	cpu->stage[WB] = cpu->stage[MEM2];
	}



  if (!stage->busy && !stage->stalled) {

    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES) {
      //print_stage_content("Memory2", stage);
    }
  }

  return 0;
}


int mulFU (APEX_CPU* cpu){

CPU_Stage* stage = &cpu->stage[WB];
 if(stage->stalled){
	printf("MUL_FU_STAGE(STALL) \n");
	}
 if (!stage->busy && !stage->stalled)
  {
	 if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Instruction at MUL_FU_STAGE--> ", stage);

    }
   }
   return 0;

}


int branchFU(APEX_CPU* cpu) {
CPU_Stage* stage = &cpu->stage[WB];
 if(stage->stalled){
	printf("BRANCH_FU_STAGE(STALL) \n");
	}
 if (!stage->busy && !stage->stalled)
  {
	 if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Instruction at BRANCH_FU_STAGE---> ", stage);

    }
   }
   return 0;
}
*/



void commitROB (APEX_CPU* cpu){

  if (rob[headROB].rd_ready)
  {
    cpu->regs[rob[headROB].rd] = rob[headROB].rd_value;


    for (int i = 0;i<iq_count;i++){

      if (strcmp(iq[i].opcode, "ADDL") == 0) {
        if(iq[i].rs1 == rob[headROB].rd){
          iq[i].rs1_value = rob[headROB].rd_value;
          iq[i].rs1Ready = 1;
        }
      }

      if (strcmp(iq[i].opcode, "SUBL") == 0) {
        if(iq[i].rs1 == rob[headROB].rd){
          iq[i].rs1_value = rob[headROB].rd_value;
          iq[i].rs1Ready = 1;
        }
      }

      if (strcmp(iq[i].opcode, "SUB") == 0) {
        if(iq[i].rs1 == rob[headROB].rd){
          iq[i].rs1_value = rob[headROB].rd_value;
          iq[i].rs1Ready = 1;
          
        }

        if(iq[i].rs2 == rob[headROB].rd){
          iq[i].rs2_value = rob[headROB].rd_value;
          iq[i].rs2Ready = 1;
        }
      }

      if (strcmp(iq[i].opcode, "ADD") == 0) {
        if(iq[i].rs1 == rob[headROB].rd){
          iq[i].rs1_value = rob[headROB].rd_value;
          iq[i].rs1Ready = 1;
        }

        if(iq[i].rs2 == rob[headROB].rd){
          iq[i].rs2_value = rob[headROB].rd_value;
          iq[i].rs2Ready = 1;
        }
      }





// similarly or othere instruction depending on rs1, rs2


    }


    rob[headROB].valid = 0;
    rob[headROB].rob_pc = 0;
    rob[headROB].rd = 0;
    rob[headROB].rd_value = 0;
    rob[headROB].rd_ready = 0;
    strcpy(rob[headROB].opcode, "");
    headROB ++;

  }


}


int writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];

for (int i = 0; i < rob_count; i++){

if (rob[i].valid)
{
  if (rob[i].rob_pc == stage->pc ){

    rob[i].rd_value = stage->rd_value;
    rob[i].rd_ready = 1;

  }
}

}




    /* Update register file */
    // if(strcmp(stage->opcode, "STORE") != 0 && strcmp(stage->opcode, "LOAD") != 0
    // && strcmp(stage->opcode, "STR") != 0 && strcmp(stage->opcode, "LDR") != 0 && stage->pc !=0 && strcmp(stage->opcode, "BZ") != 0)
    //   {

        //cpu->regs[stage->rd] = stage->buffer;
      //printf("R%d %d ",stage->rd,stage->buffer);

       //Ask
     // for(int i=0;i<rd_val;i++)
     //  {
	   //     if(stage->rd==temp[i]){
    	//       temp[i] = -1;
    	// }
     //  }
     //  //flag = 0;
     //  //write = 1;
      cpu->ins_completed++;
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("writeback---> ", stage);
    }


  return 0;
}


void displayFiles(APEX_CPU* cpu){

printf("\n =============== STATE OF ARCHITECTURAL REGISTER FILE ========== \n");
for(int i = 0; i<16;i++ )
printf("REG[%d] VALUE: %d \n",i,cpu->regs[i]);

}


void memoryFiles(APEX_CPU* cpu){
printf("\n============== STATE OF DATA MEMORY =============\n");
for(int i =0;i<99;i++)
printf("MEM[%d] VALUE: %d \n",i,cpu->data_memory[i]);


}







/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int APEX_cpu_run(APEX_CPU* cpu,char  const* first,char  const* second )
{
  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->clock == atoi(second)) {
      printf("(apex) >> Simulation Complete");


      memoryFiles(cpu);
      displayFiles(cpu);

      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

commitROB(cpu);
    writeback(cpu);
    //memory2(cpu);
    //memory1(cpu);

        //memFU(cpu);
        //branchFU(cpu);
        //mulFU(cpu);


        //memory2(cpu);
        //memory1(cpu);

        intFU2(cpu);
	      intFU1(cpu);



        issueInstruction(cpu);

        stateCall(cpu);

        decode(cpu);
        fetch(cpu);



    cpu->clock++;
  }

  return 0;
}
