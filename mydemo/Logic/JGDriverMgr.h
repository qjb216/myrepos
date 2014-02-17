// JGDriverMgr.h: interface for the CJGDriverMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JGDRIVERMGR_H__F17B1FFB_3936_41D0_882F_7407D4459328__INCLUDED_)
#define AFX_JGDRIVERMGR_H__F17B1FFB_3936_41D0_882F_7407D4459328__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GPIODriver.h"
#include "ADCDriver.h"
#include "PWMDriver.h"
#include "SPIDriver.h"

class CJGDriverMgr  
{
public:
	static CJGDriverMgr * Instance()
	{
		if (s_pInstance == NULL)
		{
			s_pInstance = new CJGDriverMgr();	
		}
		return s_pInstance;
	}
	
	static void Release()
	{
		if (NULL != s_pInstance)
		{
			delete s_pInstance;
			s_pInstance = NULL;
		}
	}

public:
	void DisableAllPWM();
	// 设置PWM值：

	// 设置左路PWM
	JGBOOL SetLeftPWMValue(JGInt16 pwmValue);

	// 设置中路PWM
	JGBOOL SetRightPWMValue(JGInt16 pwmValue);

	// 设置右路PWM
	JGBOOL SetMiddlePWMValue(JGInt16 pwmValue);

	// 风扇PWM
	JGBOOL SetFanPWMValue(JGInt16 pwmValue);

	// 设置u8Way路的模块加热或制冷
	JGBOOL SetTempCtrl(JGUInt8 u8Way, JGBOOL bAsc);	
	
	// 设置三路的模块加热或制冷
	JGBOOL SetTempCtrl(JGBOOL bAsc);

	// 设置热盖的温度模式：
	JGBOOL SetLidTemp(JGBOOL bAsc);
	
	//获取环境温度
	JGBOOL GetEnvTemp(JGUInt8 & nTemp);

	// 得到模块的温度：
	JGBOOL GetModelTemp(JGUInt8 u8Way, JGInt16 & nTemp);
	
	// 得到散热器的温度
	JGBOOL GetRadTemp(JGInt16 & nTemp);
	  
	// 得到热盖的温度
	JGBOOL GetLidTemp(JGInt16 & nTemp);

	// 报鸣
	// uTime 响声时间 单位10豪秒
	JGBOOL Alarm(JGUInt8 uTime); 

	// 设置模块的电源开关
	JGBOOL ModelPower(JGBOOL bOpen = true);

	// 使能，全部芯片有效
	JGBOOL EnableAll(JGBOOL bEnable = true);
	
	// 使SPI芯片有效
	JGBOOL EnableSPI(JGBOOL bEnable = true);
	
	// 得到模块型号
	JGUInt8 GetModeType();

	JGUInt8 GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9);

protected:

	// 初始化所有关键代码
	void InitCriticalSection();

	// 删除所有关键代码
	void FreeCriticalSection();

private:
	CJGDriverMgr();
	virtual ~CJGDriverMgr();
	static CJGDriverMgr * s_pInstance;

#if JG_WCE_MODE == 1
	CGPIODriver m_gpio;
	CADCDriver  m_adc;
	CPWMDriver	m_pwm;
	CSPIDriver  m_spi;
	// 报警的关键码
	CRITICAL_SECTION m_csSound;
#endif

};

#endif // !defined(AFX_JGDRIVERMGR_H__F17B1FFB_3936_41D0_882F_7407D4459328__INCLUDED_)
