// HHOOPS08SelectionSet.cpp : implementation of the HPSelectionSet class
// 
#include "StdAfx.h"
#include "Resource.h"

#include "HHOOPSSelectionSet.h"
#include "HHOOPSModel.h"
#include "HHOOPSView.h"
#include "vlist.h"

HHOOPSSelectionSet::HHOOPSSelectionSet(HBaseView* view) : HSelectionSet(view)
{
	m_SelectLevel = BODY_TYPE;	// set default level to body
	SetAllowEntitySelection(true);
}


HHOOPSSelectionSet::~HHOOPSSelectionSet()
{
	if( m_pSolidSelection )
	{
		delete_vlist(m_pSolidSelection);
		m_pSolidSelection = 0;
	}
}

// create a new list  object
void HHOOPSSelectionSet::Init()
{
	m_pSolidSelection = new_vlist(malloc, free);
	HSelectionSet::Init();
}

int HHOOPSSelectionSet::GetSolidListSize()
{
	return vlist_count( m_pSolidSelection );
}

long HHOOPSSelectionSet::GetAtSolidEntity(int index)
{
	return (long) vlist_nth_item( m_pSolidSelection, index );
}

void HHOOPSSelectionSet::Reset()
{
	if( m_pSolidSelection )
	{
		delete_vlist(m_pSolidSelection);
		m_pSolidSelection = new_vlist(malloc, free);
	}

	HSelectionSet::Reset();
}

// deselect all items as in DeSelect
void HHOOPSSelectionSet::DeSelectAll()
{
	if( m_pSolidSelection )
	{
		delete_vlist(m_pSolidSelection);
		m_pSolidSelection = new_vlist(malloc, free);
	}

	HSelectionSet::DeSelectAll();
	m_pView->EmitDeSelectAllMessage();
}


void HHOOPSSelectionSet::Select( HC_KEY key, int num_include_keys, HC_KEY * include_keys, bool emit_message)
{
	HC_KEY* sel_keys = 0;
	int		count = 0;

	ENTITY* entity = 0;

	if (((HHOOPSModel *) HSelectionSet::m_pView->GetModel())->IsSolidModel() != true)
	{
		char	type[64];
		HC_KEY	segkey;

		HC_Show_Key_Type(key, type);
		if (!streq(type, "segment"))
			segkey = HC_KShow_Owner_By_Key(key);
		else
			segkey = key;

		HSelectionSet::Select( segkey, num_include_keys, include_keys, emit_message);
	}
	else
	{
		entity = HA_Compute_Entity_Pointer(key, m_SelectLevel);
		if (entity)
		{
			vlist_add_last(m_pSolidSelection, (void*)entity);
			if (m_SelectLevel == BODY_TYPE)  // && body segments is true 
			{
				sel_keys = new HC_KEY[1];
				count = HA_Compute_Geometry_Keys(entity, 1, sel_keys, "bodies");
			}
			else if (m_SelectLevel == FACE_TYPE)
			{
				count = HA_Compute_Geometry_Key_Count(entity, "faces");
				if( count > 0 )
				{
					sel_keys = new HC_KEY[count];
					HA_Compute_Geometry_Keys(entity, count, sel_keys, "faces");
				}
			}
			else if (m_SelectLevel == EDGE_TYPE)
			{
				count = HA_Compute_Geometry_Key_Count(entity, "edges");
				if( count > 0 )
				{
					sel_keys = new HC_KEY[count];
					HA_Compute_Geometry_Keys(entity, count, sel_keys, "edges");
				}
			}
			else if (m_SelectLevel == VERTEX_TYPE)
			{
				count = HA_Compute_Geometry_Key_Count(entity, "vertices");
				if( count > 0 )
				{
					sel_keys = new HC_KEY[count];
					HA_Compute_Geometry_Keys(entity, count, sel_keys, "vertices");
				}
			}
		}		 
		for (int c = 0; c < count; c++)
			HSelectionSet::Select(sel_keys[c],  num_include_keys, include_keys, emit_message);  // call base class fcn for each key

		if( sel_keys )
			delete [] sel_keys;
	}
}


void HHOOPSSelectionSet::Select(ENTITY* entity, bool emit_message)
{
	HC_KEY*	sel_keys = 0;
	int		count = 0;
	int eclass;	
	eclass = entity->identity();
	if (entity)
	{
		vlist_add_last(m_pSolidSelection, (void*)entity);
		if (m_SelectLevel == BODY_TYPE)  // && body segments is true 
		{
			sel_keys = new HC_KEY[1];
			count = HA_Compute_Geometry_Keys(entity, 1, sel_keys, "bodies");
		}
		else if (m_SelectLevel == FACE_TYPE)
		{
			count = HA_Compute_Geometry_Key_Count(entity, "faces");
			if( count > 0 )
			{
				sel_keys = new HC_KEY[count];
				HA_Compute_Geometry_Keys(entity, count, sel_keys, "faces");
			}
		}
		else if (m_SelectLevel == EDGE_TYPE)
		{
			count = HA_Compute_Geometry_Key_Count(entity, "edges");
			if( count > 0 )
			{
				sel_keys = new HC_KEY[count];
				HA_Compute_Geometry_Keys(entity, count, sel_keys, "edges");
			}
		}
	}

	for (int c = 0; c < count; c++)
		HSelectionSet::Select(sel_keys[c], "", INVALID_KEY, INVALID_KEY, emit_message);  // call base class fcn for each key

	if( sel_keys )
		delete [] sel_keys;

}

