#include <stm32f4_discovery.h>
#include <os.h>

OS_ONE(one);

void leds_off( void )
{
	LEDs = 0;
}

OS_TSK_DEF(tsk)
{
	tsk_begin();

	for (;;)
	{
		one_call(one, leds_off);
		tsk_delay(SEC);
		LED_Tick();
	}

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(tsk);
	sys_start();
}
