// GPIODriver.h: interface for the CGPIODriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GPIODRIVER_H__D85053D2_4CC4_4E1E_B21F_257588AA9811__INCLUDED_)
#define AFX_GPIODRIVER_H__D85053D2_4CC4_4E1E_B21F_257588AA9811__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "drive.h"

class CJGDriverMgr;

class CGPIODriver  
{
public:
	CGPIODriver();
	virtual ~CGPIODriver();

	friend CJGDriverMgr;

protected:

	// ʹ�ܣ�ȫ��оƬ��Ч
	JGBOOL EnableAll(JGBOOL bEnable = true);

	// ����ģ��ĵ�Դ����
	JGBOOL ModelPower(JGBOOL bOpen = true);
	
	// ��������
	JGBOOL Sound(JGBOOL bAlarm = true);

	// ʹSPIоƬ��Ч
	JGBOOL EnableSPI(JGBOOL bEnable = true);

	// ��ȡ�ȸ���Ϣ �Ƿ��Ѿ��رպ���
	// ��ʱû���õ���
	JGBOOL	IsLidReady();
	
	// ��ȡģ����Ϣ...
	JGUInt8 GetModeType();

	JGUInt8 GetType(JGUInt8& u6, JGUInt8& u7, JGUInt8& u9);
	
	// ����ģ����¶��Ǽ��Ȼ�������...
	// ��·ͨ�� bAsc  1 ��ʾ���ȣ�0 ��ʾ����
	JGBOOL TrimModelTemp(JGBOOL bAsc);
	// u8Way��Ӧ��ͨ��
	JGBOOL TrimModelTemp(JGUInt8 u8Way, JGBOOL bAsc);
	
	// �����ȸǵ��¶��Ǽ��Ȼ��ǽ���...
	JGBOOL TrimLidTemp(JGBOOL bAsc);

protected:

	// ��������
	JGBOOL OpenDriver();
	
	// �ر�����
	JGBOOL CloseDriver();	

private:

	HANDLE m_hGPIODriver;
	
};

#endif // !defined(AFX_GPIODRIVER_H__D85053D2_4CC4_4E1E_B21F_257588AA9811__INCLUDED_)
