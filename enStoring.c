#include "enStoring.h"
#include "string.h"

#define FLASH_ALIGN_LEN 256
static uint8_t flashDataBuffer[FLASH_ALIGN_LEN];
static ENP_FlashDataChunk_t dataChunk;

static bool isNodeValid(const ENP_Node_t *node);

__weak uint8_t ENP_FlashWrite(void *dst, const void *ptr, int size) {
  return 0;
}

__weak uint8_t ENP_FlashErase(uint32_t startAddr, uint32_t stopAddr) {
  return 0;
}

// Save nodes configuration
uint8_t ENP_SaveNodes(void* pFlash) {
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
      if (isNodeValid(node)) {
        for (int i = 0; i < node->varNum; i++) {
          prop = node->varAttr[i].prop;
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
              dataChunk.ckSum = Crc32(&dataChunk, crcSize, 0xFFFFFFFF);
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
    dataChunk.ckSum = Crc32(&dataChunk, crcSize, 0xFFFFFFFF);
    ENP_AssertParam(sizeof(flashDataBuffer) >= sizeof(dataChunk));
    memcpy(flashDataBuffer, &dataChunk, sizeof(dataChunk));
    ENP_FlashWrite(flashPtr, flashDataBuffer, sizeof(flashDataBuffer));
  }
  return 0;
}

// Load node
uint8_t ENP_LoadNode(void *pFlash, const ENP_Node_t *node) {
  ENP_FlashDataChunk_t *chunk = (ENP_FlashDataChunk_t *)pFlash;

  if (isNodeValid(node) && node) {
    // Search a data in the linked list
    while (chunk != 0) {
      uint32_t crcSize = sizeof(dataChunk) - sizeof(dataChunk.ckSum);
      uint32_t crc = Crc32(chunk, crcSize, 0xFFFFFFFF);
      if (crc == chunk->ckSum) {
        for (int i = 0; i < chunk->numVars; i++) {
          // in flash memory stored only CONST type data
          if (chunk->data[i].nodeId == node->id) {
            uint16_t varId = chunk->data[i].varId;
            uint32_t value = chunk->data[i].varValue;
            node->VarSetVal(node->id, varId, &value);
          }
        }
        chunk = (ENP_FlashDataChunk_t *)chunk->nextAddress;
      } else {
        return 1; // bad config
      }
    }
  }
  return 0;
}

// true - node is valid
static bool isNodeValid(const ENP_Node_t *node) {
  bool ret = true;
  if (node->VarGetVal && node->VarSetVal && node) {
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}

// Assert
void ENP_AssertFailed(uint8_t* file, uint32_t line) {
  for (;;) {
  }
}