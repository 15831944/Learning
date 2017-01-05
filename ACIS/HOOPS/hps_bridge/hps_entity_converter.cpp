/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// History : 
// Sep-06-12 bta : 93941, hps_acis_entity_converter::ConvertCoedges,
//                 revise Jun-30-08 fix so it only applies to closed coedges.
// Apr-08-09 jkf : R20 Option use_asm_highlight_segments
// Jun-30-08 jkf : R19 Fixed arrow logic in codege display
// Dec-10-03 jkf : api_facet_curve, edge  needs that the tolerance is 
//                 bigger than SPAresabs.
// Dec-01-03 jkf : Arrow logic changed at coedge display
//
/*******************************************************************/

#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif // NT

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_util.h"
#include "hps_bridge.h"
#include "hps_bridge_state.hxx"
#include "hps_entity_converter.h"
#include "hps_util.h"
#include "hps_point.h"
#include "hps_map.h"
#include "hps_bridge.err"
#include "ckoutcom.hxx"
#include "body.hxx"
#include "shell.hxx"
#include "edge.hxx"
#include "coedge.hxx"
#include "tcoedge.hxx"
#include "loop.hxx"
#include "vertex.hxx"
#include "intcurve.hxx"
#include "sps3crtn.hxx"  
#include "acistype.hxx"    
#include "get_top.hxx"
#include "text.hxx"
#include "wcs.hxx"
#include "point.hxx"
#include "getowner.hxx"
#include "transfrm.hxx"
#include "curve.hxx"
#include "getbox.hxx"
#include "surface.hxx"
#include "surdef.hxx"
#include "api.hxx"    
#include "kernapi.hxx"    
#include "acistol.hxx"
#include "option.hxx"
#include "gmeshmg.hxx"
#include "LinkedMeshManager.hxx"
#include "handlers.hxx"
#include "intrapi.hxx"
#include "af_api.hxx"
#include "fct_utl.hxx"
#include "ptlist.hxx"
#include "no_rend.hxx"
#include "hps_bridge_internal.h"
#include "option.hxx"
#include "rlt_util.hxx"
#include "rnd_api.hxx"	// required to use class rgb_color
#include "ga_api.hxx"

#define ROUND_TO 10000.0f
/**
 * Option for disabling adding conditional segments for highlighting asm_model faces/edges/vertex
 */
SESSION_GLOBAL_VAR option_header use_asm_highlight_segments( "use_asm_highlight_segments", FALSE );
//===========================================================================
// Synopsis	 : 	ACIS converter for shaded objects	
//
// Notes	 :	This class will convert ACIS ENTITY's into a shaded 
//				representation based on polygon meshes.  We also will 
//				output the correct material types to the display objects
//				so that the users color and texture settings are correctly
//				handled.
//

#include "facet_body.hxx"
//===========================================================================
// Synopsis	 :	Create display object data for an ENTITY.
//
// Notes	 :	This is method that should be called to initially create 
//				the data for an entity, it will not attempt a limited
//				rebuild of the object.   However, it should at least 
//				look to see if there is existing data created by this
//				converter and remove it from the display object before
//				creating the new data.
//
//				The ENTITY that we are creating will most likely be a top-level ENTITY
//				such as BODY, but could conceivably be a lower topological type as well.
//				However, it really should be the topmost ENTITY pointer available.
//
////HPS::SegmentKey hps_acis_entity_converter::ConvertEntity(
////	ENTITY *						in_entity,
////	const hps_rendering_options &	in_ro,
////	HPS_Map *						in_map,
////	const char *					in_pattern
////	)
////{
////	m_Pattern = in_pattern;
////	HPS::SegmentKey key = HPS_Open_Segment( in_entity, m_Pattern );
////	return hps_acis_entity_converter::ConvertEntity( in_entity, in_ro, in_map, key );
////}

