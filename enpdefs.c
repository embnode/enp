/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enpapi.h"

// Параметры конфигурации
ENP_Pars_t ENP_Pars = {
    // имя устройства
    ' ' + (' ' << 8) + (' ' << 16) + (' ' << 24),
    0,   // код устройства
    0.0, // версия платы
    0.0, // версия программы
    0,   // идентификатор сети
    0,   // идентификатор устройства
    0,   // язык
    3,   // Параметры шины CAN
    9,
    2,
    1,
    115200, // параметры UART1
    9600,   // параметры UART2
    // параметры пользователя
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    0};

// Список узлов конфигурации
const ENP_Node_t **ENP_NodeList = 0;

// Максимальное количество узлов конфигурации
uint16_t ENP_MaxNodeNum = 0;

// Количество узлов конфигурации
uint16_t ENP_NodeNum = 0;

// Функция стирания секторов памяти
uint8_t (*ENP_FlashErase)(uint32_t startAddr, uint32_t stopAddr) = 0;

// Функция записи данных во Flash
uint8_t (*ENP_FlashWrite)(const void *ptr, void *dst, int size) = 0;

// Добавление узла конфигурации
uint16_t ENP_InsertNode(const ENP_Node_t *node, uint16_t num) {
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
uint16_t ENP_DeleteNode(const ENP_Node_t *node, uint16_t num) {
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
const ENP_Node_t *ENP_FindNode(uint16_t nodeid) {
  uint16_t n;

  if (!ENP_NodeList) {
    return 0;
  }
  for (n = 0; n < ENP_NodeNum && ENP_NodeList[n]->id != nodeid; n++) {
  }

  return n == ENP_NodeNum ? 0 : ENP_NodeList[n];
}

// Получение имени узла конфигурации
uint16_t ENP_NodeName(uint16_t nodeid, char *name) {
  const ENP_Node_t *node = ENP_FindNode(nodeid);
  const char *str;

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
uint16_t ENP_VarGetAttr(uint16_t nodeid, uint16_t varid, char *name,
                        uint16_t *prop) {
  const ENP_Node_t *node;
  const char *str;

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
  const ENP_Node_t *node;
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

// Сохранение параметров конфигурации
uint8_t ENP_SavePars(ENP_Data_t *data, uint32_t update) {
  // сохраняем флаг смены прошивки
  if (!ENP_FlashWrite ||
      !ENP_FlashWrite(&update, &data->fwUpdate, sizeof(update))) {
    return 0;
  }
  // вычисляем контрольную сумму параметров конфигурации
  ENP_Pars.ckSum = CRC16(&ENP_Pars, sizeof(ENP_Pars) - sizeof(uint32_t), 0xFFFF,
                         sizeof(char));
  // сохраняем параметры конфигурации
  if (!ENP_FlashWrite(&ENP_Pars, &data->pars, sizeof(ENP_Pars))) {
    return 0;
  }
  return 1;
}

// Загрузка параметров конфигурации
uint8_t ENP_LoadPars(ENP_Data_t *data) {
  // проверяем контрольную сумму
  if (CRC16(&data->pars, sizeof(data->pars) - sizeof(uint32_t), 0xFFFF,
            sizeof(char)) == data->pars.ckSum) {
    // загружаем параметры
    ENP_Pars = data->pars;
    return 1;
  }
  return 0;
}

// Сохранение переменных узла конфигурации
uint8_t ENP_SaveNode(ENP_Data_t *data, const ENP_Node_t *node, int nodenum) {
  ENP_Vars_t *vars;
  uint16_t prop, k;
  uint32_t val;
  int i, n;

  // проверяем указатель на функцию записи
  if (!ENP_FlashWrite) {
    return 0;
  }
  // ищем последнюю запись
  vars = &data->vars;
  while (vars->nodeId < 0xFFFF && vars->varNum < 0xFFFF) {
    vars =
        (ENP_Vars_t *)((char *)vars + ((vars->varNum << 2) + 8) / sizeof(char));
  }
  // для всех узлов
  for (n = 0; n < nodenum; n++, node++) {
    if (node->VarGetAttr && node->VarGetVal) {
      // вычисляем кол-во сохраняемых переменных
      for (k = i = 0; i < node->varNum; i++)
        if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
            (prop & ENP_PROP_CONST))
          k++;
      if (k) {
        // сохраняем идентификатор узла
        if (!ENP_FlashWrite(&node->id, &vars->nodeId, sizeof(node->id)))
          return 0;
        // сохраняем переменные
        for (k = i = 0; i < node->varNum; i++)
          if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
              (prop & ENP_PROP_CONST) &&
              node->VarGetVal(node->id, i, &val) == ENP_ERROR_NONE &&
              !ENP_FlashWrite(&val, &vars->varVal[k++], sizeof(uint32_t)))
            return 0;
        // сохраняем кол-во переменных
        if (!ENP_FlashWrite(&k, &vars->varNum, sizeof(node->varNum))) {
          return 0;
        }
        // вычисляем контрольную сумму
        val = CRC16(&vars->nodeId, (k << 2) + 4, 0xFFFF, sizeof(char));
        // сохраняем контрольную сумму
        if (!ENP_FlashWrite(&val, &vars->ckSum, sizeof(val))) {
          return 0;
        }
        // переходим к следующей записи
        vars = (ENP_Vars_t *)((char *)vars + ((k << 2) + 8) / sizeof(char));
      }
    }
  }
  return 1;
}

// Загрузка переменных узла конфигурации
uint8_t ENP_LoadNode(ENP_Data_t *data, const ENP_Node_t *node, int nodenum) {
  ENP_Vars_t *vars;
  uint16_t prop, i, n, k;
  int res = 1;

  for (n = 0; n < nodenum; n++, node++) {
    if (node->VarGetAttr && node->VarSetVal) {
      // вычисляем кол-во загружаемых переменных
      for (k = i = 0; i < node->varNum; i++) {
        if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
            (prop & ENP_PROP_CONST)) {
          k++;
        }
      }
      if (k) {
        // ищем запись
        vars = &data->vars;
        while (vars->nodeId < 0xFFFF && vars->varNum < 0xFFFF &&
               vars->nodeId != node->id) {
          vars = (ENP_Vars_t *)((char *)vars +
                                ((vars->varNum << 2) + 8) / sizeof(char));
        }

        if (vars->nodeId == node->id && k == vars->varNum) {
          // загружаем переменные
          for (k = i = 0; i < node->varNum; i++) {
            if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
                (prop & ENP_PROP_CONST) &&
                node->VarSetVal(node->id, i, &vars->varVal[k++]) !=
                    ENP_ERROR_NONE) {
              res = 0;
            }
          }
        } else
          res = 0;
      }
    }
  }
  return res;
}

// Сохранение переменных конфигурации
uint8_t ENP_Save(ENP_Data_t *data, uint32_t update) {
  int n;

  if (!ENP_NodeList) {
    return 0;
  }
  // стираем flash
  if (!ENP_FlashErase || !ENP_FlashErase((uint32_t)data, (uint32_t)data)) {
    return 0;
  }
  // сохраняем параметры конфигурации
  if (!ENP_SavePars(data, update)) {
    return 0;
  }
  for (n = 0; n < ENP_NodeNum; n++) {
    if (!ENP_SaveNode(data, ENP_NodeList[n], 1)) {
      return 0;
    }
  }
  return 1;
}

// Загрузка переменных конфигурации
uint8_t ENP_Load(ENP_Data_t *data) {
  int n, res = 1;

  if (ENP_NodeList) {
    for (n = 0; n < ENP_NodeNum; n++) {
      if (!ENP_LoadNode(data, ENP_NodeList[n], 1)) {
        res = 0;
      }
    }
    return res;
  } else
    return 0;
}
