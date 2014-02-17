// Task.h: interface for the CTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASK_H__864C5C6A_ADD1_446B_ADB6_9968513511F7__INCLUDED_)
#define AFX_TASK_H__864C5C6A_ADD1_446B_ADB6_9968513511F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTask  
{
public:
	CTask();
	virtual ~CTask();

	virtual JGBOOL DoTask() = 0;

	template<class TInt>
		void SetMaxValue(TInt & val, TInt maxVal)
	{
		ASSERT(maxVal >= 0);
		if (val > maxVal)
		{
			val = maxVal;
		}
		else if (val < -maxVal)
		{
			val = -maxVal;
		}
	}
	
protected:

};

#endif // !defined(AFX_TASK_H__864C5C6A_ADD1_446B_ADB6_9968513511F7__INCLUDED_)
