/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/


//-----------------------------------------------------------//
// HOOPS-specific Header File(s)
//-----------------------------------------------------------//
#include <hc.h>

#include <vector>
#include <string>

//-----------------------------------------------------------//
// ACIS-specific Header File(s)
//-----------------------------------------------------------//

#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif // NT

#include "ckoutcom.hxx"

#include "ha_bridge_asm.h"
#include "ha_map.h"
#include "ha_map_asm.h"
#include "ha_rend_options.h"
#include "ha_rend_context.h"

#include "asm_assembly.hxx"
#include "asm_model_ref.hxx"
#include "asm_model.hxx"
#include "part.hxx"
#include "acis_pm_entity_mgr.hxx"
#include "asm_prop_api.hxx"
#include "asm_api.hxx"
#include "entity_handle.hxx"
#include "entity_handle_list.hxx"
#include "comp_handle.hxx"
#include "comp_ent_handle.hxx"
#include "rnd_api.hxx"
#include "vertex.hxx"
#include "acistype.hxx"
#include "body.hxx"
#include "ha_bridge_state.hxx"
//#include "base_thread_ctx.hxx"
#include "b_strutl.hxx"
#include "fct_utl.hxx"
component_handle_list s_HA_CHandles;

extern void merge_body_faces( BODY** bodies_to_merge, unsigned long num_bodies, const char* pattern );
extern HC_KEY KOpen_Pointer_Segment( void* SPAptr, const char* pattern );
extern char* Build_Segment_String( void* SPAptr, char* inbuffer, const char* pattern );

unsigned long HA_Compute_Model_Key_Count( asm_model* model )
{
	return get_ha_state()->s_pHA_ModelMap->FindNumMappings( model );
}

unsigned long HA_Compute_Model_Keys( asm_model* model, unsigned long count, HC_KEY* keys )
{	// there may be more than one hoops key mapped to a particular model.
	// so use the findmapping function which can return more than one key.
	// the last arg ensures that we don't stomp past the end of the array. rlw
	return get_ha_state()->s_pHA_ModelMap->FindMapping( model, keys, count );
}

asm_model* HA_Compute_Model_Pointer( HC_KEY key )
{
	asm_model* model = 0;
	model = get_ha_state()->s_pHA_ModelMap->FindMapping( key );
	return model;
}

void HA_Delete_Model_Ref( ASM_MODEL_REF* mref )
{
	if ( mref )
	{	// Find the handle to the corresponding component
		ASM_ASSEMBLY* assembly = mref->assembly();
		asm_model* model = NULL;
		check_outcome( api_asm_assembly_get_owning_model( assembly, model ) );
		entity_handle* hmref = NULL;
		check_outcome( api_asm_model_get_entity_handle( mref, model, hmref ) );
		component_handle* hcomp = NULL;
		check_outcome( asmi_model_get_component_handle( hmref, hcomp ) );
		// Find the subcomponents, and unmap them first
		component_handle_list subcomps;
		check_outcome( asmi_component_get_sub_components( hcomp, ASM_SUB, subcomps ) );
		HC_KEY keys[1024];
		component_handle* hsubcomp;
		for ( hsubcomp = subcomps.first(); hsubcomp; hsubcomp = subcomps.next() )
		{
			unsigned long count = get_ha_state()->s_pHA_CHandleMap->FindMapping( hsubcomp, keys, 1024 );
			unsigned long i;
			for ( i = 0; i < count; i++ )
			{
				HC_KEY comp_key = keys[i];
				if ( comp_key )
					get_ha_state()->s_pHA_CHandleMap->DeleteMapping( comp_key, hsubcomp );
			}
			if ( s_HA_CHandles.iteration_count() )
				s_HA_CHandles.remove( hsubcomp );
		}
		// Unmap the component and delete its segments (and subsegments)
		unsigned long count = get_ha_state()->s_pHA_CHandleMap->FindMapping( hcomp, keys, 1024 );
		unsigned long i;
		for ( i = 0; i < count; i++ )
		{
			HC_KEY comp_key = keys[i];
			if ( comp_key )
			{
				get_ha_state()->s_pHA_CHandleMap->DeleteMapping( comp_key, hcomp );
				if ( valid_segment( comp_key ) )
					HC_Delete_By_Key( comp_key );
			}
		}
		if ( s_HA_CHandles.iteration_count() )
			s_HA_CHandles.remove( hcomp );
	}
}

