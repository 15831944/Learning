#pragma once
class Ball
{
public:
	Ball();
	~Ball();
	int x;
	int y;
	int radii;
	int m_rowIndex;
	int m_colIndex;
	CRect GetContainer();
};

