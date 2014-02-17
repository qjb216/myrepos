// JGDriverMgr.cpp: implementation of the CJGDriverMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JGDriverMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CJGDriverMgr * CJGDriverMgr::s_pInstance = NULL;
CJGDriverMgr::CJGDriverMgr()
{

}

CJGDriverMgr::~CJGDriverMgr()
{

}

JGBOOL CJGDriverMgr::SetLeftPWMValue( JGInt16 pwmValue )
{
#if JG_WCE_MODE == 1
	return m_pwm.SetPWMValue(CPWMDriver::epw_Left, pwmValue);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::SetRightPWMValue( JGInt16 pwmValue )
{
#if JG_WCE_MODE == 1
	return m_pwm.SetPWMValue(CPWMDriver::epw_Right, pwmValue);
#else
	return false;
#endif 
}

JGBOOL CJGDriverMgr::SetMiddlePWMValue( JGInt16 pwmValue )
{
#if JG_WCE_MODE == 1
	return m_pwm.SetPWMValue(CPWMDriver::epw_Middle, pwmValue);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::SetFanPWMValue( JGInt16 pwmValue )
{
#if JG_WCE_MODE == 1
	return m_pwm.SetPWMValue(CPWMDriver::epw_Fan, pwmValue);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::SetTempCtrl( JGUInt8 u8Way, JGBOOL bAsc )
{
#if JG_WCE_MODE == 1
	return m_gpio.TrimModelTemp(u8Way,bAsc);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::SetTempCtrl( JGBOOL bAsc )
{
#if JG_WCE_MODE == 1
	return m_gpio.TrimModelTemp(bAsc);	
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::GetEnvTemp(JGUInt8 & nTemp)
	{

#if JG_WCE_MODE == 1
	byte nTemp0 = 0;
	int ii = 0;
	BOOL nRes;
	do
	{
		Sleep(2);
		nRes = m_spi.GetTemp(nTemp0);
		ii++;
		if(ii >100)
		{
			nTemp = 20;
			return FALSE;
		}
	}
	while(nTemp0 >= 60);
	nTemp = nTemp0;
	return nRes;	
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::GetModelTemp( JGUInt8 u8Way, JGInt16 & nTemp )
{
#if JG_WCE_MODE == 1
	return m_spi.GetTemp(u8Way, nTemp);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::GetRadTemp( JGInt16 & nTemp )
{
#if JG_WCE_MODE == 1
	return m_adc.GetTemp(CADCDriver::ec_Rad, nTemp);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::GetLidTemp( JGInt16 & nTemp )
{
#if JG_WCE_MODE == 1
	return m_adc.GetTemp(CADCDriver::ec_Lid, nTemp);
#else
	static long nT = 1000;
	nT += 100;
	nTemp = nT;
	return false;
#endif
}

JGBOOL CJGDriverMgr::SetLidTemp( JGBOOL bAsc )
{
#if JG_WCE_MODE == 1
	return m_gpio.TrimLidTemp(bAsc);
#else
	return false;
#endif
}

JGBOOL CJGDriverMgr::Alarm( JGUInt8 uTime )
{
#if JG_WCE_MODE == 1
	JGBOOL bRet = false;

	EnterCriticalSection(&m_csSound);

	if (m_gpio.Sound(true))
	{
		// 等待时间
		Sleep(uTime * 10);

		bRet = m_gpio.Sound(false);
	}

	LeaveCriticalSection(&m_csSound);

	return bRet;
#else
	return false;
#endif
}

// 设置模块的电源开关
JGBOOL CJGDriverMgr::ModelPower(JGBOOL bOpen /*= true*/)
{
#if JG_WCE_MODE == 1
	return m_gpio.ModelPower(bOpen);
#else
	return false;
#endif
}

// 使能，全部芯片有效
JGBOOL CJGDriverMgr::EnableAll(JGBOOL bEnable /*= true*/)
{
#if JG_WCE_MODE == 1
	return m_gpio.EnableAll(bEnable);
#else
	return false;
#endif
}

// 使SPI芯片有效
JGBOOL CJGDriverMgr::EnableSPI(JGBOOL bEnable /*= true*/)
{
#if JG_WCE_MODE == 1
	return m_gpio.EnableSPI(bEnable);
#else
	return false;
#endif
}

JGUInt8 CJGDriverMgr::GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9)
{
#if JG_WCE_MODE == 1
	return m_gpio.GetType(u6, u7, u9);
#else
	return false;
#endif
}

// 得到模块型号
JGUInt8 CJGDriverMgr::GetModeType()
{
#if JG_WCE_MODE == 1
	return m_gpio.GetModeType();
#else
	return false;
#endif
}

void CJGDriverMgr::InitCriticalSection()
{
#if JG_WCE_MODE == 1
	::InitializeCriticalSection(&m_csSound);
#else
	return;
#endif
}

void CJGDriverMgr::FreeCriticalSection()
{
#if JG_WCE_MODE == 1
	::DeleteCriticalSection(&m_csSound);
#else
	return ;
#endif
}

void CJGDriverMgr::DisableAllPWM()
{
#if JG_WCE_MODE == 1
	CJGDriverMgr::Instance()->EnableAll(false);	
	CJGDriverMgr::Instance()->SetRightPWMValue(0);
	CJGDriverMgr::Instance()->SetLeftPWMValue(0);
	CJGDriverMgr::Instance()->SetMiddlePWMValue(0);
#else
	return ;
#endif
}
