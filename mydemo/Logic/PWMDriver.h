// PWMDrive.h: interface for the CPWMDrive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PWMDRIVE_H__FD402504_DBB4_4E62_856C_85E81610EA98__INCLUDED_)
#define AFX_PWMDRIVE_H__FD402504_DBB4_4E62_856C_85E81610EA98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drive.h"

class CJGDriverMgr;

class CPWMDriver  
{
public:
	CPWMDriver();
	virtual ~CPWMDriver();

	friend CJGDriverMgr;
	
protected:

	// PWM��Ӧ��·��
	enum EPWMWay
	{
		epw_Right = 0,		// ����
		epw_Fan   = 1,		// ��·
		epw_Left  = 2,		// ��·
		epw_Middle= 3		// ��·
	};

	JGBOOL SetPWMValue(JGUInt8 u8Way, JGInt16 pwmValue);
	
protected:

	// ��������
	JGBOOL	OpenDriver();

	// �ر�����
	JGBOOL	CloseDriver();	

private:

	HANDLE m_hPWMDriver;

};

#endif // !defined(AFX_PWMDRIVE_H__FD402504_DBB4_4E62_856C_85E81610EA98__INCLUDED_)
