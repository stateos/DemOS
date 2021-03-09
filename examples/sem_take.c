#include <stm32f4_discovery.h>
#include <os.h>

OS_SEM(sem, 0);

OS_TSK_DEF(cons)
{
	tsk_begin();

	if (sem_take(sem))
		LED_Tick();

	tsk_end();
}

OS_TSK_DEF(prod)
{
	tsk_begin();

	tsk_delay(SEC);
	sem_give(sem);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
