/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HA_Part_H_RAJESH_B__20010502__1618__
#define __HA_Part_H_RAJESH_B__20010502__1618__

#include "dcl_ha_part.h"

#include "hashpart.hxx"

#include <hc.h> // For HA_KEY

class ENTITY;
class PART;


void DECL_HA_PART initialize_ha_part();
void DECL_HA_PART terminate_ha_part();

class DECL_HA_PART HA_Part : public ACIS_OBJECT
{
	
public:
	HA_Part();
	~HA_Part();

	logical LoadFile( const char* filePathName, logical textFlag = TRUE );
	logical AddEntities( const ENTITY_LIST& entityList );
	logical AddEntity( ENTITY*& newEntity );
	logical RemoveEntity( ENTITY*& newEntity );
	logical UpdateEntity( ENTITY*& ent );
	// Deprecated -- inconsitent naming
	logical Update( ENTITY*& ent ) { return UpdateEntity(ent); }
	PART*   GetPart() { return m_pPart; }

	logical RollBack();
	logical RollForward();

protected:

	PART* m_pPart;

	void CreateCallback();
};


#endif	//__HA_Part_H_RAJESH_B__20010502__1618__

