#include "AssaultCubeHeader.h"
#include "AssaultCubeOffsets.h"

// ATTENTION: YOU MUST RUN THIS WITH Release AND x86 

DWORD AttachProcess(const wchar_t* proc_name)
{
	DWORD PID = 0;

	// Take a snapshot of all processes in the system.
	auto hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (hProcSnap != INVALID_HANDLE_VALUE)
	{
		// processes structure object
		PROCESSENTRY32 procEnt;

		// Set the size of the structure before using it.
		procEnt.dwSize = sizeof(procEnt);

		// Now walk the snapshot of processes, and
		// display information about each process in turn
		while (Process32Next(hProcSnap, &procEnt))
		{
			if (!_wcsicmp(proc_name, procEnt.szExeFile))
			{
				std::cout << "Process found: " << procEnt.szExeFile << " PID: " << procEnt.th32ProcessID << std::endl;
				PID = procEnt.th32ProcessID;
				break;
			}
		}
	}
	else
	{
		std::cout << "Could not get snapshot of all running processes." << std::endl;
	}

	if (PID == 0)
	{
		std::cout << "Process " << proc_name << " could not be found in the process list" << std::endl;
	}

	CloseHandle(hProcSnap);
	return PID;
}


uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEnt;
		modEnt.dwSize = sizeof(modEnt);

		while (Module32Next(hSnap, &modEnt))
		{
			if (!_wcsicmp(modEnt.szModule, modName))
			{
				std::cout << "Module found: " << modName << std::endl;
				modBaseAddr = (uintptr_t)modEnt.modBaseAddr;
				break;
			}
		}
	}
	else
	{
		std::cout << "Could not get snapshot of modules." << std::endl;
	}

	if (modBaseAddr == 0)
	{
		std::cout << "Module " << modName << " could not be found in the modules list" << std::endl;
	}

	CloseHandle(hSnap);
	return modBaseAddr;
}

template <typename dataType>
dataType ReadFromProcMem(HANDLE hProc, uintptr_t memAddress)
{
	dataType val = 0;
	const BOOL readStatus = ReadProcessMemory(hProc, (BYTE*)memAddress, &val, sizeof(memAddress), nullptr);

	if (!readStatus)
	{
		std::cout << "[FAILED] Reading memory address failed." << std::endl;
	}
	else
	{
		std::cout << "[SUCCESS] Reading memory address " << memAddress << " was successful." << std::endl;
	}

	return val;
}


void ReadFromProcMem(HANDLE hProc, uintptr_t* memAddress)
{
	const BOOL readStatus = ReadProcessMemory(hProc, (BYTE*)(*memAddress), memAddress, sizeof(*memAddress), nullptr);

	if (!readStatus)
	{
		std::cout << "[FAILED] Reading memory address failed." << std::endl;
	}
	else
	{
		std::cout << "[SUCCESS] Reading memory address " << *memAddress << " was successful." << std::endl;
	}
}


void WriteToProcMem(HANDLE hProc, uintptr_t memAddress, uintptr_t valToWrite)
{
	BOOL writeStatus = WriteProcessMemory(hProc, (BYTE*)memAddress, &valToWrite, sizeof(memAddress), nullptr);

	if (writeStatus == FALSE)
	{
		std::cout << "Failed to write value to address: " << memAddress << std::endl;
	}
	else
	{
		std::cout << "Successfully written value to address: " << memAddress << std::endl;
	}
}


uintptr_t FindDynamicMemAddr(HANDLE hProc, uintptr_t modBasePtr, std::vector<uintptr_t> offsets)
{
	uintptr_t addr = modBasePtr;
	for (unsigned int offset_idx = 0; offset_idx < offsets.size(); ++offset_idx)
	{
		ReadFromProcMem(hProc, &addr);
		addr += offsets[offset_idx];
	}
	return addr;
}


void ChangeOffsetValue(const wchar_t* proc_name, std::vector<uintptr_t> offsets, uintptr_t prefferedValue)
{
	DWORD Pid = AttachProcess(proc_name);
	uintptr_t modBaseAddr = GetModuleBaseAddress(Pid, proc_name) + ENTITY_STATIC_OFFSET_ADDR;

	if (Pid == 0 || modBaseAddr == 0)
	{
		return;
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
	uintptr_t addrContainingValue= FindDynamicMemAddr(hProc, modBaseAddr, offsets);

	std::cout << "Value BEFORE change: " << ReadFromProcMem<uintptr_t>(hProc, addrContainingValue) << std::endl;
	
	WriteToProcMem(hProc, addrContainingValue, prefferedValue);

	std::cout << "Value AFTER change: " << ReadFromProcMem<uintptr_t>(hProc, addrContainingValue) << std::endl;

	CloseHandle(hProc);
}


int main()
{
	const wchar_t* proc_name = L"ac_client.exe";
	ChangeOffsetValue(proc_name, HEALTH_OFFSET, 999);
	ChangeOffsetValue(proc_name, AR_AMMO_OFFSET2, 999);
	ChangeOffsetValue(proc_name, ARMOR_OFFSET, 99);
	ChangeOffsetValue(proc_name, GRENADE_OFFSET, 99);
	return 0;
}