LOCAL_PROC void HA_Unmap_Component_Segments( char* seg )
{
	if ( seg )
	{
		HC_Open_Segment( seg );
		{
			HC_Begin_Segment_Search( "*" );
			{
				char child_seg[5120];
				char comp_ptr[32];
				logical child_found = FALSE;
				while ( HC_Find_Segment( child_seg ) )
				{
					HC_Parse_String( child_seg, "/", -1, comp_ptr );
					component_handle* comp = (component_handle*)axtop( comp_ptr );
					if ( HC_QShow_Existence( comp_ptr, "self" ) )
					{
						HC_KEY child_key = HC_KCreate_Segment( comp_ptr );

						if ( child_key )
							get_ha_state()->s_pHA_CHandleMap->DeleteMapping( child_key, comp );
						if ( s_HA_CHandles.iteration_count() )
							s_HA_CHandles.remove( comp );

						HA_Unmap_Component_Segments( child_seg );
					}
				}
			}
			HC_End_Segment_Search();
		}
		HC_Close_Segment();
	}
}

LOCAL_PROC void HA_Delete_Model_Components( asm_model* model )
{	//	if (model && sp_HA_CHandles && sp_HA_CHandles->iteration_count())
	if ( model && s_HA_CHandles.iteration_count() )
	{
		component_handle* top_component = NULL;
		check_outcome( asmi_model_get_component_handle( model, top_component ) );
		if ( top_component )
		{
			HC_KEY key = get_ha_state()->s_pHA_CHandleMap->FindMapping( top_component );
			if ( key )
			{
				char seg[128];
				HC_Show_Segment( key, seg );
				HA_Unmap_Component_Segments( seg );
				get_ha_state()->s_pHA_CHandleMap->DeleteMapping( key, top_component );
				if ( s_HA_CHandles.iteration_count() )
					s_HA_CHandles.remove( top_component );
				if ( valid_segment( key ) )
					HC_Delete_By_Key( key );
			}
		}
	}
}

void HA_Delete_Model_Geometry( asm_model* model )
{
	if ( !model )
		return;
	HC_KEY keys[1024];
	unsigned long count = get_ha_state()->s_pHA_ModelMap->FindMapping( model, keys, 1024 );
	ENTITY_LIST render_ents;
	model->get_top_level_entities( render_ents, FALSE );
	HA_Delete_Entity_Geometry( render_ents );
	unsigned long i;
	for ( i = 0; i < count; i++ )
	{
		HC_KEY key = keys[i];
		if ( key )
		{
			get_ha_state()->s_pHA_ModelMap->DeleteMapping( key, model );
			if ( valid_segment( key ) )
				HC_Delete_By_Key( key );
		}
	}
}

LOCAL_PROC void HA_Delete_Model_Geometry_Children_Internal( asm_model* model )
{
	if ( model )
	{
		HC_KEY keys[1024];
		unsigned long count = get_ha_state()->s_pHA_ModelMap->FindMapping( model, keys, 1024 );
		ENTITY_LIST render_ents;
		model->get_top_level_entities( render_ents, FALSE );
		HA_Delete_Entity_Geometry( render_ents );
		// (not sure the following is needed, as I think the HA_Delete_Entity_Geometry call effectively gets rid of all child segments)
		unsigned long i;
		for ( i = 0; i < count; i++ )
		{
			HC_KEY key = keys[i];
			if ( key )
			{
				if ( valid_segment( key ) )
				{
					HC_Open_Segment_By_Key( key );
					{
						HC_Begin_Segment_Search( "*" );
						{
							char child_seg[5120];
							while ( HC_Find_Segment( child_seg ) )
							{
								HC_Delete_Segment( child_seg );
							}
						}
						HC_End_Segment_Search();
					}
					HC_Close_Segment();
				}
			}
		}
	}
}

void HA_Delete_Model( asm_model* model )
{
	HA_Delete_Model_Components( model );
	HA_Delete_Model_Geometry( model );
}

