/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_REND_CONTEXT_HXX__
#define __HPS_REND_CONTEXT_HXX__
#include "dcl_hps.h"
#include "logical.h"
#include "entity.hxx"
#include "hps_bridge.h"
/**
 * @file hps_rend_context.h
 * @CAA2Level L1
 * @CAA2Usage U1
 *! \addtogroup HPSBRIDGEAPI
 *  \brief Declared at <hps_rend_context.h>
 *  @{
 */
class HPS_Map;
class HPS_ModelMap;
class HPS_CHandleMap;
class asm_model;
/**
 * Rendering context
 */
class DECL_HPS hps_rendering_context : public ACIS_OBJECT
{
	char* m_Pattern;
	char* m_GeomPattern;
	HPS_Map* m_EntityMap;
	HPS_ModelMap* m_ModelMap;
	HPS_CHandleMap* m_CHandleMap;
	asm_model* m_Model;
	logical m_ForceModelRebuild;

public:
	/**
	 * .
	 */
	hps_rendering_context();
	/**
	 * .
	 */
	hps_rendering_context( const hps_rendering_context& in_rendering_contex );
	/**
	 * .
	 */
	~hps_rendering_context();
	/**
	 * .
	 */
	void SetPattern( const char* in_pattern );
	/**
	 * .
	 */
	void SetGeomPattern( const char* p );
	/**
	 * .
	 */
	void SetEntityMap( HPS_Map* in_map );
	/**
	 * .
	 */
	void SetModelMap( HPS_ModelMap* in_map );
	/**
	 * .
	 */
	void SetCHandleMap( HPS_CHandleMap* in_map );
	/**
	 * .
	 */
	void SetModel( asm_model* in_model );
	/**
	 * .
	 */
	void SetForceModelRebuild( logical in_force );
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
	HPS_Map* GetEntityMap() const;
	/**
	 * .
	 */
	HPS_ModelMap* GetModelMap() const;
	/**
	 * .
	 */
	HPS_CHandleMap* GetCHandleMap() const;
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
#endif //__HPS_REND_CONTEXT_HXX__