HPS::SegmentKey hps_acis_entity_converter::ConvertEntity(
	ENTITY *						in_entity,
	const hps_rendering_options &	in_ro,
	HPS_Map *						in_map,
	HPS::SegmentKey					in_segment_key
)
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return HPS_INVALID_SEGMENT_KEY;
	m_Map = in_map;
	m_RenderingOptions = in_ro;
	curve_pixel_tol = -1; // flush the cached value
	HPS::SegmentKey color_segment_key;
	AddMapping( in_segment_key, in_entity );
	if ( is_FACET_BODY( in_entity ) )
	{
		FACET_BODY* pb = static_cast<FACET_BODY*>( in_entity );
		HPS_Render_FACET_BODY( pb, m_RenderingOptions.GetRenderFacesMode(), m_RenderingOptions.GetRenderEdgesMode(), in_segment_key );
		return in_segment_key;
	}
	API_BEGIN;
	{
		m_pEntity = in_entity;
		// Get the transform to apply
		m_BodyTransf = get_owner_transf( m_pEntity );
		if ( !m_BodyTransf.identity() )
		{
			float float_mat[16];
			transf_to_matrix( float_mat, m_BodyTransf );
			in_segment_key.GetModellingMatrixControl().SetElements( 16, float_mat );
		}
		color_segment_key = HPS_OpenColorSegment( m_pEntity, in_segment_key );
		if ( !HPS_Is_Valid_Segment_Key( color_segment_key ) )
			color_segment_key = in_segment_key;
		// Convert all the faces to mesh DOs and add them to the ACIS DO.
		// Commented this out for the case of a S-A EDGE.
		//if (m_RenderingOptions.GetRenderFacesMode() || m_lSilhouette)
		ConvertLumps( color_segment_key ); // Added argument for HPS
		if ( is_APOINT( m_pEntity ) && m_RenderingOptions.GetRenderAPOINTsMode() )
		{
			fp_sentry fps;
			APOINT* pt = (APOINT*)m_pEntity;
			SPAposition		pos = pt->coords();
			//color_segment_key.GetVisibilityControl().SetMarkers( true );
			HPS::MarkerKey	marker_key = color_segment_key.InsertMarker(
				HPS::Point( (float)pos.x(), (float)pos.y(), (float)pos.z() ) );
			color_segment_key.GetPortfolioControl().Set( HPS_Portfolio );
			color_segment_key.GetMarkerAttributeControl().SetSymbol( "AcisGlyphVertex" );
			AddMapping( marker_key, m_pEntity );
		}
		if ( is_TEXT_ENT( m_pEntity ) && m_RenderingOptions.GetRenderTextMode() )
		{
			TEXT_ENT *text = (TEXT_ENT *)m_pEntity;
			if ( text )
			{
				fp_sentry fps;
				SPAposition pos = text->location();
				char font[2056];
				const char *font_name = text->font_name();
				//pos *= m_BodyTransf;
				HPS::Key text_key = color_segment_key.InsertText( HPS_Cast_Point( const_cast<SPAposition&>( pos ) ), text->string() );
				if ( font_name && strlen( font_name ) )
					color_segment_key.GetTextAttributeControl().SetFont( font_name );
				float size = (float)( (double)text->font_size() ); // *m_BodyTransf.scaling() );
				if ( size > 0 )
					color_segment_key.GetTextAttributeControl().SetFont( font ).SetSize( size, HPS::Text::SizeUnits::Pixels );
				AddMapping( text_key, m_pEntity );
			}
		}
		if ( is_WCS( m_pEntity ) && m_RenderingOptions.GetRenderWCSsMode() )
		{
			fp_sentry fps;
			WCS *wcs = (WCS *)m_pEntity;
			SPAposition Origin = wcs->origin();
			SPAunit_vector XAxis = wcs->x_axis();
			SPAunit_vector YAxis = wcs->y_axis();
			SPAunit_vector ZAxis = wcs->z_axis();
			//Origin *= m_BodyTransf;
			//XAxis *= m_BodyTransf;
			//YAxis *= m_BodyTransf;
			//ZAxis *= m_BodyTransf;
			HPS::TextKey Textkey = color_segment_key.InsertText( HPS::Point( (float)( Origin.x() + XAxis.x() ), (float)( Origin.y() + XAxis.y() ), (float)( Origin.z() + XAxis.z() ) ), "X" );
			AddMapping( Textkey, m_pEntity );
			Textkey = color_segment_key.InsertText( HPS::Point( (float)( Origin.x() + YAxis.x() ), (float)( Origin.y() + YAxis.y() ), (float)( Origin.z() + YAxis.z() ) ), "Y" );
			AddMapping( Textkey, m_pEntity );
			Textkey = color_segment_key.InsertText( HPS::Point( (float)( Origin.x() + ZAxis.x() ), (float)( Origin.y() + ZAxis.y() ), (float)( Origin.z() + ZAxis.z() ) ), "Z" );
			AddMapping( Textkey, m_pEntity );
			HPS::LineKey	LineKey = color_segment_key.InsertLine(
				HPS::Point( (float)Origin.x(), (float)Origin.y(), (float)Origin.z() ),
				HPS::Point( (float)( Origin.x() + XAxis.x() ), (float)( Origin.y() + XAxis.y() ), (float)( Origin.z() + XAxis.z() ) ) );
			AddMapping( LineKey, m_pEntity );
			LineKey = color_segment_key.InsertLine(
				HPS::Point( (float)Origin.x(), (float)Origin.y(), (float)Origin.z() ),
				HPS::Point( (float)( Origin.x() + YAxis.x() ), (float)( Origin.y() + YAxis.y() ), (float)( Origin.z() + YAxis.z() ) ) );
			AddMapping( LineKey, m_pEntity );
			LineKey = color_segment_key.InsertLine(
				HPS::Point( (float)Origin.x(), (float)Origin.y(), (float)Origin.z() ),
				HPS::Point( (float)( Origin.x() + ZAxis.x() ), (float)( Origin.y() + ZAxis.y() ), (float)( Origin.z() + ZAxis.z() ) ) );
			AddMapping( LineKey, m_pEntity );
		}
		if ( IS_LIGHT( m_pEntity ) )
		{
			fp_sentry fps;
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
					HPS::SpotlightKit slk;
					slk.SetPosition( HPS::Point( 0, 0, 0 ) );
					slk.SetTarget( HPS::Point( 0, 0, 0 ) );
					slk.SetCameraRelative( true );
					HPS::SegmentKey viewSegmentKey = HPS_Get_Current_View().GetAttachedModel().GetSegmentKey(); // Gets the root view segment key
					HPS::SpotlightKey sKey = viewSegmentKey.InsertSpotlight( slk );
					sys_warning( 0 ); // I don't know what to do yet.
					AddMapping( sKey, m_pEntity );
				} else if ( light_type == POINT_LIGHT )
				{
					sys_error( 0 ); // I don't know what to do yet.
					//key = color_segment_key.InsertDistantLight( HPS::Vector( (float)Pos.x(), (float)Pos.y(), (float)Pos.z() ) );
					//AddMapping(key, m_pEntity);

				} else if ( light_type == AMBIENT_LIGHT )
				{
					color_segment_key.GetMaterialMappingControl().SetAmbientLightColor( HPS::RGBAColor( (float)Color.red(), (float)Color.green(), (float)Color.blue() ) );
				} else if ( light_type == SPOT_LIGHT )
				{
					HPS::Point pos( (float)Pos.x(), (float)Pos.y(), (float)Pos.z() );
					// Catch the special case if the dir = 0.0.  It is illegal to set a hoops spotlight
					// with the same target and position.  If we see this, we will default the dir to
					// 0.0 0.0 1.0f
					if ( Dir.len() == 0.0f )
						Dir.set_z( 1.0f );
					HPS::Point target( (float)( Pos.x() + Dir.x() ), (float)( Pos.y() + Dir.y() ), (float)( Pos.z() + Dir.z() ) );
					HPS::SpotlightKit SpotLight;
					SpotLight.SetPosition( pos );
					SpotLight.SetTarget( target );
					SpotLight.SetOuterCone( fCone_angle );
					HPS::SpotlightKey key = color_segment_key.InsertSpotlight( pos, target );
					sys_error( 0 ); // I don't know what to do yet.
					//AddMapping( key, m_pEntity );
				} else if ( light_type == DISTANT_LIGHT )
				{
					HPS::DistantLightKey key = color_segment_key.InsertDistantLight( HPS::Vector( (float)-Dir.x(), (float)-Dir.y(), (float)-Dir.z() ) );
					sys_error( 0 ); // I don't know what to do yet.
					//AddMapping(key, m_pEntity);
				}
				//color_segment_key.GetVisibilityControl().SetLights( true );
				color_segment_key.GetMaterialMappingControl().SetAmbientLightColor( HPS::RGBAColor( (float)Color.red(), (float)Color.green(), (float)Color.blue() ) );
			} else
			{
				//if ( m_ConverterType == ct_standard )
				//	color_segment_key.GetVisibilityControl().SetLights( false );
			}
		}
	} API_END;
	check_outcome( result );
	return color_segment_key;
} // end of ConvertEntity()

void hps_acis_entity_converter::ConvertLumps( HPS::SegmentKey in_segment_key )
{
	ENTITY_LIST lumps;
	outcome result = api_get_lumps( m_pEntity, lumps );
	// This things has no lumps, so pass it down.
	if ( lumps.iteration_count() < 1 )
		ConvertShells( m_pEntity, in_segment_key );
	lumps.init();
	ENTITY* lump = 0;
	HPS::SegmentKey temp_segment_key = in_segment_key;
	if ( lumps.iteration_count() > 1 )
		temp_segment_key = HPS_Open_Segment( "LumpSeg", in_segment_key );
	while ( ( lump = lumps.next() ) != 0 )
	{
		HPS::SegmentKey color_segment_key = HPS_OpenColorSegment( lump, temp_segment_key );
		if ( HPS_Is_Valid_Segment_Key( color_segment_key ) )
			temp_segment_key = color_segment_key;
		ConvertShells( lump, temp_segment_key );
	}
} // end of ConvertLumps

void hps_acis_entity_converter::ConvertShells(
	ENTITY *		in_entity,	/* most likely a lump */
	HPS::SegmentKey in_segment_key )
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return;
	ENTITY_LIST shells;
	outcome result = api_get_shells( in_entity, shells );
	// This thing has no shells, so pass it down.
	if ( shells.iteration_count() < 1 )
	{
		if ( m_RenderingOptions.GetRenderFacesMode() || m_ModelGeometryMode )
			ConvertFaces( in_entity, in_segment_key );
		if ( m_RenderingOptions.GetRenderCoedgeMode() )
			ConvertCoedges( in_entity, in_segment_key );
		if ( m_RenderingOptions.GetRenderEdgesMode() || m_ModelGeometryMode )
			ConvertEdges( in_entity, NULL, in_segment_key );
		if ( m_RenderingOptions.GetRenderTCoedgeMode() )
			ConvertTCoedges( in_entity, in_segment_key );
		if ( m_RenderingOptions.GetRenderVerticesMode() || m_ModelGeometryMode )
			ConvertVertices( in_entity, in_segment_key );
	}
	shells.init();
	SHELL* shell = 0;
	while ( ( shell = (SHELL*)shells.next() ) != 0 )
	{
		HPS::SegmentKey temp_segment_key = in_segment_key;
		HPS::SegmentKey color_segment_open = HPS_OpenColorSegment( shell, in_segment_key );
		if ( HPS_Is_Valid_Segment_Key( color_segment_open ) )
			temp_segment_key = color_segment_open;
		if ( shell->wire() && m_RenderingOptions.GetRenderCoedgeMode() )
			ConvertCoedges( shell, temp_segment_key );
		if ( m_RenderingOptions.GetRenderFacesMode() || m_ModelGeometryMode )
			ConvertFaces( shell, temp_segment_key );
		if ( m_RenderingOptions.GetRenderCoedgeMode() )
			ConvertCoedges( in_entity, temp_segment_key );
		if ( m_RenderingOptions.GetRenderEdgesMode() || m_ModelGeometryMode )
			ConvertEdges( shell, NULL, temp_segment_key );
		if ( m_RenderingOptions.GetRenderTCoedgeMode() )
			ConvertTCoedges( shell, temp_segment_key );
		if ( m_RenderingOptions.GetRenderVerticesMode() || m_ModelGeometryMode )
			ConvertVertices( shell, temp_segment_key );
	}
} // end of ConvertShells

