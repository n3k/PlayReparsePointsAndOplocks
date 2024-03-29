// PlayReparsePointsAndOpLocks.cpp : Defines the entry point for the console application.
//

#include "ReparsePointAndOpLocks.h"

void HandleOplock()
{
	printf("OpLock triggered, hit ENTER to close oplock\n");
	getchar();
}

HANDLE g_hFile;

void HandleOplock2() {
	WCHAR src[] = L"d:\\hacking\\Release\\foo";
	WCHAR dst[] = L"\\??\\d:\\hacking\\Release";
	printf("Entering OpLock handler!\n");
	create_mount_point_with_handle(g_hFile, dst);
	getchar(); // Stop and check that the mountpoint worked
}

void proof_of_concept() { // This form is not working
	// create oplock d:\hacking\Release\foo w
	WCHAR path[] = L"d:\\hacking\\Release\\foo";
	g_hFile = open_file_handle(path, FILE_SHARE_READ | FILE_SHARE_WRITE);

	FILE_OPLOCK *ol = create_oplock(path, FILE_SHARE_READ, HandleOplock2);
	wait_for_lock(ol, INFINITE);
	delete_oplock(ol);
}


void HandleOplock2_1() {
	WCHAR src[] = L"d:\\hacking\\Release\\foo";
	WCHAR dst[] = L"\\??\\d:\\hacking\\Release";
	printf("Entering OpLock handler!\n");

	getchar(); // Stop and check that the mountpoint worked
}

void proof_of_concept2() { 
						  
	WCHAR path[] = L"d:\\hacking\\Release\\foo";
	WCHAR dst[] = L"\\??\\d:\\hacking\\Release";
	g_hFile = open_file_handle(path, FILE_SHARE_READ | FILE_SHARE_WRITE);
	create_mount_point_with_handle(g_hFile, dst);
	getchar(); // Check the mountpoint
	
	FILE_OPLOCK *ol = create_oplock(path, FILE_SHARE_WRITE, HandleOplock2_1);
	wait_for_lock(ol, INFINITE);
	delete_oplock(ol);
}



int wmain(int argc, WCHAR* argv[])
{
	WCHAR *command_line = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x2000);
	WCHAR **args;
	BOOL exit = FALSE;
	while (exit != TRUE) {
		printf(">>> ");
		fflush(stdout);
		get_user_input(command_line, 0x2000);
		if (wcslen(command_line) == 0) {
			continue;
		}

		args = parse_arguments(command_line, ' ');
		int argc = (int)args[0];
		argc--;
		WCHAR **argv = &args[1];

		if (!wcscmp(argv[0], L"create")) {
			if (argc >= 4 && !wcscmp(argv[1], L"mountpoint")) {
				create_mount_point(argv[2], argv[3]);
			}
			else if (argc >= 4 && !wcscmp(argv[1], L"dosdevice")) {
				create_dos_device_symlink(argv[2], argv[3]);
			}
			else if (argc >= 4 && !wcscmp(argv[1], L"hardlink")) {
				create_hardlink(argv[2], argv[3]);
			}
			else if (argc >= 4 && !wcscmp(argv[1], L"oplock")) {
				DWORD shareMode = 0;
				if (wcschr(argv[3], 'r')) {
					shareMode |= FILE_SHARE_READ;
				}
				if (wcschr(argv[3], 'w')) {
					shareMode |= FILE_SHARE_WRITE;
				}
				if (wcschr(argv[3], 'd')) {
					shareMode |= FILE_SHARE_DELETE;
				}
				FILE_OPLOCK *ol = create_oplock(argv[2], shareMode, HandleOplock);
				wait_for_lock(ol, INFINITE);
				delete_oplock(ol);
			}	
			else if (argc >= 4 && !wcscmp(argv[1], L"symlink")) {
				create_pseudo_symlink(argv[2], argv[3]);
			}
		}
		else if (!wcscmp(argv[0], L"delete")) {
			if (argc >= 3 && !wcscmp(argv[1], L"mountpoint")) {
				delete_mount_point(argv[2]);
			}
			else if (argc >= 4 && !wcscmp(argv[1], L"dosdevice")) {
				remove_dos_device_symlink(argv[2], argv[3]);
			}
			else if (argc >= 4 && !wcscmp(argv[1], L"symlink")) {
				delete_pseudo_symlink(argv[2], argv[3]);
			}
		}
		else if (argc >= 2 && !wcscmp(argv[0], L"ismountpoint")) {
			is_mounted_directory(argv[1]);
		}
		else if (!wcscmp(argv[0], L"poc")) {
			proof_of_concept();
		}
		else if (!wcscmp(argv[0], L"exit")) {
			exit = TRUE;
		}
		free(args);
	}
	

	HeapFree(GetProcessHeap(), 0, command_line);
    return 0;
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpvReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:

		if (lpvReserved != NULL)
		{
			break; // do not do cleanup if process termination scenario
		}

		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

