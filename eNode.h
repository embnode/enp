/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef ENPDEFS_H_
#define ENPDEFS_H_
#include "enpHelper.h"
#include "stdint.h"

#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */

#define ENP_MAJOR_VER 1
#define ENP_MINOR_VER 1
#define ENP_SUB_VER 0

// Configuarion error codes
#define ENP_ERROR_NONE 0
#define ENP_ERROR_NODENUM 1 // node number error
#define ENP_ERROR_NODEID 2 // node id error
#define ENP_ERROR_VARID 3 // variable id error
#define ENP_ERROR_VARVAL 4 // veriable value error
#define ENP_ERROR_COMMAND 5 // wrong command

// Types of variables
#define ECP_TYPE_INT 0x0001   // int32
#define ENP_TYPE_UINT 0x0002  // uint32
#define ENP_TYPE_BOOL 0x0003  // logical
#define ENP_TYPE_REAL 0x0004  // float32
#define ENP_TYPE_HEX 0x0005   // uint32 in hex
#define ENP_TYPE_CHAR4 0x0006 // 4 cars
#define ENP_TYPE_TIME 0x0007  // time
#define ENP_TYPE_MASK 0x000F  // mask for type

// Properties of variables
#define ENP_PROP_READONLY 0x0010 // read only
#define ENP_PROP_CONST 0x0020 // stiring variable
#define ENP_PROP_ERROR 0x0040 // Error code
#define ENP_PROP_MASK 0x00F0  // mask for properties

// attributes of the variable
typedef struct {
  const char *name; // the variable name
  uint16_t prop;    // the variable property
} ENP_Attr_t;

// The node
typedef struct {
  const char *name;          // node name
  uint16_t id;               // node id
  uint16_t pid;              // parrent node is
  uint16_t varNum;           // number of variables
  const ENP_Attr_t *varAttr; // attributes of variables
  // read variable
  uint16_t (*VarGetVal)(uint16_t nodeid, uint16_t varid, void *value);
  // write variable
  uint16_t (*VarSetVal)(uint16_t nodeid, uint16_t varid, void *value);
} ENP_Node_t;

// nodes list
extern const ENP_Node_t **ENP_NodeList;
// Max number of nodes
extern uint16_t ENP_MaxNodeNum;
// Current nodes number
extern uint16_t ENP_NodeNum;

// Intialization nodes
void ENP_NodeListInit(const ENP_Node_t **nodelist, int maxnodenum);
// Find node by id
const ENP_Node_t *ENP_FindNode(uint16_t nodeid);
// Insert nodes in the list
uint16_t ENP_InsertNodes(const ENP_Node_t *node, uint16_t num);
// Delete nodes from the list
uint16_t ENP_DeleteNodes(const ENP_Node_t *node, uint16_t num);

#endif
