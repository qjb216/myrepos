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


// 接口说明...
// 	// 输入
const JGUInt8 GPG6_IN = 6;		// M384模块 0
const JGUInt8 GPG7_IN = 7;		// M0.2模块 0
//const JGUInt8 GPG8_IN = 8;		// M0.5模块 0
const JGUInt8 GPG9_IN = 9;		// 0.5模块 0
const JGUInt8 GPG11_IN = 11;	// 热盖报警 0 表示正常 1 表示不正常

// 	// 输出
const JGUInt8 GPG10_OUT = 10;	// 电源 0 
const JGUInt8 GPF0_OUT = 0;		// 必须 0
const JGUInt8 GPF2_OUT = 2;		// 声音 0 响 1 灭
const JGUInt8 GPG0_OUT = 0;		// 右   1 加热 0 制冷
const JGUInt8 GPG3_OUT = 3;		// 中	1 加热 0 制冷
const JGUInt8 GPG5_OUT = 5;		// 左	1 加热 0 制冷

const JGUInt8 GPE2_OUT = 2;		// 模块使能
const JGUInt8 GPE10_OUT = 10;	// 热盖 1 加热 0 制冷

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGPIODriver::CGPIODriver()
{
	// 打开驱动...	
	m_hGPIODriver = INVALID_HANDLE_VALUE;

	// 打开 GPIO 驱动
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
		dlg.SetText(_T("打开 GPIO 驱动失败!"));
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

// 获取热盖信息 是否已经关闭好了
JGBOOL CGPIODriver::IsLidReady()
{
	JGBOOL bReady = false;
	
	JGUInt8 uPinNum = GPG11_IN;

	// 设置 GPG11 为输入口
	BOOL bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_IN, &uPinNum, 1, NULL, 0, NULL, NULL);
	if (!bRet)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPG11 引脚输入失败!"));
		dlg.DoModal();
		return bReady;
	}

	// 获取数据...
	JGUInt8 status = 0;
	bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);

	return status ? true : false;
}

JGUInt8 CGPIODriver::GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9)
{
	JGUInt8 uReturn = 0;
	
	// 同时设置三个GPIO口为输入状态..
	JGUInt32 IOMask = (0x01 << GPG6_IN) + (0x01 << GPG7_IN) + (0x01 << GPG9_IN);
	BOOL bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_IN, &IOMask,1, NULL, 0, NULL, NULL);
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_MULTI_PIN_PULLUP, &IOMask,1, NULL, 0, NULL, NULL);
	if (!bRet || !bReturn)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPG6 7 9 引脚输入失败!"));	
		dlg.DoModal();
		return uReturn;
	}
	
	// 先判断384..
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

// 获取模块信息...
JGUInt8 CGPIODriver::GetModeType()
{
	JGUInt8 uReturn = 0;

	// 同时设置三个GPIO口为输入状态..
	JGUInt32 IOMask = (0x01 << GPG6_IN) + (0x01 << GPG7_IN) + (0x01 << GPG9_IN);
	BOOL bRet = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_IN, &IOMask,1, NULL, 0, NULL, NULL);
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_MULTI_PIN_PULLUP, &IOMask,1, NULL, 0, NULL, NULL);
	if (!bRet || !bReturn)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPG6 7 9 引脚输入失败!"));	
		dlg.DoModal();
		return uReturn;
	}
		
	// 先判断384..
	JGUInt8 uPinNum = GPG6_IN;
	JGUInt8 status = 1;
	bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
	if (bRet && status == 0)
	{
		// 为384 模块
		uReturn |= eMode384;
	}
	else
	{
		status  = 1;
		uPinNum = GPG7_IN;
		bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
		if (bRet && status == 0)
		{
			// 为0.2 模块
			uReturn |= eMode0_2;
		}		
		
		status  = 1;
		uPinNum = GPG9_IN;
		bRet  = ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_READ_PIN, &uPinNum, 1, &status, 1, NULL, NULL);
		if (bRet && status == 0)
		{
			// 为0.5 模块
			uReturn |= eMode0_5;
		}
	}
	
	return uReturn;
}

// 设置电源开关
JGBOOL CGPIODriver::ModelPower(JGBOOL bOpen /*= true*/)
{
	JGUInt8 uPinNum = GPG10_OUT;
	
	// 设置 GPG10 为输出口
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPG10 引脚输出失败!"));	
		dlg.DoModal();
		return false;
	}

	// 设置
	if (bOpen)
	{
		// 置 GPG10 为低电平,开启电源
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPG10 低电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{
		// 置 GPG10 为高电平,关闭电源
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPG10 高电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}

	return true;
}

// 设置报警
JGBOOL CGPIODriver::Sound(JGBOOL bAlarm /*= true*/)
{
	JGUInt8 uPinNum = GPF2_OUT;
	::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_EN_PULLUP, &uPinNum,1, NULL, 0, NULL, NULL);
	// 设置 GPF2 为输出口
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPF2 引脚输出失败!"));	
		dlg.DoModal();
		return false;
	}

	// 设置
	if (bAlarm)
	{
		// 置 GPF2 为低电平,开启鸣声
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPF2 低电平失败!"));
			dlg.DoModal();
			return false;
		}
	}
	else
	{
		// 置 GPF2 为高电平,关闭鸣声
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPF2 高电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}

	return  true;
}

