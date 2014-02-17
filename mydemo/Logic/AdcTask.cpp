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
	m_LidPwmCnt = 0;	// ִ���ȸǴ���	
	m_LidPwmOut = 0;	// �ȸ�PWMֵ	
	m_LidIntegral = 0;  // �ȸǻ���	
	m_LidTemp = 0;		// �ȸ��¶�		
	m_RadTemp = 0;		// ɢ�����¶�	
	m_RadWarmUpCnt = 0;	// ɢ����Ԥ�ȴ���
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
	// �õ��ȸǵ��¶ȣ�
	CJGDriverMgr::Instance()->GetLidTemp(m_LidTemp);
	
	// �õ�ɢ�����¶�
	CJGDriverMgr::Instance()->GetRadTemp(m_RadTemp);
	
	// ���ù�����������
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
	
	delta = theApp.m_LidSetTemp* 100 - m_LidTemp;										// �¶Ȳ�
	
	JGInt16 nLidPwmBuff = 0;
	
	if(abs(delta)<KP)												// �¶Ȳ�С�ڱ�������
	{
		
		nLidPwmBuff=(int)((long)delta*MAXLIDPWM/KP);					// �����������
		
		m_LidIntegral+=(long)delta; 									// �����ۼ�
		
		if(m_LidIntegral>MAXLIDPWM*KI)								// ������ֵ�ж�
			m_LidIntegral=MAXLIDPWM*KI;
		else if(m_LidIntegral<-MAXLIDPWM*KI)
			m_LidIntegral=-MAXLIDPWM*KI;
		
		nLidPwmBuff+=(int)(m_LidIntegral/KI);	    					// ���ֵ���
		
		InterlockedExchange(&theApp.m_lLidWarmFlag, LIDWARMOK);	
	}
	else															// �¶Ȳ���ڱ�������
	{   
		if(delta>0)nLidPwmBuff=MAXLIDPWM;							// ����¶ȵͣ�����
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
	
	if(m_bRadExit)		//���ɢ��������70�ȣ������һֱת��ֱ��ɢ��������60��
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
			if(RunState == ers_Stop)	//	����ֹͣ	
			{
				FanPwmOut = 0;
			}
			else//��������
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
				else if(TeTempCtrlSeg == TEMPRISESEG)	//���¶�
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
			
			////����PWM���
			CJGDriverMgr::Instance()->SetFanPWMValue(FanPwmOut);	
			
}


// ɢ����Ԥ��
void CAdcTask::RadWarmCouse()
{
	// ֻ���ڳ�������ʱ�����ܽ���ɢ����Ԥ�ȣ�
	long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
	
	
	if (RunState != ers_Stop)
	{
		return;
	}
	
	// ϵͳ����
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
			RadWarmUpCtrl();//ɢ��������Ԥ��
		}
		//		else if(RadWarmUpState == RADWARMEND)//ɢ����״̬ΪԤ�Ƚ���
		//		{
		// 			if(m_RadTemp < RADWARMSTARTTEMP)
		// 			{
		// 				//���ɢ�����¶�С�ڿ�ʼԤ���¶ȣ�40�ȣ���ɢ����Ԥ��״̬�趨Ϊ����Ԥ��
		// 				InterlockedExchange(&theApp.m_lRadWarmUpState, RADWARMUP);
		// 			}
		//		}
		
	}	
}


void CAdcTask::RadWarmUpCtrl()
{
	JGInt16 delta = RADWARMENDTEMP - m_RadTemp;	//	ɢ����Ԥ�Ƚ����¶�45��-ɢ������ǰ�¶�
	//ɢ�����¶�Ԥ��45-48��
	if(delta > KP )								//	m_RadTemp<4500
	{
		//ʹ��ģ��
		CJGDriverMgr::Instance()->EnableAll(true); 
		//��·PWM������
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
		////ģ��ֹͣ����,ɢ��������Ҫ����
		CJGDriverMgr::Instance()->DisableAllPWM();
		
		//	ɢ����Ԥ�Ƚ���
		InterlockedExchange(&theApp.m_lRadWarmUpState, RADWARMEND);
	}	
	
	//�ȸ��¶�Ԥ��30-70��
	if(m_LidTemp < 3300 && theApp.m_bProUp == 0)
		CJGDriverMgr::Instance()->SetLidTemp(true);
	else if(m_LidTemp > 10200)
		CJGDriverMgr::Instance()->SetLidTemp(false);
	if(m_LidTemp >= 3000 && m_LidTemp <= 10500)
		InterlockedExchange(&theApp.m_lLidWarmFlag, LIDWARMOK);
}


//----------------------------------------------------------------20100713
//���Ⱥ��ȸ�������ƹ���
//----------------------------------------------------------------
void CAdcTask::FanLidPwmManage(void)
{
	m_LidPwmCnt++;
	
	long RunStep = InterlockedExchangeAdd(&theApp.m_lRunStep, 0);
	JGUInt8 LidState = theApp.m_LidState;
	long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
	
	if(m_LidPwmCnt>=MAXLIDPWM-1)
		m_LidPwmCnt=0;
	
	//	if((RunStep!=RUN_SOAK)&&(TempBlock==NORMALTEMP))	//���ڵ��¶Σ��ڳ��¶�
	if(RunStep == RUN_INIT)
		RadWarmCouse();
	else
		if(RunState != ers_Stop || (RunState == ers_Stop && theApp.m_bLidPreHot == TRUE))
		{
			if(RunStep == RUN_SOAK)
				CJGDriverMgr::Instance()->SetLidTemp(false);
			else
				if(LidState>LID_CLOSED)	//�ȸ�δ�ر�
				{
					TMPCTRLVAR tempctrl[etw_Size];
					theApp.GetTempCtrlValue(tempctrl);
					
					//TRACE(L"=========%d,RunState = %d,RunStep = %d\n",tempctrl[etw_Middle].SetTemp,RunState,RunStep );
					if(tempctrl[etw_Middle].SetTemp < 3000 && RunState != ers_Stop
						/*&& RunStep != RUN_INIT*/)		//ģ�������¶ȵ���30�㣬�ȸǹر�
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
