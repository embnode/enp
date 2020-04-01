/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enpprot.h"
#include "smpprot.h"
#include "stdbool.h"
#include "string.h"

static const char *STANDART_NAME = "Too long name";
// Пакеты переменных
static ENP_Pack_t *ENP_Pack = 0;

// Чтение слова
uint16_t ENP_ReadWord(char *buf) {
  return (uint16_t)buf[0] + ((uint16_t)buf[1] << 8);
}

static uint16_t Read16(uint8_t *buf) {
  return (uint16_t)buf[0] + ((uint16_t)buf[1] << 8);
}

static uint32_t Read32(uint8_t *buf) {
  return (uint32_t)buf[0] + ((uint32_t)buf[1] << 8) + ((uint32_t)buf[2] << 16) +
         ((uint32_t)buf[3] << 24);
}
// Чтение двойного слова
uint32_t ENP_ReadDoubleWord(char *buf) {
  return (uint32_t)buf[0] + ((uint32_t)buf[1] << 8) + ((uint32_t)buf[2] << 16) +
         ((uint32_t)buf[3] << 24);
}

// Запись слова
void ENP_WriteWord(char *buf, uint16_t word) {
  *buf++ = word & 0xFF;
  *buf = word >> 8;
}

static void Write16(uint8_t *buf, uint16_t value) {
  *buf++ = value & 0xFF;
  *buf = value >> 8;
}

static void Write32(uint8_t *buf, uint32_t value) {
  *buf++ = value & 0xFF;
  *buf++ = (value >> 8) & 0xFF;
  *buf++ = (value >> 16) & 0xFF;
  *buf = (value >> 24) & 0xFF;
}

// Запись двойного слова
void ENP_WriteDoubleWord(char *buf, uint32_t dword) {
  *buf++ = dword & 0xFF;
  *buf++ = (dword >> 8) & 0xFF;
  *buf++ = (dword >> 16) & 0xFF;
  *buf = (dword >> 24) & 0xFF;
}

// Инициализация протокола
void ENP_Init(
    // список узлов конфигурации
    const ENP_Node_t **nodelist,
    // максимальное кол-во узлов конфигурации
    int maxnodenum,
    // пакеты переменных
    ENP_Pack_t *pack,
    // функция стирания флэш-памяти
    uint8_t (*erasefunc)(uint32_t startAddr, uint32_t stopAddr),
    // функция записи флэш-памяти
    uint8_t (*writefunc)(const void *ptr, void *dst, int size)) {
  // инициализируем переменные
  ENP_NodeList = nodelist;
  ENP_MaxNodeNum = maxnodenum;
  ENP_Pack = pack;
  ENP_FlashErase = erasefunc;
  ENP_FlashWrite = writefunc;
}

// Инициализация обработчика
void ENP_InitHandle(
    // экземпляр обработчика
    ENP_Handle_t *handle,
    // идентификаторы устройств
    uint16_t devId1, uint16_t devId2,
    // функция приема
    int (*rxf)(),
    // функция передачи
    int (*txf)(const char *, int)) {
  handle->rxLen = 0;
  handle->txLen = 0;
  handle->RxFun = rxf;
  handle->TxFun = txf;
  handle->packCntr = 0xFE;
  handle->sid = 0;
  handle->devId1 = devId1;
  handle->devId2 = devId2;
  handle->isNewRxFrame = false;
}

static uint16_t CalcFrameCrc(enpFrame_t const *const frame) {
  uint16_t crc = 0xFFFF;
  crc = Crc16(&frame->sync1, 1, crc);
  crc = Crc16(&frame->sync2, 1, crc);
  crc = Crc16(&frame->id, 2, crc);
  crc = Crc16(&frame->len, 1, crc);
  crc = Crc16(&frame->cmd, 1, crc);
  if (frame->len > 1) {
    crc = Crc16(frame->data, frame->len - 1, crc);
  }
  return crc;
}

