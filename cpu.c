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

  //int temp[5];
  int flag = 0, numOfStall =0;int write = 0;
  int pc =0, pcval = 0; int samePc = 0;
  int rd_val = 0;int onetime = 0;int p[20];int i=0;
  IQ iq[8];
   RenameTable rt[24]; int headRT = 0;
   int first_time = 1;int equal = 0;
  ROB rob[24];int tailROB = 0;int headROB = 0;
  LSQ lsq[6]; int lsq_count = 0;
int rob_count = 0;
int iq_count = 0; int rt_count = 0;
int sameDest = 0;
int rob_check_ready_to_issue = 0;
int samePcWB = 0;
int z = 0;
int flush_branch =0;
//int mulCall = 0;
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

  memset(cpu->regs_valid, 0, sizeof(int) * 32);
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
//cpu->data_memory[2]=5;
//cpu->data_memory[3]=5;
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
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
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

   if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
   printf("%s,R%d,#%d ", stage->opcode,stage->rs1, stage->imm);
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

  if (strcmp(stage->opcode, "OR") == 0) {
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

    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;


  /*  if(strcmp(stage->opcode,"")==0){

      return 0;
    }*/

    if (strcmp(cpu->stage[F].opcode,"")!=0)
    {

    }
    
      cpu->pc += 4;

      if (flush_branch){
        cpu->stage[F].pc = 0;
        cpu->stage[F].rs1 = 0;
        cpu->stage[F].rs2 = 0;
        strcpy(cpu->stage[F].opcode, "");
        //cpu->stage[DRF] = cpu->stage[F];

      }




    cpu->stage[DRF] = cpu->stage[F];





    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch --->", stage);
    }

    cpu->stage[F].pc = 0;
    cpu->stage[F].rs1 = 0;
    cpu->stage[F].rs2 = 0;
    strcpy(cpu->stage[F].opcode, "");


  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

void showRt(){
  printf("\n~~~~~~~~Rename Table~~~~~~~~~\n" );

     for (int k = headRT;k<rt_count; k++){

      printf("R[%d]--->P%d\n",rt[k].p_rd,rt[k].p_id);
     }
   printf("~~~~~~~~~~~~~~~~~\n" );



}




int decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];


    pc = stage->pc;

    if (strcmp(stage->opcode,"")==0){
        //showRt();

       //cpu->stage[STATES] = cpu->stage[DRF];
     return 0;
//    cpu->stage[STATES] = cpu->stage[DRF];

          }
          if (flush_branch){
            cpu->stage[DRF].pc = 0;
            cpu->stage[DRF].rs1 = 0;
            cpu->stage[DRF].rs2 = 0;
            strcpy(cpu->stage[DRF].opcode, "");
            flush_branch = 0;
            //cpu->stage[DRF] = cpu->stage[DRF];

          }









    if(strcmp(stage->opcode,"STR") != 0 || strcmp(stage->opcode,"STORE") != 0 || strcmp(stage->opcode,"BZ")!= 0)
    {


    for (int i = headRT;i<rt_count; i++){
          if(rt[i].p_rd == stage->rd )
          {
            equal = 1;
            break;
          }

    }

    if(!equal ){
      rt[rt_count].p_rd =  stage->rd;
      rt[rt_count].p_id = rt_count;
      rt_count++;

      //printf("p_rd: %d rd: %d p: %d \n",rt[rt].p_rd ,stage->rd,rt[i].p_id );
    }
    equal =0;
    }
//showRt();



    cpu->stage[STATES] = cpu->stage[DRF];
    if (ENABLE_DEBUG_MESSAGES ) {
      print_stage_content("DECODE_RF_STAGE --->", stage);
      rd_val++;
    }






    cpu->stage[DRF].pc = 0;
    cpu->stage[DRF].rs1 = 0;
    cpu->stage[DRF].rs2 = 0;
    strcpy(cpu->stage[DRF].opcode, "");


  //  showRt();





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
samePc = stage->pc;

rob[rob_count].rob_pc = stage->pc;
rob[rob_count].rd = stage->rd;
rob[rob_count].rd_ready = 0;
rob[rob_count].rd_value = 0;
rob[rob_count].valid = 1;
//rob[rob_count].opcode = stage->opcode;
strcpy(rob[rob_count].opcode, stage->opcode);
rob_count++;



