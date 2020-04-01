/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef EN_STORING_H_
#define EN_STORING_H_
#include "enpdefs.h"
#include "stdint.h"

// Параметры конфигурации
typedef struct {
  uint32_t name;      // имя устройства
  uint32_t code;      // код устройства
  float hwVer;        // версия платы
  float fwVer;        // версия программы
  uint16_t netId;     // идентификатор сети
  uint16_t devId;     // идентификатор устройства
  uint32_t lang;      // язык конфигурации
  uint16_t prescaler; // параметры шины CAN
  uint16_t seg1;
  uint16_t seg2;
  uint16_t sjw;
  uint32_t uart1Bps;             // параметры UART1
  uint32_t uart2Bps;             // параметры UART2
  uint32_t ckSum;                // контрольная сумма
} ENP_Pars_t;

// Параметры конфигурации
extern ENP_Pars_t ENP_Pars;

// Значения переменных узла конфигурации
typedef struct {
  uint32_t ckSum;     // контрольная сумма
  uint16_t nodeId;    // идентификатор узла
  uint16_t varNum;    // кол-во переменных
  uint32_t varVal[1]; // значение переменной
} ENP_Vars_t;

// Данные конфигурации (следить за выравниванием!)
typedef struct {
  uint32_t fwUpdate; // флаг смены прошивки
  ENP_Pars_t pars;   // параметры конфигурации
  ENP_Vars_t vars;   // переменные конфигурации
} ENP_Data_t;

// ENP flash interface
uint8_t ENP_FlashErase(uint32_t startAddr, uint32_t stopAddr);
uint8_t ENP_FlashWrite(void *dst, const void *ptr, int size);

// Сохранение параметров конфигурации
uint8_t ENP_SavePars(ENP_Data_t *data, uint32_t update);

// Сохранение параметров конфигурации
uint8_t ENP_LoadPars(ENP_Data_t *data);

// Сохранение переменных узла конфигурации
uint8_t ENP_SaveNode(ENP_Data_t *, const ENP_Node_t *, int nodenum);

// Загрузка переменных узла конфигурации
uint8_t ENP_LoadNode(ENP_Data_t *, const ENP_Node_t *, int nodenum);

// Сохранение переменных конфигурации
uint8_t ENP_Save(ENP_Data_t *data, uint32_t update);

// Загрузка переменных конфигурации
uint8_t ENP_Load(ENP_Data_t *data);

#endif