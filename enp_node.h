/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef _NODE_CPP_H
#define _NODE_CPP_H

#include "stdint.h"

namespace ENP {
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
