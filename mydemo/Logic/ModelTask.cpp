// ModelTask.cpp: implementation of the CModelTask class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../TemperatureCtrl.h"
#include "ModelTask.h"
#include "JGDriverMgr.h"
#include "../showmsgdlg.h"
#include "../confirmmsg.h"
#include <vector>
#include "../NetCommandMgr.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModelTask::CModelTask(CDialog_JGTrend_Run *pScreen)
{
	TRACE(L"CModelTask::CModelTask------------------\n");
	m_pScreen = pScreen;

//	m_nDeep = 0;
	// 所有数据初始化！！！	
	m_RunGradEn    = DISABLE;		// 梯度运行使能
	m_GradSpeedEn  = DISABLE;		// 梯度速度使能
	m_GradientFlag = DISABLE;		// 梯度标记

	m_TempOkFlag   = 0;				// 温度是否稳态标记	
	m_TempArriveFlag = 0;			// 温度是否到达设定值标记
	m_bHasAlarm = FALSE;
	
	memset(m_TempCtrl, 0 ,sizeof(m_TempCtrl));			// 控制模块
	memset(m_AdjTemp, 0, sizeof(m_AdjTemp));			// 梯度温度修正值
	memset(m_ExcGradient, 0, sizeof(m_ExcGradient));	// 梯度修正
	m_ExcTeTemp = 0; 									// 模块平板温度修正值
	m_bAdjust = true;

	
	m_lItemObjectId = 0;			// 运行的objectid
	m_lItemTime = 0;				// 需要运行的时间
	m_lItemTimeStep = 0;			// 时间递增或递减
	m_fItemTemp = 0;				// 节点温度
	m_fItemTempStep = 0;			// 温度递增或递减
	m_fItemTempGrands = 0;			// 梯度值；
	// 进入低温保存状态
	m_bStoreState = 0;				// 低温保存状态
	m_bBeginSetTime = 0 ;
	m_loopCount		= 0;
	m_timeStop		= 0;
	// model任务开始运行：
	InterlockedExchange(&theApp.m_lRunState, ers_Run);

	// 进入运行阶段
	InterlockedExchange(&theApp.m_lRunStep, RUN_CYCLE);

	InitializeCriticalSection(&m_cs);


	/////////////////////////////////
	m_ModifyDisptemp=0;
    m_CanAdjust=0;
	m_lItemTemp=0;

}



CModelTask::~CModelTask()
{
	DeleteCriticalSection(&m_cs);
	InterlockedExchange(&theApp.m_lRunStep, RUN_END); // 运行结束标记！！！

	ProAbort = FALSE;
	CJGDriverMgr::Instance()->DisableAllPWM();
	
	

//	if(theApp.m_CCJSTSS)
//		CJGDriverMgr::Instance()->Alarm(50);


}
//0 nCount, 0 means enter into store

#define MAX_INFORMATION  8
int nCount = 0;
//int nStatus[MAX_INFORMATION];
//int nPos = 0;
//0 nItem, 1 nRunningLoop, 2 nItem, 3 nRunningLoop,4 nItem, 5... 
int nDuration = 0;

JGBOOL CModelTask::DoTask()
{
//	memset(nStatus,0,MAX_INFORMATION * sizeof(int));//replace by file

	LogFile::writeLog(L"Start of CModelTask::Dotask");
	ASSERT(m_pScreen);
	if(m_pScreen->m_RunningPointEle.lPhase == PHASE_STORE) //if it is restore from storing, return directly.
		return true;

	const CUserDataManager::CTrendDataArr * pTrendDataArr = CUserDataManager::Instance()->GetUserTrendDataArr();
	ASSERT(pTrendDataArr);
	for(int i = 0;i<MAX_LOOP + 1;i++)
		m_indexArr[i] = 0;

	for(i = 0;i<MAX_LOOP; i++)
	{
		m_loopPass[i].loopID = 0;
		m_loopPass[i].loopPassed = 0;
	}

	m_nCurrentItemID = 0;

	Do(pTrendDataArr,m_pScreen->m_RunningPointEle.lItemIndex[0],1,0,0);
		
	return true;
}
#define RESTORE_BUFF_DURATION  5
#define MIN_LOOPNUM 2



JGUInt8 CModelTask::Do(const CUserDataManager::CTrendDataArr * pData,short nItemIndex, JGUInt16 uLoopNums,JGUInt16 uLoopID,int nDeep)
{
	long curID = InterlockedExchangeAdd(&theApp.app_m_lCurRuningItemID, 0);
//	if(curID == 9999 && theApp.m_bIsRS)
//		return 1;
	const CUserDataManager::CTrendDataArr *  pTrendDataArr = pData;
	CString str;
	str.Format(L"Do nItemIndex[%d],uLoopNums[%d],uLoopID[%d],nDeep[%d]",nItemIndex,uLoopNums,uLoopID,nDeep);
	LogFile::writeLog(str);
	int nPassedLoop = 0;
	while(true)
	{
		if (uLoopNums > MIN_LOOPNUM - 1)
		{	
			nPassedLoop = theApp.m_RuningLoopInfo.AddSimLoopRunPass(uLoopID);
			m_loopPass[nDeep - 1].loopID = uLoopID;
			m_loopPass[nDeep - 1].loopPassed = nPassedLoop;

			if(nPassedLoop < 0)
			{
				m_loopPass[nDeep - 1].loopID = 0;
				m_loopPass[nDeep - 1].loopPassed = 0;
				m_indexArr[nDeep] = 0;
				nItemIndex = 0;
				m_pScreen->m_RunningPointEle.lItemIndex[nDeep] = 0;
				LogFile::writeLog("nPassedLoop is smaller than 0,out of loop bundle");
				return 1;
			}
		}

		if(nItemIndex > 0)//if it is recovered from cutting off, empty loop count before this index
		{
			for(JGUInt16 i = 0; i < nItemIndex; i++)
			{
				CUserDataManager::CTrendDataBase * pTemTrendDataInfo = pTrendDataArr->GetAt(i);
				if(TrendData_Loop == pTemTrendDataInfo->GetDataMode())
				{
					CUserDataManager::CTrendData_Loop * pTrendDataLoop = (CUserDataManager::CTrendData_Loop*)pTemTrendDataInfo;
					theApp.m_RuningLoopInfo.SetLoopRunPassFull(pTemTrendDataInfo->GetItemID());
				}
			}
		}
		for (JGUInt16 i = nItemIndex; i < pTrendDataArr->GetSize(); i++)
		{

			m_indexArr[nDeep] = i;
			BOOL bHasTask = FALSE;

			CUserDataManager::CTrendDataBase * pTemTrendDataInfo = pTrendDataArr->GetAt(i);
			TrendDataMode nType = pTemTrendDataInfo->GetDataMode();
			if(TrendData_Item == nType)
			{
				CUserDataManager::CTrendData_Item * pTrendDataItem = (CUserDataManager::CTrendData_Item*)pTemTrendDataInfo;

				m_lItemObjectId = (long)pTrendDataItem->GetItemID(); // ID
				m_nCurrentItemID = m_lItemObjectId;
				m_lItemTime     = (long)pTrendDataItem->GetSeconds(); // 执行时间

				/////断电保护 begin to calculate running time///////////////////////////
				/*if(m_pScreen->m_RunningPointEle.lDuration != 0)
				{
					int nItemTime = m_lItemTime - m_pScreen->m_RunningPointEle.lDuration + RESTORE_BUFF_DURATION;
					m_pScreen->m_RunningPointEle.lDuration = 0;
					if((nItemTime < m_lItemTime) && (nItemTime > 0))
					{
						m_lItemTime = nItemTime;
					}
					if(nItemTime - RESTORE_BUFF_DURATION < 0)
					{
						LogFile::writeLog("Running time is too short,error");
					}
				}*/
				/////断电保护 End//////////////////////////////////////////

				m_lItemTimeStep	= (long)pTrendDataItem->GetSeconds_Step(); // 增长或加少时间
				m_fItemTemp     = pTrendDataItem->GetTempe(); // 温度
				m_fItemTempStep = pTrendDataItem->GetTempe_Step();  // 温度低增或递减；
				m_fItemTempGrands = (long)pTrendDataItem->GetTempe_Grandient(); // 梯度
				TRACE(L"m_fItemTempGrands = %d,m_fItemTemp=%f,m_fItemTempStep=%f\n",m_fItemTempGrands,m_fItemTemp,m_fItemTempStep);

				if(m_fItemTempGrands < 1.0 && m_fItemTempGrands > 0.0)
					m_fItemTempGrands = 1.0;

				m_bStoreState = 0; // 非低温保存；
				bHasTask = TRUE;
				
			}
			else if(TrendData_Loop == nType)
			{
				
				CUserDataManager::CTrendData_Loop * pTrendDataLoop = (CUserDataManager::CTrendData_Loop*)pTemTrendDataInfo;
				InterlockedExchange(&theApp.app_m_loopId, pTrendDataLoop->GetItemID());	//保存当前循环的id
				InterlockedExchange(&theApp.app_m_iLoops, pTrendDataLoop->GetLoops());	//保存当前循环总数

				if(++nDeep > 2)
				{
					LogFile::writeLog("calling is out of limitation for CModelTask::Do");
					return 255;
				}
				
				m_nCurrentItemID = m_loopPass[nDeep - 1].loopID;

				//nStatus[nPos];
				int nRes = Do(
					(const CUserDataManager::CTrendDataArr * ) pTrendDataLoop->GetTrendDataArr(),
					m_pScreen->m_RunningPointEle.lItemIndex[nDeep], 
					pTrendDataLoop->GetLoops(),
					pTrendDataLoop->GetItemID(),
					nDeep
					);

				if(--nDeep < 0)
				{
					LogFile::writeLog("nDeep0 is out of range");
					return 255;
				}

				//theApp.m_RuningLoopInfo.SetLoopRunPass(pTemTrendDataInfo->GetItemID(),0);
				if(255 == nRes)
				{
					LogFile::writeLog(L"Do return 255");
					return 255;
				}
				
				bHasTask = FALSE;
			}
			else if(TrendData_Store == nType)
			{
				CUserDataManager::CTrendData_Store * pTrendDataStore = (CUserDataManager::CTrendData_Store*)pTemTrendDataInfo;
				// 获取数据；
				m_lItemObjectId = (long)pTrendDataStore->GetItemID();
				m_nCurrentItemID = m_lItemObjectId;
				m_fItemTemp = pTrendDataStore->GetTempe();
				m_lItemTime = (long)pTrendDataStore->GetTime();

				m_lItemTimeStep = 0;
				m_fItemTempStep = 0;
				m_fItemTempGrands = 0;
				m_bStoreState = 1;
				bHasTask = TRUE;
			}
		
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////

			if (bHasTask)
			{
//				InterlockedExchange(&theApp.app_m_numLoops, j);	//保存当前已运行循环数
				// 设置数据
				ResetModelData(nPassedLoop);

				//////////////////////////////////////////////
				if(m_pScreen != NULL)
					m_pScreen->OnDoTaskNotify(m_nCurrentItemID,m_loopPass,m_indexArr,0,PHASE_RUNNING);
				
				//////////////////////////////////////////////

				// 循环执行任务...
				if (DoLoop() == 255)
				{
					// 跳出循环
					LogFile::writeLog(L"DoLoop return 255");
					return 255;
				}
			}
		}
		m_indexArr[nDeep] = 0;
		nItemIndex = 0;
		m_pScreen->m_RunningPointEle.lItemIndex[nDeep] = 0;

		//when run out of all index, empty loop count
		for (i = 0; i < pTrendDataArr->GetSize(); i++)
		{
			CUserDataManager::CTrendDataBase * pTemTrendDataInfo = pTrendDataArr->GetAt(i);
			if(TrendData_Loop == pTemTrendDataInfo->GetDataMode())
			{
				CUserDataManager::CTrendData_Loop * pTrendDataLoop = (CUserDataManager::CTrendData_Loop*)pTemTrendDataInfo;
				theApp.m_RuningLoopInfo.SetLoopRunPass(pTemTrendDataInfo->GetItemID(),0);
			}
		}

		if(uLoopNums < MIN_LOOPNUM)
		{
			LogFile::writeLog(L"out of uLoopNums");
			break;
		}
	}
	LogFile::writeLog(L"do return 1");
	return 1;
	
}

