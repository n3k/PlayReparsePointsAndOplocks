#include "ReparsePointAndOpLocks.h"

typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR  PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	} DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#define REPARSE_DATA_BUFFER_HEADER_LENGTH FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer)

HANDLE open_file_handle(WCHAR *filename, DWORD dwSharedMode) {
	HANDLE h = CreateFileW(filename,
		GENERIC_READ | GENERIC_WRITE,
		dwSharedMode,
		0,
		OPEN_EXISTING,
		// FILE_FLAG_BACKUP_SEMANTICS must be to obtain a handle to directories
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
		0);

	if (h == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	return h;
}

static UINT _create_reparse_point_for_mount_point(REPARSE_DATA_BUFFER *buffer, WCHAR *dstDirectory) {
	// forshaw stuff
	//size_t convertedChars = 0;
	//size_t origSize = strlen(dstDirectory) + 1;
	//WCHAR wcsDstDirectory[1024] = { 0 };
	//mbstowcs_s(&convertedChars, wcsDstDirectory, origSize, dstDirectory, _TRUNCATE);
	WCHAR *wcsDstDirectory = dstDirectory;
	size_t target_byte_size = wcslen(wcsDstDirectory) * 2;
	size_t path_buffer_size = target_byte_size + 8 + 4;
	size_t total_size = path_buffer_size + REPARSE_DATA_BUFFER_HEADER_LENGTH;

	buffer->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
	buffer->ReparseDataLength = (USHORT)path_buffer_size;
	buffer->Reserved = 0;
	buffer->MountPointReparseBuffer.SubstituteNameOffset = 0;
	buffer->MountPointReparseBuffer.SubstituteNameLength = (USHORT)target_byte_size;
	buffer->MountPointReparseBuffer.PrintNameOffset = (USHORT)target_byte_size + 2;
	buffer->MountPointReparseBuffer.PrintNameLength = 0;
	memcpy(buffer->MountPointReparseBuffer.PathBuffer, wcsDstDirectory, target_byte_size + 2);

	//print_memory(0, buffer, total_size);
	return total_size;
}

BOOL create_mount_point_with_handle(HANDLE hSrcDirectory, WCHAR *dstDirectory) {
	HANDLE hFile = hSrcDirectory;

	REPARSE_DATA_BUFFER *reparse_point = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x1000);
	UINT reparse_point_size = _create_reparse_point_for_mount_point(reparse_point, dstDirectory);

	DWORD bytesReturned = 0;
	BOOL ret = DeviceIoControl(hFile, FSCTL_SET_REPARSE_POINT,
		reparse_point, reparse_point_size,
		NULL, NULL,
		&bytesReturned, NULL) == TRUE;

	HeapFree(GetProcessHeap(), 0, reparse_point);
	if (!ret) {
		printf("-> error during FSCTL_SET_REPARSE_POINT: %08x\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL create_mount_point(WCHAR *srcDirectory, WCHAR *dstDirectory) {
	HANDLE hFile = open_file_handle(srcDirectory, NULL);
	if (hFile == NULL) {
		printf("-> error opening source directory for reparse point: %08x\n", GetLastError());
		return FALSE;
	}

	REPARSE_DATA_BUFFER *reparse_point = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x1000);
	UINT reparse_point_size = _create_reparse_point_for_mount_point(reparse_point, dstDirectory);

	DWORD bytesReturned = 0;
	BOOL ret = DeviceIoControl(hFile, FSCTL_SET_REPARSE_POINT,
		reparse_point, reparse_point_size,
		NULL, NULL,
		&bytesReturned, NULL) == TRUE;

	HeapFree(GetProcessHeap(), 0, reparse_point);
	if (!ret) {
		printf("-> error during FSCTL_SET_REPARSE_POINT: %08x\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

__declspec(dllexport) BOOL delete_mount_point(WCHAR *path) {
	HANDLE hFile = open_file_handle(path, NULL);
	if (hFile == NULL) {
		printf("-> error opening path for deleting reparse point: %08x\n", GetLastError());
		return FALSE;
	}

	REPARSE_DATA_BUFFER reparse_data_buffer = { 0 };
	reparse_data_buffer.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;

	DWORD bytesReturned = 0;
	BOOL ret = DeviceIoControl(hFile, FSCTL_DELETE_REPARSE_POINT,
		&reparse_data_buffer, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,
		NULL, 0,
		&bytesReturned, 0) == TRUE;

	if (!ret) {
		printf("-> error during FSCTL_DELETE_REPARSE_POINT: %08x\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}


BOOL is_mounted_directory(WCHAR *directoryName) {
	BOOL result = FALSE;

	DWORD fileAttributes = GetFileAttributesW(directoryName);
	if (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
		printf("-> The file has a reparse point => ");

		WIN32_FIND_DATAW data = { 0 };
		HANDLE h = FindFirstFileW(directoryName, &data);
		if (data.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT) {
			printf("IO_REPARSE_TAG_MOUNT_POINT\n");
			result = TRUE;
		}
		else {
			printf("%08x\n", data.dwReserved0);
		}
		FindClose(h);
	}
	else {
		printf("-> No reparse point in the file\n");
	}
	return result;
}
