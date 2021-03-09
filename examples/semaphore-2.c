#include <stm32f4_discovery.h>
#include <os.h>

sem_t sem = 0;

OS_TSK_DEF(dly)
{
	tsk_begin();

	sem_wait(&sem);
	tsk_stop();

	tsk_end();
}

OS_TSK_DEF(cons)
{
	tsk_begin();

	tsk_call(dly);
	LED_Tick();

	tsk_end();
}

OS_TSK_DEF(prod)
{
	tsk_begin();

	tsk_delay(SEC);
	sem_give(&sem);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
