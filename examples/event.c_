#include <stm32f4_discovery.h>
#include <os.h>

OS_EVT(evt);

OS_TSK_DEF(cons)
{
	tsk_begin();

	evt_wait(evt);
	LEDs = evt_take(evt) & 0x0FUL;

	tsk_end();
}

OS_TSK_DEF(prod)
{
	static unsigned x = 1;

	tsk_begin();

	tsk_delay(SEC);
	evt_give(evt, x);
	x = (x << 1) | (x >> 3);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
