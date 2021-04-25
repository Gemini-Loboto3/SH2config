#pragma once
#include "framework.h"
#include "CommCtrl.h"
#include <string>

#undef CreateWindow

class CWnd
{
public:
	CWnd() :
		hWnd(0),
		hWndParent(0),
		hInst(0)
	{}

	operator HWND() { return hWnd; }

	void CreateWindow(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance)
	{
		hInst = hInstance;
		hWndParent = hParent;
		hWnd = ::CreateWindowExW(0, lpClassName, lpWindowName, dwStyle, X, Y, Width, Height, hParent, nullptr, hInstance, (LPVOID)this);
	}
	void Destroy()
	{
		DestroyWindow(hWnd);
		hWnd = 0;
	}

	void SetWnd(HWND wnd) { hWnd = wnd; }
	void SetText(LPCWSTR lpString) { SetWindowTextW(*this, lpString); }

private:
	HWND hWnd, hWndParent;
	HINSTANCE hInst;
};

class CCtrlGroup : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_BUTTONW, lpName, WS_CHILD | BS_GROUPBOX | WS_VISIBLE, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}
};

class CCtrlButton : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_BUTTONW, lpName, WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}
};

class CCtrlTab : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_TABCONTROLW, L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_MULTILINE, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}

	void InsertItem(int index, LPCWSTR lpString)
	{
		TCITEMW item;
		item.mask = TCIF_TEXT;
		item.pszText = (LPWSTR)lpString;
		SendMessageW(*this, TCM_INSERTITEMW, (WPARAM)index, (LPARAM)&item);
	}

	void InsertItem(int index, LPCSTR lpString)
	{
		TCITEMA item;
		item.mask = TCIF_TEXT;
		item.pszText = (LPSTR)lpString;
		SendMessageW(*this, TCM_INSERTITEMA, (WPARAM)index, (LPARAM)&item);
	}

	int GetCurSel()
	{
		return SendMessageW(*this, TCM_GETCURSEL, 0, 0);
	}

	void GetRect(RECT& rect)
	{
		GetClientRect(*this, &rect);
		SendMessageW(*this, TCM_ADJUSTRECT, (WPARAM)false, (LPARAM)&rect);
	}
};

class CCtrlStatic : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont, UINT Align = 1)
	{
		szText = lpName;
		this->hFont = hFont;
		uAlign = Align;
		CWnd::CreateWindow(WC_STATICW, lpName, SS_LEFT | WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, X, Y, Width, Height, hParent, hInstance);
		SetWindowLongW(*this, GWLP_USERDATA, (LONG)this);
		SetWindowLongW(*this, GWLP_WNDPROC, (LONG)proc);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}

	void SetText(LPCWSTR lpText) { szText = lpText; }

	std::wstring szText;
	WNDPROC old_proc;
	HFONT hFont;
	UINT uAlign;

	static LRESULT CALLBACK proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		CCtrlStatic* c = reinterpret_cast<CCtrlStatic*>(GetWindowLongW(hWnd, GWLP_USERDATA));

		switch (Msg)
		{
		case WM_PAINT:
		{
			RECT rc;
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);
			SetBkMode(hdc, TRANSPARENT);
			SelectObject(hdc, c->hFont);
			SIZE size;
			GetTextExtentPoint32W(hdc, c->szText.c_str(), c->szText.size(), &size);
			int Y;
			switch (c->uAlign)
			{
			case 0: Y = rc.top; break;
			case 1: Y = (rc.bottom - size.cy) / 2; break;
			case 2: Y = rc.bottom - size.cy; break;
			}
			TextOutW(hdc, 0, Y, c->szText.c_str(), c->szText.size());
			EndPaint(hWnd, &ps);
			return 0;
		}
		default:
			return CallWindowProcW(c->old_proc, hWnd, Msg, wParam, lParam);
		}

		return 0;
	}
};

class CCtrlCheckBox : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_BUTTONW, lpName, BS_CHECKBOX | WS_VISIBLE | WS_CHILD | BS_VCENTER, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}

	bool GetCheck() { return SendMessageW(*this, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false; }
	void SetCheck(bool check) { SendMessageW(*this, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, 0); }
};

