#include "floppy.h"
#include "io.h"
#include "../../stdio.h"

enum FLPYDSK_IO {

	FLPYDSK_DOR		=	0x3f2,
	FLPYDSK_MSR		=	0x3f4,
	FLPYDSK_FIFO	=	0x3f5,
	FLPYDSK_CTRL	=	0x3f7
};

enum FLPYDSK_DOR_MASK {

	FLPYDSK_DOR_MASK_DRIVE0			=	0,	//00000000	= here for completeness sake
	FLPYDSK_DOR_MASK_DRIVE1			=	1,	//00000001
	FLPYDSK_DOR_MASK_DRIVE2			=	2,	//00000010
	FLPYDSK_DOR_MASK_DRIVE3			=	3,	//00000011
	FLPYDSK_DOR_MASK_RESET			=	4,	//00000100
	FLPYDSK_DOR_MASK_DMA			=	8,	//00001000
	FLPYDSK_DOR_MASK_DRIVE0_MOTOR	=	16,	//00010000
	FLPYDSK_DOR_MASK_DRIVE1_MOTOR	=	32,	//00100000
	FLPYDSK_DOR_MASK_DRIVE2_MOTOR	=	64,	//01000000
	FLPYDSK_DOR_MASK_DRIVE3_MOTOR	=	128	//10000000
};

static volatile uint8_t _fdIRQ = 0;


void fdInitializeDMA() {
    x86_outb(0x0A, 0x06);
    x86_outb(0xD8, 0xFF);
    x86_outb(0x04, 0);
	x86_outb(0x04, 0x10);
	x86_outb(0xd8, 0xff);  
	x86_outb(0x05, 0xff);  
	x86_outb(0x05, 0x23);
	x86_outb(0x80, 0);     
	x86_outb(0x0a, 0x02);   
}

void fdWriteDor(uint8_t cmd)
{
    x86_outb(FLPYDSK_DOR, cmd);
}


void fdWaitIRQ() {

	//! wait for irq to fire
	while ( _fdIRQ == 0)
		;
	_fdIRQ = 0;
}

void x86_fd_irq(Registers* regs) {
    _fdIRQ = 1;
}



void fdDisableController()
{
    fdWriteDor(0);
}

void fdEnableController()
{
    fdWriteDor( FLPYDSK_DOR_MASK_RESET | FLPYDSK_DOR_MASK_DMA);
}

void fdReset() {
    uint32_t st0, cyl;

    fdDisableController();
    fdEnableController();
    fdWaitIRQ();

}

void fdInstall() {
    x86_IRQ_RegisterHandler(6, x86_fd_irq);

    fdInitializeDMA();
    fdReset();


}