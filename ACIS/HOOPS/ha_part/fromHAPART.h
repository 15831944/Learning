#pragma once
#include "ha_part.h"
public ref class fromHAPART
{
public:
	fromHAPART();
	~fromHAPART();

	int sub(int a, int b);

	HA_Part* m_part;
	void GetInstance();
	PART* GetPart();
	void AddEntities(int id);
};

