/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif // NT

// Added for hpia_64-
#ifdef hp700
#include <new.h>
#endif

#include "ha_map.h"
#include "ha_map_asm.h"
#include "ha_util.h"
#include "body.hxx"
#include "face.hxx"
#include "edge.hxx"
#include "vertex.hxx"
#include "point.hxx"
#include "wcs.hxx"
#include "text.hxx"
#include "rlt_util.hxx"
#include "ga_api.hxx"
#include "at_ent.hxx"
#include "asm_assembly.hxx"
#include "asm_model_ref.hxx"
#include "entity_handle.hxx"
#include "asm_model.hxx"
#include "ha_bridge.err"

LOCAL_PROC entity_handle* get_owner_handle( entity_handle* eh )
{
	entity_handle* owner = NULL;
	if ( eh )
	{
		ENTITY* entity = eh->entity_ptr();
		asm_model* model = eh->get_owning_model();
		ENTITY* top_level = NULL;
		api_get_owner( entity, top_level );
		owner = model->get_entity_handle( top_level );
	}
	return owner;
}

void HA_EHandleMap::AddMapping( HC_KEY key, entity_handle* eh, entity_handle* owner )
{
	ENTITY* entity = eh->entity_ptr();
	// entity should be registered in Hoops Map only if the entity is a Body, Face, Edge, Vertex, Apoint, WCS, 
	if ( is_ASM_ASSEMBLY( entity ) ||
		 is_ASM_MODEL_REF( entity ) ||
		 is_BODY( entity ) ||
		 is_FACE( entity ) ||
		 is_EDGE( entity ) ||
		 is_VERTEX( entity ) ||
		 is_APOINT( entity ) ||
		 is_WCS( entity ) ||
		 is_TEXT_ENT( entity ) ||
		 IS_LIGHT( entity ) )
	{
		entity_handle *tl = 0;
		if ( !owner )
			tl = get_owner_handle( eh );
		else
			tl = owner;
		if ( is_EDGE( entity ) )
		{	// mgp (11/01/04): Is this a silhouette edge?
			ATTRIB_GEN_NAME *attrib = NULL;
			api_find_named_attribute( entity, "HA_SilhouetteFace", attrib );

			if ( attrib && attrib->identity( ATTRIB_GEN_ENTITY_LEVEL ) == ATTRIB_GEN_ENTITY_TYPE )
			{
				ATTRIB_GEN_ENTITY *att_ent = (ATTRIB_GEN_ENTITY*)attrib;
				ENTITY            *face = att_ent->value();
				if ( face && is_FACE( face ) )
				{
					asm_model* model = eh->get_owning_model();
					entity_handle* fh = model->get_entity_handle( face );
					AddMapping( key, fh, owner );
					return;
				}
			}
		}
		m_Top_Maps[tl].AddMapping( key, eh );
		m_HOOPS_to_ACIS[key] = eh;
	}
}

void HA_EHandleMap::DeleteMapping( HC_KEY key )
{
	m_HOOPS_to_ACIS.erase( key );
}

void HA_EHandleMap::DeleteMapping( entity_handle* eh )
{
	entity_handle* tl = get_owner_handle( eh );
	if ( eh == tl )
	{	// Before using the [] operator test if the entity exists in that map.
		T_ehandle_to_HA_TL_EHandleMap_const_iterator tempConstIterator = m_Top_Maps.find( tl );
		if ( tempConstIterator == m_Top_Maps.end() )
			return;
		T_ehandle_to_segment_list_iterator entity_to_segment_list_end = m_Top_Maps[tl].m_ACIS_to_HOOPS.end();
		for ( T_ehandle_to_segment_list_iterator l = m_Top_Maps[tl].m_ACIS_to_HOOPS.begin(); l != entity_to_segment_list_end; ++l )
		{
			entity_handle *map_ent = (entity_handle*)( *l ).first;
			T_segment_list_const_iterator end = m_Top_Maps[tl].m_ACIS_to_HOOPS[map_ent].end();
			for ( T_segment_list_const_iterator p = m_Top_Maps[tl].m_ACIS_to_HOOPS[map_ent].begin(); p != end; ++p )
			{
				HC_KEY key = *p;
				m_HOOPS_to_ACIS.erase( key );
			}
		}
		m_Top_Maps.erase( eh );
	} else
	{		// fang Aug 5 2004. Fixed up top map. Before fixing, top map can't be cleared if the entity is absorbed into another entity
		// if the entity is still in top level map, erase it
		if ( ( m_Top_Maps.find( eh ) ) != m_Top_Maps.end() )
			m_Top_Maps.erase( eh );
		else
		{
			T_ehandle_to_HA_TL_EHandleMap_const_iterator tempConstIterator = m_Top_Maps.find( tl );
			if ( tempConstIterator != m_Top_Maps.end() )
				m_Top_Maps[tl].DeleteMapping( eh );
		}
	}
}

