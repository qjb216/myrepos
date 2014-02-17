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

	

	// �õ���ǰ���е�ʱ��ڵ� ���� 
	long GetTimeMilliseconds( );

	// �õ����е�����
	void ResetModelData(JGUInt16 uRunsNum);

	// ģ���¶ȿ���
	void ModelTempManage(JGUInt8& TeList);

	//PWM ��� u8Way ���ȵ�·��
	void ModelDriveOut(JGUInt8 u8Way);
	
	// �¶ȿ���
	void ModelTempAdjust(JGUInt8 u8Way);

	// �¶���ʾ����
	void ModelTempDisp(JGUInt8 u8Way);

private:
	CString log_szGetTemp;
	CString log_czMidTemp;
    void ChangeTempCtrl2(int ch,const PIDCALCT* pc,int iType,int iRise,int delta,unsigned char index);
	void ChangeTempCtrlEx(bool bRiseT,JGUInt8 ch,int nMode,int delta,unsigned char index);
	void ChangeTempCtrl(JGUInt8 ch,int nMode,int delta,unsigned char index);
	// �����¶ȶ�Ҫ����100�������㡣500 == 5�ȡ�

	JGUInt8					m_RunGradEn;				// �ݶ�����ʹ��
	JGUInt8					m_GradSpeedEn;				// �ݶ��ٶ�ʹ��
	JGUInt8					m_GradientFlag;				// �ݶȱ��
	JGUInt8					m_TempOkFlag;				// �¶��Ƿ���̬���	
	JGUInt8					m_TempArriveFlag;			// �¶��Ƿ񵽴��趨ֵ���
	
	TMPCTRLVAR				m_TempCtrl[etw_Size];		// ����ģ��
	JGInt16					m_SetTDif[etw_Size];		//��¼�����¶ȱ仯ֵ
	JGInt16					m_SetTDifMax;
	JGInt16					m_AdjTemp[etw_Size];		// �ݶ��¶�����ֵ
	JGInt16					m_ExcGradient[etw_Size];	// �ݶ�����
	JGInt16					m_ExcTeTemp; 				// ģ��ƽ���¶�����ֵ
	JGBOOL					m_bAdjust;					//�ڵ��һ�ν����������
	CRITICAL_SECTION		m_cs;
	long					m_loopCount;				//�ϵ���������ѭ�����ǰ��ѭ����Ŀ

	//////////////////////////////////////////////////////////////////////////

	long				m_lItemObjectId;	// ���е�objectid
	long				m_lItemTime;		// ��Ҫ���е�ʱ��
	long				m_lItemTimeStep;	// ʱ�������ݼ�
	float				m_fItemTemp;		// �ڵ��¶�
	float				m_fItemTempStep;	// �¶ȵ�����ݼ�
	long				m_fItemTempGrands;			// �ݶ�ֵ��
	// ������±���״̬
	long				m_bStoreState;		// ���±���״̬

	// ��ʱ���
	JGUInt8				m_bBeginSetTime;	// ��ʼ��ʱ��

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
