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

// ���� �ȸǡ�ɢ���������ȵȿ���

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

	// �õ�Adc��ص����ݣ�
	void	GetAdcData();
	
	// �ȸǿ���
	void	LidCtrl();
	
	// ���ȿ���
	void	FanCtrl();

	void FanCtrl2();
	
	// ɢ����Ԥ�ȹ���
	void	RadWarmCouse();

	// ɢ����Ԥ��
	void	RadWarmUpCtrl();

	void FanLidPwmManage(void);

private:
	
	JGUInt16	m_LidPwmCnt;	// ִ���ȸǴ���

	JGUInt16	m_LidPwmOut;	// �ȸ�PWMֵ

	JGInt32		m_LidIntegral;  // �ȸǻ���

	JGInt16		m_LidTemp;		// ��ǰ�ȸ��¶�
	
	JGInt16		m_RadTemp;		// ɢ�����¶�

	JGUInt16	m_RadWarmUpCnt;	// ɢ����Ԥ�ȴ���
	JGUInt32	m_FanTimeLasting;

	BOOL		m_bRadExit;		//�ȸǳ���70�ȣ����µ�60��

	DWORD dwTick;
};

#endif // !defined(AFX_ADCTASK_H__3BD9A6C6_01D6_40DC_9AA9_2CFBED333525__INCLUDED_)
