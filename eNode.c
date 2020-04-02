/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enpHelper.h"
#include "stddef.h"

// nodes list
const ENP_Node_t **ENP_NodeList = NULL;
// Max number of nodes
uint16_t ENP_MaxNodeNum = 0;
// Current nodes number
uint16_t ENP_NodeNum = 0;

// Intialization nodes
void ENP_NodeListInit(const ENP_Node_t **nodelist, int maxnodenum) {
  ENP_NodeList = nodelist;
  ENP_MaxNodeNum = maxnodenum;
}

// Insert nodes
uint16_t ENP_InsertNodes(const ENP_Node_t *node, uint16_t num) {
  int count = 0;

  if (node && ENP_NodeList) {
    for (int i = 0; i < num; i++) {
      if (ENP_NodeNum < ENP_MaxNodeNum) {
        // Add pointer to node in the list
        ENP_NodeList[ENP_NodeNum] = node;
        ENP_NodeNum++;
        node++;
        count++;
      } else {
        break;
      }
    }
  }
  return count;
}

// Delete node
uint16_t ENP_DeleteNode(const ENP_Node_t *node, uint16_t num) {
  int count = 0;

  if (node && ENP_NodeList) {
    for (int i = 0; i < num; i++) {
      // looking for node in the list
      for (int j = 0; j < ENP_NodeNum; j++) {
        if (ENP_NodeList[j] == node) {
          // shift the list
          for (int d = j; d < ENP_NodeNum - 1; d++) {
            ENP_NodeList[d] = ENP_NodeList[d + 1];
          }
          ENP_NodeNum--;
          count++;
        }
      }
    }
  }
  return count;
}

// find node. If node doesn't find it will return NULL
const ENP_Node_t *ENP_FindNode(uint16_t nodeid) {
  const ENP_Node_t *ret = NULL;

  if (ENP_NodeList) {
    for (int i = 0; i < ENP_NodeNum; i++) {
      if (ENP_NodeList[i]->id == nodeid) {
        ret = ENP_NodeList[i];
        break;
      }
    }
  }

  return ret;
}
