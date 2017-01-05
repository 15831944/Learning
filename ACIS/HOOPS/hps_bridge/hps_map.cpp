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

#include <iomanip>
#include <sstream>

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
// Added for hpia_64-
#ifdef hp700
#include <new.h>
#endif

#include "hps_map.h"
#include "hps_util.h"
#include "api.err"
#include "hps_bridge.err"

#include "asm_assembly.hxx"
#include "asm_model_ref.hxx"
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
#include "facet_body.hxx"

template< typename T >
std::string hexify( T i )
{
	std::stringbuf buf;
	std::ostream os( &buf );
	os << "0x" << std::setfill( '0' ) << std::setw( sizeof( T ) * 2 )
		<< std::hex << i;
	return buf.str().c_str();
}

logical Is_In_Top_Map( ENTITY* ent );

void HPS_Map::AddMapping( HPS::Key in_key, ENTITY* in_entity, ENTITY* in_owner )
{	// fang Aug 5 2004
	// entity should be registered in Hoops Map only if the entity is a Body, Face, Edge, Vertex, Apoint, WCS, 
	// Text_ent or Light
	if ( is_ASM_ASSEMBLY( in_entity ) ||
		 is_ASM_MODEL_REF( in_entity ) ||
		 is_BODY( in_entity ) ||
		 is_FACE( in_entity ) ||
		 is_EDGE( in_entity ) ||
		 is_VERTEX( in_entity ) ||
		 is_APOINT( in_entity ) ||
		 is_WCS( in_entity ) ||
		 is_TEXT_ENT( in_entity ) ||
		 IS_LIGHT( in_entity ) ||
		 is_FACET_BODY( in_entity ) )
	{
		ENTITY *top_level = 0;
		if ( !in_owner )
			api_get_owner( in_entity, top_level );
		else
			top_level = in_owner;
		if ( is_EDGE( in_entity ) )
		{	// mgp (11/01/04): Is this a silhouette edge?
			ATTRIB_GEN_NAME *attrib = NULL;
			api_find_named_attribute( in_entity, "HPS_SilhouetteFace", attrib );
			if ( attrib && attrib->identity( ATTRIB_GEN_ENTITY_LEVEL ) == ATTRIB_GEN_ENTITY_TYPE )
			{
				ATTRIB_GEN_ENTITY * att_ent = (ATTRIB_GEN_ENTITY*)attrib;
				ENTITY * face = att_ent->value();
				if ( face && is_FACE( face ) )
				{
					AddMapping( in_key, face, in_owner );
					return;
				}
			}
		}
		m_Top_Maps[top_level].AddMapping( in_key, in_entity );
		m_HPS_to_ACIS[in_key] = in_entity;
		DBGPRINTF( "%4d %s(0x%p[%d]) keyID=0x%p, top_level=0x%p[%d]\n", __LINE__, __FUNCTION__,
				   in_entity, in_entity->tag(), in_key.GetInstanceID(), top_level, top_level ? top_level->tag() : -1 );
	}
	//	if ( HPS_Is_Valid_Segment_Key( HPS_Cast_SegmentKey( in_key ) ))
	//		printf( "AddMapping: in_key=\"%s\"\n", HPS_Show_Segment( HPS_Cast_SegmentKey( in_key ) ).c_str( ) );
	//DebugMaps( "AddMapping" );
}

void HPS_Map::DeleteMapping( HPS::Key in_key )
{
	DBGPRINTF( "%4d %s(keyID=0x%p)\n", __LINE__, __FUNCTION__, in_key.GetInstanceID() );
	m_HPS_to_ACIS.erase( in_key );
	//DebugMaps( "DeleteMapping" );
}

