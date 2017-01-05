// HHOOPS08View.h : interface of the HHOOPS08View class, derived from HBaseView
// Adds application-specific data and members for each view

#ifndef _HHOOPS08View_H
#define _HHOOPS08View_H

#include "HBaseView.h"
#include "HUtility.h"

class HSelectionSet;


class HHOOPSView : public HBaseView
{

public:

	HHOOPSView(	HBaseModel *model,
							const char * 	alias = 0,	
							const char *	driver_type = 0,
							const char *	instance_name = 0,
							void *			window_handle = 0,
							void *			colormap = 0);
	virtual ~HHOOPSView();

	void	Init();
	void	DeleteSelectionList(bool emit_message=false);

	void HHOOPSView::SetVisibilityFaces(bool on_off);
	
	bool HHOOPSView::GetVisibilityFaces();
	
};

#endif 




