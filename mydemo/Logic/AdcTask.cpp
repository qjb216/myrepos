// AdcTask.cpp: implementation of the CAdcTask class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../TemperatureCtrl.h"
#include "AdcTask.h"
#include "JGDriverMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAdcTask::CAdcTask():m_pRunWnd(NULL)
{
	m_LidPwmCnt = 0;	// 执行热盖次数	
	m_LidPwmOut = 0;	// 热盖PWM值	
	m_LidIntegral = 0;  // 热盖积分	
	m_LidTemp = 0;		// 热盖温度		
	m_RadTemp = 0;		// 散热器温度	
	m_RadWarmUpCnt = 0;	// 散热器预热次数
	m_bRadExit = false;
	
	m_FanTimeLasting = GetTickCount();
	dwTick = 0;
	m_bDownTo60 = false;
	InitializeCriticalSection(&m_cs);

	
	
}

CAdcTask::~CAdcTask()
{
	DeleteCriticalSection(&m_cs);
}


JGBOOL CAdcTask::DoTask()
{ 
	GetAdcData();
	if(GetTickCount() - dwTick > 250)
	{
		FanLidPwmManage();
		if(GetTickCount()-m_FanTimeLasting >= 1000)
		{
			FanCtrl2();
			m_FanTimeLasting = GetTickCount();
		}	
		dwTick = GetTickCount();
	}
	return true;
}

void CAdcTask::GetAdcData()
{
	// 得到热盖的温度：
	CJGDriverMgr::Instance()->GetLidTemp(m_LidTemp);
	
	// 得到散热器温度
	CJGDriverMgr::Instance()->GetRadTemp(m_RadTemp);
	
	// 设置公共数据区域
	InterlockedExchange(&theApp.m_lCurLidTemp, m_LidTemp);
	InterlockedExchange(&theApp.m_lCurRadTemp, m_RadTemp);
	
	if(m_pRunWnd != NULL)
	{
		m_pRunWnd->OnGetAdcData(m_LidTemp,m_RadTemp);
	}
}


void CAdcTask::LidCtrl()
{
	int delta;
	
	delta = theApp.m_LidSetTemp* 100 - m_LidTemp;										// 温度差
	
	JGInt16 nLidPwmBuff = 0;
	
	if(abs(delta)<KP)												// 温度差小于比例区间
	{
		
		nLidPwmBuff=(int)((long)delta*MAXLIDPWM/KP);					// 比例调整输出
		
		m_LidIntegral+=(long)delta; 									// 积分累计
		
		if(m_LidIntegral>MAXLIDPWM*KI)								// 积分最值判断
			m_LidIntegral=MAXLIDPWM*KI;
		else if(m_LidIntegral<-MAXLIDPWM*KI)
			m_LidIntegral=-MAXLIDPWM*KI;
		
		nLidPwmBuff+=(int)(m_LidIntegral/KI);	    					// 积分调整
		
		InterlockedExchange(&theApp.m_lLidWarmFlag, LIDWARMOK);	
	}
	else															// 温度差大于比例区间
	{   
		if(delta>0)nLidPwmBuff=MAXLIDPWM;							// 如果温度低，加热
		else nLidPwmBuff=0;
		
	}
	if(nLidPwmBuff<=0)nLidPwmBuff=0;
	if(nLidPwmBuff>=MAXLIDPWM)nLidPwmBuff=MAXLIDPWM;
	
	m_LidPwmOut=nLidPwmBuff;
	
}
#define START_COOL_RAMTEMP   7500
#define END_COOL_RAMTEMP     7000
#define COOL_TIME            1000*60*2
bool CAdcTask::IsRadNeedCool()
{
	
	Lock();
	if(m_bDownTo60 != false)
	{
		if(m_RadTemp >= END_COOL_RAMTEMP)
		{
			if(GetTickCount() - dwCount > COOL_TIME)
			{
				theApp.m_bRadHighTWarning = 2;
				//TODO: warning	
			}
			UnLock();
			return true;
		}
		else
		{
			theApp.m_bRadHighTWarning = 0;
			m_bDownTo60 = false;
			UnLock();
			return false;
		}
		
	}
	else if(m_RadTemp >= START_COOL_RAMTEMP)
	{
		CJGDriverMgr::Instance()->SetFanPWMValue(MAXFANPWM);
		dwCount = GetTickCount();
		m_bDownTo60 = true;
		theApp.m_bRadHighTWarning = 1;
		//TODO: warning	
		UnLock();
		return true;
	}
	else if(m_RadTemp <= END_COOL_RAMTEMP)
	{
		m_bDownTo60 = false;
		theApp.m_bRadHighTWarning = 0;
	}
	UnLock();
	return false;
}


void CAdcTask::TurnOnFan(bool bOn)
{
	Lock();
	if(bOn != false)
	{
		CJGDriverMgr::Instance()->SetFanPWMValue(MAXFANPWM);
	}
	else
	{
		if(m_bDownTo60 == false)
			CJGDriverMgr::Instance()->SetFanPWMValue(0);
	}
	UnLock();
}

