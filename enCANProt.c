/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enCANProt.h"
#include "string.h"
#ifdef ENP_FREERTOS
#include "cmsis_os.h"
#endif

#ifndef ENP_FREERTOS
// Ring buffer pointers
static int txHead = 0;
static int txTail = 0;
// transmit buffer
static enCANMessage_t *txBuff = NULL;

static uint16_t txMask = 0;
#endif

// recieve buffer
static enCANMessage_t *rxBuff = NULL;
static uint16_t rxMask = 0;
static int volatile rxHead = 0;
static int rxTail = 0;
char ENP_RBuf[2][ENCAN_RBUF_SIZE];
static uint16_t ENP_RHead[2] = {0, 0};
static uint16_t ENP_RTail[2] = {0, 0};

// ENP handler
static void ENP_MsgHandler(enCANMessage_t *msg);

// List of handlers
static enCANMsg_tHANDLER MsgHandler[ENCAN_MSGHANDLERSNUM] = {ENP_MsgHandler};

// number of handlers
static uint16_t MsgHandlerNum = 1;

uint32_t ENCAN_Error = 0;

static uint32_t TimeDummy(void) { return 0; }
uint32_t (*ENCAN_Time)(void) = TimeDummy;

static uint8_t TxDummy(enCANMessage_t *msg) { return 0; }
uint8_t (*ENCAN_Tx)(enCANMessage_t *msg) = TxDummy;

// get message
uint8_t ENCAN_Rx(enCANMessage_t *msg) {
  int i;

  if (msg) {
    i = (rxHead + 1) & rxMask;
    if (i != rxTail) {
      rxHead = i;
      rxBuff[i] = *msg;
      return 1;
    }
    ENCAN_Error |= ENCAN_ERROR_RXOVERFLOW;
  }
  return 0;
}

#ifdef ENP_FREERTOS
void ENCAN_Init(enCANMessage_t *rbuff, uint16_t rmask,
                uint8_t (*txfunc)(enCANMessage_t *msg),
                uint32_t (*timefunc)(void)) {
  rxBuff = rbuff;
  rxMask = rmask;

  if (txfunc) {
    ENCAN_Tx = txfunc;
  }
  if (timefunc) {
    ENCAN_Time = timefunc;
  }
}
#else

void ENCAN_Init(enCANMessage_t *rbuff, uint16_t rmask, enCANMessage_t *tbuff,
                uint16_t tmask, uint8_t (*txfunc)(enCANMessage_t *msg),
                uint32_t (*timefunc)(void)) {
  rxBuff = rbuff;
  txBuff = tbuff;
  rxMask = rmask;
  txMask = tmask;

  if (txfunc) {
    ENCAN_Tx = txfunc;
  }
  if (timefunc) {
    ENCAN_Time = timefunc;
  }
}
#endif

void ENCAN_Proc() {
  while (rxHead != rxTail) {
    rxTail = (rxTail + 1) & rxMask;
    for (int i = 0; i < MsgHandlerNum; i++)
      MsgHandler[i](rxBuff + rxTail);
  }

#ifndef ENP_FREERTOS
  if (txBuff) {
    while (txTail != txHead) {
      int i = (txTail + 1) & txMask;
      if (ENCAN_Tx(txBuff + i)) {
        txTail = i;
      } else {
        break;
      }
    }
  }
#endif
}

static void AddBroadcast(int msgid, const void *const data, int size) {
  enCANMessage_t _msg;
#ifdef ENP_FREERTOS
  if (size <= 8 && data) {
    memcpy(&_msg.data, data, size);
    _msg.Attr.id = msgid;
    _msg.Attr.size = size;
    ENCAN_Tx(&_msg);
  }
#else
  if (size <= 8 && data) {
    memcpy(&_msg.data, data, size);
    _msg.Attr.id = msgid;
    _msg.Attr.size = size;
    if (txBuff) {
      txHead = (txHead + 1) & txMask;
      if (txHead != txTail) {
        txBuff[txHead] = _msg;
      } else
        ENCAN_Error |= ENCAN_ERROR_TXOVERFLOW;
    }
  }
#endif
}

static int Getc(int i) {
  if (ENP_RTail[i] != ENP_RHead[i]) {
    return ENP_RBuf[i][ENP_RTail[i] = (ENP_RTail[i] + 1) & ENCAN_RBUF_MASK];
  } else {
    return -1;
  }
}
int ENCAN_getc1() { return Getc(0); }
int ENCAN_getc2() { return Getc(1); }

int ENCAN_putn(const char *s, int n) {
  static uint16_t pcntr = 0;

  for (; n > 0; n -= 8) {
    if (n >= 8) {
      AddBroadcast(ENCAN_SEND1 + pcntr, (void *)s, 8);
    } else {
      AddBroadcast(ENCAN_SEND1 + pcntr, (void *)s, n);
    }
    pcntr = (pcntr + 1) & ENCAN_MASK;
    s += 8;
  }
  return 1;
}

static void ENP_MsgHandler(enCANMessage_t *msg) {
  unsigned char *c = (void *)&msg->data;
  int i, j, n, msgid;
  static uint32_t ptime[2] = {0, 0};
  static uint16_t pcntr[2] = {0, 0};
  uint32_t time = ENCAN_Time();
  const uint32_t _id = msg->Attr.id;

  if (_id >= ENCAN_RECV1 && _id <= ENCAN_RECV1 + ENCAN_MASK) {
    i = 0;
    msgid = ENCAN_RECV1;
  } else if (_id >= ENCAN_RECV2 && _id <= ENCAN_RECV2 + ENCAN_MASK) {
    i = 1;
    msgid = ENCAN_RECV2;
  } else {
    return;
  }

  if (ptime[i] < time) {
    pcntr[i] = _id - msgid;
  }

  if (_id == msgid + pcntr[i]) {
    for (n = 0; n < msg->Attr.size; n++) {
      j = (ENP_RHead[i] + 1) & ENCAN_RBUF_MASK;
      if (j != ENP_RTail[i]) {
        ENP_RBuf[i][ENP_RHead[i] = j] = *c++;
      } else {
        ENCAN_Error |= ENCAN_ERROR_RXOVERFLOW;
        break;
      }
    }
    pcntr[i] = (pcntr[i] + 1) & ENCAN_MASK;
    ptime[i] = time + ENCAN_ENP_TIMEOUT;
  }
}