logical HA_Associate_Key_To_Model( asm_model* model, HC_KEY key )
{
	assert( model );
	HC_KEY keys[1024];
	int count = get_ha_state()->s_pHA_ModelMap->FindMapping( model, keys, 1024 );
	logical mapping_exists = FALSE;
	for ( int i = 0; i < count; i++ )
	{
		if ( keys[i] == key )
		{
			mapping_exists = TRUE;
			break;
		}
	}
	if ( !mapping_exists )
	{
		get_ha_state()->s_pHA_ModelMap->AddMapping( key, model );
		return TRUE;
	}
	return TRUE;
}

logical HA_Disassociate_Key_From_Model( asm_model* model, HC_KEY key )
{
	if ( !model )
		model = get_ha_state()->s_pHA_ModelMap->FindMapping( key );
	if ( model )
	{
		get_ha_state()->s_pHA_ModelMap->DeleteMapping( key, model );
		return TRUE;
	}
	return FALSE;
}

unsigned long HA_Compute_Component_Key_Count( component_handle* comp )
{
	HC_KEY	keys[40000];
	return get_ha_state()->s_pHA_CHandleMap->FindMapping( comp, keys, 40000 );
}

unsigned long HA_Compute_Component_Keys( component_handle* comp, unsigned long count, HC_KEY* keys )
{	// there may be more than one hoops key mapped to a particular component.
	// so use the findmapping function which can return more than one key.
	// the last arg ensures that we don't stomp past the end of the array. rlw
	return get_ha_state()->s_pHA_CHandleMap->FindMapping( comp, keys, count );
}

component_handle* HA_Compute_Component_Pointer( HC_KEY key )
{
	component_handle* comp = 0;
	comp = get_ha_state()->s_pHA_CHandleMap->FindMapping( key );
	return comp;
}

logical HA_Associate_Key_To_Component( component_handle* comp, HC_KEY key )
{
	assert( comp );
	HC_KEY keys[1024];
	int count = get_ha_state()->s_pHA_CHandleMap->FindMapping( comp, keys, 1024 );
	logical mapping_exists = FALSE;
	int i;
	for ( i = 0; i < count; i++ )
	{
		if ( keys[i] == key )
		{
			mapping_exists = TRUE;
			break;
		}
	}
	if ( !mapping_exists )
	{
		get_ha_state()->s_pHA_CHandleMap->AddMapping( key, comp );
		return TRUE;
	}
	return TRUE;
}

logical HA_Disassociate_Key_From_Component( component_handle* comp, HC_KEY key )
{
	if ( !comp )
		comp = get_ha_state()->s_pHA_CHandleMap->FindMapping( key );
	if ( comp )
	{
		get_ha_state()->s_pHA_CHandleMap->DeleteMapping( key, comp );
		return TRUE;
	}
	return FALSE;
}

component_entity_handle* HA_Compute_Component_Entity_Pointer( unsigned long count, HC_KEY* keys )
{
	component_entity_handle* answer = NULL;
	if ( count > 0 && keys != NULL )
	{
		ENTITY* entity = NULL;
		component_handle* comp = NULL;
		unsigned long i;
		for ( i = 0; i < count && comp == NULL; i++ )
		{
			HC_KEY key = keys[i];
			comp = get_ha_state()->s_pHA_CHandleMap->FindMapping( key );
		}
		if ( comp )
		{
			for ( i = count - 1; i != -1 && entity == NULL; i-- )
			{
				HC_KEY key = keys[i];
				entity = get_ha_state()->s_pHA_MapAsm->FindMapping( key );
			}
			if ( entity )
			{
				asm_model* model = comp->get_end_model();
				entity_handle* eh = model->get_entity_handle( entity );
				asmi_model_get_component_entity_handle( comp, eh, answer );
			}
		}
	}
	return answer;
}

component_entity_handle* HA_Compute_Component_Entity_Pointer( HC_KEY comp_key, HC_KEY ent_key )
{
	component_entity_handle* answer = NULL;
	if ( comp_key != -1 && ent_key != -1 )
	{
		component_handle* comp = get_ha_state()->s_pHA_CHandleMap->FindMapping( comp_key );
		if ( comp )
		{
			ENTITY* entity = get_ha_state()->s_pHA_MapAsm->FindMapping( ent_key );
			if ( entity )
			{
				asm_model* model = comp->get_end_model();
				entity_handle* eh = model->get_entity_handle( entity );
				asmi_model_get_component_entity_handle( comp, eh, answer );
			}
		}
	}
	return answer;
}

