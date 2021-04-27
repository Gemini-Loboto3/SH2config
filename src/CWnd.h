#pragma once
#include "framework.h"
#include "CommCtrl.h"
#include <string>
#include "CConfig.h"

#undef CreateWindow

class CWnd
{
public:
	CWnd() :
		hWnd(0),
		hWndParent(0),
		hInst(0),
		tType(TYPE_WND),
		old_proc(nullptr)
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
	void Subclass(WNDPROC new_proc)
	{
		WNDPROC ret = (WNDPROC)SetWindowLongPtrW(*this, GWLP_WNDPROC, (LONG_PTR)new_proc);
		// preserve only the original procedure
		if (old_proc == nullptr)
			old_proc = ret;
	}
	LRESULT CallProcedure(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return CallWindowProcW(old_proc, wnd, msg, wParam, lParam);
	}

	enum Type
	{
		TYPE_WND,
		TYPE_LIST,
		TYPE_CHECKBOX
	};
	Type tType;

private:
	HWND hWnd, hWndParent;
	HINSTANCE hInst;
	WNDPROC old_proc;
};

class CCtrlGroup : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_BUTTONW, lpName, WS_CHILD | BS_GROUPBOX | WS_VISIBLE, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
		SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
	}
};

class CCtrlButton : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_BUTTONW, lpName, WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
		SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
	}
};

class CCtrlTab : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_TABCONTROLW, L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_MULTILINE, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
		SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
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
		return (int)SendMessageW(*this, TCM_GETCURSEL, 0, 0);
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
		SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
		//old_proc = (WNDPROC)SetWindowLongPtrW(*this, GWLP_WNDPROC, (LONG_PTR)proc);
		Subclass(proc);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	}

	bool SetText(LPCWSTR lpText)
	{
		if (szText.compare(lpText) == 0)
			return false;
		szText = lpText;
		return true;
	}

	void OnPaint(HDC hdc)
	{
		RECT rc;
		GetClientRect(*this, &rc);

		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, hFont);
		SIZE size;
		GetTextExtentPoint32W(hdc, szText.c_str(), (int)szText.size(), &size);
#if 0
		int Y = 0;
		switch (c->uAlign)
		{
		case 0: Y = rc.top; break;
		case 1: Y = (rc.bottom - size.cy) / 2; break;
		case 2: Y = rc.bottom - size.cy; break;
		}

		ExtTextOutW(hdc, 0, Y, ETO_CLIPPED, &rc, c->szText.c_str(), c->szText.size(), nullptr);
#else
		int Y;
		switch (uAlign)
		{
		case 1: Y = (rc.bottom - size.cy) / 2; rc.top += Y; rc.bottom -= Y; break;
		case 2: Y = rc.bottom - size.cy; rc.top += Y; rc.bottom -= Y; break;
		}
		DrawTextExW(hdc, (LPWSTR)szText.c_str(), (int)szText.size(), &rc, DT_LEFT | DT_WORDBREAK, nullptr);
#endif
	}

	std::wstring szText;
	HFONT hFont;
	UINT uAlign;

	static LRESULT CALLBACK proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		CCtrlStatic* c = reinterpret_cast<CCtrlStatic*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

		switch (Msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			c->OnPaint(hdc);
			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
			return 0;
		default:
			return c->CallProcedure(hWnd, Msg, wParam, lParam);
		}

		return 0;
	}
};

class CCtrlCheckBox : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_BUTTONW, lpName, BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD | BS_VCENTER, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);

		tType = TYPE_CHECKBOX;
	}

	bool GetCheck() { return SendMessageW(*this, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false; }
	void SetCheck(bool check) { SendMessageW(*this, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, 0); }
};

class CCtrlDropBox : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		CWnd::CreateWindow(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_VSCROLL, X, Y, Width, Height, hParent, hInstance);
		SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
		SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);

		tType = TYPE_LIST;
	}

	void Reset() { SendMessageW(*this, CB_RESETCONTENT, 0, 0); }
	void AddString(LPCWSTR lpString) { SendMessageW(*this, CB_ADDSTRING, 0, (LPARAM)lpString); }

	int GetSelection() { return (int)SendMessageW(*this, CB_GETCURSEL, 0, 0); }
	void SetSelection(int sel) { SendMessageW(*this, CB_SETCURSEL, sel, 0); }
};

class CCtrlDescription
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont, HFONT hBold)
	{
		hBack.CreateWindow(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_WHITERECT | SS_RIGHTJUST | WS_BORDER, X, Y, Width, Height, hParent, hInstance);
		hStrike.CreateWindow(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 4, 22, Width - 8, Height, hBack, hInstance);
		hCaption.CreateWindow(L"", 4, 2, Width - 8, 20, hBack, hInstance, hBold, 0);
		hText.CreateWindow(L"", 4, 24, Width - 8, Height - 32, hBack, hInstance, hFont, 0);
	}

	void SetCaption(LPCWSTR lpCaption)
	{
		if (hCaption.SetText(lpCaption))
		{
			InvalidateRect(hCaption, nullptr, true);
			UpdateWindow(hCaption);
		}
	}
	void SetText(LPCWSTR lpText)
	{
		if (hText.SetText(lpText))
		{
			InvalidateRect(hText, nullptr, true);
			UpdateWindow(hText);
		}
	}

	CCtrlStatic hCaption, hText;
	CWnd hBack, hStrike;
};