if(strcmp(stage->opcode,"LOAD")==0 || strcmp(stage->opcode,"LDR")==0 || strcmp(stage->opcode,"STORE")==0 || strcmp(stage->opcode,"STR")==0 )
{
  lsq[lsq_count].lsq_pc = stage->pc;
  strcpy(lsq[lsq_count].opcode, stage->opcode);
  lsq[lsq_count].rd = stage->rd;
  lsq[lsq_count].rs1 = stage->rs1;
  lsq[lsq_count].rs2 = stage->rs2;
  //iq[iq_count].rs1_value = stage->rs1;
  lsq[lsq_count].imm = stage->imm;
  lsq[lsq_count].rs1Ready = 0;
  lsq[lsq_count].rs2Ready = 0;
  lsq[lsq_count].rd_value_ready = 0;
  lsq[lsq_count].compute_value = 0;
  lsq[lsq_count].rd_compute_value_ready = 0;
  lsq[lsq_count].valid = 1;
  lsq_count++;
}

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


}
      printf("\n~~~~~~~~IQ~~~~~~~~~\n" );
     for(int i = 0; i < iq_count; i++){

       if(strcmp(iq[i].opcode,"MOVC")==0)
       printf("IQ-->(%d) %s,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
            iq[i].rd,iq[i].imm);


            if(strcmp(iq[i].opcode,"BZ")==0)
            printf("IQ-->(%d) %s ,#%d \n",iq[i].iq_pc,iq[i].opcode,
                 iq[i].imm);
                 if(strcmp(iq[i].opcode,"BNZ")==0)
                 printf("IQ-->(%d) %s ,#%d \n",iq[i].iq_pc,iq[i].opcode,
                      iq[i].imm);


                      if(strcmp(iq[i].opcode,"JUMP")==0)
                      printf("IQ-->(%d) %s ,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,iq[i].rs1,
                           iq[i].imm);

        if(strcmp(iq[i].opcode,"ADDL")==0)
        printf("IQ-->(%d) %s,R%d,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
             iq[i].rd,iq[i].rs1,iq[i].imm);



      if(strcmp(iq[i].opcode,"LOAD")==0)
        printf("IQ-->(%d) %s,R%d,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
          iq[i].rd,iq[i].rs1,iq[i].imm);

      if(strcmp(iq[i].opcode,"STORE")==0)
          printf("IQ-->(%d) %s,R%d,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
            iq[i].rd,iq[i].rs1,iq[i].imm);
      if(strcmp(iq[i].opcode,"LDR")==0)
      printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
          iq[i].rd,iq[i].rs1,iq[i].rs2);

          if(strcmp(iq[i].opcode,"STR")==0)
          printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].rs2);



        if(strcmp(iq[i].opcode,"SUBL")==0)
        printf("IQ-->(%d) %s,R%d,R%d,#%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].imm);

        if(strcmp(iq[i].opcode,"ADD")==0)
        printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].rs2);


              if(strcmp(iq[i].opcode,"AND")==0)
              printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
                    iq[i].rd,iq[i].rs1,iq[i].rs2);

                    if(strcmp(iq[i].opcode,"OR")==0)
                    printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
                          iq[i].rd,iq[i].rs1,iq[i].rs2);

        if(strcmp(iq[i].opcode,"SUB")==0)
        printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
              iq[i].rd,iq[i].rs1,iq[i].rs2);

        if(strcmp(iq[i].opcode,"MUL")==0)
        printf("IQ-->(%d) %s,R%d,R%d,R%d \n",iq[i].iq_pc,iq[i].opcode,
        iq[i].rd,iq[i].rs1,iq[i].rs2);

     //printf("%d\n",iq[i]->iq_count );
     //print_stage_content("", stage);
   }
printf("~~~~~~~~~~~~~~~~~~~\n\n" );

