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
#define COPY_BUTTON_HEIGHT 30

// Global Variables:
HINSTANCE hInst;											// current instance
TCHAR szTitle[MAX_LOADSTRING];				// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// the main window class name
BOOL bg_AutoExit = FALSE;
BOOL bg_StartButton = FALSE;
HWND hg_textWnd = NULL;			//handle to the edit control
HWND hg_button = NULL;			//handle to the button

#define btnNameLog L"<<< SHOW LOGFILE >>>"
#define btnNameStart L"<<< START >>>"

// Forward declarations of functions included in this code module:
DWORD WINAPI		EDTThreadFunction( LPVOID lpParam );
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE hInstance,int nCmdShow);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				ParseCommandChar(TCHAR cmdChar);
void				StartEDTThread (void);

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

	if(bg_StartButton == FALSE)
	{
		StartEDTThread();
	}
	else
	{
		SendMessage(hg_button, WM_SETTEXT,0, (LPARAM)btnNameStart); // for Win32 windows
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Gelieve te verifiëren of er :\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - een kaartlezer is aangesloten;\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - een eID-kaart in de kaartlezer aanwezig is.\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Druk op de startknop hier beneden om te starten\r\n\r\n\r\n");

		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Veuillez vérifier si :\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - un lecteur de cartes est connecté ;\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - une carte eID est insérée dans le lecteur de cartes.\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Appuyez sur le bouton « Start » ci-dessous afin de lancer l'enregistrement.\r\n\r\n\r\n");

		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Bitte überprüfen Sie, ob :\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - ein Kartenleser angeschlossen ist;\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - eine eID-Karte in den Kartenleser eingeschoben ist.\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Drücken Sie die Schaltfläche \"Start\" unten um die Aufzeichnung zu starten.\r\n\r\n\r\n");

		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Please verify if :\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - a cardreader is connected;\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"   - an eID card is inserted into the card reader.\r\n");
		SendMessage(hg_textWnd, EM_REPLACESEL,0,  (LPARAM)L"Press the ‘Start’ button below to start the logging.\r\n");
	}


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

   hInst = hInstance; // Store instance handle in our global variable
   WCHAR EdtTitle[100];
   wsprintf(EdtTitle,L"EDT TOOL V%d.%d\n",EDT_VERSION_MAJOR,EDT_VERSION_MINOR);
   
   hWnd = CreateWindow(szWindowClass,EdtTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   
   if (!hWnd)
   {
      return FALSE;
   }

   hg_button = CreateWindow(L"BUTTON", btnNameLog, WS_CHILD | BS_PUSHBUTTON | BS_TEXT | BS_VCENTER,
   CW_USEDEFAULT, 0, 200, COPY_BUTTON_HEIGHT, hWnd, (HMENU)ID_BUTTON, hInstance, NULL);
   if (!(hg_button))
   {
      return FALSE;
   }
   hg_textWnd = CreateWindow(L"EDIT", NULL,  WS_CHILD | ES_MULTILINE | ES_LEFT | WS_VSCROLL | WS_HSCROLL ,
   0, COPY_BUTTON_HEIGHT, CW_USEDEFAULT, 0, hWnd, (HMENU)ID_EDITCONTROL, hInstance, NULL);

   if (!hg_textWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   ShowWindow(hg_textWnd, SW_SHOWNORMAL);
   ShowWindow(hg_button, SW_SHOWNORMAL);
   UpdateWindow(hWnd);
   return TRUE;
}


BOOL CALLBACK ChildWindowResize(HWND hWndChild, LPARAM lParam)
{
	LPRECT rcPar = (LPRECT)lParam;
	LONG childId = GetWindowLong(hWndChild,GWL_ID);

	switch(childId)
	{
	case ID_BUTTON:
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

		//copy button clicked, now select and copy all text present in the edit control
		/*if (OpenClipboard(hWnd) != 0)
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
		}*/

		// Parse the menu selections:
		switch (wmId)
		{
		case ID_BUTTON:
			if( wmEvent== BN_CLICKED)
			{
				if(bg_StartButton == TRUE)
				{
					StartEDTThread();
					bg_StartButton = FALSE;
					SendMessage(hg_button, WM_SETTEXT,0, (LPARAM)btnNameLog);
					SendMessage(hg_textWnd, WM_SETTEXT,0,  (LPARAM)L"");
				}else
				{
					logShowLogFile();
				}
			}
			break;
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
	case WM_SIZE:
		GetClientRect(hWnd,&rcClient);
		EnumChildWindows(hWnd,ChildWindowResize, (LPARAM) &rcClient);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// Add any drawing code here...
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
	case L'S':
	case L's':
		bg_StartButton = TRUE;
		break;
	};
}

void StartEDTThread (void)
{
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
}
