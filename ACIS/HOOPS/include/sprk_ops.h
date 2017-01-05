// Copyright (c) Tech Soft 3D, Inc.
//
// The information contained herein is confidential and proprietary to Tech Soft 3D, Inc.,
// and considered a trade secret as defined under civil and criminal statutes.
// Tech Soft 3D, Inc. shall pursue its civil and criminal remedies in the event of
// unauthorized use or misappropriation of its trade secrets.  Use of this information
// by anyone other than authorized employees of Tech Soft 3D, Inc. is granted only under
// a written non-disclosure agreement, expressly prescribing the scope and manner of such use.

#ifndef SPRK_STD_OPERATORS_H
#define SPRK_STD_OPERATORS_H
#include "sprk.h"

#include <list>
#include <stack>
#include <unordered_map>

namespace std
{
	template<>
	struct hash<HPS::RGBColor>
	{
		size_t operator()(HPS::RGBColor const & x) const
		{
			return (
				(51 + std::hash<float>()(x.red)) * 51 + std::hash<float>()(x.green) * 51 + std::hash<float>()(x.blue));
		}
	};
}

#ifdef _MSC_VER
#ifndef STATIC_APP
#	ifdef SPRK_OPS
#		define SPRK_OPS_API __declspec (dllexport)
#	else
#		define SPRK_OPS_API __declspec (dllimport)
#	endif
#endif
#else
#	include <stddef.h>
#	if defined(LINUX_SYSTEM) && defined(SPRK_OPS)
#		ifndef STATIC_APP
#			define SPRK_OPS_API __attribute__ ((visibility ("default")))
#		endif
#	endif
#endif

#ifndef SPRK_OPS_API
#	define SPRK_OPS_API 
#endif


namespace HPS
{

/*! The PanOrbitZoomOperator class defines an operator which allows the user to pan, orbit and zoom the camera.
 *  This Operator works for both mouse- and touch-driven devices. 
 *  Mouse-Driven Devices:
 *		Left button pressed and move:			Orbit camera
 *		Left button pressed and move + Shift:	Pan camera
 *		Left button pressed and move + Ctrl:	Zoom camera
 *  Touch-Driven Devices:
 *		Touch down and move:					Orbit camera
 *		Two finger touch and move:				Pan camera
 *		Pinch:									Zoom camera
 *  */
class SPRK_OPS_API PanOrbitZoomOperator : public Operator
{
public:
	PanOrbitZoomOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_PanOrbitZoomOperator"; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 *  This function stops the operator.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function interprets the input to decide whether to orbit, zoom or pan the camera, and updates the camera as needed.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);
	
	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function interprets the input to decide whether to orbit, zoom or pan the camera, and updates the camera as needed.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

private:
	HPS::WorldPoint			start;

	HPS::WindowPoint		start_point, new_point;
	HPS::Vector				start_sphere_pos, new_sphere_pos;

	bool					operator_active;
	float					zoom_limit;
	float					focal_length;
	HPS::WindowPoint		last_zoom;
	HPS::Vector				last_relative;
	float					zoom_start_field_width;
	float					zoom_start_field_height;
	HPS::Vector				zoom_start_camera_look;
	size_t					current_touches;

	void					ZoomStart();
	void					UpdateZoom(float zoom_factor);
	void					UpdateZoomLimit();
	void					UpdatePan(HPS::WorldPoint const & newLocation);
	void					UpdateRoll(HPS::Vector const & relative);
};

/*! The PanOperator class defines an operator which allows the user to pan the camera.
 *  This Operator works for both mouse- and touch-driven devices. 
 *  Mouse-Driven Devices:
 *		Left button pressed and move:			Pan camera
 *  Touch-Driven Devices:
 *		Touch down and move:					Pan camera
 *  */
class SPRK_OPS_API PanOperator : public Operator
{
public:
	PanOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_PanOperator"; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 *  This function stops the operator.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function pans the camera as needed.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function pans the camera as needed.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

private:
	bool					PanCommon(HPS::WindowPoint const & in_loc, HPS::KeyPath const & in_key_path);

	bool					operator_active;
	HPS::TouchID			tracked_touch_ID;
	HPS::WorldPoint			start;
};

/*! The OrbitOperator class defines an operator which allows the user to orbit the camera.
 *  This Operator works for both mouse- and touch-driven devices. 
 *  Mouse-Driven Devices:
 *		Left button pressed and move:			Orbit camera
 *  Touch-Driven Devices:
 *		Touch down and move:					Orbit camera
 *  */
class SPRK_OPS_API OrbitOperator : public Operator
{
public:
	OrbitOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_OrbitOperator"; }
	
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 *  This function stops the operator.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function orbits the camera as needed.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function orbits the camera as needed.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

private:
	bool					OrbitCommon(HPS::WindowPoint const & in_loc);

	bool					operator_active;
	HPS::TouchID			tracked_touch_ID;
	HPS::WindowPoint		start_point, new_point;
	HPS::Vector				start_sphere_pos, new_sphere_pos;
};


class SPRK_OPS_API RelativeOrbitOperator : public Operator
{

public:
	/*!
		Constructor for the HPS::RelativeOrbitOperator class

		\param in_mouse_trigger The mouse button that will trigger this operator
		\param in_modifier_trigger The modifier keys that, with the mouse trigger, will trigger this operator. Optional.
	*/
	RelativeOrbitOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonMiddle(), ModifierKeys in_modifier_trigger = ModifierKeys());
	virtual HPS::UTF8		GetName() const	{ return "HPS_RelativeOrbitOperator"; }
	
	virtual bool			OnMouseDown(MouseState const & in_state);
	virtual bool			OnMouseUp(MouseState const & in_state);
	virtual bool			OnMouseMove(MouseState const & in_state);

	virtual bool			OnTouchDown(TouchState const & in_state);
	virtual bool			OnTouchUp(TouchState const & in_state);
	virtual bool			OnTouchMove(TouchState const & in_state);

private:
	bool					OrbitCommon(HPS::WindowPoint const & in_loc);
	bool					RelativeOrbitCommon(HPS::WindowPoint const & in_loc, HPS::KeyPath & in_event_path);
	void					CalculateTarget(HPS::WindowKey const & in_window);

	HPS::Point					faux_target;
	bool						operator_active;
	HPS::TouchID				tracked_touch_ID;
	HPS::Vector					start_sphere_pos, new_sphere_pos;
	HPS::WindowPoint			start_point, new_point;
	HPS::SelectionOptionsKit	selection_options;
};

/*! The ZoomOperator class defines an operator which allows the user to zoom the camera.
 *  This Operator works for both mouse- and touch-driven devices. 
 *  Mouse-Driven Devices:
 *		Left button pressed and move:			Zoom camera
 *  Touch-Driven Devices:
 *		Pinch:									Zoom camera
 *  */
class SPRK_OPS_API ZoomOperator : public Operator
{
public:
	ZoomOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_ZoomOperator"; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 *  This function stops the operator.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function zooms the camera as needed.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function zooms the camera as needed.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

private:
	bool					operator_active;
	HPS::TouchID			tracked_touch_ID;
	HPS::WindowPoint		start_point;
	float					zoom_limit;
	float					focal_length;
	HPS::WindowPoint		last_zoom;
	HPS::Vector				last_relative;
	float					zoom_start_field_width;
	float					zoom_start_field_height;
	HPS::Vector				zoom_start_camera_look;
	HPS::Point				zoom_start_camera_target;

	void					ZoomStart();
	void					UpdateZoom(float zoom_factor);
	void					UpdateZoomLimit();
};


class SPRK_OPS_API ConstructRectangleOperator : public Operator
{
public:
	ConstructRectangleOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys(), bool draw_faces = false);
	virtual HPS::UTF8		GetName() const	{ return "HPS_ConstructRectangleOperator"; }

	virtual void			OnViewAttached();
	
	virtual bool			OnMouseDown(MouseState const & in_state);
	virtual bool			OnMouseUp(MouseState const & in_state);
	virtual bool			OnMouseMove(MouseState const & in_state);

	virtual bool			OnTouchDown(TouchState const & in_state);
	virtual bool			OnTouchUp(TouchState const & in_state);
	virtual bool			OnTouchMove(TouchState const & in_state);

	bool					IsRectangleValid() const { return is_rect_valid; }
	
	/*! Returns the rectangle associated with this operator. */
	HPS::Rectangle			GetRectangle() const { return rect; }

private:
	bool					ConstructRectCommon(WindowPoint const & in_loc);
	void					SetupConstructionSegment();

	bool					draw_faces;
	bool					use_center_marker;
	bool					is_rect_valid;
	bool					operator_active;
	HPS::WindowPoint		start_point;
	HPS::SegmentKey			scratch_seg;
	HPS::Rectangle			rect;
	HPS::LineKey			temp_line_key;
	HPS::PolygonKey			temp_polygon_key;
	HPS::MarkerKey			temp_marker_key;
	HPS::TouchID			tracked_touch_ID;
};

class SPRK_OPS_API ZoomBoxOperator : public ConstructRectangleOperator
{
public:
	ZoomBoxOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	virtual HPS::UTF8		GetName() const	{ return "HPS_ZoomBoxOperator"; }

	virtual bool			OnMouseUp(MouseState const & in_state);
	virtual bool			OnTouchUp(TouchState const & in_state);

private:
	bool					ZoomCommon(HPS::WindowKey const & in_window, HPS::KeyPath const & in_event_path);

};

/*!
	The HPS::SelectAreaOperator class is a selection mechanism that operates on a
    rectangle input. As with all operators, this operator must be made active on
    a HPS::View object before it can be used. The results of the last selection
    can be obtained by calling GetActiveSelection(). Selection
    options such as granularity, algorithm, proximity, and more are set using a
    HPS::SelectionOptionsKit passed to SetSelectionOptions. See Programming Guide 
    section "User Interaction" for more details on how to use operators.
*/
class SPRK_OPS_API SelectAreaOperator : public ConstructRectangleOperator
{
public:
	/*!
		SelectAreaOperator constructor. The operator is not active until it is
		attached to a HPS::View. Even after being attached, operator must remain 
        in scope until you're done using it.

		\param in_mouse_trigger The mouse button that will trigger this operator.
					            Defaults to left mouse button.
		\param in_modifier_trigger The modifier key that, when used with the
								   in_mouse_trigger, will trigger this operator.
                                   Passing an empty ModifierKeys object means no
                                   modifier key is necessary to trigger the operator.
	*/
	SelectAreaOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	virtual HPS::UTF8				GetName() const	{ return "HPS_SelectAreaOperator"; }

	virtual void					OnViewAttached();

	virtual bool					OnMouseUp(MouseState const & in_state);
	virtual bool					OnTouchUp(TouchState const & in_state);

	/*!
		This method returns the results of the last selection action. If no objects were selected,
		the SelectionResults object returned will be have a count of 0.
	*/
	HPS::SelectionResults			GetActiveSelection() const;

	/*!
		Sets the selection options that will be used as selection criteria for this operator.

		\param in_options The HPS::SelectionOptionsKit that contains the selection options for this operator
	*/
	void							SetSelectionOptions(HPS::SelectionOptionsKit const & in_options) { selection_options = in_options; }

	/*!
		Gets the selection options kit that was set on this operator.
	*/
	HPS::SelectionOptionsKit		GetSelectionOptions() const { return selection_options; }

private:
	bool							SelectCommon(HPS::WindowKey & in_window, HPS::ModifierKeys in_modifiers = HPS::ModifierKeys());

	HPS::SelectionResults			active_selection;
	HPS::SelectionOptionsKit		selection_options;
};


class SPRK_OPS_API HighlightAreaOperator : public SelectAreaOperator
{
public:
	/*!
		Constructor for HighlightAreaOperator

		\param in_mouse_trigger The mouse trigger that will activate this operator
		\param in_modifier_trigger The modifier keys needed to activate this operator. Optional.
	*/
	HighlightAreaOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	virtual HPS::UTF8				GetName() const	{ return "HPS_HighlightAreaOperator"; }

	virtual void					OnViewAttached();
	
	virtual bool					OnMouseUp(MouseState const & in_state);
	virtual bool					OnTouchUp(TouchState const & in_state);

	/*!
		Sets the highlight options that will be used when an entity is highlighted.

		\param in_options The HPS::HighlightOptionsKit containing the highlight options
	*/
	void							SetHighlightOptions(HPS::HighlightOptionsKit const & in_options) { highlight_options = in_options; }

	/*!
		Gets the highlight options kit associated with this operator.
	*/
	HPS::HighlightOptionsKit		GetHighlightOptions() const { return highlight_options; }

private:
	bool							HighlightCommon(WindowKey & in_window, ModifierKeys in_modifiers);

	HPS::HighlightOptionsKit		highlight_options;

};


/*!
	The HPS::SelectOperator class is a selection mechanism that is intended to
    operate on a single selection point. As with all operators, this operator must 
    be made active on a HPS::View object before it can be used. The results of 
	the last selection can be obtained by calling GetActiveSelection(). Selection
    options such as granularity, algorithm, proximity, and more are set using a
    HPS::SelectionOptionsKit passed to SetSelectionOptions. See Programming Guide 
    section "User Interaction" for more details on how to use operators.
*/
class SPRK_OPS_API SelectOperator : public Operator
{
public:
	/*!
		SelectOperator constructor. The operator is not active until it is
		attached to a HPS::View. Even after being attached, operator must remain 
        in scope until you're done using it.

		\param in_mouse_trigger The mouse button that will trigger this operator.
					            Defaults to left mouse button.
		\param in_modifier_trigger The modifier key that, when used with the
								   in_mouse_trigger, will trigger this operator.
                                   Passing an empty ModifierKeys object means no
                                   modifier key is necessary to trigger the operator.
	*/
	SelectOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	virtual HPS::UTF8					GetName() const	{ return "HPS_SelectOperator"; }

	virtual bool						OnMouseDown(MouseState const & in_state);
	virtual bool						OnTouchDown(TouchState const & in_state);

	/*!
		This method returns the results of the last selection action. If no objects were selected,
		the SelectionResults object returned will be have a count of 0.
	*/
	HPS::SelectionResults				GetActiveSelection() const;

	/*! Accepts a SelectionOptionsKit that defines the parameters that will be used for selection. 
	* \param in_options The SelectionOptionsKit from which the selection options will be set 
	*/
	void								SetSelectionOptions(HPS::SelectionOptionsKit const & in_options) { selection_options = in_options; }
	
	/*! Gets the HPS::SelectionOptionsKit that has been set on this operator.
	* \return The HPS::SelectionOptionsKit associated with this operator 
	*/
	HPS::SelectionOptionsKit			GetSelectionOptions() const { return selection_options; }

private:
	bool								SelectCommon(HPS::Point const & in_loc, HPS::WindowKey & in_window, HPS::ModifierKeys in_modifiers = HPS::ModifierKeys());

	HPS::SelectionResults				active_selection;
	HPS::SelectionOptionsKit			selection_options;
};


class SPRK_OPS_API HighlightOperator : public SelectOperator
{
public:
	/*! Constructor for HighlightOperator
	* \param in_mouse_trigger The mouse button that will activate this operator
	* \param in_modifier_trigger The modifier button that, when pressed with the mouse trigger, will activate this operator
	*/
	HighlightOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	virtual HPS::UTF8				GetName() const	{ return "HPS_HighlightOperator"; }
	
	virtual bool					OnMouseDown(MouseState const & in_state);
	virtual bool					OnTouchDown(TouchState const & in_state);

	/*! Accepts a HighlightOptionsKit that defines how an object will be highlighted. 
	* \param in_options The HighlightOptionsKit from which the highlight options will be set 
	*/
	void							SetHighlightOptions(HPS::HighlightOptionsKit const & in_options) { highlight_options = in_options; }

	/*! Gets the HPS::HighlightOptionsKit associated with this operator.
	* \return The HPS::HighlightOptionsKit associated with this operator 
	*/
	HPS::HighlightOptionsKit		GetHighlightOptions() const { return highlight_options; }

private:
	bool							HighlightCommon(HPS::WindowKey & in_window, HPS::ModifierKeys in_modifiers);

	HPS::HighlightOptionsKit		highlight_options;
};

class SPRK_OPS_API MouseWheelOperator : public Operator
{
public:
	enum class ZoomType
	{
		Fast,			//!< The operator optimizes for time. It is possible to zoom through geometry.
		Accurate,		//!< The operator optimizes zooming so that it is not possible to zoom through geometry.
	};

	/*! Constructor for MouseWheelOperator
	* \param in_default_type The default zoom type for this operator.
	* \param in_alternate_type_modifiers The modifier button that, when pressed, will activate the zoom type opposite to that specified in in_default_type.
		Passing an empty ModifierKey object prevents the zoom type from being anything but what was specified in in_default_type. */
	MouseWheelOperator(ZoomType in_default_type = ZoomType::Accurate, HPS::ModifierKeys in_alternate_type_modifiers = HPS::ModifierKeys::KeyControl());

	virtual HPS::UTF8		GetName() const	{ return "HPS_MouseWheelOperator"; }

	/*! Call when scene bounds changes to update the mouse wheel zoom limit */
	void					UpdateZoomLimit();

	virtual bool			OnMouseWheel(HPS::MouseState const & in_state);

	virtual void			OnModelAttached();

