
#include <stdint.h>

extern volatile uint32_t systick_millis_count;

uint32_t millis(void)
{
	return systick_millis_count;
}
