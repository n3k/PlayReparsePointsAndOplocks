#pragma once
#include "ReparsePointAndOpLocks.h"
#include <winioctl.h>


static HANDLE open_file_overlapped(WCHAR *filename, DWORD shareMode ) {
	DWORD flags = FILE_FLAG_OVERLAPPED;
	
	if (GetFileAttributesW(filename) & FILE_ATTRIBUTE_DIRECTORY)
	{
		flags |= FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT;
	}

	HANDLE h = CreateFileW(
		filename,
		GENERIC_READ | GENERIC_WRITE, ///////////////////// CHECK THIS LATER
		shareMode, 
		0,
		OPEN_EXISTING,		
		flags,
		0);

	if (h == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	return h;
}

void _WaitCallback(PTP_CALLBACK_INSTANCE Instance,
	PVOID Parameter, PTP_WAIT Wait,
	TP_WAIT_RESULT WaitResult)
{
	UNREFERENCED_PARAMETER(Instance);
	UNREFERENCED_PARAMETER(Wait);
	UNREFERENCED_PARAMETER(WaitResult);

	FILE_OPLOCK *opLock = (FILE_OPLOCK *)Parameter;

	DWORD dwBytes;
	if (!GetOverlappedResult(opLock->hFile, &opLock->overlapped, &dwBytes, TRUE)) {
		printf("-> error during GetOverlappedResult call: %08x\n", GetLastError());
	}

	if (opLock->Callback != NULL) {
		opLock->Callback();
		printf("-> Callback for OpLock on %S was executed!\n", opLock->fileName);
	}

	CloseHandle(opLock->hFile);
	opLock->hFile = INVALID_HANDLE_VALUE; // To avoid Closing it again
	SetEvent(opLock->g_hLockCompleted);
}

__declspec(dllexport) FILE_OPLOCK* create_oplock(WCHAR *filename, DWORD shareMode, UserCallback opLockCallback) {
	FILE_OPLOCK *opLock = (FILE_OPLOCK *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FILE_OPLOCK));

	opLock->Callback = opLockCallback;
	opLock->InputBuffer.StructureVersion = REQUEST_OPLOCK_CURRENT_VERSION;
	opLock->InputBuffer.StructureLength = sizeof(REQUEST_OPLOCK_INPUT_BUFFER);
	opLock->InputBuffer.RequestedOplockLevel = OPLOCK_LEVEL_CACHE_READ | OPLOCK_LEVEL_CACHE_HANDLE;
	opLock->InputBuffer.Flags = REQUEST_OPLOCK_INPUT_FLAG_REQUEST;
	opLock->OutputBuffer.StructureVersion = REQUEST_OPLOCK_CURRENT_VERSION;
	opLock->OutputBuffer.StructureLength = sizeof(REQUEST_OPLOCK_OUTPUT_BUFFER);
	wcsncpy_s(opLock->fileName, wcslen(filename), filename, wcslen(opLock->fileName));

	opLock->g_hLockCompleted = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	HANDLE hFile = open_file_overlapped(filename, shareMode);
	if (hFile == NULL) {
		printf("-> error opening the file %S for the opLock: %08x\n", filename, GetLastError());
		return NULL;
	}	
	opLock->hFile = hFile;

	// An auto-reset event for the ThreadPool
	opLock->overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL); 

	opLock->ptpWait = CreateThreadpoolWait(_WaitCallback, opLock, NULL);
	if (opLock->ptpWait == NULL)
	{
		printf("-> error creating threadpool %08x\n", GetLastError());
		return NULL;
	}

	SetThreadpoolWait(opLock->ptpWait, opLock->overlapped.hEvent, NULL);

	DWORD bytesReturned = 0;
	DeviceIoControl(opLock->hFile, FSCTL_REQUEST_OPLOCK,
		&opLock->InputBuffer, sizeof(opLock->InputBuffer),
		&opLock->OutputBuffer, sizeof(opLock->OutputBuffer),
		NULL, &opLock->overlapped);

	DWORD err = GetLastError();
	if (err != ERROR_IO_PENDING) {
		printf("-> DeviceIoControl for the Oplock Failed %08x\n", err);
		return NULL;
	}

	return opLock;
}

__declspec(dllexport) void wait_for_lock(FILE_OPLOCK *opLock, UINT Timeout) {
	WaitForSingleObject(opLock->g_hLockCompleted, Timeout);
}

__declspec(dllexport) void delete_oplock(FILE_OPLOCK *opLock) {
	if (opLock->ptpWait) {
		SetThreadpoolWait(opLock->ptpWait, NULL, NULL);
		CloseThreadpoolWait(opLock->ptpWait);
	}

	if (opLock->overlapped.hEvent) {
		CloseHandle((opLock->overlapped.hEvent));
	}

	if (opLock->hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(opLock->hFile);
		opLock->hFile = INVALID_HANDLE_VALUE;
	}

	HeapFree(GetProcessHeap(), 0, opLock);
}