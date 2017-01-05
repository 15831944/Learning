// HHOOPS08View.cpp : implementation of the HHOOPS08View class
// 

// Standard includes
#include "StdAfx.h"
#include "Resource.h"
#include <assert.h>
#include <math.h>
#include "hc.h"


#include "HHOOPSView.h"
#include "HHOOPSModel.h"
#include "HHOOPSSelectionSet.h"

#include "ha_bridge.h"

#include "HSharedKey.h"


HHOOPSView::HHOOPSView(HBaseModel *model,
    const char * 		alias,	
    const char *		driver_type,
    const char *		instance_name,
    void *				window_handle,
    void *				colormap) : HBaseView(model, alias, driver_type, instance_name, window_handle, colormap)

{
}

HHOOPSView::~HHOOPSView()
{
}


// app-specific init function
void HHOOPSView::Init()
{
	// call base's init function first to get the default HOOPS hierarchy for the view
	HBaseView::Init();

	// create our app-specific Selection object and initialize
	m_pSelection = new HHOOPSSelectionSet(this);
    m_pSelection->Init();

	// TODO: Add your initialization here
}

void HHOOPSView::DeleteSelectionList( bool emit_message )
{
	int				i, numSolidSelections, numHoopsSelections;
/*	char			type[4096];*/
	HHOOPSSelectionSet* selection = (HHOOPSSelectionSet *)GetSelection();
	ENTITY*			current = 0;
	numHoopsSelections = selection->GetSize();
	numSolidSelections = selection->GetSolidListSize();
	if (numHoopsSelections == 0)
		return;

	// loop through the list of selected HOOPS primitives, filtering
	// out the ones associated with solid model entities, and deleting
	// the non-solid model primitives directly using Delete_By_Key
	// The primitives associated with solid model entities need to be
	// deleted using the HOOPS/GM deletion functions so that the HOOPS<->GM 
	// mapping tables will remain in sync
	for (i = numHoopsSelections; i > 0; i--)
	{
		HC_KEY key = selection->GetAt(i-1);

		// if it's a HOOPS primitive that is not associated with a solid model entity,
		// deselect and delete it
		current = HA_Compute_Entity_Pointer(key, BODY_TYPE);
		if (current == 0)
		{
			selection->DeSelect(key, false);
			HC_Delete_By_Key(key);
		}
	}

	// loop through the list of selected solid model entities,
	// delete the HOOPS geometry associated with each
	// and then delete the entity
	for (i = numSolidSelections; i > 0; i--)
	{
		current = (ENTITY*)selection->GetAtSolidEntity(i-1);
		if (current)
		{
			selection->DeSelectEntity(current);
			((HHOOPSModel*)GetModel())->DeleteAcisEntity(current);
		}
	}

	if( emit_message )
		EmitDeleteSelectionListMessage();

	// Reset the selection set lists since we've deleted all the entities that
	// were in them  (we can't DeSelect the entities because they no longer exist)
	selection->Reset();

    SetGeometryChanged();
	Update();
}

void HHOOPSView::SetVisibilityFaces(bool on_off)
{
	HC_Open_Segment_By_Key (GetSceneKey());
	if (on_off) 
		HC_Set_Visibility ("Faces = on");
	else 
		HC_Set_Visibility ("Faces = off");
	HC_Close_Segment ();

	SetGeometryChanged();
}

bool HHOOPSView::GetVisibilityFaces()
{
	char cval[MVO_BUFFER_SIZE];
	HC_Open_Segment_By_Key (GetSceneKey());
	HC_Show_One_Net_Visibility("Faces", cval);
	HC_Close_Segment ();
	if (strstr(cval,"on"))
		return true;
	else
		return false;
}