#include "io.h"
#include "commandline.h"
#include "utils.h"
#include "debug.h"
#include <Windows.h>

static bool s_ioIsInit = false;
static char s_ioPrefix[MAX_PATH];

static void IO_Init()
{
	if (!s_ioIsInit)
	{
		CommandLine::FindParamValue("-fileroot", s_ioPrefix, sizeof(s_ioPrefix));

		s_ioIsInit = true;
	}
}

static int IO_AdjustFilepath(const char* filepath, char* adjustedPath, int adjustedPathSz)
{
	IO_Init();

	bool absolute = filepath[0] == '\0' || s_ioPrefix[0] == '\0';

	if (!absolute && (filepath[0] == '\\' || filepath[0] == '/'))
		absolute = true;

	if (!absolute && (filepath[1] && filepath[1] == ':'))
		absolute = true;

	return absolute ? StringUtils::strncpy(adjustedPath, adjustedPathSz, filepath) : StringUtils::snprintf(adjustedPath, adjustedPathSz, "%s/%s", s_ioPrefix, filepath);
}

bool IO::DirectoryExists(const char* path)
{
	char fp[MAX_PATH];
	IO_AdjustFilepath(path, fp, sizeof(fp));

	DWORD attrib = ::GetFileAttributesA(fp);
	return attrib != INVALID_FILE_ATTRIBUTES;
}

bool IO::FileExists(const char* filename)
{
	char fp[MAX_PATH];
	IO_AdjustFilepath(filename, fp, sizeof(fp));

	DWORD attrib = ::GetFileAttributesA(fp);
	return attrib != INVALID_FILE_ATTRIBUTES && attrib != FILE_ATTRIBUTE_DIRECTORY;

	/* another way...
	WIN32_FIND_DATAA fd;

	HANDLE file = ::FindFirstFileA(filename, &fd);
	if (file != INVALID_HANDLE_VALUE)
	{
		::FindClose(file);
		return true;
	}

	return false;
	*/
}

bool IO::IsDirectory(const char* path)
{
	char fp[MAX_PATH];
	IO_AdjustFilepath(path, fp, sizeof(fp));

	DWORD attrib = ::GetFileAttributesA(fp);
	return attrib == FILE_ATTRIBUTE_DIRECTORY;
}

Buffer<byte> IO::ReadFile(const char* filename, bool silent)
{
	char fp[MAX_PATH];
	IO_AdjustFilepath(filename, fp, sizeof(fp));

	Buffer<byte> buffer;

	HANDLE file = ::CreateFile(fp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		::GetFileSizeEx(file, &fileSize);
		buffer.SetSize(fileSize.LowPart);

#pragma warning(suppress: 28193) //False positive
		BOOL success = ::ReadFile(file, buffer.begin(), fileSize.LowPart, NULL, NULL);

		if (!silent && success == FALSE)
			Debug::Error(CSTR("Could not read file \'", filename, "\' (", fp, ')'));

		::CloseHandle(file);
	}
	else if (!silent)
		Debug::Error(CSTR("Could not open file \'", filename, "\' (", fp, ')'));

	return buffer;
}

bool IO::WriteFile(const char* filename, const byte* data, size_t length)
{
	char fp[MAX_PATH];
	IO_AdjustFilepath(filename, fp, sizeof(fp));

	HANDLE file = ::CreateFile(fp, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	BOOL success = ::WriteFile(file, data, (DWORD)length, NULL, NULL);
	::CloseHandle(file);

	return success;
}

Buffer<String> IO::FindFilesInDirectory(const char* search)
{
	char searchPath[MAX_PATH];
	IO_AdjustFilepath(search, searchPath, sizeof(searchPath));

	Buffer<String> filenames;

	WIN32_FIND_DATAA data;
	HANDLE hFile = ::FindFirstFileA(searchPath, &data);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			for (const char* c = data.cFileName; *c != '\0'; ++c)
				if (*c != '.')
				{
					filenames.Add(data.cFileName);
					break;
				}


		} while (::FindNextFile(hFile, &data));
	}

	return filenames;
}

