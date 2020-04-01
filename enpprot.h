/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef ENPROT_H_
#define ENPROT_H_
#include "enpapi.h"
#include "stdbool.h"

// Размер буферов приёма-передачи TODO
#define ENP_BUFFSIZE 300

// Protocol commands
#define ENP_CMD_GETNODENUM 0x01   // get number of nodes
#define ENP_CMD_GETNODEDESCR 0x02 // get node description
#define ENP_CMD_GETVARDESCR 0x03  // get variable description
#define ENP_CMD_GETVARS 0x10      // get variable value
#define ENP_CMD_SETVARS 0x11      // set variable value
#define ENP_CMD_ERROR 0x80        // Error flag

#define ENP_PAYLOAD_MAX_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

// ENP frame
typedef struct {
  uint8_t sync1;
  uint8_t sync2;
  uint16_t id;
  uint8_t len;
  uint8_t cmd;
  uint8_t data[ENP_PAYLOAD_MAX_SIZE];
  uint16_t crc;
} enpFrame_t;

typedef enum {
  ENP_PROT_STAGE_SYNC1,
  ENP_PROT_STAGE_SYNC2,
  ENP_PROT_STAGE_ID1,
  ENP_PROT_STAGE_ID2,
  ENP_PROT_STAGE_LEN,
  ENP_PROT_STAGE_CMD,
  ENP_PROT_STAGE_PAYLOAD,
  ENP_PROT_STAGE_CRC1,
  ENP_PROT_STAGE_CRC2
} protStage_t;

typedef struct {
  protStage_t stage;
  uint8_t payloadIndex;
} enpFrameState_t;

// instance of the protocol
typedef struct {
  char txBuf[ENP_BUFFSIZE];
  int txLen;
  int (*TxFun)(const char *, int);
  uint16_t devId1;
  uint16_t devId2;
  enpFrame_t rxFrame;
  enpFrame_t txFrame;
  enpFrameState_t rxFrameState;
  bool isNewRxFrame;
} ENP_Handle_t;

// Protocol initialization
void ENP_NodeListInit(const ENP_Node_t **nodelist, int maxnodenum);

// Handle init
void ENP_InitHandle(ENP_Handle_t *, uint16_t, uint16_t,
                    int (*txf)(const char *, int));
// Frame parser
bool ENP_ParseFrame(ENP_Handle_t *handle, uint8_t *data, uint32_t len);
// Answer
void ENP_AnswerProc(ENP_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif
