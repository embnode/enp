/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#include "enCANProt.h"
#include "string.h"
#ifdef ENP_FREERTOS
#include "cmsis_os.h"
#endif

#ifndef ENP_FREERTOS
// Положение выходных данных в буфере передачи
static int SMP_THead[SMP_CHANNUM] = {0, 0}; // голова
static int SMP_TTail[SMP_CHANNUM] = {0, 0}; // хвост

static smpMsg_t *SMP_TBuff[SMP_CHANNUM]; // передачи

static uint16_t SMP_TMask = 0;
#endif

// Буфер сообщений
static smpMsg_t *SMP_RBuff; // приёма

// Маски буферов
static uint16_t SMP_RMask = 0;

// Положение входных данных в буфере приёма
static int volatile SMP_RHead = 0; // голова
static int SMP_RTail = 0;          // хвост

// Буфер приёма протокола ENP
char ENP_RBuf[2][ENP_RBUF_SIZE];

// Положение входных данных в буфере приёма
static uint16_t ENP_RHead[2] = {0, 0}; // голова
static uint16_t ENP_RTail[2] = {0, 0}; // хвост

// Обработчик сообщений протокола ENP
static void ENP_MsgHandler(smpMsg_t *msg);

// Список обработчиков сообщений
static smpMsg_tHANDLER MsgHandler[SMP_MSGHANDLERSNUM] = {ENP_MsgHandler};

// Кол-во обработчиков сообщений
static uint16_t MsgHandlerNum = 1;

// Флаги ошибок портокола
uint32_t SMP_Error = 0;

// Функция получения времени
static uint32_t TimeDummy(void) { return 0; }
uint32_t (*SMP_Time)(void) = TimeDummy;

// Функция передачи сообщения
static uint8_t TxDummy(smpMsg_t *msg) { return 0; }
uint8_t (*SMP_Tx)(smpMsg_t *msg) = TxDummy;

// Функция приема сообщения
uint8_t SMP_Rx(smpMsg_t *msg) {
  int i;

  if (msg) {
    // определяем позицию нового символа в буфере
    i = (SMP_RHead + 1) & SMP_RMask;
    // переписываем данные в буфер
    if (i != SMP_RTail) {
      SMP_RHead = i;
      SMP_RBuff[i] = *msg;
      return 1;
    }
    // ошибка переполнения буфера
    SMP_Error |= SMP_ERROR_RXOVERFLOW;
  }
  return 0;
}

#ifdef ENP_FREERTOS
/*Инициализация портокола SMP*/
void SMP_Init(
    // буфер приема (1 << rmask)
    smpMsg_t *rbuff, uint16_t rmask,
    // функция передачи сообщения
    uint8_t (*txfunc)(smpMsg_t *msg),
    // функция получения времени
    uint32_t (*timefunc)(void)) {

  SMP_RBuff = rbuff;
  SMP_RMask = rmask;

  if (txfunc) {
    SMP_Tx = txfunc;
  }
  if (timefunc) {
    SMP_Time = timefunc;
  }
}
#else
// Инициализация портокола SMP
void SMP_Init(
    // буфер приема (1 << rmask)
    smpMsg_t *rbuff, uint16_t rmask,
    // буфера передачи (1 << tmask)
    smpMsg_t *tbuff1, smpMsg_t *tbuff2, uint16_t tmask,
    // функция передачи сообщения
    uint8_t (*txfunc)(smpMsg_t *msg),
    // функция получения времени
    uint32_t (*timefunc)(void)) {

  SMP_RBuff = rbuff;
  SMP_TBuff[SMP_CHAN1] = tbuff1;
  SMP_TBuff[SMP_CHAN2] = tbuff2;
  SMP_RMask = rmask;
  SMP_TMask = tmask;

  if (txfunc) {
    SMP_Tx = txfunc;
  }
  if (timefunc) {
    SMP_Time = timefunc;
  }
}
// Передача сообщения в очередь передачи
static uint8_t SMP_PutMessage(smpMsg_t msg) {
  const int _ch = msg.Attr.chan;

  if (SMP_TBuff[_ch]) {
    // определяем позицию нового символа в буфере передачи
    int i = (SMP_THead[_ch] + 1) & SMP_TMask;
    // если буфер переполнен, выставляем ошибку
    if (i != SMP_TTail[_ch]) {
      // добавляем сообщение в буфер передачи
      SMP_TBuff[_ch][i] = msg;
      SMP_THead[_ch] = i;
    } else
      SMP_Error |= SMP_ERROR_TXOVERFLOW;
  }

  return 0;
}
#endif

// Добавление обработчика сообщений
uint8_t SMP_AddHandler(smpMsg_tHANDLER func) {
  int i;

  if (func && MsgHandlerNum < SMP_MSGHANDLERSNUM) {
    // проверяем был ли добавлен обработчик ранее
    for (i = 0; i < MsgHandlerNum && MsgHandler[i] != func; i++) {
    }
    // добавляем обработчик
    if (i == MsgHandlerNum) {
      MsgHandler[MsgHandlerNum++] = func;
    }
    return 1;
  } else
    return 0;
}

