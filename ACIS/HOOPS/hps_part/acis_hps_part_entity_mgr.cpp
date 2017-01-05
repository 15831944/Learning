/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include <assert.h>
#include "acis_hps_part_entity_mgr.hxx"
#include "part.hxx"
#include "lists.hxx"
#include "roll_hm.hxx"
#include "asm_assembly.hxx"
#include "ckoutcom.hxx"
#include "asm_error.err"
#include "asm_model.hxx"
#include "asm_error.err"
#include "asm_event_info.hxx"
#include "asm_model_info.hxx"
#include "api.hxx"
#include "asm_api.hxx"
#include "kernapi.hxx"
#include "hps_bridge.h"
#include "hps_bridge_asm.h"
#include "hps_part_util.h"
#include "hps_rend_options.h"

acis_hps_part_entity_mgr::acis_hps_part_entity_mgr( PART* part ) :
acis_pm_entity_mgr( part ), is_part_viewed( FALSE )
{
	hps_rendering_options& ro = HPS_Get_Rendering_Options();
	char buffer[1024];

	if ( !ro.GetPattern() )
	{
		char pbuffer[POINTER_BUFFER_SIZE];
		sprintf( buffer, "segment pattern = ?Include Library/%s", ptoax( part, pbuffer ) );
		//sprintf( buffer, "segment pattern = ?Include Library/PART_%d", part->get_part_id() );
		HPS_Set_Rendering_Options( buffer );
	}

	if ( !ro.GetGeomPattern() )
	{
		sprintf( buffer, "geom pattern = ?Include Library/ACIS model geometry" );
		HPS_Set_Rendering_Options( buffer );
	}
}

// customer entity managers should NOT override acis_hps_part_cast_vf
// instead they should override hps_part_customer_cast_vf if deriving directly from acis_hps_part_entity_mgr
acis_hps_part_entity_mgr* acis_hps_part_entity_mgr::acis_hps_part_cast_vf()
{
	return this;
}

// customer entity managers should NOT override acis_scm_cast_vf
// instead they should override hps_part_customer_cast_vf if deriving directly from acis_hps_part_entity_mgr
acis_scm_entity_mgr* acis_hps_part_entity_mgr::acis_scm_cast_vf()
{
	return NULL;
}

// customer entity managers derived directly from acis_hps_part_entity_mgr should override hps_part_customer_cast_vf
void* acis_hps_part_entity_mgr::hps_part_customer_cast_vf()
{
	return NULL;
}

// implementation of non-virtual public interface function which wraps acis_scm_cast_vf()
// it is good design practice to make all virtual functions protected
acis_scm_entity_mgr* acis_hps_part_entity_mgr::acis_scm_cast()
{
	if ( !this )
	{
		return NULL;
	}

	return acis_scm_cast_vf();
}