printf("~~~~~~~~ROB~~~~~~~~\n" );
   for(int i = 0; i < rob_count; i++){

     if(strcmp(rob[i].opcode,"MOVC")==0)
     printf("ROB-->(%d) %s R%d rd_Ready:%d rd_Value:%d\n",rob[i].rob_pc,rob[i].opcode,rob[i].rd, rob[i].rd_ready, rob[i].rd_value
          );
          if(strcmp(rob[i].opcode,"BZ")==0 || strcmp(rob[i].opcode,"BNZ")==0)
          printf("ROB-->(%d) %s  \n",rob[i].rob_pc,rob[i].opcode
               );
               if(strcmp(rob[i].opcode,"JUMP")==0)
               printf("ROB-->(%d) %s \n",rob[i].rob_pc,rob[i].opcode
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


       if(strcmp(rob[i].opcode,"AND")==0)
         printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
        rob[i].rd, rob[i].rd_ready, rob[i].rd_value);



               if(strcmp(rob[i].opcode,"OR")==0)
                 printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
                rob[i].rd, rob[i].rd_ready, rob[i].rd_value);



      if(strcmp(rob[i].opcode,"SUB")==0)
         printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
        rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

      if(strcmp(rob[i].opcode,"MUL")==0)
        printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
        rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

      if(strcmp(rob[i].opcode,"LOAD")==0)
        printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
            rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

            if(strcmp(rob[i].opcode,"STORE")==0)
              printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
                  rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

            if(strcmp(rob[i].opcode,"LDR")==0)
              printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
             rob[i].rd, rob[i].rd_ready, rob[i].rd_value);
             if(strcmp(rob[i].opcode,"STR")==0)
               printf("ROB-->(%d) %s,R%d rd_Ready:%d rd_Value:%d \n",rob[i].rob_pc,rob[i].opcode,
              rob[i].rd, rob[i].rd_ready, rob[i].rd_value);

  }


printf("~~~~~~~~~~~~~~~~~~~\n\n" );



printf("~~~~~~~~LSQ~~~~~~~~\n" );

for(int i = 0; i < lsq_count; i++){

  if(strcmp(lsq[i].opcode,"LOAD")==0)
  printf("LSQ-->(%d) %s,R%d,R%d,#%d \n",lsq[i].lsq_pc,lsq[i].opcode,
       lsq[i].rd,lsq[i].rs1,lsq[i].imm);

       if(strcmp(lsq[i].opcode,"STORE")==0)
       printf("LSQ-->(%d) %s,R%d,R%d,#%d \n",lsq[i].lsq_pc,lsq[i].opcode,
            lsq[i].rd,lsq[i].rs1,lsq[i].imm);
       if(strcmp(lsq[i].opcode,"LDR")==0)
       printf("LSQ-->(%d) %s,R%d,R%d,R%d \n",lsq[i].lsq_pc,lsq[i].opcode,
            lsq[i].rd,lsq[i].rs1,lsq[i].rs2);

            if(strcmp(lsq[i].opcode,"STR")==0)
            printf("LSQ-->(%d) %s,R%d,R%d,R%d \n",lsq[i].lsq_pc,lsq[i].opcode,
                 lsq[i].rd,lsq[i].rs1,lsq[i].rs2);
}

printf("~~~~~~~~~~~~~~~~~~~\n\n" );




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

void lsq_removeItem (int i)
{
lsq[i].lsq_pc = 0;
strcpy(lsq[i].opcode, "");
lsq[i].rs1 = 0;
lsq[i].rs1Ready = 0;		    // Source-1 Register Address
lsq[i].rs2 =0;
lsq[i].rs2Ready= 0;
lsq[i].rs3 = 0;		    // Source-2 Register Address
lsq[i].rd = 0;		    // Destination Register Address
lsq[i].imm = 0;
lsq[i].valid = 0;
lsq[i].rd_value_ready =0;
}



