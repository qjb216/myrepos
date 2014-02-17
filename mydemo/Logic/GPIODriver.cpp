// GPIODriver.cpp: implementation of the CGPIODriver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GPIODriver.h"
#include "../showmsgdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// �ӿ�˵��...
// 	// ����
const JGUInt8 GPG6_IN = 6;		// M384ģ�� 0
const JGUInt8 GPG7_IN = 7;		// M0.2ģ�� 0
//const JGUInt8 GPG8_IN = 8;		// M0.5ģ�� 0
const JGUInt8 GPG9_IN = 9;		// 0.5ģ�� 0
const JGUInt8 GPG11_IN = 11;	// �ȸǱ��� 0 ��ʾ���� 1 ��ʾ������

// 	// ���
const JGUInt8 GPG10_OUT = 10;	// ��Դ 0 
const JGUInt8 GPF0_OUT = 0;		// ���� 0
const JGUInt8 GPF2_OUT = 2;		// ���� 0 �� 1 ��
const JGUInt8 GPG0_OUT = 0;		// ��   1 ���� 0 ����
const JGUInt8 GPG3_OUT = 3;		// ��	1 ���� 0 ����
const JGUInt8 GPG5_OUT = 5;		// ��	1 ���� 0 ����

const JGUInt8 GPE2_OUT = 2;		// ģ��ʹ��
const JGUInt8 GPE10_OUT = 10;	// �ȸ� 1 ���� 0 ����

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGPIODriver::CGPIODriver()
{
	// ������...	
	m_hGPIODriver = INVALID_HANDLE_VALUE;

	// �� GPIO ����
	OpenDriver();

}

CGPIODriver::~CGPIODriver()
{
	CloseDriver();
}


JGBOOL CGPIODriver::OpenDriver()
{
	m_hGPIODriver = ::CreateFile(_T("PIO1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	
	if (m_hGPIODriver == INVALID_HANDLE_VALUE)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("�� GPIO ����ʧ��!"));
		dlg.DoModal();
		return false;
	}
	
	return true;
}

JGBOOL CGPIODriver::CloseDriver()
{
	if (m_hGPIODriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hGPIODriver);
		m_hGPIODriver = INVALID_HANDLE_VALUE;
	}
	return true;
}

// ��ȡ�ȸ���Ϣ �Ƿ��Ѿ��رպ���
JGBOOL CGPIODriver::IsLidReady()
{
	JGBOOL bReady = false;
	
	JGUInt8 uPinNum = GPG11_IN;

	// ���� GPG11 Ϊ�����
	BOOL bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_IN, &uPinNum, 1, NULL, 0, NULL, NULL);
	if (!bRet)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPG11 ��������ʧ��!"));
		dlg.DoModal();
		return bReady;
	}

	// ��ȡ����...
	JGUInt8 status = 0;
	bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);

	return status ? true : false;
}

JGUInt8 CGPIODriver::GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9)
{
	JGUInt8 uReturn = 0;
	
	// ͬʱ��������GPIO��Ϊ����״̬..
	JGUInt32 IOMask = (0x01 << GPG6_IN) + (0x01 << GPG7_IN) + (0x01 << GPG9_IN);
	BOOL bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_IN, &IOMask,1, NULL, 0, NULL, NULL);
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_MULTI_PIN_PULLUP, &IOMask,1, NULL, 0, NULL, NULL);
	if (!bRet || !bReturn)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPG6 7 9 ��������ʧ��!"));	
		dlg.DoModal();
		return uReturn;
	}
	
	// ���ж�384..
	JGUInt8 uPinNum = GPG6_IN;
	JGUInt8 status = 1;
	bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
	u6 = status;

	status  = 1;
	uPinNum = GPG7_IN;
	bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
	u7 = status;		
	
	status  = 1;
	uPinNum = GPG9_IN;
	bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
	u9 = status;
	
	return (u6<<2) | (u7<<1) | u9;
}

