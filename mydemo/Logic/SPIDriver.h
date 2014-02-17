// SPIDriver.h: interface for the CSPIDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPIDRIVER_H__64F112A1_A4BD_4E53_B7DF_D70D57CF3B63__INCLUDED_)
#define AFX_SPIDRIVER_H__64F112A1_A4BD_4E53_B7DF_D70D57CF3B63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drive.h"

class CJGDriverMgr;

class CSPIDriver  
{
public:
	CSPIDriver();
	virtual ~CSPIDriver();

	friend CJGDriverMgr;
	
protected:
	JGBOOL GetTemp(JGUInt8& nTemp);	//读取环境温度
	JGBOOL GetTemp(JGUInt8 u8Way, JGInt16 & nTemp);

protected:

	// 转换温度；
	JGBOOL	ChangeValue2Temp(JGUInt16 uValue, JGInt16 & nTemp);

	// 开启驱动
	JGBOOL	OpenDriver();
	
	// 关闭驱动
	JGBOOL	CloseDriver();		
	
private:

	HANDLE m_hSPIDriver;
	HANDLE m_hAteDriver;	//季节识别传感器

};

extern const JGInt16 MAXTETMP;
extern const JGInt16 MINTETMP;
#endif // !defined(AFX_SPIDRIVER_H__64F112A1_A4BD_4E53_B7DF_D70D57CF3B63__INCLUDED_)
