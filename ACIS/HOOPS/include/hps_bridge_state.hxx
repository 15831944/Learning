/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: hps_bridge_internal.h,v 1.2 2002/04/02 21:18:41 jmb Exp $
#ifndef HPS_BRIDGE_PRIVATE_HXX
#define HPS_BRIDGE_PRIVATE_HXX

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "acis.hxx"
#include "hps_bridge.h"
#include "hps_rend_options.h"
#include "facet_options.hxx"
#include "dcl_hps.h"
#include "hps_map.h"

class HPS_Map;
class HPS_ModelMap;
class HPS_CHandleMap;
class IEntityConverter;

class hps_state : public ACIS_OBJECT {
public:
	HPS_Map* s_pHPS_Map;
	HPS_Map* s_pHPS_MapAsm;
	HPS_ModelMap* s_pHPS_ModelMap;
	HPS_CHandleMap* s_pHPS_CHandleMap;
	IEntityConverter* s_pIEntityConverter;
	HPS::View s_Current_View_Key;
	hps_rendering_options s_RenderingOptions;
	facet_options* s_pFacetOptions;
	hps_state() : s_pHPS_Map( NULL ), s_pHPS_MapAsm( NULL ), s_pHPS_ModelMap( NULL ), s_pHPS_CHandleMap( NULL ), s_pIEntityConverter( NULL ), s_pFacetOptions( NULL ) {}
};

hps_state* get_hps_state();

#endif

