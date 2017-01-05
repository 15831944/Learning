/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#include "dcl_hps.h"
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_util.h"
#include "hps_bridge_state.hxx"

#include "hps_rend_options.h"
#include "hps_bridge.h"
#include <ctype.h> // needed for tolower()

#include "b_strutl.hxx"
#include "option.hxx"

hps_rendering_options::hps_rendering_options()
{
	m_lMappingEnabled = TRUE;
	m_lColorSegmentMode = TRUE;
	m_lBodySegmentMode = TRUE;
	m_lRenderFacesMode = TRUE;
	m_lRenderEdgesMode = TRUE;
	m_lRenderVerticesMode = FALSE;
	m_lRenderTCoedges = FALSE;
	m_lRenderCoedges = FALSE;
	m_lRenderAPOINTs = TRUE;
	m_lRenderWCSs = TRUE;
	m_lRenderText = TRUE;
	m_view_is_iconified = FALSE;
	m_lTessellateEllipsesMode = FALSE;
	m_curveFacetTol = DEF_CURVE_FACET_TOL;
	m_saveCurveFacetTol = DEF_CURVE_FACET_TOL;
	m_CurveFacetLevel = cfl_face_facets;
	m_lMergeBodiesMode = FALSE;
	m_lMergeFacesMode = FALSE;
	m_Pattern = 0;
	m_GeomPattern = 0;
	m_FacetStyle = 0;
}

void hps_rendering_options::SetIconifiedState( logical in_iconify )
{
	// TRUE = iconified (aka minimized)
	// FALSE = restored
	m_view_is_iconified = in_iconify;
}

GLOBAL_VAR option_header render_curve_factor( "render_curve_factor", 1 );

double hps_rendering_options::GetCurveFacetTol() const
{
	double tol = m_curveFacetTol;
	// Don't try to compute the tolerance if the view
	// is iconified (minimized) - the answer will be screwed
	// up (HOOPS issue?). To get around this, the tolerance
	// is computed exactly when the window is minimized and
	// stored in m_curveFacetTol. Therefore, we can just use
	// that instead.
	if ( !m_view_is_iconified )
	{
		if ( m_CurveFacetLevel == cfl_screen || m_CurveFacetLevel == cfl_hps_primitives )
			tol = HPS_World_Coords_Per_Pixel();
	}
	tol /= render_curve_factor.count();
	if ( tol < SPAresabs )
		tol = SPAresabs;
	return tol;
}