void issueInstruction(APEX_CPU* cpu) {

for(int i = 0; i<iq_count; i++){
if(iq[i].valid) {
  //MOVC
  if(strcmp(iq[i].opcode,"MOVC")==0){
    strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
    cpu->stage[INT_FU1].pc = iq[i].iq_pc;
    cpu->stage[INT_FU1].rd = iq[i].rd;
    cpu->stage[INT_FU1].imm = iq[i].imm;

    iq_removeItem (i);
    break;
  }


  if(strcmp(iq[i].opcode,"BZ")==0){
    strcpy(cpu->stage[BRANCH].opcode, iq[i].opcode);
    cpu->stage[BRANCH].pc = iq[i].iq_pc;
    cpu->stage[BRANCH].imm = iq[i].imm;

    iq_removeItem (i);
    break;
  }





// ADDL
  if(strcmp(iq[i].opcode,"ADDL")==0 || strcmp(iq[i].opcode,"SUBL")==0
  || strcmp(iq[i].opcode,"LOAD")==0 || strcmp(iq[i].opcode,"STORE")==0){
    if(cpu->regs_valid[iq[i].rs1])
          {
              for(int j =0;j<rob_count;j++){
                  if(iq[i].rs1!=rob[j].rd) {
                      rob_check_ready_to_issue =1;
                      }
                    }
                if (rob_check_ready_to_issue ){
                  strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
                  cpu->stage[INT_FU1].pc = iq[i].iq_pc;
                  cpu->stage[INT_FU1].rd = iq[i].rd;
                  cpu->stage[INT_FU1].rs1 = iq[i].rs1;
                  cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
                  cpu->stage[INT_FU1].imm = iq[i].imm;
                  rob_check_ready_to_issue =0;
                  iq_removeItem (i);
                  break;
                }

                //printf("ADDL: %d\n",iq[i].rs1Ready );
                if(iq[i].rs1Ready){


                  strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
                  cpu->stage[INT_FU1].pc = iq[i].iq_pc;
                  cpu->stage[INT_FU1].rd = iq[i].rd;
                  cpu->stage[INT_FU1].rs1 = iq[i].rs1;
                  cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
                  cpu->stage[INT_FU1].imm = iq[i].imm;
                  iq_removeItem (i);
                  break;

                }
              }
}
//ADD
  if(strcmp(iq[i].opcode,"ADD")==0 || strcmp(iq[i].opcode,"SUB")==0 ||
  strcmp(iq[i].opcode,"LDR")==0 || strcmp(iq[i].opcode,"STR")==0 || strcmp(iq[i].opcode,"AND")==0 || strcmp(iq[i].opcode,"OR")==0){
    printf("valid: %d\n",iq[i].rs2_value);
    if(cpu->regs_valid[iq[i].rs1] && cpu->regs_valid[iq[i].rs2])
          {
              for(int j =0;j<rob_count;j++){
                  if(iq[i].rs1!=rob[j].rd && iq[i].rs2!=rob[j].rd) {
                      rob_check_ready_to_issue =1;
                      }
                    }

                    if (rob_check_ready_to_issue ){
                    strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
                    cpu->stage[INT_FU1].pc = iq[i].iq_pc;
                    cpu->stage[INT_FU1].rd = iq[i].rd;
                    cpu->stage[INT_FU1].rs1 = iq[i].rs1;
                    cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
                    cpu->stage[INT_FU1].rs2 = iq[i].rs2;
                    cpu->stage[INT_FU1].rs2_value = iq[i].rs2_value;
                      rob_check_ready_to_issue =0;
                  iq_removeItem (i);
                  break;
                }


                if(iq[i].rs1Ready && iq[i].rs2Ready  ){
                  //printf(" sub i:%d\n",i );
                  strcpy(cpu->stage[INT_FU1].opcode, iq[i].opcode);
                  cpu->stage[INT_FU1].pc = iq[i].iq_pc;
                  cpu->stage[INT_FU1].rd = iq[i].rd;
                  cpu->stage[INT_FU1].rs1 = iq[i].rs1;
                  cpu->stage[INT_FU1].rs1_value = iq[i].rs1_value;
                  cpu->stage[INT_FU1].rs2 = iq[i].rs2;
                  cpu->stage[INT_FU1].rs2_value = iq[i].rs2_value;

                  iq_removeItem (i);
                  break;
}
    }
  }

  if(strcmp(iq[i].opcode,"MUL")==0){

    if(cpu->regs_valid[iq[i].rs1] && cpu->regs_valid[iq[i].rs2])
          {
              for(int j =0;j<rob_count;j++){
                  if(iq[i].rs1!=rob[j].rd && iq[i].rs2!=rob[j].rd) {
                      rob_check_ready_to_issue =1;
                      }
                    }
                    if (rob_check_ready_to_issue ){
                      strcpy(cpu->stage[MUL_FU1].opcode, iq[i].opcode);
                      cpu->stage[MUL_FU1].pc = iq[i].iq_pc;
                      cpu->stage[MUL_FU1].rd = iq[i].rd;
                      cpu->stage[MUL_FU1].rs1 = iq[i].rs1;
                      cpu->stage[MUL_FU1].rs1_value = iq[i].rs1_value;
                      cpu->stage[MUL_FU1].rs2 = iq[i].rs2;
                      cpu->stage[MUL_FU1].rs2_value = iq[i].rs2_value;
                      rob_check_ready_to_issue =0;
                      iq_removeItem (i);
                      break;
                    }



                    if(iq[i].rs1Ready && iq[i].rs2Ready  ){
                      //printf(" sub i:%d\n",i );
                      strcpy(cpu->stage[MUL_FU1].opcode, iq[i].opcode);
                      cpu->stage[MUL_FU1].pc = iq[i].iq_pc;
                      cpu->stage[MUL_FU1].rd = iq[i].rd;
                      cpu->stage[MUL_FU1].rs1 = iq[i].rs1;
                      cpu->stage[MUL_FU1].rs1_value = iq[i].rs1_value;
                      cpu->stage[MUL_FU1].rs2 = iq[i].rs2;
                      cpu->stage[MUL_FU1].rs2_value = iq[i].rs2_value;

                      iq_removeItem (i);
                      break;

                      }
                    }

                  }




}
}
}



