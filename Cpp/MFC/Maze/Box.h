#pragma once
#include "atltypes.h"
class Box :
	public CRect
{
public:
	Box();
	~Box();
	CRect GetContent();
	bool m_isSelected;
	bool m_isEnd;
};

