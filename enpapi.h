/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef SCBAPI_H_
#define SCBAPI_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "enpdefs.h"
#include "enpprot.h"
#include "smpprot.h"
#include "stdint.h"


#define API_GET_NODE_NUM(_NODE_LIST_)                                          \
  (sizeof(_NODE_LIST_) / sizeof((_NODE_LIST_)[0]))

// Вычисление стандартного CRC-16 (ARC)
// ------------------------------------
extern uint16_t CRC16(const void *data, uint32_t size, uint16_t crc,
                      int charsize);

// Вычисление стандартного CRC-32 (ARC)
// ------------------------------------
extern uint32_t CRC32(const void *data, uint32_t size, uint32_t crc,
                      int charsize);

#ifdef __cplusplus
}
#endif

#endif