void lsqUpdate(APEX_CPU* cpu){

  for(int i = 0; i<lsq_count; i++){
  if(lsq[i].valid && lsq[i].rd_compute_value_ready) {

    if(strcmp(lsq[i].opcode,"LOAD")==0 || strcmp(lsq[i].opcode,"LDR")==0 ){

        //if(lsq[i].rs1Ready){
        strcpy(cpu->stage[MEM1].opcode, lsq[i].opcode);
        cpu->stage[MEM1].pc = lsq[i].lsq_pc;
        cpu->stage[MEM1].rd = lsq[i].rd;
        cpu->stage[MEM1].rs1 = lsq[i].rs1;
        cpu->stage[MEM1].rs1_value = lsq[i].rs1_value;
        cpu->stage[MEM1].rs2 = lsq[i].rs2;
        cpu->stage[MEM1].rs2_value = lsq[i].rs2_value;
        cpu->stage[MEM1].imm = lsq[i].imm;
        cpu->stage[MEM1].rd_value = lsq[i].rd_value;

        lsq_removeItem (i);

    //  }

  }


  if(strcmp(lsq[i].opcode,"STORE")==0 || strcmp(lsq[i].opcode,"STR")==0){
    //  printf("lsq rd_value_ready %d\n",lsq[i].rd_value_ready  );
      if(lsq[i].rd_value_ready ) {
      //if(lsq[i].rs1Ready){
      strcpy(cpu->stage[MEM1].opcode, lsq[i].opcode);
      cpu->stage[MEM1].pc = lsq[i].lsq_pc;
      cpu->stage[MEM1].rd = lsq[i].rd;
      cpu->stage[MEM1].rs1 = lsq[i].rs1;
      cpu->stage[MEM1].rs1_value = lsq[i].rs1_value;
      cpu->stage[MEM1].rs2 = lsq[i].rs2;
      cpu->stage[MEM1].rs2_value = lsq[i].rs2_value;
      cpu->stage[MEM1].imm = lsq[i].imm;

          cpu->stage[MEM1].compute_value = lsq[i].compute_value;


      cpu->stage[MEM1].rd_value = lsq[i].rd_value;

      lsq_removeItem (i);

    }

}

}

}


}




