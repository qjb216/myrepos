// BmpStatic.cpp : implementation file
//

#include "stdafx.h"
#include "temperaturectrl.h"
#include "BmpStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBmpStatic

CBmpStatic::CBmpStatic()
{
}

CBmpStatic::~CBmpStatic()
{
}


BEGIN_MESSAGE_MAP(CBmpStatic, CStatic)
	//{{AFX_MSG_MAP(CBmpStatic)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBmpStatic message handlers

void CBmpStatic::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
//	Draw(dc);
	// Do not call CStatic::OnPaint() for painting messages
}


void CBmpStatic::Draw(CPaintDC dc)
{
	//CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CDC* pDC = CDC::FromHandle(GetDlgItem(IDC_STATIC_CHOOSEFILE)->GetDC()->m_hDC);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_FILEOPEN);
	BITMAP bmp;
	bitmap.GetBitmap(&bmp);
	
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap* pold = memDC.SelectObject(&bitmap);
	
	pDC->BitBlt(0,0,bmp.bmWidth,bmp.bmHeight, &memDC, 0, 0, SRCCOPY);
	
	pDC->SetBkMode(TRANSPARENT);
	CFont* fontold = (CFont*)pDC->SelectObject(&m_DrawTitle);
	CRect rt;
	GetDlgItem(IDC_STATIC_CHOOSEFILE)->GetClientRect(&rt);
	pDC->DrawText(m_strText, &rt, DT_LEFT | DT_VCENTER);
	pDC->SelectObject(fontold);
	
	memDC.SelectObject(pold);
	memDC.DeleteDC();
	bitmap.DeleteObject();
	ReleaseDC(pDC);
}
