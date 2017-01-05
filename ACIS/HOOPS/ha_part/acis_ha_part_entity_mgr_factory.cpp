/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "asm_model.hxx"
#include "entity_mgr_factory.hxx"
#include "acis_ha_part_entity_mgr_factory.hxx"
#include "acis_ha_part_entity_mgr.hxx"
#include "part_api.hxx"
#include "pmhusk.err"
#include "roll_hm.hxx"


acis_ha_part_entity_mgr_factory::acis_ha_part_entity_mgr_factory(const VOID_LIST& seed_parts) :
	acis_pm_entity_mgr_factory(seed_parts)
{
}


LOCAL_PROC PART* find_part_with_stream(HISTORY_STREAM const* the_stream)
{
	PART* answer = NULL;
	if(!distributed_history(FALSE))
	{
		return NULL;
	}

	// iterate through part array, searching for part with same history stream
	int next_part_index = 0;
	PART* iter_part = get_next_part(next_part_index);
	while(iter_part)
	{
		if(iter_part->history_stream() == the_stream)
		{
			break; // found one
		}
		iter_part = get_next_part(next_part_index);
	}

	return iter_part;
}

asm_model_entity_mgr* acis_ha_part_entity_mgr_factory::create_entity_mgr_vf(asm_model_info const & model_info, 
                                                                            HISTORY_STREAM* share_stream)
{
	outcome result(0);
	acis_pm_entity_mgr* mgr = NULL;

	PART* part = NULL;
	
	// only look for pre-existing part if I'm not sharing my stream
	if(!share_stream)
	{
		part = get_part_from_list();
	}

	if(part == NULL)
		{result = api_part_create(0, part);}

	if (result.ok() && part)
	{
		// if new part is supposed to share history stream, reset its stream
		if(share_stream)
		{
			// If share_stream is non-null, then:
			// 1)  We're restoring an ASAT file that was saved with history
			// 2)  We're restoring with history and/or more than one model shared a history stream in the original session
			// If we're currently in distributed mode, then models cannot share history streams.
			// Therefore we'll check to see if any other models are already using this stream
			// and error out if we find one.  If we don't find one, then we'll bind the part to
			// the input stream
			if(distributed_history(FALSE))
			{
				// do the check
				PART* sharing_part = find_part_with_stream(share_stream);
				if(sharing_part)
				{
					sys_error(PM_NO_SHARE_DIST_HIST);
				}
				part->replace_history_stream(share_stream);
			}
			// If we're not in distributed mode, then all models share the same stream, but we're
			// not allowed to restore history.  Therefore we'll check to see if the
			// incoming stream is the same as the default stream.  If they are different, then
			// we're restoring with history and we'll error out.
			// 
			else
			{
				if(share_stream != get_default_stream(FALSE))
				{
					sys_error(PM_HM_RESTORE_UNDISTRIBUTED);
				}
			}
		}

		mgr = part->get_entity_manager();
		if(mgr == NULL)
			{mgr = ACIS_NEW acis_ha_part_entity_mgr(part);}
		mgr->set_model_info(model_info);
	}

	return mgr;
}
