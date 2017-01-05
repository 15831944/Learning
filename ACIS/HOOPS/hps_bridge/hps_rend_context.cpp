/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_rend_context.h"

class HPS_Map;
class HPS_ModelMap;
class HPS_CHandleMap;

hps_rendering_context::hps_rendering_context()
{
	m_Pattern = NULL;
	m_GeomPattern = NULL;
	m_EntityMap = NULL;
	m_ModelMap = NULL;
	m_CHandleMap = NULL;
	m_Model = NULL;
	m_ForceModelRebuild = FALSE;
}

hps_rendering_context::hps_rendering_context( const hps_rendering_context& in_rendering_contex )
{
	m_Pattern = NULL;
	m_GeomPattern = NULL;
	SetPattern( in_rendering_contex.GetPattern() );
	SetGeomPattern( in_rendering_contex.GetGeomPattern() );
	m_EntityMap = in_rendering_contex.GetEntityMap();
	m_ModelMap = in_rendering_contex.GetModelMap();
	m_CHandleMap = in_rendering_contex.GetCHandleMap();
	m_Model = in_rendering_contex.GetModel();
	m_ForceModelRebuild = in_rendering_contex.GetForceModelRebuild();
}

hps_rendering_context::~hps_rendering_context()
{
	if ( m_Pattern )
		ACIS_DELETE[] STD_CAST m_Pattern; m_Pattern = NULL;
	if ( m_GeomPattern )
		ACIS_DELETE[] STD_CAST m_GeomPattern; m_GeomPattern = NULL;
}

void hps_rendering_context::SetPattern( const char* in_pattern )
{
	char pat[1025];

	if ( m_Pattern )
		ACIS_DELETE[] STD_CAST m_Pattern;
	m_Pattern = NULL;

	if ( in_pattern )
	{
		if ( HPS_Parse_String( in_pattern, "=", 1, pat ) )
		{
			m_Pattern = ACIS_NEW char[strlen( pat ) + 1];
			strcpy( m_Pattern, pat );
		} else
		{
			m_Pattern = ACIS_NEW char[strlen( in_pattern ) + 1];
			strcpy( m_Pattern, in_pattern );
		}
	}
}

void hps_rendering_context::SetGeomPattern( const char* in_pattern )
{
	char pat[1025];

	if ( m_GeomPattern )
		ACIS_DELETE[] STD_CAST m_GeomPattern;
	m_GeomPattern = NULL;

	if ( in_pattern )
	{
		if ( HPS_Parse_String( in_pattern, "=", 1, pat ) )
		{
			m_GeomPattern = ACIS_NEW char[strlen( pat ) + 1];
			strcpy( m_GeomPattern, pat );
		} else
		{
			m_GeomPattern = ACIS_NEW char[strlen( in_pattern ) + 1];
			strcpy( m_GeomPattern, in_pattern );
		}
	}
}

void hps_rendering_context::SetEntityMap( HPS_Map* in_map )
{
	m_EntityMap = in_map;
}

void hps_rendering_context::SetModelMap( HPS_ModelMap* in_map )
{
	m_ModelMap = in_map;
}

void hps_rendering_context::SetCHandleMap( HPS_CHandleMap* in_map )
{
	m_CHandleMap = in_map;
}

void hps_rendering_context::SetModel( asm_model* in_model )
{
	m_Model = in_model;
}

void hps_rendering_context::SetForceModelRebuild( logical in_force )
{
	m_ForceModelRebuild = in_force;
}

const char* hps_rendering_context::GetPattern() const
{
	return m_Pattern;
}

const char* hps_rendering_context::GetGeomPattern() const
{
	return m_GeomPattern;
}

HPS_Map* hps_rendering_context::GetEntityMap() const
{
	return m_EntityMap;
}

HPS_ModelMap* hps_rendering_context::GetModelMap() const
{
	return m_ModelMap;
}

HPS_CHandleMap* hps_rendering_context::GetCHandleMap() const
{
	return m_CHandleMap;
}

asm_model* hps_rendering_context::GetModel() const
{
	return m_Model;
}

logical hps_rendering_context::GetForceModelRebuild() const
{
	return m_ForceModelRebuild;
}