logical HA_Compute_Component_Entity_Keys( component_entity_handle* comp_ent, HC_KEY& comp_key, HC_KEY& ent_key )
{
	logical answer( FALSE );
	if ( comp_ent )
	{
		component_handle* comp = comp_ent->component();
		entity_handle* ent_handle = comp_ent->entity();
		if ( comp && ent_handle )
		{
			ENTITY* ent = ent_handle->entity_ptr();
			comp_key = get_ha_state()->s_pHA_CHandleMap->FindMapping( comp );
			HC_KEY ent_keys[128];
			unsigned int ent_count = get_ha_state()->s_pHA_MapAsm->FindMapping( ent, ent_keys, 128 );
			if ( ent_count > 0 )
			{
				ent_key = ent_keys[0];
				answer = ( comp_key != -1 && ent_key != -1 );
			}
		}
	}
	return answer;
}

extern SESSION_GLOBAL_VAR option_header use_asm_highlight_segments;

void HA_Highlight_Component_Entity_old( component_entity_handle* ce, logical on, const rgb_color& color )
{
	if ( ce )
	{
		component_handle* c = ce->component();
		entity_handle* e = ce->entity();
		ENTITY* ent = e->entity_ptr();
		char buffer[64];
		HA_Build_Segment_String( ent, buffer, "entity" );

		if ( is_VERTEX( ent ) )
		{
			HC_Open_Segment( "?Style Library/AcisAsmHighlightVerts" );
			HC_Set_Color_By_Value( "geometry", "RGB", color.red(), color.green(), color.blue() );
			HC_Close_Segment();
		} else
		{
			HC_Open_Segment( "?Style Library/AcisAsmHighlightFacesEdges" );
			HC_Set_Color_By_Value( "geometry", "RGB", color.red(), color.green(), color.blue() );
			HC_Close_Segment();
		}

		HC_KEY keys[1000];
		int count = HA_Compute_Component_Keys( c, 1000, keys );
		int i;
		for ( i = 0; i < count; i++ )
		{
			HC_KEY comp_key = keys[i];
			if ( on )
			{
				HC_Open_Segment_By_Key( comp_key );
				HC_Set_Conditions( buffer );
				HC_Close_Segment();
			} else
			{
				HC_Open_Segment_By_Key( comp_key );
				HC_UnSet_One_Condition( buffer );
				HC_Close_Segment();
			}
		}
	}
}

#define SPAHIGHLIGHT_SEGMENT "SPAhighlightSegment"

void HA_Highlight_Component_Entity( component_entity_handle* ce, logical on, const rgb_color& color )
{
	if ( use_asm_highlight_segments.on() )
	{
		HA_Highlight_Component_Entity_old( ce, on, color );
		return;
	}
	if ( ce )
	{
		component_handle* c = ce->component();
		entity_handle* e = ce->entity();
		ENTITY* ent = e->entity_ptr();
		if ( is_FACE( ent ) || is_EDGE( ent ) || is_VERTEX( ent ) )
		{
			HC_KEY keys[1000];
			int count = HA_Compute_Geometry_Keys( ent, 1000, keys );
			int i;
			for ( i = 0; i < count; i++ )
			{
				HC_KEY key = keys[i];

				HC_KEY seg_key = HC_KShow_Owner_By_Key( key );
				if ( on )
				{
					HC_Open_Segment_By_Key( seg_key );
					{
						HC_Open_Segment( SPAHIGHLIGHT_SEGMENT );
						{
							HC_Set_User_Index( 0, (void*)seg_key );
							char buffer[64];
							HA_Build_Segment_String( ent, buffer, "entity" );
							if ( is_FACE( ent ) )
								HC_Conditional_Style( "?Style Library/AcisAsmHighlightFaces", buffer );
							else
								if ( is_EDGE( ent ) )
									HC_Conditional_Style( "?Style Library/AcisAsmHighlightEdges", buffer );
								else
									HC_Conditional_Style( "?Style Library/AcisAsmHighlightVerts", buffer );
							HC_Move_By_Key( key, "." );
						}
						HC_Close_Segment();
					}
					HC_Close_Segment();
				} else
				{
					HC_KEY orig_key;
					HC_Open_Segment_By_Key( seg_key );
					HC_Show_One_Net_User_Index( 0, &orig_key );
					HC_Close_Segment();
					if ( orig_key )
					{
						HC_Open_Segment_By_Key( orig_key );
						HC_Move_By_Key( key, "." );
						HC_Close_Segment();
					}
				}
			}
			char buffer[64];
			HA_Build_Segment_String( ent, buffer, "entity" );
			if ( is_FACE( ent ) )
				HC_Open_Segment( "?Style Library/AcisAsmHighlightFaces" );
			else
				if ( is_EDGE( ent ) )
					HC_Open_Segment( "?Style Library/AcisAsmHighlightEdges" );
				else
					HC_Open_Segment( "?Style Library/AcisAsmHighlightVerts" );
			{
				HC_Set_Color_By_Value( "geometry", "RGB", color.red(), color.green(), color.blue() );
			}
			HC_Close_Segment();
			count = HA_Compute_Component_Keys( c, 1000, keys );
			for ( i = 0; i < count; i++ )
			{
				HC_KEY comp_key = keys[i];
				if ( on )
				{
					HC_Open_Segment_By_Key( comp_key );
					HC_Set_Conditions( buffer );
					HC_Close_Segment();
				} else
				{
					HC_Open_Segment_By_Key( comp_key );
					HC_UnSet_One_Condition( buffer );
					HC_Close_Segment();
				}
			}
		}
	}
}

