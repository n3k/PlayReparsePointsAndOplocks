#include "ReparsePointAndOpLocks.h"


typedef struct _FILE_LINK_INFORMATION {
	BOOLEAN ReplaceIfExists;
	HANDLE  RootDirectory;
	ULONG   FileNameLength;
	WCHAR   FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;

ULONG FileLinkInformation = 11;

void resolve_nt_functions() {
	fpZwSetInformationFile = GetProcAddress(LoadLibraryA("ntdll.dll"), "ZwSetInformationFile");
	//printf("ZwSetInformationFile: %p\n", fpZwSetInformationFile);
	fpNtOpenFile = GetProcAddress(LoadLibraryA("ntdll.dll"), "NtOpenFile");
	//printf("NtOpenFile: %p\n", fpNtOpenFile);
	fpRtlInitUnicodeString = GetProcAddress(LoadLibraryA("ntdll.dll"), "RtlInitUnicodeString");
	//printf("RtlInitUnicodeString: %p\n", fpRtlInitUnicodeString);
}

BOOL create_hardlink(WCHAR *srcNativeFile, WCHAR *targetFile) {
	// srcNativeFile has to be a native path: i.e \??\c:\foo.txt
	// targetFile has to be a non-native path: i.e c:\bar.txt
	resolve_nt_functions();

	NTSTATUS status;

	UINT len_linkname = wcslen(srcNativeFile) * 2;
	UINT link_info_len = sizeof(FILE_LINK_INFORMATION) + len_linkname;
	PFILE_LINK_INFORMATION link_info = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, link_info_len);
	link_info->FileNameLength = len_linkname;

	memcpy(link_info->FileName, srcNativeFile, len_linkname);
	link_info->ReplaceIfExists = TRUE;

	HANDLE hFile = CreateFileW(targetFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	/*
	UNICODE_STRING name = { 0 };
	OBJECT_ATTRIBUTES obj_attr = { 0 };
	fpRtlInitUnicodeString(&name, nativeTargetFile);
	InitializeObjectAttributes(&obj_attr, &name, OBJ_CASE_INSENSITIVE, NULL, NULL);
	IO_STATUS_BLOCK io_status = { 0 };
	printf("x\n");
	HANDLE hFile = 0;
	status = fpNtOpenFile(&hFile, MAXIMUM_ALLOWED, &obj_attr, &io_status, FILE_SHARE_READ, NULL);
	if (!NT_SUCCESS(status))
	{
		printf("-> error opening %S for creating hardlink: %08x\n", nativeTargetFile, GetLastError());
		return FALSE;
	}
	*/
	IO_STATUS_BLOCK io_status = { 0 };

	//memset(&io_status, 0, sizeof(IO_STATUS_BLOCK));
	status = fpZwSetInformationFile(hFile, &io_status, link_info, link_info_len, FileLinkInformation);
	BOOL result = NT_SUCCESS(status);
	if (result) {
		printf("-> new hardlink created: %S -> %S\n", srcNativeFile, targetFile);
	}
	else {
		printf("-> error creating hardlink: %08x\n", GetLastError());
	}

	CloseHandle(hFile);

	HeapFree(GetProcessHeap(), 0, link_info);

	return result;
}