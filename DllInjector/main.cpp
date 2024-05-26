// Copyright 2022 Crystal Ferrai
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>
#include <strsafe.h>

bool fileExists(LPCWSTR path);
int findProcess(const wchar_t* processName);
void printLastError();

int wmain(int argc, const wchar_t* const* argv)
{
	if (argc != 3)
	{
		wprintf(L"Usage: DllInjector [process name] [dll path]\n");
		return 0;
	}

	const wchar_t* processName = argv[1];
	const wchar_t* dllPath = argv[2];

	int pid = findProcess(processName);
	if (pid < 0)
	{
		wprintf(L"Unable to find process matching the name %s.\n", processName);
		return 1;
	}

	if (!fileExists(dllPath))
	{
		wprintf(L"File '%s' not found\n", dllPath);
		return 1;
	}

	HANDLE process = OpenProcess(
		PROCESS_CREATE_THREAD |
		PROCESS_QUERY_INFORMATION |
		PROCESS_VM_OPERATION |
		PROCESS_VM_READ |
		PROCESS_VM_WRITE,
		false, pid);
	if (process == NULL)
	{
		wprintf(L"Failed to access process %s with PID %d. Error: ", processName, pid);
		printLastError();
		return 1;
	}

	wprintf(L"Injecting into process with PID %d...\n", pid);

	SIZE_T injectSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);

	LPVOID injectAddress = VirtualAllocEx(
		process,
		NULL,
		injectSize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READWRITE);
	if (injectAddress == NULL)
	{
		wprintf(L"Failed to allocate memory in target process. Error: ");
		printLastError();

		CloseHandle(process);
		return 1;
	}

	if (WriteProcessMemory(process, injectAddress, dllPath, injectSize, NULL) == FALSE)
	{
		wprintf(L"Failed to write memory in the target process. Error: ");
		printLastError();

		CloseHandle(process);
		return 1;
	}

	LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
	if (loadLibraryAddr == NULL)
	{
		wprintf(L"Failed to get the address of the LoadLibraryW function. Error: ");
		printLastError();

		CloseHandle(process);
		return 1;
	}

	HANDLE remoteThread = CreateRemoteThread(
		process,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)loadLibraryAddr,
		(LPVOID*)injectAddress,
		0,
		NULL);
	if (remoteThread == NULL)
	{
		wprintf(L"Failed to load the DLL in the target process. Error: ");
		printLastError();

		CloseHandle(process);
		return 1;
	}

	CloseHandle(process);

	wprintf(L"Injection successful.\n");

	return 0;
}

bool fileExists(LPCWSTR path)
{
	DWORD attrib = GetFileAttributes(path);
	return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

int findProcess(const wchar_t* processName)
{
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(PROCESSENTRY32W);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32FirstW(snapshot, &entry) == TRUE)
	{
		while (Process32NextW(snapshot, &entry) == TRUE)
		{
			if (_wcsicmp(entry.szExeFile, processName) == 0)
			{
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		}
	}
	CloseHandle(snapshot);
	return -1;
}

void printLastError()
{
	LPVOID buffer;
	DWORD code = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&buffer,
		0, NULL);

	wprintf(L"%s\n", (LPWSTR)buffer);

	LocalFree(buffer);
}