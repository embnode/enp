/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef ENPPROT_H_
#define ENPPROT_H_
#include "enpapi.h"
#include "stdbool.h"

// Размер буферов приёма-передачи
#define ENP_BUFFSIZE 300

// Команды протокола
#define ENP_CMD_GETNODENUM 0x01 // получить количество узлов
#define ENP_CMD_GETNODEDESCR 0x02 // получить свойства узла
#define ENP_CMD_GETVARDESCR 0x03 // получить свойства переменной
#define ENP_CMD_GETERRORNODES 0x04 // получить список неисправных узлов
#define ENP_CMD_GETVARS 0x10 // получить значения переменных конфигурации
#define ENP_CMD_SETVARS 0x11 // установить значения переменных конфигурации
// установить значения переменных конфигурации для потока
#define ENP_CMD_SETSTREAM 0x12
// подтверждение установки значения переменных конфигурации для потока
#define ENP_CMD_ACKSTREAM 0x13
#define ENP_CMD_GETPACK0 0x20 // получить значения переменных из пакета 0
#define ENP_CMD_GETPACK1 0x21 // получить значения переменных из пакета 1
#define ENP_CMD_GETPACK2 0x22 // получить значения переменных из пакета 2
#define ENP_CMD_GETPACK3 0x23 // получить значения переменных из пакета 3
#define ENP_CMD_GETPACK4 0x24 // получить значения переменных из пакета 4
#define ENP_CMD_GETPACK5 0x25 // получить значения переменных из пакета 5
#define ENP_CMD_GETPACK6 0x26 // получить значения переменных из пакета 6
#define ENP_CMD_GETPACK7 0x27 // получить значения переменных из пакета 7
#define ENP_CMD_SETPACK0 0x28 // установить значения переменных из пакета 0
#define ENP_CMD_SETPACK1 0x29 // установить значения переменных из пакета 1
#define ENP_CMD_SETPACK2 0x2A // установить значения переменных из пакета 2
#define ENP_CMD_SETPACK3 0x2B // установить значения переменных из пакета 3
#define ENP_CMD_SETPACK4 0x2C // установить значения переменных из пакета 4
#define ENP_CMD_SETPACK5 0x2D // установить значения переменных из пакета 5
#define ENP_CMD_SETPACK6 0x2E // установить значения переменных из пакета 6
#define ENP_CMD_SETPACK7 0x2F // установить значения переменных из пакета 7
#define ENP_CMD_EMULATION 0x30 // эмуляция измерений датчиков
#define ENP_CMD_NAVSOL 0x40 // навигационное решение
#define ENP_CMD_USER 0x41 // пользовательский пакет данных
#define ENP_CMD_ERROR 0x80 // Error flag
#define ENP_PACKNUM 8     // Кол-во пакетов
#define ENP_PACKVARNUM 50 // Количество переменных пакета

#define ENP_PAYLOAD_MAX_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

// Пакет переменных
typedef struct {
  uint32_t varId[ENP_PACKVARNUM]; // идентификаторы переменных
  char varNum;                    // кол-во переменных
} ENP_Pack_t;

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
}protStage_t;

typedef struct {
    protStage_t stage;
    uint8_t payloadIndex;
}enpFrameState_t;

// экземпляр протокола
typedef struct {
  char rxBuf[ENP_BUFFSIZE]; // буфер приёма
  int rxLen;
  int (*RxFun)();
  char txBuf[ENP_BUFFSIZE]; // буфер передачи
  int txLen;
  int (*TxFun)(const char *, int);
  char packCntr;   // счетчик пакетов
  uint16_t sid;    // идентификатор узла
  uint16_t devId1; // Идентификаторы устройств
  uint16_t devId2;
  enpFrame_t rxFrame;
  enpFrame_t txFrame;
  enpFrameState_t rxFrameState;
  bool isNewRxFrame;
} ENP_Handle_t;

// Инициализация протокола
//------------------------
void ENP_Init(
    // список узлов конфигурации
    const ENP_Node_t **nodelist,
    int maxnodenum, // максимальное кол-во узлов конфигурации
    ENP_Pack_t *pack, // пакеты переменных
    // функция стирания флэш-памяти
    uint8_t (*erasefunc)(uint32_t startAddr, uint32_t stopAddr),
    // функция записи флэш-памяти
    uint8_t (*writefunc)(const void *ptr, void *dst, int size));

// Инициализация обработчика
extern void ENP_InitHandle(ENP_Handle_t *handle, // экземпляр обработчика
                           uint16_t devId1, // идентификаторы устройств
                           uint16_t devId2,
                           int (*rxf)(), // функция приема
                           int (*txf)(const char *, int) // функция передачи
);

// // Обработчик протокола
// extern void ENP_Proc(ENP_Handle_t *handle);
// Чтение слова
extern uint16_t ENP_ReadWord(char *buf);
// Чтение двойного слова
extern uint32_t ENP_ReadDoubleWord(char *buf);
// Запись слова
extern void ENP_WriteWord(char *buf, uint16_t word);
// Запись двойного слова
extern void ENP_WriteDoubleWord(char *buf, uint32_t dword);
// Получение long из char массива
extern long ENP_CharToLong(char *Data);


bool ENP_ParseFrame(ENP_Handle_t* handle, uint8_t* data, uint32_t len);
void ENP_AnswerProc(ENP_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif
