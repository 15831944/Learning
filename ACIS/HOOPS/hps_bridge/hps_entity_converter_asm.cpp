/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: hps_util.cpp,v 1.6 2002/07/18 22:58:35 jhauswir Exp $

#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif // NT

//	Required for all ACIS functions
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_util.h"
#include "hps_bridge.h"
#include "hps_bridge_state.hxx"
#include "acis.hxx"
#include "b_strutl.hxx"
#include "asm_model.hxx"
#include "hps_rend_options.h"
#include "hps_rend_context.h"
#include "hps_map_asm.h"
#include "hps_bridge_asm.h"
#include "hps_entity_converter.h"
#include "asm_model_entity_mgr.hxx"
#include "asm_model_ref.hxx"
#include "asm_assembly.hxx"
#include "lists.hxx"
#include "asm_api.hxx"
#include "asm_prop_api.hxx"
#include "at_comp_prop.hxx"
#include "rh_asm.hxx"
#include "rh_asm_api.hxx"
#include "getowner.hxx"
#include "get_top.hxx"
#include "entity_color.hxx"
#include "comp_handle.hxx"
#include "comp_handle_list.hxx"
#include "ckoutcom.hxx"
#include "no_rend.hxx"
#include "point.hxx"
#include "text.hxx"
#include "wcs.hxx"
#include "rlt_util.hxx"

extern component_handle_list s_HPS_CHandles;
/*
#ifdef THREAD_SAFE_ACIS
extern safe_object_pointer<component_handle_list> sp_HPS_CHandles;
#else
extern component_handle_list* sp_HPS_CHandles;
#endif
*/

#ifndef _MAX_PATH
#ifdef MAXPATHLEN
#define _MAX_PATH MAXPATHLEN
#else
#define _MAX_PATH 1024
#endif
#endif

char* Build_Segment_String( void* SPAptr, char* inbuffer, const char* pattern )
{
	if ( !pattern )
	{
		*inbuffer = 0;
		return inbuffer;
	}
	char pbuffer[POINTER_BUFFER_SIZE];
	char* buffer = inbuffer;
	unsigned int token_number = 0;
	char token[1025];
	while ( HPS_Parse_String( pattern, "/", token_number++, token ) )
	{
		if ( strcmp( token, "pointer" ) == 0 )
			strcpy( buffer, ptoax( SPAptr, pbuffer ) );
		else
			strcpy( buffer, token ); // Just accept the token as a constant
		buffer += strlen( buffer );
		*buffer++ = '/';
	}
	*--buffer = 0;	// Null terminate and eliminate trailing slash
	return inbuffer;
}

HPS::SegmentKey HPS_Open_Asm_Pointer_Segment( HPS::SegmentKey in_segment_key, void* SPAptr, const char* pattern )
{
	char buffer[1025];
	Build_Segment_String( SPAptr, buffer, pattern );
	if ( HPS_Is_Valid_Segment_Key( in_segment_key ) )
		return	HPS_Open_Segment_Key_By_Key( in_segment_key, buffer );
	else
		return HPS_KOpen_Segment( buffer );
}

logical is_ambient_entity( ENTITY* entity )
{
	logical answer = ( is_APOINT( entity ) || is_TEXT_ENT( entity ) || is_WCS( entity ) || IS_LIGHT( entity ) );
	return answer;
}

