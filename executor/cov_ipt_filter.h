static uint32_t COV_IPT_FILTER[][2] = {
	{0x81113760, 0x81116CA0},		// arch/x86/kernel/unwind_orc.c, stacktrace
	{0x81771C90, 0x81774260},		// kasan, compiler
	{0x81405C40, 0x81406190},		// kcov
};

// @return: true if the addr is filtered out
bool coverage_filter_ipt(uint32_t addr) {
	for (unsigned int i = 0; i < sizeof(COV_IPT_FILTER) / sizeof(COV_IPT_FILTER[0]); i++) {
		if (addr >= COV_IPT_FILTER[i][0] && addr < COV_IPT_FILTER[i][1]) {
			return true;
		}
	}
	return false;
}
