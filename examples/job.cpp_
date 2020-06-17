#include <stm32f4_discovery.h>
#include <os.h>

OS_JOB(job);

OS_TSK_DEF(cons)
{
	tsk_begin();

	job_wait(job);

	tsk_end();
}

OS_TSK_DEF(prod)
{
	tsk_begin();

	tsk_delay(SEC);
	job_give(job, []{ LED_Tick(); });

	tsk_end();
}

int main()
{
	LED_Init();

	tsk_start(cons);
	tsk_start(prod);
	sys_start();
}