	virtual void			OnViewAttached();

private:
	float					zoom_limit;
	ZoomType				zoom_type;
	HPS::ModifierKeys		zoom_modifier;
};

/*! The TurntableOperator class defines an operator which allows the user to orbit the camera along a specific axis.
 *  This Operator works for both mouse- and touch-driven devices. 
 *  Mouse-Driven Devices:
 *		Left button pressed and move:			Orbit camera around z-axis
 *		Mouse wheel:							Orbit camera around y-axis
 *		Left click + Control:					Selects click location as rotation center
 *  Touch-Driven Devices:
 *		Touch down and move:					Orbit camera around z-axis
 *		Two finger touch and move:				Orbit camera around y-axis
 *		Double Tap:								Selects click location as rotation center
 *  */
class SPRK_OPS_API TurntableOperator : public Operator
{
public:
	TurntableOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_TurntableOperator"; }
	
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 *  If the Control button is pressed the click location will become the rotation center point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 *  This function stops the operator.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function orbits the camera around the z-axis.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse wheel moved
	 *  This function orbits the camera around the y-axis.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseWheel(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 *  If a double-tap is received, the touch location will become the rotation center point.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  If a one-finger touch was received, this function orbits the camera around the z-axis.
	 *  If a two-finger touch was received, this function orbits the camera around the y-axis.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

private:
	void					TurntableCommon(HPS::WindowPoint const & delta, HPS::Vector const & rotation_axis);
	void					CalculateCenterPoint(HPS::WindowKey const & window, HPS::Point const & location);

	bool						operator_active;
	bool						center_point_set;
	HPS::WorldPoint				center_point;
	HPS::TouchID				tracked_touch_ID;
	HPS::WindowPoint			start_point;
	HPS::SelectionOptionsKit	selection_options;
};
    
/*! The ZoomFitTouchOperator class defines an operator which fits the camera to the model currently loaded.
 *  This Operator works for touch-driven devices only. 
 *  Touch-Driven Devices:
 *		Double Tap:								Fits the camera to the model currently loaded.
 *  */
class SPRK_OPS_API ZoomFitTouchOperator : public Operator
{
public:
    ZoomFitTouchOperator();
    /*! Returns the name of the operator. */
    virtual HPS::UTF8		GetName() const	{ return "HPS_ZoomFitTouchOperator"; }

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and fits the camera to the model currently loaded, if a double tap was received.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
    virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
    virtual bool            OnTouchUp(TouchState const & in_state);
    
private:
    HPS::TouchID            tracked_touch_ID;
};

/*! The FlyOperator class defines an operator which allows the user to accurately move the camera around the scene.
 *  This Operator works for both mouse- and touch-driven devices. 
 *  Mouse-Driven Devices:
 *		W:										Move camera position and target forward
 *		A:										Pan camera left
 *		S:										Move camera position and target forward
 *		D:										Pan camera right
 *		C:										Pan camera down
 *		Space:									Pan camera up			
 *		Q:										Roll camera left
 *		E:										Roll camera right
 *		Right-button down and move:				Rotate camera in place
 *		Shift:									Walk at double the speed while pressed
 *		Ctrl:									Walk at half the speed while pressed
 *		Mouse Wheel:							Increase / Decrease step length
 *		Shift + Mouse Wheel:					Increase / Decrease field of view
 *  Touch-Driven Devices:
 *		touch -left side of screen:             Pans camera in left or right, moves camera forward or back
 *		touch -right side of screen:            Rotates camera in place
 *		Two-finger touch -right side of screen: Pans camera up or down, rolls camera left or right
 *		The movement speed increases as the user drags his fingers away from the initial touch position
 *  */
class SPRK_OPS_API FlyOperator : public Operator
{
public:
    FlyOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonRight(), ModifierKeys in_modifier_trigger = ModifierKeys());
    /*! Returns the name of the operator. */
    virtual HPS::UTF8		GetName() const	{ return "HPS_FlyOperator"; }

	/*! This function is called whenever HPS receives a KeyboardState event that signals a button was pressed. 
	 *  This function instructs the operator on which direction to move towards.
	 * \param in_state A KeyboardState object describing the current keyboard state.
	 *		W:										Move camera position and target forward
	 *		A:										Pan camera left
	 *		S:										Move camera position and target forward
	 *		D:										Pan camera right
	 *		C:										Pan camera down
	 *		Space:									Pan camera up			
	 *		Q:										Roll camera left
	 *		E:										Roll camera right
	 *		Shift:									Walk at double the speed while pressed
	 *		Ctrl:									Walk at half the speed while pressed
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyDown(KeyboardState const & in_state);
	/*! This function is called whenever HPS receives a KeyboardState event that signals a button was released. 
	 * \param in_state A KeyboardState object describing the current keyboard state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyUp(KeyboardState const & in_state);

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function rotates the camera in place if the right mouse button is down.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
 	virtual bool			OnMouseMove(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse wheel moved
	 *  If shift is not pressed this function increases or decreases the step length.
	 *  If shift is pressed this function increases or decreases the camera field of view.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseWheel(MouseState const & in_state);
 
	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
 	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
 	virtual bool			OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function rotates the camera in place.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
 	virtual bool			OnTouchMove(TouchState const & in_state);

	/*! This function is called whenever HPS receives a TimerTickEvent
	 *  This function moves the camera
	 * \param in_event A TimerTickEvent object describing the current timer tick.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTimerTick(HPS::TimerTickEvent const & in_event);
    
	virtual void			OnModelAttached();
	virtual void			OnViewAttached();
    virtual void            OnViewDetached();

	/* getters/setters for inverting the X and Y axis */
	void					InvertXAxis() { invert_x_axis = !invert_x_axis;}
	void					InvertYAxis() { invert_y_axis = !invert_y_axis;}
	bool					IsXAxisInverted() { return invert_x_axis; }
	bool					IsYAxisInverted() { return invert_y_axis; }

	/* getters/setters for sensitivity */
	/* Keyboard sensitivity affects the speed of movement of action triggered from the keyboard, such as walking forward by pressing W */
	float					GetKeyboardSensitivity() { return keyboard_sensitivity; }
	void					SetKeyboardSensitivity(float in_keyboard_sensitivity) { keyboard_sensitivity = in_keyboard_sensitivity; }

	/* Mouse sensitivity affects the speed of movement of action triggered from the mouse, such as rotating */
	float					GetMouseSensitivity() { return mouse_sensitivity; }
	void					SetMouseSensitivity(float in_mouse_sensitivity) { mouse_sensitivity = in_mouse_sensitivity; }

	/* Left Joystick sensitivity affects the speed of movement of action triggered from the left joystick on touch devices */
	float					GetLeftJoystickSensitivity() { return left_joystick_sensitivity; }
	void					SetLeftJoystickSensitivity(float in_left_joystick_sensitivity) { left_joystick_sensitivity = in_left_joystick_sensitivity; }

	/* Right Joystick sensitivity affects the speed of movement of action triggered from the right joystick on touch devices */
	float					GetRightJoystickSensitivity() { return right_joystick_sensitivity; }
	void					SetRightJoystickSensitivity(float in_right_joystick_sensitivity) { right_joystick_sensitivity = in_right_joystick_sensitivity; }

	/* getters/setters for joystick dead zone
	   The dead zone is an area around the initial touch down position where the user can move its finger
	   without triggering any movement.
	   The default value is 0.07. */
	float					GetJoystickDeadZone() { return joystick_dead_zone; }
	void					SetJoystickDeadZone(float in_dead_zone) { joystick_dead_zone = in_dead_zone; }

protected:
	
	enum MovementFlags
	{
		no_movement				= 0x0000,
		moving_forward			= 0x0001,
		moving_back				= 0x0002,
		moving_left				= 0x0004,
		moving_right			= 0x0008,
		moving_up				= 0x0010,
		moving_down				= 0x0020,
		roll_left				= 0x0040,
		roll_right				= 0x0080,
		rotating				= 0x0100,
		move_with_touch			= 0x0200,
		rotate_with_touch		= 0x0400
	};

	unsigned int movement_flags;

	//movement functions
	void			MoveLeft(HPS::Point & position, HPS::Point & target, HPS::Vector & up);
	void			MoveRight(HPS::Point & position, HPS::Point & target, HPS::Vector & up);
	void			MoveUp(HPS::Point & position, HPS::Point & target, HPS::Vector & up);
	void			MoveDown(HPS::Point & position, HPS::Point & target, HPS::Vector & up);
	void 			MoveForward(HPS::Point & position, HPS::Point & target, HPS::Vector const & direction);
	void			MoveBack(HPS::Point & position, HPS::Point & target, HPS::Vector const & direction);
	void 			MoveWithTouch(HPS::Point & position, HPS::Point & target, HPS::Vector & up, HPS::Vector const & walking_direction);
	void 			RotateScene(HPS::Point & position, HPS::Point & target);
	void 			RotateWithTouch(HPS::Point & position, HPS::Point & target);
	void 			RotateCommon(HPS::Point const & delta, HPS::Point & position, HPS::Point & target);

private:
    