void CModelTask::ResetModelData(JGUInt16 uRunsNum)
{
	m_count = 0;
	// 温度递增了
	m_fItemTemp  += uRunsNum * m_fItemTempStep;
	// 时间递增了
	m_lItemTime  += uRunsNum* m_lItemTimeStep;
    ////////////////////////////////////////////////////

	m_lItemTemp=m_fItemTemp*100;

	//////////////////////////////////////////////
	if(m_pScreen != NULL)
		m_pScreen->OnResetModelData(uRunsNum,m_fItemTemp,m_lItemTime);
	//////////////////////////////////////////////


	TRACE(L"m_fItemTemp = %f,m_lItemTime=%f\n",m_fItemTemp,m_lItemTime);

	m_bAdjust = true;
	m_bHasAlarm = FALSE;

	
	for(int i=0; i < etw_Size; i++)
	{
		m_TempCtrl[i].PwmOut=0;
		m_TempCtrl[i].Integral=0;
		m_TempCtrl[i].SpeedInl=0;
		m_TempCtrl[i].LastError = 0;
		m_TempCtrl[i].PrevError = 0;
	}


	if (m_fItemTempGrands != 0)
	{
		// 有梯度
		m_RunGradEn    = ENABLE;				// 梯度运行使能
		m_GradSpeedEn  = ENABLE;				// 梯度速度使能
		m_GradientFlag = ENABLE;				// 梯度标记
	}
	else
	{
		// 无梯度
		m_RunGradEn    = DISABLE;				// 梯度运行使能
		m_GradSpeedEn  = DISABLE;				// 梯度速度使能
		m_GradientFlag = DISABLE;				// 梯度标记
	}

	// 跟当前温度比较，判断到底是升温，还是降温。
	CJGDriverMgr::Instance()->GetModelTemp(0,m_TempCtrl[0].SetTempCopy);
	CJGDriverMgr::Instance()->GetModelTemp(1,m_TempCtrl[1].SetTempCopy);
	CJGDriverMgr::Instance()->GetModelTemp(2,m_TempCtrl[2].SetTempCopy);

	// 当前温度和设置温度的温差
	long lTempDif = long(m_fItemTemp * 100) - m_TempCtrl[1].SetTempCopy;

	int exc;
	unsigned char index;	
	unsigned char item[3];
	
	JGUInt16 nTemp = m_fItemTempGrands * 100;


	if(m_fItemTempGrands == 0)
	{
//		LogFile::writeLog(_T("m_fItemTempGrands == 0"));
		m_GradientFlag = DISABLE;
//		m_TempCtrl[0].SetTempCopy = m_TempCtrl[0].SetTemp; // 设置温度备份
//		m_TempCtrl[1].SetTempCopy = m_TempCtrl[1].SetTemp; // 设置温度备份
//		m_TempCtrl[2].SetTempCopy = m_TempCtrl[2].SetTemp; // 设置温度备份	
		
		m_TempCtrl[0].SetTemp     = JGUInt16(m_fItemTemp * 100); // 设置温度		20100826
		m_TempCtrl[1].SetTemp     = JGUInt16(m_fItemTemp * 100); // 设置温度
		m_TempCtrl[2].SetTemp     = JGUInt16(m_fItemTemp * 100); // 设置温度

		InterlockedExchange(&theApp.m_lSetModelTemp, m_TempCtrl[1].SetTemp);

		index = abs(m_TempCtrl[1].SetTemp) / 500;
		if(theApp.m_ModelType==5)
		{
			exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_2Tab[index+1]-Exc0_2Tab[index]))/500;			
			m_ExcTeTemp=Exc0_2Tab[index]+exc;	
		}				
		else if(theApp.m_ModelType==6)
		{
			exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_5Tab[index+1]-Exc0_5Tab[index]))/500;			
			m_ExcTeTemp=Exc0_5Tab[index]+exc;			
		}
		else if(theApp.m_ModelType==3)
		{
			exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc384Tab[index+1]-Exc384Tab[index]))/500;			
			m_ExcTeTemp=Exc384Tab[index]+exc;			
		}			
		else
		{
			exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_2and0_5Tab[index+1]-Exc0_2and0_5Tab[index]))/500;			
			m_ExcTeTemp=Exc0_2and0_5Tab[index]+exc;	
		}
		if(theApp.m_EnvTemp < TEMP_SPRING)	//模式1
		{
//			m_ExcTeTemp -= 20;
			m_ExcTeTemp-=15;
		}	
		else if(theApp.m_EnvTemp > TEMP_SUMMER)
		{
//			m_ExcTeTemp += 20;
			m_ExcTeTemp+=15;
		}
				
		m_TempCtrl[0].SetTemp = m_TempCtrl[1].SetTemp = m_TempCtrl[2].SetTemp = m_TempCtrl[1].SetTemp - m_ExcTeTemp;	

		InterlockedExchange((long*)&theApp.m_SetTempReal, m_ExcTeTemp);

		m_SetTDifMax = 0;
		for(int i = 0; i < etw_Size; i++)
		{
			m_SetTDif[i] = abs(m_TempCtrl[i].SetTempCopy - m_TempCtrl[i].SetTemp);
			if(m_SetTDifMax < m_SetTDif[i])
				m_SetTDifMax = m_SetTDif[i];
//			m_TempCtrl[i].SetTempCopy = m_TempCtrl[i].SetTemp;
		}

	
		for(i = 0; i < etw_Size; i++)
		{
			if(m_SetTDifMax == 0)
				m_TempCtrl[i].SetSpeed = 0;
			else
				m_TempCtrl[i].SetSpeed = (JGInt16)m_SetTDif[i]*MAXGRADSPEED/m_SetTDifMax;
		}

		TRACE(L"no Grad: Settemp = %d,SetTempCopy = %d,\n",m_TempCtrl[etw_Middle].SetTemp,m_TempCtrl[etw_Middle].SetTempCopy );
	}
	else
	{
		m_GradientFlag = ENABLE;
		m_GradSpeedEn = ENABLE;
		InterlockedExchange(&theApp.m_lSetModelTempPre, theApp.m_lSetModelTemp);
		// 带梯度的设置温度修正(粗调)
//		m_TempCtrl[etw_Left].SetTempCopy = m_TempCtrl[etw_Left].SetTemp; // 设置温度备份
//		m_TempCtrl[etw_Middle].SetTempCopy = m_TempCtrl[etw_Middle].SetTemp; // 设置温度备份
//		m_TempCtrl[etw_Right].SetTempCopy = m_TempCtrl[etw_Right].SetTemp; // 设置温度备份	

		m_SetTDifMax = 0;
		for(int i = 0; i < etw_Size; i++)
		{
			item[i]=abs(m_TempCtrl[i].SetTemp)/500;

			if(i == etw_Left)
			{
				m_TempCtrl[i].SetTemp = JGUInt16(m_fItemTemp * 100) - m_fItemTempGrands * 100 + 246.5 - 15.9 * (16-m_fItemTempGrands);
				m_TempCtrl[i].SetTemp -= (JGUInt16(m_fItemTemp * 100) - m_fItemTempGrands * 100 - 3000) * 0.0115- 15;
			}
			else if(i == etw_Middle)
			{
				m_TempCtrl[i].SetTemp = JGUInt16(m_fItemTemp * 100) - (273.4 - 15.1 * (16-m_fItemTempGrands));
			}
			else
			{
				m_TempCtrl[i].SetTemp = (JGUInt16(m_fItemTemp * 100) + m_fItemTempGrands * 100) - (194.6 - 11.2 * (16-m_fItemTempGrands));
				m_TempCtrl[i].SetTemp -= (JGUInt16(m_fItemTemp * 100) + m_fItemTempGrands * 100 - 3000) * 0.0075 - 20;
			}
			
			if(theApp.m_EnvTemp < TEMP_SPRING)
			{
//				m_TempCtrl[i].SetTemp += 20;
				m_TempCtrl[i].SetTemp+=15;
			}
			else if(theApp.m_EnvTemp > TEMP_SUMMER)
			{
//				m_TempCtrl[i].SetTemp -= 20;
				m_TempCtrl[i].SetTemp-=15;
			}
			
			if(theApp.m_ModelType==5)
			{
				m_TempCtrl[i].SetTemp += AdjGrad0_2[m_TempCtrl[i].SetTemp / 500][i];
			}
			else if(theApp.m_ModelType==6)
			{
				m_TempCtrl[i].SetTemp += Adj0_5Tab[m_TempCtrl[i].SetTemp / 500][i];
			}		
			else 
			{
				m_TempCtrl[i].SetTemp += AdjGrad0_2and0_5[m_TempCtrl[i].SetTemp / 500][i];
			}
			TRACE(L"Grad1: type=%d,i=%d,m_TempCtrl[i].SetTemp=%d,m_fItemTemp=%f,m_fItemTempGrands=%f\n",theApp.m_ModelType,i,m_TempCtrl[etw_Middle].SetTemp,m_fItemTemp,m_fItemTempGrands);

			if(i == etw_Middle)
			{
				InterlockedExchange((long*)&theApp.m_SetTempReal, JGUInt16(m_fItemTemp * 100) - m_TempCtrl[i].SetTemp);
			}
			m_SetTDif[i] = abs(m_TempCtrl[i].SetTempCopy - m_TempCtrl[i].SetTemp);
			if(m_SetTDifMax < m_SetTDif[i])
				m_SetTDifMax = m_SetTDif[i];
		}
		for(i = 0; i <etw_Size; i++)
		{
			if(m_SetTDifMax == 0)
				m_TempCtrl[i].SetSpeed = 0;
			else
				m_TempCtrl[i].SetSpeed = (long)m_SetTDif[i]*MAXGRADSPEED/m_SetTDifMax;
		}

		InterlockedExchange(&theApp.m_lSetModelTemp, JGUInt16(m_fItemTemp * 100));

	}
	
	m_TempCtrl[0].RampSpeed   = 0;				 // 爬坡速度
	m_TempCtrl[1].RampSpeed   = 0;				 // 爬坡速度
	m_TempCtrl[2].RampSpeed   = 0;				 // 爬坡速度

	if (lTempDif == 0)
	{	
		// 没温差，就认为不经过温度变化！
		m_TempOkFlag     = TETEMPOK;		// 温度是否稳态标记	
		m_TempArriveFlag = TEMPARRIVED;		// 温度是否到达设定值标记
		TempSegChanged(1,TEMPADJUSTSEG);
	}
	else if (lTempDif > 0)
	{
		m_TempOkFlag     = 0;
		m_TempArriveFlag = 0;
		TempSegChanged(1,TEMPRISESEG);
	}
	else
	{
		m_TempOkFlag     = 0;
		m_TempArriveFlag = 0; 
		TempSegChanged(1,TEMPDOWNSEG);
	}
	InterlockedExchange(&theApp.m_lCurRuningItemID, m_lItemObjectId);

	if (m_bStoreState)
	{
		InterlockedExchange(&theApp.m_lRunStep, RUN_SOAK);
	}
	else
	{
		InterlockedExchange(&theApp.m_lRunStep, RUN_CYCLE);
	}

	m_bBeginSetTime = 0;

