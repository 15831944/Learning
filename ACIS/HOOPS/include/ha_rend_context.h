/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HA_REND_CONTEXT_HXX__
#define __HA_REND_CONTEXT_HXX__
#include "dcl_hoops.h"
#include "logical.h"
#include "entity.hxx"
#include "ha_bridge.h"
/**
 * @file ha_rend_context.h
 * @CAA2Level L1
 * @CAA2Usage U1
 *! \addtogroup HABRIDGEAPI
 *  \brief Declared at <ha_rend_context.h>
 *  @{
 */
class HA_Map;
class HA_ModelMap;
class HA_CHandleMap;
class asm_model;
/**
 * Rendering context
 */
class DECL_HOOPS ha_rendering_context : public ACIS_OBJECT
{
    char* m_Pattern;
	char* m_GeomPattern;
    HA_Map* m_EntityMap;
	HA_ModelMap* m_ModelMap;
	HA_CHandleMap* m_CHandleMap;
	asm_model* m_Model;
	logical m_ForceModelRebuild;

public:
    /**
     * .
     */
    ha_rendering_context();
    /**
     * .
     */
    ha_rendering_context(const ha_rendering_context& ctx);
    /**
     * .
     */
    ~ha_rendering_context();
    /**
     * .
     */
    void SetPattern(const char* p);
    /**
     * .
     */
	void SetGeomPattern(const char* p);
    /**
     * .
     */
    void SetEntityMap(HA_Map* map);
    /**
     * .
     */
    void SetModelMap(HA_ModelMap* map);
    /**
     * .
     */
    void SetCHandleMap(HA_CHandleMap* map);
    /**
     * .
     */
    void SetModel(asm_model* model);
    /**
     * .
     */
    void SetForceModelRebuild(logical force);
    /**
     * .
     */
	const char* GetPattern() const; 
    /**
     * .
     */
	const char* GetGeomPattern() const;
    /**
     * .
     */
    HA_Map* GetEntityMap() const;
    /**
     * .
     */
    HA_ModelMap* GetModelMap() const;
    /**
     * .
     */
    HA_CHandleMap* GetCHandleMap() const;
    /**
     * .
     */
    asm_model* GetModel() const;
    /**
     * .
     */
    logical GetForceModelRebuild() const;
};
/*! @} */
#endif //__HA_REND_CONTEXT_HXX__
