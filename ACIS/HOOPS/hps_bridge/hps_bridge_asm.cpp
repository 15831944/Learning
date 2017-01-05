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
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_util.h"
#include "hps_bridge.h"

//-----------------------------------------------------------//
// ACIS-specific Header File(s)
//-----------------------------------------------------------//

#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#define strdup _strdup
#endif // NT

#include "ckoutcom.hxx"
#include "hps_bridge_asm.h"
#include "hps_map.h"
#include "hps_map_asm.h"
#include "hps_rend_options.h"
#include "hps_rend_context.h"
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
#include "hps_bridge_state.hxx"
#include "hps_ientityconverter.h"
//#include "base_thread_ctx.hxx"
#include "b_strutl.hxx"
#include "fct_utl.hxx"
#include "api.err"
#include "hps_bridge.err"
component_handle_list s_HPS_CHandles;

extern void merge_body_faces( BODY ** bodies_to_merge, unsigned long num_bodies );
extern HPS::SegmentKey HPS_Open_Asm_Pointer_Segment( HPS::SegmentKey in_segment_key, void * SPAptr, const char * pattern );
extern char* Build_Segment_String( void * SPAptr, char * inbuffer, const char * pattern );

unsigned long HPS_Compute_Model_Key_Count( asm_model* model )
{
	return get_hps_state()->s_pHPS_ModelMap->FindNumMappings( model );
}

unsigned long HPS_Compute_Model_Keys( asm_model* model, unsigned long count, HPS::SegmentKey* keys )
{	// there may be more than one hoops key mapped to a particular model.
	// so use the findmapping function which can return more than one key.
	// the last arg ensures that we don't stomp past the end of the array. rlw
	return get_hps_state()->s_pHPS_ModelMap->FindMapping( model, keys, count );
}

asm_model* HPS_Compute_Model_Pointer( HPS::SegmentKey key )
{
	asm_model* model = 0;
	model = get_hps_state()->s_pHPS_ModelMap->FindMapping( key );
	return model;
}

void HPS_Delete_Model_Ref( ASM_MODEL_REF * mref )
{
	if ( mref )
	{ 		// Find the handle to the corresponding component
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
		HPS::SegmentKey keys[1024];
		component_handle* hsubcomp;
		for ( hsubcomp = subcomps.first(); hsubcomp; hsubcomp = subcomps.next() )
		{
			unsigned long count = get_hps_state()->s_pHPS_CHandleMap->FindMapping( hsubcomp, keys, 1024 );
			unsigned long i;
			for ( i = 0; i < count; i++ )
			{
				HPS::SegmentKey comp_key = keys[i];
				if ( HPS_Is_Valid_Key( comp_key ) )
					get_hps_state()->s_pHPS_CHandleMap->DeleteMapping( comp_key, hsubcomp );
			}
			if ( s_HPS_CHandles.iteration_count() )
				s_HPS_CHandles.remove( hsubcomp );
		}
		// Unmap the component and delete its segments (and subsegments)
		unsigned long count = get_hps_state()->s_pHPS_CHandleMap->FindMapping( hcomp, keys, 1024 );
		unsigned long i;
		for ( i = 0; i < count; i++ )
		{
			HPS::SegmentKey comp_key = keys[i];
			if ( HPS_Is_Valid_Key( comp_key ) )
			{
				get_hps_state()->s_pHPS_CHandleMap->DeleteMapping( comp_key, hcomp );
				if ( HPS_Is_Valid_Segment_Key( comp_key ) )
					HPS_Delete_By_Key( comp_key );
			}
		}
		if ( s_HPS_CHandles.iteration_count() )
			s_HPS_CHandles.remove( hcomp );
	}
}

