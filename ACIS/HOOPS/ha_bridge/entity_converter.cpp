/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// History : 
// Sep-06-12 bta : 93941, hoops_acis_entity_converter::ConvertCoedges,
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

#include "ha_bridge.h"
#include "entity_converter.h"
#include "ha_util.h"
#include "ha_point.h"
#include "ha_map.h"
#include "ha_bridge.err"

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
#include "ha_bridge_internal.h"
#include "option.hxx"
#include "rlt_util.hxx"
#include "rnd_api.hxx"	// required to use class rgb_color
#include "ga_api.hxx"
#include "direct_render_mesh_manager.hxx"

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
HC_KEY hoops_acis_entity_converter::ConvertEntity(
	ENTITY *entity,
	const ha_rendering_options &ro,
	HA_Map *map,
	const char* pattern
	)
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return 0;
	m_Map = map;
	m_RenderingOptions = ro;
	m_Pattern = pattern;
	curve_pixel_tol = -1; // flush the cached value
	logical color_segment_open = FALSE;
	HC_KEY key = 0;
	key = HA_KOpen_Segment( entity, m_Pattern );
	AddMapping( key, entity );
	if ( is_FACET_BODY( entity ) )
	{
		FACET_BODY* pb = static_cast<FACET_BODY*>( entity );
		HA_Render_FACET_BODY( pb, m_RenderingOptions.GetRenderFacesMode(), m_RenderingOptions.GetRenderEdgesMode() );
		HC_Close_Segment();
		return key;
	}
	API_BEGIN
	{
		m_pEntity = entity;
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
		//if (m_RenderingOptions.GetRenderFacesMode() || m_lSilhouette)
		ConvertLumps();
		if ( is_APOINT( m_pEntity ) && m_RenderingOptions.GetRenderAPOINTsMode() )
		{
			fp_sentry fps;
			APOINT* pt = (APOINT*)m_pEntity;
			SPAposition pos = pt->coords();
			HC_KEY key = HC_KInsert_Marker( pos.x(), pos.y(), pos.z() );
			AddMapping( key, m_pEntity );
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
				switch ( light_type )
				{
				case EYE_LIGHT:
				{
					fp_sentry fps;
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
					fp_sentry fps;
					HC_KEY key = HC_KInsert_Local_Light( Pos.x(), Pos.y(), Pos.z() );
					AddMapping( key, m_pEntity );
				}
				break;
				case AMBIENT_LIGHT:
				{
					fp_sentry fps;
					HC_Set_Color_By_Value( "ambient", "RGB", Color.red(), Color.green(), Color.blue() );
				}
				break;
				case SPOT_LIGHT:
				{
					fp_sentry fps;
					HA_Point pos( Pos );
					// Catch the special case if the dir = 0.0.  It is illegal to set a hoops spotlight
					// with the same target and position.  If we see this, we will default the dir to 0.0 0.0 1.0f
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
					fp_sentry fps;
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
	} API_END;
	if ( color_segment_open )
		HC_Close_Segment();
	HC_Close_Segment(); // Top level entity
	check_outcome( result );
	return key;
}

void hoops_acis_entity_converter::ConvertLumps()
{
	ENTITY_LIST lumps;
	outcome result = api_get_lumps( m_pEntity, lumps );
	// This things has no lumps, so pass it down.
	if ( lumps.iteration_count() < 1 )
		ConvertShells( m_pEntity );
	lumps.init();
	ENTITY* lump = 0;
	while ( ( lump = lumps.next() ) != 0 )
	{
		logical color_segment_open = OpenColorSegment( lump );
		ConvertShells( lump );
		// Restore the old mp/dp. Since we're at the lump level and the body takes care of its own mp/dp most likely this is not needed.  But I put it in to be consistant.
		if ( color_segment_open )
			HC_Close_Segment();
	}
}

void hoops_acis_entity_converter::ConvertShells( ENTITY * entity /*most likely a lump*/ )
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return;
	ENTITY_LIST shells;
	outcome result = api_get_shells( entity, shells );
	//	if ( !result.ok())
	//		VM_THROW_OUTCOME(result);  //No cleanup needed
	// This thing has no shells, so pass it down.
	if ( shells.iteration_count() < 1 )
	{
		if ( m_RenderingOptions.GetRenderFacesMode() || m_ModelGeometryMode )
			ConvertFaces( entity );
		if ( m_RenderingOptions.GetRenderCoedgeMode() )
			ConvertCoedges( entity );
		if ( m_RenderingOptions.GetRenderEdgesMode() || m_ModelGeometryMode )
			ConvertEdges( entity );
		if ( m_RenderingOptions.GetRenderTCoedgeMode() )
			ConvertTCoedges( entity );
		if ( m_RenderingOptions.GetRenderVerticesMode() || m_ModelGeometryMode )
			ConvertVertices( entity );
	}
	shells.init();
	SHELL* shell = 0;
	while ( ( shell = (SHELL*)shells.next() ) != 0 )
	{
		logical color_segment_open = OpenColorSegment( shell );
		if ( shell->wire() && m_RenderingOptions.GetRenderCoedgeMode() )
			ConvertCoedges( shell );
		if ( m_RenderingOptions.GetRenderFacesMode() || m_ModelGeometryMode )
			ConvertFaces( shell );
		if ( m_RenderingOptions.GetRenderCoedgeMode() )
			ConvertCoedges( entity );
		if ( m_RenderingOptions.GetRenderEdgesMode() || m_ModelGeometryMode )
			ConvertEdges( shell );
		if ( m_RenderingOptions.GetRenderTCoedgeMode() )
			ConvertTCoedges( shell );
		if ( m_RenderingOptions.GetRenderVerticesMode() || m_ModelGeometryMode )
			ConvertVertices( shell );
		if ( color_segment_open )
			HC_Close_Segment();
	}
}

void Delete_Geometry_Mappings( HA_Map* map, ENTITY* ent )
{	// Find all geometry in the currently open segment and delete the associated mappings to ent.
	if ( map )
	{
		HC_Begin_Contents_Search( ".", "geometry" );
		HC_KEY geom_key = 0;
		char type[64];
		while ( HC_Find_Contents( type, &geom_key ) )
		{
			if ( geom_key )
			{
				ENTITY* found_ent = map->FindMapping( geom_key );
				if ( found_ent == ent )
				{
					map->DeleteMapping( geom_key, ent );
					HC_Delete_By_Key( geom_key );
				}
			}
		}

		HC_End_Contents_Search();
	}
}

extern HA_RENDER_progress_info * progress_meter;

#include "idx_mesh.hxx"

LOCAL_PROC void convert_faces_no_direct_rendering( hoops_acis_entity_converter* converter, ENTITY* entity, logical m_ModelGeometryMode, HA_Map* m_Map )
{
	ENTITY_LIST faces;
	outcome result = api_get_faces( entity, faces );
	ENTITY_LIST unfaceted_faces;
	{for ( ENTITY* f = faces.first(); f; f = faces.next() )
	{
		af_serializable_mesh* mesh = GetSerializableMesh( (FACE*)f );
		if ( NULL == mesh )
			unfaceted_faces.add( f );
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
			facet_options* fo = HA_Get_Facet_Options();
			api_facet_entities( entity, &unfaceted_faces, fo );
		} API_SYS_END;
	}
	for ( FACE* face = (FACE*)faces.first(); face; face = (FACE*)faces.next() )
	{	// Loop through each face and convert the INDEXED_MESH to HOOPS.
		if ( find_NORENDER_ATTRIB( face ) != NULL )
			continue;
		if ( progress_meter )
			progress_meter->update();
		logical color_segment_open = OpenColorSegment( face );
		// If this is a double sided face set polygon handeness to none.
		// Guy. Addresses concerns posed in BTS 87839. Use backplane culling instead of no polygon handedness
		if ( face->sides() == DOUBLE_SIDED )
			HC_Set_Heuristics( "no backplane cull" );
		API_SYS_BEGIN;
		{	// Facet, convert to HPS, and add to bridge mapping...
			if ( m_ModelGeometryMode && use_asm_highlight_segments.on() )
			{   // In assembly mode, different parts can share geometry.  To avoid having duplications, always clear the existing geometry.
				HA_KOpen_Segment( face, "entity" );
				Delete_Geometry_Mappings( m_Map, face );
				char buffer[64];
				HA_Build_Segment_String( face, buffer, "entity" );
				HC_Conditional_Style( "?Style Library/AcisAsmHighlightFacesEdges", buffer );
			}
			if ( render_lean & 2 )
			{	// Make sure all is faceted...
				facet_options* fo = HA_Get_Facet_Options();
				FacetEntity( face, TRUE, *(int*)NULL_REF, *(int*)NULL_REF,
							 *(unsigned *)NULL_REF, fo );
			}
			af_serializable_mesh *mesh = GetSerializableMesh( face );	// Get the MESH from the FACE
			HC_KEY key = HA_K_SEQUENTIAL_MESH_to_HOOPS( mesh, face );
			if ( render_lean & 1 )
				api_delete_entity_facets( face );
			if ( result.ok() && key )
				converter->AddMapping( key, face );
			if ( m_ModelGeometryMode && use_asm_highlight_segments.on() )
				HC_Close_Segment();
		} API_SYS_END;
		if ( color_segment_open )
			HC_Close_Segment();
	}
}

option_header hoops_direct_render( "hoops_direct_render", FALSE );

void hoops_acis_entity_converter::ConvertFaces( ENTITY * entity /*most likely a shell*/ )
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return;
	// Create a segment to hold all faces for this body/shell. This allows us to turn off markers at this level. Avoid creating a new unnamed segment if one already exists
	if ( m_ModelGeometryMode )
	{
		HC_Open_Segment( "SPA faces" );
		HC_Conditional_Style( "?Style Library/AcisShellsOn", "AcisFacesOn" );
		HC_Conditional_Style( "?Style Library/AcisShellsOffSilsOn", "AcisFacesOffSilsOn" );
		HC_Conditional_Style( "?Style Library/AcisShellsOffSilsOff", "AcisFacesOffSilsOff" );
	} else
		HC_KOpen_Segment( "" );
	HC_Set_Visibility( "markers = off" );
	if ( hoops_direct_render.on() )
		direct_render_entity( this, entity, m_ModelGeometryMode, m_Map );
	else
		convert_faces_no_direct_rendering( this, entity, m_ModelGeometryMode, m_Map );
	HC_Close_Segment();
}