void HA_EHandleMap::DeleteMapping( HC_KEY key, entity_handle* eh )
{
	entity_handle* tl = get_owner_handle( eh );
	if ( eh == tl )
		m_Top_Maps.erase( eh );
	else
	{	// Before using the [] operator test if the entity exists in that map.
		T_ehandle_to_HA_TL_EHandleMap_const_iterator tempConstIterator = m_Top_Maps.find( tl );
		if ( tempConstIterator == m_Top_Maps.end() )
			return;
		m_Top_Maps[tl].DeleteMapping( key, eh );
	}
	m_HOOPS_to_ACIS.erase( key );
}

HC_KEY HA_EHandleMap::FindMapping( entity_handle* eh )
{	// returns zero if entity not found
	HC_KEY key = 0;
	EXCEPTION_BEGIN
		entity_handle *tl = 0;
	EXCEPTION_TRY
	{
		tl = get_owner_handle( eh );
		T_ehandle_to_HA_TL_EHandleMap_const_iterator eh_to_HA_TL_Map_const_iterator = m_Top_Maps.find( eh );
		T_ehandle_to_HA_TL_EHandleMap_const_iterator eh_to_HA_TL_Map_const_iterator_end = m_Top_Maps.end();
		if ( eh_to_HA_TL_Map_const_iterator == eh_to_HA_TL_Map_const_iterator_end )
			return 0;
		if ( m_Top_Maps.find( tl ) != m_Top_Maps.end() )
			key = m_Top_Maps[tl].FindMapping( eh );
		if ( !key )
			key = m_Top_Maps[eh].FindMapping( eh );
		if ( key && !valid_segment( key ) )
		{	// This should not be hit.  If it does we need to fix something.
			// Please submit bug report.
			sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
			// The key is invalid so remove it from the map
		}
	} EXCEPTION_CATCH_FALSE
	{
		DeleteMapping( key );
		key = 0;
	} EXCEPTION_END;
	return key;
}

entity_handle* HA_EHandleMap::FindMapping( HC_KEY key )
{// returns 0 if key not found
	entity_handle* ent = NULL;
	// First thing is make sure the key is still good
	EXCEPTION_BEGIN;
	EXCEPTION_TRY
	{
		if ( !valid_segment( key ) )
		{	// This should not be hit.  If it does we need to fix something.
			// Please submit bug report.
			sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
		}
		if ( m_HOOPS_to_ACIS.find( key ) != m_HOOPS_to_ACIS.end() )
			ent = m_HOOPS_to_ACIS[key];
		else
			ent = NULL;
	}EXCEPTION_CATCH_FALSE
	{
		DeleteMapping( key );
		ent = NULL;
	}EXCEPTION_END;
	return ent;
}

