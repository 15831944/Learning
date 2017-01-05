/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#include "dcl_hoops.h"

#include "ha_rend_options.h"
#include <ctype.h> // needed for tolower()

#include "b_strutl.hxx"
#include "option.hxx"

ha_rendering_options::ha_rendering_options()
{
	m_lMappingEnabled=TRUE;
	m_lColorSegmentMode=TRUE;
	m_lBodySegmentMode=TRUE;

	m_lRenderFacesMode=TRUE;
	m_lRenderEdgesMode=TRUE;
	m_lRenderVerticesMode=FALSE;
	m_lRenderTCoedges=FALSE;
	m_lRenderCoedges=FALSE;
	m_lRenderAPOINTs=TRUE;
	m_lRenderWCSs=TRUE;
	m_lRenderText=TRUE;

	m_view_is_iconified = FALSE;

	m_lTessellateEllipsesMode=FALSE;
	m_curveFacetTol=DEF_CURVE_FACET_TOL;
	m_saveCurveFacetTol=DEF_CURVE_FACET_TOL;
	m_CurveFacetLevel=cfl_face_facets;

	m_lMergeBodiesMode=FALSE;
	m_lMergeFacesMode=FALSE;

	m_Pattern=0;
	m_GeomPattern=0;
	m_FacetStyle = 0;
}


void ha_rendering_options::SetIconifiedState(logical state)
{
	// TRUE = iconified (aka minimized)
	// FALSE = restored
	m_view_is_iconified = state;
}

GLOBAL_VAR option_header render_curve_factor("render_curve_factor", 1);

double ha_rendering_options::GetCurveFacetTol() const
{ 
	double tol = m_curveFacetTol;

	// Don't try to compute the tolerance if the view
	// is iconified (minimized) - the answer will be screwed
	// up (HOOPS issue?). To get around this, the tolerance
	// is computed exactly when the window is minimized and
	// stored in m_curveFacetTol. Therefore, we can just use
	// that instead.
	if (!m_view_is_iconified)
	{
		if (m_CurveFacetLevel==cfl_screen || m_CurveFacetLevel==cfl_hoops_primitives)
			tol = HA_World_Coords_Per_Pixel();
	}

	tol /= render_curve_factor.count();
	if (tol < SPAresabs)
		tol=SPAresabs;

	return tol;
}

void ha_rendering_options::Set_Rendering_Options(const char * list)
{
	char token[1025];
	char llist[1025];
	unsigned long options = 0;
	unsigned long bitmask = 0;
	unsigned long i;
	unsigned long token_number = 0;

	if (m_lColorSegmentMode)
		options |= HA_CREATE_COLOR_SEGMENTS;

	if (m_lBodySegmentMode)
		options |= HA_CREATE_BODY_SEGMENTS;

	if (m_lTessellateEllipsesMode)
		options |= HA_TESSELLATE_ELLIPSES;

	if (m_lRenderEdgesMode)
		options |= HA_RENDER_EDGES;

	if (m_lRenderFacesMode)
		options |= HA_RENDER_FACES;

	if (m_lMergeFacesMode)
		options |= HA_MERGE_FACES;

	if (m_lMergeBodiesMode)
		options |= HA_MERGE_BODIES;
	
	/*my canonize chars*/
	for(i=0; i<1024; i++){

		if(!list[i]) break;	
		llist[i] = (char) tolower((int)list[i]);
	}
	llist[i] = 0;


	/*loop through options*/
	while(HC_Parse_String(llist,",",token_number++,token))
	{
		logical parsed = FALSE;
		if (strstr(token,"preserve color"))
			bitmask = HA_CREATE_COLOR_SEGMENTS;
		else if (strstr(token,"create body segments"))
			bitmask = HA_CREATE_BODY_SEGMENTS;
		else if (strstr(token,"merge faces"))
			bitmask = HA_MERGE_FACES;
		else if (strstr(token,"merge bodies"))
			bitmask = HA_MERGE_BODIES;
		else if (strstr(token,"tessellate ellipses"))
			bitmask = HA_TESSELLATE_ELLIPSES;
		else if (strstr(token,"edges"))
			bitmask = HA_RENDER_EDGES;
		else if (strstr(token,"faces"))
			bitmask = HA_RENDER_FACES;
		else if (strstr(token,"segment pattern"))
		{
			SetPattern(token);
			parsed = TRUE;
		}
		else if (strstr(token,"geom pattern"))
		{
			SetGeomPattern(token);
			parsed = TRUE;
		}
		else if (strstr(token,"facet style"))
		{
			char buffer[1024];
			HC_Parse_String( token, "=", 1, buffer );
			SetFacetStyle( (char)atoi( buffer ) );
			parsed = TRUE;
		}
		else
		{
			/*error*/
			const char * mes = "Null option or unknown option";
			
			const char * fun = "HA_Set_Rendering_Options";

			HC_Report_Error( 50, 309, 1, 1, &mes, 1, &fun);
			return;
		}

		if (!parsed && !Parse_YesNo_And_Mutate_Options_Using_Bitmask(token, bitmask, &options))
			return; // parse error, don't set the options.
	}

	SetColorSegmentMode			(options & HA_CREATE_COLOR_SEGMENTS);
	SetBodySegmentMode			(options & HA_CREATE_BODY_SEGMENTS);
	SetTessellateEllipsesMode	(options & HA_TESSELLATE_ELLIPSES);
	SetRenderEdgesMode			(options & HA_RENDER_EDGES);
	SetRenderFacesMode			(options & HA_RENDER_FACES);
	SetMergeFacesMode			(options & HA_MERGE_FACES);
	SetMergeBodiesMode			(options & HA_MERGE_BODIES);
}