void hoops_acis_entity_converter::ConvertSilhouettes(
	FACE*						face,
	const SPAtransf*			tform,
	SPAposition&				eye,
	SPAposition&				target,
	int							projection,
	bool						mapping )
{
	ENTITY_LIST edge_list;
	API_NOP_BEGIN;
	{
		api_silhouette_edges( face, *tform, eye, target, projection, &edge_list );
		logical color_segment_open = OpenColorSegment( face );
		ENTITY *toplevel = NULL;
		api_get_owner( face, toplevel );
		EDGE* edge = NULL;
		for ( int i = 0; i < edge_list.count(); i++ )
		{
			edge = (EDGE*)edge_list[i];
			m_EnableMapping = mapping;
			api_add_generic_named_attribute( edge, "HA_SilhouetteFace", face );
			ConvertEdges( edge, toplevel );
			m_EnableMapping = TRUE;
		}
		if ( color_segment_open )
			HC_Close_Segment();
	} API_NOP_END;
}

void hoops_acis_entity_converter::ConvertEdges(
	ENTITY*						entity,
	ENTITY*						owner )
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return;
	// Avoid creating a new unnamed segment if one already exists
	if ( m_ModelGeometryMode )
	{
		HC_Open_Segment( "SPA edges" );
		HC_Conditional_Style( "?Style Library/AcisPolylinesOn", "AcisEdgesOn" );
		HC_Conditional_Style( "?Style Library/AcisPolylinesOff", "AcisEdgesOff" );
	}
	ENTITY_LIST edges;
	outcome result = api_get_edges( entity, edges );
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
		logical color_segment_open = OpenColorSegment( edge );
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{    // In assembly mode, different parts can share geometry.  To avoid having duplications, always clear the existing geometry.
				Delete_Geometry_Mappings( m_Map, edge );
				char buffer[64];
				HA_Build_Segment_String( edge, buffer, "entity" );
				HA_KOpen_Segment( edge, "entity" );
				HC_Conditional_Style( "?Style Library/AcisAsmHighlightFacesEdges", buffer );
			}
		}
		int num_pts = 0;
		if ( facet_level == cfl_face_facets )
		{	// We're going to try for the facets first.
			SPAposition *edge_points = 0;
			result = api_get_facet_edge_points( edge, edge_points, num_pts );
			if ( !edge_points )
				facet_level = cfl_hoops_primitives;
			else
			{
				HC_KEY key = MakeHoopsPolyline( num_pts, edge_points );
				AddMapping( key, edge, owner );
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
			ConvertCURVE( edge_geom, edge, start_param, end_param, facet_level, owner );
		}
		if ( m_ModelGeometryMode )
			if ( use_asm_highlight_segments.on() )
				HC_Close_Segment();
		// Restore the old mp/dp.
		// Since we're at the lump level and the body takes care of its own mp/dp most likely this is not needed.  But I put it in to be consistant.
		if ( color_segment_open )
			HC_Close_Segment();
	}
	if ( m_ModelGeometryMode )
		HC_Close_Segment();
}

