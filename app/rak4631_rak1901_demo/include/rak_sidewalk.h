#ifndef __RAK_SIDEWALK_H
#define	__RAK_SIDEWALK_H

#include "stdint.h"

/** @brief SenoSpace payload structure. */
#pragma pack(push, 1)
typedef struct 
{
    uint8_t  payload_type;
    float  temp;
    float  hum;
} sidewalk_payload_t; //max 9 bytes
#pragma pack(pop)

/**
 * @brief This function is used to read SenoSpace payload.
 *
 * @return sidewalk_payload_t  SenoSpace payload.
 */
sidewalk_payload_t rak_sidewalk_get_payload(void);

#endif
