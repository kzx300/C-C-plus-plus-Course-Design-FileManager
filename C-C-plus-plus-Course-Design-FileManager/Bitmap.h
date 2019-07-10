#pragma once
#include<Windows.h>
#ifndef _BITMAP_
#define BITMAP
class Bitmap
{
private:
	HBITMAP m_hBitmap;
	int m_iWidth, m_iHeight;
	void Free();
public:
	Bitmap();
	explicit Bitmap(HDC hDC, LPCWSTR szFileName);
	explicit Bitmap(HDC hDC, UINT uiResID, HINSTANCE hInstance);
	explicit Bitmap(HDC hDC, int iWidth, int iHeight, COLORREF crColor = RGB(0, 0, 0));
	virtual ~Bitmap();
	bool Create(HDC hDC, LPCWSTR szFileName);
	bool Create(HDC hDC, UINT uiResID, HINSTANCE hInstance);
	bool Create(HDC hDC, int iWidth, int iHeight, COLORREF crColor = RGB(0, 0, 0));
	void Draw(HDC hDC, int x, int y, bool bTrans = false, COLORREF crTransColor = RGB(255, 0, 255));
	inline int GetWidth() { return m_iWidth; }
	inline int GetHeight() { return m_iHeight; }
};
#endif // !_BITMAP_