void hoops_acis_entity_converter::ConvertCoedges(
	ENTITY*						entity )
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return;
	if ( m_ModelGeometryMode )
	{
		HC_Open_Segment( "SPA coedges" );
		HC_Conditional_Style( "?Style Library/AcisPolylinesOn", "AcisCoedgesOn" );
		HC_Conditional_Style( "?Style Library/AcisPolylinesOff", "AcisCoedgesOff" );
	}
	ENTITY_LIST coedges;
	outcome result = api_get_coedges( entity, coedges );
	CurveFacetLevel facet_level = m_RenderingOptions.GetCurveFacetLevel();
	coedges.init();
	COEDGE* coedge = 0;
	coedges.init();
	// Now get the edge points
	while ( ( coedge = (COEDGE *)coedges.next() ) != 0 )
	{
		FACE* face = 0;
		if ( coedge->loop() && coedge->loop()->face() )
			face = coedge->loop()->face();
		EDGE *edge = coedge->edge();
		logical color_segment_open = OpenColorSegment( coedge );
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{    // In assembly mode, different parts can share geometry.  To avoid
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
			const SPAtransf tform = get_owner_transf( entity );
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
			HC_KEY key = MakeHoopsPolyline( num_pts, m_pEdge_points );
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
			HC_KEY key2 = MakeHoopsPolyline( 3, arrow_pts );
			AddMapping( key2, coedge );
		}
		if ( color_segment_open )
			HC_Close_Segment();
	}
	if ( m_ModelGeometryMode )
		HC_Close_Segment();
}

