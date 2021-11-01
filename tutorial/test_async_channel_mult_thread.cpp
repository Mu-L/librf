﻿//验证channel是否线程安全

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <deque>
#include <mutex>

#include "librf/librf.h"

using namespace librf;

using namespace std::chrono;
static std::mutex cout_mutex;
std::atomic<intptr_t> gcounter = 0;

#define OUTPUT_DEBUG	0

future_t<> test_channel_consumer(channel_t<std::string> c, size_t cnt)
{
	for (size_t i = 0; i < cnt; ++i)
	{
		try
		{
			auto val = co_await c.read();
			++gcounter;
#if OUTPUT_DEBUG
			{
				scoped_lock<std::mutex> __lock(cout_mutex);
				std::cout << "R " << val << "@" << std::this_thread::get_id() << std::endl;
			}
#endif
		}
		catch (channel_exception& e)
		{
			//MAX_CHANNEL_QUEUE=0,并且先读后写，会触发read_before_write异常
			scoped_lock<std::mutex> __lock(cout_mutex);
			std::cout << e.what() << std::endl;
		}

#if OUTPUT_DEBUG
		co_await sleep_for(50ms);
#endif
	}
}

future_t<> test_channel_producer(channel_t<std::string> c, size_t cnt)
{
	for (size_t i = 0; i < cnt; ++i)
	{
		co_await c.write(std::to_string(i));
#if OUTPUT_DEBUG
		{
			scoped_lock<std::mutex> __lock(cout_mutex);
			std::cout << "W " << i << "@" << std::this_thread::get_id() << std::endl;
		}
#endif
	}
}

const size_t WRITE_THREAD = 6;
const size_t READ_THREAD = 6;
const size_t READ_BATCH = 1000000;
const size_t MAX_CHANNEL_QUEUE = 5;		//0, 1, 5, 10, -1

void resumable_main_channel_mult_thread()
{
	channel_t<std::string> c(MAX_CHANNEL_QUEUE);

	std::thread write_th[WRITE_THREAD];
	for (size_t i = 0; i < WRITE_THREAD; ++i)
	{
		write_th[i] = std::thread([&]
		{
			local_scheduler_t my_scheduler;
			go test_channel_producer(c, READ_BATCH * READ_THREAD / WRITE_THREAD);
			this_scheduler()->run_until_notask();

			{
				scoped_lock<std::mutex> __lock(cout_mutex);
				std::cout << "Write OK\r\n";
			}
		});
	}

	std::this_thread::sleep_for(100ms);

	std::thread read_th[READ_THREAD];
	for (size_t i = 0; i < READ_THREAD; ++i)
	{
		read_th[i] = std::thread([&]
		{
			local_scheduler_t my_scheduler;
			go test_channel_consumer(c, READ_BATCH);
			this_scheduler()->run_until_notask();

			{
				scoped_lock<std::mutex> __lock(cout_mutex);
				std::cout << "Read OK\r\n";
			}
		});
	}
	
	std::this_thread::sleep_for(100ms);
	scheduler_t::g_scheduler.run_until_notask();

	for(auto & th : read_th)
		th.join();
	for (auto& th : write_th)
		th.join();

	std::cout << "OK: counter = " << gcounter.load() << std::endl;
}

int main()
{
	resumable_main_channel_mult_thread();
	return 0;
}