/**
 * Get the entities to be rendered, to be used as total number of entities in render
 * progress callback
 */
LOCAL_PROC void get_unrendered_model_entities( asm_model * model, ENTITY_LIST & entities )
{
	component_handle_list component_list;
	// Count the total # of faces - for rendering progress.
	component_handle* top_component = NULL;
	check_outcome( asmi_model_get_component_handle( model, top_component ) );
	component_list.add( top_component );
	check_outcome( asmi_component_get_sub_components( top_component, ASM_ALL, FALSE, component_list ) );
	int ncomponents = component_list.iteration_count();
	component_list.init();
	while ( component_handle* this_component = component_list.next() )
	{
		logical is_hidden = FALSE;
		check_outcome( asmi_component_is_hidden( this_component, is_hidden ) );
		if ( FALSE == is_hidden )
		{
			asm_model* this_model = this_component->get_end_model();
			ENTITY_LIST top_ents;
			this_model->get_top_level_entities( top_ents, FALSE );
			entities.add( top_ents );
		}
	}
}


extern SESSION_GLOBAL_VAR HA_RENDER_progress_info * progress_meter;
extern HA_RENDER_progress_info * make_progress_meter( const ENTITY_LIST & entitylist, logical skip_rendered_faces );
extern int HA_Close_All_Open_Segments();