void hoops_acis_entity_converter::ConvertTCoedges(
	ENTITY*						entity )
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return;
	if ( m_ModelGeometryMode )
	{
		HC_Open_Segment( "SPA tcoedges" );
		HC_Conditional_Style( "?Style Library/AcisPolylinesOn", "AcisTCoedgesOn" );
		HC_Conditional_Style( "?Style Library/AcisPolylinesOff", "AcisTCoedgesOff" );
	}
	ENTITY_LIST tcoedges;
	outcome result = api_get_tcoedges( entity, tcoedges );
	CurveFacetLevel facet_level = m_RenderingOptions.GetCurveFacetLevel();
	tcoedges.init();
	TCOEDGE* tcoedge = 0;
	while ( ( tcoedge = (TCOEDGE *)tcoedges.next() ) != 0 )
	{
		logical color_segment_open = OpenColorSegment( tcoedge );
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{   // In assembly mode, different parts can share geometry.  To avoid
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
			const SPAtransf tform = get_owner_transf( entity );
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
		HC_KEY key = MakeHoopsPolyline( num_pts, m_pEdge_points );
		AddMapping( key, tcoedge );
		if ( color_segment_open )
			HC_Close_Segment();
	}
	if ( m_ModelGeometryMode )
		HC_Close_Segment();
}

