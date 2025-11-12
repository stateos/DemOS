#include <stm32f4_discovery.h>
#include <os.h>

OS_TSK_START(cons)
{
	tsk_begin();

	tsk_sleepFor(50*MSEC);
	LED_Tick();

	tsk_end();
}

OS_TSK_START(prod)
{
	tsk_begin();

	tsk_sleepNext(SEC);
	sys_suspend();
	tsk_sleepNext(SEC);
	sys_resume();

	tsk_end();
}

int main()
{
	LED_Init();
	sys_start();
}
