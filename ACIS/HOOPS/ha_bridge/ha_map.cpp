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
#include "ha_util.h"
#include "ha_bridge.err"

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

logical Is_In_Top_Map( ENTITY* ent );

void HA_Map::AddMapping(HC_KEY key, ENTITY* entity, ENTITY* owner)
{

	// fang Aug 5 2004
	// entity should be registered in Hoops Map only if the entity is a Body, Face, Edge, Vertex, Apoint, WCS, 
	// Text_ent or Light
	if( is_ASM_ASSEMBLY(entity) ||
		is_ASM_MODEL_REF(entity) || 
		is_BODY(entity) || 
		is_FACE(entity) || 
		is_EDGE(entity) || 
		is_VERTEX(entity) || 
		is_APOINT(entity) ||
		is_WCS(entity) ||
		is_TEXT_ENT(entity) ||
		IS_LIGHT(entity) ||
		is_FACET_BODY(entity))
	{
		ENTITY *top_level=0;
		if( !owner )
		{
			api_get_owner(entity, top_level);
		}
		else
			top_level = owner;

		if ( is_EDGE(entity) )
		{
			// mgp (11/01/04): Is this a silhouette edge?
			ATTRIB_GEN_NAME *attrib = NULL;
			api_find_named_attribute(entity, "HA_SilhouetteFace", attrib);

			if (attrib && attrib->identity(ATTRIB_GEN_ENTITY_LEVEL) == ATTRIB_GEN_ENTITY_TYPE)
			{
				ATTRIB_GEN_ENTITY *att_ent = (ATTRIB_GEN_ENTITY*)attrib;
				ENTITY            *face    = att_ent->value();

				if ( face && is_FACE(face) )
				{
					AddMapping(key, face, owner);
					return;
				}
			}
		}

		m_Top_Maps[top_level].AddMapping(key,entity);
		m_HOOPS_to_ACIS[key]=entity;
	}

//	DebugMaps();
}

void HA_Map::DeleteMapping(HC_KEY key) 
{
	m_HOOPS_to_ACIS.erase(key);
//	DebugMaps();
}

void HA_Map::DeleteMapping(ENTITY* entity)
{
	ENTITY *top_level=0;

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

	if ( Is_In_Top_Map( entity ) )
		top_level = entity;
	else
		api_get_owner(entity, top_level);

	if(entity==top_level)
	{
		// Before using the [] operator test if the entity exists in that map.
		T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(top_level);
		if(tempConstIterator == m_Top_Maps.end())
			return ;

		T_entity_to_segment_list_iterator entity_to_segment_list_end = m_Top_Maps[top_level].m_ACIS_to_HOOPS.end();
		for (T_entity_to_segment_list_iterator l=m_Top_Maps[top_level].m_ACIS_to_HOOPS.begin();
			l!=entity_to_segment_list_end;
			++l)
		{
			ENTITY *map_ent=(ENTITY*)(*l).first;

			T_segment_list_const_iterator end = m_Top_Maps[top_level].m_ACIS_to_HOOPS[map_ent].end();
			for (T_segment_list_const_iterator 
				p=m_Top_Maps[top_level].m_ACIS_to_HOOPS[map_ent].begin();
				p!=end;
				++p)
			{
				HC_KEY key = *p;
				m_HOOPS_to_ACIS.erase(key);
			}
		}

		m_Top_Maps.erase(entity);
	}
	else
	{
#if 1
		// fang Aug 5 2004. Fixed up top map. Before fixing, top map can't be cleared if the entity is absorbed into 
		// another entity
		// if the entity is still in top level map, erase it

		if( (m_Top_Maps.find( entity )) != m_Top_Maps.end() )
			m_Top_Maps.erase(entity);
		else
		{
			T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(top_level);
			if(tempConstIterator != m_Top_Maps.end())
				m_Top_Maps[top_level].DeleteMapping(entity);
		}
#else
		T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(top_level);
		if(tempConstIterator == m_Top_Maps.end())
			return ;
		m_Top_Maps[top_level].DeleteMapping(entity);
#endif
	}
//	DebugMaps();
}