//	theApp.m_bIsRS = 0;
}

void CModelTask::ResetModelStore()
{
	m_pScreen->OnDoTaskNotify(0,m_loopPass,m_indexArr,0,PHASE_STORE); //m_indexArr is not used;
	m_RunGradEn    = DISABLE;				// 梯度运行使能
	m_GradSpeedEn  = DISABLE;				// 梯度速度使能
	m_GradientFlag = DISABLE;				// 梯度标记

	m_count = 0;

	m_bHasAlarm = FALSE;
	
	//////////////////////////////////////////////
	if(m_pScreen != NULL)
		m_pScreen->OnResetModelStore(theApp.m_DWBCSZ);
	//////////////////////////////////////////////
	
	// 跟当前温度比较，判断到底是升温，还是降温。
	long lCurTemp = InterlockedExchangeAdd(&theApp.m_lCurModelTemp, 0);
	//	long lCurTemp = m_TempCtrl[etw_Middle].CurrTemp;
	
	m_lCurTemp = lCurTemp;
	
	// 当前温度和设置温度的温差



	for(int i=0; i < etw_Size; i++)
	{
		m_TempCtrl[i].PwmOut=0;
		m_TempCtrl[i].Integral=0;
		m_TempCtrl[i].LastError = 0;
		m_TempCtrl[i].PrevError = 0;
		m_TempCtrl[i].SpeedInl=0;
	}
	
	theApp.GetTempCtrlValue(m_TempCtrl);
	
	m_TempCtrl[0].SetTempCopy = m_TempCtrl[0].SetTemp; // 设置温度备份
	m_TempCtrl[1].SetTempCopy = m_TempCtrl[1].SetTemp; // 设置温度备份
	m_TempCtrl[2].SetTempCopy = m_TempCtrl[2].SetTemp; // 设置温度备份	

	m_TempCtrl[etw_Left].SetTemp = theApp.m_DWBCWD;
	m_TempCtrl[etw_Middle].SetTemp = theApp.m_DWBCWD;
	m_TempCtrl[etw_Right].SetTemp = theApp.m_DWBCWD;

	long lTempDif = theApp.m_DWBCWD - lCurTemp;
	
	InterlockedExchange(&theApp.m_lSetModelTemp, m_TempCtrl[etw_Middle].SetTemp);
	
	int exc;
	unsigned char index;	
	
	
	index = abs(m_TempCtrl[1].SetTemp) / 500;
	if(theApp.m_ModelType==5)
	{
//		exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_2Tab[index+1]-Exc0_2Tab[index]))/500;			
//		m_ExcTeTemp=Exc0_2Tab[index]+exc;	
		exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_2Tab[index+1]-Exc0_2Tab[index]))/500;			
		m_ExcTeTemp=Exc0_2Tab[index]+exc;	
		if(theApp.m_EnvTemp < TEMP_SPRING)
			m_ExcTeTemp -= 20;
		else if(theApp.m_EnvTemp > TEMP_SUMMER)		
			m_ExcTeTemp += 20;
	}				
	else if(theApp.m_ModelType==6)
	{
		exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_5Tab[index+1]-Exc0_5Tab[index]))/500;			
		m_ExcTeTemp=Exc0_5Tab[index]+exc;			
	}
	else if(theApp.m_ModelType==3)
	{
		exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc384Tab[index+1]-Exc384Tab[index]))/500;			
		m_ExcTeTemp=Exc384Tab[index]+exc;			
	}			
	else
	{			
		exc=((m_TempCtrl[1].SetTemp-500*index)*(long)(Exc0_2and0_5Tab[index+1]-Exc0_2and0_5Tab[index]))/500;			
		m_ExcTeTemp=Exc0_2and0_5Tab[index]+exc;	
		if(theApp.m_EnvTemp < TEMP_SPRING)
			m_ExcTeTemp -= 20;
		else if(theApp.m_EnvTemp > TEMP_SUMMER)		
			m_ExcTeTemp += 20;
	}
	m_TempCtrl[0].SetTemp = m_TempCtrl[1].SetTemp = m_TempCtrl[2].SetTemp = m_TempCtrl[1].SetTemp - m_ExcTeTemp;

	
	InterlockedExchange((long*)&theApp.m_SetTempReal, m_ExcTeTemp);
	
	m_SetTDifMax = 0;
	for(i = 0; i < etw_Size; i++)
	{
		m_SetTDif[i] = abs(m_TempCtrl[i].SetTempCopy - m_TempCtrl[i].SetTemp);
		if(m_SetTDifMax < m_SetTDif[i])
			m_SetTDifMax = m_SetTDif[i];
		//			m_TempCtrl[i].SetTempCopy = m_TempCtrl[i].SetTemp;
	}
	for(i = 0; i <etw_Size; i++)
	{
		if(m_SetTDifMax == 0)
			m_TempCtrl[i].SetSpeed = 0;
		else
			m_TempCtrl[i].SetSpeed = (JGInt16)m_SetTDif[i]*MAXGRADSPEED/m_SetTDifMax;
	}

	theApp.SetTempCtrlValue(m_TempCtrl);
	m_TempCtrl[0].RampSpeed   = 0;				 // 爬坡速度
	m_TempCtrl[1].RampSpeed   = 0;				 // 爬坡速度
	m_TempCtrl[2].RampSpeed   = 0;				 // 爬坡速度
	

	if (lTempDif == 0)
	{	
		// 没温差，就认为不经过温度变化！
		m_TempOkFlag     = TETEMPOK;		// 温度是否稳态标记	
		m_TempArriveFlag = TEMPARRIVED;		// 温度是否到达设定值标记
		TempSegChanged(2,TEMPADJUSTSEG);

	}
	else if (lTempDif > 0)
	{
		m_TempOkFlag     = 0;
		m_TempArriveFlag = 0;
		TempSegChanged(2,TEMPRISESEG);
		
	}
	else
	{
		m_TempOkFlag     = 0;
		m_TempArriveFlag = 0; 
		TempSegChanged(2,TEMPDOWNSEG);
		
	}
	


	m_bBeginSetTime = 0;

	InterlockedExchange(&theApp.m_lCurRuningItemID, 9999);
