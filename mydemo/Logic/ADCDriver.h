// ADCDriver.h: interface for the CADCDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADCDRIVER_H__59144E6B_1C63_4455_B75A_68444BF0ED9C__INCLUDED_)
#define AFX_ADCDRIVER_H__59144E6B_1C63_4455_B75A_68444BF0ED9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drive.h"

class CJGDriverMgr;

class CADCDriver  
{
public:
	CADCDriver();
	virtual ~CADCDriver();

	friend CJGDriverMgr;
	
protected:

	enum EChannel
	{
		ec_Lid = 0,		// 热盖
		ec_Rad = 1		// 散热器
	};

	// 得到目标通道的温度：
	JGBOOL GetTemp(JGUInt8 u8Channel, JGInt16 & nTemp);

protected:

	// 得到目标通道的采样值；
	JGBOOL	GetValue(JGUInt8 u8Channel, JGUInt16 & uValue);

	// 转换温度；
	JGBOOL	ChangeValue2Temp(JGUInt16 uValue, JGInt16 & nTemp);

	// 开启驱动
	JGBOOL	OpenDriver();

	// 关闭驱动
	JGBOOL	CloseDriver();	

	JGBOOL GetAdcdata(JGUInt8 u8Channel, JGUInt16 uValue, JGInt16 & nTemp);		//20100716


private:
	
	HANDLE m_hADCDriver;

};

extern const JGInt16 MAXADCTMP;
extern const JGInt16 MINADCTMP;

#endif // !defined(AFX_ADCDRIVER_H__59144E6B_1C63_4455_B75A_68444BF0ED9C__INCLUDED_)
