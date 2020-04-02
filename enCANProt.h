/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef EN_CAN_H_
#define EN_CAN_H_
#include "enpHelper.h"

extern uint32_t ENCAN_Error;

// Protocol errors
#define ENCAN_ERROR_NOERROR 0x00000000
#define ENCAN_ERROR_RXOVERFLOW 0x10000000  // overflow rx buffer
#define ENCAN_ERROR_TXOVERFLOW 0x20000000  // overflow tx buffer

// CAN ID
#define ENCAN_RECV1 1808
#define ENCAN_SEND1 1824
#define ENCAN_RECV2 1840
#define ENCAN_SEND2 1856

// Mask for counter
#define ENCAN_MASK 0x0F
// Timeout for can messages (ms)
#define ENCAN_ENP_TIMEOUT 1000

// message
typedef struct {
  uint8_t data[8];  // data buff for CAN classic
  struct {
    unsigned id : 11;   // standart id
    unsigned size : 4; 
  } Attr;
} enCANMessage_t;

#define ENCAN_RBUF_SIZE 128

// Index mask
#define ENCAN_RBUF_MASK (ENCAN_RBUF_SIZE - 1)
#if (ENCAN_RBUF_SIZE & ENCAN_RBUF_MASK)
#error ENCAN_RBUF_SIZE is not a power of 2
#endif

// protocol handler function
typedef void (*enCANMsg_tHANDLER)(enCANMessage_t* msg);

// Max count of protocol handlers
#define ENCAN_MSGHANDLERSNUM 20

// Init
#ifdef ENP_FREERTOS
void ENCAN_Init(enCANMessage_t*,
              uint16_t,
              uint8_t (*)(enCANMessage_t* msg),
              uint32_t (*)(void));
#else
void ENCAN_Init(enCANMessage_t*,
              uint16_t,
              enCANMessage_t*,
              uint16_t,
              uint8_t (*)(enCANMessage_t* msg),
              uint32_t (*)(void));
#endif

void ENCAN_Proc(void);
void ENCAN_TxProc(void);

// Get time function
extern uint32_t (*ENCAN_Time)(void);

// Send CAN message function
extern uint8_t (*ENCAN_Tx)(enCANMessage_t* msg);

// Rx message function
uint8_t ENCAN_Rx(enCANMessage_t* msg);

#endif