//	theApp.m_bIsRS = 0;
}


JGBOOL CModelTask::DoStore()
{
	CJGDriverMgr::Instance()->Alarm(50);

	JGBOOL bOk = FALSE;
    m_TeList=1;
	m_lItemTime = 1;
	InterlockedExchange(&theApp.m_lRunStep, RUN_SOAK);
	InterlockedExchange(&theApp.m_lRunState, ers_Storing);

	while(true)
	{
		Sleep(10);
		ModelTempManage(m_TeList);
		theApp.getNetMgr()->SendGuardHeartBeat();
		theApp.SetTempCtrlValue(m_TempCtrl);

		long lRunMsg = InterlockedExchangeAdd(&theApp.m_msgRun, 0);
		if (lRunMsg == MSG_STOP )
		{
			CJGDriverMgr::Instance()->DisableAllPWM();	
			break;
		}

		if (abs(m_TempCtrl[etw_Middle].DispTemp - m_TempCtrl[etw_Middle].SetTemp) < TE_TEMP_OK_AREA || bOk)			//20100803
		{
			// 进入调温段！

			if(m_TempCtrl[etw_Middle].SetTemp < 1000)
			{
				if(++m_count < 20)
					InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp - m_count * 8 / 9);	//20100805
				else
				{
					bOk = true;
					TempSegChanged(3,TEMPADJUSTSEG);
					
					InterlockedExchange(&theApp.m_lRunState, ers_Store);
					if(theApp.m_WDDDTSS && m_bHasAlarm == FALSE)
					{
						m_bHasAlarm = TRUE;
						CJGDriverMgr::Instance()->Alarm(10);
					}
			
					InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + m_ExcTeTemp);	
				}
			}
			else
			{
				long TempSeg = InterlockedExchangeAdd(&theApp.m_lTeTempCtrlSeg, 0);
				if(++m_count < 3)
				{
					if(TempSeg == TEMPRISESEG)
						InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp + m_count * 5);	//20100805
					else
						InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp - m_count * 5);
				}
				else
				{
					
					bOk = true;
					TempSegChanged(3,TEMPADJUSTSEG);
					InterlockedExchange(&theApp.m_lRunState, ers_Store);
					if(theApp.m_WDDDTSS && m_bHasAlarm == FALSE)
					{
						m_bHasAlarm = TRUE;
						CJGDriverMgr::Instance()->Alarm(10);
					}
			
					InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + m_ExcTeTemp);	
				}
			}
		
		}
		else
		{
			// 按原温度显示！
			InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp);	//20100805
			bOk = false;
		}

		if (m_TempArriveFlag == TEMPARRIVED || bOk)
		{
			if (m_bBeginSetTime == 0)
			{	
				// 第一次计时；
				m_bBeginSetTime = 1;
			}
			else
			{
				// 本节运行了的时间
				InterlockedExchange(&theApp.m_lCurItemRunedTime, 0);
			}
		}
			
	}

	return TRUE;
}

