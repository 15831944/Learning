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

#include "ientityconverter.h"
#include "ha_rend_options.h"
#include "ha_rend_context.h"
#include "ha_map.h"
#include "ha_map_asm.h"
#include "asm_assembly.hxx"
#include "asm_model.hxx"
#include "asm_model_ref.hxx"
#include "ha_bridge.err"
#include "asm_api.hxx"


HC_KEY IEntityConverter::ConvertEntity(
	ENTITY*						entity,
	const ha_rendering_options&	ro,
	HA_Map*						map,
	const char*					pat)
{
	HC_KEY answer = 0;

	if (is_ASM_ASSEMBLY(entity) || is_ASM_MODEL_REF(entity))
	{
		sys_error(HA_BRIDGE_ASM_NOT_SUP);
	}
	else
	{
		ha_rendering_options copy_ro(ro);
		ha_rendering_context rc;
		rc.SetPattern(pat);
		rc.SetEntityMap(map);
		HA_ModelMap arm;
		rc.SetModelMap(&arm);
		answer = ConvertEntityAsm(entity, copy_ro, rc);
	}

	return answer;
}

HC_KEY IEntityConverter::ConvertEntityAsm(
	ENTITY*						entity,
	const ha_rendering_options& ro,
	const ha_rendering_context& rc)
{
	HC_KEY answer = 0;

	if (is_ASM_ASSEMBLY(entity) || is_ASM_MODEL_REF(entity))
	{
		sys_error(HA_BRIDGE_ASM_NOT_OVR);
	}
	else
	{
		answer = ConvertEntity(entity, ro, rc.GetEntityMap(), rc.GetPattern());
	}

	return answer;
}

HC_KEY IEntityConverter::ConvertModelGeometry(
	asm_model*					model,
	const ha_rendering_options& ro,
	const ha_rendering_context& rc)
{

	HC_KEY answer = 0;
	
	MODEL_BEGIN(model)

	ENTITY_LIST ents;
	model->get_top_level_entities(ents, FALSE);
	ENTITY* this_ent = NULL;

	for (this_ent = ents.first(); this_ent; this_ent = ents.next())
	{
		ConvertEntityAsm(this_ent, ro, rc);
	}

	MODEL_END(ASM_NO_CHANGE)

	if (((ha_rendering_options&)ro).GetMappingFlag())
	{
		HA_ModelMap* model_map = rc.GetModelMap();
		if (model_map)
			answer = model_map->FindMapping(model);
	}

	return answer;
}

HC_KEY IEntityConverter::ConvertModelComponents(
	component_handle*,
	const ha_rendering_options&,
	const ha_rendering_context&)
{
	return 0;
}

