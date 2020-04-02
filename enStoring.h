/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef EN_STORING_H_
#define EN_STORING_H_
#include "eNode.h"
#include "stdint.h"

//The number of variables in data chunk
#define NUM_VARS_IN_CHUNK 30 

#define ENP_AssertParam(expr) ((expr) ? (void)0U : ENP_AssertFailed((uint8_t *)__FILE__, __LINE__))

// Shared information
typedef struct {
  uint32_t devId;     // device is
  uint32_t fwUpdate;  // change firmware flag
  uint32_t reserv1;
  uint32_t reserv2;
  uint32_t reserv3;
  uint32_t reserv4;
  uint32_t reserv5;
  uint32_t ckSum;     // CRC32
} ENP_ShareInformation_t;

// variable in a memory
typedef struct {
  uint16_t nodeId;
  uint16_t varId;
  uint32_t varValue; // value of the variable
} ENP_SavedVar_t;

typedef struct {
    uint16_t numVars;
    ENP_SavedVar_t data[NUM_VARS_IN_CHUNK];
    uint32_t nextAddress;
    uint32_t ckSum;     // checksum of the chunk
} ENP_FlashDataChunk_t;

// ENP flash interface
uint8_t ENP_FlashErase(uint32_t startAddr, uint32_t stopAddr);
uint8_t ENP_FlashWrite(void *dst, const void *ptr, int size);

// Save nodes configuration
uint8_t ENP_SaveNodes(void* pFlash);
// Load node
uint8_t ENP_LoadNode(void *pFlash, const ENP_Node_t *node);

// Assert
void ENP_AssertFailed(uint8_t* file, uint32_t line);

#endif