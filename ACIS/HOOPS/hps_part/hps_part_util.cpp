/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_part_util.h"
#include "id_attr.hxx"
#include "acis_pm_entity_mgr.hxx"
#include "asm_api.hxx"
#include "hps_bridge.h"
#include "hps_bridge_asm.h"
#include "ckoutcom.hxx"

asm_model* HAPart_Get_Owning_Model( ENTITY * in_entity )
{
	asm_model* model = NULL;
	ID_ATTRIB* att = find_ID_ATTRIB( in_entity );
	if ( att )
	{
		PART* part = att->get_part();
		if ( part )
		{
			const acis_pm_entity_mgr* mgr = part->get_entity_manager();
			if ( mgr )
				model = mgr->get_model();
		}
	}
	return model;
}

void HAPart_Render_Entity( ENTITY* in_entity )
{
	asm_model* owning_model = HAPart_Get_Owning_Model( in_entity );
	if ( owning_model )
		HPS_Render_Model( owning_model );
	else
		HPS_Render_Entity( in_entity );
}

void HAPart_Render_Entities( const ENTITY_LIST& in_entity_list )
{
	ENTITY* entity = NULL;
	for ( entity = in_entity_list.first(); entity; entity = in_entity_list.next() )
		HAPart_Render_Entity( entity );
}