void CAdcTask::FanCtrl2()
{
	if(IsRadNeedCool() != false)
		return;

	
	long RunStep = InterlockedExchangeAdd(&theApp.m_lRunStep, 0);
	if(RunStep == RUN_INIT)
	{
		if(m_RadTemp >= 5000)
		{
			CJGDriverMgr::Instance()->SetFanPWMValue(MAXFANPWM);	
		}
		else if(m_RadTemp < 4500)
		{
			CJGDriverMgr::Instance()->SetFanPWMValue(0);
		}
	}
	else
	{
		long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
		if(RunState == ers_Stop)
		{
			CJGDriverMgr::Instance()->SetFanPWMValue(0);
			return;
		}
		long TeTempCtrlSeg = InterlockedExchangeAdd(&theApp.m_lTeTempCtrlSeg, 0);
		if(TeTempCtrlSeg == TEMPADJUSTSEG)
		{
			if(m_RadTemp >= 5500)
			{
				CJGDriverMgr::Instance()->SetFanPWMValue(MAXFANPWM);	
			}
			else if(m_RadTemp < 5000)
			{
				CJGDriverMgr::Instance()->SetFanPWMValue(0);
			}
		}
		else if(TeTempCtrlSeg == TEMPRISESEG)
		{
			if(m_RadTemp >= 6000)
			{
				CJGDriverMgr::Instance()->SetFanPWMValue(MAXFANPWM);	
			}
			else
			{
				CJGDriverMgr::Instance()->SetFanPWMValue(0);
			}
		}
		else if(TeTempCtrlSeg == TEMPDOWNSEG)
		{
			CJGDriverMgr::Instance()->SetFanPWMValue(MAXFANPWM);
		}
	}
	
}


void CAdcTask::FanCtrl()
{
	JGUInt16 FanPwmOut = 0;
	static JGUInt16 FanPmOutPre = 0;
	
	long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
	long RunStep = InterlockedExchangeAdd(&theApp.m_lRunStep, 0);
	
	if(m_bRadExit)		//如果散热器超过70度，则风扇一直转，直到散热器到达60度
	{
		if(m_RadTemp >= 6000)
			FanPwmOut = MAXFANPWM;
		else
			m_bRadExit = false;
	}
	else if(m_RadTemp >= 7000)
	{
		FanPwmOut = MAXFANPWM;
		m_bRadExit = true;
	}
	else
		if(RunStep == RUN_INIT)
		{
			if(m_RadTemp >= WARMENDSETTETEMP+300)
				FanPwmOut = MAXFANPWM;
		}
		else
			if(RunState == ers_Stop)	//	程序停止	
			{
				FanPwmOut = 0;
			}
			else//程序运行
			{
				long TeTempCtrlSeg = InterlockedExchangeAdd(&theApp.m_lTeTempCtrlSeg, 0);
				
				
				if(m_RadTemp >= 5500)
				{
					FanPwmOut = MAXFANPWM;
					FanPmOutPre = FanPwmOut;
				}
				/*		else if(m_RadTemp <= MAXSETRADTEMP)
				{
				FanPwmOut = 0;
				FanPmOutPre = FanPwmOut;
				}
				*/	
				else if(TeTempCtrlSeg == TEMPADJUSTSEG && m_RadTemp >= MAXSETRADTEMP)
				{
					FanPwmOut = FanPmOutPre;
				}
				else if(TeTempCtrlSeg == TEMPRISESEG)	//升温段
				{
					FanPwmOut = 0;		
					FanPmOutPre = FanPwmOut;
				}
				else if(TeTempCtrlSeg == TEMPDOWNSEG)
				{
					FanPwmOut = MAXFANPWM;
					FanPmOutPre = FanPwmOut;
				}
				else
				{
					FanPwmOut = 0;
				}
			}
			
			////风扇PWM输出
			CJGDriverMgr::Instance()->SetFanPWMValue(FanPwmOut);	
			
}


// 散热器预热
void CAdcTask::RadWarmCouse()
{
	// 只有在程序不运行时，才能进行散热器预热；
	long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
	
	
	if (RunState != ers_Stop)
	{
		return;
	}
	
	// 系统报警
	if (theApp.m_errorMgr.HasSystemError())
	{	
		CJGDriverMgr::Instance()->DisableAllPWM();
		InterlockedExchange(&theApp.m_lRadWarmUpState, RADWARMEND);
	}
	else
	{
		long RadWarmUpState = InterlockedExchangeAdd(&theApp.m_lRadWarmUpState, 0);
		long LidWarmUpState = InterlockedExchangeAdd(&theApp.m_lLidWarmFlag, 0);
		if(RadWarmUpState == RADWARMUP || LidWarmUpState == LIDWARMUP)
		{
			RadWarmUpCtrl();//散热器正在预热
		}
		//		else if(RadWarmUpState == RADWARMEND)//散热器状态为预热结束
		//		{
		// 			if(m_RadTemp < RADWARMSTARTTEMP)
		// 			{
		// 				//如果散热器温度小于开始预热温度（40度），散热器预热状态设定为正在预热
		// 				InterlockedExchange(&theApp.m_lRadWarmUpState, RADWARMUP);
		// 			}
		//		}
		
	}	
}