unsigned long HA_EHandleMap::FindMapping( entity_handle* eh, HC_KEY* keys, unsigned long count )
{	// returns number of keys found (for multiple mappings)
	unsigned long num_keys_mapped_to_entity = 0;
	EXCEPTION_BEGIN;
	entity_handle* tl = 0;
	HC_KEY key = 0;
	EXCEPTION_TRY
	{
		tl = get_owner_handle( eh );
		// Before using the [] operator test if the entity exists in that map.
		T_ehandle_to_HA_TL_EHandleMap_const_iterator tempConstIterator = m_Top_Maps.find( tl );
		if ( tempConstIterator != m_Top_Maps.end() )
		{
			T_ehandle_to_segment_list_const_iterator tempConstIterator2 = m_Top_Maps[tl].m_ACIS_to_HOOPS.find( eh );
			if ( tempConstIterator2 != m_Top_Maps[tl].m_ACIS_to_HOOPS.end() )
			{
				T_segment_list_const_iterator end = m_Top_Maps[tl].m_ACIS_to_HOOPS[eh].end();
				T_segment_list_const_iterator p;
				for ( p = m_Top_Maps[tl].m_ACIS_to_HOOPS[eh].begin(); p != end && num_keys_mapped_to_entity < count; ++p )
				{
					key = *p;
					if ( valid_segment( key ) )
					{
						keys[num_keys_mapped_to_entity] = key;
						num_keys_mapped_to_entity++;
					} else
					{	// This should not be hit.  If it does we need to fix something. Please submit bug report.
						sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
						DeleteMapping( key );
					}
				}
			}
		}
		// Typically an ENTITY that isn't top level won't have any mappings. if you use m_Top_Maps[entity],
		// but sometimes they get absorbed into another entity.  So we need to check this case.
		if ( eh != tl )
		{	// Before using the [] operator test if the entity exists in that map.
			T_ehandle_to_HA_TL_EHandleMap_const_iterator tempConstIterator = m_Top_Maps.find( eh );
			if ( tempConstIterator != m_Top_Maps.end() )
			{	// fang Aug 5 2004. Fixed up top map. Before fixing, only part of the keys for an entity are returned
				// which causes top map can not be cleared
				T_ehandle_to_segment_list_const_iterator tesi;
				int hat_size = (int)tempConstIterator->second.m_ACIS_to_HOOPS.size();
				if ( hat_size > 0 )
				{
					for ( tesi = tempConstIterator->second.m_ACIS_to_HOOPS.begin(); tesi != tempConstIterator->second.m_ACIS_to_HOOPS.end(); tesi++ )
					{
						T_segment_list_const_iterator tsli;
						for ( tsli = tesi->second.begin(); tsli != tesi->second.end(); ++tsli )
						{
							key = *tsli;
							if ( valid_segment( key ) )
							{
								keys[num_keys_mapped_to_entity] = key;
								num_keys_mapped_to_entity++;
							} else // This should not be hit.  If it does we need to fix something. Please submit bug report.
								sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
						}
					}
				}
			}
		}
	} EXCEPTION_CATCH_FALSE
		DeleteMapping( key );
	EXCEPTION_END;
	return num_keys_mapped_to_entity;
}

unsigned long HA_EHandleMap::FindNumMappings( entity_handle* eh )
{	// returns number of keys found (for multiple mappings)
	unsigned long num_keys_mapped_to_entity = 0;
	entity_handle* tl = get_owner_handle( eh );
	// Before using the [] operator test if the entity exists in that map.
	T_ehandle_to_HA_TL_EHandleMap_const_iterator tempConstIterator = m_Top_Maps.find( tl );
	if ( tempConstIterator == m_Top_Maps.end() )
		return 0;
	num_keys_mapped_to_entity = m_Top_Maps[tl].NumSegments( eh );
	// Typically an ENTITY that isn't top level won't have any mappings
	// if you use m_Top_Maps[entity], but sometimes they get absorbed into another
	// entity.  So we need to check this case.
	if ( eh != tl )
		num_keys_mapped_to_entity += m_Top_Maps[eh].NumSegments( eh );
	return num_keys_mapped_to_entity;
}

int HA_EHandleMap::HA_Internal_Get_Map_Entries( void )
{
	int HOOPS_to_Acis_Count = -1;
	int Top_Maps_Count = -1;
	// count the entries in m_HOOPS_to_ACIS map
	HOOPS_to_Acis_Count = (int)m_HOOPS_to_ACIS.size();
	// count the entries in m_Top_Maps map
	Top_Maps_Count = (int)m_Top_Maps.size();
	return HOOPS_to_Acis_Count + Top_Maps_Count;
}

logical HA_EHandleMap::HA_Is_In_Top_Map( entity_handle* ent )
{
	if ( ( m_Top_Maps.find( ent ) ) != m_Top_Maps.end() )
		return TRUE;
	return FALSE;
}

void HA_ModelMap::AddMapping( HC_KEY key, asm_model* model )
{
	m_HOOPS_to_ACIS[key] = model;
}

void HA_ModelMap::DeleteMapping( HC_KEY key )
{
	m_HOOPS_to_ACIS.erase( key );
}

void HA_ModelMap::DeleteMapping( asm_model* model )
{
	unsigned long num = FindNumMappings( model );
	if ( num == 1 )
	{
		HC_KEY key = FindMapping( model );
		m_HOOPS_to_ACIS.erase( key );
	} else if ( num > 1 )
	{
		HC_KEY* keys = ACIS_NEW HC_KEY[num];
		unsigned long new_num = FindMapping( model, keys, num );
		unsigned long i;
		for ( i = 0; i < num; i++ )
		{
			HC_KEY this_key = keys[i];
			m_HOOPS_to_ACIS.erase( this_key );
		}
	}
}

