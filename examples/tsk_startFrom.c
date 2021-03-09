#include <stm32f4_discovery.h>
#include <os.h>

void consumer()
{
	tsk_begin();

	LED_Tick();
	tsk_stop();

	tsk_end();
}

OS_TSK_DEF(prod)
{
	static tsk_t cons = TSK_INIT(NULL);

	tsk_begin();

	for (;;)
	{
		tsk_sleepNext(SEC);
		tsk_startFrom(&cons, consumer);
		tsk_join(&cons);
	}

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(prod);
	sys_start();
}