// ��ȡģ����Ϣ...
JGUInt8 CGPIODriver::GetModeType()
{
	JGUInt8 uReturn = 0;

	// ͬʱ��������GPIO��Ϊ����״̬..
	JGUInt32 IOMask = (0x01 << GPG6_IN) + (0x01 << GPG7_IN) + (0x01 << GPG9_IN);
	BOOL bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_IN, &IOMask,1, NULL, 0, NULL, NULL);
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_MULTI_PIN_PULLUP, &IOMask,1, NULL, 0, NULL, NULL);
	if (!bRet || !bReturn)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPG6 7 9 ��������ʧ��!"));	
		dlg.DoModal();
		return uReturn;
	}
		
	// ���ж�384..
	JGUInt8 uPinNum = GPG6_IN;
	JGUInt8 status = 1;
	bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
	if (bRet && status == 0)
	{
		// Ϊ384 ģ��
		uReturn |= eMode384;
	}
	else
	{
		status  = 1;
		uPinNum = GPG7_IN;
		bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
		if (bRet && status == 0)
		{
			// Ϊ0.2 ģ��
			uReturn |= eMode0_2;
		}		
		
		status  = 1;
		uPinNum = GPG9_IN;
		bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
		if (bRet && status == 0)
		{
			// Ϊ0.5 ģ��
			uReturn |= eMode0_5;
		}
	}
	
	return uReturn;
}

// ���õ�Դ����
JGBOOL CGPIODriver::ModelPower(JGBOOL bOpen /*= true*/)
{
	JGUInt8 uPinNum = GPG10_OUT;
	
	// ���� GPG10 Ϊ�����
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPG10 �������ʧ��!"));	
		dlg.DoModal();
		return false;
	}

	// ����
	if (bOpen)
	{
		// �� GPG10 Ϊ�͵�ƽ,������Դ
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPG10 �͵�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{
		// �� GPG10 Ϊ�ߵ�ƽ,�رյ�Դ
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPG10 �ߵ�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}

	return true;
}

// ���ñ���
JGBOOL CGPIODriver::Sound(JGBOOL bAlarm /*= true*/)
{
	JGUInt8 uPinNum = GPF2_OUT;
	::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_EN_PULLUP, &uPinNum,1, NULL, 0, NULL, NULL);
	// ���� GPF2 Ϊ�����
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPF2 �������ʧ��!"));	
		dlg.DoModal();
		return false;
	}

	// ����
	if (bAlarm)
	{
		// �� GPF2 Ϊ�͵�ƽ,��������
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPF2 �͵�ƽʧ��!"));
			dlg.DoModal();
			return false;
		}
	}
	else
	{
		// �� GPF2 Ϊ�ߵ�ƽ,�ر�����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPF2 �ߵ�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}

	return  true;
}

// �����������Ҫ�򿪵�����
JGBOOL CGPIODriver::EnableSPI(JGBOOL bEnable/*= true*/)
{
	JGUInt8 uPinNum = GPF0_OUT;
	
	// ���� GPF0 Ϊ�����
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPF0 �������ʧ��!"));	
		dlg.DoModal();
		return false;
	}

	// ����
	if (bEnable)
	{
		// �� GPF0 Ϊ�͵�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPF0 �͵�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{
		// �� GPF0 Ϊ�ߵ�ƽ,�ر�
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPH_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPF0 �ߵ�ƽʧ��!"));
			dlg.DoModal();
			return false;
		}
	}

	return true;
}

// �����¶ȿ���...
JGBOOL CGPIODriver::TrimModelTemp(JGUInt8 u8Way, JGBOOL bAsc)	
{
	JGUInt32 uPinNum = 0;
    
	switch (u8Way)
	{
	case etw_Left:
		{
		uPinNum = 5;
		::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_PULLUP, &uPinNum,1, NULL, 0, NULL, NULL);
	    if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
		    dlg.SetText(_T("���� GPG  �������ʧ��!"));	
			dlg.DoModal();
		     return false;
		}
	
	    if (bAsc)
		{
		// �� GPG5 Ϊ�ߵ�ƽ,����
	      	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
			   dlg.SetText(_T("���� GPG �ߵ�ƽʧ��!"));
			   dlg.DoModal();
			   return false;
			}
		}
	    else
		{	
		// �� GPG5 Ϊ�͵�ƽ,����
	     	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		      	dlg.SetText(_T("���� GPG �͵�ƽʧ��!"));
				dlg.DoModal();
		     	return false;
			}
		}
		break;
		}
case etw_Middle:
	{
		uPinNum = 3;
		::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_PULLUP, &uPinNum,1, NULL, 0, NULL, NULL);
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
		    dlg.SetText(_T("���� GPG  �������ʧ��!"));	
			dlg.DoModal();
	     	return false;
		}
	
	    if (bAsc)
		{
		// �� GPG5 Ϊ�ߵ�ƽ,����
		    if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		     	dlg.SetText(_T("���� GPG �ߵ�ƽʧ��!"));
				dlg.DoModal();
		      	return false;
			}
		}
	    else
		{	
		// �� GPG5 Ϊ�͵�ƽ,����
	    	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		    	dlg.SetText(_T("���� GPG �͵�ƽʧ��!"));
				dlg.DoModal();
		    	return false;
			}
		}
		break;
	}

