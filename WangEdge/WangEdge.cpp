#include "stdafx.h"
#include "WangEdge.h"
#include <algorithm>
#include <math.h>
#include <time.h>
#include <vector>
#include <deque>
#include <bitset>
#include <ObjIdl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
unsigned TILE_S;
unsigned MAP_S;
struct point { long x; long y; bool v; };
enum DIR { UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3 };
enum COR { UR = 0, DR = 1, DL = 2, UL = 3 };
struct EdgeInfo { std::bitset<8> fixed; std::bitset<8> edge; };

class Input {
protected:
	Gdiplus::Bitmap* bitmaps[49];
public:
	virtual void Reset() {}
	virtual void UpdateImage(EdgeInfo&, POINT) {}
};
class EdgeInput : public Input {
public:
	EdgeInput() { for (unsigned i = 0; i < 16; ++i) bitmaps[i] = new Gdiplus::Bitmap(TILE_S, TILE_S); Reset(); }
	virtual void Reset();
	virtual void UpdateImage(EdgeInfo&, POINT);
};
class CornerInput : public Input {
public:
	CornerInput() { for (unsigned i = 0; i < 16; ++i) bitmaps[i] = new Gdiplus::Bitmap(TILE_S, TILE_S); Reset(); }
	virtual void Reset();
	virtual void UpdateImage(EdgeInfo&, POINT);
};
class BlobInput : public Input {
public:
	BlobInput() { for (unsigned i = 0; i < 49; ++i) bitmaps[i] = new Gdiplus::Bitmap(TILE_S, TILE_S); Reset(); }
	virtual void Reset();
	virtual void UpdateImage(EdgeInfo&, POINT);
};
class Wave {
protected:
	std::vector<std::vector<EdgeInfo>> w;
	Wave() : w(MAP_S, std::vector<EdgeInfo>(MAP_S)) {}
public:
	virtual void Reset() {}
	virtual bool Observe() { return false; }
	friend bool operator<(POINT, POINT);
};
class EdgeWave : public Wave {
	std::deque<point> next;
	virtual void Reset();
	virtual bool Observe();
public:
	EdgeWave() : Wave() { Reset(); }
};
class CornerWave : public Wave {
	std::deque<POINT> next;
	virtual void Reset();
	virtual bool Observe();
public:
	CornerWave() : Wave() { Reset(); }
};
class BlobWave : public Wave {
	std::deque<POINT> next;
	virtual void Reset();
	virtual bool Observe();
public:
	BlobWave() : Wave() { Reset(); }
};

Gdiplus::Bitmap* edgebmp;
Gdiplus::Bitmap* cornerbmp;
Gdiplus::Bitmap* blobbmp;
Gdiplus::Bitmap* output;
Input* input;
Wave* wave;
unsigned tilemapID;
#define IDM_SAVE 140
#define IDC_XEDIT 141
#define IDC_YEDIT 142
#define MAX_LOADSTRING 100
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	Gdiplus::GdiplusStartupInput gInput;
	ULONG_PTR gToken;
	Gdiplus::GdiplusStartup(&gToken, &gInput, nullptr);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WANGEDGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow)) return FALSE;
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WANGEDGE));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
	Gdiplus::GdiplusShutdown(gToken);
    return (int) msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WANGEDGE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WANGEDGE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
   hInst = hInstance;
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd) return FALSE;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

