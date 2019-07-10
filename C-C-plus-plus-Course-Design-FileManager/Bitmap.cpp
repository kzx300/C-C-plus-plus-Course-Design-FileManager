#include "stdafx.h"
#include "Bitmap.h"

void Bitmap::Free()
{
	if (m_hBitmap != nullptr)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = nullptr;
	}
}

Bitmap::Bitmap() :m_hBitmap(nullptr), m_iWidth(0), m_iHeight(0)
{
}

Bitmap::Bitmap(HDC hDC, LPCWSTR szFileName) : m_hBitmap(nullptr), m_iWidth(0), m_iHeight(0)
{
	Create(hDC, szFileName);
}

Bitmap::Bitmap(HDC hDC, UINT uiResID, HINSTANCE hInstance) : m_hBitmap(nullptr), m_iWidth(0), m_iHeight(0)
{
	Create(hDC, uiResID, hInstance);
}

Bitmap::Bitmap(HDC hDC, int iWidth, int iHeight, COLORREF crColor) : m_hBitmap(nullptr), m_iWidth(0), m_iHeight(0)
{
	Create(hDC, iWidth, iHeight, crColor);
}

Bitmap::~Bitmap()
{
	Free();
}

bool Bitmap::Create(HDC hDC, LPCWSTR szFileName)
{
	Free();// �ͷ���ǰ��λͼ��Ϣ���������Ƿ����

	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//��һ��λͼ�ļ�                          
	if (hFile == INVALID_HANDLE_VALUE)//����ļ��Ƿ�򿪳ɹ�                                        
		return false;                                          

	// ��ȡλͼ�ļ���ͷ����Ϣ
	BITMAPFILEHEADER  bmfHeader;//����λͼ�ļ���ͷ����Ϣ
	DWORD             dwBytesRead;//�����Ѿ���ȡ�����ֽ���
	bool bOK = ReadFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER),//��ȡ�ļ�������ȡ������Ϣ������dwBytesRead��
		&dwBytesRead, nullptr);//bOK��ʾ�Ƿ��ȡ�ɹ�
	if ((!bOK) || (dwBytesRead != sizeof(BITMAPFILEHEADER)) || (bmfHeader.bfType != 0x4D42))//����ļ�ͷ�Ƿ��ȡ�ɹ�
	{
		CloseHandle(hFile);
		return false;
	}

	BITMAPINFO* pBitmapInfo = (new BITMAPINFO);//����λͼ��Ϣ
	if (pBitmapInfo != nullptr)
	{
		// ��ȡλͼͷ��Ϣ
		bOK = ReadFile(hFile, pBitmapInfo, sizeof(BITMAPINFOHEADER),&dwBytesRead, nullptr);//���λͼͷ����Ϣ�Ƿ��ȡ�ɹ�
		if ((!bOK) || (dwBytesRead != sizeof(BITMAPINFOHEADER)))                           
		{                                                                              
			CloseHandle(hFile);
			Free();
			return false;
		}

		//����λͼ�ĳ��ȺͿ���
		m_iWidth = static_cast<int>(pBitmapInfo->bmiHeader.biWidth);//��ȡ����
		m_iHeight = static_cast<int>(pBitmapInfo->bmiHeader.biHeight);//��ȡ�߶�
		// �õ�һ��λͼ������������λͼ������
		PBYTE pBitmapBits;//����λͼ����
		m_hBitmap = CreateDIBSection(hDC, pBitmapInfo, DIB_RGB_COLORS,(PVOID*)&pBitmapBits, nullptr, 0);//����һ���������λͼ��
		if ((m_hBitmap != nullptr) && (pBitmapBits != nullptr))
		{
			SetFilePointer(hFile, bmfHeader.bfOffBits, nullptr, FILE_BEGIN);
			bOK = ReadFile(hFile, pBitmapBits, pBitmapInfo->bmiHeader.biSizeImage,
				&dwBytesRead, nullptr);
			if (bOK)
				return true;
		}
	}

	//���ִ���ִ������
	Free();
	return false;
}

bool Bitmap::Create(HDC hDC, UINT uiResID, HINSTANCE hInstance)
{
	// Free any previous DIB info
	Free();

	// Find the bitmap resource
	HRSRC hResInfo = FindResource(hInstance, MAKEINTRESOURCE(uiResID), RT_BITMAP);
	if (hResInfo == NULL)
		return FALSE;

	// Load the bitmap resource
	HGLOBAL hMemBitmap = LoadResource(hInstance, hResInfo);
	if (hMemBitmap == NULL)
		return FALSE;

	// Lock the resource and access the entire bitmap image
	PBYTE pBitmapImage = (BYTE*)LockResource(hMemBitmap);
	if (pBitmapImage == NULL)
	{
		FreeResource(hMemBitmap);
		return FALSE;
	}

	// Store the width and height of the bitmap
	BITMAPINFO* pBitmapInfo = (BITMAPINFO*)pBitmapImage;
	m_iWidth = (int)pBitmapInfo->bmiHeader.biWidth;
	m_iHeight = (int)pBitmapInfo->bmiHeader.biHeight;

	// Get a handle to the bitmap and copy the image bits
	PBYTE pBitmapBits;
	m_hBitmap = CreateDIBSection(hDC, pBitmapInfo, DIB_RGB_COLORS,
		(PVOID*)&pBitmapBits, NULL, 0);
	if ((m_hBitmap != NULL) && (pBitmapBits != NULL))
	{
		const PBYTE pTempBits = pBitmapImage + pBitmapInfo->bmiHeader.biSize +
			pBitmapInfo->bmiHeader.biClrUsed * sizeof(RGBQUAD);
		CopyMemory(pBitmapBits, pTempBits, pBitmapInfo->bmiHeader.biSizeImage);

		// Unlock and free the bitmap graphics object
		UnlockResource(hMemBitmap);
		FreeResource(hMemBitmap);
		return TRUE;
	}

	// Something went wrong, so cleanup everything
	UnlockResource(hMemBitmap);
	FreeResource(hMemBitmap);
	Free();
	return FALSE;
}

bool Bitmap::Create(HDC hDC, int iWidth, int iHeight, COLORREF crColor)
{
	m_hBitmap = CreateCompatibleBitmap(hDC, iWidth, iHeight);
	if (m_hBitmap == nullptr)
		return false;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBRUSH hBrush = CreateSolidBrush(crColor);
	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemDC, m_hBitmap));
	RECT rcBitmap = { 0,0,m_iWidth,m_iHeight };
	FillRect(hMemDC, &rcBitmap, hBrush);
	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(hBrush);
	return true;
}

void Bitmap::Draw(HDC hDC, int x, int y, bool bTrans, COLORREF crTransColor)
{
	if (m_hBitmap != nullptr)
	{
		HDC hMemDC = CreateCompatibleDC(hDC);
		HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemDC, m_hBitmap));
		if (bTrans)
			TransparentBlt(hDC, x, y, GetWidth(), GetHeight(), hMemDC, 0, 0, GetWidth(), GetHeight(), crTransColor);
		else
			BitBlt(hDC, x, y, GetWidth(), GetHeight(), hMemDC, 0, 0, SRCCOPY);
		SelectObject(hDC, hOldBitmap);
		DeleteDC(hMemDC);
	}
}