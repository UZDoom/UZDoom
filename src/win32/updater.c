/*
** updater.c
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "windows.h"
#include "shlwapi.h"
#include <string.h>
#include <process.h>

#pragma comment(lib, "Shlwapi.lib")

/*
 * this deliberately uses only C and stdlib/Win32 api calls to keep it dependency-free, and thus, updatable without replacing DLLs
 * it ignores memory management on purpose, as it is a short-lived program that immediately exits
 * (and yes, it was miserable to write)
 */

typedef struct
{
	WCHAR * str;
	int len;
} wstr_t;

#define wstr_lit(str) ((wstr_t){str, wcslen(str)})

wstr_t wstr_raw(WCHAR * str)
{
	int len = wcslen(str);
	wstr_t newstr;
	newstr.str = malloc((len + 1) * sizeof(WCHAR));
	newstr.len = len;

	memcpy(newstr.str, str, len * sizeof(WCHAR));
	newstr.str[len] = 0;

	return newstr;
}

wstr_t str_append(wstr_t str1, wstr_t str2)
{
	wstr_t newstr;
	newstr.str = malloc((str1.len + str2.len + 1) * sizeof(WCHAR));
	newstr.len = str1.len + str2.len;
	newstr.str[newstr.len] = 0;

	memcpy(newstr.str, str1.str, str1.len * sizeof(WCHAR));
	memcpy(newstr.str + str1.len, str2.str, str2.len * sizeof(WCHAR));

	free(str1.str);

	return newstr;
}

wstr_t str_concat(wstr_t str1, wstr_t str2)
{
	wstr_t newstr;
	newstr.str = malloc((str1.len + str2.len + 1) * sizeof(WCHAR));
	newstr.len = str1.len + str2.len;
	newstr.str[newstr.len] = 0;

	memcpy(newstr.str, str1.str, str1.len * sizeof(WCHAR));
	memcpy(newstr.str + str1.len, str2.str, str2.len * sizeof(WCHAR));

	return newstr;
}

typedef struct replaced_file_t
{
	wstr_t path;
	BOOL delete_only;
	struct replaced_file_t * next;
} replaced_file_t;

replaced_file_t * replaced_files;

BOOL RestoreBackup(wstr_t root_folder, wstr_t backup_folder)
{
	while(replaced_files)
	{
		wstr_t root_path = str_concat(root_folder, wstr_lit(L"\\"));
		root_path = str_append(root_path, replaced_files->path);

		wstr_t backup_path = str_concat(backup_folder, wstr_lit(L"\\"));
		backup_path = str_concat(backup_path, replaced_files->path);

		if(!DeleteFileW(root_path.str))
		{
			return 0;
		}

		if(!replaced_files->delete_only)
		{
			if(!MoveFileW(backup_path.str, root_path.str))
			{
				return 0;
			}
		}

		replaced_files = replaced_files->next;
	}

	return 1;
}

BOOL DoReplaceFile(wstr_t root_folder, wstr_t update_folder, wstr_t folder, wstr_t file);

BOOL DoReplaceFolder(wstr_t root_folder, wstr_t update_folder, wstr_t folder)
{
	wstr_t update_path;
	if(folder.str)
	{
		update_path = str_concat(update_folder, wstr_lit(L"\\"));
		update_path = str_append(update_path, folder);
	}
	else
	{
		update_path = update_folder;
	}

	wstr_t search_path = str_concat(update_path, wstr_lit(L"\\*"));

	//MessageBoxW(NULL, search_path.str, L"", MB_OK);
	WIN32_FIND_DATAW data;
	HANDLE it = FindFirstFileW(search_path.str, &data);
	if(it)
	{
		do
		{
			if(data.cFileName[0] == '.' && (data.cFileName[1] == '\0' || (data.cFileName[1] == '.' && data.cFileName[2] == '\0'))) continue;

			//MessageBoxW(NULL, data.cFileName, L"", MB_OK);
			if(!DoReplaceFile(root_folder, update_folder, folder, wstr_raw(data.cFileName)))
			{
				/*
				if(GetLastError() == ERROR_ACCESS_DENIED)
				{
					MessageBoxW(NULL, str_concat(wstr_lit(L"ACCESS DENIED - failed to replace "), wstr_raw(data.cFileName)).str, L"Update Failed", MB_OK);
				}
				else if(GetLastError() == ERROR_FILE_NOT_FOUND)
				{
					MessageBoxW(NULL, str_concat(wstr_lit(L"FILE NOT FOUND - failed to replace "), wstr_raw(data.cFileName)).str, L"Update Failed", MB_OK);
				}
				else
				{
					wchar_t tmp[1024];
					wsprintfW(tmp, L"failed to replace (0x%X)", GetLastError());
					MessageBoxW(NULL, str_concat(wstr_raw(tmp), wstr_raw(data.cFileName)).str, L"Update Failed", MB_OK);
				}
				*/
				FindClose(it);
				return 0;
			}
		}
		while(FindNextFileW(it, &data));

		FindClose(it);
	}

	return 1;
}