void hoops_acis_entity_converter::ConvertVertices(
	ENTITY*						entity )
{
	if ( find_NORENDER_ATTRIB( entity ) != NULL )
		return;
	fp_sentry fps;
	ENTITY_LIST vertices;
	outcome result = api_get_vertices( entity, vertices );
	// create a segment to hold all vertices for this body
	// Avoid creating a new unnamed segment if one already exists
	if ( m_ModelGeometryMode )
	{
		HC_Open_Segment( "SPA vertices" );
		HC_Conditional_Style( "?Style Library/AcisMarkersOn", "AcisVerticesOn" );
		HC_Conditional_Style( "?Style Library/AcisMarkersOff", "AcisVerticesOff" );
	} else
		HC_KOpen_Segment( "" );
	vertices.init();
	VERTEX* vertex = 0;
	while ( ( vertex = (VERTEX*)vertices.next() ) != 0 )
	{
		if ( find_NORENDER_ATTRIB( entity ) != NULL )
			continue;
		logical color_segment_open = OpenColorSegment( vertex );
		if ( m_ModelGeometryMode )
		{
			if ( use_asm_highlight_segments.on() )
			{   // In assembly mode, different parts can share geometry.  To avoid having duplications, always clear the existing geometry.
				HA_KOpen_Segment( vertex, "entity" );
				Delete_Geometry_Mappings( m_Map, vertex );
				char buffer[64];
				HA_Build_Segment_String( vertex, buffer, "entity" );
				HC_Conditional_Style( "?Style Library/AcisAsmHighlightVerts", buffer );
			}
		}
		APOINT* pt = vertex->geometry();
		SPAposition pos = pt->coords();
		//pos = pos * transform;
		HC_KEY key = HC_KInsert_Marker( pos.x(), pos.y(), pos.z() );
		AddMapping( key, vertex );
		if ( m_ModelGeometryMode )
			if ( use_asm_highlight_segments.on() )
				HC_Close_Segment();
		if ( color_segment_open )
			HC_Close_Segment();
	}
	HC_Close_Segment();  // Vertices segment.
}

HC_KEY hoops_acis_entity_converter::MakeHoopsPolyline( int num_pts, SPAposition* points )
{
	HC_KEY key = 0;
	if ( !( num_pts > 0 && points ) )
		return 0;
	float* float_points = 0;
	ConvertSPAPositionToFloatArray( num_pts, points, float_points );
	fp_sentry fps;
	key = HC_KInsert_Polyline( num_pts, float_points );
	ACIS_DELETE[] STD_CAST float_points; float_points = 0;
	return key;
}

void hoops_acis_entity_converter::ConvertCURVE(
	CURVE*			CUR,
	ENTITY*			ent,
	SPAparameter	start_param,
	SPAparameter	end_param,
	CurveFacetLevel	facet_level,
	ENTITY*			owner )
{

	if ( facet_level == cfl_hoops_primitives )
	{
		if ( CUR->identity() == ELLIPSE_TYPE )
			ConvertELLIPSE( (ELLIPSE*)CUR, ent, start_param, end_param );
		//#pragma message (Reminder "Uncomment out the following when 71910/2812 is resolved.")
		else if ( CUR->identity() == INTCURVE_TYPE )
			Convert_intcurve( CUR, ent, start_param, end_param, cfl_screen );
		else
			ConvertCURVE( CUR, ent, start_param, end_param, cfl_screen );
	} else
	{
		double tolerance = curve_pixel_tol;
		if ( tolerance < 0 )
			curve_pixel_tol = tolerance = m_RenderingOptions.GetCurveFacetTol(); //cfl_world
		const SPAtransf tform = get_owner_transf( ent );
		tolerance /= tform.scaling();
		if ( tolerance < SPAresabs )
			tolerance = 10 * SPAresabs;
		logical finished = FALSE;
		int num_pts = 0;
		const EDGE* edge = (EDGE*)ent;
		if ( is_EDGE( ent ) )
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
			curve const &cur_ptr = CUR->equation();
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
		HC_KEY key = MakeHoopsPolyline( num_pts, m_pEdge_points );
		AddMapping( key, ent, owner );
	}
}

