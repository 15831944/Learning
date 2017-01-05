/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "ha_rend_context.h"

class HA_Map;
class HA_ModelMap;
class HA_CHandleMap;

ha_rendering_context::ha_rendering_context()
{
    m_Pattern = NULL;
    m_GeomPattern = NULL;
    m_EntityMap = NULL;
    m_ModelMap = NULL;
	m_CHandleMap = NULL;
	m_Model = NULL;
	m_ForceModelRebuild = FALSE;
}

ha_rendering_context::ha_rendering_context(const ha_rendering_context& ctx)
{
    m_Pattern = NULL;
    m_GeomPattern = NULL;
	SetPattern(ctx.GetPattern());
	SetGeomPattern(ctx.GetGeomPattern());
    m_EntityMap = ctx.GetEntityMap();
    m_ModelMap = ctx.GetModelMap();
	m_CHandleMap = ctx.GetCHandleMap();
	m_Model = ctx.GetModel();
	m_ForceModelRebuild = ctx.GetForceModelRebuild();
}

ha_rendering_context::~ha_rendering_context()
{
	if (m_Pattern)
		ACIS_DELETE [] STD_CAST m_Pattern; m_Pattern = NULL;
	if (m_GeomPattern)
		ACIS_DELETE [] STD_CAST m_GeomPattern; m_GeomPattern = NULL;
}

void ha_rendering_context::SetPattern(const char* p)
{
	char pat[1025];

	if (m_Pattern)
		ACIS_DELETE [] STD_CAST m_Pattern;
	m_Pattern = NULL;

	if (p)
	{
		if (HC_Parse_String(p, "=", 1, pat))
		{
			m_Pattern = ACIS_NEW char[strlen(pat)+1];
			strcpy(m_Pattern, pat);
		}
		else
		{
			m_Pattern = ACIS_NEW char[strlen(p)+1];
			strcpy(m_Pattern, p);
		}
	}
}

void ha_rendering_context::SetGeomPattern(const char* p)
{
	char pat[1025];

	if (m_GeomPattern)
		ACIS_DELETE [] STD_CAST m_GeomPattern;
	m_GeomPattern = NULL;

	if(p)
	{
		if (HC_Parse_String(p, "=", 1, pat))
		{
			m_GeomPattern = ACIS_NEW char[strlen(pat)+1];
			strcpy(m_GeomPattern, pat);
		}
		else
		{
			m_GeomPattern = ACIS_NEW char[strlen(p)+1];
			strcpy(m_GeomPattern, p);
		}
	}
}

void ha_rendering_context::SetEntityMap(HA_Map* map)
{
    m_EntityMap = map;
}

void ha_rendering_context::SetModelMap(HA_ModelMap* map)
{
    m_ModelMap = map;
}

void ha_rendering_context::SetCHandleMap(HA_CHandleMap* map)
{
    m_CHandleMap = map;
}

void ha_rendering_context::SetModel(asm_model* model)
{
    m_Model = model;
}

void ha_rendering_context::SetForceModelRebuild(logical force)
{
    m_ForceModelRebuild = force;
}

const char* ha_rendering_context::GetPattern() const 
{
	return m_Pattern;
}

const char* ha_rendering_context::GetGeomPattern() const 
{
	return m_GeomPattern;
}

HA_Map* ha_rendering_context::GetEntityMap() const
{
    return m_EntityMap;
}

HA_ModelMap* ha_rendering_context::GetModelMap() const
{
    return m_ModelMap;
}

HA_CHandleMap* ha_rendering_context::GetCHandleMap() const
{
    return m_CHandleMap;
}

asm_model* ha_rendering_context::GetModel() const
{
    return m_Model;
}

logical ha_rendering_context::GetForceModelRebuild() const
{
    return m_ForceModelRebuild;
}