JGUInt8 CModelTask::DoLoop()
{
	// 根据时间来看是否执行完成
	// 设置开始时间

	JGBOOL bOk = false;
	JGMsg lRunMsg; //运行消息	
	InterlockedExchange(&theApp.m_lCurItemRunedTime, 0);
	DWORD dwStartTicket = 0;
	DWORD dwPauseStartTicket = 0;
	DWORD dwPausedDuration = 0;
	DWORD dwTick = GetTickCount(); 
//	DWORD dwTick0 = GetTickCount(); 

    /////////////////////////////////////////
	m_CanAdjust=0;
    m_TeList=1;
	while (TRUE)
	{
		// 执行模块任务
		ModelTempManage(m_TeList);	
		Sleep(1);
		if((GetTickCount() - dwTick) > 1000)
		{
			theApp.getNetMgr()->SendGuardHeartBeat();
			if(m_pScreen != NULL)
				m_pScreen->OnDoTaskNotify(m_nCurrentItemID,m_loopPass,m_indexArr,theApp.m_lCurItemRunedTime,PHASE_RUNNING);
			dwTick = GetTickCount();
		}
		
		lRunMsg = InterlockedExchangeAdd(&theApp.m_msgRun, 0);	
		// 如果要被终止，就退出。
	
		if (lRunMsg == MSG_STOP )
		{			
			//任务被停止
			CJGDriverMgr::Instance()->DisableAllPWM();
			ProAbort = TRUE;
			return 255; //跳转到任务执行结束；
		}
		else if (lRunMsg == MSG_PAUSE)
		{
			// 设置温度暂停状态！！！
			InterlockedExchange(&theApp.m_lRunState, ers_Pause);	
			// 运行状态暂停！！
			InterlockedExchange(&theApp.m_lRunStep, RUN_PAUSE);

			if(dwPauseStartTicket == 0)
			{
				dwPauseStartTicket = GetTickCount();
				
			}
		}
		else // if (lRunMsg == MSG_RUN)
		{
			// 运行状态；
			InterlockedExchange(&theApp.m_lRunState, ers_Run);
			
			// 同步温度信息
			theApp.SetTempCtrlValue(m_TempCtrl);
			
			// 跟设定温度进行比较；若误差在TE_TEMP_OK_AREA度以内就按设置温度显示；
			//照顾低温点，用DispTe而不是CurrTemp
			if((theApp.m_lSetModelTemp == theApp.m_lSetModelTempPre)
				&&(m_GradientFlag == GRADENABLE))//梯度的相邻节点设置温度相同
			{
				bOk = true;
				InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + theApp.m_SetTempReal);
			}
			else
				if (abs(m_TempCtrl[etw_Middle].DispTemp - m_TempCtrl[etw_Middle].SetTemp) < TE_TEMP_OK_AREA || bOk)			//20100803
				{

					m_ModifyDisptemp=1;
                    //////////////////////////////////////////
					if(m_TempCtrl[etw_Middle].SetTemp < 1000)
					{
						if(++m_count < 45)
							InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp - m_count * 8 / 9);	//20100805
						else
						{
							bOk = true;
							if(m_GradientFlag == FALSE)
								InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + m_ExcTeTemp);				
							else
								//		InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + m_AdjTemp[etw_Middle]-m_ExcGradient[etw_Middle]);	
								InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + theApp.m_SetTempReal);
						}
					}
					else
					{
						long TempSeg = InterlockedExchangeAdd(&theApp.m_lTeTempCtrlSeg, 0);
						if(++m_count < 3)
						{
							if(TempSeg == TEMPRISESEG)
								InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp + m_count * 5);	//20100805
							else
								InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp - m_count * 5);
						}
						else
						{
							
							bOk = true;
							if(m_GradientFlag == FALSE)
								InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + m_ExcTeTemp);				
							else
								InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].SetTemp + theApp.m_SetTempReal);	
						}
					}
				}
				else
				{
					// 按原温度显示！
					InterlockedExchange(&theApp.m_lCurModelTemp, m_TempCtrl[etw_Middle].DispTemp);	//20100805
					bOk = false;

					m_ModifyDisptemp=0;
				}
				
				if (m_TempArriveFlag == TEMPARRIVED || bOk)
				{
					/*
					if((m_count<45) && (m_TempCtrl[etw_Middle].SetTemp < 1000))                    
						continue;               //delete in 2012-4-20 by jy
					*/
					// 进入调温段！

					if (m_CanAdjust != 1)         
					{
						continue;
					}

					/////////////////////////////////////////////////////////
					if(theApp.m_WDDDTSS && m_bHasAlarm == FALSE)
					{
						m_bHasAlarm = TRUE;
						CJGDriverMgr::Instance()->Alarm(10);
					}

					TempSegChanged(0,TEMPADJUSTSEG);
					
					
					//Add by jimmy
					if(dwStartTicket == 0)
					{
						dwStartTicket = GetTickCount();
					}
					//Jimmy add the condition: theApp.m_WarnType == wNormal, when there is warning, we stop the running
					if ((lRunMsg != MSG_PAUSE) && ((theApp.m_WarnType == wNormal) || (theApp.m_WarnType == wRadOverflow)))
					{
						if(dwPauseStartTicket != 0)
						{
							dwPausedDuration += (GetTickCount() - dwPauseStartTicket);
							
							dwPauseStartTicket = 0;
							
						}
						
						long lRunTime = (GetTickCount() - dwStartTicket - dwPausedDuration)/1000;
						ASSERT(lRunTime >=0);
						InterlockedExchange(&theApp.m_lCurItemRunedTime, lRunTime);
						
					}
					else//pause
					{
						if(dwPauseStartTicket == 0)
						{
							dwPauseStartTicket = GetTickCount();
							
						}
					}
					
				}			
				
				if (theApp.m_lCurItemRunedTime > m_lItemTime)
				{
					break; //该节运行结束 
				}
				
		}
	}
	
	return 1;
}


// 这个数，只是相加的数据。
long CModelTask::GetTimeMilliseconds()
{
	SYSTEMTIME systemTime;	
	GetLocalTime(&systemTime);
	return systemTime.wSecond * 1000 + systemTime.wMilliseconds;
}

void CModelTask::ModelTempManage(JGUInt8& TeList)
{	
	//	static JGUInt8 TeList = 0;	
	CJGDriverMgr::Instance()->GetModelTemp(TeList, m_TempCtrl[TeList].CurrTemp);
	m_lCurTemp=m_TempCtrl[TeList].CurrTemp;

	////////////////////////////////////////////////
#if DRAWCURVE_MODE == 1
	if(TeList == 1)
	{
		theApp.m_jimmyCurrentT = m_lCurTemp;
	}
#endif
	////////////////////////////////////////////////
	
	long RunState = InterlockedExchangeAdd(&theApp.m_lRunState, 0);
	
	if(RunState != ers_Stop) // 运行状态
	{	
		CJGDriverMgr::Instance()->EnableAll(true);//模块使能
		ModelTempAdjust(TeList);	// 温度控制		
	}		
	TeList++;	
	if(TeList>=3)TeList=0;
}

void CModelTask::ModelDriveOut( JGUInt8 u8Way )
{
	JGBOOL bAsc = (m_TempCtrl[u8Way].PwmOut >=0) ? true : false;
	// 设置最大值
	SetMaxValue(m_TempCtrl[u8Way].PwmOut, MAXTEPWM);
	
//	m_TempCtrl[u8Way].PwmBack  = m_TempCtrl[u8Way].PwmOut;		// 输出备份
	
	unsigned short usPwmDrive = abs(m_TempCtrl[u8Way].PwmOut);
	
	if (usPwmDrive > MAXTEPWM-1)
	{
		usPwmDrive = MAXTEPWM-1;
	}
   
	////////////////////////////////////////
 //   CString str;
//	str.Format(_T("u8Way :%d  usPwmDrive:%2d"),u8Way,usPwmDrive);
//	LogFile::writeLog(str);
    /////////////////////////////////////////

	EnterCriticalSection(&m_cs);
	CJGDriverMgr::Instance()->SetTempCtrl(u8Way, bAsc);
	if (u8Way == etw_Left)
	{
		CJGDriverMgr::Instance()->SetLeftPWMValue(usPwmDrive); // 左
	}
	else if(u8Way == etw_Middle)
	{
		CJGDriverMgr::Instance()->SetMiddlePWMValue(usPwmDrive); // 中
	}
	else if(u8Way == etw_Right)
	{
		CJGDriverMgr::Instance()->SetRightPWMValue(usPwmDrive); // 右	
	} // 左
	LeaveCriticalSection(&m_cs);
}


void split(std::string& s, std::string& delim,std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index=s.find_first_of(delim,last);
	while (index!=std::string::npos)
	{
		ret->push_back(s.substr(last,index-last));
		last=index+1;
		index=s.find_first_of(delim,last);
	}
	if (index-last>0)
	{
		ret->push_back(s.substr(last,index-last));
	}
} 

