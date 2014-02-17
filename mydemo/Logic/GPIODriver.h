// GPIODriver.h: interface for the CGPIODriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GPIODRIVER_H__D85053D2_4CC4_4E1E_B21F_257588AA9811__INCLUDED_)
#define AFX_GPIODRIVER_H__D85053D2_4CC4_4E1E_B21F_257588AA9811__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drive.h"

class CJGDriverMgr;

class CGPIODriver  
{
public:
	CGPIODriver();
	virtual ~CGPIODriver();

	friend CJGDriverMgr;

protected:

	// 使能，全部芯片有效
	JGBOOL EnableAll(JGBOOL bEnable = true);

	// 设置模块的电源开关
	JGBOOL ModelPower(JGBOOL bOpen = true);
	
	// 声音开关
	JGBOOL Sound(JGBOOL bAlarm = true);

	// 使SPI芯片有效
	JGBOOL EnableSPI(JGBOOL bEnable = true);

	// 获取热盖信息 是否已经关闭好了
	// 暂时没有用到！
	JGBOOL	IsLidReady();
	
	// 获取模块信息...
	JGUInt8 GetModeType();

	JGUInt8 GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9);
	
	// 调整模块的温度是加热还是制冷...
	// 三路通道 bAsc  1 表示加热，0 表示制冷
	JGBOOL TrimModelTemp(JGBOOL bAsc);
	// u8Way对应的通道
	JGBOOL TrimModelTemp(JGUInt8 u8Way, JGBOOL bAsc);
	
	// 调整热盖的温度是加热还是将热...
	JGBOOL TrimLidTemp(JGBOOL bAsc);

protected:

	// 开启驱动
	JGBOOL OpenDriver();
	
	// 关闭驱动
	JGBOOL CloseDriver();	

private:

	HANDLE m_hGPIODriver;
	
};

#endif // !defined(AFX_GPIODRIVER_H__D85053D2_4CC4_4E1E_B21F_257588AA9811__INCLUDED_)
