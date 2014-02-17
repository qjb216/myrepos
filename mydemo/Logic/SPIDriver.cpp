// SPIDriver.cpp: implementation of the CSPIDriver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SPIDriver.h"
#include "ate.h"
#include "../showmsgdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


const JGInt16 MAXTETMP = 10500;
const JGInt16 MINTETMP = -500;

// 温度个数：
const JGInt16 SAMPLETABNUM = 111;

//模块温度采样表 -5.0 ～ 105
const JGUInt16 SampleTab[SAMPLETABNUM]={
	62845,62559,62262,61953,61633,61301,60957,60601,60232,59851,
	59458,59051,58632,58200,57755,57296,56825,56340,55842,55332,
	54808,54271,53722,53160,52586,51999,51400,50790,50168,49535,
	48891,48236,47572,46898,46214,45522,44821,44112,43397,42674,
	41945,41210,40470,39725,38977,38225,37470,36712,35953,35193,
	34433,33672,32912,32153,31396,30641,29889,29140,28395,27654,
	26918,26187,25462,24743,24030,23324,22625,21933,21249,20573,
	19905,19246,18596,17954,17322,16698,16085,15480,14886,14301,
	13726,13160,12605,12060,11524,10998,10482,9976,9480,8994,
	8517,8050,7593,7145,6706,6277,5857,5446,5044,4651,
	4266,3890,3523,3164,2812,2470,2134,1807,1487,1175,
	870
};


CSPIDriver::CSPIDriver()
{
	m_hSPIDriver = INVALID_HANDLE_VALUE;

	OpenDriver();
}

CSPIDriver::~CSPIDriver()
{
	CloseDriver();
}

// 开启驱动
JGBOOL CSPIDriver::OpenDriver()
{
	// 打开 PWM 驱动
	m_hSPIDriver = CreateFile(TEXT("SPI1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (m_hSPIDriver == INVALID_HANDLE_VALUE)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("打开 SPI 驱动失败!"));
		dlg.DoModal();
		return false;
	}

	m_hAteDriver = CreateFile(TEXT("ATE1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (m_hSPIDriver == INVALID_HANDLE_VALUE)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("打开 ATE 驱动失败!"));
		dlg.DoModal();
		return false;
	}
	
	return true;
}

// 关闭驱动
JGBOOL CSPIDriver::CloseDriver()
{
	if (m_hSPIDriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hSPIDriver);
		m_hSPIDriver = INVALID_HANDLE_VALUE;
	}
	
	if (m_hAteDriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hAteDriver);
		m_hAteDriver = INVALID_HANDLE_VALUE;
	}

	return true;
}

JGBOOL CSPIDriver::GetTemp(JGUInt8& nTemp)
{
	JGUInt8 uValue = 0;
	JGUInt32 actlen = 0;
	
	if (!::DeviceIoControl(m_hAteDriver, IOCTL_ATE_TEMP, NULL, 1, &uValue, 1,  &actlen, NULL))
	{
		CShowMsgDlg dlg;
//		MessageBox(NULL,TEXT("Open ATE Error!"), TEXT("Error"), MB_OK);
		dlg.SetText(TEXT("Open ATE Error!"));
		dlg.DoModal();
		return false;
	}

	nTemp = uValue;
	return TRUE;
}

JGBOOL CSPIDriver::GetTemp( JGUInt8 u8Way, JGInt16 & nTemp )
{
	BOOL bRet = FALSE;

	// 先写
	//	if (!::DeviceIoControl(m_hSPIDriver, IOCTL_SPI_WR, &u8Way, 1, NULL, 0, NULL, NULL))
	//	{
	
	//		return false;
	//	}

	// 再读
	JGUInt16 uValue = 0;
	JGUInt32 actlen = 0;
	if(u8Way==etw_Left)
	{	
		if (!::DeviceIoControl(m_hSPIDriver, IOCTL_SPI_LEFT, NULL, 1, &uValue, 1,  &actlen, NULL))
		{
		  return false;
		}
	}
    if(u8Way==etw_Middle)
	{	
		if (!::DeviceIoControl(m_hSPIDriver, IOCTL_SPI_MIDDLE, NULL, 1, &uValue, 1,  &actlen, NULL))
		{
		  return false;
		}
	}
	if(u8Way==etw_Right)
	{	
		if (!::DeviceIoControl(m_hSPIDriver, IOCTL_SPI_RIGHT, NULL, 1, &uValue, 1,  &actlen, NULL))
		{
		  return false;
		}
	}
	if(u8Way == etw_First)
	{ 
		if (!::DeviceIoControl(m_hSPIDriver, IOCTL_SPI_FIRST, NULL, 1, &uValue, 1,  &actlen, NULL))
		{
			MessageBox(NULL,TEXT("First Read Error"),NULL,MB_OK);
			return false;
		}
		
	}

	return ChangeValue2Temp(uValue, nTemp);

}

JGBOOL CSPIDriver::ChangeValue2Temp( JGUInt16 uValue, JGInt16 & nTemp )
{
	int i,j,k;
	
	j=uValue;
	
	if(j>62845)														// 开路检测
	{
		nTemp = MINTETMP;
		return false;
	}
	else if(j<0)													// 短路检测
    {
		return false;
	}
	i = 0;
	k = SAMPLETABNUM - 1;
	while(1)
	{
		if(j < SampleTab[SAMPLETABNUM / 2])
		{
			if(j < SampleTab[SAMPLETABNUM * 3 / 4])
			{
				i = SAMPLETABNUM * 3 / 4;
				k = SAMPLETABNUM - 1;
			}
			else
			{
				i = SAMPLETABNUM / 2;
				k = SAMPLETABNUM * 3 / 4;
			}
			
		}
		else
		{
			if(j < SampleTab[SAMPLETABNUM / 4])
			{
				i = SAMPLETABNUM / 4;
				k = SAMPLETABNUM / 2;
			}
			else
			{
				i = 0;
				k = SAMPLETABNUM / 4;
			}
		}
		break;
	}
	
	for(; i < k; i++)
	{
		if(j >= SampleTab[i+1] && j <= SampleTab[i])
		{
			nTemp = (i-5)*100 + ((FLOAT)(SampleTab[i] - j)/(SampleTab[i] - SampleTab[i+1]))*100;
			return true;
		}
	}
	nTemp = MAXTETMP;

	return true;
}