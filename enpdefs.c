/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enpapi.h"
#include "stddef.h"

// nodes list
const ENP_Node_t **ENP_NodeList = NULL;
// Max number of nodes
uint16_t ENP_MaxNodeNum = 0;
// Количество узлов конфигурации
uint16_t ENP_NodeNum = 0;

// Intialization nodes
void ENP_NodeListInit(const ENP_Node_t **nodelist, int maxnodenum) {
  ENP_NodeList = nodelist;
  ENP_MaxNodeNum = maxnodenum;
}

// Insert node
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

// Delete node
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

// Проверка конфигурации на наличие ошибок
uint8_t ENP_Error() {
  // const ENP_Node_t *node;
  // uint16_t i, n, prop;
  // uint32_t value;

  // if (ENP_NodeList) {
  //   for (n = 0; n < ENP_NodeNum; n++) {
  //     node = ENP_NodeList[n];
  //     for (i = 0; i < node->varNum; i++) {
  //       if (node->VarGetAttr &&
  //           node->VarGetAttr(node->id, i, 0, &prop) == ENP_ERROR_NONE &&
  //           (prop & ENP_PROP_ERROR) && node->VarGetVal &&
  //           node->VarGetVal(node->id, i, &value) == ENP_ERROR_NONE && value)
  //           {
  //         return 1;
  //       }
  //     }
  //   }
  // }
  return 0;
}
