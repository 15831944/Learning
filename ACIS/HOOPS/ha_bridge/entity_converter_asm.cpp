/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: ha_util.cpp,v 1.6 2002/07/18 22:58:35 jhauswir Exp $

#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif // NT

//	Required for all ACIS functions
#include "acis.hxx"
#include "hc.h"
#include "b_strutl.hxx"
#include "asm_model.hxx"
#include "ha_rend_options.h"
#include "ha_rend_context.h"
#include "ha_map_asm.h"
#include "ha_bridge_asm.h"
#include "entity_converter.h"
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

extern component_handle_list s_HA_CHandles;
/*
#ifdef THREAD_SAFE_ACIS
extern safe_object_pointer<component_handle_list> sp_HA_CHandles;
#else
extern component_handle_list* sp_HA_CHandles;
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
	while ( HC_Parse_String( pattern, "/", token_number++, token ) )
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

HC_KEY KOpen_Pointer_Segment( void* SPAptr, const char* pattern )
{
	char buffer[1025];
	Build_Segment_String( SPAptr, buffer, pattern );
	return HC_KOpen_Segment( buffer );
}

logical is_ambient_entity( ENTITY* entity )
{
	logical answer = ( is_APOINT( entity ) || is_TEXT_ENT( entity ) || is_WCS( entity ) || IS_LIGHT( entity ) );
	return answer;
}

void hoops_acis_entity_converter::ConvertAmbientEntity()
{
	fp_sentry fps;
	if ( is_APOINT( m_pEntity ) )
	{
		APOINT* pt = (APOINT*)m_pEntity;
		SPAposition pos = pt->coords();
		HC_KEY key = HC_KInsert_Marker( pos.x(), pos.y(), pos.z() );
		AddMapping( key, m_pEntity );
		HC_Conditional_Style( "?Style Library/AcisMarkersOn", "AcisAPOINTsOn" );
		HC_Conditional_Style( "?Style Library/AcisMarkersOff", "AcisAPOINTsOff" );
	}
	if ( is_TEXT_ENT( m_pEntity ) )
	{
		TEXT_ENT *text = (TEXT_ENT *)m_pEntity;
		if ( text )
		{
			SPAposition pos = text->location();
			char font[2056];
			const char *font_name = text->font_name();
			if ( font_name && strlen( font_name ) )
			{
				sprintf( font, "name = %s", font_name );
				HC_Set_Text_Font( font );
			}
			int size = text->font_size();
			if ( size > 0 )
			{
				sprintf( font, "size = %d pixels", size );
				HC_Set_Text_Font( font );
			}
			HC_KEY key = HC_KInsert_Text( pos.x(), pos.y(), pos.z(), text->string() );
			AddMapping( key, m_pEntity );
			HC_Conditional_Style( "?Style Library/AcisTextOn", "AcisTextOn" );
			HC_Conditional_Style( "?Style Library/AcisTextOff", "AcisTextOff" );
		}
	}
	if ( is_WCS( m_pEntity ) )
	{
		WCS *wcs = (WCS *)m_pEntity;
		SPAposition Origin = wcs->origin();
		SPAunit_vector XAxis = wcs->x_axis();
		SPAunit_vector YAxis = wcs->y_axis();
		SPAunit_vector ZAxis = wcs->z_axis();
		HC_KEY key = HC_KInsert_Text( Origin.x() + XAxis.x(), Origin.y() + XAxis.y(), Origin.z() + XAxis.z(), "X" );
		AddMapping( key, m_pEntity );
		key = HC_KInsert_Text( Origin.x() + YAxis.x(), Origin.y() + YAxis.y(), Origin.z() + YAxis.z(), "Y" );
		AddMapping( key, m_pEntity );
		key = HC_KInsert_Text( Origin.x() + ZAxis.x(), Origin.y() + ZAxis.y(), Origin.z() + ZAxis.z(), "Z" );
		AddMapping( key, m_pEntity );
		key = HC_KInsert_Line( Origin.x(), Origin.y(), Origin.z(), Origin.x() + XAxis.x(), Origin.y() + XAxis.y(), Origin.z() + XAxis.z() );
		AddMapping( key, m_pEntity );
		key = HC_KInsert_Line( Origin.x(), Origin.y(), Origin.z(), Origin.x() + YAxis.x(), Origin.y() + YAxis.y(), Origin.z() + YAxis.z() );
		AddMapping( key, m_pEntity );
		key = HC_KInsert_Line( Origin.x(), Origin.y(), Origin.z(), Origin.x() + ZAxis.x(), Origin.y() + ZAxis.y(), Origin.z() + ZAxis.z() );
		AddMapping( key, m_pEntity );
		HC_Conditional_Style( "?Style Library/AcisTextLinesOn", "AcisWCSsOn" );
		HC_Conditional_Style( "?Style Library/AcisTextLinesOff", "AcisWCSsOff" );
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
			switch ( light_type )
			{
			case EYE_LIGHT:
			{
				HA_Point pos( 0, 0, 0 );
				HA_Point target( 0, 0, 0 );
				char params[2056];
				//sprintf(params,"camera relative = on, illumination cone = 90 degrees");
				sprintf( params, "camera relative = on" );
				HC_KEY key = HC_KInsert_Spot_Light( pos, target, params );
				AddMapping( key, m_pEntity );
			}
			break;
			case POINT_LIGHT:
			{
				HC_KEY key = HC_KInsert_Local_Light( Pos.x(), Pos.y(), Pos.z() );
				AddMapping( key, m_pEntity );
			}
			break;
			case AMBIENT_LIGHT:
			{
				HC_Set_Color_By_Value( "ambient", "RGB", Color.red(), Color.green(), Color.blue() );
			}
			break;
			case SPOT_LIGHT:
			{
				HA_Point pos( Pos );
				// Catch the special case if the dir = 0.0.  It is illegal to set a hoops spotlight
				// with the same target and position.  If we see this, we will default the dir to
				// 0.0 0.0 1.0f
				if ( Dir.len() == 0.0f )
					Dir.set_z( 1.0f );
				HA_Point target( Pos + Dir );
				char params[2056];
				sprintf( params, "illumination cone = %f degrees", fCone_angle );
				HC_KEY key = HC_KInsert_Spot_Light( pos, target, params );
				AddMapping( key, m_pEntity );
			}
			break;
			case DISTANT_LIGHT:
			{
				HC_KEY key = HC_KInsert_Distant_Light( -Dir.x(), -Dir.y(), -Dir.z() );
				AddMapping( key, m_pEntity );
			}
			break;
			}
			HC_Set_Visibility( "lights=on" );
			HC_Set_Color_By_Value( "lights", "RGB", Color.red(), Color.green(), Color.blue() );
		} else
			HC_Set_Visibility( "lights=off" );
	}
}

HC_KEY hoops_acis_entity_converter::ConvertEntityAsm( ENTITY *entity, const ha_rendering_options &ro, const ha_rendering_context &rc )
{	// The ENTITY that we are creating. This will most likely be a top-level ENTITY such as BODY, but could  conceivably be a lower topological
	// type as well.  However, it really should be the topmost ENTITY pointer available.
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return 0;
	m_Map = rc.GetEntityMap();
	m_CHandleMap = rc.GetCHandleMap();
	m_ModelMap = rc.GetModelMap();
	m_RenderingOptions = ro;
	m_Pattern = rc.GetPattern();
	m_pEntity = entity;
	HC_KEY key = 0;
	if ( is_ambient_entity( entity ) )
	{
		char top_seg[128];
		sprintf( top_seg, "%s/entity", m_Pattern );
		key = HA_KOpen_Segment( m_pEntity, top_seg );
		AddMapping( key, m_pEntity );
		logical color_segment_open = OpenColorSegment( m_pEntity );
		ConvertAmbientEntity();
		if ( color_segment_open )
			HC_Close_Segment();
		HC_Close_Segment();
	} else
	{
		asm_model* owning_model = rc.GetModel();
		component_handle* top_component = NULL;
		if ( owning_model )
			check_outcome( asmi_model_get_component_handle( owning_model, top_component ) );
		logical color_segment_open = FALSE;
		char body_seg[128];
		// The m_ModelGeometryMode flag is set by ConvertModelGeometry, and will only be set for BODYs
		if ( m_ModelGeometryMode )
		{
			sprintf( body_seg, "%s/entity", rc.GetGeomPattern() );
			key = HA_KOpen_Segment( entity, body_seg );
		} else
		{
			sprintf( body_seg, "%s/pointer", rc.GetPattern() );
			key = KOpen_Pointer_Segment( top_component, body_seg );
			AddMapping( key, top_component );
			s_HA_CHandles.add( top_component );
		}
		API_BEGIN;
		{
			if ( !m_ModelGeometryMode )
				ConvertModelComponents( top_component, ro, rc );
			else
			{
				AddMapping( key, entity );
				// Get the transform to apply
				m_BodyTransf = get_owner_transf( m_pEntity );
				if ( !m_BodyTransf.identity() )
				{
					float float_mat[16];
					transf_to_matrix( float_mat, m_BodyTransf );
					HC_Set_Modelling_Matrix( float_mat );
				}
				color_segment_open = OpenColorSegment( m_pEntity );
				// Convert all the faces to mesh DOs and add them to the ACIS DO.
				// Commented this out for the case of a S-A EDGE.
				ConvertLumps();
			}
		} API_END;
		if ( color_segment_open )
			HC_Close_Segment();
		HC_Close_Segment(); // Top level entity or component
		check_outcome( result );
	}
	return key;
}

HC_KEY hoops_acis_entity_converter::ConvertModelGeometry( asm_model * model, const ha_rendering_options & ro, const ha_rendering_context & rc )
{
	HC_KEY answer = FindMapping( model );
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
					hoops_acis_entity_converter::ConvertEntityAsm( this_render_ent, ro, rc );
				this_render_ent = render_ents.next();
			}
			m_ModelGeometryMode = FALSE;
		}
	} MODEL_END( ASM_NO_CHANGE );
	answer = FindMapping( model );
	return answer;
}

LOCAL_PROC HC_KEY KCreateBodySegment( ENTITY* rend_ent )
{
	HC_KEY body_key = 0;
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
	body_key = HA_KOpen_Segment( rend_ent, buffer );
	if ( set_transf )
		HC_Set_Modelling_Matrix( float_mat );
	HC_Close_Segment();
	return body_key;
}

HC_KEY hoops_acis_entity_converter::ConvertModelComponents(
	component_handle*			comp,
	const ha_rendering_options& ro,
	const ha_rendering_context&	rc )
{
	HC_KEY answer = FindMapping( comp );
	logical is_hidden( FALSE );
	check_outcome( asmi_component_is_hidden( comp, is_hidden ) );
	if ( is_hidden )
		return answer;
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
	HC_Set_Modelling_Matrix( float_mat );
	// Recurse to subcomponents, if any
	component_handle_list subcomps;
	result = asmi_component_get_sub_components( comp, ASM_IMMEDIATE, subcomps );
	check_outcome( result );
	for ( component_handle* subcomp = subcomps.first(); subcomp; subcomp = subcomps.next() )
	{
		char* subcomp_pattern = "pointer";
		HC_KEY comp_key = KOpen_Pointer_Segment( subcomp, subcomp_pattern );
		AddMapping( comp_key, subcomp );
		s_HA_CHandles.add( subcomp );
		ConvertModelComponents( subcomp, ro, rc );
		HC_Close_Segment();
	}
	// Have we rendered the model geometry yet?
	HC_KEY model_key = FindMapping( this_end_model );
	if ( model_key && rc.GetForceModelRebuild() )
	{
		HA_Delete_Model_Geometry( this_end_model );
		model_key = 0;
	}
	if ( model_key == 0 )
	{
		char geom_pattern[128];
		sprintf( geom_pattern, "%s/%s", rc.GetGeomPattern(), "pointer" );
		model_key = KOpen_Pointer_Segment( this_end_model, geom_pattern );
		{
			AddMapping( model_key, this_end_model );
			char ent_pattern[100];
			Build_Segment_String( this_end_model, ent_pattern, geom_pattern );
			ha_rendering_context ent_rc( rc );
			ent_rc.SetGeomPattern( ent_pattern );
			ConvertModelGeometry( this_end_model, ro, ent_rc );
		}
		HC_Close_Segment();
	}
	if ( model_key )
	{
		// Look for component overrides, and handle them using conditional
		// styles and attribute locks
		logical has_override_color( FALSE );
		ATTRIB_COMPONENT_PROP_OWNER* comp_color_owner = NULL;
		rgb_color comp_color = get_comp_color( comp, comp_color_owner );
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
			char str[200];
			HC_Open_Segment( stylebuffer );
			{
				if ( has_override_color && has_transparency )
				{
					sprintf( str, "geometry = (r=%f g=%f b=%f), faces = (transmission = (r=%f g=%f b=%f))",
							 comp_color.red(), comp_color.green(), comp_color.blue(), transparency, transparency, transparency );
					HC_Set_Color( str );
					HC_Set_Rendering_Options( "attribute lock = (color = (geometry = diffuse, faces = transmission))" );
				} else if ( has_transparency )
				{
					sprintf( str, "faces = (transmission = (r=%f g=%f b=%f))", transparency, transparency, transparency );
					HC_Set_Color( str );
					HC_Set_Rendering_Options( "attribute lock = (color = (faces = transmission))" );
				} else if ( has_override_color )
				{
					sprintf( str, "geometry = (r=%f g=%f b=%f)", comp_color.red(), comp_color.green(), comp_color.blue() );
					HC_Set_Color( str );
					HC_Set_Rendering_Options( "attribute lock = (color = (geometry = diffuse))" );
				}
			}
			HC_Close_Segment();
			HC_Set_Conditions( handle );
			HC_Open_Segment_By_Key( model_key );
			HC_Conditional_Style( stylebuffer, handle );
			HC_Close_Segment();
		} else
		{
			// In case we're refreshing the segments, we need to make sure
			// that colors and transparencies get cleared if appropriate
			if ( HC_QShow_Existence( stylebuffer, "self" ) )
				HC_Delete_Segment( stylebuffer );
			HC_UnSet_One_Condition( handle );
		}
		// Include the corresponding model segment
		HC_Include_Segment_By_Key( model_key );
	}
	return answer;
}

void hoops_acis_entity_converter::AddMapping(
	HC_KEY			key,
	component_handle*	comp )
{
	if ( m_CHandleMap && m_EnableMapping )
		m_CHandleMap->AddMapping( key, comp );
}

void hoops_acis_entity_converter::AddMapping(
	HC_KEY			key,
	asm_model*		model )
{
	if ( m_ModelMap && m_EnableMapping )
		m_ModelMap->AddMapping( key, model );
}

HC_KEY hoops_acis_entity_converter::FindMapping( asm_model* model )
{
	HC_KEY key = 0;

	if ( m_ModelMap && m_EnableMapping )
		key = m_ModelMap->FindMapping( model );
	return key;
}

HC_KEY hoops_acis_entity_converter::FindMapping( component_handle* comp )
{
	HC_KEY key = 0;
	if ( m_CHandleMap && m_EnableMapping )
		key = m_CHandleMap->FindMapping( comp );
	return key;
}