void HA_Map::DeleteMapping(HC_KEY key, ENTITY* entity)
{
	ENTITY *top_level=0;
	api_get_owner(entity, top_level);

	if(entity==top_level)
		m_Top_Maps.erase(entity);
	else
	{
		// Before using the [] operator test if the entity exists in that map.
		T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(top_level);
		if(tempConstIterator == m_Top_Maps.end())
			return ;

		m_Top_Maps[top_level].DeleteMapping(key,entity);
	}

	m_HOOPS_to_ACIS.erase(key);
//	DebugMaps();
}

// returns zero if entity not found
HC_KEY HA_Map::FindMapping( ENTITY* entity )
{
	HC_KEY key=0;

	EXCEPTION_BEGIN
		ENTITY *top_level=0;
	EXCEPTION_TRY
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

		if ( Is_In_Top_Map( entity ) )
			top_level = entity;
		else
			api_get_owner(entity, top_level);

		T_entity_to_HA_TL_Map_const_iterator entity_to_HA_TL_Map_const_iterator= m_Top_Maps.find(entity);
		T_entity_to_HA_TL_Map_const_iterator entity_to_HA_TL_Map_const_iterator_end = m_Top_Maps.end();
		if (entity_to_HA_TL_Map_const_iterator==entity_to_HA_TL_Map_const_iterator_end)
			return 0;

		if( m_Top_Maps.find( top_level ) != m_Top_Maps.end() )
		{
			key=m_Top_Maps[top_level].FindMapping(entity);
		}
		if( !key )
		{
			key=m_Top_Maps[entity].FindMapping(entity);
		}

		if (key && !valid_segment(key))
		{
			// This should not be hit.  If it does we need to fix something.
			// Please submit bug report.
			sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
			// The key is invalid so remove it from the map
		}
	EXCEPTION_CATCH_FALSE
		DeleteMapping(key);
		key=0;
	EXCEPTION_END

	return key;
}

// returns 0 if key not found
ENTITY* HA_Map::FindMapping(HC_KEY key)
{
	ENTITY* ent = NULL;
	// First thing is make sure the key is still good
	EXCEPTION_BEGIN
	EXCEPTION_TRY
		if (!valid_segment(key))
		{
			// This should not be hit.  If it does we need to fix something.
			// Please submit bug report.
			sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
		}
		
		if( m_HOOPS_to_ACIS.find(key) != m_HOOPS_to_ACIS.end() )
		{
			ent = m_HOOPS_to_ACIS[key];
		}
		else
		{
			ent = NULL;
		}
	EXCEPTION_CATCH_FALSE
		DeleteMapping(key);
		ent = NULL;
	EXCEPTION_END
	return ent;
}