int branch (APEX_CPU* cpu){
CPU_Stage* stage = &cpu->stage[BRANCH];
if (strcmp(stage->opcode, "BZ") == 0 ) {

if (z == 0){

/*cpu->stage[F].pc = 0;
cpu->stage[F].rs1 = 0;
cpu->stage[F].rs2 = 0;
strcpy(cpu->stage[F].opcode, "");


cpu->stage[DRF].pc = 0;
cpu->stage[DRF].rs1 = 0;
cpu->stage[DRF].rs2 = 0;
strcpy(cpu->stage[DRF].opcode, "");*/

flush_branch =1;

cpu->pc = cpu->pc + stage->imm;

}

}
if ( strcmp(stage->opcode, "BNZ") == 0){

if (z != 0)
{
  cpu->stage[F].pc = 0;
  cpu->stage[F].rs1 = 0;
  cpu->stage[F].rs2 = 0;
  strcpy(cpu->stage[F].opcode, "");


  cpu->stage[DRF].pc = 0;
  cpu->stage[DRF].rs1 = 0;
  cpu->stage[DRF].rs2 = 0;
  strcpy(cpu->stage[DRF].opcode, "");



  cpu->pc = cpu->pc + stage->imm;



}

}



if ( strcmp(stage->opcode, "JUMP") == 0){


  cpu->pc = cpu->pc + stage->rs1_value + stage->imm;


}





  //cpu->stage[WB] = cpu->stage[BRANCH];

if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("BRANCH----->", stage);
}


cpu->stage[BRANCH].pc = 0;
cpu->stage[BRANCH].imm = 0;
strcpy(cpu->stage[BRANCH].opcode, "");

return 0;

}


int mem1 (APEX_CPU* cpu){
CPU_Stage* stage = &cpu->stage[MEM1];

if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "LDR") == 0  ) {

stage->rd_value = cpu->data_memory[stage->rd_value];

//cpu->stage[MEM2] = cpu->stage[MEM1];
}

if (strcmp(stage->opcode, "STORE") == 0 || strcmp(stage->opcode, "STR") == 0) {

//stage->rd_value = cpu->data_memory[stage->rd_value];

 cpu->data_memory[stage->compute_value] = stage->rd_value;

}

if (strcmp(cpu->stage[MEM1].opcode,"")!=0)
cpu->stage[MEM2] = cpu->stage[MEM1];




if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("MEM1----->", stage);
}


cpu->stage[MEM1].pc = 0;
cpu->stage[MEM1].rs1 = 0;
cpu->stage[MEM1].rs2 = 0;
strcpy(cpu->stage[MEM1].opcode, "");




return 0;
}


int mem2 (APEX_CPU* cpu){
CPU_Stage* stage = &cpu->stage[MEM2];

if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "LDR") == 0) {

//stage->rd_value = cpu->data_memory[stage->rd_value];


}


if (strcmp(cpu->stage[MEM2].opcode,"")!=0)
cpu->stage[MEM3] = cpu->stage[MEM2];
if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("MEM2----->", stage);
}


cpu->stage[MEM2].pc = 0;
cpu->stage[MEM2].rs1 = 0;
cpu->stage[MEM2].rs2 = 0;
strcpy(cpu->stage[MEM2].opcode, "");




return 0;


}

int mem3 (APEX_CPU* cpu){
CPU_Stage* stage = &cpu->stage[MEM3];

if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "LDR") == 0) {

//stage->rd_value = cpu->data_memory[stage->rd_value];


}
if (strcmp(cpu->stage[MEM3].opcode,"")!=0)
cpu->stage[WB] = cpu->stage[MEM3];

if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("MEM3----->", stage);
}




cpu->stage[MEM3].pc = 0;
cpu->stage[MEM3].rs1 = 0;
cpu->stage[MEM3].rs2 = 0;
strcpy(cpu->stage[MEM3].opcode, "");



return 0;


}





int mulFU1(APEX_CPU* cpu) {
CPU_Stage* stage = &cpu->stage[MUL_FU1];

//printf("mulFU1 %d\n",stage->pc );


if (strcmp(stage->opcode, "MUL") == 0) {
stage->buffer = stage->rs1_value*stage->rs2_value;

cpu->stage[MUL_FU2] = cpu->stage[MUL_FU1];



}

if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("MUL_FU_STAGE_1--->", stage);
}


cpu->stage[MUL_FU1].pc = 0;
cpu->stage[MUL_FU1].rs1 = 0;
cpu->stage[MUL_FU1].rs2 = 0;
strcpy(cpu->stage[MUL_FU1].opcode, "");


return 0;
}



int mulFU2(APEX_CPU* cpu) {
CPU_Stage* stage = &cpu->stage[MUL_FU2];
if (strcmp(stage->opcode, "MUL") == 0) {
stage->rd_value = stage->buffer;
cpu->stage[MUL_FU3] = cpu->stage[MUL_FU2];








}
if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("MUL_FU_STAGE_2--->", stage);
}


