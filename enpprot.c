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

// Обработчик протокола
void ENP_Proc(ENP_Handle_t *handle) {
  uint16_t id, prop, header;
  uint32_t value;
  char *rxbuf = handle->rxBuf;
  char *txbuf = handle->txBuf;
  const ENP_Node_t *node;
  int i, j, k, n, pos, len, byte, cntr;
  ENP_Pack_t *pack;

  len = 0;
  // отправляем ответ
  if (handle->txLen) {
    if (handle->TxFun(txbuf, handle->txLen)) {
      handle->txLen = 0;
    } else {
      return;
    }
  }

  // добавляем новые символы в буфер приёма
  n = 1;
  while ((byte = handle->RxFun()) >= 0) {
    if (handle->rxLen < ENP_BUFFSIZE) {
      rxbuf[handle->rxLen++] = byte;
    } else {
      for (i = 0; i < ENP_BUFFSIZE - 1; i++) {
        rxbuf[i] = rxbuf[i + 1];
      }
      rxbuf[i] = byte;
    }
    n = 0;
  }
  if (n) {
    return;
  }

  // ищем пакет
  for (pos = handle->rxLen - 8; pos >= 0; pos--) {
    header = ENP_ReadWord(rxbuf + pos);
    id = ENP_ReadWord(rxbuf + pos + 2);
    n = rxbuf[pos + 4];
    if (header == 0xCEFA && (id == handle->devId1 || id == handle->devId2) &&
        pos + n + 7 <= handle->rxLen &&
        CRC16(rxbuf + pos, n + 5, 0xFFFF, 1) ==
            ENP_ReadWord(rxbuf + pos + n + 5)) {
      // формируем ответ
      txbuf[0] = rxbuf[pos];
      txbuf[1] = rxbuf[pos + 1];
      txbuf[2] = rxbuf[pos + 2];
      txbuf[3] = rxbuf[pos + 3];
      txbuf[5] = rxbuf[pos + 5];

      // обрабатываем команду
      switch (txbuf[5]) {
      // получить количество узлов
      case ENP_CMD_GETNODENUM:
        ENP_WriteWord(txbuf + 6, ENP_NodeNum);
        len = 8;
        break;

      // получить описание узла
      case ENP_CMD_GETNODEDESCR:
        n = ENP_ReadWord(rxbuf + pos + 6);
        if (n < ENP_NodeNum) {
          // записываем номер узла конфигурации
          ENP_WriteWord(txbuf + 6, n);
          // записываем идентификатор узла конфигурации
          ENP_WriteWord(txbuf + 8, ENP_NodeList[n]->id);
          // записываем идентификатор родительского узла
          ENP_WriteWord(txbuf + 10, ENP_NodeList[n]->pid);
          // записываем количество переменных
          ENP_WriteWord(txbuf + 12, ENP_NodeList[n]->varNum);
          // записываем описание узла конфигурации
          len = 14;
          if (ENP_NodeList[n]->NodeGetName) {
            ENP_NodeList[n]->NodeGetName(ENP_NodeList[n]->id, txbuf + len);
          } else {
            txbuf[len] = 0;
          }
          // ищем конец описания переменной
          while (txbuf[len++]) {
          }
        } else {
          txbuf[5] |= 0x80;
          txbuf[6] = ENP_ERROR_NODEID;
          len = 7;
        }
        break;

      // получить описание переменной
      case ENP_CMD_GETVARDESCR:
        id = ENP_ReadWord(rxbuf + pos + 6);
        n = ENP_ReadWord(rxbuf + pos + 8);
        node = ENP_FindNode(id);
        if (node) {
          if (node->VarGetAttr(id, n, txbuf + 12, &prop) == ENP_ERROR_NONE) {
            // записываем идентификатор узла
            ENP_WriteWord(txbuf + 6, id);
            // записываем номер переменной
            ENP_WriteWord(txbuf + 8, n);
            // записываем свойства переменной
            ENP_WriteWord(txbuf + 10, prop);
            len = 12;
            // ищем конец описания переменной
            while (txbuf[len++]) {
            }
          } else {
            txbuf[5] |= 0x80; // ошибка - неверный номер переменной
            txbuf[6] = ENP_ERROR_VARID;
            len = 7;
          }
        } else {
          txbuf[5] |= 0x80; // ошибка - неверный идентификатор узла
          txbuf[6] = ENP_ERROR_NODEID;
          len = 7;
        }
        break;

      // получить список неисправных узлов
      case ENP_CMD_GETERRORNODES:
        len = 8;
        for (n = 0, i = 0; i < ENP_NodeNum; i++) {
          node = ENP_NodeList[i];
          if (node->VarGetAttr && node->VarGetVal) {
            for (j = 0; j < node->varNum; j++) {
              if (len <= 260 &&
                  node->VarGetAttr(node->id, j, 0, &prop) == ENP_ERROR_NONE &&
                  (prop & ENP_PROP_ERROR) &&
                  node->VarGetVal(node->id, j, &value) == ENP_ERROR_NONE &&
                  value) {
                // записываем идентификатор узла
                ENP_WriteWord(txbuf + len, node->id);
                len += 2;
                ENP_WriteDoubleWord(txbuf + len, value);
                len += 4;
                n++;
              }
            }
          }
        }
        // записываем кол-во неисправных узлов
        ENP_WriteWord(txbuf + 6, n);
        break;

      // получить значение переменных
      case ENP_CMD_GETVARS:
        id = ENP_ReadWord(rxbuf + pos + 6); // читаем идентификатор узла
        i = ENP_ReadWord(rxbuf + pos + 8); // читаем номер первой переменной
        n = ENP_ReadWord(rxbuf + pos + 10); // читаем количество переменных
        node = ENP_FindNode(id);
        if (node) {
          ENP_WriteWord(txbuf + 6, id); // записываем идентификатор узла
          ENP_WriteWord(txbuf + 8, i); // записываем номер первой переменной
          len = 12;
          for (j = 0; j < n; j++, i++) {
            if (len <= 260 && node->VarGetVal &&
                node->VarGetVal(id, i, &value) == ENP_ERROR_NONE) {
              // записываем значение переменной
              ENP_WriteDoubleWord(txbuf + len, value);
              len += 4;
            } else {
              break;
            }
          }
          ENP_WriteWord(txbuf + 10, j); // записываем количество переменных
        } else {
          txbuf[5] |= 0x80; // ошибка - неверный идентификатор узла
          txbuf[6] = ENP_ERROR_NODEID;
          len = 7;
        }
        break;

      // установить значения переменных
      case ENP_CMD_SETVARS:
        id = ENP_ReadWord(rxbuf + pos + 6); // читаем идентификатор узла
        i = ENP_ReadWord(rxbuf + pos + 8); // читаем номер первой переменной
        n = ENP_ReadWord(rxbuf + pos + 10); // читаем количество переменных
        j = (rxbuf[pos + 4] - 7) >> 2; // ограничиваем кол-во переменных
        if (n > j) {
          n = j;
        }
        node = ENP_FindNode(id);
        if (node) {
          ENP_WriteWord(txbuf + 6, id); // записываем идентификатор узла
          ENP_WriteWord(txbuf + 8, i); // записываем номер первой переменной
          len = 12;
          for (j = 0; j < n; j++, i++) {
            value = ENP_ReadDoubleWord(rxbuf + pos + 12 + (j << 2));
            if (node->VarSetVal &&
                node->VarSetVal(id, i, &value) == ENP_ERROR_NONE &&
                node->VarGetVal &&
                node->VarGetVal(id, i, &value) == ENP_ERROR_NONE) {
              // записываем значение переменной
              ENP_WriteDoubleWord(txbuf + len, value);
              len += 4;
            } else {
              break;
            }
          }
          // записываем количество переменных
          ENP_WriteWord(txbuf + 10, j);
        } else {
          // ошибка - неверный идентификатор узла
          txbuf[5] |= 0x80;
          txbuf[6] = ENP_ERROR_NODEID;
          len = 7;
        }
        break;

      // установить значения п.к.(с контролем установки)
      case ENP_CMD_SETSTREAM:
        id = ENP_ReadWord(rxbuf + pos + 6); // читаем идентификатор узла
        cntr = rxbuf[pos + 8]; // читаем циклический номер запроса
        i = rxbuf[pos + 9]; // читаем номер первой переменной
        n = rxbuf[pos + 10]; // читаем количество переменных
        node = ENP_FindNode(id); // проверка существования узла
        if (node) {
          // если узел новый или счетчик пакетов отличается больше чем на 1
          // начинаем новый прием данных
          if (id != handle->sid || ((cntr - handle->packCntr) & 0xFF) > 1) {
            handle->sid = id;
            handle->packCntr = (cntr - 1) & 0xFF;
          }
          // запись данных
          if (((handle->packCntr + 1) & 0xFF) == cntr) {
            for (j = 0; j < n; j++, i++) {
              value = ENP_ReadDoubleWord(rxbuf + pos + 11 + (j << 2));
              if (!node->VarSetVal) {
                value = ENP_ERROR_NODEID;
                break;
              } else
                value = node->VarSetVal(id, i, &value);
              if (value != ENP_ERROR_NONE) {
                break;
              }
            }
            // обновление счетчика пакетов
            handle->packCntr = cntr;
          }
          // повтор предыдущего ответа
          else if (handle->packCntr == cntr) {
            value = ENP_ERROR_REPLY;
          }

          // запись ответа в буфер
          txbuf[5] = ENP_CMD_ACKSTREAM;
          // записываем идентификатор узла
          ENP_WriteWord(txbuf + 6, id);
          txbuf[8] = handle->packCntr;
          txbuf[9] = value;
          len = 10;
        } else {
          txbuf[5] |= 0x80; // ошибка - неверный идентификатор узла
          txbuf[6] = ENP_ERROR_NODEID;
          len = 7;
        }
        break;

      // получить значения пакета переменных
      case ENP_CMD_GETPACK0:
      case ENP_CMD_GETPACK1:
      case ENP_CMD_GETPACK2:
      case ENP_CMD_GETPACK3:
      case ENP_CMD_GETPACK4:
      case ENP_CMD_GETPACK5:
      case ENP_CMD_GETPACK6:
      case ENP_CMD_GETPACK7:
        // определяем пакет переменных
        pack = ENP_Pack + (n = txbuf[5] - ENP_CMD_GETPACK0);
        len = 8;
        if (ENP_Pack && n < ENP_PACKNUM) {
          for (j = 0; j < pack->varNum; j++) {
            id = pack->varId[j] >> 16;
            i = pack->varId[j] & 0xFFFF;
            node = ENP_FindNode(id);
            if (len <= 260 && node && node->VarGetVal &&
                node->VarGetVal(id, i, &value) == ENP_ERROR_NONE) {
              // записываем значение переменной
              ENP_WriteDoubleWord(txbuf + len, value);
              len += 4;
            } else {
              break;
            }
          }
          // записываем контрольную сумму переменных пакета
          ENP_WriteWord(txbuf + 6, CRC16(&pack->varId[0], j * sizeof(uint32_t),
                                         0xFFFF, sizeof(char)));
        } else {
          txbuf[5] |= 0x80; // ошибка - неверная команда
          txbuf[6] = ENP_ERROR_COMMAND;
          len = 7;
        }
        break;

      // установить значения пакета переменных
      case ENP_CMD_SETPACK0:
      case ENP_CMD_SETPACK1:
      case ENP_CMD_SETPACK2:
      case ENP_CMD_SETPACK3:
      case ENP_CMD_SETPACK4:
      case ENP_CMD_SETPACK5:
      case ENP_CMD_SETPACK6:
      case ENP_CMD_SETPACK7:
        // определяем пакет переменных
        pack = ENP_Pack + (n = txbuf[5] - ENP_CMD_SETPACK0);
        k = (rxbuf[pos + 4] - 3) >> 2; // определяем кол-во переменных
        // проверяем контрольную сумму переменных пакета
        if (ENP_Pack && n < ENP_PACKNUM && k < pack->varNum) {
          if (CRC16(&pack->varId[0], k * sizeof(uint32_t), 0xFFFF,
                    sizeof(char)) == ENP_ReadWord(rxbuf + pos + 6)) {
            len = 6;
            for (j = 0; j < k; j++) {
              id = pack->varId[j] >> 16;
              i = pack->varId[j] & 0xFFFF;
              node = ENP_FindNode(id);
              value = ENP_ReadDoubleWord(rxbuf + pos + 8 + (j << 2));
              if (len <= 260 && node && node->VarSetVal &&
                  node->VarSetVal(id, i, &value) == ENP_ERROR_NONE &&
                  node->VarGetVal &&
                  node->VarGetVal(id, i, &value) == ENP_ERROR_NONE) {
                // записываем значение переменной
                ENP_WriteDoubleWord(txbuf + len, value);
                len += 4;
              } else
                break;
            }
          } else {
            txbuf[5] |= 0x80; // ошибка - неверный идентификатор переменной
            txbuf[6] = ENP_ERROR_NODEID;
            len = 7;
          }
        } else {
          txbuf[5] |= 0x80; // ошибка - неверная команда
          txbuf[6] = ENP_ERROR_COMMAND;
          len = 7;
        }
        break;

      // навигационное решение
      case ENP_CMD_NAVSOL:
        len = 0;
        break;

      // неверная команда
      default:
        txbuf[5] |= 0x80;
        txbuf[6] = ENP_ERROR_COMMAND;
        len = 7;
        break;
      }
      if (len) {
        txbuf[4] = len - 5; // записываем длину данных
        // записываем контрольную сумму
        ENP_WriteWord(txbuf + len, CRC16(txbuf, len, 0xFFFF, 1));
        len += 2;
        handle->txLen = len; // отправляем ответ
        if (handle->TxFun(txbuf, handle->txLen)) {
          handle->txLen = 0;
        }
      }
      // очищаем буфер приёма
      len = handle->rxLen;
      for (i = 0, j = pos + rxbuf[pos + 4] + 7; j < len;
           rxbuf[i++] = rxbuf[j++]) {
      }

      handle->rxLen = i;
      break;
    }
  }
}

static uint16_t CalcFrameCrc(enpFrame_t const *const frame) {
  uint16_t crc = 0xFFFF;
  crc = CRC16(&frame->sync1, 1, crc, 1);
  crc = CRC16(&frame->sync2, 1, crc, 1);
  crc = CRC16(&frame->id, 2, crc, 1);
  crc = CRC16(&frame->len, 1, crc, 1);
  crc = CRC16(&frame->cmd, 1, crc, 1);
  if (frame->len > 1) {
    crc = CRC16(frame->data, frame->len - 1, crc, 1);
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