LOCAL_PROC HC_KEY HA_Render_Model_Internal( asm_model*	model, const char*	pattern, logical force_rebuild )
{
	HC_KEY key = 0;
	if ( !model )
		return 0;
	char pattern_copy[1024];
	char geom_pattern_copy[1024];
	EXCEPTION_BEGIN
		progress_meter = NULL;
	EXCEPTION_TRY
	{
		// The call to get_unrendered_model_entities currently results in the
		// pattern being stomped.  Make a copy.
		logical input_pattern( TRUE );
		strcpy( pattern_copy, get_ha_state()->s_RenderingOptions.GetPattern() );
		if ( !pattern || !*pattern )
		{
			input_pattern = FALSE;
		}
		strcpy( geom_pattern_copy, get_ha_state()->s_RenderingOptions.GetGeomPattern() );
		ENTITY_LIST ents;
		model->get_top_level_entities( ents, TRUE );
		ha_rendering_options * ro = &get_ha_state()->s_RenderingOptions;
		if ( ro && ro->GetRenderFacesMode() )
		{
			ENTITY_LIST entities;
			get_unrendered_model_entities( model, entities );
			progress_meter = make_progress_meter( entities, TRUE );
		}
		HA_Map* map = 0;
		HA_ModelMap* model_map = 0;
		HA_CHandleMap* chandle_map = 0;
		if ( get_ha_state()->s_RenderingOptions.GetMappingFlag() )
		{
			map = get_ha_state()->s_pHA_MapAsm;
			model_map = get_ha_state()->s_pHA_ModelMap;
			chandle_map = get_ha_state()->s_pHA_CHandleMap;
		}
		ha_rendering_context rc;
		rc.SetEntityMap( map );
		rc.SetPattern( input_pattern ? pattern : pattern_copy );
		rc.SetModelMap( model_map );
		rc.SetGeomPattern( geom_pattern_copy );
		rc.SetCHandleMap( chandle_map );
		rc.SetModel( model );
		rc.SetForceModelRebuild( force_rebuild );
		ENTITY* entity = NULL;
		for ( entity = ents.first(); entity; entity = ents.next() )
		{
			ENTITY* owner = NULL;
			api_get_owner( entity, owner );
			if ( get_ha_state()->s_pIEntityConverter )
				key = get_ha_state()->s_pIEntityConverter->ConvertEntityAsm( owner, get_ha_state()->s_RenderingOptions, rc );
			if ( get_ha_state()->s_RenderingOptions.GetMergeFacesMode() && !get_ha_state()->s_RenderingOptions.GetMergeBodiesMode() )
			{	// merge bodies can't be done on Wire bodies 
				if ( ( entity->identity() == BODY_TYPE ) && ( is_wire_body( entity ) == FALSE ) )
					merge_body_faces( (BODY**)&entity, 1, pattern_copy );
			}
		}
		logical dump( FALSE );
		if ( dump )
		{
			FILE* old_debug_file_ptr = debug_file_ptr;
			debug_file_ptr = fopen( "C:\\hoops.dbg", "w" );
			HA_Print_Segment_Tree( "?picture", TRUE );
			HA_Print_Segment_Tree( "?include library", TRUE );
			HA_Print_Segment_Tree( "?style library", TRUE );
			fclose( debug_file_ptr );
			debug_file_ptr = old_debug_file_ptr;
		}
		if ( get_ha_state()->s_RenderingOptions.GetMappingFlag() )
			key = get_ha_state()->s_pHA_ModelMap->FindMapping( model );
	} EXCEPTION_CATCH_TRUE
	{
		if ( progress_meter )
		{
			ACIS_DELETE progress_meter;
			progress_meter = NULL;
		}
		if ( error_no != 0 )
			HA_Close_All_Open_Segments();
	} EXCEPTION_END;
	IDC_ASSERT( strcmp( pattern_copy, get_ha_state()->s_RenderingOptions.GetPattern() ) == 0 );
	IDC_ASSERT( strcmp( geom_pattern_copy, get_ha_state()->s_RenderingOptions.GetGeomPattern() ) == 0 );
	return key;
}

HC_KEY HA_Render_Model( asm_model*	model, const char*	pattern )
{
	HC_KEY answer = HA_Render_Model_Internal( model, pattern, FALSE );
	HA_ReRender_Visibility_ASM();
	return answer;
}

LOCAL_PROC HC_KEY HA_Render_Model_Geometry_Internal( asm_model*	model, logical force_rebuild )
{
	if ( !model )
		return 0;
	const char* pattern = get_ha_state()->s_RenderingOptions.GetPattern();
	const char* geom_pattern = get_ha_state()->s_RenderingOptions.GetGeomPattern();
	HA_Map* map = 0;
	HA_ModelMap* model_map = 0;
	HA_CHandleMap* chandle_map = 0;
	if ( get_ha_state()->s_RenderingOptions.GetMappingFlag() )
	{
		map = get_ha_state()->s_pHA_MapAsm;
		model_map = get_ha_state()->s_pHA_ModelMap;
		chandle_map = get_ha_state()->s_pHA_CHandleMap;
	}
	HC_KEY key = 0;
	if ( get_ha_state()->s_pIEntityConverter )
	{
		ha_rendering_context rc;
		rc.SetEntityMap( map );
		rc.SetPattern( pattern );
		rc.SetModelMap( model_map );
		rc.SetGeomPattern( geom_pattern );
		rc.SetCHandleMap( chandle_map );
		rc.SetModel( model );
		rc.SetForceModelRebuild( force_rebuild );
		char ent_geom_pattern[128];
		sprintf( ent_geom_pattern, "%s/%s", rc.GetGeomPattern(), "pointer" );
		HC_KEY model_key = KOpen_Pointer_Segment( model, ent_geom_pattern );
		{
			model_map->AddMapping( model_key, model );
			char ent_pattern[100];
			Build_Segment_String( model, ent_pattern, ent_geom_pattern );
			ha_rendering_context ent_rc( rc );
			ent_rc.SetGeomPattern( ent_pattern );
			key = get_ha_state()->s_pIEntityConverter->ConvertModelGeometry( model, get_ha_state()->s_RenderingOptions, ent_rc );
		}
		HC_Close_Segment();
	}
	if ( get_ha_state()->s_RenderingOptions.GetMergeFacesMode() && !get_ha_state()->s_RenderingOptions.GetMergeBodiesMode() )
	{
		ENTITY_LIST ents;
		model->get_top_level_entities( ents, FALSE );
		ENTITY* entity = NULL;
		for ( entity = ents.first(); entity; entity = ents.next() )
		{
			// merge bodies can't be done on Wire bodies 
			if ( ( entity->identity() == BODY_TYPE ) && ( is_wire_body( entity ) == FALSE ) )
				merge_body_faces( (BODY**)&entity, 1, pattern );
		}
	}
	if ( get_ha_state()->s_RenderingOptions.GetMappingFlag() )
		return get_ha_state()->s_pHA_ModelMap->FindMapping( model );
	return key;
}