void hps_acis_entity_converter::ConvertAmbientEntity( HPS::SegmentKey CurrentSegment )
{
	fp_sentry fps;
	if ( is_APOINT( m_pEntity ) )
	{
		APOINT* pt = (APOINT*)m_pEntity;
		SPAposition pos = pt->coords();
		HPS::MarkerKey mark_key = CurrentSegment.InsertMarker( HPS::Point( (float)pos.x(), (float)pos.y(), (float)pos.z() ) );
		AddMapping( mark_key, m_pEntity );
		HPS_Style_Library.Subsegment( "AcisMarkersOn" ).SetCondition( "AcisAPOINTsOn" );
		HPS_Style_Library.Subsegment( "AcisMarkersOff" ).SetCondition( "AcisAPOINTsOff" );
	}
	if ( is_TEXT_ENT( m_pEntity ) )
	{
		TEXT_ENT *text = (TEXT_ENT *)m_pEntity;
		if ( text )
		{
			if ( strlen( text->font_name() ) )
				CurrentSegment.GetTextAttributeControl().SetFont( text->font_name() );
			if ( text->font_size() > 0 )
				CurrentSegment.GetTextAttributeControl().SetSize( (float)text->font_size(), HPS::Text::SizeUnits::Pixels );
			SPAposition pos = text->location();
			HPS::Key key = CurrentSegment.InsertText( HPS_Cast_Point( pos ), text->string() );
			AddMapping( key, m_pEntity );
			HPS_Style_Library.Subsegment( "AcisTextOn" ).SetCondition( "AcisTextOn" );
			HPS_Style_Library.Subsegment( "AcisTextOff" ).SetCondition( "AcisTextOff" );
		}
	}
	if ( is_WCS( m_pEntity ) )
	{
		WCS *wcs = (WCS *)m_pEntity;
		SPAposition Origin = wcs->origin();
		SPAunit_vector XAxis = wcs->x_axis();
		SPAunit_vector YAxis = wcs->y_axis();
		SPAunit_vector ZAxis = wcs->z_axis();
		HPS::Key keyx = CurrentSegment.InsertText( HPS_Point( SPAvector( Origin.x() + XAxis.x(), Origin.y() + XAxis.y(), Origin.z() + XAxis.z() ) ), "X" );
		HPS::Key keyy = CurrentSegment.InsertText( HPS_Point( SPAvector( Origin.x() + XAxis.x(), Origin.y() + XAxis.y(), Origin.z() + XAxis.z() ) ), "Y" );
		HPS::Key keyz = CurrentSegment.InsertText( HPS_Point( SPAvector( Origin.x() + XAxis.x(), Origin.y() + XAxis.y(), Origin.z() + XAxis.z() ) ), "Z" );
		AddMapping( keyx, m_pEntity );
		AddMapping( keyy, m_pEntity );
		AddMapping( keyz, m_pEntity );
		keyx = CurrentSegment.InsertLine( HPS_Cast_Point( Origin ), HPS_Point( SPAvector( Origin.x() + XAxis.x(), Origin.y() + XAxis.y(), Origin.z() + XAxis.z() ) ) );
		keyy = CurrentSegment.InsertLine( HPS_Cast_Point( Origin ), HPS_Point( SPAvector( Origin.x() + YAxis.x(), Origin.y() + YAxis.y(), Origin.z() + YAxis.z() ) ) );
		keyz = CurrentSegment.InsertLine( HPS_Cast_Point( Origin ), HPS_Point( SPAvector( Origin.x() + ZAxis.x(), Origin.y() + ZAxis.y(), Origin.z() + ZAxis.z() ) ) );
		AddMapping( keyx, m_pEntity );
		AddMapping( keyy, m_pEntity );
		AddMapping( keyz, m_pEntity );
		HPS_Style_Library.Subsegment( "AcisTextLinesOn" ).SetCondition( "AcisWCSsOn" );
		HPS_Style_Library.Subsegment( "AcisTextLinesOff" ).SetCondition( "AcisWCSsOff" );
	}
	if ( IS_LIGHT( m_pEntity ) )
	{
		RH_LIGHT *rh_light = (RH_LIGHT *)m_pEntity;
		if ( RHLight_IsOn( rh_light ) )
		{
			rgb_color Color;
			float fBrightness;
			SPAposition Pos;
			SPAunit_vector Dir;
			float fCone_angle;
			RHLType light_type;
			// get the light's render args defined
			light_type = RHLight_Type( rh_light );
			RHLight_Intensity( rh_light, &fBrightness );
			RHLight_Color( rh_light, &Color );
			RHLight_Location( rh_light, &Pos );
			RHLight_Direction( rh_light, &Dir );
			RHLight_ConeAngle( rh_light, &fCone_angle );
			Color = rgb_color( Color.red()*fBrightness, Color.green()*fBrightness, Color.blue()*fBrightness );
			if ( light_type == EYE_LIGHT )
			{
				HPS_Point pos( 0, 0, 0 );
				HPS_Point target( 0, 0, 0 );
				char params[2056];
				sprintf( params, "camera relative = on" );
				//HPS::Key key = HX_KInsert_Spot_Light( pos, target, params );
				//AddMapping( key, m_pEntity );
			} else if ( light_type == POINT_LIGHT )
			{
				//HPS::Key key = HX_KInsert_Local_Light( Pos.x(), Pos.y(), Pos.z() );
				//AddMapping( key, m_pEntity );
			} else if ( light_type == AMBIENT_LIGHT )
			{
				//HX_Set_Color_By_Value( "ambient", "RGB", Color.red(), Color.green(), Color.blue() );
			} else if ( light_type == SPOT_LIGHT )
			{
				HPS_Point pos( Pos );
				// Catch the special case if the dir = 0.0.  It is illegal to set a hoops spotlight
				// with the same target and position.  If we see this, we will default the dir to
				// 0.0 0.0 1.0f
				if ( Dir.len() == 0.0f )
					Dir.set_z( 1.0f );
				HPS_Point target( Pos + Dir );
				char params[2056];
				sprintf( params, "illumination cone = %f degrees", fCone_angle );
				//HPS::Key key = HX_KInsert_Spot_Light( pos, target, params );
				//AddMapping( key, m_pEntity );
			} else if ( light_type == DISTANT_LIGHT )
			{
				//HPS::Key key = HX_KInsert_Distant_Light( -Dir.x(), -Dir.y(), -Dir.z() );
				//AddMapping( key, m_pEntity );
			}
			//HX_Set_Visibility( "lights=on" );
			//HX_Set_Color_By_Value( "lights", "RGB", Color.red(), Color.green(), Color.blue() );
		} // else
		//HX_Set_Visibility( "lights=off" );
	}
}

