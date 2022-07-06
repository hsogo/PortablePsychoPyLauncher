//
// PortablePsychoPyLauncher
// 
// Author: Hiroyuki Sogo
//
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <fstream>
#include <string>
#include "resource.h"

#define TCHAR_SIZE(buf) (sizeof(buf)/sizeof(TCHAR))

TCHAR szTmp[4096];
TCHAR szCmd[1024];
TCHAR szHomeDir[512];
TCHAR szWinPythonDir[512];
TCHAR szPythonDir[512];
TCHAR szPythonDirFull[1024];
TCHAR szPsychoPyDir[512];
TCHAR szPath[2048];

TCHAR szPsychoPyConfigDir[1024];
TCHAR szAppDataDir[512];

//From: http://www.hiramine.com/programming/windows/copydirectory.html
BOOL CopyDirectory(LPCTSTR lpExistingDirectoryName,
	LPCTSTR lpNewDirectoryName)
{
	// check input values
	if (NULL == lpExistingDirectoryName
		|| NULL == lpNewDirectoryName)
	{
		return FALSE;
	}

	// preserve directory name (adding '\' if it doesn't exist)
	TCHAR szDirectoryPathName_existing[_MAX_PATH];
	_tcsncpy_s(szDirectoryPathName_existing, _MAX_PATH, lpExistingDirectoryName, _TRUNCATE);
	if ('\\' != szDirectoryPathName_existing[_tcslen(szDirectoryPathName_existing) - 1])
	{	// Add '\'
		_tcsncat_s(szDirectoryPathName_existing, _MAX_PATH, _T("\\"), _TRUNCATE);
	}
	TCHAR szDirectoryPathName_new[_MAX_PATH];
	_tcsncpy_s(szDirectoryPathName_new, _MAX_PATH, lpNewDirectoryName, _TRUNCATE);
	if ('\\' != szDirectoryPathName_new[_tcslen(szDirectoryPathName_new) - 1])
	{	// Add '\'
		_tcsncat_s(szDirectoryPathName_new, _MAX_PATH, _T("\\"), _TRUNCATE);
	}

	if (-1 == _taccess(szDirectoryPathName_existing, 0))
	{
		return FALSE;
	}
	if (0 == _tcsicmp(szDirectoryPathName_existing, szDirectoryPathName_new))
	{
		return FALSE;
	}

	// Create new directory
	CreateDirectory(szDirectoryPathName_new, NULL);

	// Parse files in the directory
	TCHAR szFindFilePathName[_MAX_PATH];
	_tcsncpy_s(szFindFilePathName, _MAX_PATH, szDirectoryPathName_existing, _TRUNCATE);
	_tcsncat_s(szFindFilePathName, _T("*"), _TRUNCATE);

	WIN32_FIND_DATA		fd;
	HANDLE hFind = FindFirstFile(szFindFilePathName, &fd);
	if (INVALID_HANDLE_VALUE == hFind)
	{	// FindFirstFile failed
		return FALSE;
	}

	do
	{
		if ('.' != fd.cFileName[0])
		{
			TCHAR szFoundFilePathName_existing[_MAX_PATH];
			_tcsncpy_s(szFoundFilePathName_existing, _MAX_PATH, szDirectoryPathName_existing, _TRUNCATE);
			_tcsncat_s(szFoundFilePathName_existing, _MAX_PATH, fd.cFileName, _TRUNCATE);

			TCHAR szFoundFilePathName_new[_MAX_PATH];
			_tcsncpy_s(szFoundFilePathName_new, _MAX_PATH, szDirectoryPathName_new, _TRUNCATE);
			_tcsncat_s(szFoundFilePathName_new, _MAX_PATH, fd.cFileName, _TRUNCATE);

			if (FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes)
			{	// Call CopyDirctory recursively if the target is directory
				if (!CopyDirectory(szFoundFilePathName_existing, szFoundFilePathName_new))
				{
					FindClose(hFind);
					return FALSE;
				}
			}
			else
			{	// Copy file
				if (!CopyFile(szFoundFilePathName_existing, szFoundFilePathName_new, FALSE))
				{
					FindClose(hFind);
					return FALSE;
				}
			}
		}
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);

	return TRUE;
}