    enum class TouchPosition
    {
        Left,
        Right,
        None
    };
    
    //touch IDs tracked by the operator
    HPS::TouchID            left_side_touch;
    HPS::TouchID            right_side_touch;
    HPS::TouchID            second_right_touch;
    
    //current and start location for touch operations
	HPS::WindowPoint		start_point;
    HPS::WindowPoint		left_start_point;
    HPS::WindowPoint		right_start_point;
    HPS::WindowPoint        current_right_touch_position;
    HPS::WindowPoint        second_right_start_point;
    HPS::WindowPoint        current_second_right_touch_position;
    HPS::WindowPoint        start_mid_point;
    
    //virtual joystick information
    HPS::SegmentKey         left_joystick_segment;
    HPS::SegmentKey         right_joystick_segment;
    float		            joystick_dead_zone;

	//step length for left touch, right touch, and desktop operation
	float					keyboard_sensitivity;
	float                   old_keyboard_sensitivity;
	float					left_joystick_sensitivity;
	float                   old_left_joystick_sensitivity;
	float					right_joystick_sensitivity;
	float                   old_right_joystick_sensitivity;
	float                   mouse_sensitivity;
    
    //operator state flags
    bool                    two_right_fingers_down;
	bool					invert_x_axis;
	bool					invert_y_axis;
    Drawing::Handedness		world_handedness;
	bool					shift_pressed;
	bool					ctrl_pressed;
	HPS::KeyPath			event_path;
	HPS::WindowPoint		rotation_location;
    HPS::WindowPoint        moving_position;

    //utility functions
	void					UpdateKeyboardState(KeyboardState const & in_state);
	void					ComputeReasonableStepLength();
    void                    CreateJoystick(HPS::TouchState const & in_state, TouchPosition touch_position);
};

/*! The WalkOperator class defines an operator which allows the user to accurately move the camera around the scene,
while snapped to a plane. This operator is intended for walking through models that have a defined ground plane, such as a floor in a building. 
For models that don't have a ground plane, Visualize will attempt to determine a ground plane heuristically.
 *  This operator works for both mouse and touch-driven devices.
\htmlonly
 *
 *  <h3>Mouse-driven devices:</h3>
 *	<style>.walkOperator_leftColumn {font-weight:bold; text-align:right; }</style>
 *  <table style="padding-right:5px;">
 *    <tr><td class="td-bk walkOperator_leftColumn">W:</td><td class="td-bk">Move camera position and target forward</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">S:</td><td class="td-bk">Move camera position and target backward</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">A:</td><td class="td-bk">Pan camera left</td></tr>
 *
 *    <tr><td class="td-bk walkOperator_leftColumn">D:</td><td class="td-bk">Pan camera right</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">C:</td><td class="td-bk">Pan camera down</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">space:</td><td class="td-bk">Pan camera up</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Right-button down and move:</td><td class="td-bk">Rotate camera in place</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Shift:</td><td class="td-bk">Walk at double the speed while pressed</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Ctrl:</td><td class="td-bk">Walk at half the speed while pressed</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Mouse wheel:</td><td class="td-bk">Increase / decrease step length</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Shift + mouse wheel:</td><td class="td-bk">Increase / decrease field of view</td></tr>
 * </table>
 *
 *
 * <h3>Touch devices:</h3>
 * <table>
 *    <tr><td class="td-bk walkOperator_leftColumn">Touch left side of screen:</td><td class="td-bk">Pans camera in left or right, moves camera forward or back.</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Touch right side of screen:</td><td class="td-bk">Rotates camera in place.</td></tr>
 *    <tr><td class="td-bk walkOperator_leftColumn">Two-finger touch and move one finger:</td><td class="td-bk">Pans camera up or down.</td></tr>
 *  <tr><td class="td-bk walkOperator_leftColumn">Double-touch anywhere on the screen:</td><td class="td-bk">Re-center the model.</td></tr>
 * </table>
 * <br>
 * 	<p><b>NOTE:</b> The movement speed increases as the user drags his fingers away from the initial touch position</p>
\endhtmlonly
 *  */
class SPRK_OPS_API WalkOperator : public FlyOperator
{
public:
	WalkOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonRight(), ModifierKeys in_modifier_trigger = ModifierKeys());

	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_WalkOperator"; }

	virtual void			OnViewAttached();
	virtual void			OnModelAttached();

	/*! This function is called whenever HPS receives a TimerTickEvent
	 *  This function moves the camera
	 * \param in_state A TimerTickEvent object describing the current timer tick.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTimerTick(HPS::TimerTickEvent const & in_event);

	/* Getters / Setters for the plane the operator walks on. */
	void					SetGroundPlane(HPS::Plane const & in_plane) { ground = in_plane; }
	HPS::Plane				GetGroundPlane() { return ground; }

	/* Getters / Setters for camera vertical offset from the ground plane */
	void					SetWalkerHeight(float height) { height_off_ground = height; }
	float					GetWalkerHeight() { return height_off_ground; }

private:
	HPS::Plane				ground;
	HPS::Vector				walking_direction;
	float					height_off_ground;

	void					CalculateGroundPlane();
	void					SnapToPlane();
	void					AdjustWalkingDirection(HPS::Vector const & camera_direction, HPS::Vector const & camera_up);
};

/*! The SimpleWalkOperator class defines an operator which allows the user to move forward and backwards and rotate 
	while staying snapped to a plane.
 *  This Operator works for both mouse-driven devices only. 
 *  Mouse-Driven Devices:
 *	Left-mouse button down and move:			Pans the camera forward and backward, or rotates the scene
 *												based on the movement direction. The mouse sensitivity changes
 *												based on how far the mouse is dragged from the starting point
 *  Shift + Mouse Wheel:						Increases / Decreases field of view
 *  */
class SPRK_OPS_API SimpleWalkOperator : public WalkOperator
{
public:
	SimpleWalkOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const { return "HPS_SimpleWalkOperator"; }

	/*! This function is called whenever HPS receives a KeyboardState event that signals a button was pressed. 
	 *  This operator does not handle key down events.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyDown(KeyboardState const & in_state) { HPS_UNREFERENCED(in_state); return false; }
	/*! This function is called whenever HPS receives a KeyboardState event that signals a button was released. 
	 *  This operator does not handle key up events.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyUp(KeyboardState const & in_state) { HPS_UNREFERENCED(in_state); return false; }
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse wheel moved
	 *  This operator does not handle mouse wheel events.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseWheel(MouseState const & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function rotates the camera in place and moves forward/back if the left mouse button is down.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This operator does not handle touch down events.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state) { HPS_UNREFERENCED(in_state); return false; }
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This operator does not handle touch move events.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state) { HPS_UNREFERENCED(in_state); return false; }
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This operator does not handle touch up events.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a TimerTickEvent
	 *  This function moves the camera
	 * \param in_state A TimerTickEvent object describing the current timer tick.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTimerTick(HPS::TimerTickEvent const & in_event);	

private:
	void					MoveWithMouse(HPS::Point & position, HPS::Point & target);

	HPS::KeyPath			event_path;
	HPS::WindowPoint		rotation_location;
	HPS::WindowPoint		start_point;
	bool					moving;
};


/*! 
 \htmlonly
 <p>The CuttingSectionOperator class defines an operator which allows the user to insert and interact with cutting sections. This Operator works for both mouse- and touch-driven devices. Each instance of the operator supports inserting and interacting with a single cutting plane. Multiple instances of the operator can be pushed to the View’s operator stack when the user wants to add an additional cutting plane to the scene. The cutting plane associated with an instance of the operator will be removed when the operator is popped from the View’s operator stack.</p>
 
 <h4>Mouse-driven devices:</h4>
 <table>
    <tr>
		<th>Action</th>
		<th>Result</th>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Mouse move</td>
		<td class="td-bk">When the operator is first activated, moving the mouse over the model will display a indicator
            representing the normal of the face the user is currently hovering over. Once a cutting section has been inserted, it will be highlighted when the user hovers over it. The highlight options can also be customized, or this behavior can be disabled entirely.</td>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Left click</td>
        <td class="td-bk">When the operator is first activated, a left click will insert a cutting section at this position.</td>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Double left click</td>
        <td class="td-bk">Flips the cutting section the user double clicked on.</td>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Left mouse down and move</td>
        <td class="td-bk">Translates the cutting section the user clicked on along its normal.</td>
	</tr>
 </table>
 
 <h4>Touch-driven devices:</h4>
 <table>
	<tr>
		<th>Action</th>
		<th>Result</th>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Touch down</td>
		<td class="td-bk">Inserts a cutting section at the position of the touch.</td>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Double tap</td>
		<td class="td-bk">Flips the cutting section the user double tapped.</td>
	</tr>
	
	<tr>
		<td class="td-bk" style="width: 33%;">Touch down and drag</td>
		<td class="td-bk">Translates the cutting section the user tapped along its normal.</td>
	</tr>
 </table>
 \endhtmlonly
 */
class SPRK_OPS_API CuttingSectionOperator : public SelectOperator
{
private:
	enum class OpState
	{
		Uninitialized,
		Initialized,
		Translating,
		FacePicking,
	};

public:
	CuttingSectionOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	/*! Returns the name of the operator. */
	virtual HPS::UTF8				GetName() const	{ return "HPS_CuttingSectionOperator"; }
	