cpu->stage[MUL_FU2].pc = 0;
cpu->stage[MUL_FU2].rs1 = 0;
cpu->stage[MUL_FU2].rs2 = 0;
strcpy(cpu->stage[MUL_FU2].opcode, "");


return 0;
}



int mulFU3(APEX_CPU* cpu) {
CPU_Stage* stage = &cpu->stage[MUL_FU3];



if (ENABLE_DEBUG_MESSAGES) {
  print_stage_content("MUL_FU_STAGE_3--->", stage);
}

//printf("%d\n", stage->same);
if (strcmp(stage->opcode, "MUL") == 0) {

if(!cpu->stage[WB].busy){
cpu->stage[WB] = cpu->stage[MUL_FU3];

cpu->stage[MUL_FU3].pc = 0;
cpu->stage[MUL_FU3].rs1 = 0;
cpu->stage[MUL_FU3].rs2 = 0;
strcpy(cpu->stage[MUL_FU3].opcode, "");

}


}



return 0;
}



















int intFU1(APEX_CPU* cpu)
{

  CPU_Stage* stage = &cpu->stage[INT_FU1];

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {

     stage->buffer = stage->rs1_value+stage->imm;
//

    }
    if (strcmp(stage->opcode, "STR") == 0) {
     stage->buffer = stage->rs1_value+stage->rs2_value;
//printf("INT_FU1 STR buffer : %d \n",stage->buffer);

    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
     stage->buffer = stage->rs1_value+stage->imm;

    }
    if (strcmp(stage->opcode, "LDR") == 0) {
     stage->buffer = stage->rs1_value+stage->rs2_value;


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
      printf("add fu1: %d\n",stage->rs2_value );
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
    stage->buffer = stage->rs1_value-stage->rs2_value;

    }
     if (strcmp(stage->opcode, "AND") == 0) {
    stage->buffer = stage->rs1_value&stage->rs2_value;

    }
    if (strcmp(stage->opcode, "OR") == 0) {
    stage->buffer = stage->rs1_value|stage->rs2_value;

    }

    if (stage->buffer==0){
       z = 1;
    }
    else z = 0;
    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[INT_FU2] = cpu->stage[INT_FU1];

    if (ENABLE_DEBUG_MESSAGES) {
         print_stage_content("INT1_FU_STAGE ---> ", stage);
         }


         cpu->stage[INT_FU1].pc = 0;
         cpu->stage[INT_FU1].rs1 = 0;
         cpu->stage[INT_FU1].rs2 = 0;
         strcpy(cpu->stage[INT_FU1].opcode, "");



  return 0;
}


int intFU2(APEX_CPU* cpu)

{
  CPU_Stage* stage = &cpu->stage[INT_FU2];

    /* Store */

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->rd_value = stage->buffer;

    }
    if (strcmp(stage->opcode, "ADDL") == 0) {
    stage->rd_value = stage->buffer;

    }
    if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "LDR") == 0 || strcmp(stage->opcode, "STORE") == 0 || strcmp(stage->opcode, "STR") == 0) {
    //stage->rd_value = stage->buffer;
    for (int i = 0; i< lsq_count;i++){
      if (stage->pc == lsq[i].lsq_pc )
      {
        lsq[i].rs1_value = stage->rs1_value;
        lsq[i].rs2_value = stage->rs2_value;
        lsq[i].imm = stage->imm;
        if (strcmp(stage->opcode, "STORE") == 0 || strcmp(stage->opcode, "STR") == 0){
            lsq[i].compute_value = stage->buffer;
          //  printf("store compute_value : %d\n",lsq[i].compute_value );
        }
        else
        lsq[i].rd_value = stage->buffer;

        lsq[i].rd_compute_value_ready =1;
      }
    }

    //cpu->stage[MEM1] = cpu->stage[INT_FU2];

    if (ENABLE_DEBUG_MESSAGES) {

      print_stage_content("INT2_FU_STAGE --->", stage);


      cpu->stage[INT_FU2].pc = 0;
      cpu->stage[INT_FU2].rs1 = 0;
      cpu->stage[INT_FU2].rs2 = 0;
      strcpy(cpu->stage[INT_FU2].opcode, "");
      return 0;

    }

    }

    if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0) {
    stage->rd_value = stage->buffer;


    }

    if (strcmp(stage->opcode, "SUB") == 0) {
    stage->rd_value = stage->buffer;

    }


    if (strcmp(stage->opcode, "SUBL") == 0) {
    stage->rd_value = stage->buffer;

    }

    /* Copy data from Execute latch to Memory latch*/
    if (strcmp(cpu->stage[INT_FU2].opcode,"")!=0){

    cpu->stage[WB] = cpu->stage[INT_FU2];
    printf("add: %d\n",stage->rd_value );
    cpu->stage[WB].busy = 1;
}



    if (ENABLE_DEBUG_MESSAGES) {

      print_stage_content("INT2_FU_STAGE --->", stage);

    }
    cpu->stage[INT_FU2].pc = 0;
    cpu->stage[INT_FU2].rs1 = 0;
    cpu->stage[INT_FU2].rs2 = 0;
    strcpy(cpu->stage[INT_FU2].opcode, "");


  return 0;

}