void hoops_acis_entity_converter::ConvertELLIPSE(
	ELLIPSE*		ellipse,  // Could be actual CURVE or the CURVE of an EDGE.
	ENTITY*			ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
	SPAparameter	start_param,
	SPAparameter	end_param )
{
	HC_KEY key = 0;
	SPAvector major_axis;
	SPAvector minor_axis;
	SPAvector normal;
	SPAposition center;
	double radius_ratio;
	float hstart, hend;
	float hcenter[3];
	float hmajor[3];
	float hminor[3];
	radius_ratio = ellipse->radius_ratio();
	major_axis = ellipse->major_axis();
	normal = ellipse->normal();
	minor_axis = normal * major_axis * radius_ratio;
	center = ellipse->centre();
	hcenter[0] = (float)center.x();
	hcenter[1] = (float)center.y();
	hcenter[2] = (float)center.z();
	hmajor[0] = (float)( major_axis.x() + center.x() );
	hmajor[1] = (float)( major_axis.y() + center.y() );
	hmajor[2] = (float)( major_axis.z() + center.z() );
	hminor[0] = (float)( minor_axis.x() + center.x() );
	hminor[1] = (float)( minor_axis.y() + center.y() );
	hminor[2] = (float)( minor_axis.z() + center.z() );
	//	const curve &ell_curve=ellipse->equation();
	//	SPAinterval curve_interval(ell_curve.param_range());
	//	double length=curve_interval.length();
	//	hstart=(float)(-0.5+(start_param-curve_interval.start_pt())/length);
	//	hend=(float)(-0.5+(end_param-curve_interval.start_pt())/length);
	hstart = (float)( ( start_param ) / ( 2 * M_PI ) );
	hend = (float)( ( end_param ) / ( 2 * M_PI ) );
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
		sys_error( HA_BRIDGE_ELLIPSE_OUT_OF_RANGE );
	// Within range.
	fp_sentry fps;
	if ( hstart >= 0.0 && hend <= 1.0 ) // 0.0 <= hstart < hend <= 1.0
	{
		key = HC_KInsert_Elliptical_Arc( hcenter, hmajor, hminor, hstart, hend );
		AddMapping( key, ent );
	} else if ( hstart < 0.0 && hend > 0.0 ) // hstart < 0.0 < hend
	{
		key = HC_KInsert_Elliptical_Arc( hcenter, hmajor, hminor, 1.0 + hstart, 1.0f );
		AddMapping( key, ent );
		key = HC_KInsert_Elliptical_Arc( hcenter, hmajor, hminor, 0.0f, hend );
		AddMapping( key, ent );
	} else if ( hend <= 0.0 )   // hstart < hend <= 0.0 
	{
		key = HC_KInsert_Elliptical_Arc( hcenter, hmajor, hminor, hstart + 1.0, hend + 1.0 );
		AddMapping( key, ent );
	} else if ( 1.0 <= hend )  // hstart < 1.0 <= hend.
	{
		key = HC_KInsert_Elliptical_Arc( hcenter, hmajor, hminor, hstart, 1.0f );
		AddMapping( key, ent );
		key = HC_KInsert_Elliptical_Arc( hcenter, hmajor, hminor, 0.0f, hend - 1.0 );
		AddMapping( key, ent );
	} else // ???????
		sys_error( HA_BRIDGE_ELLIPSE_OUT_OF_RANGE );
}