// Find all geometry in the currently open segment and delete the associated
// mappings to ent.
void Delete_Geometry_Mappings( HPS_Map * in_map, ENTITY * in_ent )
{
}

extern HA_RENDER_progress_info * progress_meter;
#include "idx_mesh.hxx"
void hps_acis_entity_converter::MeshFaces( HPS::SegmentKey in_segment_key, ENTITY* in_entity )
{
	ENTITY_LIST faces;
	outcome result = api_get_faces( in_entity, faces );
	ENTITY_LIST unfaceted_faces;
	{for ( ENTITY* f = faces.first(); f; f = faces.next() )
	{
		af_serializable_mesh* mesh = GetSerializableMesh( (FACE*)f );
		if ( NULL == mesh )
		{
			unfaceted_faces.add( f );
		}
	}}
	static option_header * opt_render_lean = 0;
	static logical opt_render_lean_requested = FALSE;
	if ( opt_render_lean == 0 && !opt_render_lean_requested )
	{
		opt_render_lean = find_option( "render_lean" );
		opt_render_lean_requested = TRUE;
	}
	int render_lean = opt_render_lean ? ( opt_render_lean->count() & 3 ) : 0;
	if ( !( render_lean & 2 ) && unfaceted_faces.count() > 0 )
	{	// Make sure all is faceted...
		API_SYS_BEGIN;
		{	// Facet at top-level and don't refacet already faceted faces.
			facet_options* fo = HPS_Get_Facet_Options();
			api_facet_entities( in_entity, &unfaceted_faces, fo );
		} API_SYS_END;
	}
	for ( FACE* face = (FACE*)faces.first(); face; face = (FACE*)faces.next() )
	{	// Loop through each face and convert the INDEXED_MESH to HOOPS.
		if ( find_NORENDER_ATTRIB( face ) != NULL )
			continue;
		if ( progress_meter )
			progress_meter->update();
		HPS::SegmentKey color_segment_key = HPS_INVALID_SEGMENT_KEY;
		color_segment_key = HPS_OpenColorSegment( face, in_segment_key );
		if ( !HPS_Is_Valid_Segment_Key( color_segment_key ) )
			color_segment_key = in_segment_key;
		if ( face->sides() == DOUBLE_SIDED )
			color_segment_key.GetCullingControl().SetBackFace( false );
		API_SYS_BEGIN;
		{	// Facet, convert to HPS, and add to bridge mapping...
			if ( m_ModelGeometryMode && use_asm_highlight_segments.on() )
			{	// In assembly mode, different parts can share geometry. To avoid having duplications, always clear the existing geometry.
				color_segment_key = HPS_KOpen_Segment( in_entity, m_Pattern, HPS_Get_Current_View().GetAttachedModel().GetSegmentKey() );
				Delete_Geometry_Mappings( m_Map, face );
				char buffer[64];
				HPS_Build_Segment_String( face, buffer, "entity" );
				color_segment_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisAsmHighlightFacesEdges" ).SetCondition( buffer ) );
			}
			if ( render_lean & 2 )
			{	// Make sure all is faceted...
				facet_options* fo = HPS_Get_Facet_Options();
				FacetEntity( face, TRUE, *(int*)NULL_REF, *(int*)NULL_REF,
							 *(unsigned *)NULL_REF, fo );
			}
			af_serializable_mesh *mesh = GetSerializableMesh( face ); // Get the MESH from the FACE
			HPS::Key shell_key = SequentialMeshToHOOPS( color_segment_key, mesh );
			//HPS_Get_Current_View().GetOwningLayouts()[0].GetOwningCanvases()[0].UpdateWithNotifier().Wait();
			if ( HPS_Is_Valid_Key( shell_key ) )
				AddMapping( shell_key, face );
			if ( render_lean & 1 )
				api_delete_entity_facets( face );
		} API_SYS_END;
	}
} // end of MeshFaces()

option_header hps_direct_render( "hps_direct_render", FALSE );
void hps_acis_entity_converter::ConvertFaces( ENTITY * in_entity /*most likely a shell*/, HPS::SegmentKey in_segment_key )
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return;
	// create a segment to hold all faces for this body/shell
	// This allows us to turn off markers at this level.

	HPS::SegmentKey face_key = in_segment_key;
	if ( m_ModelGeometryMode )
	{
		face_key = HPS_Open_Segment( "SPA faces", in_segment_key );
		face_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisShellsOn" ).SetCondition( "AcisFacesOn" ) );
		face_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisShellsOffSilsOn" ).SetCondition( "AcisFacesOffSilsOn" ) );
		face_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisShellsOffSilsOff" ).SetCondition( "AcisFacesOffSilsOff" ) );
		//HPS::NamedStyleDefinition nsd;
		//if ( HPS_Portfolio.ShowNamedStyleDefinition( "AcisShellsOffSilsOff", nsd ) )
		//	nsd.GetSource();
		//face_key.GetStyleControl().PushNamed( "AcisFacesOn" );
	}
	else if ( m_Pattern == NULL )
		face_key = HPS_Open_Segment( "ACIS Faces", in_segment_key );

	face_key.GetVisibilityControl().SetMarkers( false );
	//	printf( "ConvertFaces: face_key=\"%s\"\n", HPS_Show_Segment( face_key ).c_str( ) );
	MeshFaces( face_key, in_entity );
} // end of ConvertFaces()

void hps_acis_entity_converter::ConvertSilhouettes(
	FACE *				in_face,
	const SPAtransf *	in_tform,
	SPAposition &		in_eye,
	SPAposition &		in_target,
	int					in_projection,
	bool				in_mapping,
	HPS::SegmentKey		in_segment_key )
{
	ENTITY_LIST edge_list;
	API_NOP_BEGIN;
	{
		api_silhouette_edges( in_face, *in_tform, in_eye, in_target, in_projection, &edge_list );
		HPS::SegmentKey color_segment_key = HPS_OpenColorSegment( in_face, in_segment_key );
		if ( !HPS_Is_Valid_Segment_Key( color_segment_key ) )
			color_segment_key = in_segment_key;
		ENTITY *toplevel = NULL;
		api_get_owner( in_face, toplevel );
		EDGE* edge = NULL;
		for ( int i = 0; i < edge_list.count(); i++ )
		{
			edge = (EDGE*)edge_list[i];
			m_EnableMapping = in_mapping;
			api_add_generic_named_attribute( edge, "HPS_SilhouetteFace", in_face );
			ConvertEdges( edge, toplevel, color_segment_key );
			m_EnableMapping = TRUE;
		}
	} API_NOP_END;
} // end of ConvertSilhouettes()

