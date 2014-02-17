#if !defined(AFX_BMPSTATIC_H__A43ACD45_2D76_4B5F_AEF2_03CFF42C02EA__INCLUDED_)
#define AFX_BMPSTATIC_H__A43ACD45_2D76_4B5F_AEF2_03CFF42C02EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BmpStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBmpStatic window

class CBmpStatic : public CStatic
{
// Construction
public:
	CBmpStatic();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBmpStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBmpStatic();
private:
	void Draw(CPaintDC dc);

	CString m_DrawTitle;
	CString m_strText;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBmpStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BMPSTATIC_H__A43ACD45_2D76_4B5F_AEF2_03CFF42C02EA__INCLUDED_)
