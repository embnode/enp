/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef _NODE_ROOT_CPP_H
#define _NODE_ROOT_CPP_H
#include "enp_node.h"
#include "enpapi.h"

namespace NS_NODE_ROOT {
enum {
  VAR_ERROR,             // код ошибки
  VAR_TIME,              // время работы
  VAR_FIRMWARE_VERSION,  // версия ПО
  VAR_DEV_ID,            // номер устройства
  VAR_CHANGE_FIRMWARE,   // сменить пррошивку
  VAR_SAVE,              // сохранить настройки
  VAR_REBOOT,            // перезагрузка
  VAR_NUM                // количество переменных
};
class IcallbackRoot {
 public:
  virtual uint32_t getTime() = 0;
  virtual void save() = 0;
  virtual void reboot() = 0;
  virtual void changeFirmware(bool) = 0;
  virtual float getFirmwareVersion() = 0;
  virtual bool isFirmwareChange() = 0;
  virtual uint16_t getDevID() = 0;
  virtual void setDevID(uint16_t) = 0;
};
class CNodeRoot : public ENP::INode {
 public:
  explicit CNodeRoot(IcallbackRoot* callback);
  virtual ENP::retCode_t getVal(uint16_t, void*) override;
  virtual ENP::retCode_t setVal(uint16_t, void*) override;
  virtual ENP::retCode_t getAttr(uint16_t, char*, uint16_t*) override;

 private:
  const ENP_Attr_t attr[VAR_NUM] = {
      {"Error code", ENP_TYPE_HEX | ENP_PROP_ERROR},
      {"Work time (hh:mm:ss)", ENP_TYPE_TIME | ENP_PROP_READONLY},
      {"Firmware version", ENP_TYPE_REAL | ENP_PROP_READONLY},
      {"DevID", ENP_TYPE_UINT},
      {"Change firmware", ENP_TYPE_BOOL},
      {"Save settings", ENP_TYPE_BOOL},
      {"Reboot", ENP_TYPE_BOOL},
  };
  uint32_t _error;
  IcallbackRoot* _ptrCallback;
};
}  // namespace NS_NODE_ROOT

#endif