LOCAL_PROC void HPS_Unmap_Component_Segments( HPS::SegmentKey in_segment_key,  const char * seg )
{
	HPS::SegmentKey key = HPS_Open_Segment( seg, in_segment_key );
	if ( HPS_Is_Valid_Segment_Key( key ) )
	{
		HPS::SearchResults searchResults;
		key.Find( HPS::Search::Type::Include, HPS::Search::Space::Subsegments, searchResults );
		HPS::SearchResultsIterator it = searchResults.GetIterator();
		while ( it.IsValid() )
		{
			HPS::Key it_key = it.GetItem();
			HPS::IncludeKey inc_key = ( HPS::IncludeKey )it_key;
			HPS::SegmentKey seg_key = inc_key.GetTarget();
			const char* child_seg = seg_key.Name();
			char comp_ptr[32];
			HPS_Parse_String( child_seg, "/", -1, comp_ptr );
			component_handle* comp = (component_handle*)axtop( comp_ptr );
			HPS::SegmentKey child_key = seg_key.Subsegment( child_seg, false );
			if ( HPS_Is_Valid_Segment_Key( child_key ) )
				get_hps_state()->s_pHPS_CHandleMap->DeleteMapping( child_key, comp );
			if ( s_HPS_CHandles.iteration_count() )
				s_HPS_CHandles.remove( comp );
			HPS_Unmap_Component_Segments( in_segment_key, child_seg );
			it.Next();
		}
	}
}

LOCAL_PROC void HPS_Delete_Model_Components( asm_model * model )
{	//	if (model && sp_HPS_CHandles && sp_HPS_CHandles->iteration_count())
	if ( model && s_HPS_CHandles.iteration_count() )
	{
		component_handle* top_component = NULL;
		check_outcome( asmi_model_get_component_handle( model, top_component ) );
		if ( top_component )
		{
			HPS::SegmentKey key( get_hps_state()->s_pHPS_CHandleMap->FindMapping( top_component ) );
			if ( HPS_Is_Valid_Segment_Key( key ) )
			{
				HPS::UTF8 seg = key.Name();
				HPS_Unmap_Component_Segments( key, seg );
				get_hps_state()->s_pHPS_CHandleMap->DeleteMapping( key, top_component );
				if ( s_HPS_CHandles.iteration_count() )
					s_HPS_CHandles.remove( top_component );
				if ( HPS_Is_Valid_Key( key ) )
					HPS_Delete_By_Key( key );
			}
		}
	}
}

void HPS_Delete_Model_Geometry( asm_model * model )
{
	if ( !model )
		return;
	HPS::SegmentKey keys[1024];
	unsigned long count = get_hps_state()->s_pHPS_ModelMap->FindMapping( model, keys, 1024 );
	ENTITY_LIST render_ents;
	model->get_top_level_entities( render_ents, FALSE );
	HPS_Delete_Entity_Geometry( render_ents );
	unsigned long i;
	for ( i = 0; i < count; i++ )
	{
		HPS::SegmentKey key = keys[i];
		if ( HPS_Is_Valid_Key( key ) )
		{
			get_hps_state()->s_pHPS_ModelMap->DeleteMapping( key, model );
			if ( HPS_Is_Valid_Segment_Key( key ) )
				HPS_Delete_By_Key( key );
		}
	}
}

LOCAL_PROC void HPS_Delete_Model_Geometry_Children_Internal( asm_model * model )
{
	if ( model )
	{
		HPS::SegmentKey keys[1024];
		unsigned long count = get_hps_state()->s_pHPS_ModelMap->FindMapping( model, keys, 1024 );
		ENTITY_LIST render_ents;
		model->get_top_level_entities( render_ents, FALSE );
		HPS_Delete_Entity_Geometry( render_ents );
		// (not sure the following is needed, as I think the HPS_Delete_Entity_Geometry call effectively gets rid of all child segments)
		unsigned long i;
		for ( i = 0; i < count; i++ )
		{
			HPS::SegmentKey key = keys[i];
			if ( HPS_Is_Valid_Key( key ) )
			{
				if ( HPS_Is_Valid_Segment_Key( key ) )
				{
					HPS::SearchResults searchResults;
					key.Find( HPS::Search::Type::Include, HPS::Search::Space::Subsegments, searchResults );
					HPS::SearchResultsIterator it = searchResults.GetIterator();
					while ( it.IsValid() )
					{
						HPS::Key it_key = it.GetItem();
						it.Next();
					}
				}
			}
		}
	}
}

