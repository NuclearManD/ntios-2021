#include "debug/printf.h"

#include "avr/pgmspace.h"
#include <stdarg.h>
#include "imxrt.h"
 #include "usb_desc.h"

#ifdef PRINT_DEBUG_STUFF

void putchar_debug(char c);
static void puint_debug(unsigned int num);


FLASHMEM void printf_debug(const char *format, ...)
{
	va_list args;
	unsigned int val;
	int n;

	va_start(args, format);
	for (; *format != 0; format++) { // no-frills stand-alone printf
		if (*format == '%') {
			++format;
			if (*format == '%') goto out;
			if (*format == '-') format++; // ignore size
			while (*format >= '0' && *format <= '9') format++; // ignore size
			if (*format == 'l') format++; // ignore long
			if (*format == '\0') break;
			if (*format == 's') {
				printf_debug((char *)va_arg(args, int));
			} else if (*format == 'd') {
				n = va_arg(args, int);
				if (n < 0) {
					n = -n;
					putchar_debug('-');
				}
				puint_debug(n);
			} else if (*format == 'u') {
				puint_debug(va_arg(args, unsigned int));
			} else if (*format == 'x' || *format == 'X') {
				val = va_arg(args, unsigned int);
				for (n=0; n < 8; n++) {
					unsigned int d = (val >> 28) & 15;
					putchar_debug((d < 10) ? d + '0' : d - 10 + 'A');
					val <<= 4;
				}
			} else if (*format == 'c' ) {
				putchar_debug((char)va_arg(args, int));
			}
		} else {
			out:
			if (*format == '\n') putchar_debug('\r');
			putchar_debug(*format);
		}
	}
	va_end(args);
}

static void puint_debug(unsigned int num)
{
	char buf[12];
	unsigned int i = sizeof(buf)-2;

	buf[sizeof(buf)-1] = 0;
	while (1) {
		buf[i] = (num % 10) + '0';
		num /= 10;
		if (num == 0) break;
		i--;
	}
	printf_debug(buf + i);
}

// first is this normal Serial?
#if defined(PRINT_DEBUG_USING_USB) && defined(CDC_STATUS_INTERFACE) && defined(CDC_DATA_INTERFACE)
#include "usb_dev.h"
#include "usb_serial.h"
FLASHMEM void putchar_debug(char c)
{
	usb_serial_putchar(c);
}

#elif defined(PRINT_DEBUG_USING_USB) && defined(SEREMU_INTERFACE) && !defined(CDC_STATUS_INTERFACE) && !defined(CDC_DATA_INTERFACE)
#include "usb_dev.h"
#include "usb_seremu.h"
FLASHMEM void putchar_debug(char c)
{
	usb_seremu_putchar(c);
}

#else
FLASHMEM void putchar_debug(char c)
{
	while (!(LPUART3_STAT & LPUART_STAT_TDRE)) ; // wait
	LPUART3_DATA = c;
}

#endif




#endif // PRINT_DEBUG_STUFF
