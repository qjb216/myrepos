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
	// ����PWMֵ��

	// ������·PWM
	JGBOOL SetLeftPWMValue(JGInt16 pwmValue);

	// ������·PWM
	JGBOOL SetRightPWMValue(JGInt16 pwmValue);

	// ������·PWM
	JGBOOL SetMiddlePWMValue(JGInt16 pwmValue);

	// ����PWM
	JGBOOL SetFanPWMValue(JGInt16 pwmValue);

	// ����u8Way·��ģ����Ȼ�����
	JGBOOL SetTempCtrl(JGUInt8 u8Way, JGBOOL bAsc);	
	
	// ������·��ģ����Ȼ�����
	JGBOOL SetTempCtrl(JGBOOL bAsc);

	// �����ȸǵ��¶�ģʽ��
	JGBOOL SetLidTemp(JGBOOL bAsc);
	
	//��ȡ�����¶�
	JGBOOL GetEnvTemp(JGUInt8 & nTemp);

	// �õ�ģ����¶ȣ�
	JGBOOL GetModelTemp(JGUInt8 u8Way, JGInt16 & nTemp);
	
	// �õ�ɢ�������¶�
	JGBOOL GetRadTemp(JGInt16 & nTemp);
	  
	// �õ��ȸǵ��¶�
	JGBOOL GetLidTemp(JGInt16 & nTemp);

	// ����
	// uTime ����ʱ�� ��λ10����
	JGBOOL Alarm(JGUInt8 uTime); 

	// ����ģ��ĵ�Դ����
	JGBOOL ModelPower(JGBOOL bOpen = true);

	// ʹ�ܣ�ȫ��оƬ��Ч
	JGBOOL EnableAll(JGBOOL bEnable = true);
	
	// ʹSPIоƬ��Ч
	JGBOOL EnableSPI(JGBOOL bEnable = true);
	
	// �õ�ģ���ͺ�
	JGUInt8 GetModeType();

	JGUInt8 GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9);

protected:

	// ��ʼ�����йؼ�����
	void InitCriticalSection();

	// ɾ�����йؼ�����
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
	// �����Ĺؼ���
	CRITICAL_SECTION m_csSound;
#endif

};

#endif // !defined(AFX_JGDRIVERMGR_H__F17B1FFB_3936_41D0_882F_7407D4459328__INCLUDED_)
