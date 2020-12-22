//
// PPPLauncher_CopyConfig
// 
// Author: Hiroyuki Sogo
//
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>

#define TCHAR_SIZE(buf) (sizeof(buf)/sizeof(TCHAR))

//From: http://www.hiramine.com/programming/windows/copydirectory.html
BOOL CopyDirectory( LPCTSTR lpExistingDirectoryName,
					LPCTSTR lpNewDirectoryName )
{
	// check input values
	if( NULL == lpExistingDirectoryName
	 || NULL == lpNewDirectoryName )
	{
		return FALSE;
	}

	// preserve directory name (adding '\' if it doesn't exist)
	TCHAR szDirectoryPathName_existing[_MAX_PATH];
	_tcsncpy_s( szDirectoryPathName_existing, _MAX_PATH, lpExistingDirectoryName, _TRUNCATE );
	if( '\\' != szDirectoryPathName_existing[_tcslen(szDirectoryPathName_existing) - 1] )
	{	// Add '\'
		_tcsncat_s( szDirectoryPathName_existing, _MAX_PATH, _T("\\"), _TRUNCATE );
	}
	TCHAR szDirectoryPathName_new[_MAX_PATH];
	_tcsncpy_s( szDirectoryPathName_new, _MAX_PATH, lpNewDirectoryName, _TRUNCATE );
	if( '\\' != szDirectoryPathName_new[_tcslen(szDirectoryPathName_new) - 1] )
	{	// Add '\'
		_tcsncat_s( szDirectoryPathName_new, _MAX_PATH, _T("\\"), _TRUNCATE );
	}

	if( -1 == _taccess( szDirectoryPathName_existing, 0 ) )
	{
		return FALSE;
	}
	if( 0 == _tcsicmp( szDirectoryPathName_existing, szDirectoryPathName_new ) )
	{
		return FALSE;
	}

	// Create new directory
	CreateDirectory( szDirectoryPathName_new, NULL );

	// Parse files in the directory
	TCHAR szFindFilePathName[_MAX_PATH];
	_tcsncpy_s( szFindFilePathName, _MAX_PATH, szDirectoryPathName_existing, _TRUNCATE );
	_tcsncat_s( szFindFilePathName, _T("*"), _TRUNCATE );

	WIN32_FIND_DATA		fd;
	HANDLE hFind = FindFirstFile( szFindFilePathName, &fd );
	if( INVALID_HANDLE_VALUE == hFind )
	{	// FindFirstFile failed
		return FALSE;
	}

	do
	{
		if( '.' != fd.cFileName[0] )
		{
			TCHAR szFoundFilePathName_existing[_MAX_PATH];
			_tcsncpy_s( szFoundFilePathName_existing, _MAX_PATH, szDirectoryPathName_existing, _TRUNCATE );
			_tcsncat_s( szFoundFilePathName_existing, _MAX_PATH, fd.cFileName, _TRUNCATE );

			TCHAR szFoundFilePathName_new[_MAX_PATH];
			_tcsncpy_s( szFoundFilePathName_new, _MAX_PATH, szDirectoryPathName_new, _TRUNCATE );
			_tcsncat_s( szFoundFilePathName_new, _MAX_PATH, fd.cFileName, _TRUNCATE );

			if( FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes )
			{	// Call CopyDirctory recursively if the target is directory
				if( !CopyDirectory( szFoundFilePathName_existing, szFoundFilePathName_new ) )
				{
					FindClose( hFind );
					return FALSE;
				}
			}
			else
			{	// Copy file
				if( !CopyFile( szFoundFilePathName_existing, szFoundFilePathName_new, FALSE ) )
				{
					FindClose( hFind );
					return FALSE;
				}
			}
		}
	} while( FindNextFile( hFind, &fd ) );

	FindClose( hFind );

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

	TCHAR szPsychoPyConfigDir[1024];
	TCHAR szAppDataDir[512];

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	BOOL found;
	size_t requiredSize;

	GetStartupInfo(&si);
	//si.dwFlags = STARTF_USESHOWWINDOW;
	//si.wShowWindow = SW_HIDE;

	GetCurrentDirectory(TCHAR_SIZE(szHomeDir), szHomeDir);
	GetEnvironmentVariable(_T("PATH"), szPath, TCHAR_SIZE(szPath));

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

	if(!found){
		MessageBox(NULL,_T("PsychoPy is not found in site-packages."),_T("Error"),MB_ICONERROR);
		return -1;
	}

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

	_stprintf_s(szCmd, _T("\"%s\\%s\\pythonw.exe\" -m psychopy.app.psychopyApp"), szWinPythonDir, szPythonDir);

	// Get full path of %APPDATA%\psychopy3 directory
	_tgetenv_s(&requiredSize, NULL, 0, _T("APPDATA"));
	if (requiredSize == 0){
		return -1;
	}
	else if (requiredSize >= TCHAR_SIZE(szAppDataDir)){
		return -1;
	}
	_tgetenv_s(&requiredSize, szAppDataDir, requiredSize, _T("APPDATA"));

	// Copy configuration to %APPDATA%\psychopy3
	_stprintf_s(szPsychoPyConfigDir, _T("%s\\psychopy3\\"), szAppDataDir);
	if (PathIsDirectory(_T(".\\psychopy3\\"))){
		CopyDirectory(_T(".\\psychopy3\\"),szPsychoPyConfigDir);
	}

	// Copy configuration to %APPDATA%\psychopy3
	_stprintf_s(szPsychoPyConfigDir, _T("%s\\psychopy3\\"), szAppDataDir);
	if (PathIsDirectory(_T(".\\psychopy3\\"))) {
		CopyDirectory(_T(".\\psychopy3\\"), szPsychoPyConfigDir);
	}


	// Start PsychoPy
	if(!CreateProcess(
			NULL, szCmd, NULL, NULL, FALSE,
			NULL, NULL, NULL, &si, &pi)){
		_stprintf_s(szTmp, _T("Could not start PortablePsychoPy.\n\ncommand:\n%s"), szCmd);
		MessageBox(NULL,szTmp,_T("Error"),MB_ICONERROR);
		return -1;
	}

	// Waiting...
	DWORD r = WaitForSingleObject( pi.hProcess, INFINITE ) ;

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

	MessageBox(NULL, _T("PortablePsychoPy is successfully closed."), _T("Information"), MB_ICONINFORMATION);
	return exitCode;

}