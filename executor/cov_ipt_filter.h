static uint32_t COV_IPT_FILTER[][2] = {
	{0x810FF640, 0x81117D20},		// optprobe_template_entry - vsmp_irq_enable
	{0x81766750, 0x817749E0},		// kasan, compiler
	{0x81405C40, 0x81406190},		// kcov
	{0x8159AC50, 0x815DB890},		// perf
	{0x8121DDD0, 0x812BD470},		// sched
	{0x816060D0, 0x8161D960},		// memory allocation
	{0x82B64820, 0x82B6EEB0},		// __schedule
	{0x81DC5C80, 0x81DC5EA0},		// list_debug
	{0x81E29160, 0x81E29C20},		// stack_depot
};

static const uint32_t COV_IPT_END = 0x81007414, COV_IPT_START = 0x810082dd;		// end of trace

// @return: true if the addr is filtered out
bool coverage_filter_ipt(uint32_t addr) {
	for (unsigned int i = 0; i < sizeof(COV_IPT_FILTER) / sizeof(COV_IPT_FILTER[0]); i++) {
		if (addr >= COV_IPT_FILTER[i][0] && addr < COV_IPT_FILTER[i][1]) {
			return true;
		}
	}
	return false;
}
