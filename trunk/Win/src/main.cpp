/* ****************************************************************************

 * EDT Project.
 * Copyright (C) 2010-2011 FedICT.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version
 * 3.0 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, see
 * http://www.gnu.org/licenses/.

**************************************************************************** */

#include "EDT.h"
#include "stdafx.h"
#include "main.h"
#include <string>
#include <Winuser.h>

#define MAX_LOADSTRING 100
#define COPY_BUTTON_HEIGHT 20

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
BOOL bg_AutoExit = FALSE;
HWND hg_textWnd = NULL;			//handle to the edit control

// Forward declarations of functions included in this code module:
DWORD WINAPI		EDTThreadFunction( LPVOID lpParam );
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE hInstance,int nCmdShow);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				ParseCommandChar(TCHAR cmdChar);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	if(lpCmdLine != NULL)
	{
		LPTSTR lpcmdchar = lpCmdLine;
		//DWORD cmdlineLen = wcslen(lpCmdLine);
		while(*lpcmdchar != '\0')
		{
			if(*lpcmdchar != L' ') 
			{
				TCHAR cmdChar = *lpcmdchar;
				ParseCommandChar(cmdChar);
			}
			lpcmdchar++;
		}
	}
	//lpCmdLine = GetCommandLine();

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_EDTNG, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	//Edit_LimitText(hg_textWnd, 999999);
	//EM_EXLIMITTEXT -> rich control 2

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDTNG));

	//Start EDT thread
	DWORD   dwThreadId;
	HANDLE  hThreadHandle; 

	// Create EDT thread
	hThreadHandle = CreateThread( 
		NULL,                   // default security attributes
		0,                      // use default stack size  
		EDTThreadFunction,		// thread function name
		hg_textWnd,				// argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);			// returns the thread identifier 



	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDTNG));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_EDTNG);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   HWND hCopyButton;

   hInst = hInstance; // Store instance handle in our global variable
   WCHAR EdtTitle[100];
   wsprintf(EdtTitle,L"EDT TOOL V%d.%d\n",EDT_VERSION_MAJOR,EDT_VERSION_MINOR);
   
   hWnd = CreateWindow(szWindowClass,EdtTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   
   if (!hWnd)
   {
      return FALSE;
   }

   hCopyButton = CreateWindow(L"BUTTON", L"<<< COPY ALL >>>", WS_CHILD | BS_PUSHBUTTON | BS_TEXT | BS_VCENTER,
   CW_USEDEFAULT, 0, 200, COPY_BUTTON_HEIGHT, hWnd, (HMENU)ID_COPYCONTROL, hInstance, NULL);
   if (!(hCopyButton))
   {
      return FALSE;
   }

   hg_textWnd = CreateWindow(L"EDIT", NULL, WS_CHILD | ES_MULTILINE | ES_LEFT ,
   0, COPY_BUTTON_HEIGHT, CW_USEDEFAULT, 0, hWnd, (HMENU)ID_EDITCONTROL, hInstance, NULL);

   if (!hg_textWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   ShowWindow(hg_textWnd, SW_SHOWNORMAL);
   ShowWindow(hCopyButton, SW_SHOWNORMAL);
   UpdateWindow(hWnd);
   return TRUE;
}


BOOL CALLBACK ChildWindowResize(HWND hWndChild, LPARAM lParam)
{
	LPRECT rcPar = (LPRECT)lParam;
	LONG childId = GetWindowLong(hWndChild,GWL_ID);

	switch(childId)
	{
	case ID_COPYCONTROL:
		MoveWindow(hWndChild,
			rcPar->left,
			rcPar->bottom - COPY_BUTTON_HEIGHT,
			rcPar->right - rcPar->left,
			COPY_BUTTON_HEIGHT,
			TRUE);
		break;
	case ID_EDITCONTROL:
		MoveWindow(hWndChild,
			rcPar->left,
			rcPar->top,
			rcPar->right - rcPar->left,
			rcPar->bottom - COPY_BUTTON_HEIGHT,
			TRUE);
		break;
	default:
		break;
	};
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rcClient;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch(wmEvent)
		{
		case BN_CLICKED:
			//copy button clicked, now select and copy all text present in the edit control

			if (OpenClipboard(hWnd) != 0)
			{
				if ( EmptyClipboard() != 0)
				{
					int textLen = GetWindowTextLength(hg_textWnd);
					DWORD memSize = (textLen + 1)* sizeof(wchar_t);
					HGLOBAL hglbMem = GlobalAlloc(GMEM_MOVEABLE,memSize);
					
					if(hglbMem != NULL)
					{
						LPTSTR lpstrCopy = (LPTSTR)GlobalLock(hglbMem);
						GetWindowText(hg_textWnd,lpstrCopy,memSize);
						GlobalUnlock(hglbMem);
						SetClipboardData(CF_UNICODETEXT,hglbMem);
					}
				}
				CloseClipboard();
			}
			break;
		default:
			// Parse the menu selections:
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_AUTO_EXIT:
				if(bg_AutoExit == TRUE)
					DestroyWindow(hWnd);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
		break;
	case WM_SIZE:
		GetClientRect(hWnd,&rcClient);
		EnumChildWindows(hWnd,ChildWindowResize, (LPARAM) &rcClient);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void ParseCommandChar(TCHAR cmdChar)
{
	switch(cmdChar)
	{
	case L'A':
	case L'a':
		bg_AutoExit = TRUE;
		break;
	};
}
