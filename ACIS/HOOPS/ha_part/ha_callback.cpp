/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

// Modifications
//
// jce (06/07/06) : Moved the body of HA_EntityCallback::execute from
//                  ha_callback.h to this new source file

#include "ha_callback.h"
#include "ha_bridge.h"
#include "getowner.hxx"
#include "hashpart.hxx"
#include "rlt_util.hxx"
#include "at_col.hxx"
#include "asm_assembly.hxx"
#include "asm_model_ref.hxx"

ENTITY_LIST* create_ents_list;
ENTITY_LIST* delete_ents_list;
ENTITY_LIST* change_ents_list;

LOCAL_PROC void set_lists_for_roll_callbacks( ENTITY_LIST* c, ENTITY_LIST* d, ENTITY_LIST* m)
{
	create_ents_list = c;
	delete_ents_list = d;
	change_ents_list = m;
}

//======================================================================
// HA_EntityCallback is used to hook up to acis entity callback notifier

void HA_EntityCallback::execute( entity_event_type ev_type, ENTITY *ent)
{
	if (!ent)
		return;

	if (is_ASM_ASSEMBLY(ent) || is_ASM_MODEL_REF(ent))
		return;

	// For the case where a face is absorbed into a sheet its no
	// longer TL.  PART should check if TL anyways.
	// !!!!! This caused regressions, but most likely zombies accur on part:clear.
	
	if (!IS_LIGHT(ent) && !is_toplevel(ent))
	{
		// fang begin - 08/16/04. If the entity is no longer a top level entity but still in
		// top level hoops map, we need to remove it from hoops map
		if (Is_In_Top_Map(ent))
		{
			if (ev_type == pm_Delete_Entity || 
				ev_type == pm_Remove_Entity || 
				ev_type == pm_Part_Delete_Delete_Entity)
			{
				HA_Delete_Entity_Geometry(ent);
			}
			else if (ev_type == pm_Roll_Create_Entity && delete_ents_list)
			{
				delete_ents_list->add(ent);
			}
		}
		// fang end
		return;
	}

	switch(ev_type)
	{
		case pm_Create_Entity:
			HA_Render_Entity(ent);
			break;

		case pm_Delete_Entity:
		case pm_Remove_Entity:
		case pm_Part_Delete_Delete_Entity:
			// We may get called for anything in the part with an ID
			// but only toplevel things can actually be deleted from the 
			// HOOPS data base
			// fang - 08/16/04 But if the entity which was top level before
			// and is no longer a top level, we still need to delete it from Hoops 
			// database. Code for HA_Delete_Entity_Geometry has been changed to do that
			HA_Delete_Entity_Geometry(ent);
			break;

		case pm_Update_Entity:

			HA_Delete_Entity_Geometry(ent);
			HA_Render_Entity(ent);
			break;

		case pm_Roll_Create_Entity:
			if (delete_ents_list)
			{
				delete_ents_list->add(ent);
			}
			break;

		case pm_Roll_Delete_Entity:
			if (create_ents_list)
			{
				create_ents_list->add(ent);
			}
			break;
		
		case pm_Roll_Update_Entity:
			if (change_ents_list)
			{
				change_ents_list->add(ent);
			}
			break;
	}
}


void HA_History_Callbacks::Before_Roll_Bulletin_Board(BULLETIN_BOARD*, logical discard) 
{
	// Do your before roll stuff for one BULLETIN_BOARD
	// if discard is true, the  roll is due to  error processing and the 
	// BULLETIN_BOARD will be deleted along with all its BULLETINS.

	if (discard)
		return;
	
	set_lists_for_roll_callbacks( &create_ents_list, &delete_ents_list, &change_ents_list);
}

void HA_History_Callbacks::After_Roll_Bulletin_Board(BULLETIN_BOARD*, logical discard) 
{
	// Do your after roll stuff for one BULLETIN_BOARD;
	// if discard is true, the  roll is due to  error processing and the 
	// BULLETIN_BOARD will be deleted along with all its BULLETINS.

	if (discard)
		return;

	API_NOP_BEGIN

	delete_ents_list.add( change_ents_list);
	HA_Delete_Entity_Geometry( delete_ents_list);

	create_ents_list.add( change_ents_list);
	HA_Render_Entities( create_ents_list);

	create_ents_list.clear();
	delete_ents_list.clear();
	change_ents_list.clear();

	set_lists_for_roll_callbacks( NULL, NULL, NULL);

	API_NOP_END
}
