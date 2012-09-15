// local_supervisor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "local_supervisor.h"
#include "TCPLineServer.h"
#include <string>
#include <map>
#include "ProcessConfigFile.h"
#include "TokenizeLine.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	ProcessConfigFile configFile("processes.cfg");
	std::map<unsigned int, Process_Type> processMap;

	configFile.ReadProcesses(&processMap);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	ZeroMemory(&msg, sizeof(msg));

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_LOCAL_SUPERVISOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LOCAL_SUPERVISOR));
	
	/*STARTUPINFO siStartupInfo; 
    PROCESS_INFORMATION piProcessInfo; 
    memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
    memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
    siStartupInfo.cb = sizeof(siStartupInfo);

	LPTSTR lpCurrentDirectory = TEXT("C:\\Users\\Niclas\\Documents\\elektronik");
	LPTSTR lpCommandLine = TEXT("\"C:\\Users\\Niclas\\Documents\\elektronik\\setup_hid_viewer.exe\"");

	CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE, NULL, NULL, lpCurrentDirectory, &siStartupInfo, &piProcessInfo);*/

	WSADATA WsaDat;
	if(WSAStartup(MAKEWORD(2,2),&WsaDat)!=0)
	{
		MessageBox(NULL, TEXT("Error!"), TEXT("WSA Initialization failed!"), MB_OK);
		WSACleanup();
		exit(1);
	}

	TCPLineServer server(5666);

	// Main message loop:
	do
	{
		/*DWORD exitCode;

		//GetExitCodeProcess(piProcessInfo.hProcess, &exitCode);

		if(exitCode != STILL_ACTIVE) {
			MessageBoxA(NULL, "Died!", "Died", MB_OK);
			break;
		}*/

		int error;

		if((error = server.Update()) != 0) {
			if(error == -1) {
				MessageBox(NULL, TEXT("Error in TCPLineServer::Update()!"), TEXT("Error!"), MB_OK);
			} else if(error == -2) {
				MessageBox(NULL, TEXT("Message corrupted due to buffer overrun!"), TEXT("Warning!"), MB_OK);
			} else if(error == -3) {
				MessageBox(NULL, TEXT("What happened now?"), TEXT("Warning!"), MB_OK);
			}
		}

		while(server.HasData()) {
			std::string message = server.Get();
			std::vector<std::string> tokens = TokenizeLine(message, ';');
			if(tokens[0] == "start") {
				unsigned int id = atoi(tokens[1].c_str());
				STARTUPINFO siStartupInfo;
				PROCESS_INFORMATION piProcessInfo;
				memset(&siStartupInfo, 0, sizeof(siStartupInfo));
				memset(&piProcessInfo, 0, sizeof(piProcessInfo));
				siStartupInfo.cb = sizeof(siStartupInfo);

				/* MSDN: The Unicode version of CreateProcess can modify the contents of lpCommandLine.
				 * Therefore, this parameter cannot be a pointer to read-only memory (such as a const variable or a
				 * literal string). If this parameter is a constant string, the function may cause an access violation.
				 */
				char* commandLine = new char[processMap[id].commandLine.length()+1];
				strncpy(commandLine, processMap[id].commandLine.c_str(), processMap[id].commandLine.length()+1);
				CreateProcess(NULL, commandLine, NULL, NULL, FALSE, NULL, NULL, processMap[id].currentDir.c_str(), &siStartupInfo, &piProcessInfo);
			}
		}

		while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if (!TranslateAccelerator( 
					msg.hwnd,  // handle to receiving window 
                    hAccelTable,    // handle to active accelerator table 
                    &msg))         // message data 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
		}
	} while (WM_QUIT != msg.message);

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOCAL_SUPERVISOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_LOCAL_SUPERVISOR);
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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      100, 100, 400, 200, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
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
