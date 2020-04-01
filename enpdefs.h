/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef ENPDEFS_H_
#define ENPDEFS_H_
#include "enpapi.h"
#include "stdint.h"

#if  defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif /* __weak */
  #ifndef __packed
    #define __packed __attribute__((__packed__))
  #endif /* __packed */
#endif /* __GNUC__ */

// Коды ошибок конфигурации
#define ENP_ERROR_NONE 0    // нет ошибок
#define ENP_ERROR_NODENUM 1 // недопустимый номер узла
#define ENP_ERROR_NODEID 2 // неверный идентификатор узла конфигурации
#define ENP_ERROR_VARID 3 // неверный идентификатор переменной конфигурации
#define ENP_ERROR_VARVAL 4 // неверное значение переменной конфигурации
#define ENP_ERROR_COMMAND 5 // неверная команда
#define ENP_ERROR_ACCESSN 6 // отказ в доступе
#define ENP_ERROR_REPLY 7   // повтор предыдущего ответа
#define ENP_ERROR_UNKNOWN 255 // неизвестная ошибка

// Тип переменной
#define ECP_TYPE_INT 0x0001   // целое число со знаком
#define ENP_TYPE_UINT 0x0002  // целое число без знака
#define ENP_TYPE_BOOL 0x0003  // логическое значение
#define ENP_TYPE_REAL 0x0004  // число с плавающей точкой
#define ENP_TYPE_HEX 0x0005   // шестнадцатеричное число
#define ENP_TYPE_CHAR4 0x0006 // строка 4 символа
#define ENP_TYPE_TIME 0x0007  // время
#define ENP_TYPE_MASK 0x000F  // маска типа переменной

// Свойства переменной
#define ENP_PROP_READONLY 0x0010 // только для чтения
#define ENP_PROP_CONST 0x0020 // константа (сохраняется во флэш)
#define ENP_PROP_ERROR 0x0040 // содержит код ошибки
#define ENP_PROP_TRACE 0x0080 // вести трассировку
#define ENP_PROP_MASK 0x00F0  // маска свойства

#define ENP_NAMECHARS 64 // Максимальная длина описания узла или переменной
#define ENP_USERPARNUM 50 // Кол-во пользовательских параметров конфигурации

// Аттрибуты переменной конфигурации
typedef struct {
  const char *name; // имя переменной
  uint16_t prop;    // свойства переменной
} ENP_Attr_t;

// Узел конфигурации
typedef struct {
  const char *name;          // имя узла
  uint16_t id;               // идентификатор узла
  uint16_t pid;              // идентификатор родителя
  uint16_t varNum;           // количество переменных
  const ENP_Attr_t *varAttr; // аттрибуты переменной
  // чтение переменной
  uint16_t (*VarGetVal)(uint16_t nodeid, uint16_t varid, void *value);
  // запись переменной
  uint16_t (*VarSetVal)(uint16_t nodeid, uint16_t varid, void *value);
  // получение аттрибутов переменной
  uint16_t (*VarGetAttr)(uint16_t nodeid, uint16_t varid, char *name,
                         uint16_t *prop);
  // получение имени узла
  uint16_t (*NodeGetName)(uint16_t nodeid, char *name);
} ENP_Node_t;

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
  uint32_t user[ENP_USERPARNUM]; // параметры пользователя
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

// Список узлов конфигурации
extern const ENP_Node_t **ENP_NodeList;

// Максимальное количество узлов конфигурации
extern uint16_t ENP_MaxNodeNum;

// Количество узлов конфигурации
extern uint16_t ENP_NodeNum;

// Поиск узла конфигурации по идентификатору
extern const ENP_Node_t *ENP_FindNode(uint16_t nodeid);

// Добавление узла конфигурации
extern uint16_t ENP_InsertNode(const ENP_Node_t *node, uint16_t num);

// Удаление узла конфигурации
extern uint16_t ENP_DeleteNode(const ENP_Node_t *node, uint16_t num);

// Получение аттрибутов переменной конфигурации
extern uint16_t ENP_VarGetAttr(uint16_t nodeid, uint16_t varid, char *name,
                               uint16_t *prop);

// Получение имени узла конфигурации
extern uint16_t ENP_NodeName(uint16_t nodeid, char *name);

// Проверка узлов на наличие ошибок
extern uint8_t ENP_Error(void);

// ENP flash interface
uint8_t ENP_FlashErase(uint32_t startAddr, uint32_t stopAddr);
uint8_t ENP_FlashWrite(void* dst, const void* ptr, int size);

// Сохранение параметров конфигурации
extern uint8_t ENP_SavePars(ENP_Data_t *data, uint32_t update);

// Сохранение параметров конфигурации
extern uint8_t ENP_LoadPars(ENP_Data_t *data);

// Сохранение переменных узла конфигурации
extern uint8_t ENP_SaveNode(ENP_Data_t *data, const ENP_Node_t *node,
                            int nodenum);

// Загрузка переменных узла конфигурации
extern uint8_t ENP_LoadNode(ENP_Data_t *data, const ENP_Node_t *node,
                            int nodenum);

// Сохранение переменных конфигурации
extern uint8_t ENP_Save(ENP_Data_t *data, uint32_t update);

// Загрузка переменных конфигурации
extern uint8_t ENP_Load(ENP_Data_t *data);

#endif
