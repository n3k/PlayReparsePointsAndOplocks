#pragma once
#include "ReparsePointAndOpLocks.h"

WCHAR baseObjDir[] = L"\\RPC Control";

#pragma warning(disable : 4996)
void create_pseudo_symlink(WCHAR *src, WCHAR *dst) {

	WCHAR *slash = wcsrchr(src, L'\\');
	
	WCHAR symlink[2048] = { 0 };
	wcscpy(symlink, baseObjDir);
	wcscpy(symlink + wcslen(baseObjDir), slash);

	*slash = 0;

	printf("FilePath: %S - ObjSymLink: %S\n", src, symlink);

	create_mount_point(src, baseObjDir);

	create_dos_device_symlink(symlink, dst);
}


void delete_pseudo_symlink(WCHAR *src, WCHAR *dst) {
	WCHAR *slash = wcsrchr(src, L'\\');

	WCHAR symlink[2048] = { 0 };
	wcscpy(symlink, baseObjDir);
	wcscpy(symlink + wcslen(baseObjDir), slash);

	*slash = 0;

	printf("FilePath: %S - ObjSymLink: %S\n", src, symlink);

	remove_dos_device_symlink(symlink, dst);

	delete_mount_point(src, baseObjDir);
}