// This will just tell us what is to be rendered.
ha_rendering_options &ha_rendering_options::operator |=(const ha_rendering_options &ro)
{
	m_lRenderFacesMode|=ro.m_lRenderFacesMode;
	m_lRenderEdgesMode|=ro.m_lRenderEdgesMode;
	m_lRenderVerticesMode|=ro.m_lRenderVerticesMode;
	m_lRenderTCoedges|=ro.m_lRenderTCoedges;
	m_lRenderCoedges|=ro.m_lRenderCoedges;

	return *this;
}

void ha_rendering_options::SetPattern(const char *p)
{
	char pattern[1025];

	if (m_Pattern)
		ACIS_DELETE [] STD_CAST m_Pattern;
	m_Pattern=0;

	if(p)
	{
		if (HC_Parse_String(p, "=", 1, pattern))
		{
			m_Pattern = ACIS_NEW char[strlen(pattern)+1];
			strcpy(m_Pattern, pattern);
		}
		else
		{
			m_Pattern = ACIS_NEW char[strlen(p)+1];
			strcpy(m_Pattern, p);
		}
	}
}

void ha_rendering_options::SetGeomPattern(const char *p)
{
	char pattern[1025];

	if (m_GeomPattern)
		ACIS_DELETE [] STD_CAST m_GeomPattern;
	m_GeomPattern=0;

	if(p)
	{
		if (HC_Parse_String(p, "=", 1, pattern))
		{
			m_GeomPattern = ACIS_NEW char[strlen(pattern)+1];
			strcpy(m_GeomPattern, pattern);
		}
		else
		{
			m_GeomPattern = ACIS_NEW char[strlen(p)+1];
			strcpy(m_GeomPattern, p);
		}
	}
}