// implementation of non-virtual public interface function which wraps hps_part_customer_cast_vf()
// it is good design practice to make all virtual functions protected
void* acis_hps_part_entity_mgr::hps_part_customer_cast()
{
	if ( !this )
	{
		return NULL;
	}

	return hps_part_customer_cast_vf();
}
/*
// responsible for re-building scene graphs
void acis_hps_part_entity_mgr::entities_changed_vf( outcome& result, asm_event_type& ev_type )
{
if (!the_PART)
{
sys_error( ASM_NO_PART );
}

if (!result.ok())
{
ev_type = ASM_NO_CHANGE;
}

asm_model* model = get_model();

if (model)
{
logical updated = FALSE;
HPS::SegmentKey model_key= 0;
// B-rep scene graph segments are shared among models so always rebuild them upon change notification
if ( ev_type == ASM_BREP_GEOM_CHANGE || ev_type == ASM_BREP_RENDER_CHANGE )
{
model_key = HPS_ReRender_Model_Geometry(model);
if (model_key != 0)
updated = HC_Update_Display(); // jkf Feb0209 R20
}
// Scene graph of assembly's components is only used if this model is being viewed
if (is_viewed() &&  // only build a scene graph if it will be used
(ev_type == ASM_COMP_GEOM_CHANGE || ev_type == ASM_COMP_RENDER_CHANGE) )
{
model_key = HPS_ReRender_Model_Components(model);
if (model_key != 0)
updated = HC_Update_Display(); // jkf Feb0209 R20
}
}
}
*/
// responsible for re-building scene graphs
void acis_hps_part_entity_mgr::entities_changed_vf( outcome& result, asm_event_type& ev_type, asm_event_info* ev_info )
{
	if ( !the_PART )
	{
		sys_error( ASM_NO_PART );
	}

	if ( !result.ok() )
	{
		ev_type = ASM_NO_CHANGE;
	}

	asm_model* model = get_model();

	if ( model )
	{
		logical updated = FALSE;
		HPS::SegmentKey model_key = HPS_INVALID_SEGMENT_KEY;
		// B-rep scene graph segments are shared among models so always rebuild them upon change notification
		if ( ev_type == ASM_BREP_GEOM_CHANGE || ev_type == ASM_BREP_RENDER_CHANGE )
		{
			fp_sentry fps;
			logical do_update( FALSE );
			if ( ev_type == ASM_BREP_GEOM_CHANGE && ev_info && ev_info->type() == asm_brep_transform_info::id() )
			{
				ENTITY_LIST bodies;
				model->get_top_level_entities( bodies, FALSE );
				do_update = HPS_ReRender_Body_Transforms( bodies );
			} else
			{
				model_key = HPS_ReRender_Model_Geometry( model );
				do_update = HPS_Is_Valid_Key( model_key );
			}

			if ( do_update )
				updated = HPS_Update_Display(); // jkf Feb0209 R20
		}
		// Scene graph of assembly's components is only used if this model is being viewed
		if ( is_viewed() &&  // only build a scene graph if it will be used
			 ( ev_type == ASM_COMP_GEOM_CHANGE || ev_type == ASM_COMP_RENDER_CHANGE ) )
		{
			fp_sentry fps;
			model_key = HPS_ReRender_Model_Components( model );
			if ( HPS_Is_Valid_Key( model_key ) )
				updated = HPS_Update_Display(); // jkf Feb0209 R20
		}
	}
}

void acis_hps_part_entity_mgr::done_restoring_vf()
{
	if ( !the_PART )
	{
		sys_error( ASM_NO_PART );
	}

	// refresh the rendering for B-Rep entities
	// the entities were already added to the part by a call to acis_pm_entity_mgr::add_entities_vf()
	ENTITY_LIST ents;
	get_top_level_entities( ents, FALSE );
	for ( ENTITY* ent = ents.first(); ent; ent = ents.next() )
	{
		API_BEGIN
			the_PART->set_modified( TRUE );
		the_PART->update( ent );
		API_END
	}

	// only build scene graph for assembly's components if they will be viewed
	if ( is_viewed() )
	{
		asm_model* model = get_model();
		if ( model )
		{
			HPS_Render_Model( model );
		}
	}
}

void acis_hps_part_entity_mgr::sub_model_changed_vf( asm_event_type ev_type, asm_model* /*changed_model*/ )
{
	// This model's portion of the scene graph is unaffected by changes to a sub-model's
	// B-Rep data, so we only need to rebuild if it was a change to the assembly structure
	// and this model is being viewed.
	if ( !( ev_type == ASM_COMP_GEOM_CHANGE || ev_type == ASM_COMP_RENDER_CHANGE ) ||
		 !is_viewed() )
	{
		return;
	}

	if ( !the_PART )
	{
		sys_error( ASM_NO_PART );
	}

	MODEL_BEGIN( get_model() )

	if ( is_viewed() )
	{
		// Rebuild scene graph of assembly's components
		fp_sentry fps;
		asm_model* model = get_model();
		HPS_ReRender_Model_Components( model );
		HPS_Update_Display();  // jkf Feb0209 R20
	}

	MODEL_END( ASM_NO_CHANGE )

}

void acis_hps_part_entity_mgr::unbind_vf( asm_model* model )
{
	// Clean up hoops segment and mapping for the model
	if ( model )
	{
		HPS_Delete_Model( model );
	}
}

// notify this entity manager whether it needs to build its assembly component scene graph
void acis_hps_part_entity_mgr::set_viewed( logical is )
{
	if ( is && !is_part_viewed )
	{
		asm_model* model = get_model();
		HPS_Render_Model( model );
	}

	is_part_viewed = is;
}

logical acis_hps_part_entity_mgr::is_viewed() const
{
	return is_part_viewed;
}