	virtual void					OnModelAttached();
	virtual void					OnViewAttached();
	virtual void					OnViewDetached();

	/* This function programmatically assigns cutting planes to this operator from plane equations.
	 * If this operator has already had cutting planes associated with it, the cutting planes set using
	 * this function replace the previous cutting planes.
	 * \param in_planes An array of Plane objects describing the cutting planes to be inserted. */
	void							SetPlanes(PlaneArray const & in_planes);

	/* This function returns the planes associated with the active cutting sections
	 * \return planes associated with the active cutting sections. */
	PlaneArray						GetPlanes();

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 *  When the operator is first started, this function inserts a cutting section.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnMouseDown(MouseState const & in_state);

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnMouseUp(MouseState const & in_state);

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  When the operator is first started this function displays a normal indicator when the user mouses over geometry.
	 *  After a cutting section has been inserted this function allows the user to translate a cutting section along its normal
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnMouseMove(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and computes the position of the starting point.
	 *  When the operator is first started, this function inserts a cutting section.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnTouchDown(TouchState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnTouchUp(TouchState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  After a cutting section has been inserted this function allows the user to translate a cutting section along its normal
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnTouchMove(TouchState const & in_state);

	/*! This function is called whenever HPS receives a TimerTickEvent
	 *  This function moves the camera
	 * \param in_state A TimerTickEvent object describing the current timer tick.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool					OnTimerTick(TimerTickEvent const & in_event);

	/*! Returns the scale used for the normal indicator displayed while mousing over the model when the operator is first started.
	 * \return The scale of the normal indicator. */
	float							GetIndicatorScale() const { return indicator_scale; }

	/*! Sets the scale of the normal indicator displayed while mousing over the model when the operator is first started.
	 * \param in_scale The scale of the normal indicator */
	void							SetIndicatorScale(float in_scale) { indicator_scale = in_scale; }

	/*! Returns the material mapping kit applied to the cutting plane geometry.
	 * \return The material mapping kit applied to the cutting plane geometry. */
	MaterialMappingKit				GetPlaneMaterial() const { return plane_material; }

	/*! Sets the material mapping kit applied to the cutting plane geometry.
	 * \param in_plane_material The material mapping kit applied to the cutting plane geometry. */
	void							SetPlaneMaterial(MaterialMappingKit const & in_plane_material);

	/*! Returns the highlight options kit used when hovering over geometry, when mouse-over highlighting is enabled.
	 * \return the highlight options kit used when hovering over geometry, when mouse-over highlighting is enabled. */
	HighlightOptionsKit				GetPlaneHighlightOptions() const { return highlight_options; }

	/*! Sets the highlight options kit used when hovering over geometry, when mouse-over highlighting is enabled.
	 * \param in_highlight_options The highlight options kit used when hovering over geometry, when mouse-over highlighting is enabled. */
	void							SetPlaneHighlightOptions(HighlightOptionsKit const & in_highlight_options) { highlight_options = in_highlight_options; }

	/*! Whether mouse-over highlighting is enabled
	 * \return <span class='code'>true</span> if mouse-over highlighting is enabled, <span class='code'>false</span> otherwise. */
	bool							GetMouseOverHighlighting() const { return enable_mouse_over_highlighting; }

	/*! Sets the mouse-over highlighting behavior
	 * \param in_enable_mouse_over_highlighting Whether to enable mouse-over highlighting. */
	void							SetMouseOverHighlighting(bool in_enable_mouse_over_highlighting) { enable_mouse_over_highlighting = in_enable_mouse_over_highlighting; }

	/*! Sets the sectioning behavior
	 *  When sectioning is turned on the cutting planes are grouped together into a section
	 * \param in_sectioning Whether to enable sectioning. */
	void							SetSectioning(bool in_sectioning);

	/*! Whether sectioning is enabled
	 * \return <span class='code'>true</span> if sectioning is enabled, <span class='code'>false</span> otherwise. */
	bool							GetSectioning() { return sectioning; }

	/*! This function allows the user to pick a cutting sections through the aid of a normal indicator when moving the mouse
	 *	By default this mode is active when the operator is fist attached to a View
	 * \param in_use_indicator Whether to insert cutting sections through a normal indicator. */
	void							SetIndicatorVisibility(bool in_use_indicator);

	/*! Whether the plane normal indicator is visible
	 * \return <span class='code'>true</span> if the plane normal indicator is visible, <span class='code'>false</span> otherwise. */
	bool							GetIndicatorVisibility() { return (op_state == OpState::Uninitialized || op_state == OpState::FacePicking) ? true : false; }

	/*! Sets the visibility of cutting sections plane representation
	 *  The cutting sections plane representation visibility is on by default.
	 * \param in_visibility Whether to enable the visibility of the cutting sections plane representation. */
	void							SetPlaneVisibility(bool in_visibility);

	/*! Whether the cutting sections plane representations are visible
	 * \return <span class='code'>true</span> if the cutting sections plane representations are visible, <span class='code'>false</span> otherwise. */
	bool							GetPlaneVisibility();
	
private:

	void							SetupOperatorSegment();
	void							InsertNormalIndicator(float scale);
	ShellKey						InsertCuttingPlaneGeometry();
	void							MouseOverHighlighting(MouseState const & in_state);
    void                            TranslateCuttingPlane(KeyPath const & in_event_path, WindowPoint const & in_event_location);
    bool                            HandleMouseAndTouchDown(WindowKey const & in_event_source, int in_number_of_clicks,
                                                            KeyPath const & in_event_path, WindowPoint const & in_event_location);
	void							ViewAlignSectionPlanes(HPS::PlaneArray & in_out_planes) const;
	typedef std::pair<CuttingSectionKey, std::vector<ShellKey>>			SectionInfo;
	typedef std::vector<SectionInfo>									SectionArray;
	SectionArray					sections;

	SegmentKey						operator_root_segment;
	SegmentKey						cutting_sections_segment;
	SegmentKey						plane_representation_segment;
	bool							sectioning;

	CuttingSectionKey				translating_section;
	size_t							translating_plane_offset;
	ShellKey						translating_plane_representation;

    TouchID                         tracked_touch_ID;
	MaterialMappingKit				plane_material;
	SegmentKey						indicator_seg;
	OpState							op_state;
	WorldPoint						start_world_point;
	SelectionOptionsKit				selection_options;
	SelectionOptionsKit				mouse_over_selection_options;
	float							indicator_scale;
	Vector							plane_normal;
	bool							plane_normal_valid;
	WorldPoint						anchor_point;
	SimpleCuboid					model_bounding;

	UpdateNotifier					last_highlight_notifier;
	bool							last_skipped_highlight_state_valid;
	MouseState						last_skipped_highlight_state;
	bool							is_plane_highlighted;
	PortfolioKey					portfolio;
	SegmentKey						style_segment;
	HighlightOptionsKit				highlight_options;
	bool							enable_mouse_over_highlighting;
	bool							skip_mouse_overs;

	View							attached_view;
	KeyArray						navigation_keys;
};

/*! 
 The MarkupOperator class defines an operator which allows the user to insert mark-up text and geometry. 
 This Operator works for both mouse- and touch-driven devices. 
 The markup created through this operator will be deleted when the operator is popped from the View.
 
 The MarkupOperator allows users to add the following markup types:
 - Freehand: 
	For mouse driven devices, click and move the mouse to draw freehand markup.
	For touch driven devices, touch down and move your finger to draw freehand markup

 - Text:
	For mouse driven devices, click where you want to place text. Type with the keyboard and click elsewhere when
	you are done inserting the note.
	For touch driven devices, tap where you want to place text. Use the software keyboard to type, and hide the
	keyboard when you are done inserting the note.
    When inserting text markup on touch devices, the view will shift while inserting text, to allow you to always
    see the text you are inserting. The view will reset smoothly once you hide the keyboard.

 - Circle:
	For mouse driven devices, click where you want to place the circle. Move the mouse while holding down the button 
	associated with the operator to change the radius of the circle.
	For touch driven devices, there are two options available for inserting circle markups:
		a. tap where you want to place the circle and move your finger to change the circle radius
		b. tap with two fingers. Move and pinch with the fingers to change the position and radius of the circle

 - Rectangle:
	For mouse driven devices, click where you want to place the top left corner of the rectangle, Move the mouse while holding
	down the button associated with the operator to change the position of the bottom right corner of the rectangle.
	For touch driven devices, there are two options available for inserting rectangle markups:
		a. tap where you want to place the top left corner of the rectangle and move your finger to change the position of the bottom right corner.
		b. tap with two fingers. Move and pinch with the fingers to change the position and size of the rectangle.

	On devices with a keyboard, pressing Escape will delete the markup currently being inserted.
	An event of type MarkupInsertedEvent in injected after a markup is inserted.
 */
class SPRK_OPS_API MarkupOperator : public Operator
{
public:
	enum class MarkupType
	{
		Freehand,
		Text,
		Circle,
		Rectangle,
	};

	MarkupOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	OVERRIDE { return "HPS_MarkupOperator"; }

	virtual void			OnViewAttached() OVERRIDE;
	virtual void			OnViewDetached() OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts inserting markup. In the case of text markup, this function determines where the text will be inserted.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  When the user is inserting freehand markup, this function continues the markup.
	 *  When the user is inserting a circle, this function changes the circle's radius.
	 *  When the user is inserting a rectangle, this function changes the rectangle's size.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state) OVERRIDE;


	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts inserting markup. In the case of text markup, this function determines where the text will be inserted.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  When the user is inserting freehand markup, this function continues the markup.
	 *  When the user is inserting a circle, this function changes the circle's radius.
	 *  When the user is inserting a rectangle, this function changes the rectangle's size.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a KeyboardState event that signals a button was pressed. 
	 *  This function is used when to type markups of type Text from a desktop.
	 *  To signal that you are done writing a note, press ESC.
	 * \param in_state A KeyboardState object describing the current keyboard state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyDown(KeyboardState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TextInput event that signals a text string was received. 
	 *  This function is used when to type markups of type Text from a mobile device.
	 *  To signal that you are done writing a note, inject a HideKeyboardEvent.
	 * \param in_text The text string received.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
    virtual bool            OnTextInput(HPS::UTF8 const & in_text) OVERRIDE;

	/*! Returns the type of markup the operator will insert. */
	MarkupType				GetMarkupType() { return markup_type; }

	/*! Changes the type of markup the operator will insert. */
	void					SetMarkupType(MarkupType in_markup_type) { markup_type = in_markup_type; }

	/*! Returns the color of the markup which will be inserted.
	 * \return The color which will be used for the next markup */
	RGBColor				GetColor() { return current_attributes.color; }

	/*! Changes the color of the markup which will be inserted. */
	void					SetColor(RGBColor const & in_color) { current_attributes.color = in_color; }

	/*! Changes the color of the markup which will be inserted.
	* \return The TextAttributeKit which will be used for the next markup */
	TextAttributeKit		GetTextAttributes() { return current_attributes.text_attributes; }

	/*! Returns the text attributes of the markup which will be inserted. */
	void					SetTextAttribute(TextAttributeKit const & in_text_attributes) { current_attributes.text_attributes = in_text_attributes; }

	/*! Returns the line attributes of the markup which will be inserted. 
	 * \return The LineAttributeKit which will be used for the next markup */
	LineAttributeKit		GetLineAttributes() { return current_attributes.line_attributes; }

	/*! Changes the color of the markup which will be inserted. */
	void					SetLineAttribute(LineAttributeKit const & in_line_attributes) {current_attributes.line_attributes = in_line_attributes; }

	/*! Returns the top markup segment.
	 * This segment will not be valid before the operator is attached to the view, and after it is detached from the view.
	* \return The segment containing all the markups inserted by this operator */
	SegmentKey				GetSegmentKey() { return markup_segment; }

	/*! Deletes all markups */
	void					DeleteMarkups();

	/* Whether a markup is currently being inserted.
	* \return <span class='code'>true</span> if a markup is being inserted, <span class='code'>false</span> otherwise. */
	bool					IsMarkupActive() { return operator_active; }

	class SPRK_OPS_API MarkupInsertedEvent : public HPS::Event
	{
		public:
		/*! The default constructor creates an empty MarkupInsertedEvent object. */
		MarkupInsertedEvent() : Event()
		{
			channel = GetClassID();
			consumable = false;
		}

		MarkupInsertedEvent(HPS::Key const & in_markup_key, HPS::View const & in_view) : Event()
		{
			channel = GetClassID();
			consumable = false;
			markup_key = in_markup_key;
			view = in_view;
		}

		/*! This constructor converts an Event Object to a MarkupInsertedEvent object.
		 * 	\param in_event The Event Object to be converted. */
		MarkupInsertedEvent(Event const & in_event) : Event(in_event)
		{
			if (in_event.GetChannel() == Object::ClassID<MarkupInsertedEvent>())
			{
				auto that = static_cast<MarkupInsertedEvent const &>(in_event);
				markup_key = that.markup_key;
				view = that.view;
			}
			else
				throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
		}

		~MarkupInsertedEvent();

		/*! Allocates and returns a copy of this MarkupInsertedEvent.
		 * 	\return A copy of this MarkupInsertedEvent. */
		Event * Clone() const
		{
			MarkupInsertedEvent * new_event = new MarkupInsertedEvent(*this);
			return new_event;
		}

		Key markup_key;
		View view;
	};

private:
	struct Attributes
	{
		RGBColor color;
		LineAttributeKit line_attributes;
		TextAttributeKit text_attributes;
	};
    
    class KeyboardHiddenEventHandler : public EventHandler
    {
    public:
        KeyboardHiddenEventHandler()
        : EventHandler()
        { }
        
        virtual ~KeyboardHiddenEventHandler()
        { Shutdown(); }
        
        virtual HandleResult Handle(Event const * /*in_event*/)
        {
            markup_operator->keyboard_active = false;
            
            markup_operator->ResetCameras();
            return HandleResult::Handled;
        }
        
        void SetOperator(MarkupOperator * in_operator) { markup_operator = in_operator; }
        
    private:
        MarkupOperator * markup_operator;
    };

	std::unordered_multimap<RGBColor, SegmentKey> attribute_map;

	MouseButtons			mouse_trigger;
	ModifierKeys			modifier_trigger;

	MarkupType				markup_type;
	SegmentKey				markup_segment;
	SegmentKey				current_segment;
	Attributes				current_attributes;

	static const RGBColor			default_color;
	static const TextAttributeKit	default_text_attributes;
	static const LineAttributeKit	default_line_attributes;

	Point					start_point;
	bool					start_new_line;
	LineKey					current_line;
	size_t					current_line_size;
	CircleKey				current_circle;
	LineKey					current_circle_line;
	LineKey					current_rectangle;
	View					last_attached_view;

	bool					start_new_note;
    bool                    keyboard_active;
	TextKey					default_text;
	TextKey					current_text;
	SizeTArray				text_columns;
	size_t					current_text_row;

	bool					operator_active;
	TouchID					tracked_touch_id;
    TouchID                 second_tracked_touch_id;
    
    CameraKit               original_camera;
    CameraKit               original_markup_camera;
    
    KeyboardHiddenEventHandler handler;

	bool					SetupConstructionSegments();
	void					LookupSegment();
	void					CreateNewMarkupSegment();
	void					DrawFreehand(Point const & location);
	void					DrawText();
	void					DrawCircle(Point const & location);
    void                    DrawCircleFromTwoPoints(Point const & point_one, Point const & point_two);
    void                    DrawRectangleFromTwoPoints(Point const & point_one, Point const & point_two);
	void					DrawRectangle(Point const & location);
    void                    CenterCameras(Point const & main_camera_center, Point const & markup_camera_center);
    void                    ResetCameras();
	void					EndTextNote();
};

/*! 
 * The AnnotationOperator class defines an operator which allows the user to insert annotations.
 * Annotations are text geometries with leader lines and a background.
 * This Operator works for both mouse- and touch-driven devices. 
 *
 * To operate this operator on a mouse-driven device:
 * Click on a piece of geometry and drag the mouse to position an annotation.
 * Start typing to change the annotation text.
 * Once an annotation has been inserted it can be repositioned, and its text can be changed after clicking on it.
 *
 * To operate this operator on a touch-driven device:
 * Tap on a piece of geometry and drag your finger to position an annotation.
 * Start typing to change the annotation text.
 * Once an annotation has been inserted, tap on it to edit the text.
 * Drag a previously inserted annotation around to reposition it. 
 * 
 * An event of type AnnotationInsertedEvent is injected when new annotations are added to the scene.*/
class SPRK_OPS_API AnnotationOperator : public Operator
{
public:
	AnnotationOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	OVERRIDE { return "HPS_AnnotationOperator"; }

	virtual void			OnViewAttached() OVERRIDE;
	virtual void			OnViewDetached() OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  If the user clicks on a piece of geometry other than an existing annotation, a new annotation will be inserted.
	 *  If the user clicks on an already inserted annotation, that annotation will become editable (it can be repositioned,
	 *  and its text can be changed). 
	 *  The user can click on the background to stop editing an annotation.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  When users are inserting or editing an annotation, they will be able to reposition it by dragging the mouse
	 *  while holding down the trigger button specified in the operator constructor (defaults to Left mouse button)
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state) OVERRIDE;


	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
  	 *  If the user taps a piece of geometry other than an existing annotation, a new annotation will be inserted.
	 *  If the user taps an already inserted annotation, that annotation will become editable (it can be repositioned,
	 *  and its text can be changed). 
	 *  The user can tap on the background to stop editing an annotation.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  When users are inserting or editing an annotation, they will be able to reposition it by tapping it and dragging their finger
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state) OVERRIDE;


	/*! This function is called whenever HPS receives a KeyboardState event that signals a button was pressed. 
	 *  This function is used to edit the text of annotations
	 * \param in_state A KeyboardState object describing the current keyboard state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyDown(KeyboardState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TextInput event that signals a text string was received. 
	 *  This function is used to edit the text of annotations from a mobile device.
	 * \param in_text The text string received.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
    virtual bool            OnTextInput(HPS::UTF8 const & in_text) OVERRIDE;

	/*! This function sets the shape which will be use for the annotation background.
	 *  The shape name provided has to match the name of a shape defined in a portfolio accessible by 
	 *  the view attached to this operator, or to a segment above that.
	 *  This setting influences all shapes inserted by this operator after it is called, 
	 *  it does not change the shape of annotations already inserted by this operator.
	 *  By default a rectangle shape ('box') is used.
	 * \param in_shape The name of the shape definition used when inserting annotations. */
	void					SetBackgroundShape(const char * in_shape);

	/*! This function returns the name of the shape definition currently used to 
	 *  draw the annotation background by this operator.
	 * \return The name of the shape definition currently used to draw the annotation background */
	UTF8					GetBackgroundShape() const;

	/* An event injected every time a new annotation is inserted. 
	 * It contains the key of the segment containing the annotation and the view currently attached to this operator.
	 * This operator places each annotation in a separate segment. */
	class SPRK_OPS_API AnnotationInsertedEvent : public HPS::Event
	{
		public:
		/*! The default constructor creates an empty AnnotationInsertedEvent object. */
		AnnotationInsertedEvent() : Event()
		{
			channel = GetClassID();
			consumable = false;
		}

		AnnotationInsertedEvent(HPS::SegmentKey const & in_text_key, HPS::View const & in_view) : Event()
		{
			channel = GetClassID();
			consumable = false;
			text_key = in_text_key;
			view = in_view;
		}

		/*! This constructor converts an Event Object to a MarkupInsertedEvent object.
		 * 	\param in_event The Event Object to be converted. */
		AnnotationInsertedEvent(Event const & in_event) : Event(in_event)
		{
			if (in_event.GetChannel() == Object::ClassID<AnnotationInsertedEvent>())
			{
				auto that = static_cast<AnnotationInsertedEvent const &>(in_event);
				text_key = that.text_key;
				view = that.view;
			}
			else
				throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
		}

		~AnnotationInsertedEvent();

		/*! Allocates and returns a copy of this AnnotationInsertedEvent.
		 * 	\return A copy of this MarkupIAnnotationInsertedEventnsertedEvent. */
		Event * Clone() const
		{
			AnnotationInsertedEvent * new_event = new AnnotationInsertedEvent(*this);
			return new_event;
		}

		SegmentKey text_key;	//the key of the segment containing the annotation
		View view;
	};

private:
	void AddAnnotationUserData();
	void StartNewNote();

    bool InputDown(WindowKey const & window_key, Point const & location, KeyPath & event_path);
    bool InputMove(Point const & location, KeyPath & event_path);

	void CenterCamera(Point const & camera_center);
	void ResetCamera();

    class KeyboardHiddenEventHandler : public EventHandler
    {
    public:
        KeyboardHiddenEventHandler()
        : EventHandler()
        { }
    
        virtual ~KeyboardHiddenEventHandler()
        { Shutdown(); }
    
        virtual HandleResult Handle(Event const * /*in_event*/)
        {
            annotation_operator->keyboard_active = false;
        
            annotation_operator->ResetCamera();
            return HandleResult::Handled;
        }
    
        void SetOperator(AnnotationOperator * in_operator) { annotation_operator = in_operator; }
        
    private:
        AnnotationOperator * annotation_operator;
    };


	SegmentKey annotation_segment;						//segment containing all annotations
	View last_attached_view;							//the last view that was attached to this operator
	SelectionOptionsKit selection_options;				//selection options used to select geometry
	SelectionOptionsKit annotation_selection_options;	//selection options used when trying to select annotations
	bool editing_text;									//whether we are in a state which allows editing of an annotation
	bool start_new_note;                                //whether we should delete the text of an annotation when typing
	UTF8 annotation_user_data;							//the user data set on annotation geometry to distinguish it from non annotation geometry
	UTF8 annotation_shape;								//the name of the shape definition used to draw the annotation background
	
	PortfolioKey current_portfolio;						//the portfolio used to store the default definitions used by this operator
	SegmentKey highlight_style_segment;					//the highlight background style

	RGBColor background_color;							//the color of the annotation background
	RGBColor leader_line_color;							//the color of the annotation leader line and edges
	RGBColor highlight_color;							//the color the annotation becomes while it is being edited

	TextKey text_to_edit;								//the annotation text which we are working on
	SizeTArray text_columns;							//the number of columns of the text for the current annotation
	size_t current_text_row;							//the number of rows of the text for the current annotation

	//touch only flags -- these flags are only relevant when running the operator on a touch enabled device
    TouchID tracked_touch_id;							//the ID of the touch we are tracking
    bool keyboard_active;								//whether the software keyboard is visible
	CameraKit original_camera;							//the camera setting as they were just before the annotation text is edited
    KeyboardHiddenEventHandler handler;					//a handler that takes care of resetting the camera when the software keyboard is hidden
    Point touch_down_position;							//the position the last touch down occurred at
    bool always_show_keyboard_on_touch_up;				//true if we are just inserting the annotation, false if we are editing a previously inserted one.
};

/*! 
 * The HandlesOperator class defines an operator which allows the user to move geometry.
 * This Operator works for both mouse- and touch-driven devices. 
 *
 * To operate this operator on a mouse-driven device:
 * Double click on a piece of geometry to activate the handles
 * Click and drag the handles to move the selected geometry
 * Click on the background to dismiss the handles
 *
 * To operate this operator on a touch-driven device:
 * Double tap on a piece of geometry to activate the handles
 * Tap and drag the handles to move the selected geometry
 * Tap on the background to dismiss the handles
 *
 * This operator works on the segment containing the geometry selected. 
 * An event of type GeometryTransformedEvent in injected after the handles are dismissed, containing the information
 * on which segment was affected and what transform was applied to it.*/
class SPRK_OPS_API HandlesOperator : public Operator
{
public:
	HandlesOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());

	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	OVERRIDE { return "HPS_HandlesOperator"; }

	virtual void			OnViewAttached() OVERRIDE;
	virtual void			OnViewDetached() OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  If the user double clicks on a piece of geometry, handles will be inserted there.
	 *  If the user clicks on a handle, the segment containing that handle will be translated or rotated appropriately.
	 *  The user can click on the background to remove the handles.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  When users click and drag a handle, the associated geometry will be translated or rotated as appropriate.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
  	 *  If the user double taps a piece of geometry, handles will be inserted there.
	 *  If the user taps on a handle, the segment containing that handle will be translated or rotated appropriately.
	 *  The user can tap on the background to remove the handles.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  When users click and drag a handle, the associated geometry will be translated or rotated as appropriate.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state) OVERRIDE;

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const & in_state) OVERRIDE;