void HPS_Map::DeleteMapping( ENTITY* in_entity )
{
	ENTITY *top_level = 0;
	// knt 3 Aug 2006. BTS #82057. Added the check here to see if the
	// given entity is in the top map. The bug was that an EDGE was lost
	// and found itself in this routine. We called api_get_owner on the
	// given EDGE and it's owning COEDGE, too, was already lost. Trying
	// to ask for the COEDGE's owner caused a crash. My thinking behind the
	// design of my fix is as follows: originally, the EDGE in question was
	// a top-level entity - so it was put in the Top-Map. Now the EDGE
	// in question is no longer top-level, but we need to clear it from 
	// the Top-Map. So the point is this: if we know it is in the Top-Map,
	// then we don't need to ask for it's owner. From the perspective
	// of this routine, we just need to make sure we clean things up -
	// so we treat input entities that are in the Top-Map as if they are
	// top-level entities (i.e., set their top_level owner to themselves).
	// I had to put this logic in a handful of places related to the delete
	// logic.
	if ( Is_In_Top_Map( in_entity ) )
		top_level = in_entity;
	else
		api_get_owner( in_entity, top_level );
	if ( in_entity == top_level )
	{
		// Before using the [] operator test if the entity exists in that map.
		T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( top_level );
		if ( tempConstIterator == m_Top_Maps.end() )
			return;
		T_entity_to_HPS_KeyArray_iterator entity_to_segment_list_end = m_Top_Maps[top_level].m_ACIS_to_HOOPS.end();
		for ( T_entity_to_HPS_KeyArray_iterator l = m_Top_Maps[top_level].m_ACIS_to_HOOPS.begin();
			  l != entity_to_segment_list_end;
			  ++l )
		{
			ENTITY * map_ent = (ENTITY*)( *l ).first;
			HPS::KeyArray key_array = m_Top_Maps[top_level].m_ACIS_to_HOOPS[map_ent];
			for ( auto key_ptr = key_array.begin(); key_ptr != key_array.end(); ++key_ptr )
			{
				HPS::Key key = *key_ptr;
				if ( HPS_Is_Valid_Key( key ) )
				{
					DBGPRINTF( "%4d %s(0x%p[%d]) keyID=0x%p\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag(), key.GetInstanceID() );
					m_HPS_to_ACIS.erase( key );
				}
			}
		}
		DBGPRINTF( "%4d %s(0x%p[%d])\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag() );
		m_Top_Maps.erase( in_entity );
	} else
	{
#if 1
		// fang Aug 5 2004. Fixed up top map. Before fixing, top map can't be cleared if the entity is absorbed into 
		// another entity
		// if the entity is still in top level map, erase it
		if ( ( m_Top_Maps.find( in_entity ) ) != m_Top_Maps.end() )
		{
			DBGPRINTF( "%4d %s(0x%p[%d])\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag() );
			m_Top_Maps.erase( in_entity );
		} else
		{
			T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( top_level );
			if ( tempConstIterator != m_Top_Maps.end() )
			{
				DBGPRINTF( "%4d %s(0x%p[%d])\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag() );
				m_Top_Maps[top_level].DeleteMapping( in_entity );
			}
		}
#else
		T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( top_level );
		if ( tempConstIterator == m_Top_Maps.end() )
			return;
		m_Top_Maps[top_level].DeleteMapping( in_entity );
#endif
	}
	std::stringstream my_string;
	my_string << "DeleteMapping: entity.tag=" << in_entity->tag();
	//	DebugMaps( my_string.str().c_str() );
}

void HPS_Map::DeleteMapping( HPS::Key key, ENTITY* in_entity )
{
	ENTITY *top_level = 0;
	api_get_owner( in_entity, top_level );
	if ( in_entity == top_level )
	{
		DBGPRINTF( "%4d %s(0x%p[%d])\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag() );
		m_Top_Maps.erase( in_entity );
	} else
	{
		// Before using the [] operator test if the entity exists in that map.
		T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( top_level );
		if ( tempConstIterator == m_Top_Maps.end() )
			return;
		DBGPRINTF( "%4d %s(0x%p[%d]) keyID=0x%p, top_level=0x%p[%d]\n", __LINE__, __FUNCTION__,
				   in_entity, in_entity->tag(), key.GetInstanceID(), top_level, top_level ? top_level->tag() : -1 );
		m_Top_Maps[top_level].DeleteMapping( key, in_entity );
	}
	DBGPRINTF( "%4d %s(0x%p[%d]) keyID=0x%p\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag(), key.GetInstanceID() );
	m_HPS_to_ACIS.erase( key );
	// size_t key_ID = key.GetInstanceID();
	// std::stringstream my_string;
	// my_string << "DeleteMapping: key=" << hexify<size_t>( key_ID ) << ", entity.tag=" << in_entity->tag();
	//	DebugMaps( my_string.str().c_str() );
	}

HPS::Key HPS_Map::FindMapping( ENTITY* in_entity )
{	// returns zero if entity not found
	HPS::Key key = HPS_INVALID_KEY;
	EXCEPTION_BEGIN
		ENTITY *top_level = 0;
	EXCEPTION_TRY;
	{
		// knt 3 Aug 2006. BTS #82057. Added the check here to see if the
		// given entity is in the top map. The bug was that an EDGE was lost
		// and found itself in this routine. We called api_get_owner on the
		// given EDGE and it's owning COEDGE, too, was already lost. Trying
		// to ask for the COEDGE's owner caused a crash. My thinking behind the
		// design of my fix is as follows: originally, the EDGE in question was
		// a top-level entity - so it was put in the Top-Map. Now the EDGE
		// in question is no longer top-level, but we need to clear it from 
		// the Top-Map. So the point is this: if we know it is in the Top-Map,
		// then we don't need to ask for it's owner. From the perspective
		// of this routine, we just need to make sure we clean things up -
		// so we treat input entities that are in the Top-Map as if they are
		// top-level entities (i.e., set their top_level owner to themselves).
		// I had to put this logic in a handful of places related to the delete
		// logic.
		if ( Is_In_Top_Map( in_entity ) )
			top_level = in_entity;
		else
			api_get_owner( in_entity, top_level );
		T_entity_to_HPS_TL_Map_const_iterator entity_to_HPS_TL_Map_const_iterator = m_Top_Maps.find( in_entity );
		T_entity_to_HPS_TL_Map_const_iterator entity_to_HPS_TL_Map_const_iterator_end = m_Top_Maps.end();
		if ( entity_to_HPS_TL_Map_const_iterator == entity_to_HPS_TL_Map_const_iterator_end )
			return HPS_INVALID_KEY;
		if ( m_Top_Maps.find( top_level ) != m_Top_Maps.end() )
			key = m_Top_Maps[top_level].FindMapping( in_entity );
		if ( !HPS_Is_Valid_Key( key ) )
			key = m_Top_Maps[in_entity].FindMapping( in_entity );
		if ( HPS_Is_Valid_Key( key ) && !HPS_Is_Valid_Segment_Key( HPS_Cast_SegmentKey( key ) ) )
		{
			// This should not be hit.  If it does we need to fix something.
			// Please submit bug report.
			sys_error( HPS_MSG_SEGMENT_DOES_NOT_EXIST );
			// The key is invalid so remove it from the map
		}
	} EXCEPTION_CATCH_FALSE;
	{
		DeleteMapping( key );
		key = HPS_INVALID_KEY;
	} EXCEPTION_END;
	return key;
}

// returns 0 if key not found
ENTITY* HPS_Map::FindMapping( HPS::Key in_key )
{
	ENTITY* ent = NULL;
	// First thing is make sure the in_key is still good
	EXCEPTION_BEGIN;
	EXCEPTION_TRY;
	{
		if ( !HPS_Is_Valid_Key( in_key ) )
		{
			// This should not be hit.  If it does we need to fix something.
			// Please submit bug report.
			sys_error( HPS_MSG_SEGMENT_DOES_NOT_EXIST );
		}
		if ( m_HPS_to_ACIS.find( in_key ) != m_HPS_to_ACIS.end() )
			ent = m_HPS_to_ACIS[in_key];
	} EXCEPTION_CATCH_FALSE;
	{
		DeleteMapping( in_key );
		ent = NULL;
	} EXCEPTION_END;
	return ent;
}

// returns number of keys found (for multiple mappings)
unsigned long HPS_Map::FindMapping( ENTITY* in_entity, HPS::KeyArray& out_key_array )
{
	unsigned long num_keys_mapped_to_entity = 0;
	EXCEPTION_BEGIN;
	ENTITY *top_level = 0;
	HPS::Key key = HPS_INVALID_KEY;
	EXCEPTION_TRY;
	{
		if ( Is_In_Top_Map( in_entity ) )
			top_level = in_entity;
		else
			api_get_owner( in_entity, top_level );
		// Before using the [] operator test if the entity exists in that map.
		T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( top_level );
		if ( tempConstIterator != m_Top_Maps.end() )
		{
			T_entity_to_HPS_KeyArray_const_iterator tempConstIterator2 = m_Top_Maps[top_level].m_ACIS_to_HOOPS.find( in_entity );
			if ( tempConstIterator2 != m_Top_Maps[top_level].m_ACIS_to_HOOPS.end() )
			{
				HPS::KeyArray key_array = m_Top_Maps[top_level].m_ACIS_to_HOOPS[in_entity];
				for ( auto key_ptr = key_array.begin(); key_ptr != key_array.end(); ++key_ptr )
				{
					key = *key_ptr;
					// Check to see if 
					if ( HPS_Is_Valid_Key( key ) )
					{
						out_key_array.push_back( key );
						num_keys_mapped_to_entity++;
						//size_t key_ID1 = key.GetInstanceID();
						//size_t key_ID2 = num_keys_mapped_to_entity ? keys[0].GetInstanceID() : 0;
					} else
					{
						// This should not be hit.  If it does we need to fix something. Please submit bug report.
						sys_warning( HPS_MSG_SEGMENT_DOES_NOT_EXIST );
						//DeleteMapping( key );
					}
				}
			}
		}
		// Typically an ENTITY that isn't top level won't have any mappings if you use m_Top_Maps[entity],
		// but sometimes they get absorbed into another entity.  So we need to check this case.
		if ( in_entity != top_level )
		{
			// Before using the [] operator test if the entity exists in that map.
			T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( in_entity );
			if ( tempConstIterator != m_Top_Maps.end() )
			{
				T_entity_to_HPS_KeyArray ent_key_map = tempConstIterator->second.m_ACIS_to_HOOPS;
				T_entity_to_HPS_KeyArray_const_iterator tesi;
				size_t hat_size = ent_key_map.size();
				if ( hat_size > 0 )
				{
					for ( tesi = ent_key_map.begin(); tesi != ent_key_map.end(); tesi++ )
					{
						HPS::KeyArray keys = ( *tesi ).second;
						for ( auto key_ptr = keys.begin(); key_ptr != keys.end(); ++key_ptr )
						{
							key = *key_ptr;
							// Check to see if 
							if ( HPS_Is_Valid_Key( key ) )
							{
								out_key_array[num_keys_mapped_to_entity] = key;
								num_keys_mapped_to_entity++;
							} else
							{
								// This should not be hit.  If it does we need to fix something. Please submit bug report.
								sys_error( HPS_MSG_SEGMENT_DOES_NOT_EXIST );

							}
						}
					}
				}
			}
		}
	} EXCEPTION_CATCH_FALSE
		DeleteMapping( key );
	EXCEPTION_END;
	size_t key_ID = num_keys_mapped_to_entity ? out_key_array[0].GetInstanceID() : 0;
	std::stringstream my_string;
	my_string << "FindMapping: key[0]=" << hexify<size_t>( key_ID ) << ", in_entity.tag=" << in_entity->tag() << ", num_keys=" << num_keys_mapped_to_entity;
	//	DebugMaps( my_string.str().c_str() );
	return num_keys_mapped_to_entity;
}

// returns number of keys found (for multiple mappings)
unsigned long HPS_Map::FindNumMappings( ENTITY* in_entity )
{
	unsigned long num_keys_mapped_to_entity = 0;
	ENTITY *top_level = 0;
	api_get_owner( in_entity, top_level );
	// Before using the [] operator test if the entity exists in that map.
	T_entity_to_HPS_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find( top_level );
	if ( tempConstIterator == m_Top_Maps.end() )
		return 0;
	num_keys_mapped_to_entity = m_Top_Maps[top_level].NumSegments( in_entity );
	// Typically an ENTITY that isn't top level won't have any mappings
	// if you use m_Top_Maps[entity], but sometimes they get absorbed into another
	// entity.  So we need to check this case.
	if ( in_entity != top_level )
		num_keys_mapped_to_entity += m_Top_Maps[in_entity].NumSegments( in_entity );
	return num_keys_mapped_to_entity;
}

void HPS_Map::DebugMaps( const char * in_title )
{
	//	return;	// Comment out for debugging the maps.
	FILE *fp;
	static char file_mode[3] = "w";
	fp = fopen( "hoopsmap_HPS.txt", file_mode );
	file_mode[0] = 'a';
	printf( "%s\n", in_title );
	fprintf( fp, "%s\n", in_title );
	//	return;	// Comment out for intense debugging.  Gets rather verbose.
	printf( "m_HPS_to_ACIS map:\n" );
	fprintf( fp, "m_HPS_to_ACIS map:\n" );
	STD map<HPS::Key, ENTITY*>::iterator topi;
	for ( topi = m_HPS_to_ACIS.begin(); topi != m_HPS_to_ACIS.end(); topi++ )
	{
		const char* enttype = topi->second->type_name();
		if ( HPS_Is_Valid_Key( topi->first ) )
		{
			printf( "key_ID=0x%p\t%8s.tag=%3d\n", topi->first.GetInstanceID(), enttype, topi->second->tag() );
			fprintf( fp, "key_ID=0x%p\t%8s.tag=%3d\n", topi->first.GetInstanceID(), enttype, topi->second->tag() );
		} else
		{
			printf( "key_ID=Invalid Key\t%8s.tag=%3d\n", enttype, topi->second->tag() );
			fprintf( fp, "key_ID=Invalid Key\t%8s.tag=%3d\n", enttype, topi->second->tag() );
		}
		//printf(     "key =0x%p\tENITTY=0x%p\n", topi->first, topi->second);
		//fprintf(fp, "key =0x%p\tENITTY=0x%p\n", topi->first, topi->second);

		logical found = FALSE;
		T_entity_to_HPS_TL_Map_const_iterator etli;
		for ( etli = m_Top_Maps.begin(); etli != m_Top_Maps.end() && !found; etli++ )
		{
			T_entity_to_HPS_KeyArray_const_iterator tesi;
			int hat_size = (int)etli->second.m_ACIS_to_HOOPS.size();
			if ( hat_size > 0 )
			{
				for ( tesi = etli->second.m_ACIS_to_HOOPS.begin(); tesi != etli->second.m_ACIS_to_HOOPS.end() && !found; tesi++ )
				{
					for ( auto key_ptr = tesi->second.begin(); key_ptr != tesi->second.end() && !found; ++key_ptr )
						found = ( topi->first == *key_ptr );
				}
			}
		}
		//if (!found)
		//	sys_error(HPS_MSG_SEGMENT_DOES_NOT_EXIST);
	}
	//printf("m_Top_Maps map:\n");
	fprintf( fp, "m_Top_Maps map:\n" );
	T_entity_to_HPS_TL_Map_const_iterator etli;
	for ( etli = m_Top_Maps.begin(); etli != m_Top_Maps.end(); etli++ )
	{
		const char* enttype = etli->first->type_name();
		printf( "%8s.tag=%3d\n", enttype, etli->first->tag() );
		fprintf( fp, "%8s.tag=%3d\n", enttype, etli->first->tag() );
		T_entity_to_HPS_KeyArray_const_iterator tesi;
		int hat_size = (int)etli->second.m_ACIS_to_HOOPS.size();
		if ( hat_size > 0 )
		{
			for ( tesi = etli->second.m_ACIS_to_HOOPS.begin(); tesi != etli->second.m_ACIS_to_HOOPS.end(); tesi++ )
			{
				const char* enttype1 = ( (ENTITY*)tesi->first )->type_name();
				printf( "\t%8s.tag=%3d\n", enttype1, tesi->first->tag() );
				fprintf( fp, "\t%8s.tag=%3d\n", enttype1, tesi->first->tag() );
				for ( auto key_ptr = tesi->second.begin(); key_ptr != tesi->second.end(); ++key_ptr )
				{
					if ( HPS_Is_Valid_Key( *key_ptr ) )
					{
						printf( "\t\tkey_ID=0x%p\n", ( *key_ptr ).GetInstanceID() );
						fprintf( fp, "\t\tkey_ID=0x%p\n", ( *key_ptr ).GetInstanceID() );
						logical found = FALSE;
						STD map<HPS::Key, ENTITY*>::iterator topi;
						for ( topi = m_HPS_to_ACIS.begin(); topi != m_HPS_to_ACIS.end() && !found; topi++ )
							found = ( topi->first == *key_ptr );
						//if (!found)
						//	sys_error(HPS_MSG_SEGMENT_DOES_NOT_EXIST);
					}
				}
			}
		}
	}
	printf( "\n" );
	fprintf( fp, "\n" );
	fclose( fp );
}

int HPS_Map::HPS_Internal_Get_Map_Entries( void )
{
	int HPS_to_Acis_Count = -1;
	int Top_Maps_Count = -1;
	// count the entries in m_HPS_to_ACIS map
	HPS_to_Acis_Count = (int)m_HPS_to_ACIS.size();
	// count the entries in m_Top_Maps map
	Top_Maps_Count = (int)m_Top_Maps.size();

	return HPS_to_Acis_Count + Top_Maps_Count;
}

logical HPS_Map::HPS_Is_In_Top_Map( ENTITY* in_entity )
{
	if ( ( m_Top_Maps.find( in_entity ) ) != m_Top_Maps.end() )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void HPS_TL_Map::DeleteMapping( ENTITY* entity )
{
	DBGPRINTF( "%4d %s(0x%p[%d])\n", __LINE__, __FUNCTION__, entity, entity->tag() );
	m_ACIS_to_HOOPS.erase( entity );
} // Does this ever happen?

void HPS_TL_Map::AddMapping( HPS::Key key, ENTITY* entity )
{
	DBGPRINTF( "%4d %s(0x%p[%d]) keyID=0x%p\n", __LINE__, __FUNCTION__, entity, entity->tag(), key.GetInstanceID() );
	key_array.push_back( key );
	m_ACIS_to_HOOPS[entity].push_back( key_array.back() );
}

void HPS_TL_Map::DeleteMapping( HPS::Key in_key, ENTITY* in_entity )
{
	//		SPAUNUSED(key)
	//		DeleteMapping(entity);
	DBGPRINTF( "%4d %s(0x%p[%d])\n", __LINE__, __FUNCTION__ );
	HPS::KeyArray key_array = m_ACIS_to_HOOPS[in_entity];
	for ( auto key_ptr = key_array.begin(); key_ptr != key_array.end(); ++key_ptr )
	{
		HPS::Key key = *key_ptr;
		if ( key.GetInstanceID() == in_key.GetInstanceID() )
		{
			DBGPRINTF( "%4d %s(0x%p[%d]) keyID=0x%p\n", __LINE__, __FUNCTION__, in_entity, in_entity->tag(), in_key.GetInstanceID() );
			m_ACIS_to_HOOPS[in_entity].erase( key_ptr );
		}
	}
}

HPS::Key HPS_TL_Map::FindMapping( ENTITY* in_entity )
{
	if ( m_ACIS_to_HOOPS.size() > 0 )
	{
		size_t size = m_ACIS_to_HOOPS[in_entity].size();
		if ( m_ACIS_to_HOOPS[in_entity].size() > 0 )
			return m_ACIS_to_HOOPS[in_entity].front();
	}
	for ( auto key_ptr = key_array.begin(), e = key_array.end(); key_ptr != e; ++key_ptr )
	{
		HPS::Key key = ( *key_ptr );
		return key;
	}
	return HPS_INVALID_KEY;
}

int HPS_TL_Map::NumSegments( ENTITY* in_entity )
{
	return (int)m_ACIS_to_HOOPS[in_entity].size();
}

void HPS_TL_Map::Clear()
{
	DBGPRINTF( "CLEARING m_ACIS_to_HOOPS\n", __LINE__, __FUNCTION__ );
	m_ACIS_to_HOOPS.clear();
}