void hoops_acis_entity_converter::Convert_intcurve(
	CURVE*			cur,  // Could be actual CURVE or the CURVE of an EDGE.
	ENTITY*			ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
	SPAparameter	start_param,
	SPAparameter	end_param,
	CurveFacetLevel	facet_level )
{
	SPAUNUSED( facet_level )
		curve const &cur_ptr = cur->equation();
	if ( cur_ptr.type() == intcurve_type )
	{
		bs3_curve bs = ( (intcurve const *)&cur_ptr )->cur();
		if ( bs )
		{
			if ( ( (intcurve const *)&cur_ptr )->reversed() )
			{
				SPAparameter temp = -start_param;
				start_param = -end_param;
				end_param = temp;
			}
			int dim, deg, npts, nkts;
			logical rat;
			SPAposition *pts = 0;
			double *wts = 0;
			double *kts = 0;
			float *control_points = 0;
			float *weights = 0;
			float *knots = 0;
			API_SYS_BEGIN
			{
				bs3_curve_to_array( bs, dim, deg, rat, npts, pts, wts, nkts, kts );
				ConvertSPAPositionToFloatArray( npts, pts, control_points );
				ConvertDoubleToFloatArray( npts, wts, weights );
				ConvertDoubleToFloatArray( nkts, kts, knots );
				SPAinterval range = bs3_curve_range( bs );
				double start_range = range.start_pt();
				double end_range = range.end_pt();
				double length = end_range - start_range;
				float start = (float)( ( start_param - start_range ) / length );
				float end = (float)( ( end_param - start_range ) / length );
				// Round
				int s = (int)( start*ROUND_TO );
				int e = (int)( end*ROUND_TO );
				start = (float)( (float)s ) / ROUND_TO;
				end = (float)( (float)e ) / ROUND_TO;
				HC_KEY key = 0;
				// Check for periocity
				int offset = (int)start;
				if ( offset )
				{
					start -= offset;
					end -= offset;
				}
				float diff = end - start;
				if ( diff > ( float )1.0 + (float)SPAresabs )
					sys_error( HA_BRIDGE_PARAM_OUT_OF_RANGE );
				if ( start > end )
					sys_error( HA_BRIDGE_PARAM_OUT_OF_RANGE );
				fp_sentry fps;
				// Within range.
				if ( start >= ( float )0.0 && end <= ( float )1.0 ) // 0.0 <= start < hend <= 1.0
				{
					key = HC_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, (double)start, (double)end );
					AddMapping( key, ent );
				} else if ( start < ( float )0.0 && end >( float )0.0 ) // start < 0.0 < hend
				{
					key = HC_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, 1.0 + (double)start, 1.0 );
					AddMapping( key, ent );
					key = HC_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, 0.0, (double)end );
					AddMapping( key, ent );
				} else if ( end <= ( float )0.0 )   // hstart < hend <= 0.0 
				{
					key = HC_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, (double)start + 1.0, (double)end + 1.0 );
					AddMapping( key, ent );
				} else if ( ( float )1.0 <= end )  // hstart < 1.0 <= hend.
				{
					key = HC_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, (double)start, 1.0 );
					AddMapping( key, ent );
					key = HC_KInsert_NURBS_Curve( deg, npts, control_points, weights, knots, 0.0, (double)end - 1.0 );
					AddMapping( key, ent );
				} else // ???????
					sys_error( HA_BRIDGE_PARAM_OUT_OF_RANGE );
			} API_SYS_END;
			ACIS_DELETE[] pts; pts = 0;
			ACIS_DELETE[] STD_CAST wts; wts = 0;
			ACIS_DELETE[] STD_CAST kts; kts = 0;
			ACIS_DELETE[] STD_CAST control_points; control_points = 0;
			ACIS_DELETE[] STD_CAST weights; weights = 0;
			ACIS_DELETE[] STD_CAST knots; knots = 0;
		}
	}
}

