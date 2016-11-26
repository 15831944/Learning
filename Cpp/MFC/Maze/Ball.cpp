#include "stdafx.h"
#include "Ball.h"


Ball::Ball()
	: m_rowIndex(0)
	, m_colIndex(0)
{
}


Ball::~Ball()
{
}


CRect Ball::GetContainer()
{
	return CRect(x - radii, y - radii, x + radii + 1, y + radii + 1);
}
