#ifndef __PIR_H__
#define __PIR_H__

#define NBR_OF_PIR  2

typedef struct
{
    uint8_t   state;
    uint8_t   new_state;
} pir_st;

void pir_initialize(void);
#endif