void hoops_acis_entity_converter::AddMapping(
	HC_KEY	key,
	ENTITY*	entity,
	ENTITY*	owner )
{
	if ( m_Map && m_EnableMapping )
		m_Map->AddMapping( key, entity, owner );
}

void hoops_acis_entity_converter::ReRenderVisibilityAsm( const ha_rendering_options & ro )
{
	const char * pattern = ro.GetPattern();
	if ( pattern )
	{
		HC_Open_Segment( pattern );
		{	// Faces
			if ( ro.GetRenderFacesMode() )
			{
				HC_UnSet_One_Condition( "AcisFacesOffSilsOn" );
				HC_UnSet_One_Condition( "AcisFacesOffSilsOff" );
				HC_Set_Conditions( "AcisFacesOn" );
			} else if ( FALSE /*face_silhouettes*/ )
			{
				HC_UnSet_One_Condition( "AcisFacesOn" );
				HC_UnSet_One_Condition( "AcisFacesOffSilsOff" );
				HC_Set_Conditions( "AcisFacesOffSilsOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisFacesOn" );
				HC_UnSet_One_Condition( "AcisFacesOffSilsOn" );
				HC_Set_Conditions( "AcisFacesOffSilsOff" );
			}
			// Edges
			if ( ro.GetRenderEdgesMode() )
			{
				HC_UnSet_One_Condition( "AcisEdgesOff" );
				HC_Set_Conditions( "AcisEdgesOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisEdgesOn" );
				HC_Set_Conditions( "AcisEdgesOff" );
			}
			// Vertices
			if ( ro.GetRenderVerticesMode() )
			{
				HC_UnSet_One_Condition( "AcisVerticesOff" );
				HC_Set_Conditions( "AcisVerticesOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisVerticesOn" );
				HC_Set_Conditions( "AcisVerticesOff" );
			}
			// Coedges
			if ( ro.GetRenderCoedgeMode() )
			{
				HC_UnSet_One_Condition( "AcisCoedgesOff" );
				HC_Set_Conditions( "AcisCoedgesOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisCoedgesOn" );
				HC_Set_Conditions( "AcisCoedgesOff" );
			}
			// Tcoedges
			if ( ro.GetRenderTCoedgeMode() )
			{
				HC_UnSet_One_Condition( "AcisTCoedgesOff" );
				HC_Set_Conditions( "AcisTCoedgesOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisTCoedgesOn" );
				HC_Set_Conditions( "AcisTCoedgesOff" );
			}
			// APOINTs
			if ( ro.GetRenderAPOINTsMode() )
			{
				HC_UnSet_One_Condition( "AcisAPOINTsOff" );
				HC_Set_Conditions( "AcisAPOINTsOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisAPOINTsOn" );
				HC_Set_Conditions( "AcisAPOINTsOff" );
			}
			// Text
			if ( ro.GetRenderTextMode() )
			{
				HC_UnSet_One_Condition( "AcisTextOff" );
				HC_Set_Conditions( "AcisTextOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisTextOn" );
				HC_Set_Conditions( "AcisTextOff" );
			}
			// WCSs
			if ( ro.GetRenderWCSsMode() )
			{
				HC_UnSet_One_Condition( "AcisWCSsOff" );
				HC_Set_Conditions( "AcisWCSsOn" );
			} else
			{
				HC_UnSet_One_Condition( "AcisWCSsOn" );
				HC_Set_Conditions( "AcisWCSsOff" );
			}

			// debug code to check the resulting conditions
			logical debug( FALSE );
			if ( debug )
			{
				HC_KEY contents_key = 0;
				char type[64];

				HC_Begin_Contents_Search( ".", "conditions" );
				while ( HC_Find_Contents( type, &contents_key ) )
				{
					char conditions[128];
					HC_Show_Net_Conditions( conditions );
					int dummy = 0;
				}
				HC_End_Contents_Search();
			}
		}
		HC_Close_Segment();
	}
} // end of ReRenderVisibilityAsm()
