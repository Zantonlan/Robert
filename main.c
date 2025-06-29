#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

bool congratulate = true;
int screenWidth;
int screenHeight;
int dpiX;
int dpiY;

#define IMAGECACHESIZE 64

typedef struct imageStore {
	const char* key;
	HENHMETAFILE emf;
	struct imageStore* next;
} imageStore;

imageStore* imageCache[IMAGECACHESIZE] = {0};

unsigned int hash(const char* str) {
	unsigned int hash = 5381;
	while (*str) {
		hash = ((hash << 5) + hash) + *str++;
	}
	return hash % IMAGECACHESIZE;
}

void insertEMF(const char* name, HENHMETAFILE emf) {
	unsigned int index = hash(name);
	imageStore* entry = malloc(sizeof(imageStore));
	entry->key = _strdup(name);
	entry->emf = emf;
	entry->next = imageCache[index];
	imageCache[index] = entry;
}

HENHMETAFILE getEMF(const char* name) {
	unsigned int index = hash(name);
	imageStore* current = imageCache[index];
	while (current) {
		if (strcmp(current->key, name) == 0) {
			return current->emf;
		}
		current = current->next;
	}
	return NULL;
}

void freeEMFTable() {
	for (int i = 0; i < IMAGECACHESIZE; i++) {
		imageStore* current = imageCache[i];
		while (current) {
			imageStore* next = current->next;
			DeleteEnhMetaFile(current->emf);
			free((void*)current->key);
			free(current);
			current = next;
		}
		imageCache[i] = NULL;
	}
}

//Draw an EMF.
//Only to be called from WM_PAINT.
//HDC hdc: the hdc
//HWND hwnd: the hwnd
//const char* key: the key to load
//int x: the x position
//int y: the y position
//float sx: the x scale factor
//float sy: the y scale factor
void drawEMF(HDC hdc, HWND hwnd, const char* key, int x, int y, float sx, float sy) {
	HENHMETAFILE emf = getEMF(key);
	if (emf) {
		ENHMETAHEADER header = {0};
		header.iType = EMR_HEADER;
		GetEnhMetaFileHeader(emf, sizeof(header), &header);
		
		int width = (header.rclFrame.right - header.rclFrame.left) * dpiX / 2540;
		int height = (header.rclFrame.bottom - header.rclFrame.top) * dpiY / 2540;

		RECT rect = {
			.left = x,
			.top = y,
			.right = (int)(width*sx),
			.bottom = (int)(height*sy)
		};

		PlayEnhMetaFile(hdc, emf, &rect);
	}
}

HWND CreateButton(int x, int y, int sx, int sy, HWND parentWindow, HINSTANCE app, int id, LPCSTR text) {
	return CreateWindowEx(0, "BUTTON", text, WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON, x, y, sx, sy, parentWindow, (HMENU)id, app, NULL);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg)
	{
		case WM_DESTROY:
		{
			freeEMFTable();
			PostQuitMessage(0);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			drawEMF(hdc, hwnd, "background", 0, 0, .5, .2);

			int trapezoidpos[2] = {0, 0};
			POINT trapezoid[4] = {
				{trapezoidpos[1]+50, trapezoidpos[2]},
				{trapezoidpos[1]+150, trapezoidpos[2]},
				{trapezoidpos[1]+200, trapezoidpos[2]+100},
				{trapezoidpos[1], trapezoidpos[2]+100},
			};

			Polygon(hdc, trapezoid, 4);

			Rectangle(hdc, 0, 0, 10, 10);

			SetBkMode(hdc, TRANSPARENT);
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
		case WM_COMMAND:
		{
			int res;
			if (LOWORD(wparam) == 1) {
				if (congratulate) {
					res = MessageBoxA(hwnd, "Good job!", "Congratulations!", MB_YESNO | MB_ICONEXCLAMATION);
				} else {
					MessageBoxA(hwnd, "I'm still angry.", "...", MB_OK | MB_ICONSTOP);
					congratulate = true;
				}

				if (res == IDYES) {
					MessageBoxA(hwnd, "Thank you for accepting.", "Thank You", MB_OK | MB_ICONINFORMATION);
				} else if (res == IDNO) {
					MessageBoxA(hwnd, "Fine. I guess I won't congratulate you next time.", "Fine.", MB_OK | MB_ICONSTOP);
					congratulate = false;
				}
			} else if (LOWORD(wparam) == 2) {
				res = MessageBoxA(hwnd, "Are you sure you want to close Robert?", "Confirmation", MB_YESNO | MB_ICONQUESTION);
				if (res == IDYES) {
					PostQuitMessage(0);
				}
			}
			break;
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {0};
	HWND hwnd;
	MSG msg;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

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
		0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), //CW_USEDEFAULT for default window position, 640x480 default window size
		NULL, NULL, hInst, NULL);

	if(!hwnd) return 0;

	CreateButton(50, 50, 100, 30, hwnd, hInst, 1, "Click me!");
	CreateButton(screenWidth-100, screenHeight-30, 100, 30, hwnd, hInst, 2, "Exit");

	HDC screen = GetDC(NULL);
	dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	ReleaseDC(NULL, screen);

	HENHMETAFILE test = GetEnhMetaFileA("./assets/testdrawing.emf");
	if (!test) {
		MessageBox(hwnd, "Couldn't load test ENF!", "Error", MB_ICONERROR);
		return 0;
	}
	insertEMF("background", test);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