case etw_Right:
	{	
		uPinNum = 0;
	    ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_PULLUP, &uPinNum,1, NULL, 0, NULL, NULL);
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
	    	dlg.SetText(_T("���� GPG  �������ʧ��!"));	
			dlg.DoModal();
	    	return false;
		}
	
    	if (bAsc)
		{
		// �� GPG5 Ϊ�ߵ�ƽ,����
		    if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		    	dlg.SetText(_T("���� GPG �ߵ�ƽʧ��!"));
				dlg.DoModal();
		        return false;
			}
		}
     	else
		{	
		// �� GPG5 Ϊ�͵�ƽ,����
	    	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
				dlg.SetText(_T("���� GPG �͵�ƽʧ��!"));
				dlg.DoModal();
				return false;
			}
		}
		break;
	}
	//default:
		//ASSERT(false);
		//return false; // ���ش���
	}
	
	// ͬʱ����GPIO��Ϊ���״̬..
	
	return true;
}

JGBOOL CGPIODriver::TrimModelTemp(JGBOOL bAsc)
{
	// ͬʱ��������GPIO��Ϊ���״̬..
	JGUInt32 IOMask = (0x01 << GPG0_OUT) + (0x01 << GPG3_OUT) + (0x01 << GPG5_OUT);
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_MULTI_PIN_PULLUP, &IOMask,1, NULL, 0, NULL, NULL);
	//if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_OUT, &IOMask, 1, NULL, 0, NULL, NULL))
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_OUT, &IOMask, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPG0 3 5 �������ʧ��!"));	
		dlg.DoModal();
		return false;
	}
    if (bAsc)
	{
		// �� GPG5 Ϊ�ߵ�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN, &IOMask, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPG5 �ߵ�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{	
		// �� GPG5 Ϊ�͵�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_MULTI_PIN, &IOMask, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPG5 �͵�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}
/*
	// �� 5
	JGUInt8 uPinNum = GPG5_OUT; 
	// ����
	if (bAsc)
	{
		// �� GPG5 Ϊ�ߵ�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("���� GPG5 �ߵ�ƽʧ��!"));	
			return false;
		}
	}
	else
	{	
		// �� GPG5 Ϊ�͵�ƽ,����
		if (! ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("���� GPG5 �͵�ƽʧ��!"));	
			return false;
		}
	}

	// �� 3
	uPinNum = GPG3_OUT; 
	// ����
	if (bAsc)
	{
		// �� GPG3 Ϊ�ߵ�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("���� GPG3 �ߵ�ƽʧ��!"));	
			return false;
		}
	}
	else
	{	
		// �� GPG3 Ϊ�͵�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("���� GPG3 �͵�ƽʧ��!"));	
			return false;
		}
	}

	// �� 0
	uPinNum = GPG0_OUT; 
	// ����
	if (bAsc)
	{
		// �� GPG0 Ϊ�ߵ�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("���� GPG0 �ߵ�ƽʧ��!"));	
			return false;
		}
	}
	else
	{	
		// �� GPG0 Ϊ�͵�ƽ,����
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("���� GPG0 �͵�ƽʧ��!"));	
			return false;
		}
	}*/

	return true;
}

JGBOOL CGPIODriver::EnableAll( JGBOOL bEnable /*= true*/ )
{
	JGUInt8 uPinNum = GPE2_OUT;
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPE 2 �������ʧ��!"));
		dlg.DoModal();
		return false;
	}

	if (bEnable)
	{
		// ��Ϊ�͵�ƽ
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_CLR_PIN, &uPinNum, 1, NULL, 0 , NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPE 2 �͵�ƽʧ��!"));
			dlg.DoModal();
			return false;
		}
	}
	else
	{	
		// ��Ϊ�ߵ�ƽ
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPE 2 �ߵ�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}

	return true;
}

JGBOOL CGPIODriver::TrimLidTemp( JGBOOL bAsc )
{
	JGUInt8 uPinNum = GPE10_OUT;
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_EN_PULLUP, &uPinNum,1, NULL, 0, NULL, NULL);
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("���� GPE 10 �������ʧ��!"));
		dlg.DoModal();
		return false;
	}
	
	if (bAsc)
	{
		// ��Ϊ�͵�ƽ
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPE 10 �͵�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{	
		// ��Ϊ�ߵ�ƽ
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("���� GPE 10 �ߵ�ƽʧ��!"));	
			dlg.DoModal();
			return false;
		}
	}
	
	return true;	
}