void hps_acis_entity_converter::ConvertEdges( ENTITY * in_entity, ENTITY * in_owner, HPS::SegmentKey in_segment_key )
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return;
	// Avoid creating a new unnamed segment if one already exists
	HPS::SegmentKey edge_seg_key = in_segment_key;
	if ( m_ModelGeometryMode )
	{
		edge_seg_key = HPS_Open_Segment( "SPA edges", edge_seg_key );
		HPS_Style_Library.Subsegment( "AcisPolylinesOff", false ).GetConditionControl().AddCondition( "AcisEdgesOff" );
		HPS_Style_Library.Subsegment( "AcisPolylinesOn", false ).GetConditionControl().AddCondition( "AcisEdgesOn" );
		edge_seg_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisPolylinesOff", false ) );
		edge_seg_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisPolylinesOn", false ) );
	}
	else if ( m_Pattern == NULL )
		edge_seg_key = HPS_Open_Segment( "ACIS Edges", in_segment_key );

	ENTITY_LIST edges;
	outcome result = api_get_edges( in_entity, edges );
	if ( !result.ok() )
		return;
	CurveFacetLevel facet_level = m_RenderingOptions.GetCurveFacetLevel();
	edges.init();
	EDGE* edge = 0;
	while ( ( edge = (EDGE*)edges.next() ) != 0 )
	{
		if ( find_NORENDER_ATTRIB( edge ) != NULL )
			continue;
		CURVE *edge_geom = edge->geometry();
		if ( !edge_geom )
			continue;
		HPS::SegmentKey color_segment_key = HPS_OpenColorSegment( edge, edge_seg_key );
		if ( !HPS_Is_Valid_Segment_Key( color_segment_key ) )
			color_segment_key = edge_seg_key;
		else
			color_segment_key.GetLineAttributeControl().UnsetWeight();
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{	// In assembly mode, different parts can share geometry.  To avoid having duplications, always clear the existing geometry.
				Delete_Geometry_Mappings( m_Map, edge );
				char buffer[64];
				HPS_Build_Segment_String( edge, buffer, "entity" );
				color_segment_key = HPS_KOpen_Segment( edge, "entity", color_segment_key );
				color_segment_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisAsmHighlightFacesEdges" ).SetCondition( buffer ) );
			}
		}
		int num_pts = 0;
		if ( facet_level == cfl_face_facets )
		{	// We're going to try for the facets first.
			SPAposition *edge_points = 0;
			result = api_get_facet_edge_points( edge, edge_points, num_pts );
			if ( !result.ok() )
				continue;
			if ( !edge_points )
				facet_level = cfl_hps_primitives;
			else
			{
				HPS::Key key = MakeHoopsPolyline( num_pts, edge_points, FALSE, color_segment_key );
				AddMapping( key, edge, in_owner );
				if ( edge_points )
					ACIS_DELETE[] edge_points; edge_points = 0;
			}
		}
		if ( facet_level != cfl_face_facets )
		{
			SPAparameter start_param = edge->start_param();
			SPAparameter end_param = edge->end_param();
			if ( edge->sense() == REVERSED )
			{
				SPAparameter temp = -start_param;
				start_param = -end_param;
				end_param = temp;
			}
			ConvertCURVE( edge_geom, edge, start_param, end_param, facet_level, in_owner, color_segment_key );
			//color_segment_key.GetVisibilityControl().SetLines( true );
		}
	}
} // end of ConvertEdges()

void hps_acis_entity_converter::ConvertCoedges(
	ENTITY *		in_entity,
	HPS::SegmentKey	in_segment_key )
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return;
	HPS::SegmentKey coedge_key = in_segment_key;
	if ( m_ModelGeometryMode )
	{
		HPS::SegmentKey key = HPS_Open_Segment( "SPA coedges" );
//		HPS_Style_Library.Subsegment( "AcisPolylinesOn" ).GetConditionControl().AddCondition( "AcisCoedgesOn" );
//		HPS_Style_Library.Subsegment( "AcisPolylinesOff" ).GetConditionControl().AddCondition( "AcisCoedgesOff" );
//		key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisPolylinesOn" ) );
//		key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisPolylinesOff" ) );
	} else if ( m_Pattern == NULL )
		coedge_key = HPS_Open_Segment( "ACIS CoEdges", in_segment_key );

	ENTITY_LIST coedges;
	outcome result = api_get_coedges( in_entity, coedges );
	CurveFacetLevel facet_level = m_RenderingOptions.GetCurveFacetLevel();
	coedges.init();
	COEDGE* coedge = 0;
	coedges.init();
	// Now get the edge points
	while ( ( coedge = (COEDGE *)coedges.next() ) != 0 )
	{
		FACE* face = 0;
		// Check if face.
		if ( coedge->loop() && coedge->loop()->face() )
			face = coedge->loop()->face();
		EDGE *edge = coedge->edge();
		HPS::SegmentKey color_segment_key = HPS_OpenColorSegment( coedge, coedge_key );
		if ( !HPS_Is_Valid_Segment_Key( color_segment_key ) )
			color_segment_key = coedge_key;
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{	// In assembly mode, different parts can share geometry.  To avoid
				// having duplications, always clear the existing geometry.
				Delete_Geometry_Mappings( m_Map, coedge );
			}
		}
		double percent = 0.05;
		if ( edge->geometry() )
		{
			double tolerance = 0.1;
			if ( facet_level == cfl_bounding_box )
			{
				SPAbox edge_box = get_edge_box( edge );
				tolerance = ( edge_box.high() - edge_box.low() ).len();
				tolerance *= m_RenderingOptions.GetCurveFacetTol();
			} else
				tolerance = m_RenderingOptions.GetCurveFacetTol();
			const SPAtransf tform = get_owner_transf( in_entity );
			tolerance /= tform.scaling();
			if ( tolerance < SPAresabs )
				tolerance = 10 * SPAresabs;
			SPAparameter start_param;
			SPAparameter end_param;
			if ( coedge->sense() )
			{
				start_param = edge->end_param();
				end_param = edge->start_param();
			} else
			{
				start_param = edge->start_param();
				end_param = edge->end_param();
			}
			if ( edge->sense() == REVERSED )
			{
				SPAparameter temp = -start_param;
				start_param = -end_param;
				end_param = temp;
			}
			curve const &cur_ptr = edge->geometry()->equation();
			double offset = ( end_param - start_param )*percent;
			start_param += offset;
			end_param -= offset;
			int num_pts = 0;
			logical finished = FALSE;
			while ( !finished )
			{
				num_pts = 0;
				result = api_facet_curve( cur_ptr, start_param, end_param, tolerance, 0.0, 0.0, m_Max_num_pts_so_far, num_pts, &m_pEdge_points, &m_pParam_vals );
				finished = TRUE;
				if ( num_pts + 3 > m_Max_num_pts_so_far )
				{	// We need 3 extra points to draw arrow head.
					if ( m_pEdge_points )
						ACIS_DELETE[] m_pEdge_points; m_pEdge_points = 0;
					if ( m_pParam_vals )
						ACIS_DELETE[] STD_CAST m_pParam_vals; m_pParam_vals = 0;
					// Lets increase the max points by 50%.
					m_Max_num_pts_so_far = (int)( num_pts*1.5 );
					m_pEdge_points = ACIS_NEW SPAposition[m_Max_num_pts_so_far];
					m_pParam_vals = ACIS_NEW double[m_Max_num_pts_so_far];
					finished = FALSE;
				}
			}
			// Side of the arrow
			SPAposition ep1 = m_pEdge_points[0];
			SPAposition ep2 = m_pEdge_points[num_pts - 1];
			SPAposition start = coedge->start_pos();
			double dist1 = ( start - ep1 ).len();
			double dist2 = ( start - ep2 ).len();
			logical at_start = dist2 < dist1;
			if ( coedge->start() == coedge->end() )
			{
				logical is_intcurve = cur_ptr.type() == intcurve_type;
				logical is_rev_intcurve = FALSE;
				if ( is_intcurve )
					is_rev_intcurve = ( (intcurve const&)cur_ptr ).reversed();
				REVBIT coed_sense = coedge->sense();
				REVBIT ed_sense = edge->sense();
				if ( coed_sense == FORWARD && ed_sense == FORWARD && is_rev_intcurve == FALSE )
					at_start = FALSE;
				if ( coed_sense == REVERSED && ed_sense == FORWARD && is_rev_intcurve == TRUE )
					at_start = TRUE;
				if ( coed_sense == REVERSED && ed_sense == REVERSED && is_rev_intcurve == FALSE )
					at_start = TRUE;
				if ( coed_sense == REVERSED && ed_sense == FORWARD && is_rev_intcurve == FALSE )
					at_start = TRUE;
				if ( coed_sense == FORWARD && ed_sense == FORWARD && is_rev_intcurve == TRUE )
					at_start = FALSE;
				// Sep-06-12 bta : 93941,
				// moved this inside the coedge->start()==coedge->end() clause - it should not be called for open coedges. 
				if ( edge->sense() == REVERSED ) // jkf edge points were made wrt curve (Jun-30-08)
					at_start = !at_start;
			}
			SPAunit_vector face_normal;
			if ( face )
			{
				SPAbox face_box = get_face_box( face );
				double normal_offset = ( face_box.high() - face_box.low() ).len();
				normal_offset *= 0.02;
				// Now offset into the face a little bit.
				// We have the shortest edge length, so we can use that extra bit of info.
				for ( int i = 0; i < num_pts; i++ )
				{
					SPAposition point_on_surf;
					const surface &surf = face->geometry()->equation();
					SPApar_pos pp;
					SPAposition temp2;
					// This may not be needed because m_pEdge_points[i] should be on the surface.
					//surf.point_perp(m_pEdge_points[i],point_on_surf,*(SPApar_pos *)NULL_REF,pp);
					//SPAunit_vector face_normal=surf.point_normal(point_on_surf);
					face_normal = surf.point_normal( m_pEdge_points[i] );
					if ( face->sense() == REVERSED )
						face_normal = -face_normal;
					m_pEdge_points[i] += face_normal*normal_offset;
					/*
					// Now slap on an arrowhead
					if (i==num_pts-1)
					{
					SPAunit_vector last_segment=normalise(m_pEdge_points[i]-m_pEdge_points[i-1]);
					SPAunit_vector cross_prod=normalise(face_normal*last_segment);
					m_pEdge_points[num_pts]=m_pEdge_points[num_pts-1]+(cross_prod*normal_offset)-(last_segment*normal_offset);
					m_pEdge_points[num_pts+1]=m_pEdge_points[num_pts-1];
					m_pEdge_points[num_pts+2]=m_pEdge_points[num_pts-1]-(cross_prod*normal_offset)-(last_segment*normal_offset);
					}
					*/
				}
			} else
			{
				/*
				// Now slap on an arrowhead
				SPAbox edge_box=get_edge_box(edge);
				double normal_offset=(edge_box.high()-edge_box.low()).len();
				normal_offset*=0.07;
				SPAvector last_segment=m_pEdge_points[num_pts-1]-m_pEdge_points[num_pts-1-1];
				SPAunit_vector normalised_last_segment=normalise(last_segment);
				SPAunit_vector cross_prod=normalise(last_segment.make_ortho());
				m_pEdge_points[num_pts]=m_pEdge_points[num_pts-1]+(cross_prod*normal_offset)-(normalised_last_segment*normal_offset);
				m_pEdge_points[num_pts+1]=m_pEdge_points[num_pts-1];
				m_pEdge_points[num_pts+2]=m_pEdge_points[num_pts-1]-(cross_prod*normal_offset)-(normalised_last_segment*normal_offset);
				*/
			}
			//color_segment_key.GetVisibilityControl().SetLines( true );
			HPS::Key key = MakeHoopsPolyline( num_pts, m_pEdge_points, FALSE, color_segment_key );
			AddMapping( key, coedge );
			// Add the Arrow
			SPAposition arrow_pts[3];
			if ( at_start )
			{
				SPAbox edge_box = get_edge_box( edge );
				double normal_offset = ( edge_box.high() - edge_box.low() ).len();
				normal_offset *= 0.02;
				SPAvector last_segment = m_pEdge_points[0] - m_pEdge_points[1];
				SPAunit_vector normalised_last_segment = normalise( last_segment );
				SPAunit_vector cross_prod = face ? normalise( face_normal*last_segment ) : normalise( last_segment.make_ortho() );
				arrow_pts[1] = m_pEdge_points[0];
				arrow_pts[0] = m_pEdge_points[0] + ( cross_prod*normal_offset ) - ( normalised_last_segment*normal_offset );
				arrow_pts[2] = m_pEdge_points[0] - ( cross_prod*normal_offset ) - ( normalised_last_segment*normal_offset );
			} else
			{
				SPAbox edge_box = get_edge_box( edge );
				double normal_offset = ( edge_box.high() - edge_box.low() ).len();
				normal_offset *= 0.02;
				SPAvector last_segment = m_pEdge_points[num_pts - 1] - m_pEdge_points[num_pts - 2];
				SPAunit_vector normalised_last_segment = normalise( last_segment );
				SPAunit_vector cross_prod = face ? normalise( face_normal*last_segment ) : normalise( last_segment.make_ortho() );
				arrow_pts[1] = m_pEdge_points[num_pts - 1];
				arrow_pts[0] = m_pEdge_points[num_pts - 1] + ( cross_prod*normal_offset ) - ( normalised_last_segment*normal_offset );
				arrow_pts[2] = m_pEdge_points[num_pts - 1] - ( cross_prod*normal_offset ) - ( normalised_last_segment*normal_offset );
			}
			//color_segment_key.GetVisibilityControl().SetLines( true );
			HPS::Key key2 = MakeHoopsPolyline( 3, arrow_pts, FALSE, color_segment_key );
			AddMapping( key2, coedge );
		}
	}
} // end of ConvertCoedges()