void HA_ModelMap::DeleteMapping( HC_KEY key, asm_model* model )
{
	unsigned long num = FindNumMappings( model );
	if ( num == 1 )
	{
		HC_KEY key = FindMapping( model );
		m_HOOPS_to_ACIS.erase( key );
	} else if ( num > 1 )
	{
		HC_KEY* keys = ACIS_NEW HC_KEY[num];
		unsigned long new_num = FindMapping( model, keys, num );
		int key_found = FALSE;
		unsigned long i;
		for ( i = 0; !key_found && i < num; i++ )
		{
			HC_KEY this_key = keys[i];
			if ( this_key == key )
			{
				m_HOOPS_to_ACIS.erase( this_key );
				key_found = TRUE;
			}
		}
		ACIS_DELETE[] STD_CAST keys;
	}
}

HC_KEY HA_ModelMap::FindMapping( asm_model* model )
{	// returns zero if entity not found
	HC_KEY key = 0;
	FindMapping( model, &key, 1 );
	return key;
}

asm_model* HA_ModelMap::FindMapping( HC_KEY key )
{	// returns 0 if key not found
	asm_model* model = NULL;
	EXCEPTION_BEGIN;
	EXCEPTION_TRY
	{	// First thing is make sure the key is still good
		if ( !valid_segment( key ) ) // This should not be hit.  If it does we need to fix something. Please submit bug report.
		sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
		if ( m_HOOPS_to_ACIS.find( key ) != m_HOOPS_to_ACIS.end() )
			model = m_HOOPS_to_ACIS[key];
		else
			model = NULL;
	} EXCEPTION_CATCH_FALSE
	{
		DeleteMapping( key );
		model = NULL;
	} EXCEPTION_END;
	return model;
}

unsigned long HA_ModelMap::FindMapping( asm_model* model, HC_KEY* keys, unsigned long count )
{	// Passing a NULL model results in all keys being returned
	unsigned long num_keys_mapped_to_model = 0;
	EXCEPTION_BEGIN
		HC_KEY key = 0;
	EXCEPTION_TRY
	{
		STD map<HC_KEY, asm_model*>::iterator p;
		for ( p = m_HOOPS_to_ACIS.begin(); p != m_HOOPS_to_ACIS.end() && num_keys_mapped_to_model < count; ++p )
		{
			if ( p->second == model || model == NULL )
			{
				key = p->first;
				if ( valid_segment( key ) )
				{
					if ( keys )
						keys[num_keys_mapped_to_model] = key;
					num_keys_mapped_to_model++;
				} else // This should not be hit.  If it does we need to fix something. Please submit bug report.
					sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
			}
		}

	} EXCEPTION_CATCH_FALSE
		DeleteMapping( key );
	EXCEPTION_END;
	return num_keys_mapped_to_model;
}

unsigned long HA_ModelMap::FindNumMappings( asm_model* model )
{
	unsigned long num_keys_mapped_to_model = 0;
	EXCEPTION_BEGIN
		HC_KEY key = 0;
	EXCEPTION_TRY
	{
		STD map<HC_KEY, asm_model*>::iterator p;
		for ( p = m_HOOPS_to_ACIS.begin(); p != m_HOOPS_to_ACIS.end(); ++p )
		{
			if ( p->second == model )
			{
				key = p->first;
				if ( valid_segment( key ) )
					num_keys_mapped_to_model++;
				else // This should not be hit.  If it does we need to fix something. Please submit bug report.
					sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
			}
		}
	} EXCEPTION_CATCH_FALSE
		DeleteMapping( key );
	EXCEPTION_END;
	return num_keys_mapped_to_model;
}

void HA_ModelMap::Clear()
{
	m_HOOPS_to_ACIS.clear();
}

void HA_CHandleMap::AddMapping( HC_KEY key, component_handle* comp )
{
	m_HOOPS_to_ACIS[key] = comp;
}

void HA_CHandleMap::DeleteMapping( HC_KEY key )
{
	m_HOOPS_to_ACIS.erase( key );
}

