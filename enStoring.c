#include "enStoring.h"

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
    0};

__weak uint8_t ENP_FlashWrite(void *dst, const void *ptr, int size) {
  return 0;
}

__weak uint8_t ENP_FlashErase(uint32_t startAddr, uint32_t stopAddr) {
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
  ENP_Pars.ckSum =
      Crc16(&ENP_Pars, sizeof(ENP_Pars) - sizeof(uint32_t), 0xFFFF);
  // сохраняем параметры конфигурации
  if (!ENP_FlashWrite(&ENP_Pars, &data->pars, sizeof(ENP_Pars))) {
    return 0;
  }
  return 1;
}

// Загрузка параметров конфигурации
uint8_t ENP_LoadPars(ENP_Data_t *data) {
  // проверяем контрольную сумму
  if (Crc16(&data->pars, sizeof(data->pars) - sizeof(uint32_t), 0xFFFF) ==
      data->pars.ckSum) {
    // загружаем параметры
    ENP_Pars = data->pars;
    return 1;
  }
  return 0;
}

// Сохранение переменных узла конфигурации
uint8_t ENP_SaveNode(ENP_Data_t *data, const ENP_Node_t *node, int nodenum) {
  // ENP_Vars_t *vars;
  // uint16_t prop, k;
  // uint32_t val;
  // int i, n;

  // // проверяем указатель на функцию записи
  // if (!ENP_FlashWrite) {
  //   return 0;
  // }
  // // ищем последнюю запись
  // vars = &data->vars;
  // while (vars->nodeId < 0xFFFF && vars->varNum < 0xFFFF) {
  //   vars =
  //       (ENP_Vars_t *)((char *)vars + ((vars->varNum << 2) + 8) /
  //       sizeof(char));
  // }
  // // для всех узлов
  // for (n = 0; n < nodenum; n++, node++) {
  //   if (node->VarGetAttr && node->VarGetVal) {
  //     // вычисляем кол-во сохраняемых переменных
  //     for (k = i = 0; i < node->varNum; i++)
  //       if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
  //           (prop & ENP_PROP_CONST))
  //         k++;
  //     if (k) {
  //       // сохраняем переменные
  //       for (k = i = 0; i < node->varNum; i++)
  //         if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
  //             (prop & ENP_PROP_CONST) &&
  //             node->VarGetVal(node->id, i, &val) == ENP_ERROR_NONE &&
  //             !ENP_FlashWrite(&val, &vars->varVal[k++], sizeof(uint32_t)))
  //           return 0;
  //       // сохраняем кол-во переменных
  //       if (!ENP_FlashWrite(&k, &vars->varNum, sizeof(node->varNum))) {
  //         return 0;
  //       }
  //       // вычисляем контрольную сумму
  //       val = Crc16(&vars->nodeId, (k << 2) + 4, 0xFFFF);
  //       // сохраняем контрольную сумму
  //       if (!ENP_FlashWrite(&val, &vars->ckSum, sizeof(val))) {
  //         return 0;
  //       }
  //       // переходим к следующей записи
  //       vars = (ENP_Vars_t *)((char *)vars + ((k << 2) + 8) / sizeof(char));
  //     }
  //   }
  // }
  return 1;
}

// Загрузка переменных узла конфигурации
uint8_t ENP_LoadNode(ENP_Data_t *data, const ENP_Node_t *node, int nodenum) {
  // ENP_Vars_t *vars;
  // uint16_t i, n, k;
  int res = 1;

  // for (n = 0; n < nodenum; n++, node++) {
  //   if (node->VarGetAttr && node->VarSetVal) {
  //     // вычисляем кол-во загружаемых переменных
  //     for (k = i = 0; i < node->varNum; i++) {
  //       // if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
  //       //     (prop & ENP_PROP_CONST)) {
  //       //   k++;
  //       // }
  //     }
  //     if (k) {
  //       // ищем запись
  //       vars = &data->vars;
  //       while (vars->nodeId < 0xFFFF && vars->varNum < 0xFFFF &&
  //              vars->nodeId != node->id) {
  //         vars = (ENP_Vars_t *)((char *)vars +
  //                               ((vars->varNum << 2) + 8) / sizeof(char));
  //       }

  //       if (vars->nodeId == node->id && k == vars->varNum) {
  //         // загружаем переменные
  //         // for (k = i = 0; i < node->varNum; i++) {
  //         //   if (node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE
  //         &&
  //         //       (prop & ENP_PROP_CONST) &&
  //         //       node->VarSetVal(node->id, i, &vars->varVal[k++]) !=
  //         //           ENP_ERROR_NONE) {
  //         //     res = 0;
  //         //   }
  //         // }
  //       } else
  //         res = 0;
  //     }
  //   }
  // }
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