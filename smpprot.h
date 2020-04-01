/****************************************************************************
 *                      Embedded node protocol(ENP)                         *
 ****************************************************************************/
#ifndef SMP_H_
#define SMP_H_
#include "enpapi.h"
#include "stdint.h"

extern uint32_t SMP_Error;

// Флаги ошибок портокола
#define SMP_ERROR_NOERROR 0x00000000 // нет ошибок
#define SMP_ERROR_RXOVERFLOW 0x10000000 // переполнение приемного буфера
#define SMP_ERROR_TXOVERFLOW 0x20000000 // переполнение передающего буфера

// Cообщения связи с пунктом контроля
#define SMP_RECV1 1808 // данные принятые с канала 1
#define SMP_SEND1 1824 // данные для передачи по каналу 1
#define SMP_RECV2 1840 // данные принятые с канала 2
#define SMP_SEND2 1856 // данные для передачи по каналу 2

// Маска счетчика пакетов канала передачи данных
#define SMP_MASK 0x0F
// Таймаут сообщений (мс)
#define SMP_ENP_TIMEOUT 1000

// Идентификаторы каналов
enum { SMP_CHAN1, SMP_CHAN2, SMP_CHANNUM };

// Сообщение
typedef struct {
  uint8_t data[8]; // данные
  // аттрибуты
  struct {
    unsigned id : 11;  // идентификатор
    unsigned size : 4; // размер
    unsigned chan : 1; // канал
  } Attr;
} smpMsg_t;

// Размер буфера приема протокола ENP
#define ENP_RBUF_SIZE 128

// Маски индексов
#define ENP_RBUF_MASK (ENP_RBUF_SIZE - 1)
#if (ENP_RBUF_SIZE & ENP_RBUF_MASK)
#error ENP_RBUF_SIZE is not a power of 2
#endif

// Обработчик сообщения протокола
typedef void (*smpMsg_tHANDLER)(smpMsg_t *msg);

// Максимальное кол-во обработчиков сообщений
#define SMP_MSGHANDLERSNUM 20

// Инициализация портокола
#ifdef ENP_FREERTOS
void SMP_Init(smpMsg_t *, uint16_t,
                     uint8_t (*)(smpMsg_t *msg), uint32_t (*)(void));
#else
void SMP_Init(smpMsg_t *, uint16_t, smpMsg_t *, smpMsg_t *, uint16_t,
                     uint8_t (*)(smpMsg_t *msg), uint32_t (*)(void));
#endif


// Процедура протокола
void SMP_Proc(void);
void SMP_TxProc(void);

// Функция получения времени
extern uint32_t (*SMP_Time)(void);

// Функция передачи сообщения
extern uint8_t (*SMP_Tx)(smpMsg_t *msg);

// Функция приема сообщения
uint8_t SMP_Rx(smpMsg_t *msg);

// Добавление обработчика сообщений
uint8_t SMP_AddHandler(smpMsg_tHANDLER func);

// Формирование и добавление в очередь широковещательного сообщения
uint8_t SMP_AddBroadcast(int, const void * const, int);

// Приём символа для протокола ENP
int SMP_getc1(void);
int SMP_getc2(void);

// Передача строки из n символов для протокола ENP
int SMP_putn1(const char *s, int n);
int SMP_putn2(const char *s, int n);

#endif
