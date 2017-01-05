/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_CALLBACK_HXX
#define __HPS_CALLBACK_HXX

#include "ent_cb.hxx"
#include "hist_cb.hxx"
#include "lists.hxx"
#include "dcl_hps_part.h"

class PART;

//======================================================================
// HPS_EntityCallback is used to hook up to acis entity callback notifier

class DECL_HPS_PART HPS_EntityCallback : public entity_callback
{
	//	HPS_EntityCallbackHandler* m_pCallbackHandler;
	PART* m_pPart;

public:
	HPS_EntityCallback() :m_pPart( 0 ) {}
	HPS_EntityCallback( PART *part ) :m_pPart( part ) {}
	virtual ~HPS_EntityCallback() {}

	virtual void execute( entity_event_type event, ENTITY *ent );
};

class DECL_HPS_PART HPS_History_Callbacks : public history_callbacks
{
	ENTITY_LIST create_ents_list;
	ENTITY_LIST delete_ents_list;
	ENTITY_LIST change_ents_list;

public:

	void Before_Roll_Bulletin_Board( BULLETIN_BOARD*, logical discard );
	void After_Roll_Bulletin_Board( BULLETIN_BOARD*, logical discard );
};

#endif	//	__HPS_CALLBACK_HXX
