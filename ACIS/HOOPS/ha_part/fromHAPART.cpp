#include "fromHAPART.h"



fromHAPART::fromHAPART()
{
}

fromHAPART::~fromHAPART()
{
	if (m_part!=nullptr)
	{
		delete m_part;
	}
}

int fromHAPART::sub(int a, int b)
{
	return a - b;
}

void fromHAPART::GetInstance()
{
	m_part = new HA_Part();
}

PART* fromHAPART::GetPart()
{
	return m_part->GetPart();
}

void fromHAPART::AddEntities(int id)
{
	m_part->AddEntities((const ENTITY_LIST&)id);
}
