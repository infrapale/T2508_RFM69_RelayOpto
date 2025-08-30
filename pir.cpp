#include "main.h"
#include "atask.h"
#include "uart.h"
#include "json.h"
#include "rfm_receive.h"
#include "rfm_send.h"
#include "io.h"
#include "pir.h"

pir_st  pir[NBR_OF_PIR] = 
{
    {"Piha 1 ", 0, 0, 0},
    {"Piha 2 ", 0, 0, 0},
};


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

void pir_set_state(uint8_t indx, uint8_t activated)
{
    pir[indx].new_state = activated;
} 



void pir_state_machine(pir_st pirp)
{
    switch(pirp->state)
    {
        case 0:  // start
            pirp->state = 10;
        case 10:  // not activated
            if(pirp->new_state == PIR_ACTIVE)
            {
                pirp->is_active = pirp->new_state;
                pirp->state = 20;
            }
            break;
        case 20:
            if(pirp->new_state == PIR_INACTIVE)
            {
                pirp->is_active = pirp->new_state;
                pirp->state = 10;
            }
            break;
        case 30:
            pirp->state = 30;
            break;
        case 40:
            pirp->state = 10;
            break;

    }
}

void pir_task(void)
{
    switch(pir_handle.state)
    {
        case 0:
          break;
    }

}

