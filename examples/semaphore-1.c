#include <stm32f4_discovery.h>
#include <os.h>

OS_SEM(sem, 0);

OS_TSK_START(cons)
{
	tsk_begin();

	sem_wait(sem);
	LED_Tick();

	tsk_end();
}

OS_TSK_START(prod)
{
	tsk_begin();

	tsk_delay(SEC);
	sem_give(sem);

	tsk_end();
}

int main()
{
	LED_Init();
	sys_start();
}
