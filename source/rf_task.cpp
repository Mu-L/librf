﻿#include "librf/librf.h"

namespace librf
{
	task_t::task_t() noexcept
		: _stop(nostopstate)
	{
	}

	task_t::~task_t()
	{
	}

	const stop_source & task_t::get_stop_source()
	{
		_stop.make_sure_possible();
		return _stop;
	}
}
