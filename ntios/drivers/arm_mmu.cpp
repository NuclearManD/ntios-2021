
#include "../ntios.h"
#include "../mmu.h"

// None of this works yet.  Cannot test it yet either.  Just saving it here for now.
// Was here originally bc I thought the T4.1 had an MMU.  It doesn't.

/*
#if CPU_ARCH == ARMv7

// From https://github.com/dwelch67/raspberrypi/blob/cbb3a102d83dfeb4586e503749111d1bfbf8fc88/twain/vectors.s
extern "C" void start_mmu (void*, unsigned int);
__asm__(
".globl start_mmu\n"
"start_mmu:\n"
	"mov r2,#0\n"
	"mcr p15,0,r2,c7,c7,0 ;@ invalidate caches\n"
	"mcr p15,0,r2,c8,c7,0 ;@ invalidate tlb\n"

	"mvn r2,#0\n"
	"mcr p15,0,r2,c3,c0,0 ;@ domain\n"

	"mcr p15,0,r0,c2,c0,0 ;@ tlb base\n"
	"mcr p15,0,r0,c2,c0,1 ;@ tlb base\n"

	"mrc p15,0,r2,c1,c0,0\n"
	"orr r2,r2,r1\n"
	"mcr p15,0,r2,c1,c0,0\n"
	"bx lr\n"
);

uint32_t _arm_get_mmu_capabilities() {
	uint32_t retval;
	__asm__(
		"mrc p15, 0, r2, c0, c1, 4\n"
		"mov %[retval], r2\n"
		:[retval]"=r"(retval)
		:
		:"r2"
	);
	return retval;
}


bool _ntios_is_mmu_active = false;

uint32_t _1mb_page_table[4096];


void _arm_mmu_create_1mb_entry(uint8_t domain, uint8_t perms, uint16_t vmem_mb, uint16_t pmem_mb) {
	uint32_t entry = 0;
	
	entry |= pmem_mb << 20;
	entry |= perms << 10;
	entry |= domain << 5;
	entry |= 2;

	_1mb_page_table[vmem_mb] = entry;
}

void mmu_section ( unsigned int vadd, unsigned int padd, unsigned int flags )
{
    unsigned int ra;
    unsigned int rc;

    ra=vadd>>20;
    rc=(padd&0xFFF00000)|0xC00|flags|2;
    //hexstrings(rb); hexstring(rc);
    _1mb_page_table[ra] = rc;
}



bool ntios_has_mmu() {
	return true;
}

bool ntios_is_using_mmu() {
	return _ntios_is_mmu_active;
}

bool ntios_activate_mmu() {
	start_mmu((void*)_1mb_page_table,0x00000001|0x1000|0x0004);
	_ntios_is_mmu_active = true;
	return true;
}

#include <Arduino.h>

void ntios_setup_mmu() {
	//asm volatile("mov r0, #0x3\n"
	//             "mcr p15, 0, r0, c3, c0, 0\n");
	/*for (int i = 0; i < 10; i++)
		_arm_mmu_create_1mb_entry(0, 3, i, i);*/
	/*for(uint32_t ra=0;;ra+=0x00100000)
	{
		mmu_section(ra,ra,0x0000);
		if(ra==0xFFF00000) break;
	}
	mmu_section(0x20000000,0x20000000,0x0000); //NOT CACHED!
	mmu_section(0x20200000,0x20200000,0x0000); //NOT CACHED!
	
	Serial.printf("mmu: %08x\n", _arm_get_mmu_capabilities());
}

#endif
*/