void hps_acis_entity_converter::ConvertTCoedges(
	ENTITY *		in_entity,
	HPS::SegmentKey	in_segment_key )
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return;
	HPS::SegmentKey tCoEdges_key = in_segment_key;
	if ( m_ModelGeometryMode )
	{
		//HPS::SegmentKey key = HPS_Open_Segment( "SPA tcoedges" );
		//key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisPolylinesOn" ).AddCondition( "AcisTCoedgesOn" ) );
		//key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisPolylinesOff" ).AddCondition( "AcisTCoedgesOff" ) );
	} else if ( m_Pattern == NULL )
		tCoEdges_key = HPS_Open_Segment( "ACIS tCoEdges", in_segment_key );
	ENTITY_LIST tcoedges;
	outcome result = api_get_tcoedges( in_entity, tcoedges );
	CurveFacetLevel facet_level = m_RenderingOptions.GetCurveFacetLevel();
	tcoedges.init();
	TCOEDGE* tcoedge = 0;
	while ( ( tcoedge = (TCOEDGE *)tcoedges.next() ) != 0 )
	{
		HPS::SegmentKey temp_key = tCoEdges_key;
		HPS::SegmentKey color_segment_key = HPS_OpenColorSegment( tcoedge, temp_key );
		if ( !HPS_Is_Valid_Segment_Key( color_segment_key ) )
			color_segment_key = tCoEdges_key;
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{	// In assembly mode, different parts can share geometry.  To avoid
				// having duplications, always clear the existing geometry.
				Delete_Geometry_Mappings( m_Map, tcoedge );
			}
		}
		int num_pts = 0;
		CURVE *geom = tcoedge->get_3D_curve();
		if ( geom )
		{
			double tolerance = 0.1;
			if ( facet_level == cfl_bounding_box )
			{
				SPAbox tcoedge_box = get_tcoedge_box( tcoedge );
				tolerance = ( tcoedge_box.high() - tcoedge_box.low() ).len();
				tolerance *= m_RenderingOptions.GetCurveFacetTol();
			} else
				tolerance = m_RenderingOptions.GetCurveFacetTol();
			const SPAtransf tform = get_owner_transf( in_entity );
			tolerance /= tform.scaling();
			if ( tolerance < SPAresabs )
				tolerance = 10 * SPAresabs;
			curve const &cur_ptr = geom->equation();
			SPAinterval range = tcoedge->param_range();
			SPAparameter start_param = range.start_pt();
			SPAparameter end_param = range.end_pt();
			logical finished = FALSE;
			while ( !finished )
			{
				result = api_facet_curve( cur_ptr, start_param, end_param, tolerance, 0.0, 0.0,
										  m_Max_num_pts_so_far,
										  num_pts, &m_pEdge_points, &m_pParam_vals );
				finished = TRUE;
				if ( num_pts > m_Max_num_pts_so_far )
				{
					if ( m_pEdge_points )
						ACIS_DELETE[] m_pEdge_points; m_pEdge_points = 0;
					if ( m_pParam_vals )
						ACIS_DELETE[] STD_CAST m_pParam_vals; m_pParam_vals = 0;
					// Lets increase the max points by 50%.
					m_Max_num_pts_so_far = (int)( num_pts*1.5 );
					m_pEdge_points = ACIS_NEW SPAposition[m_Max_num_pts_so_far];
					m_pParam_vals = ACIS_NEW double[m_Max_num_pts_so_far];
					finished = FALSE;
				}
			}
		}
		//temp_key.GetVisibilityControl().SetLines( true );
		HPS::Key key = MakeHoopsPolyline( num_pts, m_pEdge_points, FALSE, color_segment_key );
		AddMapping( key, tcoedge );
	}
} // end of ConvertTCoedges()

