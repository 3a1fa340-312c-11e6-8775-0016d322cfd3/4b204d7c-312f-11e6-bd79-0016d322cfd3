void do_AbortHandler(){}

void do_BreakHandler(){}

void str8131_cpu_clock_scale_end(void);

void do_IRQ()
{
	str8131_cpu_clock_scale_end();	
}

void do_FIQ(){}