bool ENP_ParseFrame(ENP_Handle_t *handle, uint8_t *data, uint32_t len) {
  enpFrameState_t *state = &handle->rxFrameState;
  enpFrame_t *frame = &handle->rxFrame;
  uint16_t calcedCrc = 0;
  bool isFrameParsed = false;

  for (int i = 0; i < len; i++) {
    switch (state->stage) {
    case ENP_PROT_STAGE_SYNC1:
      if (data[i] == 0xFA) {
        frame->sync1 = data[i];
        state->stage = ENP_PROT_STAGE_SYNC2;
      }
      break;
    case ENP_PROT_STAGE_SYNC2:
      if (data[i] == 0xCE) {
        frame->sync2 = data[i];
        state->stage = ENP_PROT_STAGE_ID1;
      } else { // restart and try again
        state->stage = ENP_PROT_STAGE_SYNC1;
      }
      break;
    case ENP_PROT_STAGE_ID1:
      frame->id = data[i];
      state->stage = ENP_PROT_STAGE_ID2;
      break;
    case ENP_PROT_STAGE_ID2:
      frame->id = (data[i] << 8) | frame->id;
      state->stage = ENP_PROT_STAGE_LEN;
      break;
    case ENP_PROT_STAGE_LEN: // lenght
      frame->len = data[i];
      state->stage = ENP_PROT_STAGE_CMD;
      break;
    case ENP_PROT_STAGE_CMD:
      frame->cmd = data[i];
      if (frame->len == 1) {
        // no payload in frame
        state->stage = ENP_PROT_STAGE_CRC1;
      } else {
        state->stage = ENP_PROT_STAGE_PAYLOAD;
        state->payloadIndex = 0;
      }
      break;
    case ENP_PROT_STAGE_PAYLOAD: // payload
      frame->data[state->payloadIndex] = data[i];
      state->payloadIndex++;
      // minus code from lenght
      if (state->payloadIndex >= frame->len - 1) {
        state->stage = ENP_PROT_STAGE_CRC1;
      }
      break;
    case ENP_PROT_STAGE_CRC1: // checksum
      frame->crc = data[i];
      state->stage = ENP_PROT_STAGE_CRC2;
      break;
    case ENP_PROT_STAGE_CRC2: // checksum
      // calculate message crc
      frame->crc = (data[i] << 8) | frame->crc;
      calcedCrc = CalcFrameCrc(frame);
      if (frame->crc == calcedCrc) {
        isFrameParsed = true;
        handle->isNewRxFrame = true;
      }
      state->stage = ENP_PROT_STAGE_SYNC1;
      break;

    default:
      state->stage = ENP_PROT_STAGE_SYNC1;
      break;
    }
  }
  return isFrameParsed;
}

static void FormErrorFrame(enpFrame_t *frame, uint8_t error) {
  frame->cmd |= ENP_CMD_ERROR;
  frame->data[0] = error;
  frame->len = 2;
}

