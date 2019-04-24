#pragma once
#include <mutex>
#include <iostream>
#include <functional>
#include <string_view>

namespace management
{
	class task_manager
	{
	private:
		struct task
		{
			int idx;
			bool error;
			std::string_view status;
			std::function<void()> on_error;
		};
	public:
		task_manager(int total_tasks);

		template <typename T>
		T register_task(std::string_view status, std::function<T()> action, std::function<void()> on_error = nullptr, bool log = false)
		{
			if (log)
				std::cout << "[~] " << status << std::endl;

			_cur_task.status = status;
			_cur_task.on_error = on_error; // std::move instead

			auto result = action();
			++_cur_task.idx;
			return result;
		}

		void inc_idx();
		void exception(std::string_view error);
		float get_progress();
		task get_task();
	private:
		int _total_tasks;
		task _cur_task;
	};
}