//lSatus:0 dotask,1: resetModuleData 2:resetModuleTask 3:doStore
void CModelTask::TempSegChanged(long lSatus,long lNewSeg)
{
// 	TRACE(L"CModelTask::TempSegChanged,STATUS = %d,newseg = %d\r\n",lSatus,lNewSeg);
	long lOldSeg = InterlockedExchangeAdd(&theApp.m_lTeTempCtrlSeg, 0);
	InterlockedExchange(&theApp.m_lTeTempCtrlSeg, lNewSeg); 
	if(m_pScreen != NULL)
	{
        m_pScreen->OnTempSegChanged(lSatus,lNewSeg,lOldSeg);
	}
}


void CModelTask::ChangeTempCtrlEx(bool bRiseT,JGUInt8 ch,int nMode,int delta,unsigned char index)
{
	m_TempCtrl[ch].PwmOut=(int)((long)delta*MAXTEPWM* PidCalcT[nMode].P/(long)TePidConst[index].Karea) ;// 比例调整输出
	
	m_TempCtrl[ch].Integral += (long)delta * PidCalcT[nMode].I; 										// 积分累计
	
	if(m_TempCtrl[ch].Integral>MAXTEPWM*(long)TePidConst[index].IConst)									// 积分最值判断
	{
		m_TempCtrl[ch].Integral=MAXTEPWM*(long)TePidConst[index].IConst;
	}
	
	else if(m_TempCtrl[ch].Integral<-MAXTEPWM*(long)TePidConst[index].IConst)
	{
		m_TempCtrl[ch].Integral=-MAXTEPWM*(long)TePidConst[index].IConst;
	}
	
	m_TempCtrl[ch].PwmOut += (long)(m_TempCtrl[ch].Integral/(long)TePidConst[index].IConst);	    			// 积分调整
	

	long dError = delta - m_TempCtrl[ch].LastError;

	m_TempCtrl[ch].PrevError = m_TempCtrl[ch].LastError;
	m_TempCtrl[ch].LastError = delta;

	m_TempCtrl[ch].PwmOut += dError * PidCalcT[nMode].Out * TePidConst[index].Base;

//	TRACE(L"PID:m_TempCtrl[ch].PwmOut = %d,m_TempCtrl[ch].Integral = %d,delta = %d\n",m_TempCtrl[ch].PwmOut,m_TempCtrl[ch].Integral,delta);
	
	/*
	if(bRiseT != false)
		m_TempCtrl[ch].PwmOut += TePidConst[index].Base * PidCalcT[nMode].Out;
	else 
		m_TempCtrl[ch].PwmOut -= TePidConst[index].Base * PidCalcT[nMode].Out;
	*/
}

void CModelTask::ChangeTempCtrl2(int ch,const PIDCALCT* pc,int iType,int iRise,int delta,unsigned char index)
{
	m_TempCtrl[ch].PwmOut=(int)((long)delta*MAXTEPWM* pc[iType*2+iRise].P/(long)TePidConst[index].Karea) ;// 比例调整输出
    
	//	TRACE(L"PID0:m_TempCtrl[ch].PwmOut = %d,delta= %d\n",m_TempCtrl[ch].PwmOut,delta);
	
	m_TempCtrl[ch].Integral += (long)delta * pc[iType*2+iRise].I; 										// 积分累计
	
	if(m_TempCtrl[ch].Integral>MAXTEPWM*(long)TePidConst[index].IConst)									// 积分最值判断
	{
		m_TempCtrl[ch].Integral=MAXTEPWM*(long)TePidConst[index].IConst;
	}
	
	else if(m_TempCtrl[ch].Integral<-MAXTEPWM*(long)TePidConst[index].IConst)
	{
		m_TempCtrl[ch].Integral=-MAXTEPWM*(long)TePidConst[index].IConst;
	}
	
	m_TempCtrl[ch].PwmOut += (long)(m_TempCtrl[ch].Integral/(long)TePidConst[index].IConst);	    			// 积分调整
	
	//	TRACE(L"PID:m_TempCtrl[ch].PwmOut = %d,m_TempCtrl[ch].Integral = %d\n",m_TempCtrl[ch].PwmOut,m_TempCtrl[ch].Integral);
	
	if(iRise==0)
		m_TempCtrl[ch].PwmOut += TePidConst[index].Base * pc[iType*2+iRise].Out;
	else 
		m_TempCtrl[ch].PwmOut -= TePidConst[index].Base * pc[iType*2+iRise].Out;

//	TRACE(L"PID2:m_TempCtrl[ch].PwmOut = %d\n",m_TempCtrl[ch].PwmOut);
}

void CModelTask::ChangeTempCtrl(JGUInt8 ch,int nMode,int delta,unsigned char index)
{
	m_TempCtrl[ch].PwmOut=(int)((long)delta*MAXTEPWM* PidCalcT[nMode].P/(long)TePidConst[index].Karea) ;// 比例调整输出
    
//	TRACE(L"PID0:m_TempCtrl[ch].PwmOut = %d,delta= %d\n",m_TempCtrl[ch].PwmOut,delta);
	
	m_TempCtrl[ch].Integral += (long)delta * PidCalcT[nMode].I; 										// 积分累计
	
	if(m_TempCtrl[ch].Integral>MAXTEPWM*(long)TePidConst[index].IConst)									// 积分最值判断
	{
		m_TempCtrl[ch].Integral=MAXTEPWM*(long)TePidConst[index].IConst;
	}
	
	else if(m_TempCtrl[ch].Integral<-MAXTEPWM*(long)TePidConst[index].IConst)
	{
		m_TempCtrl[ch].Integral=-MAXTEPWM*(long)TePidConst[index].IConst;
	}
	
	m_TempCtrl[ch].PwmOut += (long)(m_TempCtrl[ch].Integral/(long)TePidConst[index].IConst);	    			// 积分调整

//	TRACE(L"PID:m_TempCtrl[ch].PwmOut = %d,m_TempCtrl[ch].Integral = %d\n",m_TempCtrl[ch].PwmOut,m_TempCtrl[ch].Integral);
	
	if(nMode%2 == 0)
		m_TempCtrl[ch].PwmOut += TePidConst[index].Base * PidCalcT[nMode].Out;
	else 
		m_TempCtrl[ch].PwmOut -= TePidConst[index].Base * PidCalcT[nMode].Out;

//	TRACE(L"PID2:m_TempCtrl[ch].PwmOut = %d\n",m_TempCtrl[ch].PwmOut);


}