// returns number of keys found (for multiple mappings)
unsigned long HA_Map::FindMapping(ENTITY* entity, HC_KEY* keys, unsigned long count)
{
	unsigned long num_keys_mapped_to_entity=0;

	EXCEPTION_BEGIN
		ENTITY *top_level=0;
		HC_KEY key = 0;
	EXCEPTION_TRY
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

		if ( Is_In_Top_Map(entity) )
			top_level = entity;
		else
			api_get_owner(entity, top_level);

		// Before using the [] operator test if the entity exists in that map.
		T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(top_level);
		if(tempConstIterator != m_Top_Maps.end())
		{	
			T_entity_to_segment_list_const_iterator tempConstIterator2 = m_Top_Maps[top_level].m_ACIS_to_HOOPS.find(entity);
			if(tempConstIterator2 != m_Top_Maps[top_level].m_ACIS_to_HOOPS.end())
			{
				T_segment_list_const_iterator end = m_Top_Maps[top_level].m_ACIS_to_HOOPS[entity].end();
				T_segment_list_const_iterator p;
				for (p=m_Top_Maps[top_level].m_ACIS_to_HOOPS[entity].begin();
					 p!=end && num_keys_mapped_to_entity < count;
					 ++p)
				{
					key = *p;

					// Check to see if 
					if (valid_segment(key))
					{
						keys[num_keys_mapped_to_entity] = key;
						num_keys_mapped_to_entity++;
					}
					else
					{
						// This should not be hit.  If it does we need to fix something.
						// Please submit bug report.
						sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
						DeleteMapping(key);
					}
				}
			}
		}
		
		// Typically an ENTITY that isn't top level won't have any mappings
		// if you use m_Top_Maps[entity], but sometimes they get absorbed into another
		// entity.  So we need to check this case.
		if (entity!=top_level)
		{
			// Before using the [] operator test if the entity exists in that map.
			T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(entity);
			if(tempConstIterator != m_Top_Maps.end())
			{			
	#if 1	
				// fang Aug 5 2004. Fixed up top map. Before fixing, only part of the keys for an entity are returned
				// which causes top map can not be cleared
				
				T_entity_to_segment_list_const_iterator tesi;
				int hat_size = (int)tempConstIterator->second.m_ACIS_to_HOOPS.size();
				if (hat_size > 0)
				{
					for (tesi = tempConstIterator->second.m_ACIS_to_HOOPS.begin(); tesi != tempConstIterator->second.m_ACIS_to_HOOPS.end(); tesi++)
					{					
						T_segment_list_const_iterator tsli;
						for (tsli = tesi->second.begin(); tsli != tesi->second.end(); ++tsli)
						{
							key = *tsli;
		
							// Check to see if 
							if (valid_segment(key))
							{
								keys[num_keys_mapped_to_entity] = key;
								num_keys_mapped_to_entity++;
							}
							else
							{
								// This should not be hit.  If it does we need to fix something.
								// Please submit bug report.
								sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
								
							}
						}
					}
				}
					
	#else
				T_entity_to_segment_list_const_iterator tempConstIterator2 = m_Top_Maps[entity].m_ACIS_to_HOOPS.find(entity);
				if(tempConstIterator2 != m_Top_Maps[top_level].m_ACIS_to_HOOPS.end())  // add toplevel to top level map
				{
					T_segment_list_const_iterator end = m_Top_Maps[entity].m_ACIS_to_HOOPS[entity].end();
					T_segment_list_const_iterator p;
					for (p=m_Top_Maps[entity].m_ACIS_to_HOOPS[entity].begin();
						 p!=end && num_keys_mapped_to_entity < count;
						 ++p)
					{						
						key = *p;

						// Check to see if 
						if (valid_segment(key))
						{
							keys[num_keys_mapped_to_entity] = key;
							num_keys_mapped_to_entity++;
						}
						else
						{
							// This should not be hit.  If it does we need to fix something.
							// Please submit bug report.
							sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
							
						}
					}
				}
	#endif

			}
		}
	EXCEPTION_CATCH_FALSE
		DeleteMapping(key);
	EXCEPTION_END

//	DebugMaps();
	return num_keys_mapped_to_entity;
}

// returns number of keys found (for multiple mappings)
unsigned long HA_Map::FindNumMappings(ENTITY* entity)
{
	unsigned long num_keys_mapped_to_entity=0;

	ENTITY *top_level=0;
	api_get_owner(entity, top_level);

	// Before using the [] operator test if the entity exists in that map.
	T_entity_to_HA_TL_Map_const_iterator tempConstIterator = m_Top_Maps.find(top_level);
	if(tempConstIterator == m_Top_Maps.end())
		return 0;

	num_keys_mapped_to_entity=m_Top_Maps[top_level].NumSegments(entity);

	// Typically an ENTITY that isn't top level won't have any mappings
	// if you use m_Top_Maps[entity], but sometimes they get absorbed into another
	// entity.  So we need to check this case.
	if (entity!=top_level)
		num_keys_mapped_to_entity+=m_Top_Maps[entity].NumSegments(entity);

	return num_keys_mapped_to_entity;
}