ha_rendering_options &ha_rendering_options::operator =(const ha_rendering_options &ro)
{
	m_lMappingEnabled=ro.m_lMappingEnabled;
	m_lColorSegmentMode=ro.m_lColorSegmentMode;
	m_lBodySegmentMode=ro.m_lBodySegmentMode;

	m_lRenderFacesMode=ro.m_lRenderFacesMode;
	m_lRenderEdgesMode=ro.m_lRenderEdgesMode;
	m_lRenderVerticesMode=ro.m_lRenderVerticesMode;
	m_lRenderTCoedges=ro.m_lRenderTCoedges;
	m_lRenderCoedges=ro.m_lRenderCoedges;
	m_lRenderAPOINTs=ro.m_lRenderAPOINTs;
	m_lRenderWCSs=ro.m_lRenderWCSs;
	m_lRenderText=ro.m_lRenderText;

	m_lTessellateEllipsesMode=ro.m_lTessellateEllipsesMode;
	m_curveFacetTol=ro.m_curveFacetTol;
	m_CurveFacetLevel=ro.m_CurveFacetLevel;

	m_lMergeBodiesMode=ro.m_lMergeBodiesMode;
	m_lMergeFacesMode=ro.m_lMergeFacesMode;

	m_view_is_iconified = ro.m_view_is_iconified;
	m_saveCurveFacetTol = ro.m_saveCurveFacetTol;

	SetPattern(ro.GetPattern());
	SetGeomPattern(ro.GetGeomPattern());
	SetFacetStyle(ro.GetFacetStyle());

	return *this;
}

void ha_rendering_options::Show_Rendering_Options(char * list)
{
	if(!list)
		return;

	strcpy(list,"");

	GetColorSegmentMode()		? strcat(list,"preserve color = on, ")		: strcat(list,"preserve color = off, ");
	GetBodySegmentMode()		? strcat(list,"create body segments = on, "): strcat(list,"create body segments = off, ");
	GetMergeFacesMode()			? strcat(list,"merge faces = on, ")			: strcat(list,"merge faces = off, ");
	GetMergeBodiesMode()		? strcat(list,"merge bodies = on, ")		: strcat(list,"merge bodies = off, ");
	GetRenderEdgesMode()		? strcat(list,"edges = on, ")				: strcat(list,"edges = off, ");
	GetRenderFacesMode()		? strcat(list,"faces = on, ")				: strcat(list,"faces = off, ");
	GetTessellateEllipsesMode()	? strcat(list,"tessellate ellipses = on ")	: strcat(list,"tessellate ellipses = off ");
}

void ha_rendering_options::Show_One_Rendering_Option(const char * type, char * value)
{
	char ltype[1025];
	int i;

	if(type && value)
	{
		// my canonize chars
		for(i=0; i<1024; i++)
		{
			if(!type[i]) break;	
			ltype[i] = (char) tolower((int)type[i]);
		}
		ltype[i] = 0;
	
		strcpy(value,"");

		if (strstr(ltype,"preserve color"))
		{
			GetColorSegmentMode() ? strcat(value,"on") : strcat(value,"off");
		}
		else if (strstr(ltype,"create body segments"))
		{
			GetBodySegmentMode() ? strcat(value,"on") : strcat(value,"off");
		} 
		else if (strstr(ltype,"merge faces"))
		{
			GetMergeFacesMode() ? strcat(value,"on") : strcat(value,"off");
		} 
		else if (strstr(ltype,"merge bodies"))
		{
			GetMergeBodiesMode() ? strcat(value,"on") : strcat(value,"off");
		} 
		else if (strstr(ltype,"edges"))
		{
			GetRenderEdgesMode() ? strcat(value,"on") : strcat(value,"off");
		} 
		else if (strstr(ltype,"faces"))
		{
			GetRenderFacesMode() ? strcat(value,"on") : strcat(value,"off");
		} 
		else if (strstr(ltype,"tessellate ellipses"))
		{
			GetTessellateEllipsesMode() ? strcat(value,"on") : strcat(value,"off");
		} 
		else
		{		
			// error
			char * mes = "Unknown type in:";
			char * mesv[2]; 
		
			char * fun = "HA_Show_One_Rendering_Option";
			char * funv[1];

			mesv[0]=mes;
			mesv[1]=(char*)type;
			funv[0]=fun;

			HC_Report_Error(50,309,1,2,mesv,1,funv);
			return;
		}	
	}
	else
	{		
		// error
		char * mes = "Null type or value";
		char * mesv[1]; 
		
		char * fun = "HA_Show_One_Rendering_Option";
		char * funv[1];

		mesv[0]=mes;
		funv[0]=fun;

		HC_Report_Error(50,309,1,1,mesv,1,funv);
	}		
}

