/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "c_node_root.h"
#include "string.h"

using namespace NS_NODE_ROOT;

CNodeRoot::CNodeRoot(IcallbackRoot *callback) {
  _ptrCallback = callback;
  this->_error = 0;
}

ENP::retCode_t CNodeRoot::getVal(uint16_t varid, void *value) {
  ENP::retCode_t ret = ENP::ERROR_NONE;
  // Если указатель на void не нулевой и есть такая переменная то приводим
  // значение к соответствующему типу
  if ((varid < VAR_NUM) && (value != nullptr)) {
    switch (varid) {
    case VAR_ERROR:
      *static_cast<uint32_t *>(value) = _error;
      break;
    case VAR_TIME:
      *static_cast<uint32_t *>(value) = _ptrCallback->getTime();
      break;
    case VAR_FIRMWARE_VERSION:
      *static_cast<float *>(value) = _ptrCallback->getFirmwareVersion();
      break;
    case VAR_CHANGE_FIRMWARE:
      *static_cast<uint32_t *>(value) = _ptrCallback->isFirmwareChange();
      break;
    case VAR_DEV_ID:
      *static_cast<uint32_t *>(value) = _ptrCallback->getDevID();
      break;
    case VAR_SAVE:
      *static_cast<uint32_t *>(value) = 0;
      break;
    case VAR_REBOOT:
      *static_cast<uint32_t *>(value) = 0;
      break;
    default:
      ret = ENP::ERROR_VARID;
      break;
    }
  } else {
    ret = ENP::ERROR_VARID;
  }
  return ret;
}

ENP::retCode_t CNodeRoot::setVal(uint16_t varid, void *value) {
  ENP::retCode_t ret = ENP::ERROR_NONE;
  // Если указатель на void не нулевой и есть такая переменная то приводим
  // значение к соответствующему типу
  if ((varid < VAR_NUM) && (value != nullptr)) {
    switch (varid) {
    case VAR_ERROR:
      _error = *static_cast<uint32_t *>(value);
      break;
    case VAR_SAVE:
      if (*static_cast<uint32_t *>(value)) {
        _ptrCallback->save();
      }
      break;
    case VAR_CHANGE_FIRMWARE:
      _ptrCallback->changeFirmware(*static_cast<uint32_t *>(value) > 0);
      break;
    case VAR_DEV_ID:
      _ptrCallback->setDevID(*static_cast<uint32_t *>(value));
      break;
    case VAR_REBOOT:
      if (*static_cast<uint32_t *>(value)) {
        _ptrCallback->reboot();
      }
      break;
    default:
      ret = ENP::ERROR_VARID;
      break;
    }
  } else {
    ret = ENP::ERROR_VARID;
  }
  return ret;
}

ENP::retCode_t CNodeRoot::getAttr(uint16_t varid, char *name, uint16_t *prop) {
  ENP::retCode_t ret = ENP::ERROR_NONE;
  if ((varid < VAR_NUM) && (this->attr != nullptr)) {
    if (name != nullptr) {
      strcpy(name, this->attr[varid].name);
    }
    if (prop != nullptr) {
      *prop = this->attr[varid].prop;
    }
  } else {
    ret = ENP::ERROR_VARID;
  }
  return ret;
}
