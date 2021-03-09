#include <stm32f4_discovery.h>
#include <os.h>

void consumer( void );
void producer( void );

tsk_t cons = TSK_INIT(consumer);
tsk_t prod = TSK_INIT(producer);
tmr_t tmr  = 0;

void consumer( void )
{
	tsk_begin();

	tsk_suspend(tsk_this());
	LED_Tick();

	tsk_end();
}

void producer( void )
{
	tsk_begin();

	tmr_waitFor(&tmr, SEC);
	tsk_resume(&cons);

	tsk_end();
}

int main( void )
{
	LED_Init();

	tsk_start(&cons);
	tsk_start(&prod);
	sys_start();
}