bool operator~(POINT p) { return (p.x >= 0 && p.x < long(MAP_S ) && p.y >= 0 && p.y < long(MAP_S)); }
bool operator<(point p0, point p1) {
	double x0 = p0.x + (p0.v ? 0 : 0.5);
	double y0 = p0.y + (p0.v ? 0.5 : 0);
	double x1 = p1.x + (p1.v ? 0 : 0.5);
	double y1 = p1.y + (p1.v ? 0.5 : 0);
	double d0 = sqrt((MAP_S / 2 - x0)*(MAP_S / 2 - x0) + (MAP_S / 2 - y0)*(MAP_S / 2 - y0));
	double d1 = sqrt((MAP_S / 2 - x1)*(MAP_S / 2 - x1) + (MAP_S / 2 - y1)*(MAP_S / 2 - y1));
	return d0 > d1;
}
bool operator<(POINT p0, POINT p1) {
	double d0 = sqrt((MAP_S / 2 - 1 - p0.x)*(MAP_S / 2 - 1 - p0.x) + (MAP_S / 2 - 1 - p0.y)*(MAP_S / 2 - 1 - p0.y));
	double d1 = sqrt((MAP_S / 2 - 1 - p1.x)*(MAP_S / 2 - 1 - p1.x) + (MAP_S / 2 - 1 - p1.y)*(MAP_S / 2 - 1 - p1.y));
	for (POINT dir : {POINT{ 0,-1 }, POINT{ -1,0 }, POINT{ 0,1 }, POINT{ 1,0 }}) {
		if (~POINT{ p0.x + dir.x,p0.y + dir.y })
			if (wave->w[p0.x + dir.x][p0.y + dir.y].fixed[DR] && wave->w[p0.x + dir.x][p0.y + dir.y].edge[DR])
				d0 *= 1.3;
		if (~POINT{ p1.x + dir.x,p1.y + dir.y })
			if (wave->w[p1.x + dir.x][p1.y + dir.y].fixed[DR] && wave->w[p1.x + dir.x][p1.y + dir.y].edge[DR])
				d1 *= 1.3;
	}
	return d0 > d1;
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void EdgeInput::Reset() {
	unsigned xs[16] = { 0,0,1,1,0,0,1,1,3,3,2,2,3,3,2,2 };
	unsigned ys[16] = { 3,2,3,2,0,1,0,1,3,2,3,2,0,1,0,1 };
	unsigned i, x, y, mx, my; Gdiplus::Color c; int r, g, b;
	for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
		r = g = b = 0;
		for (i = 0; i < 16; ++i) {
			edgebmp->GetPixel(TILE_S*4*(tilemapID % 8) + TILE_S * xs[i] + x, TILE_S*4*(tilemapID / 8) + TILE_S * ys[i] + y, &c);
			bitmaps[i]->SetPixel(x, y, c);
			r += c.GetR();
			g += c.GetG();
			b += c.GetB();
		}
		for (my = 0; my < MAP_S; ++my) for (mx = 0; mx < MAP_S; ++mx)
			output->SetPixel(TILE_S*mx + x, TILE_S*my + y, Gdiplus::Color(BYTE(r / 16), BYTE(g / 16), BYTE(b / 16)));
	}
}
void EdgeInput::UpdateImage(EdgeInfo& state, POINT p) {
	if (!~p) return;
	unsigned x, y; Gdiplus::Color c;
	if (state.fixed.all()) {
		unsigned long i = state.edge.to_ulong();
		for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
			bitmaps[i]->GetPixel(x, y, &c);
			output->SetPixel(TILE_S*p.x + x, TILE_S*p.y + y, c);
		}
		return;
	}
	int r, g, b;
	if (state.fixed.none()) {
		unsigned i;
		for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
			r = g = b = 0;
			for (i = 0; i < 16; ++i) {
				bitmaps[i]->GetPixel(x, y, &c);
				r += c.GetR();
				g += c.GetG();
				b += c.GetB();
			}
			output->SetPixel(TILE_S*p.x + x, TILE_S*p.y + y, Gdiplus::Color(BYTE(r / 16), BYTE(g / 16), BYTE(b / 16)));
		}
		return;
	}
	int count; bool tf[2] = { true, false };
	for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
		r = g = b = count = 0;
		for (bool up : tf) if (!state.fixed[UP] || (state.edge[UP] == up))
			for (bool right : tf) if (!state.fixed[RIGHT] || (state.edge[RIGHT] == right))
				for (bool down : tf) if (!state.fixed[DOWN] || (state.edge[DOWN] == down))
					for (bool left : tf) if (!state.fixed[LEFT] || (state.edge[LEFT] == left)) {
						bitmaps[up*int(pow(float(2), UP)) + right*int(pow(float(2), RIGHT)) + down*int(pow(float(2), DOWN)) + left*int(pow(float(2), LEFT))]->GetPixel(x, y, &c);
						r += c.GetR();
						g += c.GetG();
						b += c.GetB();
						++count;
					}
		output->SetPixel(TILE_S*p.x + x, TILE_S*p.y + y, Gdiplus::Color(BYTE(r / count), BYTE(g / count), BYTE(b / count)));
	}
}
void CornerInput::Reset() {
	unsigned bxs[18] = { 1,2,3,0,1,2,3,0,1,2,3,0,1,2,3,0,1,2 };
	unsigned bys[18] = { 0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4 };
	unsigned xs[16] = { 0,0,1,1,0,2,3,1,3,1,0,2,3,3,2,2 };
	unsigned ys[16] = { 3,2,3,0,0,3,0,1,3,2,1,2,2,1,0,1 };
	unsigned x, y, mx, my; Gdiplus::Color c; int r, g, b;
	for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
		r = g = b = 0;
		for (unsigned i = 0; i < 16; ++i) {
			cornerbmp->GetPixel(TILE_S * 4 * bxs[tilemapID] + TILE_S*xs[i] + x, TILE_S * 4 * bys[tilemapID] + TILE_S*ys[i] + y, &c);
			bitmaps[i]->SetPixel(x, y, c);
			r += c.GetR();
			g += c.GetG();
			b += c.GetB();
		}
		c = Gdiplus::Color(BYTE(r /= 16), BYTE(g /= 16), BYTE(b /= 16));
		for (my = 0; my < MAP_S; ++my) for (mx = 0; mx < MAP_S; ++mx)
			output->SetPixel(TILE_S*mx + x, TILE_S*my + y, c);
	}
}
void CornerInput::UpdateImage(EdgeInfo& state, POINT p) {
	if (!~p) return;
	unsigned x, y; Gdiplus::Color c;
	if (state.fixed.all()) {
		unsigned whichbmp = unsigned(state.edge.to_ulong());
		for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
			bitmaps[whichbmp]->GetPixel(x, y, &c);
			output->SetPixel(p.x*TILE_S + x, p.y*TILE_S + y, c);
		}
		return;
	}
	int r, g, b, count; bool t[2] = { false, true };
	if (state.fixed.none()) for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
		r = g = b = 0;
		for (int i = 0; i < 16; ++i) {
			bitmaps[i]->GetPixel(x, y, &c);
			r += c.GetR();
			g += c.GetG();
			b += c.GetB();
		}
		output->SetPixel(TILE_S*p.x + x, TILE_S*p.y + y, Gdiplus::Color(BYTE(r / 16), BYTE(g / 16), BYTE(b / 16)));
	}
	else for (y = 0; y < TILE_S; ++y) for (x = 0; x < TILE_S; ++x) {
		r = g = b = count = 0;
		for (bool ur : t) if (!state.fixed[UR] || (state.edge[UR] == ur))
			for (bool dr : t) if (!state.fixed[DR] || (state.edge[DR] == dr))
				for (bool dl : t) if (!state.fixed[DL] || (state.edge[DL] == dl))
					for (bool ul : t) if (!state.fixed[UL] || (state.edge[UL] == ul)) {
						bitmaps[ur*int(pow(float(2), UR)) + dr*int(pow(float(2), DR)) + dl*int(pow(float(2), DL)) + ul*int(pow(float(2), UL))]->GetPixel(x, y, &c);
						r += c.GetR();
						g += c.GetG();
						b += c.GetB();
						++count;
					}
		output->SetPixel(TILE_S*p.x + x, TILE_S*p.y + y, Gdiplus::Color(BYTE(r / count), BYTE(g / count), BYTE(b / count)));
	}
}
void BlobInput::Reset() { //not done
	unsigned xs[7];
}
void BlobInput::UpdateImage(EdgeInfo& state, POINT p) { //not done

}