void hps_acis_entity_converter::ConvertVertices(
	ENTITY*			in_entity,
	HPS::SegmentKey	in_segment_key )
{
	if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
		return;
	fp_sentry fps;
	ENTITY_LIST vertices;
	outcome result = api_get_vertices( in_entity, vertices );
	if ( !result.ok() )
		return;
	// create a segment to hold all vertices for this body
	// Avoid creating a new unnamed segment if one already exists
	HPS::SegmentKey vertices_key = HPS_INVALID_SEGMENT_KEY;
	if ( m_ModelGeometryMode )
	{
		vertices_key = HPS_Open_Segment( "SPA vertices", in_segment_key );
		vertices_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisMarkersOn" ).SetCondition( "AcisVerticesOn" ) );
		vertices_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisMarkersOff" ).SetCondition( "AcisVerticesOff" ) );
	}
	else  if ( m_Pattern == NULL )
		vertices_key = HPS_Open_Segment( "ACIS Vertices", in_segment_key );
	vertices.init();
	VERTEX* vertex = 0;
	while ( ( vertex = (VERTEX*)vertices.next() ) != 0 )
	{
		if ( find_NORENDER_ATTRIB( in_entity ) != NULL )
			continue;
		HPS_OpenColorSegment( vertex, vertices_key );
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{	// In assembly mode, different parts can share geometry.  To avoid having duplications, always clear the existing geometry.
				Delete_Geometry_Mappings( m_Map, vertex );
				char buffer[64];
				HPS_Build_Segment_String( vertex, buffer, "entity" );
				vertices_key = HPS_KOpen_Segment( in_entity, m_Pattern, HPS_Get_Current_View().GetAttachedModel().GetSegmentKey() );
				vertices_key.GetStyleControl().PushSegment( HPS_Style_Library.Subsegment( "AcisAsmHighlightVerts" ).SetCondition( buffer ) );
			}
		}
		APOINT* pt = vertex->geometry();
		SPAposition pos = pt->coords();
		//pos *= m_BodyTransf;
		//in_segment_key.GetVisibilityControl().SetMarkers( true );
		HPS::MarkerKey	key = vertices_key.InsertMarker( HPS::Point( (float)pos.x(), (float)pos.y(), (float)pos.z() ) );
		in_segment_key.GetPortfolioControl().Set( HPS_Portfolio );
		vertices_key.GetPortfolioControl().Set( HPS_Portfolio );
		vertices_key.GetMarkerAttributeControl().SetSymbol( "AcisGlyphVertex" );
		AddMapping( key, vertex );
	}
} // end of ConvertVertices()

HPS::Key hps_acis_entity_converter::MakeHoopsPolyline( int in_num_pts, SPAposition* in_points, bool in_bUseBodyTransf, HPS::SegmentKey in_segment_key )
{
	HPS::Key key = HPS_INVALID_KEY;
	if ( !( in_num_pts > 0 && in_points ) )
		return key;
	fp_sentry fps;
	HPS::PointArray HpsPoints;
	HpsPoints.reserve( in_num_pts );
	ConvertSPAPositionToPointArray( in_num_pts, in_points, HpsPoints, ( ( !m_ModelGeometryMode ) & in_bUseBodyTransf ) ? &m_BodyTransf : 0 );
	key = in_segment_key.InsertLine( HpsPoints );
	return key;
}

void hps_acis_entity_converter::ConvertCURVE(
	CURVE*			in_CUR,
	ENTITY*			in_ent,
	SPAparameter	in_start_param,
	SPAparameter	in_end_param,
	CurveFacetLevel	in_facet_level,
	ENTITY*			in_owner,
	HPS::SegmentKey	in_segment_key )
{

	if ( in_facet_level == cfl_hps_primitives )
	{
		if ( in_CUR->identity() == ELLIPSE_TYPE )
			ConvertELLIPSE( (ELLIPSE*)in_CUR, in_ent, in_start_param, in_end_param, in_segment_key );
		else if ( in_CUR->identity() == INTCURVE_TYPE )
			Convert_intcurve( in_CUR, in_ent, in_start_param, in_end_param, cfl_screen, in_segment_key );
		else
			ConvertCURVE( in_CUR, in_ent, in_start_param, in_end_param, cfl_screen, in_owner, in_segment_key );
	} else
	{
		double tolerance = curve_pixel_tol;
		if ( tolerance < 0 )
			curve_pixel_tol = tolerance = m_RenderingOptions.GetCurveFacetTol(); //cfl_world
		const SPAtransf tform = get_owner_transf( in_ent );
		tolerance /= tform.scaling();
		if ( tolerance < SPAresabs )
			tolerance = 10 * SPAresabs;
		logical finished = FALSE;
		int num_pts = 0;
		const EDGE* edge = (EDGE*)in_ent;
		if ( is_EDGE( in_ent ) )
		{
			AF_POINT* first_point;
			int total_count = 0;
			outcome result = api_facet_edge( edge, tolerance, 0.0, 0.0, total_count, first_point );
			if ( total_count > 0 )
			{
				if ( total_count > m_Max_num_pts_so_far )
				{
					if ( m_pEdge_points )
						ACIS_DELETE[] m_pEdge_points;
					m_pEdge_points = 0;
					if ( m_pParam_vals )
						ACIS_DELETE[] STD_CAST m_pParam_vals;
					m_pParam_vals = 0;
					// Lets increase the max points by 50%.
					m_Max_num_pts_so_far = (int)( total_count * 1.5 );
					m_pEdge_points = ACIS_NEW SPAposition[m_Max_num_pts_so_far];
					m_pParam_vals = ACIS_NEW double[m_Max_num_pts_so_far];
				}
				AF_POINT* curr = first_point;
				int count;
				for ( count = 0; count < total_count; count++ )
				{
					if ( m_pEdge_points )
						m_pEdge_points[count] = curr->get_position();
					if ( m_pParam_vals )
						m_pParam_vals[count] = curr->get_parameter();
					curr = curr->next( 0 );
				}
				api_delete_all_AF_POINTs( first_point );
				finished = TRUE;
				num_pts = total_count;
			}
		}
		while ( !finished )
		{
			curve const &cur_ptr = in_CUR->equation();
			outcome result;
			result = api_facet_edge( edge, tolerance, 0.0, 0.0, m_Max_num_pts_so_far, num_pts, &m_pEdge_points, &m_pParam_vals );
			finished = TRUE;
			if ( num_pts > m_Max_num_pts_so_far )
			{
				if ( m_pEdge_points )
					ACIS_DELETE[] m_pEdge_points; m_pEdge_points = 0;
				if ( m_pParam_vals )
					ACIS_DELETE[] STD_CAST m_pParam_vals; m_pParam_vals = 0;
				// Lets increase the max points by 50%.
				m_Max_num_pts_so_far = (int)( num_pts*1.5 );
				m_pEdge_points = ACIS_NEW SPAposition[m_Max_num_pts_so_far];
				m_pParam_vals = ACIS_NEW double[m_Max_num_pts_so_far];
				finished = FALSE;
			}
		}
		HPS::Key key = MakeHoopsPolyline( num_pts, m_pEdge_points, FALSE, in_segment_key );
		AddMapping( key, in_ent, in_owner );
	}
} // end of ConvertCURVE()

