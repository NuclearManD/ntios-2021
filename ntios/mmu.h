
#include <stdbool.h>
#include <stdint.h>

#if CPU_ARCH == ARMv7
	//#define NTIOS_HAS_MMU
#endif

#ifdef NTIOS_HAS_MMU

bool ntios_has_mmu();
bool ntios_is_using_mmu();
bool ntios_activate_mmu();
void ntios_setup_mmu();

#endif
