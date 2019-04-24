#include <iostream>
#include "task_manager.hpp"

namespace management
{
	task_manager::task_manager(int total_tasks)
		: _total_tasks(total_tasks)
		, _cur_task({ 0, false, "", nullptr })
	{ }

	void task_manager::inc_idx()
	{
		++_cur_task.idx;
	}

	void task_manager::exception(std::string_view error)
	{
		std::cout << "[-] " << error << std::endl;

		_cur_task.status = error;
		_cur_task.error = true;

		if (_cur_task.on_error != nullptr)
			_cur_task.on_error();
	}

	float task_manager::get_progress()
	{
		return static_cast<float>(_cur_task.idx) / static_cast<float>(_total_tasks);
	}

	task_manager::task task_manager::get_task()
	{
		return _cur_task;
	}
}