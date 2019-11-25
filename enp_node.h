/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef _NODE_CPP_H
#define _NODE_CPP_H

#include "stdint.h"

namespace ENP {
const uint16_t TYPE_INT = 0x0001; // целое число со знаком int
const uint16_t TYPE_UINT = 0x0002; // целое число без знака uint
const uint16_t TYPE_BOOL = 0x0003; // логическое значение
const uint16_t TYPE_REAL = 0x0004; // число с плавающей точкой float
const uint16_t TYPE_HEX = 0x0005; // шестнадцатеричное число
const uint16_t TYPE_CHAR4 = 0x0006; // строка 4 символа
const uint16_t TYPE_TIME = 0x0007;  // время
const uint16_t TYPE_MASK = 0x000F;  // маска типа переменной
// Максимальная длина описания узла или переменной
const uint8_t MAX_NAME_LEN = 64;
const uint16_t PROP_READONLY = 0x0010; // только для чтения
const uint16_t PROP_CONST = 0x0020; // константа (сохраняется во флэш)
const uint16_t PROP_ERROR = 0x0040; // содержит код ошибки
const uint16_t PROP_TRACE = 0x0080; // вести трассировку
const uint16_t PROP_MASK = 0x00F0;  // маска свойства
// Коды ошибок конфигурации
typedef enum retCode__ {
  ERROR_NONE,    // нет ошибок
  ERROR_NODENUM, // недопустимый номер узла
  ERROR_NODEID, // неверный идентификатор узла конфигурации
  ERROR_VARID, // неверный идентификатор переменной конфигурации
  ERROR_VARVAL, // неверное значение переменной конфигурации
  ERROR_COMMAND,      // неверная команда
  ERROR_ACCESS,       // отказ в доступе
  ERROR_REPLY,        // повтор предыдущего ответа
  ERROR_NULL_POINTER, //передан нулевой указатель
} retCode_t;

class INode {
public:
  // чтение переменной
  virtual ENP::retCode_t getVal(uint16_t varid, void *value) = 0;
  // запись переменной
  virtual ENP::retCode_t setVal(uint16_t varid, void *value) = 0;
  // получение аттрибутов переменной
  virtual ENP::retCode_t getAttr(uint16_t varid, char *name,
                                 uint16_t *prop) = 0;
};
} // namespace ENP

#endif
