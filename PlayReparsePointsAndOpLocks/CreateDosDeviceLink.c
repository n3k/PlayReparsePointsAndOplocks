#include "ReparsePointAndOpLocks.h"

static void make_dos_symlink(WCHAR *target, WCHAR *outSymlink) {
	WCHAR root[] = L"Global\\GLOBALROOT";
	
	if (target[0] == '\\') { // We know is trying to write into a different place than GLOBAL??
		wmemcpy(outSymlink, root, wcslen(root));
		wmemcpy(outSymlink + wcslen(root), target, wcslen(target));
	}
	else {
		wmemcpy(outSymlink, target, wcslen(target));
	}
	printf("Symlink: %S\n", outSymlink);
}

void create_dos_device_symlink(WCHAR *src, WCHAR *dst) {
	WCHAR symlink[2048] = { 0 };
	make_dos_symlink(src, symlink);

	if (DefineDosDevice(DDD_NO_BROADCAST_SYSTEM | DDD_RAW_TARGET_PATH, symlink, dst))
	{
		printf("-> created DOS symlink into %S\n", dst);
	}
	else
	{
		printf("-> error creating DOS symlink: %08x\n", GetLastError());
	}
}

void remove_dos_device_symlink(WCHAR *src, WCHAR *dst) {
	WCHAR symlink[2048] = { 0 };
	make_dos_symlink(src, symlink);

	if (DefineDosDeviceW(
		DDD_NO_BROADCAST_SYSTEM | DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE,
		symlink,
		dst))
	{
		printf("-> DOS Symlink to %S was removed\n", dst);
	} 
	else {
		printf("-> error removing DOS symlink: %08x\n", GetLastError());
	}
}