void EdgeWave::Reset() {
	next.clear();
	for (long y = 0; y < long(MAP_S); ++y) for (long x = 0; x < long(MAP_S); ++x) {
		next.push_back({ x,y,true });
		next.push_back({ x,y,false });
		w[x][y] = { std::bitset<8>(),std::bitset<8>() };
	}
}
bool EdgeWave::Observe() {
	if (next.empty()) return false;
	std::make_heap(next.begin(), next.end());
	point p = next.front();
	next.pop_front();
	bool r = (rand() & 1);
	if (p.v) {
		w[p.x][p.y].edge.set(DOWN, r);
		w[p.x][p.y].fixed.set(DOWN);
		if (~POINT{ p.x,p.y + 1 }) {
			w[p.x][p.y + 1].edge.set(UP, r);
			w[p.x][p.y + 1].fixed.set(UP);
			input->UpdateImage(w[p.x][p.y + 1], POINT{ p.x,p.y + 1 });
		}
		else {
			w[p.x][0].edge.set(UP, r);
			w[p.x][0].fixed.set(UP);
			input->UpdateImage(w[p.x][0], POINT{ p.x,0 });
		}
	}
	else {
		w[p.x][p.y].edge.set(RIGHT, r);
		w[p.x][p.y].fixed.set(RIGHT);
		if (~POINT{ p.x + 1,p.y }) {
			w[p.x + 1][p.y].edge.set(LEFT, r);
			w[p.x + 1][p.y].fixed.set(LEFT);
			input->UpdateImage(w[p.x + 1][p.y], POINT{ p.x + 1,p.y });
		}
		else {
			w[0][p.y].edge.set(LEFT, r);
			w[0][p.y].fixed.set(LEFT);
			input->UpdateImage(w[0][p.y], POINT{ 0,p.y });
		}
	}
	input->UpdateImage(w[p.x][p.y], POINT{ p.x,p.y });
	return true;
}
void CornerWave::Reset() {
	next.clear();
	for (int y = 0; y < int(MAP_S); ++y) for (int x = 0; x < int(MAP_S); ++x) {
		w[x][y] = { std::bitset<8>(), std::bitset<8>() };
		next.push_back(POINT{ x,y });
	}
}
bool CornerWave::Observe() {
	if (next.empty()) return false;
	std::make_heap(next.begin(), next.end());
	std::pop_heap(next.begin(), next.end());
	POINT p = next.back();
	next.pop_back();
	bool r = (rand() & 1);
	w[p.x][p.y].edge.set(DR, r);
	w[p.x][p.y].fixed.set(DR);
	input->UpdateImage(w[p.x][p.y], p);
	if (~POINT{ p.x + 1, p.y }) {
		w[p.x + 1][p.y].edge.set(DL, r);
		w[p.x + 1][p.y].fixed.set(DL);
		input->UpdateImage(w[p.x + 1][p.y], POINT{ p.x + 1,p.y });
		if (~POINT{ p.x, p.y + 1 }) {
			w[p.x][p.y + 1].edge.set(UR, r);
			w[p.x][p.y + 1].fixed.set(UR);
			input->UpdateImage(w[p.x][p.y + 1], POINT{ p.x,p.y + 1 });
			w[p.x + 1][p.y + 1].edge.set(UL, r);
			w[p.x + 1][p.y + 1].fixed.set(UL);
			input->UpdateImage(w[p.x + 1][p.y + 1], POINT{ p.x + 1,p.y + 1 });
		}
		else {
			w[p.x][0].edge.set(UR, r);
			w[p.x][0].fixed.set(UR);
			input->UpdateImage(w[p.x][0], POINT{ p.x,0 });
			w[p.x + 1][0].edge.set(UL, r);
			w[p.x + 1][0].fixed.set(UL);
			input->UpdateImage(w[p.x + 1][0], POINT{ p.x + 1,0 });
		}
	}
	else {
		w[0][p.y].edge.set(DL, r);
		w[0][p.y].fixed.set(DL);
		input->UpdateImage(w[0][p.y], POINT{ 0,p.y });
		if (~POINT{ p.x, p.y + 1 }) {
			w[p.x][p.y + 1].edge.set(UR, r);
			w[p.x][p.y + 1].fixed.set(UR);
			input->UpdateImage(w[p.x][p.y + 1], POINT{ p.x,p.y + 1 });
			w[0][p.y + 1].edge.set(UL, r);
			w[0][p.y + 1].fixed.set(UL);
			input->UpdateImage(w[0][p.y + 1], POINT{ 0,p.y + 1 });
		}
		else {
			w[p.x][0].edge.set(UR, r);
			w[p.x][0].fixed.set(UR);
			input->UpdateImage(w[p.x][0], POINT{ p.x,0 });
			w[0][0].edge.set(UL, r);
			w[0][0].fixed.set(UL);
			input->UpdateImage(w[0][0], POINT{ 0,0 });
		}
	}
	return true;
}
void BlobWave::Reset() { //done
	next.clear();
	for (int y = 0; y < int(MAP_S); ++y) for (int x = 0; x < int(MAP_S); ++x) {
		w[x][y] = { std::bitset<8>(), std::bitset<8>() };
		next.push_back(POINT{ x,y });
	}
}
bool BlobWave::Observe() { //not done
	return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool ctrl = false;
    switch (message)
    {
	case WM_CREATE:
		{
			srand(unsigned(time(NULL)));

			TILE_S = 32;
			MAP_S = 16;
			tilemapID = 0;
			edgebmp = new Gdiplus::Bitmap(L"Edge.bmp");
			cornerbmp = new Gdiplus::Bitmap(L"Corner.bmp");
			blobbmp = new Gdiplus::Bitmap(L"Blob.bmp");
			output = new Gdiplus::Bitmap(TILE_S*MAP_S, TILE_S*MAP_S);
			input = new EdgeInput();
			wave = new EdgeWave();

			HMENU hmenuMain = GetMenu(hWnd);
			DeleteMenu(hmenuMain, 1, MF_BYPOSITION);
			HMENU hmenuFile = GetSubMenu(hmenuMain, 0);
			ModifyMenu(hmenuFile, IDM_EXIT, MF_STRING, IDM_EXIT, L"Exit	Alt+F4");
			InsertMenu(hmenuFile, IDM_EXIT, MF_STRING, IDM_SAVE, L"Save	Ctrl+S");
			InsertMenu(hmenuFile, IDM_EXIT, MF_STRING, 99, L"Reset	Esc");

			HBITMAP save;
			Gdiplus::Bitmap(L"save.bmp").GetHBITMAP(Gdiplus::Color::White, &save);
			SetMenuItemBitmaps(hmenuFile, 0, MF_BYPOSITION, save, NULL);

			HMENU hmenuEdge = CreatePopupMenu();
			AppendMenu(hmenuMain, MF_STRING | MF_POPUP, (UINT_PTR)hmenuEdge, L"Edge...");
			WCHAR* enames[33] = {L"Wang", L"BlockyPipe", L"Border", L"Brench", L"BrickWall", L"Bridge",
				L"Circuit", L"Drawn", L"Dropbox", L"Dual", L"Dungeon", L"Greek", L"Groove",
				L"Laser", L"Lattice", L"Ledge", L"Line", L"Molecule", L"Octal",
				L"O-Ring", L"Polygon", L"Rail", L"RedNoise", L"Road", L"Square", L"ThinPipe",
				L"Tilt", L"Trench", L"Tube", L"Urban", L"WidePipe", L"ZigZag"};
			for (unsigned i = 0; i < 33; ++i)
				AppendMenu(hmenuEdge, MF_STRING, i, enames[i]);

			HMENU hmenuCorner = CreatePopupMenu();
			AppendMenu(hmenuMain, MF_STRING | MF_POPUP, (UINT_PTR)hmenuCorner, L"Corner...");
			WCHAR* cnames[18] = { L"Wang", L"Terrain", L"Square", L"SeaSand", L"SandGrass",
				L"Roof", L"PCB", L"Path", L"Patch", L"Lido", L"Island", L"Ground",
				L"Glob", L"Cliff", L"Cellar", L"Brench", L"Border", L"Beam" };
			for (unsigned i = 0; i < 18; ++i)
				AppendMenu(hmenuCorner, MF_STRING, i + 33, cnames[i]);

			HMENU hmenuBlob = CreatePopupMenu();
			AppendMenu(hmenuMain, MF_STRING | MF_POPUP, (UINT_PTR)hmenuBlob, L"Blob...");
			WCHAR* bnames[6] = { L"Wang", L"Islands", L"Dungeon", L"Commune", L"Trench", L"Bridge" };
			for (unsigned i = 0; i < 6; ++i)
				AppendMenu(hmenuBlob, MF_STRING, i + 33 + 18, bnames[i]);

			CheckMenuRadioItem(hmenuEdge, 0, 31, 0, MF_BYPOSITION);

			//HWND hxEdit = CreateWindow(TEXT("COMBOBOX"), TEXT(""), CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			//	int(TILE_S*MAP_S), 0, 50, 25, hWnd, NULL, HINSTANCE(IDC_XEDIT), NULL);
			//SendMessage(hxEdit, (UINT)CB_ADDSTRING, (WPARAM)0, LPARAM(L"16"));
			//SendMessage(hxEdit, (UINT)CB_ADDSTRING, (WPARAM)0, LPARAM(L"32"));

			SetTimer(hWnd, 0, 10, TIMERPROC(NULL));
		}
		break;
	case WM_TIMER:
		{
			if (wave->Observe())
				InvalidateRect(hWnd, nullptr, false);
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			if (wmId < 100) {
				if (wmId < 33) {
					tilemapID = wmId;
					delete(input);
					input = new EdgeInput();
					delete(wave);
					wave = new EdgeWave();
					HMENU hmenuMain = GetMenu(hWnd);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 1), 0, 31, wmId, MF_BYPOSITION);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 2), 0, 17, 18, MF_BYPOSITION);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 3), 0, 5, 6, MF_BYPOSITION);
				}
				else if (wmId < 51) {
					tilemapID = wmId - 33;
					delete(input);
					input = new CornerInput();
					delete(wave);
					wave = new CornerWave();
					HMENU hmenuMain = GetMenu(hWnd);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 1), 0, 31, 32, MF_BYPOSITION);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 2), 0, 17, wmId - 33, MF_BYPOSITION);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 3), 0, 5, 6, MF_BYPOSITION);
				}
				else if (wmId < 57) {
					tilemapID = wmId - 51;
					delete(input);
					input = new BlobInput();
					delete(wave);
					wave = new BlobWave();
					HMENU hmenuMain = GetMenu(hWnd);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 1), 0, 31, 32, MF_BYPOSITION);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 2), 0, 17, 18, MF_BYPOSITION);
					CheckMenuRadioItem(GetSubMenu(hmenuMain, 3), 0, 5, wmId - 51, MF_BYPOSITION);
				}
				else {
					input->Reset();
					wave->Reset();
				}
				InvalidateRect(hWnd, nullptr, true);
				break;
			}
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDM_SAVE:
				{
					CLSID pngClsid;
					GetEncoderClsid(L"image/png", &pngClsid);
					if (output->Save(L"Output.png", &pngClsid) == Gdiplus::Status::Ok)
						MessageBox(hWnd, L"Image saved as Output.png", L"Saved", MB_OK);
				}
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_KEYDOWN:
		{
			switch (wParam) {
				case VK_ESCAPE:
				{
					input->Reset();
					wave->Reset();
					InvalidateRect(hWnd, nullptr, true);
				}
				break;
				case VK_CONTROL:
				{
					ctrl = true;
				}
				break;
				case 0x53: //S
				{
					if (ctrl) {
						CLSID pngClsid;
						GetEncoderClsid(L"image/png", &pngClsid);
						if (output->Save(L"Output.png", &pngClsid) == Gdiplus::Status::Ok)
							MessageBox(hWnd, L"Image saved as Output.png", L"Saved", MB_OK);
					}
				}
				break;
			}
		}
		break;
	case WM_KEYUP:
		{
			switch (wParam) {
				case VK_CONTROL:
				{
					ctrl = false;
				}
				break;
			}
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			Gdiplus::Graphics graphics(hdc);
			graphics.DrawImage(output, 0, 0);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		{
			delete(edgebmp);
			delete(cornerbmp);
			delete(output);
			delete(input);
			delete(wave);
			PostQuitMessage(0);
		}
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