void hps_acis_entity_converter::ConvertELLIPSE(
	ELLIPSE*		in_ellipse,  // Could be actual CURVE or the CURVE of an EDGE.
	ENTITY*			in_ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
	SPAparameter	in_start_param,
	SPAparameter	in_end_param,
	HPS::SegmentKey	in_segment_key )
{
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	SPAvector major_axis;
	SPAvector minor_axis;
	SPAvector normal;
	SPAposition center;
	double radius_ratio = in_ellipse->radius_ratio();;
	float hstart, hend;
	major_axis = in_ellipse->major_axis() * m_BodyTransf;
	normal = in_ellipse->normal() * m_BodyTransf;
	minor_axis = normal * major_axis * radius_ratio;
	center = in_ellipse->centre() * m_BodyTransf;
	HPS::Point	hcenter( (float)center.x(), (float)center.y(), (float)center.z() );
	HPS::Point	hmajor(
		(float)( major_axis.x() + center.x() ),
		(float)( major_axis.y() + center.y() ),
		(float)( major_axis.z() + center.z() ) );
	HPS::Point	hminor(
		(float)( minor_axis.x() + center.x() ),
		(float)( minor_axis.y() + center.y() ),
		(float)( minor_axis.z() + center.z() ) );
	//	const curve &ell_curve=in_ellipse->equation();
	//	SPAinterval curve_interval(ell_curve.param_range());
	//	double length=curve_interval.length();
	//	hstart=(float)(-0.5+(in_start_param-curve_interval.start_pt())/length);
	//	hend=(float)(-0.5+(in_end_param-curve_interval.start_pt())/length);
	hstart = (float)( ( in_start_param ) / ( 2 * M_PI ) );
	hend = (float)( ( in_end_param ) / ( 2 * M_PI ) );
	// Round
	int s = (int)( hstart*ROUND_TO );
	int e = (int)( hend*ROUND_TO );
	hstart = (float)( (float)s ) / ROUND_TO;
	hend = (float)( (float)e ) / ROUND_TO;
	// Check for periocity
	int offset = (int)hstart;
	if ( offset )
	{
		hstart -= offset;
		hend -= offset;
	}
	if ( hstart > hend )
		sys_error( HPS_MSG_ELLIPSE_OUT_OF_RANGE );
	// Within range.
	fp_sentry fps;
	if ( hstart >= 0.0 && hend <= 1.0 )                                                                                                                                                                                         // 0.0 <= hstart < hend <= 1.0
	{
		HPS::EllipticalArcKey key = in_segment_key.InsertEllipticalArc( hcenter, hmajor, hminor, hstart, hend );
		AddMapping( key, in_ent );
	} else if ( hstart < 0.0 && hend > 0.0 )
	{                                                                                                                                                                                        // hstart < 0.0 < hend
		HPS::EllipticalArcKey key = in_segment_key.InsertEllipticalArc( hcenter, hmajor, hminor, 1.0f + hstart, 1.0f );
		AddMapping( key, in_ent );
		key = in_segment_key.InsertEllipticalArc( hcenter, hmajor, hminor, 0.0f, hend );
		AddMapping( key, in_ent );
	} else if ( hend <= 0.0 )
	{                                                                                                                                                                                              // hstart < hend <= 0.0
		HPS::EllipticalArcKey key = in_segment_key.InsertEllipticalArc( hcenter, hmajor, hminor, hstart + 1.0f, hend + 1.0f );
		AddMapping( key, in_ent );
	} else if ( 1.0 <= hend )
	{                                                                                                                                                                                           // hstart < 1.0 <= hend.
		HPS::EllipticalArcKey key = in_segment_key.InsertEllipticalArc( hcenter, hmajor, hminor, hstart, 1.0f );
		AddMapping( key, in_ent );
		key = in_segment_key.InsertEllipticalArc( hcenter, hmajor, hminor, 0.0f, hend - 1.0f );
		AddMapping( key, in_ent );
	}
} // end of ConvertELLIPSE

void hps_acis_entity_converter::Convert_intcurve(
	CURVE*			in_cur,  // Could be actual CURVE or the CURVE of an EDGE.
	ENTITY*			in_ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
	SPAparameter	in_start_param,
	SPAparameter	in_end_param,
	CurveFacetLevel	in_facet_level,
	HPS::SegmentKey	in_segment_key )
{
	SPAUNUSED( in_facet_level )
		curve const &cur_ptr = in_cur->equation();
	if ( cur_ptr.type() == intcurve_type )
	{
		bs3_curve bs = ( (intcurve const *)&cur_ptr )->cur();
		if ( bs )
		{
			if ( ( (intcurve const *)&cur_ptr )->reversed() )
			{
				SPAparameter temp = -in_start_param;
				in_start_param = -in_end_param;
				in_end_param = temp;
			}
			int dim, deg, npts, nkts;
			logical rat;
			SPAposition *pts = 0;
			double *wts = 0;
			double *kts = 0;
			HPS::PointArray ControlPoints;
			HPS::FloatArray weights;
			HPS::FloatArray knots;
			API_SYS_BEGIN;
			{
				bs3_curve_to_array( bs, dim, deg, rat, npts, pts, wts, nkts, kts );
				ControlPoints.reserve( npts );
				ConvertSPAPositionToPointArray( npts, pts, ControlPoints );
				weights.reserve( npts );
				DoubleToFloatArray( npts, wts, weights );
				knots.reserve( nkts );
				DoubleToFloatArray( nkts, kts, knots );
				SPAinterval range = bs3_curve_range( bs );
				double start_range = range.start_pt();
				double end_range = range.end_pt();
				double length = end_range - start_range;
				float start = (float)( ( in_start_param - start_range ) / length );
				float end = (float)( ( in_end_param - start_range ) / length );
				// Round
				int s = (int)( start*ROUND_TO );
				int e = (int)( end*ROUND_TO );
				start = (float)( (float)s ) / ROUND_TO;
				end = (float)( (float)e ) / ROUND_TO;
				HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
				// Check for periocity
				int offset = (int)start;
				if ( offset )
				{
					start -= offset;
					end -= offset;
				}
				float diff = end - start;
				if ( diff > ( float )1.0 + (float)SPAresabs )
					sys_error( HPS_MSG_PARAM_OUT_OF_RANGE );
				if ( start > end )
					sys_error( HPS_MSG_PARAM_OUT_OF_RANGE );
				fp_sentry fps;
				// Within range.
				HPS::NURBSCurveKey nkey( key );
				if ( start >= ( float )0.0 && end <= ( float )1.0 )                                                                                                                                                                                                                                                                                                                                                                                                                                                                                // 0.0 <= start < hend <= 1.0
				{
					nkey = in_segment_key.InsertNURBSCurve( deg, ControlPoints, weights, knots, start, end );
					AddMapping( nkey, in_ent );
				} else if ( ( start < ( float )0.0 ) && ( end >( float )0.0 ) )
				{                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // start < 0.0 < hend
					nkey = in_segment_key.InsertNURBSCurve( deg, ControlPoints, weights, knots, 1.0f + start, 1.0f );
					AddMapping( nkey, in_ent );
					nkey = in_segment_key.InsertNURBSCurve( deg, ControlPoints, weights, knots, 0.0f, end );
					AddMapping( nkey, in_ent );
				} else if ( end <= ( float )0.0 )
				{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      // hstart < hend <= 0.0
					nkey = in_segment_key.InsertNURBSCurve( deg, ControlPoints, weights, knots, start + 1.0f, end + 1.0f );
					AddMapping( nkey, in_ent );
				} else if ( ( float )1.0 <= end )
				{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // hstart < 1.0 <= hend.
					nkey = in_segment_key.InsertNURBSCurve( deg, ControlPoints, weights, knots, start, 1.0f );
					AddMapping( nkey, in_ent );
					nkey = in_segment_key.InsertNURBSCurve( deg, ControlPoints, weights, knots, 0.0, end - 1.0f );
					AddMapping( nkey, in_ent );
				} else // ???????
					sys_error( HPS_MSG_PARAM_OUT_OF_RANGE );
				//// Shift range so that > 0.
				//if ( start < 0.0 )
				//{
				//	end += ( -start );
				//	start += ( -start );
				//}
				//do
				//{
				//	if ( start < end )
				//	{
				//		double this_end = end;
				//		if ( end > 1.0 )
				//			this_end = 1.0;
				//		if ( start >= this_end )
				//			start = this_end;
				//		HPS::SegmentKey key = HX_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, start, this_end );
				//		AddMapping( key, in_ent );
				//	}
				//	start = 0.0;
				//	end -= 1.0;
				//} while ( end > 1.0 );

			} API_SYS_END;
			ACIS_DELETE[] pts; pts = 0;
			ACIS_DELETE[] STD_CAST wts; wts = 0;
			ACIS_DELETE[] STD_CAST kts; kts = 0;
		}
	}
} // end of Convert_intcurve()