void CAdcTask::RadWarmUpCtrl()
{
	JGInt16 delta = RADWARMENDTEMP - m_RadTemp;	//	散热器预热结束温度45度-散热器当前温度
	//散热器温度预热45-48°
	if(delta > KP )								//	m_RadTemp<4500
	{
		//使能模块
		CJGDriverMgr::Instance()->EnableAll(true); 
		//三路PWM输出最大
		CJGDriverMgr::Instance()->SetLeftPWMValue(MAXTEPWM -1); 
		CJGDriverMgr::Instance()->SetMiddlePWMValue(MAXTEPWM -1);
		CJGDriverMgr::Instance()->SetRightPWMValue(MAXTEPWM -1);
		m_RadWarmUpCnt++;
		
		long lCurModelTemp = InterlockedExchangeAdd(&theApp.m_lCurModelTemp, 0);
		if(m_RadWarmUpCnt < 15)
			CJGDriverMgr::Instance()->SetTempCtrl(false);
		else if(m_RadWarmUpCnt < 20)
		{
//			if(lCurModelTemp < 2500)
				CJGDriverMgr::Instance()->SetTempCtrl(true);
//			else
//				CJGDriverMgr::Instance()->SetTempCtrl(false);
		}
		
		else
			m_RadWarmUpCnt = 0;
	}
	else if(delta < 0)
	{
		
	}
	else
	{
		////模块停止工作,散热器不需要控温
		CJGDriverMgr::Instance()->DisableAllPWM();
		
		//	散热器预热结束
		InterlockedExchange(&theApp.m_lRadWarmUpState, RADWARMEND);
	}	
	
	//热盖温度预热30-70°
	if(m_LidTemp < 3300 && theApp.m_bProUp == 0)
		CJGDriverMgr::Instance()->SetLidTemp(true);
	else if(m_LidTemp > 10200)
		CJGDriverMgr::Instance()->SetLidTemp(false);
	if(m_LidTemp >= 3000 && m_LidTemp <= 10500)
		InterlockedExchange(&theApp.m_lLidWarmFlag, LIDWARMOK);
}


//----------------------------------------------------------------20100713
//风扇和热盖脉宽调制管理
//----------------------------------------------------------------
void CAdcTask::FanLidPwmManage(void)
{
	m_LidPwmCnt++;
	
	long RunStep = InterlockedExchangeAdd(&theApp.m_lRunStep, 0);
	JGUInt8 LidState = theApp.m_LidState;
	long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
	
	if(m_LidPwmCnt>=MAXLIDPWM-1)
		m_LidPwmCnt=0;
	
	//	if((RunStep!=RUN_SOAK)&&(TempBlock==NORMALTEMP))	//不在低温段，在常温段
	if(RunStep == RUN_INIT)
		RadWarmCouse();
	else
		if(RunState != ers_Stop || (RunState == ers_Stop && theApp.m_bLidPreHot == TRUE))
		{
			if(RunStep == RUN_SOAK)
				CJGDriverMgr::Instance()->SetLidTemp(false);
			else
				if(LidState>LID_CLOSED)	//热盖未关闭
				{
					TMPCTRLVAR tempctrl[etw_Size];
					theApp.GetTempCtrlValue(tempctrl);
					
					//TRACE(L"=========%d,RunState = %d,RunStep = %d\n",tempctrl[etw_Middle].SetTemp,RunState,RunStep );
					if(tempctrl[etw_Middle].SetTemp < 3000 && RunState != ers_Stop
						/*&& RunStep != RUN_INIT*/)		//模块设置温度低于30°，热盖关闭
					{
						CJGDriverMgr::Instance()->SetLidTemp(false);
					}
					else
					{
						
						LidCtrlEx();
					}
				}
				else
				{
					CJGDriverMgr::Instance()->SetLidTemp(false);
				}			
		}
		else if(RunState == ers_Stop && LidState == LID_BOOT)
		{
			long runinfo = InterlockedExchangeAdd(&theApp.m_HasRunOnce, 0);
			if(runinfo == 0)
			{
				LidCtrlEx();
			}
			else
				CJGDriverMgr::Instance()->SetLidTemp(false);
		}
		else if(LidState == LID_NOWAIT)
		{
			if(RunState == ers_Run)
			{
				LidCtrlEx();
			}
			else
				CJGDriverMgr::Instance()->SetLidTemp(false);
		}
		else
		{		
			CJGDriverMgr::Instance()->SetLidTemp(false);
		}
		
		
}


void CAdcTask::LidCtrlEx()
{
	LidCtrl();
				
	if(m_LidPwmCnt < m_LidPwmOut && theApp.m_bProUp == 0)
	     CJGDriverMgr::Instance()->SetLidTemp(true);
	else
		 CJGDriverMgr::Instance()->SetLidTemp(false);
}