class CCtrlDropBox : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD | WS_VSCROLL, X + Width - 80, Y, 80/*Width*/, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}

	void Reset() { SendMessageW(*this, CB_RESETCONTENT, 0, 0); }
	void AddString(LPCWSTR lpString) { SendMessageW(*this, CB_ADDSTRING, 0, (LPARAM)lpString); }

	int GetSelection() { return SendMessageW(*this, CB_GETCURSEL, 0, 0); }
	void SetSelection(int sel) { SendMessageW(*this, CB_SETCURSEL, sel, 0); }
};

class CCtrlDescription
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont, HFONT hBold)
	{
		hBack.CreateWindow(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_WHITERECT | SS_RIGHTJUST | WS_BORDER, X, Y, Width, Height, hParent, hInstance);
		hStrike.CreateWindow(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 4, 22, Width - 8, Height, hBack, hInstance);
		hCaption.CreateWindow(L"AudioClipDetection", 4, 0/*Y + 2*/, Width - 8, 24, hBack, hInstance, hBold, 0);
		hText.CreateWindow(L"Detects when any audio event is stopped prematurely and fades out the sound to avoid \"popping / clicking\" that would otherwise be heard.", 4, 24/*Y + 17*/, Width - 8, Height - 32, hBack, hInstance, hFont, 0);
	}

	void SetCaption(LPCWSTR lpCaption)
	{
		hCaption.SetText(lpCaption);
		InvalidateRect(hCaption, nullptr, true);
	}
	void SetText(LPCWSTR lpText)
	{
		hText.SetText(lpText);
		InvalidateRect(hText, nullptr, true);
	}

	CCtrlStatic hCaption,
		hText;
	CWnd hBack, hStrike;
};

// combined controls
class CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont) {}
	virtual void Release() {}
	// lists
	virtual void Reset() {}
	virtual void AddString(LPCWSTR lpString) {}
	virtual int GetSelection() { return 0; }
	virtual void SetSelection(int sel) {}
	// checkboxes
	virtual bool GetCheck() { return false; }
	virtual void SetCheck(bool check) {}
};

class CFieldCombo : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		box.CreateWindow(lpName, X, Y, Width, Height, hParent, hInstance, hFont);
	}
	virtual void Release()
	{
		box.Destroy();
	}

	virtual bool GetCheck() { return box.GetCheck(); }
	virtual void SetCheck(bool check) { box.SetCheck(check); }

	CCtrlCheckBox box;
};

class CFieldList : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		const int sub = 80;
		hStatic.CreateWindow(lpName, X, Y, Width - sub, Height, hParent, hInstance, hFont);
		hList.CreateWindow(X + Width - sub, Y, sub, Height, hParent, hInstance, hFont);
	}
	virtual void Release()
	{
		hStatic.Destroy();
		hList.Destroy();
	}

	virtual void Reset() { hList.Reset(); }
	virtual void AddString(LPCWSTR lpString) { hList.AddString(lpString); }

	virtual int GetSelection() { return hList.GetSelection(); }
	virtual void SetSelection(int sel) { hList.SetSelection(sel); }

private:
	CCtrlStatic hStatic;
	CCtrlDropBox hList;
};

class CFieldPad : public CCombined
{
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		hStatic.CreateWindow(lpName, X, Y, Width - 80, Height, hParent, hInstance, hFont);
		hList.CreateWindow(X + Width - 80, Y, 80, Height, hParent, hInstance, hFont);
	}
	virtual void Release()
	{
		hStatic.Destroy();
		hList.Destroy();
	}

	virtual void Reset() { hList.Reset(); }
	virtual void AddString(LPCWSTR lpString) { hList.AddString(lpString); }

	virtual int GetSelection() { return hList.GetSelection(); }
	virtual void SetSelection(int sel) { hList.SetSelection(sel); }

private:
	CCtrlStatic hStatic;
	CCtrlDropBox hList;
};
