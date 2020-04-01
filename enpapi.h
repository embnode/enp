/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef ENP_HELPER_H_
#define ENP_HELPER_H_

#include "enpdefs.h"
#include "enpprot.h"
#include "smpprot.h"
#include "stdint.h"
#ifdef __cplusplus
extern "C" {
#endif

#define API_GET_NODE_NUM(_NODE_LIST_)                                          \
  (sizeof(_NODE_LIST_) / sizeof((_NODE_LIST_)[0]))

// Crc 16 calculate
uint16_t Crc16(const void *data, uint32_t size, uint16_t crc);

// Crc 32 calculate
uint32_t CRC32(const void *data, uint32_t size, uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif
