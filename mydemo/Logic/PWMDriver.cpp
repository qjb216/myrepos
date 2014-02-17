// PWMDrive.cpp: implementation of the CPWMDrive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PWMDriver.h"
#include "../showmsgdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const JGInt16 FREQ_VALUE = 50;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPWMDriver::CPWMDriver()
{
	m_hPWMDriver = INVALID_HANDLE_VALUE;

	OpenDriver();
}

CPWMDriver::~CPWMDriver()
{
	CloseDriver();
}

// ��������
JGBOOL CPWMDriver::OpenDriver()
{
	// �� PWM ����
	m_hPWMDriver = CreateFile(TEXT("PWM1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (m_hPWMDriver == INVALID_HANDLE_VALUE)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("�� PWM ����ʧ��!"));
		dlg.DoModal();
		return false;
	}

 	JGByte prescale[2] = {0, 24};		
	JGByte divider[2]  = {0, 2};

	for (JGByte j = 0; j < 4; j++)
	{
		prescale[0] = j;
		divider[0]  = j;

		// ����PWM��ʱ��Ԥ��Ƶֵ
		BOOL ret = ::DeviceIoControl(m_hPWMDriver, IOCTL_PWM_SET_PRESCALER, prescale, 2, 
			NULL, 0, NULL, NULL);
		if (ret != TRUE)
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� PWM ��ʱ��Ԥ��Ƶֵʧ��!"));
			dlg.DoModal();
			return false;
		}
		// 	
		// ����PWM��ʱ����Ƶֵ
		ret = ::DeviceIoControl(m_hPWMDriver, IOCTL_PWM_SET_DIVIDER, divider, 2, 
			NULL, 0, NULL, NULL);
		if (ret != TRUE)
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� PWM ��ʱ����Ƶֵʧ��!"));
			dlg.DoModal();
			return false;
		}
	}

	return true;
}

// �ر�����
JGBOOL CPWMDriver::CloseDriver()
{
	if (m_hPWMDriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPWMDriver);
		m_hPWMDriver = INVALID_HANDLE_VALUE;
	}

	return true;
}

JGBOOL CPWMDriver::SetPWMValue( JGUInt8 u8Way, JGInt16 pwmValue )
{
	// 0�� 1450 ת���� 50 �� 0
	JGInt16 uValue = FREQ_VALUE - JGInt16((pwmValue * 1.0f) / MAXTEPWM * FREQ_VALUE);

	if (uValue >= FREQ_VALUE)
	{
		uValue = 49; // ��С����49������
	}

	BOOL ret;
	
	// ���� PWM����ź�
	DWORD buff[3] ;
	buff[0] = u8Way;
	buff[1] = FREQ_VALUE;
	buff[2] = uValue; 
	ret = ::DeviceIoControl(m_hPWMDriver, IOCTL_PWM_START, buff, 3, NULL, 0, NULL, NULL);
	if (ret != TRUE)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� PWM �ź����ʧ��!"));
		dlg.DoModal();
		return false;
	}
	
	// ��ȡ PWM���Ƶ��
	DWORD timer = u8Way, curfreq, actlen;
	ret = ::DeviceIoControl(m_hPWMDriver, IOCTL_PWM_GET_FREQUENCY, &timer, 1, &curfreq, 1, &actlen, NULL);
	if (ret != TRUE) 
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("��ȡ PWM �ź����Ƶ��ʧ��!"));
		dlg.DoModal();
		return false;
	}	

	return true;
}