/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef _NODE_ROOT_CPP_H
#define _NODE_ROOT_CPP_H
#include "eINode.h"
#include "enpapi.h"

namespace NS_NODE_ROOT {
enum {
  VAR_ERROR,            
  VAR_TIME,             
  VAR_FIRMWARE_VERSION, 
  VAR_CHANGE_FIRMWARE,  
  VAR_SAVE,             
  VAR_REBOOT,           
  VAR_NUM               
};
class IcallbackRoot {
public:
  virtual uint32_t getTime() = 0;
  virtual void save() = 0;
  virtual void reboot() = 0;
  virtual void changeFirmware(bool) = 0;
  virtual float getFirmwareVersion() = 0;
  virtual bool isFirmwareChange() = 0;
};
class CNodeRoot : public ENP::INode {
public:
  explicit CNodeRoot(IcallbackRoot *callback);
  virtual ENP::retCode_t getVal(uint16_t varid, void *value) override;
  virtual ENP::retCode_t setVal(uint16_t varid, void *value) override;
  virtual ENP::retCode_t getAttr(uint16_t varid, char *name,
                                 uint16_t *prop) override;

private:
  const ENP_Attr_t attr[VAR_NUM] = {
      {"Error code", ENP_TYPE_HEX | ENP_PROP_ERROR},
      {"Work time (hh:mm:ss)", ENP_TYPE_TIME | ENP_PROP_READONLY},
      {"firmware version", ENP_TYPE_REAL | ENP_PROP_READONLY},
      {"Change firmware", ENP_TYPE_BOOL},
      {"Save settings", ENP_TYPE_BOOL},
      {"Reboot", ENP_TYPE_BOOL},
  };
  uint32_t _error;
  IcallbackRoot *_ptrCallback;
};
} // namespace NS_NODE_ROOT

#endif