// combined controls
class CCombined
{
public:
	CCombined() : cCtrl(nullptr),
		cValue(nullptr),
		uType(TYPE_DEFAULT)
	{}

	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont) {}
	virtual void Release() {}
	virtual void SetHover(std::wstring Text, std::wstring Desc, CCtrlDescription* ctrl) { szText = Text; szDesc = Desc; cCtrl = ctrl; }
	// lists
	virtual void Reset() {}
	virtual void AddString(LPCWSTR lpString) {}
	virtual int GetSelection() { return 0; }
	virtual void SetSelection(int sel) {}
	// checkboxes
	virtual bool GetCheck() { return false; }
	virtual void SetCheck(bool check) {}

	// random shit with configuration
	void SetConfigPtr(CConfigOption* c) { cValue = c; }
	void SetConfigValue(int val)
	{
		if (cValue)
			cValue->cur_val = val;
	}

	enum TYPE
	{
		TYPE_DEFAULT,
		TYPE_CHECK,
		TYPE_LIST,
		TYPE_PAD
	};

	UINT uType;
	std::wstring szText, szDesc;
	CCtrlDescription* cCtrl;
	CConfigOption* cValue;
};

class CFieldCheck : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		uType = TYPE_CHECK;
		box.CreateWindow(lpName, X, Y, Width, Height, hParent, hInstance, hFont);
		SetWindowLongPtrW(box, GWLP_USERDATA, (LONG_PTR)this);
		//old = (WNDPROC)SetWindowLongPtrW(box, GWLP_WNDPROC, (LONG_PTR)proc);
		box.Subclass(proc);
	}
	virtual void Release()
	{
		box.Destroy();
	}

	virtual bool GetCheck() { return box.GetCheck(); }
	virtual void SetCheck(bool check) { box.SetCheck(check); }

	static LRESULT CALLBACK proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		CFieldCheck* check = (CFieldCheck*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		switch (Msg)
		{
		case WM_MOUSEMOVE:
			if (check->cCtrl)
			{
				check->cCtrl->SetCaption(check->szText.c_str());
				check->cCtrl->SetText(check->szDesc.c_str());
			}
			break;
		}

		return check->box.CallProcedure(hWnd, Msg, wParam, lParam);
	}

	CCtrlCheckBox box;
	WNDPROC old;
};

class CFieldList : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		uType = TYPE_LIST;
		const int sub = 140;
		hStatic.CreateWindow(lpName, X, Y, Width - sub, Height, hParent, hInstance, hFont);
		hList.CreateWindow(X + Width - sub, Y, sub, Height, hParent, hInstance, hFont);

		SetWindowLongPtrW(hStatic, GWLP_USERDATA, (LONG_PTR)this);
		SetWindowLongPtrW(hList,   GWLP_USERDATA, (LONG_PTR)this);
		//old_static = (WNDPROC)SetWindowLongPtrW(hStatic, GWLP_WNDPROC, (LONG_PTR)proc_static);
		hStatic.Subclass(proc_static);
		hList.Subclass(proc_list);
		//old_list   = (WNDPROC)SetWindowLongPtrW(hList,   GWLP_WNDPROC, (LONG_PTR)proc_list);
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

	static LRESULT CALLBACK proc_list(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		CFieldList* list = (CFieldList*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		switch (Msg)
		{
		case WM_MOUSEMOVE:
			if (list->cCtrl)
			{
				list->cCtrl->SetCaption(list->szText.c_str());
				list->cCtrl->SetText(list->szDesc.c_str());
			}
			break;
		}

		return list->hList.CallProcedure(hWnd, Msg, wParam, lParam);
	}

	static LRESULT CALLBACK proc_static(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		CFieldList* st = (CFieldList*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		switch (Msg)
		{
		case WM_NCHITTEST:
			if (st->cCtrl)
			{
				st->cCtrl->SetCaption(st->szText.c_str());
				st->cCtrl->SetText(st->szDesc.c_str());
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				st->hStatic.OnPaint(hdc);
				EndPaint(hWnd, &ps);
				return 0;
			}
		case WM_ERASEBKGND:
			return 0;
		}

		return st->hStatic.CallProcedure(hWnd, Msg, wParam, lParam);
	}

private:
	CCtrlStatic hStatic;
	CCtrlDropBox hList;
};

class CFieldPad : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		uType = TYPE_PAD;
		const int W = 110;
		hStatic.CreateWindow(lpName, X, Y, Width - W, Height, hParent, hInstance, hFont);
		hList.CreateWindow(X + Width - W, Y, W, Height, hParent, hInstance, hFont);
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
