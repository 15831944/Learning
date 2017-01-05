/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_Part_H___
#define __HPS_Part_H___

#include "dcl_hps_part.h"
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hashpart.hxx"

class ENTITY;
class PART;

void DECL_HPS_PART initialize_hps_part();
void DECL_HPS_PART terminate_hps_part();

class DECL_HPS_PART HPS_Part : public ACIS_OBJECT
{
public:
	HPS_Part();
	~HPS_Part();
	logical		LoadFile( const char* in_file_path_name, logical in_text_flag = TRUE );
	logical		AddEntities( const ENTITY_LIST& in_entity_list );
	logical		AddEntity( ENTITY*& in_entity );
	logical		RemoveEntity( ENTITY*& in_entity );
	logical		UpdateEntity( ENTITY*& in_entity );
	// Deprecated -- inconsitent naming
	logical		Update( ENTITY*& in_entity ) { return UpdateEntity( in_entity ); }
	PART *		GetPart() { return m_pPart; }
	logical		RollBack();
	logical		RollForward();
protected:
	PART* m_pPart;
	void CreateCallback();
};

#endif	//__HPS_Part_H___