	/*! This function is used to provide a set of colors which will be applied to the handles.
	 * The changes will take effect the next time the handles are inserted.
	 * The materials need to contain a diffuse color, and will be applied as follows:
	 *	Entry 0		-	translation handle in the x direction
	 *	Entry 1		-	translation handle in the y direction
	 *	Entry 2		-	translation handle in the z direction
	 *	Entry 3		-	rotation handle in the x direction
	 *	Entry 4		-	rotation handle in the y direction
	 *	Entry 5		-	rotation handle in the z direction
	 * \param in_colors An array of materials to be applied to the handles. */
	void					SetHandlesColors(MaterialKitArray const & in_colors) { handles_colors = in_colors; }

	/* This function returns the materials used to color the handles.
	 * Only the diffuse color field of the materials is guaranteed to have a value.
	 * The materials are used as follows:
	 *
	 *	Entry 0		-	translation handle in the x direction
	 *	Entry 1		-	translation handle in the y direction
	 *	Entry 2		-	translation handle in the z direction
	 *	Entry 3		-	rotation handle in the x direction
	 *	Entry 4		-	rotation handle in the y direction
	 * \return An array of materials associated with the handles */
	MaterialKitArray		GetHandlesColors() const { return handles_colors; }

	/* Sets the highlight color used when the user mouses over the handles.
	* The default value for the highlight color is RGBAColor(1.0f, 0.55f, 0.0f)
	* \param in_highlight_color The color used to highlight handles when the user mouses over them. */
	void					SetHighlightColor(RGBAColor const & in_highlight_color);

