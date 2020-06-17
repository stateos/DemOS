#include <stm32f4_discovery.h>
#include <os.h>

OS_TSK_DEF(cons)
{
	tsk_begin();

	LED_Tick();
	tsk_sleep();

	tsk_end();
}

OS_TSK_DEF(prod)
{
	tsk_begin();

	tsk_delay(SEC);
	tsk_restart(cons);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
