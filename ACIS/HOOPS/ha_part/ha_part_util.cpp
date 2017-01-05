/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "ha_part_util.h"
#include "id_attr.hxx"
#include "acis_pm_entity_mgr.hxx"
#include "asm_api.hxx"
#include "ha_bridge.h"
#include "ha_bridge_asm.h"
#include "ckoutcom.hxx"

asm_model* HAPart_Get_Owning_Model(ENTITY* ent)
{
	asm_model* model = NULL;

	ID_ATTRIB* att = find_ID_ATTRIB(ent);

	if (att)
	{
		PART* part = att->get_part();

		if (part)
		{
			const acis_pm_entity_mgr* mgr = part->get_entity_manager();

			if (mgr)
			{
				model = mgr->get_model();
			}
		}
	}

	return model;
}

void HAPart_Render_Entity(ENTITY* ent)
{
	asm_model* owning_model = HAPart_Get_Owning_Model(ent);

	if (owning_model)
		HA_Render_Model(owning_model);
	else
		HA_Render_Entity(ent);
}

void HAPart_Render_Entities(const ENTITY_LIST& ents)
{
	ENTITY* entity = NULL;

	for (entity = ents.first(); entity; entity = ents.next())
		HAPart_Render_Entity(entity);
}