	/* Returns the color used to highlight the handles then the user mouses over them.
	 * \return the color used to highlight the handles then the user mouses over them. */
	RGBAColor				GetHighlightColor() const { return highlight_color; }

	/* This function can be used by the user to modify the appearance of the handles.
	*  If the user passes values outside of the acceptable ranges, these values will be ignored and a warning will be issued for each out-of-range value.
	* Changes will take effect the next time handles are inserted.
	* \param in_length The length of the handles, specified as a percentage of the view containing it. Valid range is (0, 1].
	* \param in_radius The radius of the handles, specified as a percentage of the view containing it. Valid range is (0, 1].
	* \param in_offset The distance between the end of a translation handle and the rotation handle associated with it, specified as a percentage of the handle length. Valid range is (0, 1].
	* \param in_angle The angle formed by the rotation handles, specified in degrees. Valid range is > 0
	* \param in_handles_points The number of points used to draw the handles. A higher number will yield a smoother appearance. Valid range is > 0
	* \param in_tip_size The size of the tip at the end of the handles, specified as a percentage of the handle. Valid range is [0, 1] */
	void					SetHandlesAppearance(float in_length = 0.2f, float in_radius = 0.005f, float in_offset = 0.2f, float in_angle = 25.0f, size_t in_handles_points = 30, float in_tip_size = 0.15f);

