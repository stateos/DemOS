#include <stm32f4_discovery.h>
#include <os.h>

OS_MTX(mtx);

OS_TSK_DEF(cons)
{
	tsk_begin();
	
	mtx_wait(mtx);
	LED_Tick();
	mtx_give(mtx);

	tsk_end();
}

OS_TSK_DEF(prod)
{
	tsk_begin();
	
	mtx_wait(mtx);
	tsk_delay(SEC);
	mtx_give(mtx);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