void CModelTask::ModelTempAdjust( JGUInt8 ch )
{
	int delta;

	unsigned char index;
		
	delta=m_TempCtrl[ch].SetTemp-m_TempCtrl[ch].CurrTemp;
		
	index=abs(m_TempCtrl[ch].SetTemp)/500;

	///////////////////////////////////////////
//	CString str;
//	str.Format(TEXT("way=%d SetTemp=%d CurrTemp=d% delta=%d"),ch,m_TempCtrl[ch].SetTemp,m_TempCtrl[ch].CurrTemp,delta);
//	LogFile::writeLog(str);
	//////////////////////////////////////////////

	//TRACE(L"delta=%d\n,index=%d,TePidConst[index].Karea=%d\n",delta,index,TePidConst[index].Karea);
	if(abs(delta)<TePidConst[index].Karea)								
	{
//		TRACE(L"Set temp = %d,Curr Temp = %d,delta=%d,TePidConst[index].Karea=%d\n",m_TempCtrl[ch].SetTemp,m_TempCtrl[ch].CurrTemp,delta,TePidConst[index].Karea);
		if((m_TempOkFlag!=TETEMPOK)&&(abs(delta)<TE_TEMP_OK_AREA))
		{
			if(m_GradientFlag==GRADENABLE)m_TempOkFlag|=(0x01<<ch);
			else if(ch==etw_Middle)
			{
				m_TempOkFlag=TETEMPOK;
			}

			TRACE(L"m_TempOkFlag = %d,m_GradientFlag=%d\r\n",m_TempOkFlag,m_GradientFlag);
		}
		if(m_TempArriveFlag!=TEMPARRIVED)
		{
			if(m_GradientFlag==GRADENABLE)
			{
				if(m_TempOkFlag==TETEMPOK)
				{
					m_TempArriveFlag=TEMPARRIVED;
					TRACE(L"m_TempArriveFlag=TEMPARRIVED\r\n");
				}
			}
			else
			{
				if((((theApp.m_lTeTempCtrlSeg==TEMPDOWNSEG)&&(delta>0))||((theApp.m_lTeTempCtrlSeg==TEMPRISESEG)&&(delta<0))||
				(theApp.m_lTeTempCtrlSeg==TEMPADJUSTSEG))&&(ch==etw_Middle)&&(m_TempOkFlag==TETEMPOK))
				{
					m_TempArriveFlag=TEMPARRIVED;
				}
			}
		}		
		byte nMode = 0;
		byte nType = 0;
		byte nRise = 0;//0升温 1降温

		if(m_TempCtrl[ch].SetTemp >= m_TempCtrl[ch].SetTempCopy)
			nRise = 0;
		else 
			nRise = 1;

		//降温并且设置温度小于40度，则使用block模式
		if(theApp.m_TempCtrlWay == 0 || (nRise == 1 && (m_TempCtrl[ch].SetTemp < 4000)))
			nMode = 0;
		else if(theApp.m_volume == volume_20 || theApp.m_volume == volume_50)
			nMode = 1;
		else 
			nMode = 2;

//		if(theApp.m_ModelType == 5)
//			nType = 1;
//		else 
//			nType = 0;


//		int nSel = nMode * 4 + nType * 2 + nRise;
		switch (theApp.m_ModelType)
		{
		case 5:
			nType=0;
			break;
		case 6:
			nType=1;
			break;
		case 4:
			nType=2;
			break;
		case 3:
			nType=3;
			nMode = 0;   //为防止意外情况.
			break;
		default:
			nType=0;
//			LogFile::writeLog(_T("Unknown ModelType"));
			break;
		}

		switch(nMode)
		{
		case 0:
			ChangeTempCtrl2(ch,Pid_Block,nType,nRise,delta,index);//for test in 2012-4-23
			break;
		case 1:
			ChangeTempCtrl2(ch,Pid_Tube1,nType,nRise,delta,index);
			break;
		case 2:
            ChangeTempCtrl2(ch,Pid_Tube2,nType,nRise,delta,index);
			break;
		default:
			ChangeTempCtrl2(ch,Pid_Block,0,nRise,delta,index);
			break;
		}

//		ChangeTempCtrl(ch,nSel,delta,index);

		if(m_TempCtrl[ch].PwmOut < 1-MAXTEPWM) m_TempCtrl[ch].PwmOut = 1-MAXTEPWM;
		if(m_TempCtrl[ch].PwmOut > MAXTEPWM-1) m_TempCtrl[ch].PwmOut = MAXTEPWM-1;
 		
		if(m_TempArriveFlag==TEMPARRIVED)
		{
			if(m_GradientFlag==GRADENABLE)
				//m_TempCtrl[ch].DispTemp=m_TempCtrl[ch].SetTemp+m_AdjTemp[ch]-m_ExcGradient[ch];
				m_TempCtrl[ch].DispTemp=m_TempCtrl[ch].SetTemp+theApp.m_SetTempReal;
			else 
			{
				m_TempCtrl[ch].DispTemp=m_TempCtrl[ch].SetTemp+m_ExcTeTemp;
			}
		}
		else 
		ModelTempDisp(ch);
	}
	else																			// 温度差大于比例区间
	{

		m_TempCtrl[ch].SampleTime++;
		if(m_TempCtrl[ch].SampleTime>10)
		{
			m_TempCtrl[ch].SampleTime=0;
			
			if(m_RunGradEn==ENABLE)
			{
				if(delta>0)m_TempCtrl[ch].RampSpeed=(m_TempCtrl[ch].CurrTemp-m_TempCtrl[ch].CurrTempCopy) / 3;//090411xu
				else m_TempCtrl[ch].RampSpeed=(m_TempCtrl[ch].CurrTempCopy-m_TempCtrl[ch].CurrTemp) / 3;//090411xu
			}
			else m_TempCtrl[ch].RampSpeed=abs(m_TempCtrl[ch].CurrTempCopy-m_TempCtrl[ch].CurrTemp);
			
			if(m_GradSpeedEn==ENABLE)
			{
				m_TempCtrl[ch].SpeedInl+=abs(m_TempCtrl[ch].SetSpeed-m_TempCtrl[ch].RampSpeed);
				if(m_TempCtrl[ch].SpeedInl>(MAXTEPWM*GRADSPEEDKI))m_TempCtrl[ch].SpeedInl=(MAXTEPWM*GRADSPEEDKI);//090411xu
				else if(m_TempCtrl[ch].SpeedInl<-(MAXTEPWM*GRADSPEEDKI))m_TempCtrl[ch].SpeedInl=-(MAXTEPWM*GRADSPEEDKI);//090411xu
			}
			else
			{
				if(theApp.m_lTeTempCtrlSeg==TEMPDOWNSEG)m_TempCtrl[ch].SpeedInl+=abs(theApp.m_CoolSpeed/10-m_TempCtrl[ch].RampSpeed);
				else m_TempCtrl[ch].SpeedInl+=abs(theApp.m_HeatSpeed/10-m_TempCtrl[ch].RampSpeed);
				if(m_TempCtrl[ch].SpeedInl>SPEEDKI*MAXTEPWM)m_TempCtrl[ch].SpeedInl=SPEEDKI*MAXTEPWM;
				else if(m_TempCtrl[ch].SpeedInl<-SPEEDKI*MAXTEPWM)m_TempCtrl[ch].SpeedInl=-SPEEDKI*MAXTEPWM;
			}
			m_TempCtrl[ch].CurrTempCopy=m_TempCtrl[ch].CurrTemp;
		}	  

		if(delta>0)
		{
			if(m_GradSpeedEn==ENABLE)
			{
				m_TempCtrl[ch].PwmOut=MAXTEPWM*(long)m_TempCtrl[ch].SetSpeed/MAXGRADSPEED+m_TempCtrl[ch].SpeedInl/GRADSPEEDKI;

			}
			else 
			{
				if((theApp.m_HeatSpeed==0)||(theApp.m_HeatSpeed>TEMP_RAMP_SPEED * 10))m_TempCtrl[ch].PwmOut=MAXTEPWM;
				else if(theApp.m_HeatSpeed<200 && theApp.m_HeatSpeed>=100)	//设置温度分段1-2°
				{
					m_TempCtrl[ch].PwmOut=MAXTEPWM*(long)(theApp.m_HeatSpeed / 10)/TEMP_RAMP_SPEED+m_TempCtrl[ch].SpeedInl/(SPEEDKI * 5);//20100728
					switch(theApp.m_ModelType)
					{
					case 5:
						m_TempCtrl[ch].PwmOut *= 0.75;
						break;
					default:
						break;
					}
				}
				else	//2-2.5°
				{
					m_TempCtrl[ch].PwmOut=MAXTEPWM*(long)(8 * theApp.m_HeatSpeed / 75)/TEMP_RAMP_SPEED+m_TempCtrl[ch].SpeedInl/(SPEEDKI * 6);//20100728
					switch(theApp.m_ModelType)
					{
					case 5:
						m_TempCtrl[ch].PwmOut *= 0.75;
						break;
					default:
						break;
					}
				}
			}
			if(m_TempCtrl[ch].PwmOut>MAXTEPWM-1)m_TempCtrl[ch].PwmOut=MAXTEPWM-1;	//090411xu
			else if(m_TempCtrl[ch].PwmOut<0)m_TempCtrl[ch].PwmOut=0;//090411xu
		}
		else 
		{
			JGInt32 MaxTePwm = 0;
//			if(m_TempCtrl[ch].SetTemp>2000)//设置温度大于20度
//			{
				if(m_GradSpeedEn==ENABLE)
				{
					MaxTePwm=MAXTEPWM*(long)m_TempCtrl[ch].SetSpeed/MAXGRADSPEED+m_TempCtrl[ch].SpeedInl/GRADSPEEDKI;
			
				}
				else 
				{
					if((theApp.m_CoolSpeed==0)||(theApp.m_CoolSpeed>TEMP_RAMP_SPEED * 10))
					{
						MaxTePwm = MAXTEPWM-1;
					}
					else if(theApp.m_CoolSpeed>=100 && theApp.m_CoolSpeed<=150)
					{
						MaxTePwm=MAXTEPWM*(long)(3 * theApp.m_CoolSpeed / 40)/TEMP_RAMP_SPEED+m_TempCtrl[ch].SpeedInl/(SPEEDKI*3);//20100728
						switch(theApp.m_ModelType)
						{
						case 5:
							m_TempCtrl[ch].PwmOut *= 0.75;
							break;
						default:
							break;
						}
					}
					else if(theApp.m_CoolSpeed>150 && theApp.m_CoolSpeed<=200)
					{		
						MaxTePwm=MAXTEPWM*(long)(7 * theApp.m_CoolSpeed / 80)/TEMP_RAMP_SPEED+m_TempCtrl[ch].SpeedInl/(SPEEDKI*3);//20100728
						switch(theApp.m_ModelType)
						{
						case 5:
							m_TempCtrl[ch].PwmOut *= 0.75;
							break;
						default:
							break;
						}
					}
					else
					{
						MaxTePwm=MAXTEPWM*(long)(9 * theApp.m_CoolSpeed / 80)/TEMP_RAMP_SPEED+m_TempCtrl[ch].SpeedInl/(SPEEDKI*5/2);//20100728
						switch(theApp.m_ModelType)
						{
						case 5:
							m_TempCtrl[ch].PwmOut *= 0.75;
							break;
						default:
							break;
						}
					}
				}



				if(MaxTePwm>MAXTEPWM-1)MaxTePwm=MAXTEPWM-1;	//090411 xu	
				m_TempCtrl[ch].PwmOut = -MaxTePwm;
//			}

//			m_TempCtrl[ch].PwmOut = -MAXTEPWM;
		}
		ModelTempDisp(ch);

	}
#if DRAWCURVE_MODE == 1
	theApp.m_PwmOut = m_TempCtrl[ch].PwmOut;
#endif
	ModelDriveOut(ch);																// 输出
																				
//	m_TempCtrl[ch].PreTemp=m_TempCtrl[ch].CurrTemp;									// 记录本次调整时的温度值
}

