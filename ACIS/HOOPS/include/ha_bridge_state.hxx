/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: ha_bridge_internal.h,v 1.2 2002/04/02 21:18:41 jmb Exp $
#ifndef HA_BRIDGE_PRIVATE_HXX
#define HA_BRIDGE_PRIVATE_HXX

#include "acis.hxx"
#include "ha_bridge.h"
#include "ha_rend_options.h"
#include "facet_options.hxx"
#include "dcl_hoops.h"

class HA_Map;
class HA_ModelMap;
class HA_CHandleMap;
class IEntityConverter;

class ha_state : public ACIS_OBJECT {
public:
	HA_Map* s_pHA_Map;
	HA_Map* s_pHA_MapAsm;
	HA_ModelMap* s_pHA_ModelMap;
	HA_CHandleMap* s_pHA_CHandleMap;
	IEntityConverter* s_pIEntityConverter;
	HC_KEY s_Current_View_Key;
	ha_rendering_options s_RenderingOptions;
	facet_options* s_pFacetOptions;
	struct {
		unsigned license_okay:1;
	} bit_flag;

	ha_state() : s_pHA_Map(NULL), s_pHA_MapAsm(NULL), s_pHA_ModelMap(NULL), s_pHA_CHandleMap(NULL),
		s_pIEntityConverter(NULL), 	s_Current_View_Key(0), s_pFacetOptions(NULL)
	{
		bit_flag.license_okay = 0;
	}	
};

DECL_HOOPS ha_state* get_ha_state();

#endif

