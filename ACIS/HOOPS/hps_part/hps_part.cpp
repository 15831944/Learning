/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "acis.hxx"    

#include "lists.hxx"
#include "ckoutcom.hxx"

#include "part.hxx"    
#include "hashpart.hxx"
#include "part_api.hxx"    


#include "hps_part.h"
#include "hps_callback.h"
#include "hps_bridge_internal.h"


void initialize_hps_part()
{
	HPS_History_Callbacks *cb = ACIS_NEW HPS_History_Callbacks;
	get_history_callbacks_list().add( cb );
}

void terminate_hps_part()
{
}

HPS_Part::HPS_Part()
{
	// create a new part
	m_pPart = NULL;
	outcome result = api_part_create( 0, m_pPart );
	check_outcome( result );
	CreateCallback();
}

HPS_Part::~HPS_Part()
{
	outcome res = api_part_delete( m_pPart );
	check_outcome( res );
	m_pPart = 0;
}

void HPS_Part::CreateCallback()
{
	// Add the HPS_EntityCallback to the PART callbacks
	if ( m_pPart )
		m_pPart->set_callback( ACIS_NEW HPS_EntityCallback( m_pPart ) );
}

logical HPS_Part::LoadFile( const char* in_file_path_name, logical in_text_flag )
{
	if ( !in_file_path_name || !m_pPart )
		return FALSE;
	ENTITY_LIST list;
	outcome res = m_pPart->load( in_file_path_name, in_text_flag, list, TRUE );
	check_outcome( res );
	return TRUE;
}

logical HPS_Part::AddEntities( const ENTITY_LIST& in_entity_list )
{
	if ( !m_pPart )
		return FALSE;
	// add given entites to the part
	ENTITY* entity = 0;
	in_entity_list.init();
	while ( ( entity = in_entity_list.next() ) != 0 )
		m_pPart->add( entity );
	return TRUE;
}

logical HPS_Part::AddEntity( ENTITY*& in_entity )
{
	if ( !m_pPart )
		return FALSE;
	// add given entites to the part
	m_pPart->add( in_entity );
	return TRUE;
}

logical HPS_Part::RemoveEntity( ENTITY*& in_entity )
{
	if ( !m_pPart )
		return FALSE;

	// add given entites to the part
	m_pPart->remove( in_entity );

	return TRUE;

}

logical HPS_Part::UpdateEntity( ENTITY*& in_entity )
{
	if ( !m_pPart )
		return FALSE;
	m_pPart->set_modified( TRUE );
	m_pPart->update( in_entity );
	return TRUE;
}

logical HPS_Part::RollBack()
{
	HISTORY_STREAM* hs = m_pPart->history_stream();
	int n_actual;
	outcome o = api_part_roll_n_states( -1, hs, n_actual );
	check_outcome( o );
	return TRUE;
}

logical HPS_Part::RollForward()
{
	HISTORY_STREAM* hs = m_pPart->history_stream();
	int n_actual;
	outcome o = api_part_roll_n_states( 1, hs, n_actual );
	check_outcome( o );
	return TRUE;
}
