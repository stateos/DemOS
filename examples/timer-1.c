#include <stm32f4_discovery.h>
#include <os.h>

void consumer( void );
void producer( void );

OS_TMR(tmr);
OS_TSK(tsk, NULL);

void consumer()
{
	tsk_begin();

	LED_Tick();
	tsk_flip(producer);

	tsk_end();
}

void producer()
{
	tsk_begin();

	tmr_waitFor(tmr, SEC);
	tsk_flip(consumer);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_startFrom(tsk, producer);
	sys_start();
}