// Обработка очереди сообщений
void SMP_Proc() {
  // обработка очереди полученых сообщений
  while (SMP_RHead != SMP_RTail) {
    SMP_RTail = (SMP_RTail + 1) & SMP_RMask;
    for (int i = 0; i < MsgHandlerNum; i++)
      MsgHandler[i](SMP_RBuff + SMP_RTail);
  }

#ifndef ENP_FREERTOS
  // отправка сообщений из очереди
  for (int ch = SMP_CHAN1; ch < SMP_CHANNUM; ch++) {
    if (SMP_TBuff[ch]) {
      while (SMP_TTail[ch] != SMP_THead[ch]) {
        int i = (SMP_TTail[ch] + 1) & SMP_TMask;
        if (SMP_Tx(SMP_TBuff[ch] + i)) {
          SMP_TTail[ch] = i;
        } else {
          break;
        }
      }
    }
  }
#endif
}

/* Формирование и добавление в очередь сообщения
    msgid - идентификатор сообщения
    *data - указатель на данные сообщения
    size - размер данных сообщения <= 8 */
uint8_t SMP_AddBroadcast(int msgid, const void * const data, int size) {

  int _res = 0;
  smpMsg_t _msg;
#ifdef ENP_FREERTOS
  if (size <= 8 && data) {
    // формируем сообщение
    memcpy(&_msg.data, data, size);
    _msg.Attr.id = msgid;
    _msg.Attr.size = size;
    _msg.Attr.chan = SMP_CHAN1;
    SMP_Tx(&_msg);
  }
#else
  if (size <= 8 && data) {
    // формируем сообщение
    memcpy(&_msg.data, data, size);
    _msg.Attr.id = msgid;
    _msg.Attr.size = size;
    // отправляем сообщение
    _msg.Attr.chan = SMP_CHAN1;
    if (SMP_PutMessage(_msg))
      _res = 1;
    _msg.Attr.chan = SMP_CHAN2;
    if (SMP_PutMessage(_msg))
      _res = 1;
  }
#endif

  return _res;
}

/*  Приём символа для протокола ENP
 - возвращает -1 если буфер приёма пуст, иначе принятый символ */
static int SMP_getc(int i) {
  if (ENP_RTail[i] != ENP_RHead[i]) {
    return ENP_RBuf[i][ENP_RTail[i] = (ENP_RTail[i] + 1) & ENP_RBUF_MASK];
  } else {
    return -1;
  }
}
int SMP_getc1() { return SMP_getc(0); }
int SMP_getc2() { return SMP_getc(1); }

/*  Передача строки из n символов для протокола ENP учетом места для ответа
     - возвращает 1 если строка помещена в буфер передачи, иначе 0 */
static int SMP_putn(const char *s, int n, int i) {
  static uint16_t pcntr[2] = {0, 0};
  static uint16_t id[2] = {SMP_SEND1, SMP_SEND2};

  for (; n > 0; n -= 8) {
    if (n >= 8) {
      SMP_AddBroadcast(id[i] + pcntr[i], (void *)s, 8);
    } else {
      SMP_AddBroadcast(id[i] + pcntr[i], (void *)s, n);
    }
    // меняем счетчик пакетов
    pcntr[i] = (pcntr[i] + 1) & SMP_MASK;
    s += 8;
  }
  return 1;
}
int SMP_putn1(const char *s, int n) { return SMP_putn(s, n, 0); }
int SMP_putn2(const char *s, int n) { return SMP_putn(s, n, 1); }

// Обработчик сообщений протокола ENP
static void ENP_MsgHandler(smpMsg_t *msg) {
  unsigned char *c = (void *)&msg->data;
  int i, j, n, msgid;
  static uint32_t ptime[2] = {0, 0};
  static uint16_t pcntr[2] = {0, 0};
  uint32_t time = SMP_Time();
  const uint32_t _id = msg->Attr.id;

  // определяем поток ENP
  if (_id >= SMP_RECV1 && _id <= SMP_RECV1 + SMP_MASK) {
    i = 0;
    msgid = SMP_RECV1;
  } else if (_id >= SMP_RECV2 && _id <= SMP_RECV2 + SMP_MASK) {
    i = 1;
    msgid = SMP_RECV2;
  } else {
    return;
  }

  // сбрасываем счетчик пакетов
  if (ptime[i] < time) {
    pcntr[i] = _id - msgid;
  }

  // проверяем на совпадение счетчика пакетов
  if (_id == msgid + pcntr[i]) {
    for (n = 0; n < msg->Attr.size; n++) {
      // определяем позицию нового символа в буфере передачи
      j = (ENP_RHead[i] + 1) & ENP_RBUF_MASK;
      // добавляем символ в буфер передачи, если там есть место
      if (j != ENP_RTail[i]) {
        ENP_RBuf[i][ENP_RHead[i] = j] = *c++;
      } else {
        SMP_Error |= SMP_ERROR_RXOVERFLOW;
        break;
      }
    }
    // меняем счетчик пакетов
    pcntr[i] = (pcntr[i] + 1) & SMP_MASK;
    ptime[i] = time + SMP_ENP_TIMEOUT;
  }
}
