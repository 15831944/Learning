/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: ha_bridge_internal.h,v 1.2 2002/04/02 21:18:41 jmb Exp $
#ifndef HA_BRIDGE_INTERNAL_HXX
#define HA_BRIDGE_INTERNAL_HXX

#include "ha_bridge.h"

// apis for internal use only
DECL_HOOPS void HA_Internal_Delete_Entity_Geometry(ENTITY* entity);
DECL_HOOPS void check(outcome& o);

class FACET_BODY;
DECL_HOOPS void HA_Render_FACET_BODY(FACET_BODY* pb, logical renderFaces, logical renderEdges);


#endif /* HA_BRIDGE_INTERNAL_HXX */