BOOL findDirectory(LPCTSTR lpTargetDir, LPCTSTR lpSearchName, LPTSTR foundDirName, int n)
{
	WIN32_FIND_DATA ffd;
	BOOL found = FALSE;
	HANDLE h = FindFirstFile(lpTargetDir, &ffd);
	if (h != INVALID_HANDLE_VALUE){
		do{
			if (_tcsncmp(lpSearchName, ffd.cFileName, lstrlen(lpSearchName)) == 0)
			{
				_tcscpy_s(foundDirName, n, ffd.cFileName);
				found = TRUE;
				break;
			}
		} while (FindNextFile(h, &ffd));

	}
	FindClose(h);
	return found;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lp) {
	TCHAR strWindowText[1024];
	BOOL* pfound = (BOOL*)lp;
	GetWindowText(hwnd, strWindowText, 1024);

	std::basic_string<TCHAR> wt = strWindowText;
	if (wt.find(_T("PsychoPy Runner"))!= std::string::npos || wt.find(_T("PsychoPy Builder"))!= std::string::npos || wt.find(_T("PsychoPy Coder"))!= std::string::npos)
	{
		*pfound = TRUE;
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static HFONT hFont, hFontPrev;
	static TCHAR szText[] = _T("Starting PsychoPy. Please wait...");

	switch (msg) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		hFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
		hFontPrev = (HFONT)SelectObject(hdc, hFont);
		TextOut(hdc, 10, 10, szText, lstrlen(szText));
		SelectObject(hdc, hFontPrev);
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return (DefWindowProc(hWnd, msg, wp, lp));
	}
	return 0L;
}

DWORD WINAPI findPsychoPyWindowThread(void* param)
{
	HWND* phWnd = (HWND*)param;
	BOOL found = FALSE;
	while (!found) {
		EnumWindows(EnumWindowsProc, (LPARAM)&found);
		Sleep(500);
	}
	SendMessage(*phWnd, WM_CLOSE, NULL, NULL);

	return 0;
}


