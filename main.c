#include <windows.h>
#include <stdio.h>

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			int trapezoidpos[2] = {0, 0};
			POINT trapezoid[4] = {
				{trapezoidpos[1]+50, trapezoidpos[2]},
				{trapezoidpos[1]+150, trapezoidpos[2]},
				{trapezoidpos[1]+200, trapezoidpos[2]+100},
				{trapezoidpos[1], trapezoidpos[2]+100},
			};

			Polygon(hdc, trapezoid, 4);

			Rectangle(hdc, 0, 0, 10, 10);

			TextOut(hdc, 10, 110, "Hey!", strlen("Hey!"));
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wparam;
			RECT rc;
			GetClientRect(hwnd, &rc);
			FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
			return 1;
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc = {0};
	HWND hwnd;
	MSG msg;

	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = "Classy";
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	if(!RegisterClassEx(&wc)) return 0;

	hwnd = CreateWindowExA(0,
		"Classy",
		"Robert",
		WS_POPUP, //WS_OVERLAPPEDWINDOW for reg. window
		0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CXSCREEN), //CW_USEDEFAULT for default window position, 640x480 default window size
		NULL, NULL, hInst, NULL);

	if(!hwnd) return 0;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
