#pragma once
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <cstddef>
#include <tchar.h>
#include <vector>

/*
* Traverse the process list to get the handle of the given process name.
*
* [proc_name] => the name of the process of which the handle is required
*
* [RETURNS] => the handle of the given process name
*/
DWORD AttachProcess(const wchar_t* proc_name);

/*
* Traverse the module list to get the base address of the given module name.
*
* [hProc] => the handle of the processes of the game
* [modName] => the module name to get the base address of
*
* [RETURNS] => pointer to the base address of the module
*/
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

/*
* Reads the value from the given process memory address.
*
* [hProc] => the handle of the processes of the game
* [memAddress] => the memory address from which to read the value
*
* [RETURNS] => the value that has been read
*/
template <typename dataType>
dataType ReadFromProcMem(HANDLE hProc, uintptr_t memAddress);

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
void ReadFromProcMem(HANDLE hProc, uintptr_t* memAddress);

/*
* Writes the given value to the given process memory address.
*
* [hProc] => the handle of the processes of the game
* [memAddress] => the memory address to access the value of
* [valToWrite] => the value to write to the given process memory address
*
*/
void WriteToProcMem(HANDLE hProc, uintptr_t memAddress, uintptr_t valToWrite);

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
uintptr_t FindDynamicMemAddr(HANDLE hProc, uintptr_t modBasePtr, std::vector<uintptr_t> offsets);


void ChangeOffsetValue(const wchar_t* proc_name, std::vector<uintptr_t> offsets, uintptr_t prefferedValue);