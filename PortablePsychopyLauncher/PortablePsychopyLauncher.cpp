//
// PortablePsychoPyLauncher
// 
// Author: Hiroyuki Sogo
//
#include <windows.h>
#include <tchar.h>

#define TCHAR_SIZE(buf) (sizeof(buf)/sizeof(TCHAR))


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

int APIENTRY _tWinMain(
    HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    LPTSTR lpCmdLine, 
    int nCmdShow)
{
	TCHAR szTmp[4096];
	TCHAR szCmd[1024];
	TCHAR szHomeDir[512];
	TCHAR szWinPythonDir[512];
	TCHAR szPythonDir[512];
	TCHAR szPythonDirFull[1024];
	TCHAR szPsychoPyDir[512];
	TCHAR szPath[2048];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

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
	HOME=C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\settings
	Path=C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\Lib\site-packages\PyQt5;
	     C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\Lib\site-packages\PyQt4;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\Lib\site-packages\PySide2;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\DLLs;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\Scripts;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\..\t;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\..\t\mingw32\bin;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\..\t\R\bin\x64;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\..\t\Julia\bin;
		 C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\..\t\n;
		 C:\Users\Hiroyuki\AppData\Local\Programs\Python\Python36-32;
	QT_API=pyqt5
	tmp_pyz=C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64\Lib\site-packages\PyQt4\qt.conf
	WINPYARCH = WIN-AMD64
	WINPYDEBUG = disabled
	WINPYDIR = C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\python-3.6.7.amd64
	WINPYDIRBASE = C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0
	winpython_ini = C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\settings\winpython.ini
	WINPYVER = 3.6.7.0Zero
	WINPYWORKDIR = C:\Users\Hiroyuki\Desktop\WinPython-64bit-3.6.7.0-PsychoPy3.0.0\scripts\..\Notebooks
	*/


	_stprintf_s(szCmd, _T("\"%s\\%s\\pythonw.exe\" -m psychopy.app.psychopyApp"), szWinPythonDir, szPythonDir);

	// Start PsychoPy
	if (!CreateProcess(
		NULL, szCmd, NULL, NULL, FALSE,
		NULL, NULL, NULL, &si, &pi)){
		_stprintf_s(szTmp, _T("Could not start PortablePsychoPy.\n\ncommand:\n%s"), szCmd);
		MessageBox(NULL, szTmp, _T("Error"), MB_ICONERROR);
		return -1;
	}

	DWORD r = WaitForSingleObject(pi.hProcess, INFINITE);
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

	MessageBox(NULL, _T("PortablePsychoPy is successfully closed."), _T("Information"), MB_ICONINFORMATION);
	return exitCode;

}