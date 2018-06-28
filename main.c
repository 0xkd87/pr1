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
//    usleep(10000);
    PruDataRamMap_t* _pru[2];
    _pru[0] = (PruDataRamMap_t*)(&pArg[0]);
   // _pru[1] = (PruDataRamMap_t*)(pArg[1]);
    int i = 0;
    while(i < 500 )
    {

 //       printf("[i = %d] [PRU_0]: [%d] |  dT(ns): [%f] \n", i, dataRamMap_pru[0]->Idx, (dataRamMap_pru[0]->dT * 5.0)*0.000000001);

 //       printf("[i = %d] [PRU_0]: [%d] |  dT(ns): [%f] \n", i, _pru[0]->Idx, (_pru[0]->dT * 5.0)*0.000000001);
       // printf("[i = %d] [PRU_1]: [%d] |  dT(ns): [%f] \n", i, _pru[1].Idx, (_pru[1]->dT * 5.0)*0.000000001);

//        printf("[PRU_0]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", _pru[0]->Idx, (_pru[0]->dT * 5.0)*0.000000001, _pru[0]->TimeStamp, _pru[0]->TimeStamp_Past);
//        printf("[PRU_1]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", _pru[1]->Idx, (_pru[1]->dT * 5.0)*0.000000001, _pru[1]->TimeStamp, _pru[1]->TimeStamp_Past);
        usleep(1000); //10ms sleep
        i++;
    }
    return 0;
}

int pThreadFxnPru0(void *pArg)
{

    printf("===> [Entered] Thread_PRU[0].......!\n");
    PruDataRamMap_t *_pru = (PruDataRamMap_t*)pArg;

    // Map PRU DATARAM;
    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void * *)&_pru);

    // Manually initialize PRU DATARAM - this struct is mapped to the// PRU, so these assignments actually modify DATARAM directly.
    _pru->Idx = 0x4;
    _pru->SigFromA8 = 0x1;
    _pru->TimeStamp = 0;
    _pru->TimeStamp_Past = 0;
    _pru->dT = 0;
    // Memory fence: not strictly needed here, as compiler will insert
    // an implicit fence when prussdrv_exec_code(...) is called, but
    // a good habit to be in.
    __sync_synchronize();

    if(prussdrv_exec_code(PRU0, Arr_pru0, sizeof Arr_pru0))
    {
        printf("[Error]: PRU code execution\n");

        return -1;
    }
    while(_pru->Idx < ITER)
    {


        //Keep waitning for the signal from PRU (PRU 0 in this case will send an event to the Host (to us)
        prussdrv_pru_wait_event(PRU_EVTOUT_0);


        // if it reaches here, there's a signal from PRU indicating that the data is ready;
        //Clear the event: if you don't do this, PRU will eventually stall --  you will not be able to wait again.
        prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);//has to be cleared to get the next event

        __sync_synchronize(); //??

        printf("[PRU_0]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", _pru->Idx, (_pru->dT*5.0)*0.000000001, _pru->TimeStamp, _pru->TimeStamp_Past);
        usleep(500);
        _pru->SigFromA8 = 0x1; //loop on

    }

    printf("    [DONE]:  Exiting...Thread_PRU[0] \n");
        prussdrv_pru_disable(PRU0);

    return 0;
}

int pThreadFxnPru1(void *pArg)
{

    printf("===> [Entered] Thread_PRU[1].......!\n");
    PruDataRamMap_t *_pru = (PruDataRamMap_t*)pArg;
    // Map PRU DATARAM;
    prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void * *)&_pru);

    // Manually initialize PRU DATARAM - this struct is mapped to the// PRU, so these assignments actually modify DATARAM directly.
    _pru->Idx = 0x0;
    _pru->SigFromA8 = 0x1;
    _pru->TimeStamp = 0;
    _pru->TimeStamp_Past = 0;
    _pru->dT = 0;
    // Memory fence: not strictly needed here, as compiler will insert
    // an implicit fence when prussdrv_exec_code(...) is called, but
    // a good habit to be in.
    __sync_synchronize();

    if(prussdrv_exec_code(PRU1, Arr_pru1, sizeof Arr_pru1))
    {
        printf("[Error]: PRU code execution\n");

        return -1;
    }
    while(_pru->Idx < ITER)
    {


        //Keep waitning for the signal from PRU (PRU 0 in this case will send an event to the Host (to us)
        prussdrv_pru_wait_event(PRU_EVTOUT_1);


        // if it reaches here, there's a signal from PRU indicating that the data is ready;
        //Clear the event: if you don't do this, PRU will eventually stall --  you will not be able to wait again.
        prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU1_ARM_INTERRUPT);//has to be cleared to get the next event

        __sync_synchronize(); //??

        printf("[PRU_1]: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", _pru->Idx, (_pru->dT*5.0)*0.000000001, _pru->TimeStamp, _pru->TimeStamp_Past);
        usleep(500);
        _pru->SigFromA8 = 0x1; //loop on

    }

        printf("    [DONE]:  Exiting...Thread_PRU[1] \n");

    prussdrv_pru_disable(PRU1);
    return 0;
}

int main(int argc, char * argv[])
{

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

  pthread_create (&Thread_pru[0], NULL, (void *) &pThreadFxnPru0, &dataRamMap_pru[0]);
  pthread_create (&Thread_pru[1], NULL, (void *) &pThreadFxnPru1, &dataRamMap_pru[1]);
  usleep(10000);
  pthread_create (&ThreadMain, NULL, (void *) &pThreadFxnMain_10ms, (void*)dataRamMap_pru);

  pthread_join(Thread_pru[0], NULL);
  pthread_join(Thread_pru[1], NULL);
  pthread_join(ThreadMain, NULL);

    printf("  ===== [//]END ====== \n");


  prussdrv_exit();
}


