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
	STARTUPINFO si;// �X�^�[�g�A�b�v���
	PROCESS_INFORMATION pi;// �v���Z�X���

	//STARTUPINFO �\���̂̓��e���擾 
	GetStartupInfo(&si);
	//�����Ȃ���ԂŋN��������ɂ́A
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
			NULL,					// ���s�\���W���[���̖��O
			szCmd,	// �R�}���h���C���̕�����
			NULL,					// �Z�L�����e�B�L�q�q
			NULL,					// �Z�L�����e�B�L�q�q
			FALSE,					// �n���h���̌p���I�v�V����
			NULL,					// �쐬�̃t���O 
									//CREATE_NEW_PROCESS_GROUP	: �V���ȃv���Z�X
			NULL,					// �V�������u���b�N
			NULL,					// �J�����g�f�B���N�g���̖��O
			&si,					// �X�^�[�g�A�b�v���
			&pi					// �v���Z�X���
			)){

		wsprintf(szTmp, _T("Could not start PortableIPython.\n\ncommand:\n%s"),szCmd);
		MessageBox(NULL,szTmp,_T("Error"),MB_ICONERROR);
	}

	return 0;

}