void hps_rendering_options::Set_Rendering_Options( const char * in_list )
{
	char token[1025];
	unsigned long options = 0;
	unsigned long bitmask = 0;
	unsigned long token_number = 0;
	if ( m_lColorSegmentMode )
		options |= HPS_CREATE_COLOR_SEGMENTS;
	if ( m_lBodySegmentMode )
		options |= HPS_CREATE_BODY_SEGMENTS;
	if ( m_lTessellateEllipsesMode )
		options |= HPS_TESSELLATE_ELLIPSES;
	if ( m_lRenderEdgesMode )
		options |= HA_RENDER_EDGES;
	if ( m_lRenderFacesMode )
		options |= HA_RENDER_FACES;
	if ( m_lMergeFacesMode )
		options |= HPS_MERGE_FACES;
	if ( m_lMergeBodiesMode )
		options |= HPS_MERGE_BODIES;
	/*my canonize chars*/
	/*loop through options*/
	while ( HPS_Parse_String( in_list, ",", token_number++, token ) )
	{
		logical parsed = FALSE;
		if ( HPS_stristr( token, "preserve color" ) )
			bitmask = HPS_CREATE_COLOR_SEGMENTS;
		else if ( HPS_stristr( token, "create body segments" ) )
			bitmask = HPS_CREATE_BODY_SEGMENTS;
		else if ( HPS_stristr( token, "merge faces" ) )
			bitmask = HPS_MERGE_FACES;
		else if ( HPS_stristr( token, "merge bodies" ) )
			bitmask = HPS_MERGE_BODIES;
		else if ( HPS_stristr( token, "tessellate ellipses" ) )
			bitmask = HPS_TESSELLATE_ELLIPSES;
		else if ( HPS_stristr( token, "edges" ) )
			bitmask = HA_RENDER_EDGES;
		else if ( HPS_stristr( token, "faces" ) )
			bitmask = HA_RENDER_FACES;
		else if ( HPS_stristr( token, "segment pattern" ) )
		{
			SetPattern( token );
			parsed = TRUE;
		} else if ( HPS_stristr( token, "geom pattern" ) )
		{
			SetGeomPattern( token );
			parsed = TRUE;
		} else if ( HPS_stristr( token, "facet style" ) )
		{
			char buffer[1024];
			HPS_Parse_String( token, "=", 1, buffer );
			SetFacetStyle( (char)atoi( buffer ) );
			parsed = TRUE;
		} else
		{
			HPS::Database::GetEventDispatcher().InjectEvent( HPS::ErrorEvent( "HPS_Set_Rendering_Options: Null option or unknown option" ) );
			return;
		}
		if ( !parsed && !Parse_YesNo_And_Mutate_Options_Using_Bitmask( token, bitmask, &options ) )
			return; // parse error, don't set the options.
	}
	SetColorSegmentMode( options & HPS_CREATE_COLOR_SEGMENTS );
	SetBodySegmentMode( options & HPS_CREATE_BODY_SEGMENTS );
	SetTessellateEllipsesMode( options & HPS_TESSELLATE_ELLIPSES );
	SetRenderEdgesMode( options & HA_RENDER_EDGES );
	SetRenderFacesMode( options & HA_RENDER_FACES );
	SetMergeFacesMode( options & HPS_MERGE_FACES );
	SetMergeBodiesMode( options & HPS_MERGE_BODIES );
}

// This will just tell us what is to be rendered.
hps_rendering_options &hps_rendering_options::operator |=( const hps_rendering_options &in_rendering_options )
{
	m_lRenderFacesMode |= in_rendering_options.m_lRenderFacesMode;
	m_lRenderEdgesMode |= in_rendering_options.m_lRenderEdgesMode;
	m_lRenderVerticesMode |= in_rendering_options.m_lRenderVerticesMode;
	m_lRenderTCoedges |= in_rendering_options.m_lRenderTCoedges;
	m_lRenderCoedges |= in_rendering_options.m_lRenderCoedges;
	return *this;
}

void hps_rendering_options::SetPattern( const char *in_pattern )
{
	char pattern[1025];
	if ( m_Pattern )
		ACIS_DELETE[] STD_CAST m_Pattern;
	m_Pattern = 0;
	if ( in_pattern )
	{
		if ( HPS_Parse_String( in_pattern, "=", 1, pattern ) )
		{
			m_Pattern = ACIS_NEW char[strlen( pattern ) + 1];
			strcpy( m_Pattern, pattern );
		} else
		{
			m_Pattern = ACIS_NEW char[strlen( in_pattern ) + 1];
			strcpy( m_Pattern, in_pattern );
		}
	}
}

void hps_rendering_options::SetGeomPattern( const char *in_pattern )
{
	char pattern[1025];
	if ( m_GeomPattern )
		ACIS_DELETE[] STD_CAST m_GeomPattern;
	m_GeomPattern = 0;
	if ( in_pattern )
	{
		if ( HPS_Parse_String( in_pattern, "=", 1, pattern ) )
		{
			m_GeomPattern = ACIS_NEW char[strlen( pattern ) + 1];
			strcpy( m_GeomPattern, pattern );
		} else
		{
			m_GeomPattern = ACIS_NEW char[strlen( in_pattern ) + 1];
			strcpy( m_GeomPattern, in_pattern );
		}
	}
}

