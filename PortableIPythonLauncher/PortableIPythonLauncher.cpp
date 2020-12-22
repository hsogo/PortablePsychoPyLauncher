#include <windows.h>
#include <tchar.h>

int APIENTRY _tWinMain(
    HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    LPTSTR lpCmdLine, 
    int nCmdShow)
{
	TCHAR szTmp[1920];
	TCHAR szCmd[2048];
	TCHAR szPortablePythonDir[128];
	STARTUPINFO si;// スタートアップ情報
	PROCESS_INFORMATION pi;// プロセス情報

	//STARTUPINFO 構造体の内容を取得 
	GetStartupInfo(&si);
	//見えない状態で起動させるには、
	//si.dwFlags = STARTF_USESHOWWINDOW;
	//si.wShowWindow = SW_HIDE;

	WIN32_FIND_DATA ffd;
	bool found = false;
	HANDLE h = FindFirstFile(_T(".\\*.*"), &ffd);
	if ( h != INVALID_HANDLE_VALUE ) {
		do {
			if(wcsncmp(_T("PortablePython"), ffd.cFileName,14)==0 ||
				wcsncmp(_T("Portable Python "), ffd.cFileName,16)==0){
				wcscpy_s(szPortablePythonDir, ffd.cFileName);
				found = true;
				break;
			}
		}while(FindNextFile(h, &ffd));
		FindClose(h);
	}

	if(!found){
		MessageBox(NULL,_T("Portable Python is not found in this directory."),_T("Error"),MB_ICONERROR);
		return 0;
	}

	wsprintf(szCmd,_T("\"%s\\App\\python.exe\" \"%s\\App\\Scripts\\ipython-script.py\""),szPortablePythonDir,szPortablePythonDir);

	if(!CreateProcess(
			NULL,					// 実行可能モジュールの名前
			szCmd,	// コマンドラインの文字列
			NULL,					// セキュリティ記述子
			NULL,					// セキュリティ記述子
			FALSE,					// ハンドルの継承オプション
			NULL,					// 作成のフラグ 
									//CREATE_NEW_PROCESS_GROUP	: 新たなプロセス
			NULL,					// 新しい環境ブロック
			NULL,					// カレントディレクトリの名前
			&si,					// スタートアップ情報
			&pi					// プロセス情報
			)){

		wsprintf(szTmp, _T("Could not start PortableIPython.\n\ncommand:\n%s"),szCmd);
		MessageBox(NULL,szTmp,_T("Error"),MB_ICONERROR);
	}

	return 0;

}