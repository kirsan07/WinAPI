?#include<Windows.h>
#include<Richedit.h>
#include"resource.h"

CONST CHAR g_sz_CLASS_NAME[] = "My Text editor";

INT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT CALLBACK DlgProcAbout(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL LoadTextFileToEdit(HWND hEdit, LPCSTR lpszFileName);
BOOL SaveTextFileFromEdit(HWND hEdit, LPCSTR lpszFileName);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(wc);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.style = 0;

	wc.hIcon = (HICON)LoadImage(hInstance, "ICO\\pencil_editor.ico", IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_LOADFROMFILE);
	wc.hIconSm = (HICON)LoadImage(hInstance, "ICO\\pencil_editor.ico", IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_LOADFROMFILE);
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

	wc.hInstance = hInstance;
	wc.lpszClassName = g_sz_CLASS_NAME;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Class registration failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	HWND hwnd = CreateWindowEx
	(
		NULL,
		g_sz_CLASS_NAME,
		g_sz_CLASS_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window creation failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

INT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CHAR szFileName[MAX_PATH]{};
	switch (uMsg)
	{
	case WM_CREATE:
	{
		LoadLibrary("riched20.dll");
		RECT rect;
		GetWindowRect(hwnd, &rect);
		HWND hEdit = CreateWindowEx
		(
			NULL, RICHEDIT_CLASS, "",
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			0, 0,
			rect.right, rect.bottom,
			hwnd,
			(HMENU)IDC_EDIT,
			GetModuleHandle(NULL),
			NULL
		);
	}
	break;
	case WM_SIZE:
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		SetWindowPos(GetDlgItem(hwnd, IDC_EDIT), NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_NOZORDER);
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_OPEN:
		{
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));


			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Text files: (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = "txt";

			if (GetOpenFileName(&ofn))
			{
				HWND hEdit = GetDlgItem(hwnd, IDC_EDIT);
				LoadTextFileToEdit(hEdit, szFileName);
			}
		}
		break;
		case ID_FILE_SAVE:
			if (strlen(szFileName))SaveTextFileFromEdit(GetDlgItem(hwnd, IDC_EDIT), szFileName);
			else SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVEAS, 0);
			break;
		case ID_FILE_SAVEAS:
		{

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = "Text files: (*.txt)\0*.txt\0All files (*.*)\0:*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = "txt";

			if (GetSaveFileName(&ofn))SaveTextFileFromEdit(GetDlgItem(hwnd, IDC_EDIT), szFileName);
		}
		break;
		case ID_HELP_ABOUT:
			DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_ABOUT), hwnd, DlgProcAbout, 0);
			break;
		}
	}
	break;
	case WM_DESTROY:PostQuitMessage(0);	break;
	case WM_CLOSE:	DestroyWindow(hwnd); break;
	default:		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return FALSE;
}
INT CALLBACK DlgProcAbout(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:EndDialog(hwnd, 0);
		}
	case WM_CLOSE:EndDialog(hwnd, 0);
	}
	return FALSE;
}

BOOL LoadTextFileToEdit(HWND hEdit, LPCSTR lpszFileName)
{
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != UINT_MAX)
		{
			LPSTR lpszFileText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
			if (lpszFileText)
			{
				DWORD dwRead;
				if (ReadFile(hFile, lpszFileText, dwFileSize, &dwRead, NULL))
				{
					lpszFileText[dwFileSize] = 0;
					if (SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)lpszFileText))bSuccess = TRUE;
				}
				GlobalFree(hFile);
			}
			CloseHandle(hFile);
		}
	}
	return bSuccess;
}
BOOL SaveTextFileFromEdit(HWND hEdit, LPCSTR lpszFileName)
{
	BOOL bSuccess = FALSE;
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwTextLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
		if (dwTextLength > 0)
		{
			LPSTR lpszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength);
			if (lpszText)
			{
				if (SendMessage(hEdit, WM_GETTEXT, dwTextLength, (LPARAM)lpszText))
				{
					DWORD dwWritten;
					if (WriteFile(hFile, lpszText, dwTextLength, &dwWritten, 0))bSuccess = TRUE;
				}
				GlobalFree(lpszText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}