void commitROB (APEX_CPU* cpu){

  if (rob[headROB].rd_ready)
  {
    for(int i = headRT;i<rt_count;i++){
      if(rt[i].p_rd == rob[headROB].rd)
    headRT ++;
  }
// if(store)
  if (strcmp(rob[headROB].opcode,"STORE")!=0 || strcmp(rob[headROB].opcode,"STR")!=0){

    cpu->regs[rob[headROB].rd] = rob[headROB].rd_value;
    cpu->regs_valid[rob[headROB].rd] = 1;
}


  for (int i = 0;i<lsq_count;i++){
        if(rob[headROB].rd == lsq[i].rd ){
        lsq[i].rd_value = rob[headROB].rd_value;
        lsq[i].rd_value_ready = 1;
      }
    }





    for (int i = 0;i<iq_count;i++){
          if(rob[headROB].rd == iq[i].rs1 ){

          iq[i].rs1_value = rob[headROB].rd_value;
          iq[i].rs1Ready = 1;
        }

        if(rob[headROB].rd == iq[i].rs2){
          iq[i].rs2_value = rob[headROB].rd_value;
          iq[i].rs2Ready = 1;
        }
      }



            printf("ROB retired instruction --> pc(%d) %s \n\n",rob[headROB].rob_pc, rob[headROB].opcode );

    rob[headROB].valid = 0;
    rob[headROB].rob_pc = 0;
    rob[headROB].rd = 0;
    rob[headROB].rd_value = 0;
    rob[headROB].rd_ready = 0;
    strcpy(rob[headROB].opcode, "");
    headROB ++;



  }
  else if (strcmp(rob[headROB].opcode,"BZ")==0)
{
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



  //printf(" %s\n",stage[WB].opcode );
  // if (strcmp(stage->opcode, "STORE") == 0 )


for (int i = 0; i < rob_count; i++){

if (rob[i].valid)
{
  if (rob[i].rob_pc == stage->pc ){




    rob[i].rd_value = stage->rd_value;
    rob[i].rd_ready = 1;


  }
}

}
      cpu->ins_completed++;


      cpu->stage[WB].pc = 0;
      cpu->stage[WB].rs1 = 0;
      cpu->stage[WB].rs2 = 0;
      cpu->stage[WB].busy = 0;
      strcpy(cpu->stage[WB].opcode, "");


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
      printf("---------------------------------------------------------------------------------------------\n");
      printf("                                     Clock Cycle #: %d                                        \n", cpu->clock);
    printf("---------------------------------------------------------------------------------------------\n");
    }

        commitROB(cpu);

        writeback(cpu);



        mem3(cpu);
        mem2(cpu);
        mem1(cpu);

        lsqUpdate(cpu);

        intFU2(cpu);
	      intFU1(cpu);

        branch(cpu);

        mulFU3(cpu);
        mulFU2(cpu);
        mulFU1(cpu);

        issueInstruction(cpu);

        stateCall(cpu);

showRt();
        decode(cpu);
        fetch(cpu);



    cpu->clock++;
  }

  return 0;
}
