/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HA_MAP_ASM_H_
#define __HA_MAP_ASM_H_
#include "hc.h"

#include "dcl_hoops.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#include <map>
#pragma warning(pop)

//#define HA_USE_STL
#ifdef HA_USE_STL
#include <list>
#endif // HA_USE_STL

#include "logical.h"

#include <assert.h>

#ifdef NT
#include <windows.h>
#endif // NT

#include "kernapi.hxx"

class asm_model;

// This will set some #defines, such as RWSTD_NO_NAMESPACE, used below
#ifdef hp700
#include <stdcomp.h>
#endif

#ifdef RWSTD_NO_NAMESPACE
#  define STD
#else
#  define STD std::
#endif

#include "ha_map.h"
class entity_handle;
class component_handle;

typedef STD map<entity_handle*,T_segment_list> T_ehandle_to_segment_list;
typedef T_ehandle_to_segment_list::const_iterator T_ehandle_to_segment_list_const_iterator;
typedef T_ehandle_to_segment_list::iterator T_ehandle_to_segment_list_iterator;

// TL (Top Level)
class HA_TL_EHandleMap: public ACIS_OBJECT
{
public:
	T_ehandle_to_segment_list m_ACIS_to_HOOPS;

	~HA_TL_EHandleMap()
	{
		Clear();
	}

	void Clear() 
	{
		m_ACIS_to_HOOPS.clear();
	}
	
	void DeleteMapping(entity_handle* entity)
	{
		m_ACIS_to_HOOPS.erase(entity);
	} // Does this ever happen?
	
	void AddMapping(HC_KEY key, entity_handle* entity) 
	{
		m_ACIS_to_HOOPS[entity].push_back(key);
	}
	
	void DeleteMapping(HC_KEY key, entity_handle* entity) 
	{
//		SPAUNUSED(key)
//		DeleteMapping(entity);
		m_ACIS_to_HOOPS[entity].remove(key);
	}
	
	HC_KEY FindMapping(entity_handle* entity)
	{
		if (m_ACIS_to_HOOPS.size()>0)
		{
			if (m_ACIS_to_HOOPS[entity].size()>0)
			{
				return m_ACIS_to_HOOPS[entity].front();
			}
		}
		return 0;
	}
	
	int NumSegments(entity_handle* entity)
	{
		return m_ACIS_to_HOOPS[entity].size();
	}
};

typedef STD map <entity_handle*,HA_TL_EHandleMap> T_ehandle_to_HA_TL_EHandleMap;
typedef T_ehandle_to_HA_TL_EHandleMap::const_iterator T_ehandle_to_HA_TL_EHandleMap_const_iterator;
typedef T_ehandle_to_HA_TL_EHandleMap::iterator T_ehandle_to_HA_TL_EHandleMap_iterator;

class DECL_HOOPS HA_EHandleMap: public ACIS_OBJECT
{
public:
	~HA_EHandleMap()
	{
		m_HOOPS_to_ACIS.clear();
		m_Top_Maps.clear();
	}

	void AddMapping(HC_KEY key, entity_handle* entity, entity_handle* owner=NULL);

	// delete functions return FALSE if item to be deleted could not be found.
	void DeleteMapping(HC_KEY key);
	void DeleteMapping(entity_handle* entity);
	void DeleteMapping(HC_KEY key, entity_handle* entity);

	// returns zero if entity not found
	HC_KEY FindMapping(entity_handle* entity);

	// returns 0 if key not found
	entity_handle* FindMapping(HC_KEY key);

	// returns number of keys found (for multiple mappings)
	unsigned long FindMapping(entity_handle* entity, HC_KEY* keys, unsigned long count);

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings(entity_handle* entity);

//	void DebugMaps(void);
	int HA_Internal_Get_Map_Entries(void);
	logical HA_Is_In_Top_Map( entity_handle* entity );

private:
	STD map<HC_KEY,entity_handle*> m_HOOPS_to_ACIS;
	STD map <entity_handle*,HA_TL_EHandleMap> m_Top_Maps;
};

class DECL_HOOPS HA_ModelMap : public ACIS_OBJECT
{
public:
	~HA_ModelMap()
	{
		m_HOOPS_to_ACIS.clear();
	}

	void AddMapping(HC_KEY key, asm_model* model);

	void DeleteMapping(HC_KEY key);
	void DeleteMapping(asm_model* model);
	void DeleteMapping(HC_KEY key, asm_model* model);

    void Clear();

	// returns zero if entity not found
	HC_KEY FindMapping(asm_model* model);

	// returns 0 if key not found
	asm_model* FindMapping(HC_KEY key);

	// returns number of keys found (for multiple mappings)
	// Passing a NULL model results in all keys being returned
	unsigned long FindMapping(asm_model* model, HC_KEY* keys, unsigned long count);

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings(asm_model* model);

private:
	STD map<HC_KEY, asm_model*> m_HOOPS_to_ACIS;
};

class DECL_HOOPS HA_CHandleMap : public ACIS_OBJECT
{
public:
	~HA_CHandleMap()
	{
		m_HOOPS_to_ACIS.clear();
	}

	void AddMapping(HC_KEY key, component_handle* comp);

	void DeleteMapping(HC_KEY key);
	void DeleteMapping(component_handle* comp);
	void DeleteMapping(HC_KEY key, component_handle* comp);

    void Clear();

	// returns zero if entity not found
	HC_KEY FindMapping(component_handle* comp);

	// returns 0 if key not found
	component_handle* FindMapping(HC_KEY key);

	// returns number of keys found (for multiple mappings)
	// Passing a NULL model results in all keys being returned
	unsigned long FindMapping(component_handle* comp, HC_KEY* keys, unsigned long count);

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings(component_handle* comp);

private:
	STD map<HC_KEY, component_handle*> m_HOOPS_to_ACIS;
};

#endif // __HA_MAP_ASM_H_