void HPS_Delete_Model( asm_model * model )
{
	HPS_Delete_Model_Components( model );
	HPS_Delete_Model_Geometry( model );
}

logical HPS_Associate_Key_To_Model( asm_model * model, HPS::SegmentKey key )
{
	assert( model );
	HPS::SegmentKey keys[1024];
	int count = get_hps_state()->s_pHPS_ModelMap->FindMapping( model, keys, 1024 );
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
		get_hps_state()->s_pHPS_ModelMap->AddMapping( key, model );
		return TRUE;
	}
	return TRUE;
}

logical HPS_Disassociate_Key_From_Model( asm_model * model, HPS::SegmentKey key )
{
	if ( !model )
		model = get_hps_state()->s_pHPS_ModelMap->FindMapping( key );
	if ( model )
	{
		get_hps_state()->s_pHPS_ModelMap->DeleteMapping( key, model );
		return TRUE;
	}
	return FALSE;
}

unsigned long HPS_Compute_Component_Key_Count( component_handle * comp )
{
	HPS::SegmentKey	keys[40000];
	return get_hps_state()->s_pHPS_CHandleMap->FindMapping( comp, keys, 40000 );
}

unsigned long HPS_Compute_Component_Keys( component_handle* comp, unsigned long count, HPS::SegmentKey* keys )
{	// there may be more than one hoops key mapped to a particular component.
	// so use the findmapping function which can return more than one key.
	// the last arg ensures that we don't stomp past the end of the array. rlw
	return get_hps_state()->s_pHPS_CHandleMap->FindMapping( comp, keys, count );
}

component_handle * HPS_Compute_Component_Pointer( HPS::SegmentKey key )
{
	component_handle* comp = 0;
	comp = get_hps_state()->s_pHPS_CHandleMap->FindMapping( key );
	return comp;
}

logical HPS_Associate_Key_To_Component( component_handle * comp, HPS::SegmentKey key )
{
	assert( comp );
	HPS::SegmentKey keys[1024];
	int count = get_hps_state()->s_pHPS_CHandleMap->FindMapping( comp, keys, 1024 );
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
		get_hps_state()->s_pHPS_CHandleMap->AddMapping( key, comp );
		return TRUE;
	}
	return TRUE;
}

logical HPS_Disassociate_Key_From_Component( component_handle* comp, HPS::SegmentKey key )
{
	if ( !comp )
		comp = get_hps_state()->s_pHPS_CHandleMap->FindMapping( key );
	if ( comp )
	{
		get_hps_state()->s_pHPS_CHandleMap->DeleteMapping( key, comp );
		return TRUE;
	}
	return FALSE;
}

