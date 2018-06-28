/*
 * pru_app.h
 *
 *  Created on: Jun 27, 2018
 *      Author: kd
 */

#ifndef INCL_PRU_APP_H_
#define INCL_PRU_APP_H_

//for threading
#include <pthread.h>


/*
 * Structure definition of the memory is to be mapped with DATA RAM in the PRUs
 *
 */
typedef struct
{
  uint32_t Idx;
  uint32_t SigFromA8;
  uint32_t TimeStamp;
  uint32_t TimeStamp_Past;
  uint32_t dT;
} PruDataRamMap_t;

PruDataRamMap_t * dataRamMap_pru[2];

typedef struct
{
    PruDataRamMap_t * dataRamMap;

}_xPru;

#define PRU0 0
#define PRU1 1

#endif /* INCL_PRU_APP_H_ */