int ppl_main(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	BOOL show_normal_termination_dialog = true;
	BOOL copy_config = false;

	// read config file
	std::fstream fs;
	fs.open("pplaunch_config.txt", std::ios::in);
	if (fs.is_open())
	{
		char buff[256], *p, *pp;
		int param;
		while (fs.getline(buff, sizeof(buff)))
		{
			//If ":" is not included, this line is not option.
			if ((p = strchr(buff, ':')) == NULL) continue;

			//remove space/tab
			*p = '\0';
			while (*(p - 1) == 0x09 || *(p - 1) == 0x20)
			{
				p--;
				*p = '\0';
			}
			while (*(p + 1) == 0x09 || *(p + 1) == 0x20) p++;

			// get first character of the parameter value
			p += 1;
			param = strtol(p, &pp, 10);

			if (strcmp(buff, "show_normal_termination_dialog") == 0) show_normal_termination_dialog = (BOOL)param;
			else if (strcmp(buff, "copy_config") == 0) copy_config = (BOOL)param;
		}
	}

	GetStartupInfo(&si);
	//si.dwFlags = STARTF_USESHOWWINDOW;
	//si.wShowWindow = SW_HIDE;

	GetCurrentDirectory(TCHAR_SIZE(szHomeDir), szHomeDir);
	GetEnvironmentVariable(_T("PATH"), szPath, TCHAR_SIZE(szPath));

	BOOL found;
	found = findDirectory(_T(".\\*.*"), _T("WinPython-"), szWinPythonDir, TCHAR_SIZE(szWinPythonDir));
	if (found)
	{
		_stprintf_s(szTmp, _T("%s\\*.*"), szWinPythonDir);
		found = findDirectory(szTmp, _T("python-"), szPythonDir, TCHAR_SIZE(szPythonDir));
	}
	
	// Neither of WinPython- and python- directory were found?
	if (!found)
	{
		MessageBox(NULL, _T("WinPython is not found in this directory."), _T("Error"), MB_ICONERROR);
		return -1;
	}
	_stprintf_s(szPythonDirFull, _T("%s\\%s\\%s"), szHomeDir, szWinPythonDir, szPythonDir);

	_stprintf_s(szTmp, _T("%s\\%s\\Lib\\site-packages\\*.*"), szWinPythonDir, szPythonDir);
	found = findDirectory(szTmp, _T("psychopy"), szPsychoPyDir, TCHAR_SIZE(szPsychoPyDir));

	if (!found){
		MessageBox(NULL, _T("PsychoPy is not found in site-packages."), _T("Error"), MB_ICONERROR);
		return -1;
	}

	// Set environment variable for ffmpeg
	//if (GetEnvironmentVariable(_T("IMAGEIO_FFMPEG_EXE"), szTmp, TCHAR_SIZE(szTmp)) == 0) {
	//	GetCurrentDirectory(TCHAR_SIZE(szTmp), szTmp);
	//	_stprintf_s(szFFMPEGFile, _T("%s\\ffmpeg\\ffmpeg-win32.exe"), szTmp);
	//	SetEnvironmentVariable(_T("IMAGEIO_FFMPEG_EXE"), szFFMPEGFile);
	//}

	// Set environment variables for WinPython
	_stprintf_s(szTmp, _T("%s\\%s"), szHomeDir, szWinPythonDir);
	SetEnvironmentVariable(_T("WINPYDIR"), szTmp);
	SetEnvironmentVariable(_T("WINPYDIRBASE"), szPythonDirFull);
	_stprintf_s(szTmp, _T("%s\\%s\\settings"), szHomeDir, szWinPythonDir);
	SetEnvironmentVariable(_T("HOME"), szTmp);
	_stprintf_s(szTmp, _T("%s\\%s\\settings\\winpython.ini"), szHomeDir, szWinPythonDir);
	SetEnvironmentVariable(_T("winpython_ini"), szTmp);
	_stprintf_s(szTmp, _T("%s;%s\\DLLs;%s\\Scripts;%s"), szPythonDirFull, szPythonDirFull, szPythonDirFull, szPath);
	SetEnvironmentVariable(_T("PATH"), szTmp);
	SetEnvironmentVariable(_T("WINPYDEBUG"), _T("disabled"));

	/*
	FINDDIR=C:\WINDOWS\system32
	HOME=python_path\settings
	Path=python_path\python-3.6.7.amd64\Lib\site-packages\PyQt5;
	     python_path\python-3.6.7.amd64\Lib\site-packages\PyQt4;
		 python_path\python-3.6.7.amd64\Lib\site-packages\PySide2;
		 python_path\python-3.6.7.amd64\;
		 python_path\python-3.6.7.amd64\DLLs;
		 python_path\python-3.6.7.amd64\Scripts;
		 python_path\python-3.6.7.amd64\..\t;
		 python_path\python-3.6.7.amd64\..\t\mingw32\bin;
		 python_path\python-3.6.7.amd64\..\t\R\bin\x64;
		 python_path\python-3.6.7.amd64\..\t\Julia\bin;
		 python_path\python-3.6.7.amd64\..\t\n;
		 C:\Users\Hiroyuki\AppData\Local\Programs\Python\Python36-32;
	QT_API=pyqt5
	tmp_pyz=python_path\python-3.6.7.amd64\Lib\site-packages\PyQt4\qt.conf
	WINPYARCH = WIN-AMD64
	WINPYDEBUG = disabled
	WINPYDIR = python_path\python-3.6.7.amd64
	WINPYDIRBASE = python_path
	winpython_ini = python_path\settings\winpython.ini
	WINPYVER = 3.6.7.0Zero
	WINPYWORKDIR = python_path\scripts\..\Notebooks
	*/

	_stprintf_s(szCmd, _T("\"%s\\%s\\pythonw.exe\" -m psychopy.app.psychopyApp"), szWinPythonDir, szPythonDir);

	size_t requiredSize;
	if (copy_config) {
		// Get full path of %APPDATA%\psychopy3 directory	
		_tgetenv_s(&requiredSize, NULL, 0, _T("APPDATA"));
		if (requiredSize == 0) {
			return -1;
		}
		else if (requiredSize >= TCHAR_SIZE(szAppDataDir)) {
			return -1;
		}
		_tgetenv_s(&requiredSize, szAppDataDir, requiredSize, _T("APPDATA"));

		// Copy configuration to %APPDATA%\psychopy3
		_stprintf_s(szPsychoPyConfigDir, _T("%s\\psychopy3\\"), szAppDataDir);
		if (PathIsDirectory(_T(".\\psychopy3\\"))) {
			CopyDirectory(_T(".\\psychopy3\\"), szPsychoPyConfigDir);
		}

		// Copy configuration to %APPDATA%\psychopy3
		_stprintf_s(szPsychoPyConfigDir, _T("%s\\psychopy3\\"), szAppDataDir);
		if (PathIsDirectory(_T(".\\psychopy3\\"))) {
			CopyDirectory(_T(".\\psychopy3\\"), szPsychoPyConfigDir);
		}
	}

	WNDCLASS wc;
	HWND hWnd;
	MSG msg;
	HANDLE hThread;
	LPCWSTR szClassName = _T("PortablePsychoPyLauncher");

	ZeroMemory(&wc, sizeof(WNDCLASS));
	if (!hPrevInstance) {
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(hInstance, (PCWSTR)IDI_ICON1);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = szClassName;
		if (!RegisterClass(&wc)) {
			DWORD e;
			e= GetLastError();
			MessageBox(NULL, _T("Can't register class"), _T("Error"), MB_ICONERROR);
			return -1;
		}
	}

	hWnd = CreateWindow(szClassName,
		_T("PortablePsychoPyLauncer"),
		WS_OVERLAPPED,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		300, 100, NULL, NULL,
		hInstance, NULL);
	if (!hWnd) {
		MessageBox(NULL, _T("Can't open window"), _T("Error"), MB_ICONERROR);
		return -1;
	}
	
	hThread = CreateThread(NULL, 0, findPsychoPyWindowThread, (void*)&hWnd, 0, NULL);
	if (hThread == NULL) {
		MessageBox(NULL, _T("Can't start thread"), _T("Error"), MB_ICONERROR);
		return -1;
	}

	// Start PsychoPy
	if (!CreateProcess(
		NULL, szCmd, NULL, NULL, FALSE,
		NULL, NULL, NULL, &si, &pi)) {
		_stprintf_s(szTmp, _T("Could not start PortablePsychoPy.\n\ncommand:\n%s"), szCmd);
		MessageBox(NULL, szTmp, _T("Error"), MB_ICONERROR);
		CloseHandle(hThread);
		return -1;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Waiting...
	DWORD r = WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(hThread);

	switch (r) {
	case WAIT_FAILED:
		MessageBox(NULL, _T("Wait failed (WAIT_FAILED)"), _T("Error"), MB_ICONERROR);
		return -1;
	case WAIT_ABANDONED:
		MessageBox(NULL, _T("Wait failed (WAIT_ABANDONED)"), _T("Error"), MB_ICONERROR);
		return -1;
	case WAIT_OBJECT_0:
		break;
	case WAIT_TIMEOUT:
		MessageBox(NULL, _T("Wait failed (WAIT_TIMEOUT)"), _T("Error"), MB_ICONERROR);
		return -1;
	default:
		_stprintf_s(szTmp, _T("Wait failed (code:%d)"), r);
		MessageBox(NULL, szTmp, _T("Error"), MB_ICONERROR);
		return -1;
	}

	DWORD exitCode;
	if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
		MessageBox(NULL, _T("Could not get Error Code."), _T("Error"), MB_ICONERROR);
		return -1;
	}

	if (exitCode != 0) {
		_stprintf_s(szTmp, _T("PortablePsychoPy is closed.\n(exit code: %X)"), exitCode);
		MessageBox(NULL, szTmp, _T("PortablePsychPyLauncher"), MB_ICONERROR);
		return -1;
	}

	//Close handles
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	//Normal termination
	if (copy_config) {
		//Copy configuration from %APPDATA%\psychopy3
		_stprintf_s(szPsychoPyConfigDir, _T("%s\\psychopy3\\"), szAppDataDir);
		if (PathIsDirectory(szPsychoPyConfigDir)) {
			CopyDirectory(szPsychoPyConfigDir, _T(".\\psychopy3\\"));
		}

		//Copy configuration from %APPDATA%\psychopy3
		_stprintf_s(szPsychoPyConfigDir, _T("%s\\psychopy3\\"), szAppDataDir);
		if (PathIsDirectory(szPsychoPyConfigDir)) {
			CopyDirectory(szPsychoPyConfigDir, _T(".\\psychopy3\\"));
		}
	}


	if (show_normal_termination_dialog) {
		MessageBox(NULL, _T("PortablePsychoPy is successfully closed."), _T("Information"), MB_ICONINFORMATION);
	}
	return exitCode;

}



int APIENTRY _tWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR lpCmdLine,
	_In_ int nCmdShow)
{
	int exit_code = 0;
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, TRUE, _T("PortablePsychoPyLauncherMutex"));
	if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, _T("PortablePsychoPyLauncher is already running."), _T("Error"), MB_ICONERROR);
		return -1;
	}
	else
	{
		exit_code = ppl_main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	}

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	return exit_code;
}