	/* Returns the parameters used to decide the appearance of the handles.
	* \param out_length The length of the handles, specified as a percentage of the view containing it.
	* \param out_radius The radius of the handles, specified as a percentage of the view containing it.
	* \param out_offset The distance between the end of a translation handle and the rotation handle associated with it, specified as a percentage of the handle length.
	* \param out_angle The angle formed by the rotation handles, specified in degrees.
	* \param out_handle_points The number of points used to draw the handles. A higher number will yield a smoother appearance.
	* \param out_tip_size The size of the tip at the end of the handles, specified as a percentage of the handle. */
	void					GetHandlesAppearance(float & out_length, float & out_radius, float & out_offset, float & out_angle, size_t & out_handles_points, float & out_tip_size) const;

	/* Whether to draw trailing geometry when using the handles.
	 * Trailing geometry is geometry which shows you the path of the last movement operation you performed through the handles.
	 * \param in_state Whether to draw trailing geometry when using the handles. */
	void					SetTrailingGeometry(bool in_state) { display_trailing_geometry = in_state; }

	/* Whether trailing geometry will be drawn when using the handles.
	 * Trailing geometry is geometry which shows you the path of the last movement operation you performed through the handles.
	 * \return <span class='code'>true</span> if trailing geometry will be drawn, <span class='code'>false</span> otherwise. */
	bool					GetTrailingGeometry() const { return display_trailing_geometry; }

	class SPRK_OPS_API GeometryTransformedEvent : public HPS::Event
	{
		public:
		/*! The default constructor creates an empty GeometryTransformedEvent object. */
		GeometryTransformedEvent() : Event()
		{
			channel = GetClassID();
			consumable = false;
		}

		GeometryTransformedEvent(HPS::View const & in_view, HPS::SegmentKey const & in_segment_key, HPS::MatrixKit const & in_transform) : Event()
		{
			channel = GetClassID();
			consumable = false;
			view = in_view;
			segment_key = in_segment_key;
			transform = in_transform;
		}

		/*! This constructor converts an Event Object to a GeometryTransformedEvent object.
		 * 	\param in_event The Event Object to be converted. */
		GeometryTransformedEvent(Event const & in_event) : Event(in_event)
		{
			if (in_event.GetChannel() == Object::ClassID<GeometryTransformedEvent>())
			{
				auto that = static_cast<GeometryTransformedEvent const &>(in_event);
				view = that.view;
				segment_key = that.segment_key;
				transform = that.transform;
			}
			else
				throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
		}

		~GeometryTransformedEvent();

		/*! Allocates and returns a copy of this GeometryTransformedEvent.
		 * 	\return A copy of this GeometryTransformedEvent. */
		Event * Clone() const
		{
			GeometryTransformedEvent * new_event = new GeometryTransformedEvent(*this);
			return new_event;
		}

		SegmentKey segment_key;
		MatrixKit transform;
		View view;
	};

private:
	enum class Movement
	{
		None,
		Translation,
		Rotation,
	};

	CanvasArray				canvases;					//A list of Canvas objects that the View attached to this operator belongs to. This value only gets updated on OnViewAttached.
	View					last_attached_view;			//The last View to where this operator was pushed.
	TouchID					tracked_touch_id;			

	SegmentKey				handles;					//top level segment
	SegmentKey				handles_segment;			//segment where the matrices for the handles transformations are accumulated
	SegmentKey				handles_geometry;			//segment where the handles geometry lives
	SegmentKey				reference_segment;			//segment containing all the net attributes of segment_to_move
	SegmentKey				segment_to_move;			//segment containing geometry affected by the handles
	SegmentKey				temporary_segment_to_move;	//segment where we accumulate changes while the handles are active
	SegmentKey				handles_trail;				//segment containing the trailing geometry
	KeyPath					path_to_segment_to_move;

	WorldPoint				handles_origin_point;
	WorldPoint				movement_start_point;
	Point					input_down_position;

	Camera::Projection		camera_projection;

	SelectionOptionsKit		geometry_selection_options;
	SelectionOptionsKit		handles_selection_options;

	HighlightOptionsKit		hide_highlight_options;
	HighlightOptionsKit		partial_transparency_options;
	HighlightOptionsKit		highlight_options;
	SegmentKey				style_segment;
	SegmentKey				transparency_style_segment;
	UTF8					style_name;
	bool					something_is_highlighted;
	MaterialKitArray		handles_colors;
	RGBAColor				highlight_color;
	Key						highlighted_handle;

	//state flags
	bool					are_handles_on;
	bool					remove_handles;
	Movement				move_geometry;
	Vector					movement_direction;
	Vector					base_vector;

	//handles appearance
	float					handles_length;		//Length of the handles. Expressed in percentage of screen [0 - 1]
	float					handles_radius;		//Radius of the handles. Expressed in percentage of screen [0 - 1]
	float					arc_offset;			//Distance between the end of the translation handle and the start of the translation arc. Expressed in percentage of handles_length [0 - 1].
	float					arc_angle;			//The angle formed by the arc handles. Must be greater than zero.
	size_t					arc_points;			//The number of points used to draw the arc handles.
	float					tip_size;			//The size of the tips at the end of the handles, specified as a percentage of the handles length

	//handle geometry
	CylinderKey				x;
	CylinderKey				y;
	CylinderKey				z;
	CylinderKey				arc_x;
	CylinderKey				arc_y;
	CylinderKey				arc_z;

	//trailing geometry
	bool					display_trailing_geometry;
	Point					trailing_line_start;
	Point					trailing_circle_start;
	Point					trailing_circle_center;
	Vector					trailing_rotation;
	MarkerKey				trail_marker;
	LineKey					trailing_line;
	CircularArcKey			trailing_circle;
	float					rotation_direction;

	bool					InputDown(size_t in_click_count, WindowKey const & in_window, Point const & in_location);
	bool					InputMove(KeyPath const & in_path, Point const & in_location);
	void					InputUp(Point const & in_location);

	void					InsertHandles();
	void					RemoveHandles();
	bool					HighlightHandles(WindowKey & in_window, Point const & in_location);

	//moves the geometry affected by handles into a new segment, and hides the original
	void					ReferenceGeometry(KeyPath const & in_path);

	//copies the accumulated transform from the reference geometry segment back into the original place.
	//removes the hide highlight from the original geometry
	void					CommitChanges();
};

}
#endif


