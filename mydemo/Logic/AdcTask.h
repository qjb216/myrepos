// AdcTask.h: interface for the CAdcTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADCTASK_H__3BD9A6C6_01D6_40DC_9AA9_2CFBED333525__INCLUDED_)
#define AFX_ADCTASK_H__3BD9A6C6_01D6_40DC_9AA9_2CFBED333525__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Dialog_JGTrend_Run.h"
#include "Task.h"
#include <winbase.h>

// 负责 热盖、散热器、风扇等控制

class CAdcTask : public CTask
{
private:
	CDialog_JGTrend_Run* m_pRunWnd;	
	bool m_bDownTo60;
	CRITICAL_SECTION m_cs;
	void Lock(){EnterCriticalSection(&m_cs);};
	void UnLock(){LeaveCriticalSection(&m_cs);};

public:
	void RegisterRunWnd(CDialog_JGTrend_Run* pRunWnd)
	{
		m_pRunWnd = pRunWnd;
	};
	void UnRegisterRunWnd()
	{
		m_pRunWnd = NULL;
	};

public:
	void LidCtrlEx();
	void TurnOnFan(bool);
	bool IsRadNeedCool();
	CAdcTask();
	virtual ~CAdcTask();

	virtual JGBOOL DoTask();
private:
	DWORD dwCount;

protected:

	// 得到Adc相关的数据：
	void	GetAdcData();
	
	// 热盖控制
	void	LidCtrl();
	
	// 风扇控制
	void	FanCtrl();

	void FanCtrl2();
	
	// 散热器预热管理
	void	RadWarmCouse();

	// 散热器预热
	void	RadWarmUpCtrl();

	void FanLidPwmManage(void);

private:
	
	JGUInt16	m_LidPwmCnt;	// 执行热盖次数

	JGUInt16	m_LidPwmOut;	// 热盖PWM值

	JGInt32		m_LidIntegral;  // 热盖积分

	JGInt16		m_LidTemp;		// 当前热盖温度
	
	JGInt16		m_RadTemp;		// 散热器温度

	JGUInt16	m_RadWarmUpCnt;	// 散热器预热次数
	JGUInt32	m_FanTimeLasting;

	BOOL		m_bRadExit;		//热盖超过70度，降温到60度

	DWORD dwTick;
};

#endif // !defined(AFX_ADCTASK_H__3BD9A6C6_01D6_40DC_9AA9_2CFBED333525__INCLUDED_)
