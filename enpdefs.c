/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enpapi.h"
#include "stdlib.h"
#include "string.h"

#define FLASH_ALIGN_LEN 256
static ENP_FlashDataChunk_t dataChunk;
// Параметры конфигурации
static uint8_t flashDataBuffer[FLASH_ALIGN_LEN];

// Список узлов конфигурации
const ENP_Node_t** ENP_NodeList = 0;

// Максимальное количество узлов конфигурации
uint16_t ENP_MaxNodeNum = 0;

// Количество узлов конфигурации
uint16_t ENP_NodeNum = 0;

static bool ENP_isValidNode(const ENP_Node_t* node);

// Добавление узла конфигурации
uint16_t ENP_InsertNode(const ENP_Node_t* node, uint16_t num) {
  int i, j, n = 0;

  if (node && ENP_NodeList) {
    for (; n < num; n++, node++) {
      if (ENP_NodeNum < ENP_MaxNodeNum) {
        // ищем место в списке
        for (i = 0; i < ENP_NodeNum && ENP_NodeList[i]->id < node->id; i++)
          ;
        // сдвигаем список
        for (j = ENP_NodeNum++; j > i; j--)
          ENP_NodeList[j] = ENP_NodeList[j - 1];
        ENP_NodeList[i] = node;
      } else {
        break;
      }
    }
  }
  return n;
}

// Удаление узла конфигурации
uint16_t ENP_DeleteNode(const ENP_Node_t* node, uint16_t num) {
  int i, n = 0;

  if (node && ENP_NodeList) {
    for (; n < num; n++, node++) {
      // ищем узел в списке
      for (i = 0; i < ENP_NodeNum && ENP_NodeList[i] != node; i++) {
      }
      if (i == ENP_NodeNum) {
        break;
      } else {
        // сдвигаем список
        for (; i < ENP_NodeNum - 1; i++) {
          ENP_NodeList[i] = ENP_NodeList[i + 1];
        }
        ENP_NodeNum--;
      }
    }
  }
  return n;
}

// Поиск узла конфигурации по идентификатору
const ENP_Node_t* ENP_FindNode(uint16_t nodeid) {
  uint16_t n;

  if (!ENP_NodeList) {
    return 0;
  }
  for (n = 0; n < ENP_NodeNum && ENP_NodeList[n]->id != nodeid; n++) {
  }

  return n == ENP_NodeNum ? 0 : ENP_NodeList[n];
}

// Получение имени узла конфигурации
uint16_t ENP_NodeName(uint16_t nodeid, char* name) {
  const ENP_Node_t* node = ENP_FindNode(nodeid);
  const char* str;

  if (node && name) {
    str = node->name;
    while (*str) {
      *name++ = *str++;
    }
    *name = 0;
    return ENP_ERROR_NONE;
  } else
    return ENP_ERROR_NODEID;
}

// Получение аттрибутов переменной конфигурации
uint16_t ENP_VarGetAttr(uint16_t nodeid,
                        uint16_t varid,
                        char* name,
                        uint16_t* prop) {
  const ENP_Node_t* node;
  const char* str;

  node = ENP_FindNode(nodeid);
  if (node) {
    if (varid < node->varNum && node->varAttr) {
      if (name) {
        str = node->varAttr[varid].name;
        while (*str) {
          *name++ = *str++;
        }
        *name = 0;
      }
      if (prop) {
        *prop = node->varAttr[varid].prop;
      }
      return ENP_ERROR_NONE;
    } else {
      return ENP_ERROR_VARID;
    }
  } else
    return ENP_ERROR_NODEID;
}

// Проверка конфигурации на наличие ошибок
uint8_t ENP_Error() {
  const ENP_Node_t* node;
  uint16_t i, n, prop;
  uint32_t value;

  if (ENP_NodeList) {
    for (n = 0; n < ENP_NodeNum; n++) {
      node = ENP_NodeList[n];
      for (i = 0; i < node->varNum; i++) {
        if (node->VarGetAttr &&
            node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
            (prop & ENP_PROP_ERROR) && node->VarGetVal &&
            node->VarGetVal(node->id, i, &value) == ENP_ERROR_NONE && value) {
          return 1;
        }
      }
    }
  }
  return 0;
}

// Сохранение данных в флэш память
uint8_t ENP_SaveData(void* dest, const void* src, int size) {
  // сохраняем флаг смены прошивки
  uint32_t eraseAddr = (uint32_t)dest;
  ENP_FlashErase(eraseAddr, eraseAddr + size);
  // сохраняем параметры конфигурации
  ENP_FlashWrite(dest, src, size);
  return 0;
}

bool ENP_isParsValid(const ENP_Pars_t* const pars) {
  bool ret = false;
  // проверяем контрольную сумму
  int sizeParams = sizeof(ENP_Pars_t) - sizeof(pars->ckSum);
  if (CRC16(pars, sizeParams, 0xFFFF) == pars->ckSum) {
    return true;
  }
  return ret;
}

