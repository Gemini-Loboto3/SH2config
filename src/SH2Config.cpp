// config.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "resource.h"
#include "CWnd.h"
#include "CConfig.h"
#include <memory>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;
HFONT hFont, hBold;
bool bIsLooping = true;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

WNDPROC TabProc_old;
LRESULT CALLBACK TabProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

CConfig cfg;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow)
{
	cfg.ParseXml();
	// Initialize global strings
	InitCommonControls();
	MyRegisterClass(hInstance);

	LOGFONT lf;
	GetObjectA(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	hFont = CreateFont(lf.lfHeight, lf.lfWidth,
		lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);
	hBold = CreateFont(lf.lfHeight - 4, lf.lfWidth/* - 5*/,
		lf.lfEscapement, lf.lfOrientation, lf.lfWeight * 2,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
		lf.lfPitchAndFamily, lf.lfFaceName);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// Main message loop:
	while (bIsLooping)
	{
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.hCursor        = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName  = L"SH2CONFIG";
	wcex.hIconSm = wcex.hIcon= LoadIconW(wcex.hInstance, MAKEINTRESOURCEW(IDI_CONFIG));

	return RegisterClassExW(&wcex);
}

CWnd hWnd;
CCtrlCheckBox hCheck;
CCtrlTab hTab;
CCtrlButton hBnClose, hBnDefault, hBnSave,
	hBnLaunch;
CCtrlDescription hDesc;

std::vector<std::shared_ptr<CCombined>> hCtrl;
std::vector<std::shared_ptr<CCtrlGroup>> hGroup;

std::shared_ptr<CCombined> MakeControl(CWnd &hParent, int section, int option, int pos, RECT tab, int base_Y)
{
	std::shared_ptr<CCombined> c;
	int W = (tab.right - tab.left - 40) / 2;
	int X = tab.left + 16 + (pos % 2) * W;
	int Y = tab.top + (pos / 2) * 25 + base_Y;

	std::wstring name, desc;
	name = cfg.GetOptionString(section, option).c_str();
	desc = cfg.GetOptionDesc(section, option).c_str();

	switch (cfg.section[section].option[option].type)
	{
	case CConfigOption::TYPE_CHECK:
	{
		c = std::make_shared<CFieldCombo>();
		c->CreateWindow(name.c_str(), X, Y, W - 20, 25, hParent, hInst, hFont);
		c->SetHover(name.c_str(), desc.c_str(), &hDesc);
		// set current value
		c->SetConfigPtr(&cfg.section[section].option[option]);
		c->SetCheck((bool)c->cValue->cur_val);
	}
	break;
	case CConfigOption::TYPE_LIST:
	{
		c = std::make_shared<CFieldList>();
		c->CreateWindow(name.c_str(), X, Y, W - 20, 25, hParent, hInst, hFont);
		for (size_t j = 0, sj = cfg.section[section].option[option].value.size(); j < sj; j++)
			c->AddString(cfg.GetValueString(section, option, j).c_str());
		c->SetHover(name.c_str(), desc.c_str(), &hDesc);
		// set current value
		c->SetConfigPtr(&cfg.section[section].option[option]);
		c->SetSelection(c->cValue->cur_val);
	}
	break;
	case CConfigOption::TYPE_PAD:
	{
		c = std::make_shared<CFieldList>();
		c->CreateWindow(cfg.GetOptionString(section, option).c_str(), X, Y, W - 20, 25, hParent, hInst, hFont);
		// TODO: populate list with controller enumeration
	}
	break;
	case CConfigOption::TYPE_TEXT:
		c = std::make_shared<CCombined>();
		break;
	case CConfigOption::TYPE_UNK:
		c = std::make_shared<CCombined>();
		break;
	}

	

	return c;
}

void PopulateTab(int section)
{
	hDesc.SetCaption(cfg.GetGroupString(section).c_str());
	hDesc.SetText(L"");

	for (size_t i = 0, si = hCtrl.size(); i < si; i++)
		hCtrl[i]->Release();
	hCtrl.clear();
	for (size_t i = 0, si = hGroup.size(); i < si; i++)
		DestroyWindow(*hGroup[i]);
	hGroup.clear();

	RECT rect;
	hTab.GetRect(rect);

	int Y = 20;

	// process a group
	for (size_t i = 0, si = cfg.group[section].sub.size(), pos = 0; i < si; i++)
	{
		size_t count = cfg.group[section].sub[i].opt.size();
		int rows = ((count + 1) & ~1) / 2;

		std::shared_ptr<CCtrlGroup> gp = std::make_shared<CCtrlGroup>();
		gp->CreateWindow(cfg.GetGroupLabel(section, i).c_str(), rect.left + 2, Y + rect.top - 20, rect.right - rect.left - 6, rows * 25 + 30, hTab, hInst, hFont);
		hGroup.push_back(gp);

		RECT gp_rect;
		GetClientRect(*gp, &gp_rect);

		pos = 0;
		// process a sub
		for (size_t j = 0, sj = cfg.group[section].sub[i].opt.size(); j < sj; j++, pos++)
		{
			int sec, opt;
			cfg.FindSectionAndOption(cfg.group[section].sub[i].opt[j].sec, cfg.group[section].sub[i].opt[j].op, sec, opt);
			hCtrl.push_back(MakeControl(hTab, sec, opt, pos, rect, Y));
		}

		Y += gp_rect.bottom - gp_rect.top;
	}
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	hWnd.CreateWindow(L"SH2CONFIG", L"Silent Hill 2 Enhanced Edition Configuration Tool", WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, hInstance);
	if (!hWnd)
		return FALSE;

	RECT r;
	GetClientRect(hWnd, &r);

	hTab.CreateWindow(0, 0, r.right, r.bottom - 132, hWnd, hInstance, hFont);
	for (size_t i = 0, si = cfg.group.size(); i < si; i++)
		hTab.InsertItem(i, cfg.GetGroupString(i).c_str());
	TabProc_old = (WNDPROC)SetWindowLongW(hTab, GWLP_WNDPROC, (LONG)TabProc);

	hDesc.CreateWindow(2, r.bottom - 130, r.right - 4, 98, hWnd, hInstance, hFont, hBold);

	int Y = r.bottom - 30;
	hBnClose.CreateWindow(L"Close", 4, Y, 60, 26, hWnd, hInstance, hFont);

	int X = r.right - 234;
	hBnDefault.CreateWindow(L"Defaults", X, Y, 60, 26, hWnd, hInstance, hFont); X += 64;
	hBnDefault.CreateWindow(L"Save", X, Y, 40, 26, hWnd, hInstance, hFont); X += 44;
	hBnDefault.CreateWindow(L"Save && Launch Game", X, Y, 120, 26, hWnd, hInstance, hFont);

	PopulateTab(0);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK TabProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
			{
				CCombined* wnd = reinterpret_cast<CCombined*>(GetWindowLongW((HWND)lParam, GWLP_USERDATA));
				int sel = wnd->GetSelection();
				wnd->SetConfigValue(sel);
			}
			break;
		case BN_CLICKED:
			{
				CCombined* wnd = reinterpret_cast<CCombined*>(GetWindowLongW((HWND)lParam, GWLP_USERDATA));
				bool checked = wnd->GetCheck();
				wnd->SetConfigValue(checked);
			}
			break;
		}
		break;
	}

	return CallWindowProcW(TabProc_old, hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
			CWnd* h = (CWnd*)pCS->lpCreateParams;
			h->SetWnd(wnd);
		}
		return 0;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(wnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(wnd, &ps);
		}
		return 0;
	case WM_DESTROY:
		bIsLooping = false;
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case TCN_SELCHANGE:
			PopulateTab(hTab.GetCurSel());
			break;
		}
		break;
	}

	return DefWindowProcW(wnd, message, wParam, lParam);
}
