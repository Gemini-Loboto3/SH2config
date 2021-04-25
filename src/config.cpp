// config.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "resources.h"
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
	hBold = CreateFont(lf.lfHeight - 5, lf.lfWidth/* - 5*/,
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
	wcex.hIcon          = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_CONFIG));
	wcex.hCursor        = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName  = L"SH2CONFIG";
	wcex.hIconSm        = LoadIconW(wcex.hInstance, MAKEINTRESOURCEW(IDI_SMALL));

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

std::shared_ptr<CCombined> MakeControl(int section, int option, int pos, RECT tab, int base_Y)
{
	std::shared_ptr<CCombined> c;
	int W = (tab.right - tab.left - 40) / 2;
	int X = tab.left + 10 + (pos % 2) * W;
	int Y = tab.top + (pos / 2) * 25 + base_Y;

	switch (cfg.section[section].option[option].type)
	{
	case CConfigOption::TYPE_CHECK:
	{
		c = std::make_shared<CFieldCombo>();
		c->CreateWindow(cfg.GetOptionString(section, option).c_str(), X, Y, W - 20, 25, hTab, hInst, hFont);
	}
	break;
	case CConfigOption::TYPE_LIST:
	{
		c = std::make_shared<CFieldList>();
		c->CreateWindow(cfg.GetOptionString(section, option).c_str(), X, Y, W - 20, 25, hTab, hInst, hFont);
		for (size_t j = 0, sj = cfg.section[section].option[option].value.size(); j < sj; j++)
			c->AddString(cfg.GetValueString(section, option, j).c_str());
		c->SetSelection(0);
	}
	break;
	case CConfigOption::TYPE_PAD:
	{
		c = std::make_shared<CFieldList>();
		c->CreateWindow(cfg.GetOptionString(section, option).c_str(), X, Y, W - 20, 25, hTab, hInst, hFont);
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
			hCtrl.push_back(MakeControl(sec, opt, pos, rect, Y));
		}

		Y += gp_rect.bottom - gp_rect.top;

		//std::shared_ptr<CCombined> c;
		//
		//switch (cfg.section[section].option[i].type)
		//{
		//	case CConfigOption::TYPE_CHECK:
		//	{
		//		c = std::make_shared<CFieldCombo>();
		//		c->CreateWindow(cfg.GetOptionString(section, i).c_str(), 20, 50 + i * 25, 550, 25, hTab, hInst, hFont);
		//	}
		//	break;
		//case CConfigOption::TYPE_LIST:
		//	{
		//		c = std::make_shared<CFieldList>();
		//		c->CreateWindow(cfg.GetOptionString(section, i).c_str(), 20, 50 + i * 25, 550, 25, hTab, hInst, hFont);
		//		for (size_t j = 0, sj = cfg.section[section].option[i].value.size(); j < sj; j++)
		//			c->AddString(cfg.GetValueString(section, i, j).c_str());
		//		c->SetSelection(0);
		//	}
		//	break;
		//case CConfigOption::TYPE_PAD:
		//	{
		//		c = std::make_shared<CFieldList>();
		//		c->CreateWindow(cfg.GetOptionString(section, i).c_str(), 20, 50 + i * 25, 550, 25, hTab, hInst, hFont);
		//		// TODO: populate list with controller enumeration
		//	}
		//	break;
		//case CConfigOption::TYPE_TEXT:
		//	c = std::make_shared<CCombined>();
		//	break;
		//case CConfigOption::TYPE_UNK:
		//	c = std::make_shared<CCombined>();
		//	break;
		//}


		//hCtrl.push_back(c);
	}
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	//HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	hWnd.CreateWindow(L"SH2CONFIG", L"Silent Hill 2 Enhanced Edition Configuration Tool", WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, hInstance);

	RECT r;
	GetClientRect(hWnd, &r);

	hTab.CreateWindow(0, 0, r.right, r.bottom - 130, hWnd, hInstance, hFont);
	for (size_t i = 0, si = cfg.group.size(); i < si; i++)
		hTab.InsertItem(i, cfg.GetGroupString(i).c_str());
	PopulateTab(0);

	hDesc.CreateWindow(0, r.bottom - 128, r.right - 2, 94, hWnd, hInstance, hFont, hBold);

	int Y = r.bottom - 30;
	hBnClose.CreateWindow(L"Close", 4, Y, 60, 26, hWnd, hInstance, hFont);

	int X = r.right - 234;
	hBnDefault.CreateWindow(L"Defaults", X, Y, 60, 26, hWnd, hInstance, hFont); X += 64;
	hBnDefault.CreateWindow(L"Save", X, Y, 40, 26, hWnd, hInstance, hFont); X += 44;
	hBnDefault.CreateWindow(L"Save && Launch Game", X, Y, 120, 26, hWnd, hInstance, hFont);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
			CWnd* h = (CWnd*)pCS->lpCreateParams;
			h->SetWnd(hwnd);
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hwnd, &ps);
		}
		break;
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
	default:
		return DefWindowProcW(hwnd, message, wParam, lParam);
	}
	return 0;
}