HPS::SegmentKey hps_acis_entity_converter::ConvertEntityAsm( ENTITY * entity, const hps_rendering_options & ro, const hps_rendering_context & rc )
{	// The ENTITY that we are creating. This will most likely be a top-level ENTITY such as BODY, but could  conceivably be a lower topological
	// type as well.  However, it really should be the topmost ENTITY pointer available.
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return HPS_INVALID_SEGMENT_KEY;
	m_Map = rc.GetEntityMap();
	m_CHandleMap = rc.GetCHandleMap();
	m_ModelMap = rc.GetModelMap();
	m_RenderingOptions = ro;
	m_Pattern = rc.GetPattern();
	m_pEntity = entity;
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	HPS::SegmentKey color_seg_key = HPS_INVALID_SEGMENT_KEY;
	if ( is_ambient_entity( entity ) )
	{
		char top_seg[128];
		sprintf( top_seg, "%s/entity", m_Pattern );
		key = HPS_Open_Segment( m_pEntity, top_seg );
		AddMapping( key, m_pEntity );
		color_seg_key = HPS_OpenColorSegment( m_pEntity, key );
		ConvertAmbientEntity( color_seg_key );
	} else
	{
		asm_model* owning_model = rc.GetModel();
		component_handle* top_component = NULL;
		if ( owning_model )
			check_outcome( asmi_model_get_component_handle( owning_model, top_component ) );
		char body_seg[128];
		// The m_ModelGeometryMode flag is set by ConvertModelGeometry, and will only be set for BODYs
		if ( m_ModelGeometryMode )
		{
			sprintf( body_seg, "%s/entity", rc.GetGeomPattern() );
			key = HPS_Open_Segment( entity, body_seg );
		} else
		{
			sprintf( body_seg, "%s/pointer", rc.GetPattern() );
			key = HPS_Open_Asm_Pointer_Segment( key, top_component, body_seg );
			AddMapping( key, top_component );
			s_HPS_CHandles.add( top_component );
		}
		API_BEGIN;
		{
			if ( !m_ModelGeometryMode )
				ConvertModelComponents( top_component, ro, rc, key );
			else
			{
				AddMapping( key, entity );
				// Get the transform to apply
				m_BodyTransf = get_owner_transf( m_pEntity );
				if ( !m_BodyTransf.identity() )
				{
					float float_mat[16];
					transf_to_matrix( float_mat, m_BodyTransf );
					key.GetModellingMatrixControl().SetElements( 16, float_mat );
				}
				color_seg_key = HPS_OpenColorSegment( m_pEntity, key );
				// Convert all the faces to mesh DOs and add them to the ACIS DO.
				// Commented this out for the case of a S-A EDGE.
				ConvertLumps( color_seg_key );
			}
		} API_END;
		check_outcome( result );
	}
	return key;
}

