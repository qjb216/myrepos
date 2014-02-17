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

	// PWM对应的路线
	enum EPWMWay
	{
		epw_Right = 0,		// 风扇
		epw_Fan   = 1,		// 右路
		epw_Left  = 2,		// 左路
		epw_Middle= 3		// 中路
	};

	JGBOOL SetPWMValue(JGUInt8 u8Way, JGInt16 pwmValue);
	
protected:

	// 开启驱动
	JGBOOL	OpenDriver();

	// 关闭驱动
	JGBOOL	CloseDriver();	

private:

	HANDLE m_hPWMDriver;

};

#endif // !defined(AFX_PWMDRIVE_H__FD402504_DBB4_4E62_856C_85E81610EA98__INCLUDED_)
