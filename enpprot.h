/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef ENPPROT_H_
#define ENPPROT_H_
#include "enpapi.h"

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
#define ENP_CMD_USER 0x41 // пользовательский пакет данных
#define ENP_CMD_WRITE_FIRMWARE 0x42 // обновление прошивки
#define ENP_CMD_INIT_FIRMWARE 0x43 // firmware write init
#define ENP_CMD_COMPLETE_FIRMWARE 0x44 // firmware write complete

/**
 * @brief Команды файловой системы
 * 
 */
#define ENP_CMD_FS_READ_DIR 0x50
#define ENP_CMD_FS_CHANGE_DIR 0x51

// экземпляр протокола
typedef struct {
    char rxBuf[ENP_BUFFSIZE]; // буфер приёма
    int rxLen;
    int (*RxFun)();
    char txBuf[ENP_BUFFSIZE]; // буфер передачи
    int txLen;
    int (*TxFun)(const char*, int);
    char packCntr; // счетчик пакетов
    uint16_t sid; // идентификатор узла
    uint16_t devId1; // Идентификаторы устройств
    uint16_t devId2;
} ENP_Handle_t;

// Инициализация протокола
//------------------------
void ENP_Init(
    // список узлов конфигурации
    const ENP_Node_t** nodelist,
    int maxnodenum // максимальное кол-во узлов конфигурации
);

// Инициализация обработчика
extern void ENP_InitHandle(ENP_Handle_t* handle, // экземпляр обработчика
    uint16_t devId1, // идентификаторы устройств
    uint16_t devId2,
    int (*rxf)(), // функция приема
    int (*txf)(const char*, int) // функция передачи
);

// Обработчик протокола
extern void ENP_Proc(ENP_Handle_t* handle);
// Чтение слова
extern uint16_t ENP_ReadWord(char* buf);
// Чтение двойного слова
extern uint32_t ENP_ReadDoubleWord(char* buf);
// Запись слова
extern void ENP_WriteWord(char* buf, uint16_t word);
// Запись двойного слова
extern void ENP_WriteDoubleWord(char* buf, uint32_t dword);
// Получение long из char массива
extern long ENP_CharToLong(char* Data);
// Write firmware to the internal flash
int ENP_FirmwareWrite(const void*, int);
// Init write firmware
int ENP_FirmwareInit(const void*, int);
// Complete write firmware
int ENP_FirmwareComplete(const void*, int);

#endif