void HA_Map::DebugMaps(void)
{
	FILE *fp;

	fp = fopen( "c:/log/hoopsmap.txt", "a" );

	printf("====================================\n");
	fprintf(fp, "====================================\n");
	{
		printf("m_HOOPS_to_ACIS map:\n");
		fprintf(fp, "m_HOOPS_to_ACIS map:\n");
		STD map<HC_KEY,ENTITY*>::iterator topi;
 		for (topi = m_HOOPS_to_ACIS.begin(); topi != m_HOOPS_to_ACIS.end(); topi++)
		{
			const char* enttype = topi->second->type_name();
			printf("key = %d\t%s=0x%x\n", topi->first, enttype, topi->second);
			fprintf(fp, "key = %d\t%s=0x%x\n", topi->first, enttype, topi->second);
//			printf("key = %d\tENITTY=0x%x\n", topi->first, topi->second);
//			fprintf(fp, "key = %d\tENITTY=0x%x\n", topi->first, topi->second);

			logical found = FALSE;
			T_entity_to_HA_TL_Map_const_iterator etli;
			for (etli = m_Top_Maps.begin(); etli != m_Top_Maps.end() && !found; etli++)
			{
				T_entity_to_segment_list_const_iterator tesi;
				int hat_size = (int) etli->second.m_ACIS_to_HOOPS.size();
				if (hat_size > 0)
				{
					for (tesi = etli->second.m_ACIS_to_HOOPS.begin(); tesi != etli->second.m_ACIS_to_HOOPS.end() && !found; tesi++)
					{
						T_segment_list_const_iterator tsli;
						for (tsli = tesi->second.begin(); tsli != tesi->second.end() && !found; ++tsli)
							found = (topi->first == *tsli);
					}
				}
			}
//			if (!found)
//				sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
		}
		printf("\n");
		fprintf(fp, "\n");
	}

	{
		//printf("m_Top_Maps map:\n");
		fprintf(fp, "m_Top_Maps map:\n");
		T_entity_to_HA_TL_Map_const_iterator etli;
		for (etli = m_Top_Maps.begin(); etli != m_Top_Maps.end(); etli++)
		{
			const char* enttype = etli->first->type_name();
			printf("%s=0x%x\n", enttype, etli->first);
			fprintf(fp, "%s=0x%x\n", enttype, etli->first);

			T_entity_to_segment_list_const_iterator tesi;
			int hat_size = (int) etli->second.m_ACIS_to_HOOPS.size();
			if (hat_size > 0)
			{
				for (tesi = etli->second.m_ACIS_to_HOOPS.begin(); tesi != etli->second.m_ACIS_to_HOOPS.end(); tesi++)
				{
					const char* enttype1 = ((ENTITY*)tesi->first)->type_name();
					printf("\t%s=0x%x\n", enttype1, tesi->first);
					fprintf(fp, "\t%s=0x%x\n", enttype1, tesi->first);
					
					T_segment_list_const_iterator tsli;
					for (tsli = tesi->second.begin(); tsli != tesi->second.end(); ++tsli)
					{
						printf("\t\tHC_KEY=%d\n", *tsli);
						fprintf(fp, "\t\tHC_KEY=%d\n", *tsli);

						logical found = FALSE;
						STD map<HC_KEY,ENTITY*>::iterator topi;
				 		for (topi = m_HOOPS_to_ACIS.begin(); topi != m_HOOPS_to_ACIS.end() && !found; topi++)
							found = (topi->first == *tsli);
//						if (!found)
//							sys_error(HA_BRIDGE_SEGMENT_DOES_NOT_EXIST);
					}
				}
			}
		}
	}
	printf("\n");
	fprintf(fp, "\n");
	fclose( fp );
}

int HA_Map::HA_Internal_Get_Map_Entries(void)
{
	int HOOPS_to_Acis_Count = -1;
	int Top_Maps_Count = -1;

	// count the entries in m_HOOPS_to_ACIS map
	HOOPS_to_Acis_Count = (int) m_HOOPS_to_ACIS.size();

	// count the entries in m_Top_Maps map
	Top_Maps_Count = (int) m_Top_Maps.size();

	return HOOPS_to_Acis_Count + Top_Maps_Count;
}

logical HA_Map::HA_Is_In_Top_Map( ENTITY* ent )
{
	if( (m_Top_Maps.find( ent )) != m_Top_Maps.end() )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