void HHOOPSSelectionSet::DeSelect( HC_KEY key, bool emit_message )
{
	HC_KEY* sel_keys = 0;
	int		count = 0;
	if (((HHOOPSModel *) m_pView->GetModel())->IsSolidModel() != true)
	{
		char	type[1024];
		HC_KEY	segkey;

		HC_Show_Key_Type(key, type);
		if (!streq(type, "segment"))
			segkey = HC_KShow_Owner_By_Key(key);
		else
			segkey = key;

		HSelectionSet::DeSelect(segkey, emit_message);
	}
	else
	{
		ENTITY*	entity = 0;
		entity = HA_Compute_Entity_Pointer(key, m_SelectLevel);
		if (entity)
		{
			vlist_remove(m_pSolidSelection, (void*)entity);
			if (m_SelectLevel == BODY_TYPE)  // && body segments is true 
			{
				sel_keys = new HC_KEY[1];
				count = HA_Compute_Geometry_Keys(entity, 1, sel_keys, "bodies");
			}
			else if (m_SelectLevel == FACE_TYPE)
			{
				count = HA_Compute_Geometry_Key_Count(entity, "faces");
				if( count > 0 )
				{
					sel_keys = new HC_KEY[count];
					HA_Compute_Geometry_Keys(entity, count, sel_keys, "faces");
				}
			}
			else if (m_SelectLevel == EDGE_TYPE)
			{
				count = HA_Compute_Geometry_Key_Count(entity, "edges");
				if( count > 0 )
				{
					sel_keys = new HC_KEY[count];
					HA_Compute_Geometry_Keys(entity, count, sel_keys, "edges");
				}
			}
		}
		if (count > 0)
		{
			for( int c = 0; c < count; c++)
				HSelectionSet::DeSelect(sel_keys[c], emit_message);	// call base class fcn for each key
		}
		else
			HSelectionSet::DeSelect(key, emit_message);
	}
	if( sel_keys )
		delete[] sel_keys;
}


void HHOOPSSelectionSet::DeSelectEntity(ENTITY* entity, bool emit_message )
{
	HC_KEY*	sel_keys = 0;
	int		count = 0;

	if (entity)
	{
		vlist_remove(m_pSolidSelection, (void*) entity);
		if (m_SelectLevel == BODY_TYPE)  // && body segments is true 
		{
			sel_keys = new HC_KEY[1];
			count = HA_Compute_Geometry_Keys(entity, 1, sel_keys, "bodies");
		}
		else if (m_SelectLevel == FACE_TYPE)
		{
			count = HA_Compute_Geometry_Key_Count(entity, "faces");
			if( count > 0 )
			{
				sel_keys = new HC_KEY[count];
				HA_Compute_Geometry_Keys(entity, count, sel_keys, "faces");
			}
		}
		else if (m_SelectLevel == EDGE_TYPE)
		{
			count = HA_Compute_Geometry_Key_Count(entity, "edges");
			if( count > 0 )
			{
				sel_keys = new HC_KEY[count];
				HA_Compute_Geometry_Keys(entity, count, sel_keys, "edges");
			}
		}
	}

	if (count > 0)
	{
		for( int c = 0; c < count; c++)
			HSelectionSet::DeSelect(sel_keys[c], emit_message);	// call base class fcn for each key
	}
	if( sel_keys )
		delete[] sel_keys;
}

bool HHOOPSSelectionSet::IsSelected(HC_KEY key)
{
	if (((HHOOPSModel *) m_pView->GetModel())->IsSolidModel() != true)
	{
		char	type[1024];
		HC_Show_Key_Type(key, type);
		if (!streq(type, "segment"))
			key = HC_KShow_Owner_By_Key(key);
	}
	else
	{
		int count;
		if (m_SelectLevel == BODY_TYPE)  // && body segments is true
		{
			ENTITY* entity = 0;
			entity = HA_Compute_Entity_Pointer(key, m_SelectLevel);
			if (entity)
				count = HA_Compute_Geometry_Keys(entity, 1, &key, "bodies");
			assert(count == 1);
		}
	}
	// call base class function with the computed key
	return HSelectionSet::IsSelected(key);
}


// If it is a call to select something from HNet message,
// by pass our selection functionality and just call the 
// vanilla Select. JUST SELECT THIS KEY!
// This is to work around the case where the master client
// is in some selection mode (face) and slave is in some (body)
void HHOOPSSelectionSet::SelectFromMessage(HC_KEY key, int num_include_keys, HC_KEY * include_keys, bool emit_message)
{
	HSelectionSet::Select( key, num_include_keys, include_keys,  emit_message);
}

//	See the Description for SelectFromMessage
void HHOOPSSelectionSet::DeSelectFromMessage(HC_KEY key, bool emit_message)
{
	HSelectionSet::DeSelect(key, emit_message);
}

