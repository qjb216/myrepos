// ADCDriver.cpp: implementation of the CADCDriver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ADCDriver.h"
#include "JGDriverMgr.h"
#include "../showmsgdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


const JGInt16 MAXADCTMP	= 11500;
const JGInt16 MINADCTMP	= -500;

const JGInt16 ADCTABNUM	 = 121;//25;

//////////20100716
const JGUInt16 SAM_NUM = 128;
const JGUInt8 AD_CH_MAX = 2;
JGUInt16	AdData[AD_CH_MAX];
/////////

/*
// -5 ～ 105 度
const JGUInt16 AdcTab[ADCTABNUM]={
		982,							//-5
		958,929,						//0,5
		895,856,
		812,764,
		711,655,
		597,538,
		478,420,
		364,311,
		261,214,
		172,133,
		98,67,
		39,13,
		2,1
};
*/
/*
const JGUInt16  AdcTab[ADCTABNUM]={
	3610,							//-5
		3527,3427,						//0,5
		3311,3176,
		3025,2857,
		2675,2482,
		2281,2076,
		1871,1669,
		1475,1290,
		1116,955,
		808,673,
		552,443,
		346,259,
		182,114
};
*/
/*
const JGUInt16  AdcTab[ADCTABNUM]={
	3610,3595,3578,3562,3545,							//-5
		3527,3508,3489,3469,3448, 3427,3405,3383,3359,3335,						//0,5
		3311,3285,3259,3232,3205, 3176,3147,3118,3087,3056,						//10,15
		3025,2992,2959,2926,2892, 2857,2821,2786,2749,2712,					//20,25
		2675,2637,2599,2560,2521, 2482,2442,2402,2362,2321,						//30,35
		2281,2240,2199,2158,2117, 2076,2034,1993,1952,1911,						//40,45
		1871,1830,1789,1749,1709, 1669,1630,1590,1552,1513,						//50,55
		1475,1437,1400,1363,1326, 1290,1254,1219,1184,1150,						//60,65
		1116,1083,1050,1018,986, 955,925,895,865,836,						//70,75
		808,780,752,725,699, 673,648,623,599,575,						//80,85
		552,529,507,485,464, 443,423,403,383,364,						//90,95
		346,327,310,292,275, 259,243,227,212,197,						//100,105
		182,168,154,140,127, 114							//110,115
};
*/


const JGUInt16  AdcTab[ADCTABNUM]={
	3924,3908,3890,3872,3853,							//-5
		3834,3814,3793,3771,3749, 3726,3702,3677,3652,3626,						//0,5
		3599,3571,3543,3514,3484, 3453,3421,3389,3356,3322,						//10,15
		3288,3253,3217,3181,3143, 3106,3067,3028,2989,2948,					//20,25
		2908,2867,2825,2783,2741, 2698,2655,2611,2567,2523,						//30,35
		2479,2435,2390,2346,2301, 2256,2212,2167,2122,2078,						//40,45
		2034,1989,1945,1901,1856, 1815,1772,1729,1687,1645,						//50,55
		1603,1562,1521,1481,1441, 1402,1363,1325,1287,1250,						//60,65
		1213,1177,1142,1107,1072, 1039,1005,973,940,909,						//70,75
		878,848,818,789,760, 732,704,677,651,625,						//80,85
		600,575,551,527,504, 482,459,438,417,396,						//90,95
		376,356,337,318,299, 281,264,247,230,214,						//100,105
		198,182,167,152,138, 124							//110,115
};


CADCDriver::CADCDriver()
{
	m_hADCDriver = INVALID_HANDLE_VALUE;

	OpenDriver();
}

CADCDriver::~CADCDriver()
{
	CloseDriver();
}

// 开启驱动
JGBOOL CADCDriver::OpenDriver()
{
	// 打开 ADC 驱动
	m_hADCDriver = CreateFile(TEXT("ADC1:"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	if (m_hADCDriver == INVALID_HANDLE_VALUE)
	{
		CShowMsgDlg dlg;
		dlg.SetText(_T("打开 ADC 驱动失败!"));
		dlg.DoModal();
		return false;
	}
	
	return true;
}

// 关闭驱动
JGBOOL CADCDriver::CloseDriver()
{
	if (m_hADCDriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hADCDriver);
		m_hADCDriver = INVALID_HANDLE_VALUE;
	}
	
	return true;
}


JGBOOL CADCDriver::GetTemp( JGUInt8 u8Channel, JGInt16 & nTemp )
{
	JGUInt16 uValue;
	if (!GetValue(u8Channel, uValue))
	{
		return false;
	}
	return ChangeValue2Temp(uValue, nTemp);
//	return GetAdcdata(u8Channel, uValue, nTemp);
}


JGBOOL CADCDriver::GetValue( JGUInt8 u8Channel, JGUInt16 & uValue)
{
	BOOL bRet = FALSE;
	
	ASSERT(u8Channel == ec_Lid || u8Channel == ec_Rad);
	
	bRet = ::DeviceIoControl(m_hADCDriver, IOCTL_SET_ADC_CHANNEL, &u8Channel, 1, NULL, 0, NULL, NULL);
	
	if (!bRet)
	{
		// 出错
		return false;
	}
	
	JGUInt32 nReadValue = 0;
	JGUInt32 actlen = 0;
	bRet = ::ReadFile(m_hADCDriver, &nReadValue, 1, &actlen, NULL);
	
	if (!bRet)
	{
		//出错 ！
//		MessageBox(NULL, TEXT("Read Error"), NULL, MB_OK);
		CShowMsgDlg dlg;
		dlg.SetText(TEXT("Read Error"));
		dlg.DoModal();
		return false;
	}
	
	uValue = (JGUInt16)nReadValue;
	
	return true;
}

JGBOOL CADCDriver::ChangeValue2Temp( JGUInt16 uValue, JGInt16 & nTemp )
{	
	int i,j;
	
	j=uValue;
	
	if(j>3924)														// 开路检测
	{
		nTemp = MINADCTMP;
		return false;
	}
	else if(j<0)													// 短路检测
    {
		return false;
	}
	for(i = 0; i < ADCTABNUM - 1; i++)
	{
		if(j >= AdcTab[i+1] && j <= AdcTab[i])
		{
			nTemp = (i-5 + (double)(AdcTab[i] - j)/(AdcTab[i] - AdcTab[i+1]))*100;
			return true;
		}
	}
	nTemp = MAXADCTMP;
	return true;
}

JGBOOL CADCDriver::GetAdcdata(JGUInt8 u8Channel, JGUInt16 uValue, JGInt16 & nTemp)
{
	if(u8Channel > ec_Rad)		//20100716只能给热盖和散热器算平均温度
		return FALSE;
	static JGUInt16 char_dec[AD_CH_MAX]={SAM_NUM,SAM_NUM};
	static JGUInt32 accumul[AD_CH_MAX]={0L,0L};	
	
//	JGUInt16 uValue;
//	GetValue(u8Channel, uValue);
	accumul[u8Channel] += uValue;
	char_dec[u8Channel]--;

	if (char_dec[u8Channel] == 0)							 
	{                 		
		char_dec[u8Channel] = SAM_NUM;                  		
		AdData[u8Channel] = (accumul[u8Channel]>>5);   
		accumul[u8Channel] = 0L;  
		if(u8Channel == ec_Rad)
		{
			return ChangeValue2Temp(AdData[u8Channel], nTemp);
		}
		else if(u8Channel == ec_Lid)
		{
			return ChangeValue2Temp(AdData[u8Channel], nTemp); 
		}           			
	}
	return false;
}