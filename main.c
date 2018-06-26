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
#include "c_fw_pru0_bin.h"
// Define a struct describing how we want to map PRU DATARAM. This
// struct could be up to 8kB. (Actually, technically 16kB, since the
// DATARAM for PRU0 and PRU1 is contiguous; but DATARAM0 and DATARAM1
// are reversed for PRU1, so this gets complicated...
typedef struct {
  uint32_t iPingPong;
  uint32_t SigFromA8;
  uint32_t TimeStamp;
  uint32_t TimeStamp_Past;
  uint32_t dT;
} PrussDataRam_t;
int main(int argc, char * argv[])
{

  tpruss_intc_initdata prussIntCInitData = PRUSS_INTC_INITDATA;
  PrussDataRam_t * prussDataRam;
  int ret;
  // First, initialize the driver and open the kernel device
  printf("HelloPRU example1\n");
  prussdrv_init();
  ret = prussdrv_open(PRU_EVTOUT_0);
  if(ret != 0) {
    printf("Failed to open PRUSS driver!\n");
    return ret;
  }
  // Set up the interrupt mapping so we can wait on INTC later
  prussdrv_pruintc_init(&prussIntCInitData);
  // Map PRU DATARAM; reinterpret the pointer type as a pointer to
  // our defined memory mapping struct. We could also use uint8_t *
  // to access the RAM as a plain array of bytes, or uint32_t * to
  // access it as words.
  prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void * *)&prussDataRam);
  // Manually initialize PRU DATARAM - this struct is mapped to the
  // PRU, so these assignments actually modify DATARAM directly.
  prussDataRam->iPingPong = 0x0;
  prussDataRam->SigFromA8 = 0x1;
  prussDataRam->TimeStamp = 0;
  prussDataRam->TimeStamp_Past = 0;
  prussDataRam->dT = 0;
  // Memory fence: not strictly needed here, as compiler will insert
  // an implicit fence when prussdrv_exec_code(...) is called, but
  // a good habit to be in.
  // This ensures that the writes to x, y, sum are fully complete
  // before the PRU code is executed: imagine what kind of painful-
  // to-debug problems you'd see if the compiler or hardware deferred
  // the writes until after the PRU started running!
  __sync_synchronize();
  prussdrv_exec_code(0, Arr_pru, sizeof Arr_pru);

  //time_t start_t, end_t;
  //double diff_t;

while(prussDataRam->iPingPong < 100)
{


    //time(&start_t);
    prussdrv_pru_wait_event(PRU_EVTOUT_0);
    // Clear the event: if you don't do this you will not be able to
    // wait again.
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);//has to be cleared to get the next event
    __sync_synchronize();

    printf("Ping: [%d] |  dT(ns): [%f] (Ts[%d] -> Ts-1[%d])\n", prussDataRam->iPingPong, (prussDataRam->dT*5)*0.000000001, prussDataRam->TimeStamp, prussDataRam->TimeStamp_Past);
    //usleep(400000);
    prussDataRam->SigFromA8 = 0x1; //loop on

   // time(&end_t);
   // diff_t = difftime(end_t, start_t);
   // printf("Execution time = %f\n", diff_t);
}

    printf("  out of while..! Exiting...\n");

  // Disable the PRU and exit; if we don't do this the PRU may
  // continue running after our program quits! The TI kernel driver
  // is not very careful about cleaning up after us.
  // Since it is possible for the PRU to trash memory and otherwise
  // cause lockups or crashes, especially if it's manipulating
  // peripherals or writing to shared DDR, this is important!
  prussdrv_pru_disable(0);
  prussdrv_exit();
}
