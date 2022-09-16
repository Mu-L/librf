
#include "librf/librf.h"
#include <iostream>

extern void resumable_main_yield_return();
extern void resumable_main_timer();
extern void resumable_main_suspend_always();
extern void resumable_main_sleep();
extern void resumable_main_routine();
extern void resumable_main_resumable();
extern void resumable_main_mutex();
extern void resumable_main_exception(bool bomb);
extern void resumable_main_event();
extern void resumable_main_event_v2();
extern void resumable_main_event_timeout();
extern void resumable_main_dynamic_go();
extern void resumable_main_channel();
extern void resumable_main_cb();
extern void resumable_main_modern_cb();
extern void resumable_main_multi_thread();
extern void resumable_main_channel_mult_thread();
extern void resumable_main_when_all();
extern void resumable_main_layout();
extern void resumable_main_switch_scheduler();
extern void resumable_main_stop_token();

extern void resumable_main_benchmark_mem(bool wait_key);
extern void benchmark_main_channel_passing_next();
extern void resumable_main_benchmark_asio_server();
extern void resumable_main_benchmark_asio_client(intptr_t nNum);

extern void test_async_cinatra_client();

int main(int argc, const char* argv[])
{
	(void)argc;
	(void)argv;

	//resumable_main_resumable();
	//return 0;

	//if (argc > 1)
	//	resumable_main_benchmark_asio_client(atoi(argv[1]));
	//else
	//	resumable_main_benchmark_asio_server();

	resumable_main_cb();
	resumable_main_layout();
	resumable_main_modern_cb();
	resumable_main_suspend_always();
	resumable_main_yield_return();
	resumable_main_resumable();
	resumable_main_routine();
#ifndef __clang__
	resumable_main_exception(false);
#endif
	resumable_main_dynamic_go();
	resumable_main_multi_thread();
	resumable_main_timer();
	resumable_main_benchmark_mem(false);
	resumable_main_mutex();
	resumable_main_event();
	resumable_main_event_v2();
	resumable_main_event_timeout();
	resumable_main_channel();
	resumable_main_channel_mult_thread();
	resumable_main_sleep();
	resumable_main_when_all();
	resumable_main_switch_scheduler();
	resumable_main_stop_token();
	std::cout << "ALL OK!" << std::endl;

	benchmark_main_channel_passing_next();

	return 0;
}