void hps_acis_entity_converter::AddMapping(
	HPS::Key	in_key,
	ENTITY *	in_entity,
	ENTITY *	in_owner )
{
	if ( m_Map && m_EnableMapping )
		m_Map->AddMapping( in_key, in_entity, in_owner );
}

void hps_acis_entity_converter::ReRenderVisibilityAsm( const hps_rendering_options & in_ro )
{
	const char * pattern = in_ro.GetPattern();
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	if ( pattern )
		key = HPS_Open_Segment( pattern );
	if ( HPS_Is_Valid_Segment_Key( key ) )
	{	// Faces
		if ( in_ro.GetRenderFacesMode() )
			key.GetConditionControl().UnsetCondition( "AcisFacesOffSilsOn" ).UnsetCondition( "AcisFacesOffSilsOff" ).AddCondition( "AcisFacesOn" );
		else if ( FALSE /*face_silhouettes*/ )
			key.GetConditionControl().UnsetCondition( "AcisFacesOn" ).UnsetCondition( "AcisFacesOffSilsOff" ).AddCondition( "AcisFacesOffSilsOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisFacesOn" ).UnsetCondition( "AcisFacesOffSilsOn" ).AddCondition( "AcisFacesOffSilsOff" );
		// Edges
		if ( in_ro.GetRenderEdgesMode() )
			key.GetConditionControl().UnsetCondition( "AcisEdgesOff" ).AddCondition( "AcisEdgesOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisEdgesOn" ).AddCondition( "AcisEdgesOff" );
		// Vertices
		if ( in_ro.GetRenderVerticesMode() )
			key.GetConditionControl().UnsetCondition( "AcisVerticesOff" ).AddCondition( "AcisVerticesOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisVerticesOn" ).AddCondition( "AcisVerticesOff" );
		// Coedges
		if ( in_ro.GetRenderCoedgeMode() )
			key.GetConditionControl().UnsetCondition( "AcisCoedgesOff" ).AddCondition( "AcisCoedgesOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisCoedgesOn" ).AddCondition( "AcisCoedgesOff" );
		// Tcoedges
		if ( in_ro.GetRenderTCoedgeMode() )
			key.GetConditionControl().UnsetCondition( "AcisTCoedgesOff" ).AddCondition( "AcisTCoedgesOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisTCoedgesOn" ).AddCondition( "AcisTCoedgesOff" );
		// APOINTs
		if ( in_ro.GetRenderAPOINTsMode() )
			key.GetConditionControl().UnsetCondition( "AcisAPOINTsOff" ).AddCondition( "AcisAPOINTsOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisAPOINTsOn" ).AddCondition( "AcisAPOINTsOff" );
		// Text
		if ( in_ro.GetRenderTextMode() )
			key.GetConditionControl().UnsetCondition( "AcisTextOff" ).AddCondition( "AcisTextOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisTextOn" ).AddCondition( "AcisTextOff" );
		// WCSs
		if ( in_ro.GetRenderWCSsMode() )
			key.GetConditionControl().UnsetCondition( "AcisWCSsOff" ).AddCondition( "AcisWCSsOn" );
		else
			key.GetConditionControl().UnsetCondition( "AcisWCSsOn" ).AddCondition( "AcisWCSsOff" );
	}
} // end of ReRenderVisibilityAsm()

HPS::ShellKey hps_acis_entity_converter::SequentialMeshToHOOPS( HPS::SegmentKey in_segment_key, af_serializable_mesh *in_Mesh, FACE * /*face*/ )
{
	HPS::ShellKey	ShellKey;
	if ( !in_Mesh )
		return( ShellKey );
	fp_sentry fps;
	// First find the number of polys.
	int polygonCount = in_Mesh->number_of_polygons();
	// Get the number of vertices off all the polys.
	int vertexCount = in_Mesh->number_of_vertices();
	//int polynodeCount = mesh->get_num_polynode();
	// Guy. Fixes 89417.
	if ( ( polygonCount < 1 ) || ( vertexCount < 3 ) )
		return( ShellKey );
	EXCEPTION_BEGIN;
	float *	points = 0;
	int *	pFacetConnectivity = 0;
	double*	normals = 0;
	float*	texture_uvs = 0;
	EXCEPTION_TRY;
	{
		points = ACIS_NEW float[vertexCount * 3];
		in_Mesh->serialize_positions( points );
		int polynodeCount = in_Mesh->number_of_polygon_coedges();
		// Build the connectivity array.
		pFacetConnectivity = ACIS_NEW int[polygonCount + polynodeCount];
		in_Mesh->serialize_polygons_for_hoops( pFacetConnectivity );
		// Create the HOOPS rep.
		{
			HPS::PointArray PtArray;
			PtArray.reserve( vertexCount );
			ConvertFloatsToPointArray( vertexCount * 3, points, PtArray );
			int				FaceListCount = polygonCount + polynodeCount;
			HPS::IntArray	FaceList;
			FaceList.reserve( FaceListCount );
			for ( int i = 0; i < FaceListCount; ++i )
				FaceList.push_back( pFacetConnectivity[i] );
			// kev. Added fp-sentry. Hoops was causing fp-exceptions.
			fp_sentry		fps;
			ShellKey = in_segment_key.InsertShell( PtArray, FaceList );
		}
		if ( !HPS_Is_Valid_Key( ShellKey ) )
			return( ShellKey );
		// Normals and uv parameter
		normals = ACIS_NEW double[3 * vertexCount];
		in_Mesh->serialize_normals( normals );
		texture_uvs = ACIS_NEW float[2 * vertexCount];
		in_Mesh->serialize_uv_data( texture_uvs, TRUE );
		HPS::VectorArray	NormalArray;
		NormalArray.reserve( vertexCount );
		HPS::SizeTArray		VertexArray;
		VertexArray.reserve( vertexCount );
		HPS::FloatArray		UvArray;
		UvArray.reserve( vertexCount );
		for ( int i = 0; i < vertexCount; ++i )
		{
			VertexArray.push_back( i );
			int NormalCounter = i * 3;
			NormalArray.push_back( HPS::Vector( (float)normals[NormalCounter], (float)normals[NormalCounter + 1], (float)normals[NormalCounter + 2] ) );
			int UvCounter = i * 2;
			UvArray.push_back( texture_uvs[UvCounter] );
			UvArray.push_back( texture_uvs[UvCounter + 1] );
		}
		ShellKey.SetVertexNormalsByList( VertexArray, NormalArray );
		ShellKey.SetVertexParametersByList( VertexArray, UvArray );
	} EXCEPTION_CATCH_TRUE;
	{
		ACIS_DELETE[] STD_CAST normals; normals = 0;
		ACIS_DELETE[] STD_CAST points; points = 0;
		ACIS_DELETE[] STD_CAST pFacetConnectivity; pFacetConnectivity = 0;
		ACIS_DELETE[] STD_CAST texture_uvs; texture_uvs = 0;
	} EXCEPTION_END;
	return( ShellKey );
} // end of SequentialMeshToHOOPS

void hps_acis_entity_converter::DoubleToFloatArray( int in_num_pts, double * in_doubles, HPS::FloatArray & out_floats )
{
	if ( ( in_num_pts > 0 ) && in_doubles )
		for ( int i = 0; i < in_num_pts; ++i )
			out_floats.push_back( (float)in_doubles[i] );
} // end of ConvertDoubleToFloatArray
