/*
 * main.c
 *
 *  Created on: Jun 25, 2018
 *      Author: kd
 */
#include <unistd.h>
#include <time.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "c_fw_pru0_bin.h" //code to be loaded in I-RAM of PRU0
#include "c_fw_pru1_bin.h" //code to be loaded in I-RAM of PRU1

#include "pru_app.h" // Application specific includes


#define ITER 1000




int pThreadFxnMain_10ms(PruDataRamMap_t* pArg[])
{
    printf("  Entered Main........\n");
    float dT[2] = {0,0};
    float v[2] = {0,0};
    float dX = 16.6;
    int j = 0;
    int i = 0;
    while(j < ITER )
    {
        for(i = 0; i <=1; i++)
        {
            dT[i] = (dataRamMap_pru[i]->dT * 5.0)*0.000000001;
            v[i] = dX/dT[i];
            printf("[PRU_%d]: [%d] |  dT: [%f] (ns) | V: [%f] (m/s)\n", i, dataRamMap_pru[i]->Idx, dT[i], v[i]);

        }
        //printf("[PRU_0]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", dataRamMap_pru[0]->Idx, (dataRamMap_pru[0]->dT * 5.0)*0.000000001, dataRamMap_pru[0]->TimeStamp, dataRamMap_pru[0]->TimeStamp_Past);
        //printf("[PRU_1]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", dataRamMap_pru[1]->Idx, (dataRamMap_pru[1]->dT * 5.0)*0.000000001, dataRamMap_pru[1]->TimeStamp, dataRamMap_pru[1]->TimeStamp_Past);
        usleep(1000); //10ms sleep
        j++;
    }

    return 0;
}

int pThreadFxnPru0(PruDataRamMap_t * pArg)
{


    printf("===> [Entered] Thread_PRU[0].......!\n");
 //   PruDataRamMap_t *_pru;


    // Map PRU DATARAM;
    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void * *)&dataRamMap_pru[0]);

    // Manually initialize PRU DATARAM - this struct is mapped to the// PRU, so these assignments actually modify DATARAM directly.
    dataRamMap_pru[0]->Idx = 0x4;
    dataRamMap_pru[0]->SigFromA8 = 0x1;
    dataRamMap_pru[0]->TimeStamp = 0;
    dataRamMap_pru[0]->TimeStamp_Past = 0;
    dataRamMap_pru[0]->dT = 0;
    // Memory fence: not strictly needed here, as compiler will insert
    // an implicit fence when prussdrv_exec_code(...) is called, but
    // a good habit to be in.
    __sync_synchronize();

    if(prussdrv_exec_code(PRU0, Arr_pru0, sizeof Arr_pru0))
    {
        printf("[Error]: PRU code execution\n");

        return -1;
    }
    while(dataRamMap_pru[0]->Idx < ITER)
    {


        //Keep waitning for the signal from PRU (PRU 0 in this case will send an event to the Host (to us)
        prussdrv_pru_wait_event(PRU_EVTOUT_0);


        // if it reaches here, there's a signal from PRU indicating that the data is ready;
        //Clear the event: if you don't do this, PRU will eventually stall --  you will not be able to wait again.
        prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);//has to be cleared to get the next event

        __sync_synchronize(); //??

        //printf("[PRU_0]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", dataRamMap_pru[0]->Idx, (dataRamMap_pru[0]->dT*5.0)*0.000000001, dataRamMap_pru[0]->TimeStamp, dataRamMap_pru[0]->TimeStamp_Past);
        usleep(500);
        dataRamMap_pru[0]->SigFromA8 = 0x1; //loop on

    }

    printf("    [DONE]:  Exiting...Thread_PRU[0] \n");
        prussdrv_pru_disable(PRU0);

    return 0;
}

int pThreadFxnPru1(void *pArg)
{

    printf("===> [Entered] Thread_PRU[1].......!\n");
    //PruDataRamMap_t *_pru = (PruDataRamMap_t*)pArg;
    // Map PRU DATARAM;
    prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void * *)&dataRamMap_pru[1]);

    // Manually initialize PRU DATARAM - this struct is mapped to the// PRU, so these assignments actually modify DATARAM directly.
    dataRamMap_pru[1]->Idx = 0x0;
    dataRamMap_pru[1]->SigFromA8 = 0x1;
    dataRamMap_pru[1]->TimeStamp = 0;
    dataRamMap_pru[1]->TimeStamp_Past = 0;
    dataRamMap_pru[1]->dT = 0;
    // Memory fence: not strictly needed here, as compiler will insert
    // an implicit fence when prussdrv_exec_code(...) is called, but
    // a good habit to be in.
    __sync_synchronize();

    if(prussdrv_exec_code(PRU1, Arr_pru1, sizeof Arr_pru1))
    {
        printf("[Error]: PRU code execution\n");

        return -1;
    }
    while(dataRamMap_pru[1]->Idx < ITER)
    {


        //Keep waitning for the signal from PRU (PRU 0 in this case will send an event to the Host (to us)
        prussdrv_pru_wait_event(PRU_EVTOUT_1);


        // if it reaches here, there's a signal from PRU indicating that the data is ready;
        //Clear the event: if you don't do this, PRU will eventually stall --  you will not be able to wait again.
        prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU1_ARM_INTERRUPT);//has to be cleared to get the next event

        __sync_synchronize(); //??

        //printf("[PRU_1]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", dataRamMap_pru[1]->Idx, (dataRamMap_pru[1]->dT*5.0)*0.000000001, dataRamMap_pru[1]->TimeStamp, dataRamMap_pru[1]->TimeStamp_Past);
        usleep(500);
        dataRamMap_pru[1]->SigFromA8 = 0x1; //loop on

    }

        printf("    [DONE]:  Exiting...Thread_PRU[1] \n");

    prussdrv_pru_disable(PRU1);
    return 0;
}

int main(int argc, char * argv[])
{
  //PruDataRamMap_t * dataRamMap_pru[2];
  tpruss_intc_initdata prussIntCInitData = PRUSS_INTC_INITDATA;

  int ret;
  // First, initialize the driver and open the kernel device
  printf("HelloPRU example1\n");
  prussdrv_init();

  ret = prussdrv_open(PRU_EVTOUT_0);
  if(ret != 0)
  {
    printf("Failed to open PRUSS driver [0] !\n");
    return ret;
  }

  ret = prussdrv_open(PRU_EVTOUT_1);
  if(ret != 0)
  {
    printf("Failed to open PRUSS driver [1]!\n");
    return ret;
  }
  // Set up the interrupt mapping so we can wait on INTC later
  prussdrv_pruintc_init(&prussIntCInitData);


  //============ Threading =========
 //PRU is ready at this point, create the threads
  pthread_t Thread_pru[2],ThreadMain;

  pthread_create (&Thread_pru[0], NULL, (void *) &pThreadFxnPru0, (int *)(&dataRamMap_pru[0]));
  pthread_create (&Thread_pru[1], NULL, (void *) &pThreadFxnPru1, (PruDataRamMap_t*)dataRamMap_pru[1]);
  usleep(10000);
  pthread_create (&ThreadMain, NULL, (void *) &pThreadFxnMain_10ms, dataRamMap_pru);




  pthread_join(Thread_pru[0], NULL);
  pthread_join(Thread_pru[1], NULL);
  pthread_join(ThreadMain, NULL);

    printf("  ===== [//]END ====== \n");


  prussdrv_exit();
}