void ENP_AnswerProc(ENP_Handle_t *handle) {
  uint16_t id, prop, varId, varNum, varCounter, nodeNum;
  uint32_t value;
  const ENP_Node_t *node;
  uint8_t varIndex = 0;
  enpFrame_t *txFrame = &handle->txFrame;
  enpFrame_t *rxFrame = &handle->rxFrame;
  uint32_t stringLenght = 0;
  const char *str;
  bool isNodeValid = false;
  char *txBuff = handle->txBuf;
  uint16_t crcIndex;

  // if frame was parsed. preparation of an answer
  if (handle->isNewRxFrame) {
    memcpy(txFrame, rxFrame, sizeof(enpFrame_t));
    // command handler
    switch (rxFrame->cmd) {
    // get number of nodes
    case ENP_CMD_GETNODENUM:
      Write16(&txFrame->data[0], ENP_NodeNum);
      txFrame->len = 3;
      break;

    // get node description
    case ENP_CMD_GETNODEDESCR:
      nodeNum = Read16(&rxFrame->data[0]); // node number
      if (nodeNum < ENP_NodeNum) {
        Write16(&txFrame->data[0], nodeNum);
        Write16(&txFrame->data[2], ENP_NodeList[nodeNum]->id);
        Write16(&txFrame->data[4], ENP_NodeList[nodeNum]->pid);
        Write16(&txFrame->data[6], ENP_NodeList[nodeNum]->varNum);
        txFrame->len = 9; // 8 byte data + 1 command byte
        node = ENP_NodeList[nodeNum];
        str = node->name;
        stringLenght = strlen(str);
        // check string lenght before copy
        if (stringLenght >= ENP_PAYLOAD_MAX_SIZE - txFrame->len) {
          str = STANDART_NAME;
          stringLenght = strlen(str);
        }
        strcpy((char *)&txFrame->data[8], str);
        txFrame->len += stringLenght;
      } else {
        FormErrorFrame(txFrame, ENP_ERROR_NODEID);
      }
      break;

    // get description of variable
    case ENP_CMD_GETVARDESCR:
      id = Read16(&rxFrame->data[0]);
      varNum = Read16(&rxFrame->data[2]); // the variable number
      node = ENP_FindNode(id);
      if (node) {
        if (varNum < node->varNum) {
          prop = node->varAttr[varNum].prop;
          Write16(&txFrame->data[0], id);
          Write16(&txFrame->data[2], varNum);
          Write16(&txFrame->data[4], prop);
          txFrame->len = 7; // 6 data bytes + 1 byte cmd
          str = node->varAttr[varNum].name;
          stringLenght = strlen(str);
          // check string lenght before copy
          if (stringLenght >= ENP_PAYLOAD_MAX_SIZE - txFrame->len) {
            str = STANDART_NAME;
            stringLenght = strlen(str);
          }
          strcpy((char *)&txFrame->data[6], str);
          txFrame->len += stringLenght;
        } else {
          FormErrorFrame(txFrame, ENP_ERROR_VARID);
        }
      } else {
        FormErrorFrame(txFrame, ENP_ERROR_NODEID);
      }
      break;

    // get variable value
    case ENP_CMD_GETVARS:
      id = Read16(&rxFrame->data[0]);
      varId = Read16(&rxFrame->data[2]);  // first variable number
      varNum = Read16(&rxFrame->data[4]); // number of variables
      node = ENP_FindNode(id);
      if (node) {
        varIndex = 6; // id, first variable and count of variables
        Write16(&txFrame->data[0], id);
        Write16(&txFrame->data[2], varId);
        txFrame->len = 7; // cmd + id + varId + varCounter
        varCounter = 0;
        isNodeValid = true;
        for (int j = varId; j < varNum; j++) {
          /*C compiler processes IF from right to left. Nested construction is
           * needed for unambiguous execution and understanding of the reader*/
          varCounter++;
          if (node->VarGetVal) {
            if (node->VarGetVal(id, j, &value) == ENP_ERROR_NONE) {
              // write variable
              Write32(&txFrame->data[varIndex], value);
              varIndex += 4;
              txFrame->len += 4;
              if (varIndex >= ENP_PAYLOAD_MAX_SIZE) {
                isNodeValid = false;
                break;
              }
            } else {
              isNodeValid = false;
              break;
            }
          } else {
            isNodeValid = false;
            break;
          }
        }
        if (isNodeValid) {
          Write16(&txFrame->data[4], varCounter);
        } else {
          FormErrorFrame(txFrame, ENP_ERROR_VARID);
          break;
        }
      } else {
        FormErrorFrame(txFrame, ENP_ERROR_NODEID);
      }
      break;

    // set variable value
    case ENP_CMD_SETVARS:
      id = Read16(&rxFrame->data[0]);
      varId = Read16(&rxFrame->data[2]);  // first variable number
      varNum = Read16(&rxFrame->data[4]); // number of variables
      node = ENP_FindNode(id);
      if (node) {
        Write16(&txFrame->data[0], id);
        Write16(&txFrame->data[2], varId);
        varCounter = 0;
        for (int j = 0; j < varNum; j++, varId++) {
          value = Read32(&rxFrame->data[6 + (j << 2)]);
          varCounter++;
          /*C compiler processes IF from right to left. Nested construction is
           * needed for unambiguous execution and understanding of the reader*/
          if (node->VarSetVal) {
            if (node->VarSetVal(id, varId, &value) != ENP_ERROR_NONE) {
              isNodeValid = false;
              break;
            }
          } else {
            isNodeValid = false;
            break;
          }
        }
        if (isNodeValid) {
          Write16(&txFrame->data[4], varCounter);
        } else {
          FormErrorFrame(txFrame, ENP_ERROR_VARID);
          break;
        }
      } else {
        FormErrorFrame(txFrame, ENP_ERROR_NODEID);
      }
      break;

    // Unknown command
    default:
      FormErrorFrame(txFrame, ENP_ERROR_COMMAND);
      break;
    }

    txFrame->crc = CalcFrameCrc(txFrame);
    crcIndex = txFrame->len + 5; // sync + id + cmd
    memcpy(txBuff, txFrame, txFrame->len + 5);
    txBuff[crcIndex++] = txFrame->crc & 0xFF;
    txBuff[crcIndex] = (txFrame->crc >> 8) & 0xFF;
    handle->txLen = txFrame->len + 7;

    if (handle->TxFun) {
      handle->TxFun(txBuff, handle->txLen);
    }
    handle->isNewRxFrame = false;
  }
}
