#ifndef __PIR_H__
#define __PIR_H__

#define NBR_OF_PIR      2
#define PIR_LABEL_LEN   8
//#define PIR_ACTIVE      1
//#define PIR_INACTIVE    0

typedef enum
{
    PIR_STATUS_UNDEFINED = 0,
    PIR_STATUS_ACTIVE,
    PIR_STATUS_INACTIVE
} pir_status_et;

typedef struct
{              //12345678
    char        zone[8];
    char        name[8];
    uint16_t    state;
    uint16_t    prev_state;
    uint8_t     new_active;
    uint8_t     prev_active;
    uint32_t    timeout;
} pir_st;

void pir_initialize(void);

void pir_set_status(uint8_t indx, uint8_t new_status);

#endif