HC_KEY HA_ReRender_Model_Geometry( asm_model* model )
{
	HA_Delete_Model_Geometry_Children_Internal( model );
	HC_KEY answer = HA_Render_Model_Geometry_Internal( model, TRUE );
	return answer;
}

HC_KEY HA_ReRender_Model_Entity( entity_handle*	ent_handle )
{
	HC_KEY answer = 0;
	if ( ent_handle && get_ha_state()->s_pIEntityConverter )
	{
		ENTITY* entity = ent_handle->entity_ptr();
		asm_model* model = ent_handle->get_owning_model();
		if ( entity && model )
		{
			const char* pattern = get_ha_state()->s_RenderingOptions.GetPattern();
			const char* geom_pattern = get_ha_state()->s_RenderingOptions.GetGeomPattern();
			HA_Map* map = 0;
			HA_ModelMap* model_map = 0;
			HA_CHandleMap* chandle_map = 0;
			if ( get_ha_state()->s_RenderingOptions.GetMappingFlag() )
			{
				map = get_ha_state()->s_pHA_MapAsm;
				model_map = get_ha_state()->s_pHA_ModelMap;
				chandle_map = get_ha_state()->s_pHA_CHandleMap;
			}
			ha_rendering_context rc;
			rc.SetEntityMap( map );
			rc.SetPattern( pattern );
			rc.SetModelMap( model_map );
			rc.SetGeomPattern( geom_pattern );
			rc.SetCHandleMap( chandle_map );
			rc.SetModel( model );
			rc.SetForceModelRebuild( TRUE );
			HA_Delete_Entity_Geometry( entity );
			HC_KEY model_key = get_ha_state()->s_pHA_ModelMap->FindMapping( model );
			if ( model_key )
			{
				HC_Open_Segment_By_Key( model_key );
				{
					char geom_pattern[128];
					HC_Show_Segment( model_key, geom_pattern );
					char ent_pattern[100];
					Build_Segment_String( model, ent_pattern, geom_pattern );
					ha_rendering_context ent_rc( rc );
					ent_rc.SetGeomPattern( ent_pattern );
					answer = get_ha_state()->s_pIEntityConverter->ConvertModelGeometry( model, get_ha_state()->s_RenderingOptions, ent_rc );
				}
				HC_Close_Segment();
			}
		}
	}
	return answer;
}

HC_KEY HA_ReRender_Model_Components( asm_model*	model )
{
	HC_KEY answer = 0;
	if ( model && get_ha_state()->s_pIEntityConverter )
	{
		entity_handle_list empty;
		component_handle* top_component = model->get_component_handle( empty );
		if ( top_component )
		{
			HA_Delete_Model_Components( model );
			HA_Render_Model( model );
			HA_ReRender_Visibility_ASM();
			answer = get_ha_state()->s_pHA_CHandleMap->FindMapping( top_component );
		}
	}
	return answer;
}