Buffer<String> _FindFilesInDirectoryRecursive(const char* rootDir, const char* dir, const char* wildcard)
{
	Buffer<String> filenames;

	WIN32_FIND_DATAA data;
	HANDLE hFile = ::FindFirstFileA(CSTR(rootDir, dir, wildcard), &data);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			for (const char* c = data.cFileName; *c != '\0'; ++c)
				if (*c != '.')
				{
					String filename(String::Concat(dir, data.cFileName));

					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						filenames.AddMultiple(_FindFilesInDirectoryRecursive(rootDir, CSTR(filename, '/'), wildcard));
					else
						filenames.Add(filename);

					break;
				}


		} while (::FindNextFile(hFile, &data));
	}

	return filenames;
}

Buffer<String> IO::FindFilesInDirectoryRecursive(const char* dir, const char* wildcard)
{
	char searchPath[MAX_PATH];
	IO_AdjustFilepath(dir, searchPath, sizeof(searchPath));

	return _FindFilesInDirectoryRecursive(searchPath, "", wildcard);
}


/*
	File dialog stuff
*/

#include <ShObjIdl.h>
#include <string.h> //for wcscat

inline Buffer<COMDLG_FILTERSPEC> ConvertFilters(const Buffer<Pair<const wchar_t*>>& filters)
{
	Buffer<COMDLG_FILTERSPEC> fileTypes;
	fileTypes.SetSize(filters.GetSize());

	for (uint32 i = 0; i < filters.GetSize(); ++i)
	{
		fileTypes[i].pszName = filters[i].first;
		fileTypes[i].pszSpec = filters[i].second;
	}

	return fileTypes;
}

String IO::OpenFileDialog(const wchar_t* dir, const Buffer<Pair<const wchar_t*>>& filters)
{
	Buffer<COMDLG_FILTERSPEC> _fileTypes = ConvertFilters(filters);
	IFileOpenDialog* fileOpenDialog;
	String value = "";

	HRESULT result = ::CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&fileOpenDialog);
	if (SUCCEEDED(result))
	{
		fileOpenDialog->SetFileTypes((UINT)_fileTypes.GetSize(), _fileTypes.begin());

		IShellItem* folder;

		WCHAR fullPath[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, fullPath);
		wcscat_s(fullPath, dir);
		SHCreateItemFromParsingName(fullPath, NULL, IID_PPV_ARGS(&folder));
		fileOpenDialog->SetFolder(folder);

		result = fileOpenDialog->Show(NULL);
		if (SUCCEEDED(result))
		{
			IShellItem* item;

			result = fileOpenDialog->GetResult(&item);
			if (SUCCEEDED(result))
			{
				LPWSTR chosenPath;
				result = item->GetDisplayName(SIGDN_FILESYSPATH, &chosenPath);

				if (SUCCEEDED(result))
					value = StringUtils::FromWide(chosenPath);

				item->Release();
			}
		}

		fileOpenDialog->Release();
	}

	return value;
}

String IO::SaveFileDialog(const wchar_t* dir, const Buffer<Pair<const wchar_t*>>& filters)
{
	Buffer<COMDLG_FILTERSPEC> _fileTypes = ConvertFilters(filters);
	IFileSaveDialog* fileSaveDialog;
	String value = "";

	HRESULT result = ::CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, (void**)&fileSaveDialog);
	if (SUCCEEDED(result))
	{
		IShellItem* folder;

		WCHAR fullPath[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, fullPath);
		wcscat_s(fullPath, dir);
		SHCreateItemFromParsingName(fullPath, NULL, IID_PPV_ARGS(&folder));

		fileSaveDialog->SetDefaultExtension(L"lvl");
		fileSaveDialog->SetFileTypes((UINT)_fileTypes.GetSize(), _fileTypes.begin());
		fileSaveDialog->SetFolder(folder);

		result = fileSaveDialog->Show(NULL);
		if (SUCCEEDED(result))
		{
			IShellItem* item;

			result = fileSaveDialog->GetResult(&item);
			if (SUCCEEDED(result))
			{
				LPWSTR chosenPath;
				result = item->GetDisplayName(SIGDN_FILESYSPATH, &chosenPath);

				if (SUCCEEDED(result))
				{
					value = StringUtils::FromWide(chosenPath);
				}

				item->Release();
			}
		}

		fileSaveDialog->Release();
	}

	return value;
}

