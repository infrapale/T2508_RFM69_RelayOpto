#include "main.h"
#include "atask.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"
#include "pir.h"

pir_st  pir[NBR_OF_PIR] = {0};


typedef struct
{
  uint8_t   sensor_indx;
} pir_ctrl_st;

void pir_task(void);

atask_st pir_handle     = {"PIR Task       ", 1000,0, 0, 255, 0, 1, pir_task};
pir_ctrl_st pir_ctrl;

void pir_initialize(void)
{
    pir_ctrl.sensor_indx = 0;
    atask_add_new(&pir_handle);
}

void pir_task(void)
{
    switch(pir_handle.state)
    {
        case 0:
          break;
    }

}