HC_KEY HA_ReRender_Component( component_handle*	comp )
{
	HC_KEY answer = 0;
	if ( comp && get_ha_state()->s_pIEntityConverter )
	{
		asm_model* model = comp->get_owning_model();
		if ( model )
		{
			HA_Delete_Model_Components( model );
			HA_Render_Model( model );
			HA_ReRender_Visibility_ASM();
			answer = get_ha_state()->s_pHA_CHandleMap->FindMapping( comp );
		}
	}
	return answer;
}

void HA_ReRender_Visibility_ASM()
{
	IEntityConverter* icvrt = HA_Get_Entity_Converter();
	if ( icvrt )
	{
		ha_rendering_options& ro = HA_Get_Rendering_Options();
		icvrt->ReRenderVisibilityAsm( ro );
	}
}

LOCAL_PROC HC_KEY HA_Find_Geomtype_Segment( const char* target_segment )
{
	HC_KEY answer( 0 );
	HC_Begin_Segment_Search( "*" );
	{
		char child_path[5120];
		char child_name[32];
		while ( HC_Find_Segment( child_path ) && answer == 0 )
		{
			HC_Parse_String( child_path, "/", -1, child_name );
			if ( STRICMP( child_name, target_segment ) == 0 )
				answer = HC_KCreate_Segment( child_path );
			else
			{
				HC_Open_Segment( child_path );
				answer = HA_Find_Geomtype_Segment( target_segment );
				HC_Close_Segment();
			}
		}
	}
	HC_End_Segment_Search();
	return answer;
}

void HA_Show_Visibility( asm_model* model, const char* geomtype, char* visibility )
{	// internal use only
	assert( model );
	HC_KEY keys[1024];
	int count = get_ha_state()->s_pHA_ModelMap->FindMapping( model, keys, 1024 );
	if ( count == 1 )
	{
		HC_Open_Segment_By_Key( keys[0] );
		{
			char target_segment[16];
			if ( STRICMP( geomtype, "faces" ) == 0 )
				strcpy( target_segment, "SPA faces" );
			else if ( STRICMP( geomtype, "edges" ) == 0 )
				strcpy( target_segment, "SPA edges" );
			else if ( STRICMP( geomtype, "vertices" ) == 0 )
				strcpy( target_segment, "SPA vertices" );
			else if ( STRICMP( geomtype, "coedges" ) == 0 )
				strcpy( target_segment, "SPA coedges" );
			else if ( STRICMP( geomtype, "tcoedges" ) == 0 )
				strcpy( target_segment, "SPA tcoedges" );
			HC_KEY geom_key = HA_Find_Geomtype_Segment( target_segment );
			char target_path[1024];
			HC_Show_Segment( geom_key, target_path );
			HC_Open_Segment_By_Key( geom_key );
			HC_Show_Net_Visibility( visibility );
			HC_Close_Segment();
		}
		HC_Close_Segment();
	} else
		visibility[0] = '\0';
}

void HA_Show_Conditions( asm_model* model, const char* geomtype, char* conditions )
{	// internal use only
	assert( model );
	HC_KEY keys[1024];
	int count = get_ha_state()->s_pHA_ModelMap->FindMapping( model, keys, 1024 );
	if ( count == 1 )
	{
		HC_Open_Segment_By_Key( keys[0] );
		{
			char target_segment[16];
			if ( STRICMP( geomtype, "faces" ) == 0 )
				strcpy( target_segment, "SPA faces" );
			else if ( STRICMP( geomtype, "edges" ) == 0 )
				strcpy( target_segment, "SPA edges" );
			else if ( STRICMP( geomtype, "vertices" ) == 0 )
				strcpy( target_segment, "SPA vertices" );
			else if ( STRICMP( geomtype, "coedges" ) == 0 )
				strcpy( target_segment, "SPA coedges" );
			else if ( STRICMP( geomtype, "tcoedges" ) == 0 )
				strcpy( target_segment, "SPA tcoedges" );
			HC_KEY geom_key = HA_Find_Geomtype_Segment( target_segment );
			char target_path[1024];
			HC_Show_Segment( geom_key, target_path );
			HC_Open_Segment_By_Key( geom_key );
			HC_Show_Net_Conditions( conditions );
			HC_Close_Segment();
		}
		HC_Close_Segment();
	} else
		conditions[0] = '\0';
}
