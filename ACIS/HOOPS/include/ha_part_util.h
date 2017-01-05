/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#ifndef HA_PART_UTIL_H
#define HA_PART_UTIL_H

#include "dcl_ha_part.h"

class asm_model;
class ENTITY;
class ENTITY_LIST;

DECL_HA_PART asm_model*		HAPart_Get_Owning_Model( ENTITY * in_entity );
DECL_HA_PART void			HAPart_Render_Entity( ENTITY * in_entity );
DECL_HA_PART void			HAPart_Render_Entities( const ENTITY_LIST & in_entity_list );

#endif // HA_PART_UTIL_H