void HA_CHandleMap::DeleteMapping( component_handle* comp )
{
	unsigned long num = FindNumMappings( comp );
	if ( num == 1 )
	{
		HC_KEY key = FindMapping( comp );
		m_HOOPS_to_ACIS.erase( key );
	} else if ( num > 1 )
	{
		HC_KEY* keys = ACIS_NEW HC_KEY[num];
		unsigned long new_num = FindMapping( comp, keys, num );
		unsigned long i;
		for ( i = 0; i < num; i++ )
		{
			HC_KEY this_key = keys[i];
			m_HOOPS_to_ACIS.erase( this_key );
		}
	}
}

void HA_CHandleMap::DeleteMapping( HC_KEY key, component_handle* comp )
{
	unsigned long num = FindNumMappings( comp );
	if ( num == 1 )
	{
		HC_KEY key = FindMapping( comp );
		m_HOOPS_to_ACIS.erase( key );
	} else if ( num > 1 )
	{
		HC_KEY* keys = ACIS_NEW HC_KEY[num];
		unsigned long new_num = FindMapping( comp, keys, num );
		int key_found = FALSE;
		unsigned long i;
		for ( i = 0; !key_found && i < num; i++ )
		{
			HC_KEY this_key = keys[i];
			if ( this_key == key )
			{
				m_HOOPS_to_ACIS.erase( this_key );
				key_found = TRUE;
			}
		}
		ACIS_DELETE[] STD_CAST keys;
	}
}

HC_KEY HA_CHandleMap::FindMapping( component_handle* comp )
{	// returns zero if entity not found
	HC_KEY key = 0;
	FindMapping( comp, &key, 1 );
	return key;
}

component_handle* HA_CHandleMap::FindMapping( HC_KEY key )
{	// returns 0 if key not found
	component_handle* comp = NULL;
	// First thing is make sure the key is still good
	EXCEPTION_BEGIN;
	EXCEPTION_TRY
	{
		if ( !valid_segment( key ) ) // This should not be hit.  If it does we need to fix something. Please submit bug report.
		sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
		if ( m_HOOPS_to_ACIS.find( key ) != m_HOOPS_to_ACIS.end() )
			comp = m_HOOPS_to_ACIS[key];
		else
			comp = NULL;
	} EXCEPTION_CATCH_FALSE
	{
		DeleteMapping( key );
		comp = NULL;
	} EXCEPTION_END;
	return comp;
}

unsigned long HA_CHandleMap::FindMapping( component_handle* comp, HC_KEY* keys, unsigned long count )
{	// Passing a NULL comp results in all keys being returned
	unsigned long num_keys_mapped_to_comp = 0;
	EXCEPTION_BEGIN
		HC_KEY key = 0;
	EXCEPTION_TRY
	{
		STD map<HC_KEY, component_handle*>::iterator p;
		for ( p = m_HOOPS_to_ACIS.begin(); p != m_HOOPS_to_ACIS.end() && num_keys_mapped_to_comp < count; ++p )
		{
			if ( p->second == comp || comp == NULL )
			{
				key = p->first;
				if ( valid_segment( key ) )
				{
					if ( keys )
						keys[num_keys_mapped_to_comp] = key;
					num_keys_mapped_to_comp++;
				} else // This should not be hit.  If it does we need to fix something. Please submit bug report.
					sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
			}
		}

	} EXCEPTION_CATCH_FALSE
		DeleteMapping( key );
	EXCEPTION_END;
	return num_keys_mapped_to_comp;
}

unsigned long HA_CHandleMap::FindNumMappings( component_handle* comp )
{
	unsigned long num_keys_mapped_to_comp = 0;
	EXCEPTION_BEGIN
		HC_KEY key = 0;
	EXCEPTION_TRY
	{
		STD map<HC_KEY, component_handle*>::iterator p;
		for ( p = m_HOOPS_to_ACIS.begin(); p != m_HOOPS_to_ACIS.end(); ++p )
		{
			if ( p->second == comp )
			{
				key = p->first;
				if ( valid_segment( key ) )
					num_keys_mapped_to_comp++;
				else // This should not be hit.  If it does we need to fix something. Please submit bug report.
					sys_error( HA_BRIDGE_SEGMENT_DOES_NOT_EXIST );
			}
		}

	} EXCEPTION_CATCH_FALSE
		DeleteMapping( key );
	EXCEPTION_END;
	return num_keys_mapped_to_comp;
}

void HA_CHandleMap::Clear()
{
	m_HOOPS_to_ACIS.clear();
}