component_entity_handle * HPS_Compute_Component_Entity_Pointer( unsigned long count, HPS::SegmentKey * keys )
{
	component_entity_handle* answer = NULL;
	if ( count > 0 && keys != NULL )
	{
		ENTITY* entity = NULL;
		component_handle* comp = NULL;
		unsigned long i;
		for ( i = 0; i < count && comp == NULL; i++ )
		{
			HPS::SegmentKey key = keys[i];
			comp = get_hps_state()->s_pHPS_CHandleMap->FindMapping( key );
		}
		if ( comp )
		{
			for ( i = count - 1; i != -1 && entity == NULL; i-- )
			{
				HPS::SegmentKey key = keys[i];
				entity = get_hps_state()->s_pHPS_MapAsm->FindMapping( key );
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

component_entity_handle * HPS_Compute_Component_Entity_Pointer( HPS::SegmentKey comp_key, HPS::SegmentKey ent_key )
{
	component_entity_handle* answer = NULL;
	if ( HPS_Is_Valid_Key( comp_key ) && HPS_Is_Valid_Key( ent_key ) )
	{
		component_handle* comp = get_hps_state()->s_pHPS_CHandleMap->FindMapping( comp_key );
		if ( comp )
		{
			ENTITY* entity = get_hps_state()->s_pHPS_MapAsm->FindMapping( ent_key );
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

logical HPS_Compute_Component_Entity_Keys( component_entity_handle * comp_ent, HPS::SegmentKey & comp_key, HPS::SegmentKey & ent_key )
{
	logical answer( FALSE );
	if ( comp_ent )
	{
		component_handle* comp = comp_ent->component();
		entity_handle* ent_handle = comp_ent->entity();
		if ( comp && ent_handle )
		{
			ENTITY* ent = ent_handle->entity_ptr();
			comp_key = HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_CHandleMap->FindMapping( comp ) );
			HPS::KeyArray key_array;
			unsigned int ent_count = get_hps_state()->s_pHPS_MapAsm->FindMapping( ent, key_array );
			if ( ent_count > 0 )
			{
				ent_key = HPS_Cast_SegmentKey( key_array[0] );
				answer = ( HPS_Is_Valid_Key( comp_key ) && HPS_Is_Valid_Key( ent_key ) );
			}
		}
	}
	return answer;
}

extern SESSION_GLOBAL_VAR option_header use_asm_highlight_segments;

void HPS_Highlight_Component_Entity_old( component_entity_handle * ce, logical on, const rgb_color & color )
{
	if ( ce )
	{
		component_handle* c = ce->component();
		entity_handle* e = ce->entity();
		ENTITY* ent = e->entity_ptr();
		char buffer[64];
		HPS_Build_Segment_String( ent, buffer, "entity" );
		if ( is_VERTEX( ent ) )
		{
			HPS::SegmentKey key = HPS_Open_Segment( "?Style Library/AcisAsmHighlightVerts" );
			HPS::RGBColor hps_color( (float)color.red(), (float)color.green(), (float)color.blue() );
			key.GetMaterialMappingControl().SetVertexColor( hps_color ).SetMarkerColor( hps_color );
		} else
		{
			HPS::SegmentKey key = HPS_Open_Segment( "?Style Library/AcisAsmHighlightVerts" );
			HPS::RGBColor hps_color( (float)color.red(), (float)color.green(), (float)color.blue() );
			key.GetMaterialMappingControl().SetFaceColor( hps_color ).SetLineColor( hps_color );
		}
		HPS::SegmentKey keys[1000];
		int count = HPS_Compute_Component_Keys( c, 1000, keys );
		int i;
		for ( i = 0; i < count; i++ )
		{
			HPS::SegmentKey comp_key = keys[i];
			if ( on )
				comp_key.GetConditionControl().SetCondition( buffer );
			else
				comp_key.GetConditionControl().UnsetCondition( buffer );
		}
	}
}

#define SPAHIGHLIGHT_SEGMENT "SPAhighlightSegment"

void HPS_Highlight_Component_Entity( component_entity_handle * ce, logical on, const rgb_color & color )
{
	if ( use_asm_highlight_segments.on() )
	{
		HPS_Highlight_Component_Entity_old( ce, on, color );
		return;
	}
	if ( ce )
	{
		component_handle* c = ce->component();
		entity_handle* e = ce->entity();
		ENTITY* ent = e->entity_ptr();
		if ( is_FACE( ent ) || is_EDGE( ent ) || is_VERTEX( ent ) )
		{
			HPS::RGBColor hps_color( (float)color.red(), (float)color.green(), (float)color.blue() );
			char buffer[64];
			HPS_Build_Segment_String( ent, buffer, "entity" );
			if ( HPS_Is_Valid_Segment_Key( HPS_Style_Library ) )
			{
				if ( is_FACE( ent ) )
					HPS_Style_Library.Subsegment( "AcisAsmHighlightFaces" ).GetMaterialMappingControl().SetFaceColor( hps_color );
				else if ( is_EDGE( ent ) )
					HPS_Style_Library.Subsegment( "AcisAsmHighlightEdges" ).GetMaterialMappingControl().SetLineColor( hps_color ).SetEdgeColor( hps_color );
				else
					HPS_Style_Library.Subsegment( "AcisAsmHighlightVerts" ).GetMaterialMappingControl().SetVertexColor( hps_color ).SetMarkerColor( hps_color );
			}
			HPS::KeyArray key_array;
			int count = HPS_Compute_Geometry_Keys( ent, key_array );
			int i;
			for ( i = 0; i < count; i++ )
			{
				HPS::SegmentKey comp_key = HPS_Cast_SegmentKey( key_array[i] );
				if ( on )
					comp_key.SetCondition( buffer );
				else
					comp_key.GetConditionControl().UnsetCondition( buffer );
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
extern int HPS_Close_All_Open_Segments();


LOCAL_PROC HPS::SegmentKey HPS_Render_Model_Internal( asm_model * model, const char * pattern, logical force_rebuild )
{
	if ( pattern && ( pattern[0] == '?' || pattern[0] == '/' ) )
		pattern++;
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	if ( !model )
		return key;
	char pattern_copy[1024];
	char geom_pattern_copy[1024];
	EXCEPTION_BEGIN
		progress_meter = NULL;
	EXCEPTION_TRY
	{	// The call to get_unrendered_model_entities currently results in the
		// pattern being stomped.  Make a copy.
		logical input_pattern( TRUE );
		strcpy( pattern_copy, get_hps_state()->s_RenderingOptions.GetPattern() );
		if ( !pattern || !*pattern )
		{
			input_pattern = FALSE;
		}
		strcpy( geom_pattern_copy, get_hps_state()->s_RenderingOptions.GetGeomPattern() );
		ENTITY_LIST ents;
		model->get_top_level_entities( ents, TRUE );
		hps_rendering_options * ro = &get_hps_state()->s_RenderingOptions;
		if ( ro && ro->GetRenderFacesMode() )
		{
			ENTITY_LIST entities;
			get_unrendered_model_entities( model, entities );
			progress_meter = make_progress_meter( entities, TRUE );
		}
		HPS_Map* map = 0;
		HPS_ModelMap* model_map = 0;
		HPS_CHandleMap* chandle_map = 0;
		if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
		{
			map = get_hps_state()->s_pHPS_MapAsm;
			model_map = get_hps_state()->s_pHPS_ModelMap;
			chandle_map = get_hps_state()->s_pHPS_CHandleMap;
		}
		hps_rendering_context rc;
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
			if ( get_hps_state()->s_pIEntityConverter )
				key = get_hps_state()->s_pIEntityConverter->ConvertEntityAsm( owner, get_hps_state()->s_RenderingOptions, rc );
			if ( get_hps_state()->s_RenderingOptions.GetMergeFacesMode() && !get_hps_state()->s_RenderingOptions.GetMergeBodiesMode() )
			{	// merge bodies can't be done on Wire bodies 
				if ( ( entity->identity() == BODY_TYPE ) && ( is_wire_body( entity ) == FALSE ) )
					merge_body_faces( (BODY**)&entity, 1 );
			}
		}
		logical dump( FALSE );
		if ( dump )
		{
			FILE* old_debug_file_ptr = debug_file_ptr;
			debug_file_ptr = fopen( "C:\\hoops.dbg", "w" );
			HPS_Print_Segment_Tree( "?picture" );
			HPS_Print_Segment_Tree( "?include library" );
			HPS_Print_Segment_Tree( "?style library" );
			fclose( debug_file_ptr );
			debug_file_ptr = old_debug_file_ptr;
		}
		if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
			key = HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_ModelMap->FindMapping( model ) );
	} EXCEPTION_CATCH_TRUE
	{
		if ( progress_meter )
		{
			ACIS_DELETE progress_meter;
			progress_meter = NULL;
		}
		if ( error_no != 0 )
			HPS_Close_All_Open_Segments();
	} EXCEPTION_END;
	IDC_ASSERT( strcmp( pattern_copy, get_hps_state()->s_RenderingOptions.GetPattern() ) == 0 );
	IDC_ASSERT( strcmp( geom_pattern_copy, get_hps_state()->s_RenderingOptions.GetGeomPattern() ) == 0 );
	return key;
}

HPS::SegmentKey HPS_Render_Model( asm_model * model, const char * pattern )
{
	HPS::SegmentKey answer = HPS_Render_Model_Internal( model, pattern, FALSE );
	HPS_ReRender_Visibility_ASM();
	return answer;
}

LOCAL_PROC HPS::SegmentKey HPS_Render_Model_Geometry_Internal( asm_model * model, logical force_rebuild )
{
	if ( !model )
		return HPS_INVALID_SEGMENT_KEY;
	const char* pattern = get_hps_state()->s_RenderingOptions.GetPattern();
	const char* geom_pattern = get_hps_state()->s_RenderingOptions.GetGeomPattern();
	HPS_Map* map = 0;
	HPS_ModelMap* model_map = 0;
	HPS_CHandleMap* chandle_map = 0;
	if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
	{
		map = get_hps_state()->s_pHPS_MapAsm;
		model_map = get_hps_state()->s_pHPS_ModelMap;
		chandle_map = get_hps_state()->s_pHPS_CHandleMap;
	}
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	if ( get_hps_state()->s_pIEntityConverter )
	{
		hps_rendering_context rc;
		rc.SetEntityMap( map );
		rc.SetPattern( pattern );
		rc.SetModelMap( model_map );
		rc.SetGeomPattern( geom_pattern );
		rc.SetCHandleMap( chandle_map );
		rc.SetModel( model );
		rc.SetForceModelRebuild( force_rebuild );
		char ent_geom_pattern[128];
		sprintf( ent_geom_pattern, "%s/%s", rc.GetGeomPattern(), "pointer" );
		HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
		HPS::SegmentKey model_key = HPS_Open_Asm_Pointer_Segment( key, model, ent_geom_pattern );
		{
			model_map->AddMapping( model_key, model );
			char ent_pattern[100];
			Build_Segment_String( model, ent_pattern, ent_geom_pattern );
			hps_rendering_context ent_rc( rc );
			ent_rc.SetGeomPattern( ent_pattern );
			key = get_hps_state()->s_pIEntityConverter->ConvertModelGeometry( model, get_hps_state()->s_RenderingOptions, ent_rc );
		}
	}
	if ( get_hps_state()->s_RenderingOptions.GetMergeFacesMode() && !get_hps_state()->s_RenderingOptions.GetMergeBodiesMode() )
	{
		ENTITY_LIST ents;
		model->get_top_level_entities( ents, FALSE );
		ENTITY* entity = NULL;
		for ( entity = ents.first(); entity; entity = ents.next() )
		{
			// merge bodies can't be done on Wire bodies 
			if ( ( entity->identity() == BODY_TYPE ) && ( is_wire_body( entity ) == FALSE ) )
				merge_body_faces( (BODY**)&entity, 1 );
		}
	}
	if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
		return HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_ModelMap->FindMapping( model ) );
	return key;
}

HPS::SegmentKey HPS_ReRender_Model_Geometry( asm_model* model )
{
	HPS_Delete_Model_Geometry_Children_Internal( model );
	HPS::SegmentKey answer = HPS_Render_Model_Geometry_Internal( model, TRUE );
	return answer;
}

HPS::SegmentKey HPS_ReRender_Model_Entity( entity_handle * ent_handle )
{
	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;
	if ( ent_handle && get_hps_state()->s_pIEntityConverter )
	{
		ENTITY* entity = ent_handle->entity_ptr();
		asm_model* model = ent_handle->get_owning_model();
		if ( entity && model )
		{
			const char* pattern = get_hps_state()->s_RenderingOptions.GetPattern();
			const char* geom_pattern = get_hps_state()->s_RenderingOptions.GetGeomPattern();
			HPS_Map* map = 0;
			HPS_ModelMap* model_map = 0;
			HPS_CHandleMap* chandle_map = 0;
			if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
			{
				map = get_hps_state()->s_pHPS_MapAsm;
				model_map = get_hps_state()->s_pHPS_ModelMap;
				chandle_map = get_hps_state()->s_pHPS_CHandleMap;
			}
			hps_rendering_context rc;
			rc.SetEntityMap( map );
			rc.SetPattern( pattern );
			rc.SetModelMap( model_map );
			rc.SetGeomPattern( geom_pattern );
			rc.SetCHandleMap( chandle_map );
			rc.SetModel( model );
			rc.SetForceModelRebuild( TRUE );
			HPS_Delete_Entity_Geometry( entity );
			HPS::SegmentKey model_key = HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_ModelMap->FindMapping( model ) );
			if ( HPS_Is_Valid_Key( model_key ) )
			{
				{
					HPS::UTF8 geom_pattern = model_key.Name();
					char ent_pattern[100];
					Build_Segment_String( model, ent_pattern, geom_pattern );
					hps_rendering_context ent_rc( rc );
					ent_rc.SetGeomPattern( ent_pattern );
					answer = get_hps_state()->s_pIEntityConverter->ConvertModelGeometry( model, get_hps_state()->s_RenderingOptions, ent_rc );
				}
			}
		}
	}
	return answer;
}

HPS::SegmentKey HPS_ReRender_Model_Components( asm_model * model )
{
	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;
	if ( model && get_hps_state()->s_pIEntityConverter )
	{
		entity_handle_list empty;
		component_handle* top_component = model->get_component_handle( empty );
		if ( top_component )
		{
			HPS_Delete_Model_Components( model );
			HPS::SegmentKey key = HPS_Render_Model( model );
			HPS_ReRender_Visibility_ASM();
			answer = HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_CHandleMap->FindMapping( top_component ) );
		}
	}
	return answer;
}

HPS::SegmentKey HPS_ReRender_Component( component_handle * comp )
{
	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;
	if ( comp && get_hps_state()->s_pIEntityConverter )
	{
		asm_model* model = comp->get_owning_model();
		if ( model )
		{
			HPS_Delete_Model_Components( model );
			HPS_Render_Model( model );
			HPS_ReRender_Visibility_ASM();
			answer = HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_CHandleMap->FindMapping( comp ) );
		}
	}
	return answer;
}

void HPS_ReRender_Visibility_ASM()
{
	IEntityConverter* icvrt = HPS_Get_Entity_Converter();
	if ( icvrt )
	{
		hps_rendering_options& ro = HPS_Get_Rendering_Options();
		icvrt->ReRenderVisibilityAsm( ro );
	}
}

LOCAL_PROC HPS::SegmentKey HPS_Find_Geomtype_Segment( const char * target_segment )
{
	HPS::SegmentKey answer( HPS_INVALID_SEGMENT_KEY );
	{
		char child_path[5120];
		char child_name[32];
		{
			HPS_Parse_String( child_path, "/", -1, child_name );
			if ( STRICMP( child_name, target_segment ) == 0 )
				answer = HPS_Open_Segment( child_path );
			else
			{
				HPS_Open_Segment( child_path );
				answer = HPS_Find_Geomtype_Segment( target_segment );
			}
		}
	}
	return answer;
}

void HPS_Show_Visibility( asm_model* model, const char* geomtype, char* visibility )
{	// internal use only
	assert( model );
	HPS::SegmentKey keys[1024];
	int count = get_hps_state()->s_pHPS_ModelMap->FindMapping( model, keys, 1024 );
	bool geom_visible = false;
	if ( count == 1 )
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
		HPS::SegmentKey geom_key = HPS_Find_Geomtype_Segment( target_segment );
		geom_key.GetVisibilityControl().ShowFaces( geom_visible );
	}
	if ( geom_visible )
		strcpy( visibility, "on" );
	else
		strcpy( visibility, "off" );

}

void HPS_Show_Conditions( asm_model* model, const char* geomtype, char* conditions )
{	// internal use only
	assert( model );
	HPS::SegmentKey keys[1024];
	int count = get_hps_state()->s_pHPS_ModelMap->FindMapping( model, keys, 1024 );
	if ( count == 1 )
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
		HPS::SegmentKey geom_key = HPS_Find_Geomtype_Segment( target_segment );
//			char target_path[1024];
//			HX_Show_Segment( geom_key, target_path );
//			HX_Show_Net_Conditions( conditions );
	} else
		conditions[0] = '\0';
}
