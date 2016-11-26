#include "stdafx.h"
#include "Box.h"


Box::Box()
	: m_isSelected(false)
	, m_isEnd(false)
{
}


Box::~Box()
{
}


CRect Box::GetContent()
{
	return CRect(left + 1, top + 1, right - 1, bottom - 1);
}
