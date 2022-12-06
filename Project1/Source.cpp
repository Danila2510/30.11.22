#include <windows.h>
#include <tchar.h>
#include <string>
#include <commctrl.h>
#include <tlhelp32.h>
#include <time.h>
#include "resource.h"

#pragma comment(lib,"comctl32")

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

HWND hReload;
HWND hTerminate;
HWND hNew;
HWND hProcName;
HWND hProcList;
HWND hStatID;
HWND hTimeDate;
HANDLE Th1;

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX) };
	icc.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icc);

	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}

void ShowProcList(HWND hList)
{
	EnableWindow(hTerminate, FALSE);
	EnableWindow(hNew, TRUE);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processInfo = { sizeof(PROCESSENTRY32) };

	BOOL res = Process32First(hSnapshot, &processInfo);

	if (res == TRUE) {
		do {
			LRESULT index = SendMessage(hList, LB_ADDSTRING, 0, LPARAM(processInfo.szExeFile));
			SendMessage(hList, LB_SETITEMDATA, WPARAM(index), LPARAM(processInfo.th32ProcessID));
		} while (Process32Next(hSnapshot, &processInfo));
	}
	CloseHandle(hSnapshot);
}

DWORD WINAPI Thread1(LPVOID lp)
{
	while (true)
	{
		SendMessage(hProcList, LB_RESETCONTENT, 0, 0);
		ShowProcList(hProcList);
		Sleep(5000);
	}
	return 0;
}


BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp)
{
	static time_t t;
	static TCHAR str[100];
	switch (mes)
	{
	case WM_INITDIALOG:
	{	
		SetTimer(hWnd, 1, 1000, NULL);
		Th1 = CreateThread(NULL, 0, Thread1, hWnd, 0, NULL);
		hTimeDate = GetDlgItem(hWnd, IDC_EDIT3);
		hReload = GetDlgItem(hWnd, IDC_BUTTON1);
		hTerminate = GetDlgItem(hWnd, IDC_BUTTON2);
		hNew = GetDlgItem(hWnd, IDC_BUTTON3);
		hProcName = GetDlgItem(hWnd, IDC_EDIT1);
		hProcList = GetDlgItem(hWnd, IDC_LIST1);
		hStatID = GetDlgItem(hWnd, IDC_EDIT2);
	   	ShowProcList(hProcList);
	}
	return TRUE;

	case WM_COMMAND:
	{
		if (LOWORD(wp) == IDC_BUTTON1) {
			SendMessage(hProcList, LB_RESETCONTENT, 0, 0);
			ShowProcList(hProcList);
		}
		else if (LOWORD(wp) == IDC_BUTTON2) {
			LRESULT index = SendMessage(hProcList, LB_GETCURSEL, 0, 0);
			int procId = SendMessage(hProcList, LB_GETITEMDATA, WPARAM(index), 0);
			HANDLE procHandle = OpenProcess(PROCESS_TERMINATE, TRUE, procId);
			TerminateProcess(procHandle, 0);
			SendMessage(hProcList, LB_RESETCONTENT, 0, 0);
			ShowProcList(hProcList);
			CloseHandle(procHandle);		
		}
		else if (LOWORD(wp) == IDC_BUTTON3) 
		{
			wchar_t buf[260]{ 0 };
			GetWindowText(hProcName, buf, 260);
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			if (CreateProcess(buf, buf, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
			{
			   ShowProcList(hProcList);
			}	
		}
		else if (HIWORD(wp) == LBN_SELCHANGE) 
		{
			EnableWindow(hTerminate, TRUE);
			LRESULT index = SendMessage(hProcList, LB_GETCURSEL, 0, 0);
			int procId = SendMessage(hProcList, LB_GETITEMDATA, WPARAM(index), 0);
			TCHAR buf[30];
			wsprintf(buf, TEXT("%d"), procId);
			SendMessage(hStatID, WM_SETTEXT, 0, LPARAM(buf));			
		}
	}
	break;
	case WM_TIMER:
		t = time(NULL);
		lstrcpy(str, _tctime(&t));
		str[lstrlen(str) - 1] = '\0';
		SendMessage(hTimeDate, WM_SETTEXT, 0, LPARAM(str));
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		KillTimer(hWnd, 1);
		EndDialog(hWnd, 0);
		return TRUE;
	}
	return FALSE;
}