BOOL DoReplaceFile(wstr_t root_folder, wstr_t update_folder, wstr_t folder, wstr_t file)
{
	wstr_t new_path;
	if(folder.str)
	{
		new_path = str_concat(folder, wstr_lit(L"\\"));
		new_path = str_append(new_path, file);
	}
	else
	{
		new_path = file;
	}

	wstr_t root_path = str_concat(root_folder, wstr_lit(L"\\"));
	root_path = str_append(root_path, new_path);

	wstr_t update_path = str_concat(update_folder, wstr_lit(L"\\"));
	update_path = str_concat(update_path, new_path);

	if(GetFileAttributesW(update_path.str) & FILE_ATTRIBUTE_DIRECTORY)
	{
		return DoReplaceFolder(root_folder, update_folder, new_path);
	}
	else
	{
		int was_deleted = 0;

		if(PathFileExistsW(root_path.str))
		{
			was_deleted = 1;

			//int delete_ok = DeleteFileW(root_path.str);

			//DeleteFileW keeps RANDOMLY FAILING FOR NO REASON??????????? ugh
			//use SHFileOperationW instead
			// IT FAILS BECAUSE OF ERROR_SHARING_VIOLATION????????????????????????????????????????????????????????????
			// HOW DOES SHFileOperationW WORK THEN??????? IT'S CLEARLY NOT IN USE

			wstr_t root_path_fucked_up = str_concat(root_path, (wstr_t){L"\0", 1}); // for SHFileOperationW
			SHFILEOPSTRUCTW op;
			op.hwnd = NULL;
			op.wFunc = FO_DELETE;
			op.pFrom = root_path_fucked_up.str;
			op.pTo = NULL;
			op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

			int delete_ok = SHFileOperationW(&op) == 0;

			if(!delete_ok)
			{
				return 0;
			}
		}

		// file was deleted, must be restored if update fails
		replaced_file_t * old_head = replaced_files;

		replaced_files = malloc(sizeof(replaced_file_t));
		replaced_files->path = new_path;
		replaced_files->delete_only = !was_deleted;

		replaced_files->next = old_head;

		return MoveFileW(update_path.str, root_path.str);
	}
}

int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE nothing, LPWSTR cmdline, int nCmdShow)
{
	wstr_t path;
	path.str = malloc(sizeof(WCHAR) * 8192);

	GetModuleFileNameW(NULL, path.str, 8192);

	path.len = (wcsrchr(path.str, '\\') - path.str);

	path.str[path.len] = 0;

	wstr_t exe_path = str_concat(path, wstr_lit(L"\\uzdoom.exe"));
	wstr_t update_path = str_concat(path, wstr_lit(L"\\update"));
	wstr_t backup_path = str_concat(path, wstr_lit(L"\\update_backup"));

	if(PathFileExistsW(update_path.str) && PathFileExistsW(backup_path.str))
	{

		Sleep(1000); // sleep for 1s to make sure parent proccess fully exits
		//MessageBoxW(NULL, cmdline, L"", MB_OK);
		//MessageBoxW(NULL, exe_path.str, L"", MB_OK);

		//MessageBoxW(NULL, L"ok1", L"", MB_OK);
		BOOL ok = DoReplaceFolder(path, update_path, (wstr_t){NULL, 0});
		if(!ok)
		{
			MessageBoxW(NULL, L"Update failed, restoring backup", L"Update failed", MB_OK);

			if(!RestoreBackup(path, backup_path))
			{
				MessageBoxW(NULL, L"Backup restore failed???????", L"Backup restore failed", MB_OK);
				return 0;
			}
		}


		//ugh can only remove directory recursively with shell ops (or std filesystem but that's C++)

		//RemoveDirectoryW(update_path.str);
		//RemoveDirectoryW(backup_path.str);

		wstr_t update_path_fucked_up = str_concat(update_path, (wstr_t){L"\0", 1}); // for SHFileOperationW
		wstr_t backup_path_fucked_up = str_concat(backup_path, (wstr_t){L"\0", 1}); // for SHFileOperationW

		SHFILEOPSTRUCTW op;
		op.hwnd = NULL;
		op.wFunc = FO_DELETE;
		op.pFrom = update_path_fucked_up.str;
		op.pTo = NULL;
		op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

		SHFileOperationW(&op);
		op.pFrom = backup_path_fucked_up.str;
		SHFileOperationW(&op);

		wstr_t exe_path_new = wstr_raw(L"\"");

		wchar_t tmp[2] = L"\0\0";

		for(int i = 0; i < exe_path.len; i++)
		{
			if(exe_path.str[i] == L'\"' || exe_path.str[i] == L'\\')
			{
				exe_path_new = str_concat(exe_path_new, (wstr_t){L"\\", 1});
			}
			tmp[0] = exe_path.str[i];
			exe_path_new = str_concat(exe_path_new, (wstr_t){tmp, 1});
		}
		exe_path_new = str_concat(exe_path_new, (wstr_t){L"\"", 1});

		int argc;
		LPWSTR * argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		argv[0] = exe_path_new.str;
		_wexecv(exe_path.str, (const wchar_t * const *)argv);
		return 0;
	}
	else
	{
		return 1;
	}

}