// 软件启动必须要打开的设置
JGBOOL CGPIODriver::EnableSPI(JGBOOL bEnable/*= true*/)
{
	JGUInt8 uPinNum = GPF0_OUT;
	
	// 设置 GPF0 为输出口
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_SET_PIN_OUT, &uPinNum, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPF0 引脚输出失败!"));	
		dlg.DoModal();
		return false;
	}

	// 设置
	if (bEnable)
	{
		// 置 GPF0 为低电平,开启
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPF_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPF0 低电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{
		// 置 GPF0 为高电平,关闭
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPH_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPF0 高电平失败!"));
			dlg.DoModal();
			return false;
		}
	}

	return true;
}

// 设置温度控制...
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
		    dlg.SetText(_T("设置 GPG  引脚输出失败!"));	
			dlg.DoModal();
		     return false;
		}
	
	    if (bAsc)
		{
		// 置 GPG5 为高电平,加热
	      	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
			   dlg.SetText(_T("设置 GPG 高电平失败!"));
			   dlg.DoModal();
			   return false;
			}
		}
	    else
		{	
		// 置 GPG5 为低电平,制冷
	     	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		      	dlg.SetText(_T("设置 GPG 低电平失败!"));
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
		    dlg.SetText(_T("设置 GPG  引脚输出失败!"));	
			dlg.DoModal();
	     	return false;
		}
	
	    if (bAsc)
		{
		// 置 GPG5 为高电平,加热
		    if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		     	dlg.SetText(_T("设置 GPG 高电平失败!"));
				dlg.DoModal();
		      	return false;
			}
		}
	    else
		{	
		// 置 GPG5 为低电平,制冷
	    	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		    	dlg.SetText(_T("设置 GPG 低电平失败!"));
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
	    	dlg.SetText(_T("设置 GPG  引脚输出失败!"));	
			dlg.DoModal();
	    	return false;
		}
	
    	if (bAsc)
		{
		// 置 GPG5 为高电平,加热
		    if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
		    	dlg.SetText(_T("设置 GPG 高电平失败!"));
				dlg.DoModal();
		        return false;
			}
		}
     	else
		{	
		// 置 GPG5 为低电平,制冷
	    	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
			{
				CShowMsgDlg dlg;
				dlg.SetText(_T("设置 GPG 低电平失败!"));
				dlg.DoModal();
				return false;
			}
		}
		break;
	}
	//default:
		//ASSERT(false);
		//return false; // 返回错误！
	}
	
	// 同时设置GPIO口为输出状态..
	
	return true;
}

JGBOOL CGPIODriver::TrimModelTemp(JGBOOL bAsc)
{
	// 同时设置三个GPIO口为输出状态..
	JGUInt32 IOMask = (0x01 << GPG0_OUT) + (0x01 << GPG3_OUT) + (0x01 << GPG5_OUT);
	BOOL bReturn =  ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_EN_MULTI_PIN_PULLUP, &IOMask,1, NULL, 0, NULL, NULL);
	//if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_OUT, &IOMask, 1, NULL, 0, NULL, NULL))
	if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN_OUT, &IOMask, 1, NULL, 0, NULL, NULL))
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("设置 GPG0 3 5 引脚输出失败!"));	
		dlg.DoModal();
		return false;
	}
    if (bAsc)
	{
		// 置 GPG5 为高电平,加热
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_MULTI_PIN, &IOMask, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPG5 高电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{	
		// 置 GPG5 为低电平,制冷
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_MULTI_PIN, &IOMask, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPG5 低电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}
/*
	// 左 5
	JGUInt8 uPinNum = GPG5_OUT; 
	// 设置
	if (bAsc)
	{
		// 置 GPG5 为高电平,加热
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("设置 GPG5 高电平失败!"));	
			return false;
		}
	}
	else
	{	
		// 置 GPG5 为低电平,制冷
		if (! ::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("设置 GPG5 低电平失败!"));	
			return false;
		}
	}

	// 中 3
	uPinNum = GPG3_OUT; 
	// 设置
	if (bAsc)
	{
		// 置 GPG3 为高电平,加热
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("设置 GPG3 高电平失败!"));	
			return false;
		}
	}
	else
	{	
		// 置 GPG3 为低电平,制冷
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("设置 GPG3 低电平失败!"));	
			return false;
		}
	}

	// 右 0
	uPinNum = GPG0_OUT; 
	// 设置
	if (bAsc)
	{
		// 置 GPG0 为高电平,加热
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("设置 GPG0 高电平失败!"));	
			return false;
		}
	}
	else
	{	
		// 置 GPG0 为低电平,制冷
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPG_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			JGMessageBox(_T("设置 GPG0 低电平失败!"));	
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
		dlg.SetText(_T("设置 GPE 2 引脚输出失败!"));
		dlg.DoModal();
		return false;
	}

	if (bEnable)
	{
		// 置为低电平
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_CLR_PIN, &uPinNum, 1, NULL, 0 , NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPE 2 低电平失败!"));
			dlg.DoModal();
			return false;
		}
	}
	else
	{	
		// 置为高电平
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPE 2 高电平失败!"));	
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
		dlg.SetText(_T("设置 GPE 10 引脚输出失败!"));
		dlg.DoModal();
		return false;
	}
	
	if (bAsc)
	{
		// 置为低电平
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_CLR_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPE 10 低电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}
	else
	{	
		// 置为高电平
		if (!::DeviceIoControl(m_hGPIODriver, IOCTL_GPE_SET_PIN, &uPinNum, 1, NULL, 0, NULL, NULL))
		{
			CShowMsgDlg dlg;
			dlg.SetText(_T("设置 GPE 10 高电平失败!"));	
			dlg.DoModal();
			return false;
		}
	}
	
	return true;	
}