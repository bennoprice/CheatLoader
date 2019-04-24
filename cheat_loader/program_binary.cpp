#include <Windows.h>
#include <fstream>
#include <iostream>
#include "xorstr.hpp"
#include "task_manager.hpp"
#include "program_binary.hpp"

namespace global
{
	extern std::unique_ptr<management::task_manager> task_manager;
}

namespace memory
{
	program_binary::program_binary(std::shared_ptr<std::vector<char>> binary)
		: _binary(binary)
	{ }

	void program_binary::dump_to_disk(std::string_view name, bool execute) const
	{
		std::ofstream file(name.data(), std::ios::binary);
		file.write(_binary->data(), _binary->size());
		file.close();

		std::cout << _xor_("[+] dumped binary to disk") << std::endl;

		if (execute)
		{
			::system(name.data());
			std::cout << _xor_("[+] executed binary") << std::endl;
		}
	}

	void program_binary::memory_execute() const
	{
		auto dos_header = PIMAGE_DOS_HEADER(_binary->data());
		auto nt_header = PIMAGE_NT_HEADERS(_binary->data() + dos_header->e_lfanew);

		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			global::task_manager->exception(_xor_("binary invalid").c_str());
		if (nt_header->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
			global::task_manager->exception(_xor_("binary is not x64").c_str());

		char cur_file_path[MAX_PATH];
		::GetModuleFileNameA(0, cur_file_path, sizeof(cur_file_path));

		STARTUPINFOA startup_info = { 0 };
		PROCESS_INFORMATION proc_info = { 0 };
		::CreateProcessA(cur_file_path, 0, 0, 0, false, CREATE_SUSPENDED, 0, 0, &startup_info, &proc_info);

		CONTEXT context = { 0 };
		context.ContextFlags = CONTEXT_FULL;
		::GetThreadContext(proc_info.hThread, &context);

		auto p_image_base = ::VirtualAllocEx(proc_info.hProcess, LPVOID(nt_header->OptionalHeader.ImageBase), nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		::WriteProcessMemory(proc_info.hProcess, p_image_base, _binary->data(), nt_header->OptionalHeader.SizeOfHeaders, nullptr);

		for (auto i = 0u; i < nt_header->FileHeader.NumberOfSections; ++i)
		{
			auto section_header = PIMAGE_SECTION_HEADER(_binary->data() + dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + sizeof(IMAGE_SECTION_HEADER) * i);
			::WriteProcessMemory(proc_info.hProcess, LPVOID(DWORD64(p_image_base) + section_header->VirtualAddress), _binary->data() + section_header->PointerToRawData, section_header->SizeOfRawData, nullptr);
		}
		::WriteProcessMemory(proc_info.hProcess, LPVOID(context.Rdx + 0x10), &nt_header->OptionalHeader.ImageBase, sizeof(LPVOID), nullptr);

		std::cout << _xor_("[+] mapped binary to memory") << std::endl;

		context.Rcx = DWORD64(p_image_base) + nt_header->OptionalHeader.AddressOfEntryPoint;
		::SetThreadContext(proc_info.hThread, &context);
		::ResumeThread(proc_info.hThread);

		std::cout << _xor_("[+] executed mapped memory") << std::endl;
	}
}