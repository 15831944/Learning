/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_Map_H_
#define __HPS_Map_H_
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.

#include "dcl_hps.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#include <map>
#pragma warning(pop)

//#define HPS_USE_STL
#ifdef HPS_USE_STL
#include <list>
#endif // HPS_USE_STL

#include "logical.h"

#include <assert.h>

#ifdef NT
#include <windows.h>
#endif // NT

#include "kernapi.hxx"
#include "hps_util.h"

// This will set some #defines, such as RWSTD_NO_NAMESPACE, used below
#ifdef hp700
#include <stdcomp.h>
#endif

#ifdef RWSTD_NO_NAMESPACE
#  define STD
#else
#  define STD std::
#endif

#if defined(aix)
#define HPS_TYPENAME
#else
#define HPS_TYPENAME typename
#endif

typedef STD map<ENTITY*, HPS::KeyArray> T_entity_to_HPS_KeyArray;
typedef T_entity_to_HPS_KeyArray::const_iterator T_entity_to_HPS_KeyArray_const_iterator;
typedef T_entity_to_HPS_KeyArray::iterator T_entity_to_HPS_KeyArray_iterator;

// TL (Top Level)
class HPS_TL_Map : public ACIS_OBJECT
{
	HPS::KeyArray key_array;
public:
	T_entity_to_HPS_KeyArray m_ACIS_to_HOOPS;

	~HPS_TL_Map()
	{
		Clear();
	}
	void Clear();
	void DeleteMapping( ENTITY* in_entity );
	void AddMapping( HPS::Key in_key, ENTITY* in_entity );
	void DeleteMapping( HPS::Key key, ENTITY* in_entity );
	HPS::Key FindMapping( ENTITY* in_entity );
	int NumSegments( ENTITY* in_entity );
};

typedef STD map <ENTITY*, HPS_TL_Map> T_entity_to_HPS_TL_Map;
typedef T_entity_to_HPS_TL_Map::const_iterator T_entity_to_HPS_TL_Map_const_iterator;
typedef T_entity_to_HPS_TL_Map::iterator T_entity_to_HPS_TL_Map_iterator;

class DECL_HPS HPS_Map : public ACIS_OBJECT
{
public:
	~HPS_Map()
	{
		m_HPS_to_ACIS.clear();
		m_Top_Maps.clear();
	}

	void AddMapping( HPS::Key in_key, ENTITY* in_entity, ENTITY* in_owner = NULL );

	// delete functions return FALSE if item to be deleted could not be found.
	void DeleteMapping( HPS::Key in_key );
	void DeleteMapping( ENTITY* in_entity );
	void DeleteMapping( HPS::Key in_key, ENTITY* in_entity );

	// returns zero if entity not found
	HPS::Key FindMapping( ENTITY* in_entity );

	// returns 0 if key not found
	ENTITY* FindMapping( HPS::Key in_key );

	// returns number of keys found (for multiple mappings)
	unsigned long FindMapping( ENTITY* in_entity, HPS::KeyArray& out_key_array );

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings( ENTITY* in_entity );

	void DebugMaps( const char * in_title = "" );
	int HPS_Internal_Get_Map_Entries( void );
	logical HPS_Is_In_Top_Map( ENTITY* in_entity );

private:
	STD map<HPS::Key, ENTITY*> m_HPS_to_ACIS;
	STD map <ENTITY*, HPS_TL_Map> m_Top_Maps;
};
#endif

