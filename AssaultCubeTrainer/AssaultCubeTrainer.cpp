#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <cstddef>
#include <tchar.h>
#include <vector>
#include "AssaultCubeOffsets.h"

// ATTENTION: YOU MUST RUN THIS WITH Release AND x86 

// TODO: make specific header for implementaitons in this cpp file

/*
* Traverse the process list to get the handle of the given process name.
*
* [proc_name] => the name of the process of which the handle is required
*
* [RETURNS] => the handle of the given process name
*/
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

/*
* Traverse the module list to get the base address of the given module name.
*
* [hProc] => the handle of the processes of the game
* [modName] => the module name to get the base address of
*
* [RETURNS] => pointer to the base address of the module
*/
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

/*
* Reads the value from the given process memory address.
*
* [hProc] => the handle of the processes of the game
* [memAddress] => the memory address from which to read the value
* 
* [RETURNS] => the value that has been read
*/
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

/*
* (OVERLOADED METHOD)
* Reads the value from the given process memory address. 
* The memAddress is modified to contain the value of the memory address that has been read using access by pointer.
* This is also called dereferencing the memory address.
* 
* [hProc] => the handle of the processes of the game
* [memAddress] => the memory address from which to read the value
* 
*/
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

/*
* Writes the given value to the given process memory address.
*
* [hProc] => the handle of the processes of the game
* [memAddress] => the memory address to access the value of
* [valToWrite] => the value to write to the given process memory address
*
*/
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

/*
* This method traverses the offsets from the (module base address + entity static address) to the dynamically 
* allocated memory of ammo, health, armor etc... depending on the offsets.
* 
* [hProc] => the handle of the processes of the game
* [modeBasePtr] => base pointer (0 offset) of the module/exe of the game
* [offsets] => vector of offsets to traverse
* 
* [RETURNS] => the memory address containing the ammo, hp or armor etc... value
*/
uintptr_t FindDynamicMemAddr(HANDLE hProc, uintptr_t modBasePtr, std::vector<uintptr_t> offsets)
{
	uintptr_t addr = modBasePtr;
	for (uint32_t offset_idx = 0; offset_idx < offsets.size(); offset_idx++)
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
	ChangeOffsetValue(proc_name, HEALTH_OFFSET, 10000);
	ChangeOffsetValue(proc_name, AR_AMMO_OFFSET2, 999);
	ChangeOffsetValue(proc_name, ARMOR_OFFSET, 99);
	ChangeOffsetValue(proc_name, GRENADE_OFFSET, 99);
	return 0;
}