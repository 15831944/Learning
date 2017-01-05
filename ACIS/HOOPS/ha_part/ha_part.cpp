/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#include "acis.hxx"    

#include "hc.h"

#include "lists.hxx"
#include "ckoutcom.hxx"

#include "part.hxx"    
#include "hashpart.hxx"
#include "part_api.hxx"    


#include "ha_part.h"
#include "ha_callback.h"
#include "ha_bridge_internal.h"


void initialize_ha_part()
{
	HA_History_Callbacks *cb = ACIS_NEW HA_History_Callbacks;
	get_history_callbacks_list().add(cb);
}

void terminate_ha_part()
{
}

HA_Part::HA_Part()
{
	// create a new part
	m_pPart = NULL;
	outcome result = api_part_create ( 0, m_pPart );
	check_outcome(result);
	CreateCallback();
}

HA_Part::~HA_Part()
{
	outcome res = api_part_delete(m_pPart);
	check_outcome(res);
	m_pPart = 0;
}

void HA_Part::CreateCallback()
{
	// Add the HA_EntityCallback to the PART callbacks
	if (m_pPart)
		m_pPart->set_callback( ACIS_NEW HA_EntityCallback(m_pPart));
}


logical HA_Part::LoadFile( const char* filePathName, logical textFlag)
{
	if (!filePathName || !m_pPart )
		return FALSE;

	ENTITY_LIST list;

	outcome res = m_pPart->load(  filePathName,  textFlag,  list, TRUE );
	check_outcome( res );

	return TRUE;
}

logical HA_Part::AddEntities( const ENTITY_LIST& entityList )
{
	if( !m_pPart )
		return FALSE;

	// add given entites to the part
	ENTITY* entity = 0;
	entityList.init();
	while ((entity = entityList.next()) != 0)
		m_pPart->add( entity );

	return TRUE;

}


logical HA_Part::AddEntity( ENTITY*& newEntity )
{
	if( !m_pPart )
		return FALSE;

	// add given entites to the part
	m_pPart->add( newEntity );

	return TRUE;

}

logical HA_Part::RemoveEntity( ENTITY*& newEntity )
{
	if( !m_pPart )
		return FALSE;

	// add given entites to the part
	m_pPart->remove( newEntity );

	return TRUE;

}

logical HA_Part::UpdateEntity( ENTITY*& ent )
{
	if( !m_pPart )
		return FALSE;

   	m_pPart->set_modified(TRUE);
	m_pPart->update( ent );

	return TRUE;
}

logical HA_Part::RollBack()
{
	HISTORY_STREAM* hs = m_pPart->history_stream();

	int n_actual;
	outcome o = api_part_roll_n_states(-1, hs, n_actual); 
	check_outcome(o);

	return TRUE;
}

logical HA_Part::RollForward()
{
	HISTORY_STREAM* hs = m_pPart->history_stream();

	int n_actual;
	outcome o = api_part_roll_n_states(1, hs, n_actual); 
	check_outcome(o);

	return TRUE;
}
