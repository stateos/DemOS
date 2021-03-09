#include <stm32f4_discovery.h>
#include <os.h>

OS_SIG(sig);

OS_TSK_DEF(cons)
{
	tsk_begin();

	sig_wait(sig, SIGSET(1));
	LED_Tick();
	sig_clear(sig, 1);

	tsk_end();
}

OS_TSK_DEF(prod)
{
	tsk_begin();

	tsk_delay(SEC);
	sig_give(sig, 1);

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
