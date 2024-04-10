#pragma once
#ifdef SYZ_USE_IPT
extern "C" {
#include <libxdc.h>
}
#endif	// SYZ_USE_IPT

#if GOOS_linux
#include <linux/perf_event.h>
#include <sched.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/time.h>


#define SYZIPT_MMAP_PAGES 512 * 1024
#define SYZIPT_MMAP_BASE 0xffffffff80000000
#define FILTERS                                       \
	{                                             \
		{0x1000, UINT64_MAX}, {0, 0}, {0, 0}, \
		{                                     \
			0, 0                          \
		}                                     \
	}

typedef void* (*fetch_memory_page_t)(uint64_t, uint8_t*);

void* fetch_memory_page(uint64_t addr, uint8_t* driver_buf);
void* page_fetch_callback(void* memory_reader_func, uint64_t addr, bool* ret);

typedef struct
{
	pid_t pid;
	char* args[16];
	int perfFd;
	uint8_t* perfMmapBuf;
	uint8_t* perfMmapAux;
} run_t;

#define perfIntelPtPerfType 8
#define _PERF_EVENT_SIZE (1024 * 512)
#define _PERF_AUX_SIZE (512 * 1024)

/* Memory barriers */
#define rmb() __asm__ __volatile__("" :: \
				       : "memory")
#define wmb() __sync_synchronize()

/* Atomics */
#define ATOMIC_GET(x) __atomic_load_n(&(x), __ATOMIC_RELAXED)
#define ATOMIC_SET(x, y) __atomic_store_n(&(x), y, __ATOMIC_RELAXED)
#define ATOMIC_CLEAR(x) __atomic_store_n(&(x), 0, __ATOMIC_RELAXED)
#define ATOMIC_XCHG(x, y) __atomic_exchange_n(&(x), y, __ATOMIC_RELAXED)

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif

#define RTIT_CTL_TRACEEN BIT(0)
#define RTIT_CTL_CYCLEACC BIT(1)
#define RTIT_CTL_OS BIT(2)
#define RTIT_CTL_USR BIT(3)
#define RTIT_CTL_PWR_EVT_EN BIT(4)
#define RTIT_CTL_FUP_ON_PTW BIT(5)
#define RTIT_CTL_CR3EN BIT(7)
#define RTIT_CTL_TOPA BIT(8)
#define RTIT_CTL_MTC_EN BIT(9)
#define RTIT_CTL_TSC_EN BIT(10)
#define RTIT_CTL_DISRETC BIT(11)
#define RTIT_CTL_PTW_EN BIT(12)
#define RTIT_CTL_BRANCH_EN BIT(13)
#define RTIT_CTL_MTC_RANGE_OFFSET 14
#define rmb() __asm__ __volatile__("" :: \
				       : "memory")

static long perf_event_open(
    struct perf_event_attr* hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags);

static perf_event_attr pe = {
    .type = perfIntelPtPerfType, // used
    .size = sizeof(perf_event_attr), // used
    .config = RTIT_CTL_DISRETC, // used
    {.sample_period = 0ull},
    .sample_type = 0ull,
    .read_format = 0ull,
    .disabled = true, // used
    .inherit = false,
    .pinned = false,
    .exclusive = false,
    .exclude_user = true, // used
    .exclude_kernel = false, // used
    .exclude_hv = true, // used
    .exclude_idle = true, // used
    .mmap = false,
    .comm = false,
    .freq = false,
    .inherit_stat = false,
    .enable_on_exec = false,
};
#endif		// GOOS_LINUX

#if GOOS_windows
	// TODO
#endif		// GOOS_WINDOWS

libxdc_config_t libxdc_cfg = {
    .filter = FILTERS,
    .page_cache_fetch_fptr = &page_fetch_callback,
    .page_cache_fetch_opaque = (void*)fetch_memory_page,
    .bitmap_ptr = NULL,
    .bitmap_size = 0x10000,
    .cov_ptr = NULL,
    .cov_size = 0x100000,
    .align_psb = false,
};

struct ipt_driver_t {
	int driver_fd;
	uint8_t* memory_buf;
};

struct ipt_decoder_t {
	libxdc_t* libxdc;
	uint32_t* cov_data;
	uint8_t* trace_input;

	void init()
	{
		// mmap signal
		cov_data = (uint32_t*)mmap(NULL, libxdc_cfg.cov_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		libxdc_cfg.cov_ptr = cov_data;
		libxdc = libxdc_init(&libxdc_cfg);
		if (libxdc == NULL) {
			failmsg("libxdc init failed", "cov data: %p", cov_data);
		}
		trace_input = (uint8_t*)mmap(NULL, _PERF_AUX_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	}

	void decode(uint64_t size)
	{
		printf("Decoding trace..., size: 0x%lx\n", size);
		struct timeval start, end;
		gettimeofday(&start, NULL);
		decoder_result_t res = libxdc_decode(libxdc, trace_input, size);
		gettimeofday(&end, NULL);
		unsigned long time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
		if (res == decoder_success) {
			printf("Decode success! Time taken in usec: %lu\n", time);
		} else {
			printf("Decode failed with result: %d\n", res);
		}
	}

	inline uint32_t get_cov(uint32_t index)
	{
		return cov_data[index + 1];
	}

	void reset_signal()
	{
		*(uint32_t*)cov_data = 0;
	}

	uint32_t get_cov_count()
	{
		return *(uint32_t*)cov_data;
	}
};

ipt_driver_t ipt_driver;

void* fetch_memory_page(uint64_t addr, uint8_t* driver_buf)
{
	return (void*)(driver_buf + (addr - SYZIPT_MMAP_BASE));
}

void* page_fetch_callback(void* memory_reader_func, uint64_t addr, bool* ret)
{
	addr &= 0xfffffffffffff000;
	// printf("Info: page base addr 0x%lx\n", addr);
	fetch_memory_page_t fetch_memory_page_func = (fetch_memory_page_t)memory_reader_func;
	*ret = true;
	// TODO: return false if failed
	return fetch_memory_page_func(addr, ipt_driver.memory_buf);
}