// SPIDriver.h: interface for the CSPIDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPIDRIVER_H__64F112A1_A4BD_4E53_B7DF_D70D57CF3B63__INCLUDED_)
#define AFX_SPIDRIVER_H__64F112A1_A4BD_4E53_B7DF_D70D57CF3B63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drive.h"

class CJGDriverMgr;

class CSPIDriver  
{
public:
	CSPIDriver();
	virtual ~CSPIDriver();

	friend CJGDriverMgr;
	
protected:
	JGBOOL GetTemp(JGUInt8& nTemp);	//��ȡ�����¶�
	JGBOOL GetTemp(JGUInt8 u8Way, JGInt16 & nTemp);

protected:

	// ת���¶ȣ�
	JGBOOL	ChangeValue2Temp(JGUInt16 uValue, JGInt16 & nTemp);

	// ��������
	JGBOOL	OpenDriver();
	
	// �ر�����
	JGBOOL	CloseDriver();		
	
private:

	HANDLE m_hSPIDriver;
	HANDLE m_hAteDriver;	//����ʶ�𴫸���

};

extern const JGInt16 MAXTETMP;
extern const JGInt16 MINTETMP;
#endif // !defined(AFX_SPIDRIVER_H__64F112A1_A4BD_4E53_B7DF_D70D57CF3B63__INCLUDED_)
