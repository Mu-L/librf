﻿#include "librf/librf.h"

namespace librf
{
	LIBRF_API state_base_t::~state_base_t()
	{
	}
	
	LIBRF_API void state_base_t::destroy_deallocate()
	{
		delete this;
	}

	LIBRF_API void state_base_t::resume()
	{
		if (likely(_coro))
		{
			coroutine_handle<> handler = _coro;
			_coro = nullptr;
			_scheduler->del_final(this);
			handler.resume();
		}
	}

	LIBRF_API bool state_base_t::has_handler() const  noexcept
	{
		return (bool)_coro;
	}

	LIBRF_API state_base_t* state_base_t::get_parent() const noexcept
	{
		return nullptr;
	}

	LIBRF_API void state_future_t::destroy_deallocate()
	{
		size_t _Size = this->_alloc_size;
#if RESUMEF_DEBUG_COUNTER
		std::cout << "destroy_deallocate, size=" << _Size << std::endl;
#endif
		this->~state_future_t();

		_Alloc_char _Al;
		return _Al.deallocate(reinterpret_cast<char*>(this), _Size);
	}

	LIBRF_API state_generator_t* state_generator_t::_Alloc_state()
	{
		_Alloc_char _Al;
		size_t _Size = _Align_size<state_generator_t>();
#if RESUMEF_DEBUG_COUNTER
		std::cout << "state_generator_t::alloc, size=" << sizeof(state_generator_t) << std::endl;
#endif
		char* _Ptr = _Al.allocate(_Size);
		return new(_Ptr) state_generator_t();
	}

	LIBRF_API void state_generator_t::destroy_deallocate()
	{
		size_t _Size = _Align_size<state_generator_t>();
#if RESUMEF_DEBUG_COUNTER
		std::cout << "destroy_deallocate, size=" << _Size << std::endl;
#endif
		this->~state_generator_t();

		_Alloc_char _Al;
		return _Al.deallocate(reinterpret_cast<char*>(this), _Size);
	}

	LIBRF_API void state_generator_t::resume()
	{
		if (likely(_coro))
		{
			_coro.resume();
			if (likely(!_coro.done()))
			{
				_scheduler->add_generator(this);
			}
			else
			{
				coroutine_handle<> handler = _coro;
				_coro = nullptr;
				_scheduler->del_final(this);

				handler.destroy();
			}
		}
	}

	LIBRF_API bool state_generator_t::has_handler() const noexcept
	{
		return (bool)_coro;
	}
	
	LIBRF_API bool state_generator_t::switch_scheduler_await_suspend(scheduler_t* sch)
	{
		assert(sch != nullptr);

		if (_scheduler != nullptr)
		{
			if (_scheduler == sch) return false;

			auto task_ptr = _scheduler->del_switch(this);
			
			_scheduler = sch;
			if (task_ptr != nullptr)
				sch->add_switch(std::move(task_ptr));
		}
		else
		{
			_scheduler = sch;
		}

		return true;
	}

	LIBRF_API state_base_t* state_future_t::get_parent() const noexcept
	{
		return _parent;
	}

	LIBRF_API void state_future_t::resume()
	{
		std::unique_lock<lock_type> __guard(_mtx);

		if (_is_initor == initor_type::Initial)
		{
			assert((bool)_initor);

			coroutine_handle<> handler = _initor;
			_is_initor = initor_type::None;
			__guard.unlock();

			handler.resume();
			return;
		}
		
		if (_coro)
		{
			coroutine_handle<> handler = _coro;
			_coro = nullptr;
			__guard.unlock();

			handler.resume();
			return;
		}

		if (_is_initor == initor_type::Final)
		{
			assert((bool)_initor);

			coroutine_handle<> handler = _initor;
			_is_initor = initor_type::None;
			__guard.unlock();

			handler.destroy();
			return;
		}
	}

	LIBRF_API bool state_future_t::has_handler() const noexcept
	{
		scoped_lock<lock_type> __guard(this->_mtx);
		return has_handler_skip_lock();
	}

	LIBRF_API bool state_future_t::switch_scheduler_await_suspend(scheduler_t* sch)
	{
		assert(sch != nullptr);
		scoped_lock<lock_type> __guard(this->_mtx);

		if (_scheduler != nullptr)
		{
			if (_scheduler == sch) return false;

			auto task_ptr = _scheduler->del_switch(this);

			_scheduler = sch;
			if (task_ptr != nullptr)
				sch->add_switch(std::move(task_ptr));
		}
		else
		{
			_scheduler = sch;
		}

		if (_parent != nullptr)
			_parent->switch_scheduler_await_suspend(sch);

		return true;
	}

	LIBRF_API void state_t<void>::future_await_resume()
	{
		scoped_lock<lock_type> __guard(this->_mtx);

		if (this->_exception)
			std::rethrow_exception(std::move(this->_exception));
		if (this->_has_value.load(std::memory_order_acquire) == result_type::None)
			std::rethrow_exception(std::make_exception_ptr(future_exception{error_code::not_ready}));
	}

	LIBRF_API void state_t<void>::set_value()
	{
		scoped_lock<lock_type> __guard(this->_mtx);
		this->_has_value.store(result_type::Value, std::memory_order_release);

		scheduler_t* sch = this->get_scheduler();
		if (sch != nullptr)
		{
			if (this->has_handler_skip_lock())
				sch->add_generator(this);
			else
				sch->del_final(this);
		}
	}

	LIBRF_API void state_t<void>::set_exception(std::exception_ptr e)
	{
		scoped_lock<lock_type> __guard(this->_mtx);
		this->_exception = std::move(e);

		scheduler_t* sch = this->get_scheduler();
		if (sch != nullptr)
		{
			if (this->has_handler_skip_lock())
				sch->add_generator(this);
			else
				sch->del_final(this);
		}
	}
}
