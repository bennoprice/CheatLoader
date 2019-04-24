#pragma once
#include <memory>
#include <vector>

namespace memory
{
	class program_binary
	{
	public:
		explicit program_binary(std::shared_ptr<std::vector<char>> binary);
		void dump_to_disk(std::string_view name, bool execute) const;
		void memory_execute() const;
	private:
		std::shared_ptr<std::vector<char>> _binary;
	};
}