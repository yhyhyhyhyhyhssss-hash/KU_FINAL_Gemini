
#ifndef __RW_UART_H
#define __RW_UART_H

/* Enable the printf() format string checking of gcc: http://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Function-Attributes.html */
#ifndef PRINTF_STYLE
#if defined( __GNUC__ )
#define PRINTF_STYLE(Fmt, FirstArg) __attribute__ ((format (printf, (Fmt), (FirstArg))))
#else
#define PRINTF_STYLE(Fmt, FirstArg)
#endif /* !__GNUC__ */
#endif /* PRINTF_STYLE */


#include "Var_Config.h"

// void SendDetHeadHj(float hj);
// void SendDetHJThreshold(float hjerror);
// void SendDetHead(Type_INSFix Fix);
// void INSAutoFixZero(void);



// void TransAntPannelStatusDatas(void);

void debug_error(const char *format, ...);

#endif /* __RW_UART_H */