__weak uint8_t ENP_FlashWrite(void* dst, const void* ptr, int size) {
  return 0;
}

__weak uint8_t ENP_FlashErase(uint32_t startAddr, uint32_t stopAddr) {
  return 0;
}

// Загрузка переменных узла конфигурации
uint8_t ENP_LoadNode(void* pFlash, const ENP_Node_t* node) {
  ENP_FlashDataChunk_t* chunk = (ENP_FlashDataChunk_t*)pFlash;

  if (ENP_isValidNode(node) && node) {
    // Search a data in the linked list
    while (chunk != 0) {
      uint32_t crcSize = sizeof(dataChunk) - sizeof(dataChunk.ckSum);
      uint32_t crc = CRC32(chunk, crcSize, 0xFFFFFFFF);
      if (crc == chunk->ckSum) {
        for (int i = 0; i < chunk->numVars; i++) {
          // in flash memory stored only CONST type data
          if (chunk->data[i].nodeId == node->id) {
            uint16_t varId = chunk->data[i].varId;
            uint32_t value = chunk->data[i].varValue;
            node->VarSetVal(node->id, varId, &value);
          }
        }
        chunk = (ENP_FlashDataChunk_t*)chunk->nextAddress;
      } else {
        return 1;  // bad config
      }
    }
  }
  return 0;
}

// Сохранение переменных конфигурации
uint8_t ENP_Save(void* pFlash) {
  uint16_t prop;
  ENP_SavedVar_t var;
  uint32_t val;
  static uint32_t ch = 0;
  uint8_t* flashPtr = (uint8_t*)pFlash;
  dataChunk.nextAddress = (uint32_t)(pFlash) + FLASH_ALIGN_LEN;

  if (ENP_NodeList) {
    ENP_FlashErase((uint32_t)flashPtr, (uint32_t)flashPtr);
    for (int n = 0; n < ENP_NodeNum; n++) {
      // Calculation of the number of the stored parameters
      const ENP_Node_t* node = ENP_NodeList[n];
      if (ENP_isValidNode(node)) {
        for (int i = 0; i < node->varNum; i++) {
          node->VarGetAttr(node->id, i, 0, &prop);
          if (prop & ENP_PROP_CONST) {
            node->VarGetVal(node->id, i, &val);
            var.nodeId = node->id;
            var.varId = i;
            var.varValue = val;
            dataChunk.data[ch] = var;
            ch++;
            if (ch >= NUM_VARS_IN_CHUNK) {
              dataChunk.numVars = NUM_VARS_IN_CHUNK;
              uint32_t crcSize = sizeof(dataChunk) - sizeof(dataChunk.ckSum);
              dataChunk.ckSum = CRC32(&dataChunk, crcSize, 0xFFFFFFFF);
              ENP_AssertParam(sizeof(flashDataBuffer) >= sizeof(dataChunk));
              memcpy(flashDataBuffer, &dataChunk, sizeof(dataChunk));
              ENP_FlashWrite(flashPtr, flashDataBuffer,
                             sizeof(flashDataBuffer));
              flashPtr += FLASH_ALIGN_LEN;
              dataChunk.nextAddress = (uint32_t)flashPtr + FLASH_ALIGN_LEN;
              ch = 0;
            }
          }
        }
      }
    }
    // Save remaining data
    dataChunk.numVars = ch;
    dataChunk.nextAddress = 0;
    uint32_t crcSize = sizeof(dataChunk) - sizeof(dataChunk.ckSum);
    dataChunk.ckSum = CRC32(&dataChunk, crcSize, 0xFFFFFFFF);
    ENP_AssertParam(sizeof(flashDataBuffer) >= sizeof(dataChunk));
    memcpy(flashDataBuffer, &dataChunk, sizeof(dataChunk));
    ENP_FlashWrite(flashPtr, flashDataBuffer, sizeof(flashDataBuffer));
  }
  return 0;
}

// Assert
void ENP_AssertFailed(uint8_t* file, uint32_t line) {
  for (;;) {
  }
}

// true - node is valid
static bool ENP_isValidNode(const ENP_Node_t* node) {
  uint16_t prop;
  uint32_t val;
  if (node) {
    if (node->VarGetAttr && node->VarGetVal && node->VarSetVal) {
      for (int i = 0; i < node->varNum; i++) {
        uint16_t retAttr = node->VarGetAttr(node->id, i, 0, &prop);
        uint16_t retGetVal = node->VarGetVal(node->id, i, &val);
        if ((retAttr != ENP_ERROR_NONE) || (retGetVal != ENP_ERROR_NONE)) {
          return false;
        }
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
  return true;
}