hps_rendering_options &hps_rendering_options::operator =( const hps_rendering_options &in_rendering_options )
{
	m_lMappingEnabled = in_rendering_options.m_lMappingEnabled;
	m_lColorSegmentMode = in_rendering_options.m_lColorSegmentMode;
	m_lBodySegmentMode = in_rendering_options.m_lBodySegmentMode;
	m_lRenderFacesMode = in_rendering_options.m_lRenderFacesMode;
	m_lRenderEdgesMode = in_rendering_options.m_lRenderEdgesMode;
	m_lRenderVerticesMode = in_rendering_options.m_lRenderVerticesMode;
	m_lRenderTCoedges = in_rendering_options.m_lRenderTCoedges;
	m_lRenderCoedges = in_rendering_options.m_lRenderCoedges;
	m_lRenderAPOINTs = in_rendering_options.m_lRenderAPOINTs;
	m_lRenderWCSs = in_rendering_options.m_lRenderWCSs;
	m_lRenderText = in_rendering_options.m_lRenderText;
	m_lTessellateEllipsesMode = in_rendering_options.m_lTessellateEllipsesMode;
	m_curveFacetTol = in_rendering_options.m_curveFacetTol;
	m_CurveFacetLevel = in_rendering_options.m_CurveFacetLevel;
	m_lMergeBodiesMode = in_rendering_options.m_lMergeBodiesMode;
	m_lMergeFacesMode = in_rendering_options.m_lMergeFacesMode;
	m_view_is_iconified = in_rendering_options.m_view_is_iconified;
	m_saveCurveFacetTol = in_rendering_options.m_saveCurveFacetTol;
	SetPattern( in_rendering_options.GetPattern() );
	SetGeomPattern( in_rendering_options.GetGeomPattern() );
	SetFacetStyle( in_rendering_options.GetFacetStyle() );
	return *this;
}

void hps_rendering_options::Show_Rendering_Options( char * out_list )
{
	if ( !out_list )
		return;
	strcpy( out_list, "" );
	GetColorSegmentMode() ? strcat( out_list, "preserve color = on, " ) : strcat( out_list, "preserve color = off, " );
	GetBodySegmentMode() ? strcat( out_list, "create body segments = on, " ) : strcat( out_list, "create body segments = off, " );
	GetMergeFacesMode() ? strcat( out_list, "merge faces = on, " ) : strcat( out_list, "merge faces = off, " );
	GetMergeBodiesMode() ? strcat( out_list, "merge bodies = on, " ) : strcat( out_list, "merge bodies = off, " );
	GetRenderEdgesMode() ? strcat( out_list, "edges = on, " ) : strcat( out_list, "edges = off, " );
	GetRenderFacesMode() ? strcat( out_list, "faces = on, " ) : strcat( out_list, "faces = off, " );
	GetTessellateEllipsesMode() ? strcat( out_list, "tessellate ellipses = on " ) : strcat( out_list, "tessellate ellipses = off " );
}

void hps_rendering_options::Show_One_Rendering_Option( const char * in_type, char * out_text )
{
	if ( in_type && out_text )
	{	// my canonize chars
		strcpy( out_text, "" );
		if ( HPS_stristr( in_type, "preserve color" ) )
			GetColorSegmentMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else if ( HPS_stristr( in_type, "create body segments" ) )
			GetBodySegmentMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else if ( HPS_stristr( in_type, "merge faces" ) )
			GetMergeFacesMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else if ( HPS_stristr( in_type, "merge bodies" ) )
			GetMergeBodiesMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else if ( HPS_stristr( in_type, "edges" ) )
			GetRenderEdgesMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else if ( HPS_stristr( in_type, "faces" ) )
			GetRenderFacesMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else if ( HPS_stristr( in_type, "tessellate ellipses" ) )
			GetTessellateEllipsesMode() ? strcat( out_text, "on" ) : strcat( out_text, "off" );
		else
		{
			HPS::Database::GetEventDispatcher().InjectEvent( HPS::ErrorEvent( "HPS_Show_One_Rendering_Option: Unknown type" ) );
			return;
		}
	} else
		HPS::Database::GetEventDispatcher().InjectEvent( HPS::ErrorEvent( "HPS_Show_One_Rendering_Option: Null type or value" ) );
}
