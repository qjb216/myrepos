// ModelTask.h: interface for the CModelTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODELTASK_H__336C3E09_B86D_4095_9E44_861507AA6528__INCLUDED_)
#define AFX_MODELTASK_H__336C3E09_B86D_4095_9E44_861507AA6528__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Task.h"
#include "../Data/UserConfigManager.h"
class CDialog_JGTrend_Run;
class CModelTask  : public CTask
{
private:
	bool m_bStoring;
//	int m_nDeep;
public:
	void ExitStoring(){m_bStoring = false;};
private:
	CDialog_JGTrend_Run *m_pScreen;
	void TempSegChanged(long lSatus,long lNewSeg);
public:
	//void setScreen(CDialog_JGTrend_Run *pScreen){m_pScreen = pScreen;};
	CModelTask(CDialog_JGTrend_Run *pScreen);
	virtual ~CModelTask();

	virtual JGBOOL DoTask();
	virtual JGBOOL DoStore();
	void ResetModelStore();
protected:

	int m_nCurrentItemID;
	LOOPPASS m_loopPass[MAX_LOOP];
	long m_indexArr[MAX_LOOP + 1];

	JGUInt8 Do(const CUserDataManager::CTrendDataArr * pData,short nPassedItems, JGUInt16 uLoopNums,JGUInt16 uLoopID,int nDeep);
	
	JGUInt8 DoLoop();

	

	// 得到当前运行的时间节点 毫秒 
	long GetTimeMilliseconds( );

	// 得到运行的数据
	void ResetModelData(JGUInt16 uRunsNum);

	// 模块温度控制
	void ModelTempManage(JGUInt8& TeList);

	//PWM 输出 u8Way 加热的路线
	void ModelDriveOut(JGUInt8 u8Way);
	
	// 温度控制
	void ModelTempAdjust(JGUInt8 u8Way);

	// 温度显示调整
	void ModelTempDisp(JGUInt8 u8Way);

private:
	CString log_szGetTemp;
	CString log_czMidTemp;
    void ChangeTempCtrl2(int ch,const PIDCALCT* pc,int iType,int iRise,int delta,unsigned char index);
	void ChangeTempCtrlEx(bool bRiseT,JGUInt8 ch,int nMode,int delta,unsigned char index);
	void ChangeTempCtrl(JGUInt8 ch,int nMode,int delta,unsigned char index);
	// 所有温度都要扩大100倍来计算。500 == 5度。

	JGUInt8					m_RunGradEn;				// 梯度爬破使能
	JGUInt8					m_GradSpeedEn;				// 梯度速度使能
	JGUInt8					m_GradientFlag;				// 梯度标记
	JGUInt8					m_TempOkFlag;				// 温度是否稳态标记	
	JGUInt8					m_TempArriveFlag;			// 温度是否到达设定值标记
	
	TMPCTRLVAR				m_TempCtrl[etw_Size];		// 控制模块
	JGInt16					m_SetTDif[etw_Size];		//记录设置温度变化值
	JGInt16					m_SetTDifMax;
	JGInt16					m_AdjTemp[etw_Size];		// 梯度温度修正值
	JGInt16					m_ExcGradient[etw_Size];	// 梯度修正
	JGInt16					m_ExcTeTemp; 				// 模块平板温度修正值
	JGBOOL					m_bAdjust;					//节点第一次进入调控区间
	CRITICAL_SECTION		m_cs;
	long					m_loopCount;				//断电后计算运行循环结点前的循环数目

	//////////////////////////////////////////////////////////////////////////

	long				m_lItemObjectId;	// 运行的objectid
	long				m_lItemTime;		// 需要运行的时间
	long				m_lItemTimeStep;	// 时间递增或递减
	float				m_fItemTemp;		// 节点温度
	float				m_fItemTempStep;	// 温度递增或递减
	long				m_fItemTempGrands;			// 梯度值；
	// 进入低温保存状态
	long				m_bStoreState;		// 低温保存状态

	// 计时标记
	JGUInt8				m_bBeginSetTime;	// 开始计时；

	long				m_lCurTemp;

	JGBOOL				m_bHasAlarm;

	DWORD				m_count;

	DWORD				m_timeStop;

	JGUInt8             m_TeList;
public:
	BOOL                m_ModifyDisptemp;
	BOOL                m_CanAdjust;
	long                m_lItemTemp;
};

#endif // !defined(AFX_MODELTASK_H__336C3E09_B86D_4095_9E44_861507AA6528__INCLUDED_)