HPS::SegmentKey hps_acis_entity_converter::ConvertModelGeometry( asm_model * model, const hps_rendering_options & ro, const hps_rendering_context & rc )
{
	HPS::Key answer = FindMapping( model );
	MODEL_BEGIN( model )
	{
		ENTITY_LIST render_ents;
		model->get_top_level_entities( render_ents, FALSE );
		if ( render_ents.iteration_count() > 0 )
		{	// Signal ConvertEntityAsm that we're rendering in "Model Geometry Mode"
			m_ModelGeometryMode = TRUE;
			render_ents.init();
			ENTITY* this_render_ent = render_ents.next();
			while ( this_render_ent )
			{	// Ambient entities do not contribute model geometry
				if ( !is_ambient_entity( this_render_ent ) )
					hps_acis_entity_converter::ConvertEntityAsm( this_render_ent, ro, rc );
				this_render_ent = render_ents.next();
			}
			m_ModelGeometryMode = FALSE;
		}
	} MODEL_END( ASM_NO_CHANGE );
	answer = FindMapping( model );
	return HPS_Cast_SegmentKey( answer );
}

LOCAL_PROC HPS::SegmentKey KCreateBodySegment( ENTITY* rend_ent )
{
	HPS::SegmentKey body_key = HPS_INVALID_SEGMENT_KEY;
	char buffer[128];
	sprintf( buffer, "entity" );
	float float_mat[16];
	logical set_transf( FALSE );
	SPAtransf BodyTransf = get_owner_transf( rend_ent );
	if ( !BodyTransf.identity() )
	{
		transf_to_matrix( float_mat, BodyTransf );
		set_transf = TRUE;
	}
	body_key = HPS_KOpen_Segment( rend_ent, buffer, HPS_Get_Current_View().GetAttachedModel().GetSegmentKey() );
	if ( set_transf )
		body_key.GetModellingMatrixControl().SetElements( 16, float_mat );
	return body_key;
}

