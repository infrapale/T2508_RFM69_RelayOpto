#ifndef __PIR_H__
#define __PIR_H__

#define NBR_OF_PIR      2
#define PIR_LABEL_LEN   8
#define PIR_ACTIVE      1
#define PIR_INACTIVE    0

typedef struct
{              //12345678
    char        "Piha 1 ";
    uint8_t     is_active;
    uint8_t     new_state;
    uint32_t    timeout;
} pir_st;

void pir_initialize(void);
#endif