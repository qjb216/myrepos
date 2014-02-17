// BuffBuilder.cpp: implementation of the CBuffBuilder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TemperatureCtrl.h"
#include "BuffBuilder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CBuffBuilder::CBuffBuilder() : m_lpData(NULL), 
							m_uSumSize(0),
							m_uUsedSize(0), 
							m_uAutoIncrease(4096)
{
	
}

CBuffBuilder::~CBuffBuilder()
{
	Clear();
}

void CBuffBuilder::SetSumSize(size_t sumsize)
{
	Clear();
	
	m_uSumSize  = sumsize;
	
	m_lpData    = (byte *)malloc(m_uSumSize * sizeof(byte));
	
	ASSERT(m_lpData);
}

void CBuffBuilder::Append(const void * lpVoid, size_t usize)
{	
	if(!lpVoid || usize == 0) 
	{
		return;
	}
	
	if(m_uUsedSize + usize + 4 >= m_uSumSize)
	{
		while(m_uUsedSize + usize + 4 >= m_uSumSize )  
		{ 
			m_uSumSize += m_uAutoIncrease;
		}
		
		m_lpData = (byte *)realloc(m_lpData, m_uSumSize);
		
		ZeroMemory(m_lpData + m_uUsedSize, (m_uSumSize - m_uUsedSize) * sizeof(byte));
	}
	
	memcpy(m_lpData + m_uUsedSize, lpVoid, usize);
	
	m_uUsedSize += usize;
}

void CBuffBuilder::Clear()
{
	if (m_lpData)
	{
		free(m_lpData);
		m_lpData = NULL;
	}
	
	m_uSumSize = 0;
	
	m_uUsedSize = 0;
}