HPS::SegmentKey hps_acis_entity_converter::ConvertModelComponents(
	component_handle *				comp,
	const hps_rendering_options &	ro,
	const hps_rendering_context &	rc,
	HPS::SegmentKey					in_segment_key )
{
	HPS::Key answer = FindMapping( comp );
	logical is_hidden( FALSE );
	check_outcome( asmi_component_is_hidden( comp, is_hidden ) );
	if ( is_hidden )
		return HPS_Cast_SegmentKey( answer );
	asm_model* top_model = comp->get_owning_model();
	asm_model* this_end_model = comp->get_end_model();
	float float_mat[16];
	SPAtransf transf;
	component_handle* parent_comp = NULL;
	outcome result = asmi_component_get_parent( comp, parent_comp );
	if ( parent_comp )
		result = asmi_component_get_relative_transform( parent_comp, comp, transf, TRUE );
	else
		result = asmi_component_get_transform( comp, transf, TRUE );
	transf_to_matrix( float_mat, transf );
	in_segment_key.GetModellingMatrixControl().SetElements( 16, float_mat );
	// Recurse to subcomponents, if any
	component_handle_list subcomps;
	result = asmi_component_get_sub_components( comp, ASM_IMMEDIATE, subcomps );
	check_outcome( result );
	for ( component_handle* subcomp = subcomps.first(); subcomp; subcomp = subcomps.next() )
	{
		char* subcomp_pattern = "pointer";
		HPS::SegmentKey comp_key = HPS_Open_Asm_Pointer_Segment( in_segment_key, subcomp, subcomp_pattern );
		AddMapping( comp_key, subcomp );
		s_HPS_CHandles.add( subcomp );
		ConvertModelComponents( subcomp, ro, rc, comp_key );
	}
	// Have we rendered the model geometry yet?
	HPS::SegmentKey model_key = HPS_Cast_SegmentKey( FindMapping( this_end_model ) );
	if ( HPS_Is_Valid_Key( model_key ) && rc.GetForceModelRebuild() )
	{
		HPS_Delete_Model_Geometry( this_end_model );
		model_key = HPS_INVALID_SEGMENT_KEY;
	}
	if ( !HPS_Is_Valid_Key( model_key ) )
	{
		char geom_pattern[128];
		sprintf( geom_pattern, "%s/%s", rc.GetGeomPattern(), "pointer" );
		model_key = HPS_Open_Asm_Pointer_Segment( in_segment_key, this_end_model, geom_pattern );
		{
			AddMapping( model_key, this_end_model );
			char ent_pattern[100];
			Build_Segment_String( this_end_model, ent_pattern, geom_pattern );
			hps_rendering_context ent_rc( rc );
			ent_rc.SetGeomPattern( ent_pattern );
			ConvertModelGeometry( this_end_model, ro, ent_rc );
		}
	}
	if ( HPS_Is_Valid_Key( model_key ) )
	{
		// Look for component overrides, and handle them using conditional
		// styles and attribute locks
		logical has_override_color( FALSE );
		ATTRIB_COMPONENT_PROP_OWNER* comp_color_owner = NULL;
		rgb_color comp_color = get_comp_color( comp, comp_color_owner );
		HPS::RGBColor hps_color( (float)comp_color.red(), (float)comp_color.green(), (float)comp_color.blue() );
		has_override_color = ( comp_color_owner != NULL );
		double transparency( 1.0 );
		asmi_component_find_transparency( comp, transparency );
		logical has_transparency = ( transparency < 1.0 );
		char handle[64];
		Build_Segment_String( comp, handle, "pointer" );
		char stylebuffer[128];
		sprintf( stylebuffer, "?Style Library/ACIS styles/%s", handle );
		if ( has_override_color || has_transparency )
		{
			HPS::SegmentKey style_seg_key = HPS_Open_Segment( stylebuffer );
			{
				if ( has_override_color && has_transparency )
				{
					style_seg_key.GetMaterialMappingControl().SetFaceColor( hps_color );
					style_seg_key.GetMaterialMappingControl().SetFaceAlpha( 1 - (float)transparency );
					//HX_Set_Rendering_Options( "attribute lock = (color = (geometry = diffuse, faces = transmission))" );
				} else if ( has_transparency )
				{
					style_seg_key.GetMaterialMappingControl().SetFaceAlpha( 1 - (float)transparency );
					//HX_Set_Rendering_Options( "attribute lock = (color = (faces = transmission))" );
				} else if ( has_override_color )
				{
					style_seg_key.GetMaterialMappingControl().SetFaceColor( hps_color );
					//HX_Set_Rendering_Options( "attribute lock = (color = (geometry = diffuse))" );
				}
			}
			style_seg_key.SetCondition( handle );
			model_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( stylebuffer ).SetCondition( handle ) );
		} else
		{
			// In case we're refreshing the segments, we need to make sure
			// that colors and transparencies get cleared if appropriate
			//if ( HX_QShow_Existence( stylebuffer, "self" ) )
			//	HX_Delete_Segment( stylebuffer );
			in_segment_key.GetConditionControl().UnsetCondition( handle );
		}
		// Include the corresponding model segment
		HPS_Include_Segment_Key_By_Key( model_key, in_segment_key );
	}
	return HPS_Cast_SegmentKey( answer );
}

void hps_acis_entity_converter::AddMapping( HPS::Key key, component_handle * comp )
{
	if ( m_CHandleMap && m_EnableMapping )
		m_CHandleMap->AddMapping( key, comp );
}

void hps_acis_entity_converter::AddMapping( HPS::Key key, asm_model * model )
{
	if ( m_ModelMap && m_EnableMapping )
		m_ModelMap->AddMapping( key, model );
}

HPS::Key hps_acis_entity_converter::FindMapping( asm_model * model )
{
	HPS::Key key = HPS_INVALID_KEY;
	if ( m_ModelMap && m_EnableMapping )
		key = m_ModelMap->FindMapping( model );
	return key;
}

HPS::Key hps_acis_entity_converter::FindMapping( component_handle * comp )
{
	HPS::Key key = HPS_INVALID_KEY;
	if ( m_CHandleMap && m_EnableMapping )
		key = m_CHandleMap->FindMapping( comp );
	return key;
}