void CModelTask::ModelTempDisp( JGUInt8 u8Way )
{
	// 模块的类型
	JGUInt8 ModuleType = theApp.m_ModelType;

	JGInt16 ExcDisp;	
	m_TempCtrl[u8Way].DispTemp = m_TempCtrl[u8Way].CurrTemp;
	
    JGUInt8 ind = m_TempCtrl[u8Way].CurrTemp / 500;	// 查找温度段

    if(m_GradientFlag == ENABLE)				// 梯度补偿
	{
        if(ModuleType == 5)//0.2模块
        {
 //         ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab[ind+1][u8Way]-Adj0_2Tab[ind][u8Way]))/500;
   //       ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab[ind][u8Way]-ExcDisp;         
			if(theApp.m_EnvTemp < TEMP_SPRING)
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_0[ind+1][u8Way]-Adj0_2Tab_0[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_0[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_1[ind+1][u8Way]-Adj0_2Tab_1[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_1[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_2[ind+1][u8Way]-Adj0_2Tab_2[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_2[ind][u8Way]-ExcDisp;
				}
				//				ExcDisp -= 20;
			}
			else if(theApp.m_EnvTemp <= TEMP_SUMMER)
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_0Sec[ind+1][u8Way]-Adj0_2Tab_0Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_0Sec[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_1Sec[ind+1][u8Way]-Adj0_2Tab_1Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_1Sec[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_2Sec[ind+1][u8Way]-Adj0_2Tab_2Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_2Sec[ind][u8Way]-ExcDisp;
				}
			}
			else
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_0Thr[ind+1][u8Way]-Adj0_2Tab_0Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_0Thr[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_1Thr[ind+1][u8Way]-Adj0_2Tab_1Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_1Thr[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2Tab_2Thr[ind+1][u8Way]-Adj0_2Tab_2Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2Tab_2Thr[ind][u8Way]-ExcDisp;
				}
				//			ExcDisp += 20;
			}
        }
        else if(ModuleType==6)//0.5模块
        {
//          ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab[ind+1][u8Way]-Adj0_5Tab[ind][u8Way]))/500;
//          ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab[ind][u8Way]-ExcDisp; 
			if(theApp.m_EnvTemp < TEMP_SPRING)
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_0[ind+1][u8Way]-Adj0_5Tab_0[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_0[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_1[ind+1][u8Way]-Adj0_5Tab_1[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_1[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_2[ind+1][u8Way]-Adj0_5Tab_2[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_2[ind][u8Way]-ExcDisp;
				}
				//				ExcDisp -= 20;
			}
			else if(theApp.m_EnvTemp <= TEMP_SUMMER)
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_0Sec[ind+1][u8Way]-Adj0_5Tab_0Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_0Sec[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_1Sec[ind+1][u8Way]-Adj0_5Tab_1Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_1Sec[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_2Sec[ind+1][u8Way]-Adj0_5Tab_2Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_2Sec[ind][u8Way]-ExcDisp;
				}
			}
			else
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_0Thr[ind+1][u8Way]-Adj0_5Tab_0Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_0Thr[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_1Thr[ind+1][u8Way]-Adj0_5Tab_1Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_1Thr[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_5Tab_2Thr[ind+1][u8Way]-Adj0_5Tab_2Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_5Tab_2Thr[ind][u8Way]-ExcDisp;
				}
				//			ExcDisp += 20;
			}
        } 
//        else if(ModuleType == 3)//384模块
//        {
//            ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj384Tab[ind+1][u8Way]-Adj384Tab[ind][u8Way]))/500;
//            ExcDisp=m_AdjTemp[u8Way]-Adj384Tab[ind][u8Way]-ExcDisp;            
//        }        
		else                          //0.2and0.5模块
		{
			if(theApp.m_EnvTemp < TEMP_SPRING)
			{
 				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
 				{
				  ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_0[ind+1][u8Way]-Adj0_2and0_5Tab_0[ind][u8Way]))/500;
				  ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_0[ind][u8Way]-ExcDisp;
 				}
 				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
 				{
 					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_1[ind+1][u8Way]-Adj0_2and0_5Tab_1[ind][u8Way]))/500;
 					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_1[ind][u8Way]-ExcDisp;
 				}
 				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
 				{
 					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_2[ind+1][u8Way]-Adj0_2and0_5Tab_2[ind][u8Way]))/500;
 					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_2[ind][u8Way]-ExcDisp;
 				}
//				ExcDisp -= 20;
			}
			else if(theApp.m_EnvTemp <= TEMP_SUMMER)
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_0Sec[ind+1][u8Way]-Adj0_2and0_5Tab_0Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_0Sec[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_1Sec[ind+1][u8Way]-Adj0_2and0_5Tab_1Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_1Sec[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_2Sec[ind+1][u8Way]-Adj0_2and0_5Tab_2Sec[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_2Sec[ind][u8Way]-ExcDisp;
				}
			}
			else
			{
				if(m_fItemTempGrands > 0 && m_fItemTempGrands <= 5)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_0Thr[ind+1][u8Way]-Adj0_2and0_5Tab_0Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_0Thr[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 5 && m_fItemTempGrands <= 10)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_1Thr[ind+1][u8Way]-Adj0_2and0_5Tab_1Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_1Thr[ind][u8Way]-ExcDisp;
				}
				else if(m_fItemTempGrands > 10 && m_fItemTempGrands <= 15)
				{
					ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Adj0_2and0_5Tab_2Thr[ind+1][u8Way]-Adj0_2and0_5Tab_2Thr[ind][u8Way]))/500;
					ExcDisp=m_AdjTemp[u8Way]-Adj0_2and0_5Tab_2Thr[ind][u8Way]-ExcDisp;
				}
	//			ExcDisp += 20;
			}
        }
	}
	else//平板补偿
	{   

		if(ModuleType == 5)
		{
             ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Exc0_2Tab[ind+1]-Exc0_2Tab[ind]))/500;
             ExcDisp=-Exc0_2Tab[ind]-ExcDisp;
		}
		else if(ModuleType == 6)
		{
             ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Exc0_5Tab[ind+1]-Exc0_5Tab[ind]))/500;
             ExcDisp=-Exc0_5Tab[ind]-ExcDisp;
		}
		else if(ModuleType ==3)
		{
			 ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Exc384Tab[ind+1]-Exc384Tab[ind]))/500;
             ExcDisp=-Exc384Tab[ind]-ExcDisp;			
		}		
	    else
	    {
			ExcDisp=((m_TempCtrl[u8Way].CurrTemp-500*ind)*(long)(Exc0_2and0_5Tab[ind+1]-Exc0_2and0_5Tab[ind]))/500;
			ExcDisp=-Exc0_2and0_5Tab[ind]-ExcDisp;

        }				
    }

	m_TempCtrl[u8Way].DispTemp -= ExcDisp;	
}



