#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <cstddef>
#include <tchar.h>
#include <vector>
#include "AssaultCubeOffsets.h"


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
		std::cout << "[FAILED] Reading memory address " << memAddress << " failed." << std::endl;
	}
	else
	{
		std::cout << "[SUCCESS] Memory address " << memAddress << " dereferenced and has value: " << val << std::endl;
	}

	return val;
}

uintptr_t ReadFromProcMem(HANDLE hProc, uintptr_t memAddress)
{
	const uintptr_t old_mem_addr = memAddress;
	const BOOL readStatus = ReadProcessMemory(hProc, (BYTE*)memAddress, &memAddress, sizeof(memAddress), nullptr);

	if (!readStatus)
	{
		std::cout << "[FAILED] Reading memory address " << old_mem_addr << " failed." << std::endl;
	}
	else
	{
		std::cout << "[SUCCESS] Memory address " << old_mem_addr << " dereferenced and updated to new memory address " << memAddress << std::endl;
	}

	return memAddress;
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
	for (uint32_t offset_idx = 0; offset_idx < offsets.size(); offset_idx++)
	{
		// TODO: pas addr by reference instead of by value!
		addr = ReadFromProcMem(hProc, addr);
		addr += offsets[offset_idx];
	}
	return addr;
}


int main()
{
	const wchar_t* proc_name = L"ac_client.exe";
	DWORD Pid = AttachProcess(proc_name);
	uintptr_t modBaseAddr = GetModuleBaseAddress(Pid, proc_name) + SINGLEPLAYER_MULTIPLAYER_STATIC_OFFSET_ADDR;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);
	uintptr_t ptrHp = FindDynamicMemAddr(hProc, modBaseAddr, AR_AMMO_OFFSET);

	uintptr_t health = -1;
	ReadProcessMemory(hProc, (BYTE*)ptrHp, &health, sizeof(ptrHp), nullptr);
	std::cout << "PID: " << Pid <<  std::endl;
	std::cout << "old value: " << health <<  std::endl;

	uintptr_t health_new = 1337;
	WriteToProcMem(hProc, ptrHp, health_new);

	ReadProcessMemory(hProc, (BYTE*)ptrHp, &health, sizeof(ptrHp), nullptr);
	std::cout << "updated value: " << health << std::endl;

	return 0;
}