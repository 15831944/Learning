// Copyright (c) Tech Soft 3D, Inc.
//
// The information contained herein is confidential and proprietary to Tech Soft 3D, Inc.,
// and considered a trade secret as defined under civil and criminal statutes.
// Tech Soft 3D, Inc. shall pursue its civil and criminal remedies in the event of
// unauthorized use or misappropriation of its trade secrets.  Use of this information
// by anyone other than authorized employees of Tech Soft 3D, Inc. is granted only under
// a written non-disclosure agreement, expressly prescribing the scope and manner of such use.

#ifndef SPRK_H
#define SPRK_H

#include "hps.h"

#include <memory>

#ifdef _MSC_VER
#	pragma warning(disable: 4251) //Not an issue as long as debug and release libraries aren't mixed
#ifndef STATIC_APP
#	ifdef SPRK
#		define SPRK_API __declspec (dllexport)
#	else
#		define SPRK_API __declspec (dllimport)
#	endif
#endif
#else
#	include <stddef.h>
#	if defined(LINUX_SYSTEM) && defined(SPRK)
#		ifndef STATIC_APP
#			define SPRK_API __attribute__ ((visibility ("default")))
#		endif
#	endif
#endif

#ifndef SPRK_API
#	define SPRK_API 
#endif
namespace SPRKI
{
	class X;
};

namespace HPS
{

class Factory;
class Canvas;
class Layout;
class View;
class Model;
class Operator;
class OperatorControl;
class NavigationCubeControl;
class AxisTriadControl;
class Component;
class ComponentPath;
class ComponentHighlightEvent;
class Filter;
class Capture;
class CADModel;
class Metadata;




class SPRK_API Sprocket : public HPS::Object
{
public:
	Sprocket() {}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
	Sprocket(HPS::Sprocket const & in_that) = default;
#endif
	Sprocket(Sprocket && in_that) : HPS::Object(std::move(in_that)) {}

	virtual ~Sprocket() {}
};

typedef std::shared_ptr<Operator>										OperatorPtr;

typedef std::vector<OperatorPtr, Allocator<OperatorPtr> >				OperatorPtrArray;

typedef std::vector<Canvas, Allocator<Canvas> >							CanvasArray;
typedef std::vector<Layout, Allocator<Layout> >							LayoutArray;
typedef std::vector<View, Allocator<View> >								ViewArray;
typedef std::vector<Model, Allocator<Model> >							ModelArray;

typedef std::vector<Component, Allocator<Component> >					ComponentArray;
typedef std::vector<Metadata, Allocator<Metadata> >						MetadataArray;
typedef std::vector<Filter, Allocator<Filter> >							FilterArray;
typedef std::vector<Capture, Allocator<Capture> >						CaptureArray;
typedef std::vector<CADModel, Allocator<CADModel> >						CADModelArray;
typedef std::vector<ReferenceKeyArray, Allocator<ReferenceKeyArray>>	ReferenceKeyArrayArray;
typedef std::vector<ComponentPath, Allocator<ComponentPath>>			ComponentPathArray;
typedef std::vector<HighlightOptionsKit, Allocator<HighlightOptionsKit>> HighlightOptionsKitArray;


/*! The Canvas class is one of the four core display classes used in a Visualize application. It is meant to be used as part of a display hierarchy along
	with the Layout, View, and Model objects. The Canvas is the top-level object of the hierarchy and roughly correlates to a window. The Canvas is backed
	by an HPS::WindowKey. */
class SPRK_API Canvas : public Sprocket
{
public:
	Canvas();
	Canvas(Canvas const & that);
	~Canvas();

	HPS::Type				ObjectType() const { return HPS::Type::Canvas; }

	/*! Deletes this Canvas, and the Window that backs it. If the Canvas has Layouts attached, they are not destroyed. */
	void					Delete();

	Canvas &				operator=(Canvas const & in_that);
	void					Assign(Canvas const & in_that);
	
	bool					Equals(Canvas const & in_that) const;
	bool					operator!= (Canvas const & in_that) const;
	bool					operator== (Canvas const & in_that) const;

	/*! Get the name used when creating this Canvas. */
	UTF8					GetName() const;

	/*! Attaches a layout to this HPS::Canvas. There can only be one layout per HPS::Canvas. If a Layout is already
		attached to this Canvas, the new Layout will replace the old one.
		\param in_layout the Layout to attach to this HPS::Canvas. */
	void					AttachLayout(Layout const & in_layout);

	/*! Attaches a View to this HPS::Canvas using an implicit Layout that convers the whole window. If a Layout is already
		attached to this Canvas, the new Layout will replace the old one.
		\param in_view the View to attach to this HPS::Canvas. */
	void					AttachViewAsLayout(View const & in_view);

	/*! Detaches the Layout from this Canvas. */
	void					DetachLayout();

	/*! Returns the Layout that is currently attached to this Canvas. */
	Layout					GetAttachedLayout() const;

	/*! Returns the IncludeKey that attaches the current layout to this Canvas. */
	IncludeKey				GetAttachedLayoutIncludeLink() const;

	/*! Views can overlap. This function returns a reference to the frontmost View. */
	View					GetFrontView() const;

	WindowKey const			GetWindowKey() const;

	/*! Each of the four display hierarchy objects (HPS::Canvas, HPS::Layout, HPS::View, HPS::Model) are backed by a 
		HPS::SegmentKey. This function returns the HPS::SegmentKey that is associated with this Canvas. */
	WindowKey				GetWindowKey();

	PortfolioKey const		GetPortfolioKey() const;

	/*! Returns the HPS::PortfolioKey associated with this Canvas' SegmentKey. */
	PortfolioKey			GetPortfolioKey();

	/*! Redraws the entire Canvas, including all of its child components using the default options. */
	void					Update() const;

	/*! Redraws the entire Canvas, including all of its child components using the default options, and also returns a notifier to facilitate tracking the update progress.
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier() const;
	
	/*! Redraws the entire Canvas, including all of its child components using the specified parameters.
	 *	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 *	\param in_time_limit the maximum amount of time, in seconds, to spend updating this HPS::Canvas */
	void					Update(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

	/*! Redraws the entire Canvas, including all of its child components using the specified parameters, and also returns an 
	 *	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 *	\param in_time_limit the maximum amount of time, in seconds, to spend updating this HPS::Canvas
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

	/*! Sets the maximum frame rate for this HPS::Canvas in number of frames per second.
		\param in_frame_rate		Frame rate in number of frames per second. Pass 0 to disable frame rate.
	*/
	void					SetFrameRate(float in_frame_rate=20);

	/*! Returns true if frame rate is active for this HPS::Canvas.
		\return	Returns frame rate in number of frames per second.
	*/
	float					GetFrameRate() const;
};

class SPRK_API Rendering
{
public:
	/*! \enum Mode
	Rendering Mode */
	enum class Mode
	{
		Gouraud,			//!< Rendering Mode
		GouraudWithLines,	//!< Rendering Mode
		Flat,				//!< Rendering Mode
		FlatWithLines,		//!< Rendering Mode
		Phong,				//!< Rendering Mode
		PhongWithLines,		//!< Rendering Mode
		HiddenLine,			//!< Rendering Mode
		FastHiddenLine,		//!< Rendering Mode
		Wireframe,			//!< Rendering Mode
		Default	= Phong		//!< Rendering Mode
	};

private:
	Rendering() {}
};

class SPRK_API SprocketControl : public Sprocket
{
public:
	HPS::Type		ObjectType() const {return HPS::Type::SprocketControl;}

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	SprocketControl(HPS::SprocketControl const & in_that) = default;
#endif

protected:
	SprocketControl() {}

	/*! The move constructor creates a SprocketControl by transferring the underlying impl of the rvalue reference to this SprocketControl
	 * 	thereby avoiding a copy and allocation.
	 * 	\param in_that An rvalue reference to a SprocketControl to take the impl from.*/
	SprocketControl(SprocketControl && in_that) : Sprocket(std::move(in_that)) {}

	/*! The move assignment operator transfers the underlying impl of the rvalue reference to this SprocketControl thereby avoiding a copy.
	* \param in_that An rvalue reference to an SprocketControl to take the impl from.
	* \return A reference to this SprocketControl. */
	SprocketControl & operator=(SprocketControl && in_that)
	{
		this->Object::operator=(std::move(in_that));
		return *this;
	}
};

/*! The View class is one of the four core display classes used in a Visualize application. It is meant to be used as part of a display hierarchy along
	with the Canvas, Layout, and Model objects. The View has a parent Layout and can contain a single Model. The View is meant to represent one
	particular camera's view of the scene. Like the other three core objects, the View is backed by a HPS::SegmentKey. */
class SPRK_API View : public Sprocket
{
public:
	View();
	View(View const & that);
	explicit View(SprocketControl const & in_ctrl);
	~View();

	HPS::Type				ObjectType() const { return HPS::Type::View; }

	/*! Deletes this View. If the View has a Model attached, the Model will not be destroyed. */
	void					Delete();

	View &					operator=(View const & in_that);
	void					Assign(View const & in_that);
	
	bool					Equals(View const & in_that) const;
	bool					operator!= (View const & in_that) const;
	bool					operator== (View const & in_that) const;

	/*! Get the name used when creating this View. */
	UTF8					GetName() const;


	/*! This method computes a camera to fit the extents of the scene.
	\param out_camera The camera computed by the fit world routine. */
	HPS::View &				ComputeFitWorldCamera(HPS::CameraKit & out_camera);

	/*! This method computes a camera to fit the extents of a segment.
	\param in_segment The segment used to calculate the new camera.
	\param out_camera The camera computed by the fit world routine. */
	HPS::View &				ComputeFitWorldCamera(SegmentKey const & in_segment, HPS::CameraKit & out_camera);


	/*! This method computes a camera to fit a bounding.
	\param in_bounding The bounding used to calculate the new camera.
	\param out_camera The camera computed by the fit world routine. */
	HPS::View &				ComputeFitWorldCamera(BoundingKit const & in_bounding, HPS::CameraKit & out_camera);

	/*! This method  computes a camera to fit the extents of a segment.
	\param in_segment The segment used to calculate the new camera.
	\param in_transform An optional transform to apply to the segment bounding used to compute the camera.  This can be used to ensure that the camera is looking at the
	geometry in the correct position once any modelling matrices are applied.
	\param out_camera The camera computed by the fit world routine. */
	HPS::View &				ComputeFitWorldCamera(SegmentKey const & in_segment, MatrixKit const & in_transform, HPS::CameraKit & out_camera);


	/*! This method computes a camera to fit a bounding.
	\param in_bounding The bounding used to calculate the new camera.
	\param in_transform An optional transform to apply to the bounding used to compute the camera.  This can be used to ensure that the camera is looking at the
	geometry in the correct position once any modelling matrices are applied.
	\param out_camera The camera computed by the fit world routine. */
	HPS::View &				ComputeFitWorldCamera(BoundingKit const & in_bounding, MatrixKit const & in_transform, HPS::CameraKit & out_camera);


	/*! This method automatically adjusts the camera to fit the extents of the scene. */
	HPS::View &				FitWorld();

	/*! This method automatically adjusts the camera to fit the extents of a segment.
	\param in_segment The segment used to calculate the new camera. */
	HPS::View &				FitWorld(SegmentKey const & in_segment);


	/*! This method automatically adjusts the camera to fit a bounding.
	\param in_bounding The bounding used to calculate the new camera. */
	HPS::View &				FitWorld(BoundingKit const & in_bounding);

	/*! This method automatically adjusts the camera to fit the extents of a segment. 
		\param in_segment The segment used to calculate the new camera.
		\param in_transform An optional transform to apply to the segment bounding used to compute the camera.  This can be used to ensure that the camera is looking at the
				geometry in the correct position once any modelling matrices are applied. */
	HPS::View &				FitWorld(SegmentKey const & in_segment, MatrixKit const & in_transform);


	/*! This method automatically adjusts the camera to fit a bounding.
	\param in_bounding The bounding used to calculate the new camera.
	\param in_transform An optional transform to apply to the bounding used to compute the camera.  This can be used to ensure that the camera is looking at the
			geometry in the correct position once any modelling matrices are applied. */
	HPS::View &				FitWorld(BoundingKit const & in_bounding, MatrixKit const & in_transform);

	/*! Attaches a Model to this View. Only a single Model can be attached to a View at one time. If you attach a Model
		to a View which already has an attached Model, the new Model will replace the old one.
		\param in_model the Model to attach to this View
	*/
	void					AttachModel(Model const & in_model);

	/*! Detaches the Model from this View */
	void					DetachModel();

	/*! Returns the Model that is attached to this View */
	Model					GetAttachedModel() const;

	/*! Returns the IncludeKey associated with this View. The include key links the View with the Model. */
	IncludeKey				GetAttachedModelIncludeLink() const;

	/*! Sets the rendering mode for this View. A rendering modes enable things like "hidden line" rendering or a particular 
		lighting interpolation algorithm.
		\sa Programming Guide <a href="../../prog_guide/core.html#view">section 1.1.3</a>. */
	void					SetRenderingMode(Rendering::Mode in_mode);

	/*! Gets the rendering mode for this View. A rendering modes enable things like "hidden line" rendering or a particular 
		lighting interpolation algorithm. */
	Rendering::Mode			GetRenderingMode() const;

	/*! Gets an array of all operators currently active on this View. */
	OperatorPtrArray		GetOperators() const;

	/*! Makes a set of operators active on this View.
		\param in_operators the set of operators to be made active on this View */
	void					SetOperators(OperatorPtrArray & in_operators);

	/*! Makes an array of operators active on this View.
		\param in_count the number of operators in the in_operators array
		\param in_operators the array of operators to be made active on this View */
	void					SetOperators(size_t in_count, OperatorPtr in_operators []);

	/*! Makes an operator active on this View.  Assumes control of the memory of the operator.
		\param in_operator the operator to be made active on this View. */
	void					SetOperator(OperatorPtr const & in_operator);

	/*! Makes an operator active on this View.  Assumes control of the memory of the operator.
		\param in_operator the operator to be made active on this View. */
	void					SetOperator(Operator * in_operator);

	/*! Gets the OperatorControl that is associated with this View. */
	OperatorControl			GetOperatorControl();
	OperatorControl const	GetOperatorControl() const;

	/*! Gets the NavigationCubeControl that is associated with this View. */
	NavigationCubeControl	GetNavigationCubeControl();
	NavigationCubeControl const	GetNavigationCubeControl() const;

	/*! Gets the AxisTriadControl that is associated with this View. */
	AxisTriadControl		GetAxisTriadControl();
	AxisTriadControl const	GetAxisTriadControl() const;

	/*! Each of the four display hierarchy objects (HPS::Canvas, HPS::Layout, HPS::View, HPS::Model) are backed by a 
		HPS::SegmentKey. This function returns the HPS::SegmentKey that is associated with this View. */
	SegmentKey				GetSegmentKey();
	SegmentKey const		GetSegmentKey() const;

	/*! Returns the model override segment. This segment can be used to change attributes affecting the whole model, without
		affecting the static tree set at the model level. */
	SegmentKey				GetModelOverrideSegmentKey();
	SegmentKey const		GetModelOverrideSegmentKey() const;


	PortfolioKey const		GetPortfolioKey() const;

	/*! Returns the HPS::PortfolioKey associated with this View. */
	PortfolioKey			GetPortfolioKey();

	/*! Turns simple shadows on or off, automatically calculating shadow plane and light direction 
		\param in_state controls whether simple shadows are on or off
		\param in_percent_offset distance to add between the model's bounding box and the position of the shadow plane
				If not specified, it is set to 5%. */
	void					SetSimpleShadow(bool in_state, float in_percent_offset = 5.0f);

	/*! Returns the status of the simple shadows */
	bool					GetSimpleShadow();

	/*! Turns simple reflection on or off, automatically calculating reflection plane and light direction 
	\param in_state controls whether simple reflection is on or off
	\param in_percent_offset distance to add between the model's bounding box and the position of the reflection plane
			If not specified, it is set to 5%. */
	void					SetSimpleReflection(bool in_state, float in_percent_offset = 5.0f);

	/*! Returns the status of the simple reflection */
	bool					GetSimpleReflection();

	/*! Redraws the View and all Layouts it appears in, including all of its child components. */
	void					Update() const;

	/*! Redraws the View and all Layouts it appears in, including all of its child components using the default options, and also returns a notifier to facilitate tracking the update progress.
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier() const;

	/*! Redraws the View and all Layouts it appears in, including all of its child components using the specified parameters.
	 *	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 *	\param in_time_limit the maximum amount of time, in seconds, to spend updating. */
	void					Update(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

	/*! Redraws the View and all Layouts it appears in, including all of its child components using the specified, and also returns a notifier to facilitate tracking the update progress.
	 *	\return HPS::UpdateNotifier to facilitate tracking the update progress.
	 * 	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 * 	\param in_time_limit the maximum amount of time, in seconds, to spend updating this HPS::Canvas
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

	/*! Smoothly moves the camera from the current position to the one specified by the user.
		\param in_camera_kit a camera kit describing the destination camera
		\param in_duration the duration of the animation, specified in seconds. Defaults to 0.5 seconds. */
	void					SmoothTransition(HPS::CameraKit const & in_camera_kit, HPS::Time in_duration = 0.5);

	static UTF8				GetHideStyleName();
	static UTF8				GetShowStyleName();

	/*! Gets an array of Keys associated with the Axis Triad and Navigation Cube for this View */
	KeyArray const			GetNavigationKeys();

	/*! Returns an array containing the Layouts this View is included in. */
	LayoutArray				GetOwningLayouts();
	LayoutArray	const		GetOwningLayouts() const;
};

/*! The Model class is one of the four core display classes used in a Visualize application. It is meant to be used as part of a display hierarchy along
	with the Canvas, Layout, and View objects. The parent of the Model is the View. The Model is meant to represent a segment which contains a geometric
	model. Like the other three core objects, the Model is backed by a HPS::SegmentKey. */
class SPRK_API Model : public Sprocket
{
public:
	Model();
	Model(Model const & that);
	~Model();

	HPS::Type				ObjectType() const { return HPS::Type::Model; }

	/*! Deletes this Model. */
	void					Delete();

	Model &					operator=(Model const & in_that);
	void					Assign(Model const & in_that);
	
	bool					Equals(Model const & in_that) const;
	bool					operator!= (Model const & in_that) const;
	bool					operator== (Model const & in_that) const;

	/*! Get the name used when creating this Model. */
	UTF8					GetName() const;

	/*! Each of the four display hierarchy objects (HPS::Canvas, HPS::Layout, HPS::View, HPS::Model) are backed by a 
	HPS::SegmentKey. This function returns the HPS::SegmentKey that is associated with this HPS::Model. */
	SegmentKey				GetSegmentKey();
	SegmentKey const		GetSegmentKey() const;

	/*! Returns the HPS::LibraryKey associated with this HPS::Model. */
	SegmentKey				GetLibraryKey();
	SegmentKey const		GetLibraryKey() const;

	/*! Returns the HPS::PortfolioKey associated with this Model's HPS::SegmentKey. */
	PortfolioKey			GetPortfolioKey();
	PortfolioKey const		GetPortfolioKey() const;

	/*! Returns an HPS::Vector pointing in the direction of the top of this Model. */
	HPS::Vector				GetUpVector();
	HPS::Vector const		GetUpVector() const;

	/*! Returns an HPS::Vector pointing in the direction of the front of this Model. */
	HPS::Vector				GetFrontVector();
	HPS::Vector const		GetFrontVector() const;

	/*! Returns an array containing the Views this Model is included in. */
	HPS::ViewArray			GetOwningViews();
	HPS::ViewArray const	GetOwningViews() const;

	/*! Sets the Model orientation
	\param in_up_vector the vector describing the direction of the top of this Model.
	\param in_front_vector the vector pointing in the direction of the front of this Model.*/
	void					SetOrientation(HPS::Vector & in_up_vector, HPS::Vector & in_front_vector);

	/*! Returns the vectors describing the up and front directions of this Model. */
	bool					ShowOrientation(HPS::Vector & out_up_vector, HPS::Vector & out_front_vector);

	/*! Redraws the Model and all Views it appears in, including all of its child components. */
	void					Update() const;

	/*! Redraws the Model and all Views it appears in, including all of its child components using the default options, and also returns a notifier to facilitate tracking the update progress.
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier() const;

	/*! Redraws the Model and all Views it appears in, including all of its child components using the specified parameters.
	 *	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 *	\param in_time_limit the maximum amount of time, in seconds, to spend updating. */
	void					Update(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

	/*! Redraws the Model and all Views it appears in, including all of its child components using the specified, and also returns a notifier to facilitate tracking the update progress.
	 *	\return HPS::UpdateNotifier to facilitate tracking the update progress.
	 * 	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 * 	\param in_time_limit the maximum amount of time, in seconds, to spend updating this HPS::Canvas
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

};

/*! The Layout class is one of the four core display classes used in a Visualize application. It is meant to be used as part of a display hierarchy along
	with the Canvas, View, and Model objects. The parent of the Layout is the Canvas, and a Layout can have multiple Views. The Layout is meant to describe
	the way the Views are presented in the Canvas. Like the other three core  objects, the Layout is backed by a HPS::SegmentKey. */
class SPRK_API Layout : public Sprocket
{
public:
	Layout();
	Layout(Layout const & that);
	~Layout();

	HPS::Type				ObjectType() const { return HPS::Type::Layout; }

	/*! Deletes this Layout. If the Layout has Views attached, they are not destroyed. */
	void					Delete();

	Layout &				operator=(Layout const & in_that);
	void					Assign(Layout const & in_that);

	bool					Equals(Layout const & in_that) const;
	bool					operator!= (Layout const & in_that) const;
	bool					operator== (Layout const & in_that) const;

	/*! Get the name used when creating this Layout. */
	UTF8					GetName() const;

	/*! Returns the number of Layers currently associated with this Layout */
	size_t					GetLayerCount() const;

	/*! Attached a View to this Layout, and places it to the front of the layer list 
	 * \param in_view The View to be attached to this Layer.
	 * \param in_position A Rectangle describing the position this View occupies with respect to the window. */
	void					AttachViewFront(View const & in_view, HPS::Rectangle const & in_position = HPS::Rectangle(-1.0f, 1.0f, -1.0f, 1.0f));
	/*! Attached a View to this Layout, and places it to the back of the layer list 
	 * \param in_view The View to be attached to this Layer.
	 * \param in_position A Rectangle describing the position this View occupies with respect to the window. */
	void					AttachViewBack(View const & in_view, HPS::Rectangle const & in_position = HPS::Rectangle(-1.0f, 1.0f, -1.0f, 1.0f));

	/*! Detaches this Layer from this Layout
	 * \param in_layer The Layer to detach. */
	void					DetachLayer(size_t in_layer);

	/*! Detaches this View from this Layout
	 * \param in_view The View to detach. */
	void					DetachView(View const & in_view);	// everywhere it occurs

	/*! Brings a Layer to the front of the list, causing to be drawn on top of the other layers. 
	* \param in_layer The layer to be moved to the front*/
	void					BringToFront(size_t in_layer);

	/*! Reassigns a Layer to a different View.
	 * \param in_layer The layer to be reassigned.
	 * \param in_view The View in_layer will be assigned to. */
	void					ReassignLayer(size_t in_layer, HPS::View in_view);

	/*! Changes the position of layer with respect to its window.
	 * \param in_layer The layer to be repositioned.
	 * \param in_position The new position for the layer. */
	void					RepositionLayer(size_t in_layer, HPS::Rectangle const & in_position);

	/*! Returns the View assigned to the layer shown on top of the window */
	View					GetFrontView() const;

	/*! Returns the View assigned to the specified Layer 
	 * \param in_layer The Layer of interest.
	 * \return The View associated to in_layer. */
	View					GetAttachedView(size_t in_layer = 0) const;

	/*! Returns the position assigned to the specified Layer
	 * \param in_layer The layer of interest.
	 * \return The position of in_layer. */
	Rectangle				GetPosition(size_t in_layer) const;

	/*! Returns the IncludeKey associated with this Layout. The include key links the Layer segment key with the View. */
	IncludeKey				GetAttachedViewIncludeLink(size_t in_layer) const;

	/*! Returns the backing SegmentKey associated with a given layer. The owner of this key is the layout segment key. */
	SegmentKey				GetLayerSegmentKey(size_t in_layer) const;

	/*! Returns the position within the layer list of the top layer at a specified position 
	 * \param in_point A point in Window coordinates. 
	 * \return The position within the layer list of the top layer at position in_point. */
	size_t					GetLayerAt(HPS::WindowPoint const & in_point) const;

	/*! Returns whether a layer is present at the specified position.
	 * \param in_point A point in Window coordinates. 
	 * \return <span class='code'>true</span> if a layer is present at in_point, <span class='code'>false</span> otherwise. */
	bool					ShowLayerAt(HPS::WindowPoint const & in_point) const;

	/*! Each of the four display hierarchy objects (HPS::Canvas, HPS::Layout, HPS::View, HPS::Model) are backed by a 
	HPS::SegmentKey. This function returns the HPS::SegmentKey that is associated with this HPS::Layout. */
	SegmentKey				GetSegmentKey();
	SegmentKey const		GetSegmentKey() const;

	/*! Returns an array containing the Canvas this Layout is included in. */
	CanvasArray				GetOwningCanvases();
	CanvasArray const		GetOwningCanvases() const;

	/*! Redraws the Layout and all Canvases it appears in, including all of its child components. */
	void					Update() const;

	/*! Redraws the Layout and all Canvases it appears in, including all of its child components using the default options, and also returns a notifier to facilitate tracking the update progress.
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier() const;

	/*! Redraws the Layout and all Canvases it appears in, including all of its child components using the specified parameters.
	 *	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 *	\param in_time_limit the maximum amount of time, in seconds, to spend updating. */
	void					Update(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

	/*! Redraws the Layout and all Canvases it appears in, including all of its child components using the specified, and also returns a notifier to facilitate tracking the update progress.
	 *	\return HPS::UpdateNotifier to facilitate tracking the update progress.
	 * 	\param in_type one of the enum values specified by HPS::Window::UpdateType
	 * 	\param in_time_limit the maximum amount of time, in seconds, to spend updating this HPS::Canvas
	 * \return An UpdateNotifier object that can be used to find out information about the update. */
	UpdateNotifier			UpdateWithNotifier(HPS::Window::UpdateType in_type, HPS::Time in_time_limit = -1.0) const;

};

class SPRK_API CanvasDeletedEvent : public Event
{
public:
	/*! The default constructor creates an empty CanvasDeletedEvent object. */
	CanvasDeletedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	CanvasDeletedEvent(Canvas const & in_canvas)
		: Event(), canvas(in_canvas)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a CanvasDeletedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	CanvasDeletedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<CanvasDeletedEvent>())
		{
			auto that = static_cast<CanvasDeletedEvent const &>(in_event);
			canvas = that.canvas;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~CanvasDeletedEvent();

	/*! Allocates and returns a copy of this CanvasDeletedEvent.
	 * 	\return A copy of this CanvasDeletedEvent. */
	Event * Clone() const
	{
		CanvasDeletedEvent * new_event = new CanvasDeletedEvent(*this);
		return new_event;
	}

	Canvas canvas;
};

class SPRK_API LayoutDetachedEvent : public Event
{
public:
	/*! The default constructor creates an empty LayoutDetachedEvent object. */
	LayoutDetachedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	LayoutDetachedEvent(Canvas const & in_canvas, Layout const & in_layout)
		: Event(), canvas(in_canvas), layout(in_layout)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a LayoutDetachedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	LayoutDetachedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<LayoutDetachedEvent>())
		{
			auto that = static_cast<LayoutDetachedEvent const &>(in_event);
			canvas = that.canvas;
			layout = that.layout;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~LayoutDetachedEvent();

	/*! Allocates and returns a copy of this LayoutDetachedEvent.
	 * 	\return A copy of this LayoutDetachedEvent. */
	Event * Clone() const
	{
		LayoutDetachedEvent * new_event = new LayoutDetachedEvent(*this);
		return new_event;
	}

	Canvas canvas;
	Layout layout;
};

class SPRK_API LayoutDeletedEvent : public Event
{
public:
	/*! The default constructor creates an empty LayoutDeletedEvent object. */
	LayoutDeletedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	LayoutDeletedEvent(Layout const & in_layout)
		: Event(), layout(in_layout)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a LayoutDeletedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	LayoutDeletedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<LayoutDeletedEvent>())
		{
			auto that = static_cast<LayoutDeletedEvent const &>(in_event);
			layout = that.layout;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~LayoutDeletedEvent();

	/*! Allocates and returns a copy of this LayoutDeletedEvent.
	 * 	\return A copy of this LayoutDeletedEvent. */
	Event * Clone() const
	{
		LayoutDeletedEvent * new_event = new LayoutDeletedEvent(*this);
		return new_event;
	}

	Layout layout;
};

class SPRK_API ViewDetachedEvent : public Event
{
public:
	enum class Action
	{
		None = 0,
		SpecificLayer,
		AllLayers,
	};

	/*! The default constructor creates an empty ViewDetachedEvent object. */
	ViewDetachedEvent() : Event(), action(Action::None), layer(0)
	{
		channel = GetClassID();
		consumable = false;
	}

	ViewDetachedEvent(Layout const & in_layout, size_t in_layer, View const & in_view)
		: Event(), layout(in_layout), action(Action::SpecificLayer), layer(in_layer), view(in_view)
	{
		channel = GetClassID();
		consumable = false;
	}

	ViewDetachedEvent(Layout const & in_layout, View const & in_view)
		: Event(), layout(in_layout), action(Action::AllLayers), layer(std::numeric_limits<size_t>::max()), view(in_view)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a ViewDetachedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	ViewDetachedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<ViewDetachedEvent>())
		{
			auto that = static_cast<ViewDetachedEvent const &>(in_event);
			layout = that.layout;
			action = that.action;
			layer = that.layer;
			view = that.view;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~ViewDetachedEvent();

	/*! Allocates and returns a copy of this ViewDetachedEvent.
	 * 	\return A copy of this ViewDetachedEvent. */
	Event * Clone() const
	{
		ViewDetachedEvent * new_event = new ViewDetachedEvent(*this);
		return new_event;
	}

	Layout layout;
	Action action;
	size_t layer;
	View view;
};

class SPRK_API ViewDeletedEvent : public Event
{
public:
	/*! The default constructor creates an empty ViewDeletedEvent object. */
	ViewDeletedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	ViewDeletedEvent(View const & in_view)
		: Event(), view(in_view)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a ViewDeletedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	ViewDeletedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<ViewDeletedEvent>())
		{
			auto that = static_cast<ViewDeletedEvent const &>(in_event);
			view = that.view;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~ViewDeletedEvent();

	/*! Allocates and returns a copy of this ViewDeletedEvent.
	 * 	\return A copy of this ViewDeletedEvent. */
	Event * Clone() const
	{
		ViewDeletedEvent * new_event = new ViewDeletedEvent(*this);
		return new_event;
	}

	View view;
};

class SPRK_API ModelDetachedEvent : public Event
{
public:
	/*! The default constructor creates an empty ModelDetachedEvent object. */
	ModelDetachedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	ModelDetachedEvent(View const & in_view, Model const & in_model)
		: Event(), view(in_view), model(in_model)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a ModelDetachedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	ModelDetachedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<ModelDetachedEvent>())
		{
			auto that = static_cast<ModelDetachedEvent const &>(in_event);
			view = that.view;
			model = that.model;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~ModelDetachedEvent();

	/*! Allocates and returns a copy of this ModelDetachedEvent.
	 * 	\return A copy of this ModelDetachedEvent. */
	Event * Clone() const
	{
		ModelDetachedEvent * new_event = new ModelDetachedEvent(*this);
		return new_event;
	}

	View view;
	Model model;
};

class SPRK_API ModelDeletedEvent : public Event
{
public:
	/*! The default constructor creates an empty ModelDeletedEvent object. */
	ModelDeletedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	ModelDeletedEvent(Model const & in_model)
		: Event(), model(in_model)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a ModelDeletedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	ModelDeletedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<ModelDeletedEvent>())
		{
			auto that = static_cast<ModelDeletedEvent const &>(in_event);
			model = that.model;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~ModelDeletedEvent();

	/*! Allocates and returns a copy of this ModelDeletedEvent.
	 * 	\return A copy of this ModelDeletedEvent. */
	Event * Clone() const
	{
		ModelDeletedEvent * new_event = new ModelDeletedEvent(*this);
		return new_event;
	}

	Model model;
};

class SPRK_API CameraChangedEvent : public Event
{
public:
	/*! The default constructor creates an empty CameraChangedEvent object. */
	CameraChangedEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	CameraChangedEvent(View const & in_view)
		: Event(), view(in_view)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a CameraChangedEvent object.
	 * 	\param in_event The Event Object to be converted. */
	CameraChangedEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<CameraChangedEvent>())
		{
			auto that = static_cast<CameraChangedEvent const &>(in_event);
			view = that.view;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~CameraChangedEvent();

	virtual bool Drop(Event const * in_that_event) const
	{	
		CameraChangedEvent const * that_event = static_cast<CameraChangedEvent const *>(in_that_event);

		if (view == that_event->view)
			return true;

		return false;
	}

	virtual intptr_t Freshen() const 
	{ 
		return GetClassID(); 
	}

	/*! Allocates and returns a copy of this CameraChangedEvent.
	 * 	\return A copy of this CameraChangedEvent. */
	Event * Clone() const
	{
		CameraChangedEvent * new_event = new CameraChangedEvent(*this);
		return new_event;
	}

	View view;
};

/*! The SprocketPath class is a utility class which can be used to automatically obtain a KeyPath
 * from the Model to the Canvas. */
class SPRK_API SprocketPath : public Sprocket
{
public:
	SprocketPath();
	SprocketPath(Model const & in_model, View const & in_view, Layout const & in_layout, Canvas const & in_canvas);
	SprocketPath(Canvas const & in_canvas, Layout const & in_layout, View const & in_view, Model const & in_model);
	SprocketPath(Canvas const & in_canvas, size_t in_layer = 0);
	SprocketPath(SprocketPath const & that);
	~SprocketPath();

	HPS::Type				ObjectType() const { return HPS::Type::SprocketPath; }

	SprocketPath &			operator=(SprocketPath const & in_that);
	void					Set(SprocketPath const & in_that);

	bool					Equals(SprocketPath const & in_that) const;
	bool					operator!= (SprocketPath const & in_that) const;
	bool					operator== (SprocketPath const & in_that) const;

	/*! Returns the Canvas associated with this SprocketPath */
	Canvas					GetCanvas() const;

	/*! Returns the Layout associated with this SprocketPath */
	Layout					GetLayout() const;

	/*! Returns the View associated with this SprocketPath */
	View					GetView() const;

	/*! Returns the Model associated with this SprocketPath */
	Model					GetModel() const;

	/*! Creates a key path from the model segment up to the canvas window key,
		including all segments and include keys along the way.
		This can be used to facilitate highlighting or gathering of net attributes.
		\return A key path from the model segment to the canvas window key. */
	KeyPath					GetKeyPath() const;
};


/*! The Operator class is the base class for classes which handle under input and operate on the scene.
 * This class handles GUI events such as key presses, mouse movement and clicks, and touches */
class SPRK_API Operator : public Sprocket
{
public:
	Operator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	Operator(Operator const & in_that);

	virtual ~Operator();

	HPS::Type				ObjectType() const { return HPS::Type::Operator; }

	virtual Operator &		operator= (Operator const & in_that);
	
	/*! Assigns the parameter in_that to this operator, resulting in a clone. */
	virtual void			Assign(Operator const & in_that);

	/*! Tests this operator for equality against the parameter <span class=code>in_that</span>. Same as Operator::operator== */
	virtual bool			Equals(Operator const & in_that) const;
	
	/*! Tests this operator for non-equality against the parameter <span class=code>in_that</span>. */
	virtual bool			operator!= (Operator const & in_that) const;
	
	/*! Tests this operator for equality against the parameter <span class=code>in_that</span>. Same as Operator::Equals. */
	virtual bool			operator== (Operator const & in_that) const;

	/*! Detaches the Operator from the View it was previously attached to. */
	void					DetachView();

	/*! Returns the View the Operator is currently attached to. */
	View					GetAttachedView() const;

	/*! Returns the name of the Operator. */
	virtual UTF8			GetName() const	{ return "Operator"; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse wheel was scrolled.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseWheel(MouseState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse entered the window.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseEnter(MouseState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse left the window.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseLeave(MouseState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchUp(TouchState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const  & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a KeyboardEvent that signals a key was pressed.
	 * \param in_state A KeyboardState object describing the current keyboard state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyDown(KeyboardState const & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a KeyboardEvent that signals a key was released.
	 * \param in_state A KeyboardState object describing the current keyboard state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnKeyUp(KeyboardState const & in_state) { HPS_UNREFERENCED(in_state); return false; }

	/*! This function is called whenever HPS receives a TextInputEvent that signals text was entered.
	 * \param in_text The text entered.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTextInput(HPS::UTF8 const & in_text) { HPS_UNREFERENCED(in_text); return false; }
	
	/*! This function is called whenever HPS receives a TimerTickEvent. 
	 * \param in_event A TimerTickEvent object describing the current event state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTimerTick(HPS::TimerTickEvent const & in_event) { HPS_UNREFERENCED(in_event); return false; }

	/*! This function is called whenever a view is attached to this operator. */
	virtual void			OnViewAttached() { return; }
	
	/*! This function is called whenever a view is detached from this operator. */
	virtual void			OnViewDetached() { return; }
	
	/*! This function is called whenever a model is attached to the view that is attached to this operator.
		If no view is attached to this operator, this function will not be called. */
	virtual void			OnModelAttached() { return; }
	
	/*! This function is called whenever a model is detached from the view that is attached to this operator.
		If no view is attached to this operator, this function will not be called. */
	virtual void			OnModelDetached() { return; }

	/*! Sets the mouse trigger for this operator */
	void					SetMouseTrigger(MouseButtons in_buttons) { mouse_trigger = in_buttons; }
	
	/*! Gets the mouse trigger associated with this operator.
	*/
	MouseButtons			GetMouseTrigger() const { return mouse_trigger; }

	/*! Sets the modifier trigger associated with this operator. 
	* \param in_modifiers The modifier keys that are used to trigger this operator.
	*/
	void					SetModifierTrigger(ModifierKeys in_modifiers) { modifier_trigger = in_modifiers; }
	
	/*! Gets the modifier trigger associated with this operator. */
	ModifierKeys			GetModifierTrigger() const { return modifier_trigger; }

	/*! Returns <span class=code>true</span> if the MouseButtons and ModifierKeys are active. */
	virtual bool			IsMouseTriggered(MouseState const  & in_state) { return in_state.HasAll(mouse_trigger, modifier_trigger); }

	enum class Priority
	{
		Low,
		Default,
		High
	};

protected:

	HPS::MouseButtons		mouse_trigger;
	HPS::ModifierKeys		modifier_trigger;
};

/*! The OperatorControl class is a smart pointer that is tied to a database object.
*	This control allows you to add, remove, or otherwise manipulate operators associated with a view. */
class SPRK_API OperatorControl : public SprocketControl
{
public:
	/*! Initializes a control tied to the view in_view. */
	explicit OperatorControl(View const & in_view);

	/*! Initializes a control tied to the same object as in_that. */
	OperatorControl(OperatorControl const & in_that);

	/*! The move constructor creates a OperatorControl by transferring the underlying impl of the rvalue reference to this OperatorControl
	 * 	thereby avoiding a copy and allocation.
	 * 	\param in_that An rvalue reference to a OperatorControl to take the impl from.*/
	OperatorControl(OperatorControl && in_that);

	/*! The move assignment operator transfers the underlying impl of the rvalue reference to this OperatorControl thereby avoiding a copy.
	* \param in_that An rvalue reference to a OperatorControl to take the impl from.
	* \return A reference to this OperatorControl. */
	OperatorControl & operator=(OperatorControl && in_that);

	/*! Releases a reference to the database object this control is tied to. */
	~OperatorControl();

	HPS::Type				ObjectType() const {return HPS::Type::OperatorControl;};

	/*! Share the smart-pointer. */
	OperatorControl &		operator=(OperatorControl const & in_that);

	
	/*! Returns the number of operators in use here. */
	size_t					GetCount();

	/*! Returns the number of operators of the specified priority in use here.
	*	\param in_priority The priority of the operators to be counted. */
	size_t					GetCount(Operator::Priority in_priority);


	/*! Adds an operator to the top of the operator stack within its priority and assumes control of its memory.
	*	Existing operators in use are unmodified but operators on top get priority.
	*	\param in_operator The operator to push to the top of the operator stack.
	*	\param in_priority The priority of the operator.  All those with Low priority will be below all those with <span class='code'>Normal</span> priority which will, in turn, be below High priority operators.
	*	\return A reference to this object. */
	OperatorControl &		Push(OperatorPtr const & in_operator, Operator::Priority in_priority = Operator::Priority::Default);

	/*! Adds an operator to the top of the operator stack within its priority and assumes control of its memory.
	*	Existing operators in use are unmodified but operators on top get priority.
	*	\param in_operator The operator to push to the top of the operator stack.
	*	\param in_priority The priority of the operator.  All those with Low priority will be below all those with <span class='code'>Normal</span> priority which will, in turn, be below High priority operators.
	*	\return A reference to this object. */
	OperatorControl &		Push(Operator * in_operator, Operator::Priority in_priority = Operator::Priority::Default);

	/*! Removes the top operator from the operator stack of the specified priority and frees the memory associated with it.
	*	\param in_priority The priority of the operator to be removed.
	*	\return <span class='code'>true</span> if an operator was present, <span class='code'>false</span> otherwise. */
	bool					Pop(Operator::Priority in_priority = Operator::Priority::Default);

	/*! Removes the top operator from the <span class='code'>Normal</span> priority operator stack and passes a handle to it to the user.
	*	The user is responsible for freeing memory associated with the operator.
	*	\param out_operator A handle to the top operator, passed to the user.
	*	\return <span class='code'>true</span> if an operator was present, <span class='code'>false</span> otherwise. */
	bool					Pop(OperatorPtr & out_operator);

	/*! Removes the top operator from the operator stack of the specified priority and passes a handle to it to the user.
	*	The user is responsible for freeing memory associated with the operator.
	*	\param in_priority The priority of the operator to be removed.
	*	\param out_operator A handle to the top operator, passed to the user.
	*	\return <span class='code'>true</span> if an operator was present, <span class='code'>false</span> otherwise. */
	bool					Pop(Operator::Priority in_priority, OperatorPtr & out_operator);


	/*! Sets an operator as the only operator in use of the specified priority and assumes control of its memory.
	*	This replaces any existing operators of the same priority in use and frees the memory associated with them.
	*	\param in_operator An operator to use on this view.
	*	\param in_priority The priority of the operator.  All those with Low priority will be below all those with <span class='code'>Normal</span> priority which will, in turn, be below High priority operators.
	*	\return A reference to this object. */
	OperatorControl &		Set(OperatorPtr const & in_operator, Operator::Priority in_priority = Operator::Priority::Default);

	/*! Sets an operator as the only operator in use of the specified priority and assumes control of its memory.
	*	This replaces any existing operators in use and frees the memory associated with them.
	*	\param in_operator An operator to use on this view.
	*	\param in_priority The priority of the operator.  All those with Low priority will be below all those with <span class='code'>Normal</span> priority which will, in turn, be below High priority operators.
	*	\return A reference to this object. */
	OperatorControl &		Set(Operator * in_operator, Operator::Priority in_priority = Operator::Priority::Default);

	/*! Sets a collection of operators as the only operators in use of the specified priority and assumes control of their memory.
	*	This replaces any existing operators in use and frees the memory associated with them.
	*	\param in_operators An array of source operators to be used.
	*	\param in_priority The priority of the operators.  All those with Low priority will be below all those with <span class='code'>Normal</span> priority which will, in turn, be below High priority operators.
	*	\return A reference to this object. */
	OperatorControl &		Set(OperatorPtrArray & in_operators, Operator::Priority in_priority = Operator::Priority::Default);

	/*! Sets a collection of operators as the only operators in use  of the specified priority and assumes control of their memory.
	*	This replaces any existing operators in use and frees the memory associated with them.
	*	\param in_count The number of elements in the operator array.
	*	\param in_operators An array of source operators to be used.
	*	\param in_priority The priority of the operators.  All those with Low priority will be below all those with <span class='code'>Normal</span> priority which will, in turn, be below High priority operators.
	*	\return A reference to this object. */
	OperatorControl &		Set(size_t in_count, OperatorPtr in_operators [], Operator::Priority in_priority = Operator::Priority::Default);


	/*! Removes the top operator from the operator stack of the specified priority and frees the memory associated with it.
	*	\param in_priority The priority of the operator to be removed.
	*	\return A reference to this object. */
	OperatorControl &		UnsetTop(Operator::Priority in_priority = Operator::Priority::Default);

	/*! Removes all operators from the operator stack of the specified priority and frees the memory associated with them.
	*	\param in_priority The priority of the operator to be removed.
*	\return A reference to this object. */
	OperatorControl &		UnsetEverything(Operator::Priority in_priority);

	/*! Removes all operators from the operator stack of any priority and frees the memory associated with them.
	*	\return A reference to this object. */
	OperatorControl &		UnsetEverything();


	/*! Shows the top operator on the <span class='code'>Normal</span> priority operator stack.  The operator stack is unmodified.
	*	\param out_operator The operator at the top of the operator stack
	*	\return <span class='code'>true</span> if an operator was present, <span class='code'>false</span> otherwise. */
	bool					ShowTop(OperatorPtr & out_operator) const;

	/*! Shows the top operator on the operator stack of the specified priority.  The operator stack is unmodified.
	*	\param in_priority The priority of the operator to be shown.
	*	\param out_operator The operator at the top of the operator stack
	*	\return <span class='code'>true</span> if an operator was present, <span class='code'>false</span> otherwise. */
	bool					ShowTop(Operator::Priority in_priority, OperatorPtr & out_operator) const;
	
	/*! Shows all <span class='code'>Normal</span> priority operators in use on this segment.  The operator stack is unmodified.
	*	\param out_operators Handles to all <span class='code'>Normal</span> priority operators used by this View.
	*	\return <span class='code'>true</span> if any operator was present, <span class='code'>false</span> otherwise. */
	bool					Show(OperatorPtrArray & out_operators) const;

	/*! Shows all operators in use on this segment of the specified priority.  The operator stack is unmodified.
	*	\param in_priority The priority of the operators to be shown.
	*	\param out_operators Handles to all operators of the specified priority used by this View.
	*	\return <span class='code'>true</span> if any operator was present, <span class='code'>false</span> otherwise. */
	bool					Show(Operator::Priority in_priority, OperatorPtrArray & out_operators) const;

private:
	//! Private default constructor to prevent instantiation without a view.
	OperatorControl() {}
};

/*! The navigationCubeControl allows you to turn on or off the navigation cube, move it to a specific corner of the screen,
 *  change the material palette used to style it, or otherwise manipulate it. */
class SPRK_API NavigationCubeControl : public SprocketControl
{
public:

	enum class Location
	{
		TopRight,	//!< Position Navigation Cube on the top right corner
		TopLeft,	//!< Position Navigation Cube on the top left corner
		BottomRight,//!< Position Navigation Cube on the bottom right corner
		BottomLeft,	//!< Position Navigation Cube on the bottom left corner
		Custom,		//!< Position Navigation Cube at a custom location
	};

	/*! Initializes a control tied to the view in_view. */
	explicit NavigationCubeControl(View const & in_view);

	/*! Initializes a control tied to the same object as in_that. */
	NavigationCubeControl(NavigationCubeControl const & in_that);

	/*! The move constructor creates a NavigationCubeControl by transferring the underlying impl of the rvalue reference to this NavigationCubeControl
	 * 	thereby avoiding a copy and allocation.
	 * 	\param in_that An rvalue reference to a NavigationCubeControl to take the impl from.*/
	NavigationCubeControl(NavigationCubeControl && in_that);

	/*! The move assignment operator transfers the underlying impl of the rvalue reference to this NavigationCubeControl thereby avoiding a copy.
	* \param in_that An rvalue reference to a NavigationCubeControl to take the impl from.
	* \return A reference to this NavigationCubeControl. */
	NavigationCubeControl & operator=(NavigationCubeControl && in_that);

	/*! Releases a reference to the database object this control is tied to. */
	~NavigationCubeControl();

	HPS::Type				ObjectType() const {return HPS::Type::NavigationCubeControl;};

	/*! Share the smart-pointer. */
	NavigationCubeControl &	operator=(NavigationCubeControl const & in_that);

	/*! Sets the location of this navigation cube.
	 *	\param in_location the location of this navigation cube 
	 *	\param in_rectangle the position of the subwindow containing the navigation cube. Only considered if
		in_location is set to Custom.
	 *  \param in_offsets the pixel-space rectangle to offset to the given position.
			only used when in_location is Custom
	 *	\return A reference to this object. 
	 *	By default the navigation cube is set to the top right corner of the View*/
	NavigationCubeControl &	SetLocation(Location in_location, HPS::Rectangle const & in_rectangle = HPS::Rectangle(), HPS::IntRectangle in_offsets = HPS::IntRectangle::Zero());

	/*! Gets the location of the navigation cube.
	 *	\return The location of the navigation cube relative to the view. */
	Location				GetLocation() const;

	/*! Sets the position of the subwindow containing this navigation cube.
	 *	\param in_rectangle a rectangle describing the subwindow containing the navigation cube
	 *	\return A reference to this object. */
	NavigationCubeControl & SetPosition(HPS::Rectangle const & in_rectangle);

	/*! Gets the poition of the subwindow containing the navigation cube.
	 *	\return The position of the subwindow containing the navigation cube. */
	HPS::Rectangle			GetPosition() const;

	/*! Sets the position of the subwindow containing the navigation cube.
	*	\param in_offsets a rectangle describing the offsets being applied to the current position.
	*	\return A reference to this object. */
	NavigationCubeControl &		SetPositionOffsets(HPS::IntRectangle const & in_offsets);

	/*! Gets the poition of the subwindow containing the navigation cube.
	*	\return The position of the subwindow containing the navigation cube. */
	HPS::IntRectangle			GetPositionOffsets() const;

	/*! Sets the visibility of this navigation cube.
	 * \param in_visibility the visibility of this navigation cube
	 *	\return A reference to this object. 
	 * By default the navigation cube is not visible */
	NavigationCubeControl &	SetVisibility(bool in_visibility);

	/*! Gets the visibility of the navigation cube.
	 *	\return <span class='code'>true</span> if the navigation cube is visible, <span class='code'>false</span> otherwise. */
	bool					GetVisibility() const;

	/*! Sets the text visibility of this navigation cube.
	 * \param in_text_visibility the visibility of the text for this navigation cube
	 *	\return A reference to this object. */
	NavigationCubeControl &	SetTextVisibility(bool in_text_visibility);

	/*! Gets the text visibility of the navigation cube.
	 *	\return <span class='code'>true</span> if the text on the navigation cube is visible, <span class='code'>false</span> otherwise. */
	bool					GetTextVisibility() const;

	/*! Sets the material palette of this navigation cube.
	 *	The material palette supplied to this function should originate from a material kit array with 9 entries.
	 *	The entries in the material kit array are applied to the navigation cube as follows:
	 *	Entry 0		-	back face color
	 *	Entry 1		-	top face color
	 *	Entry 2		-	left face color
	 *	Entry 3		-	front face color
	 *	Entry 4		-	bottom face color
	 *	Entry 5		-	right face color
	 *	Entry 6		-	mouse-over highlight color
	 *	Entry 7		-	text color
	 *	Entry 8		-	line color
	 *	\param in_material_palette the name of the material palette to be used for this navigation cube
	 *	\return A reference to this object. */
	NavigationCubeControl &	SetMaterialPalette(char const * in_material_palette);

	/*! Gets the material palette used for this navigation cube.
	 *	\return The name of the material palette used for this navigation cube. */
	HPS::UTF8			GetMaterialPalette() const;

	/*! Sets the interactivity of this navigation cube.
	 *  When the navigation cube is interactive, dragging the triad will rotate the scene
	 *  and clicking on a face, edge or vertex of the cube will snap the camera to the appropriate plane.
	 *  Making the Navigation Cube interactive pushes the Navigation Cube operator on the current View's operator stack with High priority
	 * \param in_interactivity the interactivity of this navigation cube
	 * \return A reference to this object. 
	 * By default the navigation cube is not interactive */
	NavigationCubeControl &	SetInteractivity(bool in_interactivity);


	/*! Gets the interactivity of this navigation cube.
	 *	\return <span class='code'>true</span> if the navigation cube is interactive, <span class='code'>false</span> otherwise. */
	bool					GetInteractivity() const;

	/*! Sets the size of the subwindow containing the navigation cube, as a percentage of the main window.
	 * \param in_size The size of the subwindow containing the navigation cube, as a percentage of the main window. [0-1]
	 * \return A reference to this object. 
	 * By default the navigation cube subwindow is 20% of the main window */
	NavigationCubeControl &	SetSize(float in_size);

	/*! Gets the size of the subwindow containing the navigation cube, as a percentage of the main window.
	 *	\return the size of the subwindow containing the navigation cube, as a percentage of the main window. If the location is Custom, this will return -1.0f. */
	float					GetSize() const;

	/*! Gets the segment where this navigation cube resides.
	 *	\return The segment key associated with this navigation cube. */
	HPS::SegmentKey			GetSegmentKey() const;

private:
	//! Private default constructor to prevent instantiation without a view.
	NavigationCubeControl() {}

	//! Returns the navigation cube segment key.
	HPS::SegmentKey GetNavigationCubeSegment() const;
};

/*! The AxisTriadControl allows you to turn on or off the axis triad, move it to a specific corner of the screen,
 *  change the material palette used to style it, or otherwise manipulate it. */
class SPRK_API AxisTriadControl : public SprocketControl
{
public:

	enum class Location
	{
		TopRight,	//!< Position Axis Triad on the top right corner
		TopLeft,	//!< Position Axis Triad on the top left corner
		BottomRight,//!< Position Axis Triad on the bottom right corner
		BottomLeft,	//!< Position Axis Triad on the bottom left corner
		Custom,		//!< Position Axis Triad at a custom location
	};

	/*! Initializes a control tied to the view in_view. */
	explicit AxisTriadControl(View const & in_view);

	/*! Initializes a control tied to the same object as in_that. */
	AxisTriadControl(AxisTriadControl const & in_that);

	/*! The move constructor creates a AxisTriadControl by transferring the underlying impl of the rvalue reference to this AxisTriadControl
	 * 	thereby avoiding a copy and allocation.
	 * 	\param in_that An rvalue reference to a AxisTriadControl to take the impl from.*/
	AxisTriadControl(AxisTriadControl && in_that);

	/*! The move assignment operator transfers the underlying impl of the rvalue reference to this AxisTriadControl thereby avoiding a copy.
	* \param in_that An rvalue reference to a AxisTriadControl to take the impl from.
	* \return A reference to this AxisTriadControl. */
	AxisTriadControl & operator=(AxisTriadControl && in_that);

	/*! Releases a reference to the database object this control is tied to. */
	~AxisTriadControl();

	HPS::Type				ObjectType() const {return HPS::Type::AxisTriadControl;};

	/*! Share the smart-pointer. */
	AxisTriadControl &		operator=(AxisTriadControl const & in_that);

	/*! Sets the location of this axis triad.
	 *	\param in_location the location of this axis triad
	 *  \param in_position the position of the subwindow containing the axis triad.
			only used when in_location is Custom
	 *  \param in_offsets the pixel-space rectangle to offset to the given position.
			only used when in_location is Custom
	 *	\return A reference to this object. 
	 *  By default the axis triad is placed on the bottom left corner of the View */
	HPS::AxisTriadControl &	SetLocation(Location in_location, HPS::Rectangle const & in_position = HPS::Rectangle(), HPS::IntRectangle in_offsets = HPS::IntRectangle::Zero());

	/*! Gets the location of the axis triad.
	 *	\return The location of the axis triad relative to the view. */
	Location				GetLocation() const;

	/*! Sets the position of the subwindow containing the axis triad.
	 *	\param in_position a rectangle describing the subwindow containing the axis triad.
	 *	\return A reference to this object. */
	AxisTriadControl &		SetPosition(HPS::Rectangle const & in_position);

	/*! Gets the position of the subwindow containing the axis triad.
	 *	\return The position of the subwindow containing the axis triad. */
	HPS::Rectangle			GetPosition() const;

	/*! Sets the position of the subwindow containing this axis triad.
	*	\param in_offsets a rectangle describing the offsets being applied to the current position
	*	\return A reference to this object. */
	AxisTriadControl &		SetPositionOffsets(HPS::IntRectangle const & in_offsets);

	/*! Gets the poition of the subwindow containing the axis triad.
	*	\return The position of the subwindow containing the axis triad. */
	HPS::IntRectangle			GetPositionOffsets() const;

	/*! Sets the visibility of this axis triad.
	 * \param in_visibility the visibility of this axis triad
	 *	\return A reference to this object. 
	 *  By default the axis triad is not visible */
	HPS::AxisTriadControl &	SetVisibility(bool in_visibility);

	/*! Gets the visibility of the axis triad.
	 *	\return <span class='code'>true</span> if the axis triad is visible, <span class='code'>false</span> otherwise. */
	bool					GetVisibility() const;

	/*! Sets the text visibility of this axis triad.
	 * \param in_text_visibility the visibility of the text for this axis triad
	 *	\return A reference to this object. */
	HPS::AxisTriadControl &	SetTextVisibility(bool in_text_visibility);

	/*! Gets the text visibility of the axis triad.
	 *	\return <span class='code'>true</span> if the axis triad text is visible, <span class='code'>false</span> otherwise. */
	bool					GetTextVisibility() const;

	/*! Sets the material palette of this axis triad.
	 *	The material palette supplied to this function should originate from a material kit array with 3 entries.
	 *	The entries in the material kit array are applied to the navigation cube as follows:
	 *	Entry 0		-	x-axis color
	 *	Entry 1		-	y-axis color
	 *	Entry 2		-	z-axis color
	 *	\param in_material_palette the name of the material palette to be used for this axis triad
	 *	\return A reference to this object. */
	HPS::AxisTriadControl &	SetMaterialPalette(char const * in_material_palette);

	/*! Gets the material palette used for this axis triad.
	 *	\return The name of the material palette used for this axis triad. */
	HPS::UTF8			GetMaterialPalette() const;

	/*! Sets the interactivity of this axis triad.
	 *  When the axis triad is interactive, dragging the triad will rotate the scene.
	 *  Making the Axis Triad interactive pushes the Axis Triad operator on the current View's operator stack with High priority
	 * \param in_interactivity the interactivity of this axis triad
	 *	\return A reference to this object. 
	 * By default the axis triad is not interactive. */
	HPS::AxisTriadControl &	SetInteractivity(bool in_interactivity);

	/*! Gets the interactivity of this axis triad.
	 *	\return <span class='code'>true</span> if the axis triad is interactive, <span class='code'>false</span> otherwise. */
	bool					GetInteractivity() const;

	/*! Sets the size of the subwindow containing the axis triad, as a percentage of the main window.
	 * \param in_size The size of the subwindow containing the axis triad, as a percentage of the main window. [0-1]
	 * \return A reference to this object. 
	 * By default the axis triad subwindow is 20% of the main window */
	AxisTriadControl &		SetSize(float in_size);

	/*! Gets the size of the subwindow containing the axis triad, as a percentage of the main window.
	 *	\return the size of the subwindow containing the axis triad, as a percentage of the main window. If the location is Custom, this will return -1.0f. */
	float					GetSize() const;

	/*! Gets the segment where this axis triad resides.
	 *	\return The segment key associated with this axis triad. */
	HPS::SegmentKey			GetSegmentKey() const;

private:
	//! Private default constructor to prevent instantiation without a view.
	AxisTriadControl(){};

	//! Returns the axis triad segment key.
	HPS::SegmentKey			GetAxisTriadSegment() const;
};

/*! The Component class is a smart pointer.  It represents an arbitrary component of a structured file. */
class SPRK_API Component : public Sprocket
{
public:
	/*! \enum ComponentType
	 *	Enumerates the format-dependent types a Component can represent. */
	enum class ComponentType : uint32_t
	{
		None									= 0x00000000,
		GenericMask								= 0xfffff000,

		ExchangeComponentMask					= 0x00001000,		//!< Mask for any Exchange components.
		ExchangeModelFile						= 0x00001001,		//!< Represents an A3DAsmModelFile in Exchange.  This will be represented by an Exchange::CADModel.
		ExchangeProductOccurrence				= 0x00001002,		//!< Represents an A3DAsmProductOccurrence in Exchange.  This will be represented by an Exchange::Component.
		ExchangePartDefinition					= 0x00001003,		//!< Represents an A3DAsmPartDefinition in Exchange.  This will be represented by an Exchange::Component.

		ExchangeView							= 0x00001004,		//!< Represents an A3DMkpView in Exchange.  This will be represented by an Exchange::Capture component.
		ExchangeFilter							= 0x00001005,		//!< Represents an A3DMkpFilter in Exchange.  This will be represented by an Exchange::Filter component.

		ExchangeRepresentationItemMask			= 0x00003000,		//!< Mask for any Exchange representation items.
		ExchangeRIBRepModel						= 0x00003001,		//!< Represents an A3DRiBrepModel in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRICurve							= 0x00003002,		//!< Represents an A3DRiCurve in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRIDirection						= 0x00003003,		//!< Represents an A3DRiDirection in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRIPlane							= 0x00003004,		//!< Represents an A3DRiPlane in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRIPointSet						= 0x00003005,		//!< Represents an A3DRiPointSet in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRIPolyBRepModel					= 0x00003006,		//!< Represents an A3DRiPolyBrepModel in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRIPolyWire						= 0x00003007,		//!< Represents an A3DRiPolyWire in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRISet							= 0x00003008,		//!< Represents an A3DRiSet in Exchange.  This will be represented by an Exchange::Component.
		ExchangeRICoordinateSystem				= 0x00003009,		//!< Represents an A3DRiCoordinateSystem in Exchange.  This will be represented by an Exchange::Component.

		ExchangeTopologyMask					= 0x00005000,		//!< Mask for any Exchange topology items.
		ExchangeTopoBody						= 0x00005001,		//!< Represents an A3DTopoBody in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoConnex						= 0x00005002,		//!< Represents an A3DTopoConnex in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoShell						= 0x00005003,		//!< Represents an A3DTopoShell in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoFace						= 0x00005004,		//!< Represents an A3DTopoFace in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoLoop						= 0x00005005,		//!< Represents an A3DTopoLoop in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoCoEdge						= 0x00005006,		//!< Represents an A3DTopoCoEdge in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoEdge						= 0x00005007,		//!< Represents an A3DTopoEdge in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoVertex						= 0x00005008,		//!< Represents an A3DTopoVertex in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoSingleWireBody				= 0x00005009,		//!< Represents an A3DTopoSingleWireBody in Exchange.  This will be represented by an Exchange::Component.
		ExchangeTopoWireEdge					= 0x0000500a,		//!< Represents an A3DTopoWireEdge in Exchange.  This will be represented by an Exchange::Component.

		ExchangeDrawingMask						= 0x00009000,		//!< Mask for any Exchange drawing item.
		ExchangeDrawingModel					= 0x00009001,		//!< Represents an A3DDrawingModel in Exchange. This is the starting point of a drawing. It is found under a product occurrence. Generally there is only one Drawing Model per file, but it is possible to have more than one. This will be represented by an Exchange::Component.
		ExchangeDrawingView						= 0x00009002,		//!< Represents an A3DDrawingView in Exchange. A Drawing Sheet can contain views. Views are basically snapshot of the model from a particular angle. Each view has a name and a type, for example, front view, side view, section, etc. This will be represented by an Exchange::Component.
		ExchangeDrawingSheet					= 0x00009003,		//!< Represents an A3DDrawingSheet in Exchange. A DrawingModel contains one or more sheets. Each sheet represents a physical sheet of paper. Generally a sheet defines the frame of the drawing, contains the name of the paper format it reproduces, and contains a transform. One of the sheets is classified as the "active sheet" by the model, and this one will be what you see when you first load a drawing. There can only be one active sheet at the time, with all the inactive ones being hidden. Drawing sheets have metadata associated with them called 'IsDefaultSheet'. This is a BooleanMetadata. It will return true if the sheet is the active sheet, and false otherwise. This will be represented by an Exchange::Component.
		ExchangeBasicDrawingBlock				= 0x00009004,		//!< Represents an A3DDrawingBlockBasic in Exchange. Sheets and views contain blocks. Blocks can be of two types: basic or operator blocks. Operator blocks are blocks which reference a particular view. These two types are internally treated as the same thing. Blocks contain the actual representation items which will be tessellated and drawn on the screen. This will be represented by an Exchange::Component.
		ExchangeOperatorDrawingBlock			= 0x00009005,		//!< Represents an A3DDrawingBlockOperator in Exchange. Sheets and views contain blocks. Blocks can be of two types: basic or operator blocks. Operator blocks are blocks which reference a particular view. These two types are internally treated as the same thing. Blocks contain the actual representation items which will be tessellated and drawn on the screen. This will be represented by an Exchange::Component.

		ExchangePMIMask							= 0x00011000,		//!< Mask for any Exchange PMI items.
		ExchangePMI								= ExchangePMIMask,	//!< Represents a generic A3DMkpMarkup in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIText							= 0x00011100,		//!< Represents an A3DMarkupText in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIRichText						= 0x00011200,		//!< Represents an A3DMarkupRichText in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIRoughness					= 0x00011300,		//!< Represents an A3DMarkupRoughness in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIGDT							= 0x00011400,		//!< Represents an A3DMarkupGDT in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIDatum						= 0x00011500,		//!< Represents an A3DMarkupDatum in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMILineWelding					= 0x00011600,		//!< Represents an A3DMarkupLineWelding in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMISpotWelding					= 0x00011700,		//!< Represents an A3DMarkupSpotWelding in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIDimension					= 0x00011800,		//!< Represents an A3DMarkupDimension in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIBalloon						= 0x00011900,		//!< Represents an A3DMarkupBalloon in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMICoordinate					= 0x00011a00,		//!< Represents an A3DMarkupCoordinate in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIFastener 					= 0x00011b00,		//!< Represents an A3DMarkupFastener in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMILocator						= 0x00011c00,		//!< Represents an A3DMarkupLocator in Exchange.  This will be represented by an Exchange::Component.
		ExchangePMIMeasurementPoint				= 0x00011d00,		//!< Represents an A3DMarkupMeasurementPoint in Exchange.  This will be represented by an Exchange::Component.


		ParasolidComponentMask					= 0x00020000,		//!< Mask for any Parasolid components.

		ParasolidModelFile						= 0x00020001,		//!< Abstract component representing the root of a Parasolid file. Does not map to any Parasolid element.
		ParasolidAssembly						= 0x00020002,		//!< Represents a PK_ASSEMBLY in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidInstance						= 0x00020003,		//!< Represents a PK_INSTANCE in Parasolid.  This will be represented by a Parasolid::Component.

		ParasolidTopologyMask					= 0x00060000,		//!< Mask for any Parasolid topological items.
		ParasolidTopoBody						= 0x00060001,		//!< Represents a PK_BODY in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoRegion						= 0x00060002,		//!< Represents a PK_REGION in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoShell						= 0x00060003,		//!< Represents a PK_SHELL in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoFace						= 0x00060004,		//!< Represents a PK_FACE in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoLoop						= 0x00060005,		//!< Represents a PK_LOOP in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoFin						= 0x00060006,		//!< Represents a PK_FIN in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoEdge						= 0x00060007,		//!< Represents a PK_EDGE in Parasolid.  This will be represented by a Parasolid::Component.
		ParasolidTopoVertex						= 0x00060008,		//!< Represents a PK_VERTEX in Parasolid.  This will be represented by a Parasolid::Component.


		DWGComponentMask						= 0x00100000,		//!< Mask for any DWG components.
		DWGModelFile							= 0x00100001,		//!< Represents an AcDbDatabase in RealDWG.  This will be represented by a DWG::CADModel.
		DWGLayout								= 0x00100002,		//!< Represents an AcDbLayout in RealDWG.
		DWGBlockTable							= 0x00100003,		//!< Represents an AcDbBlockTable in RealDWG.
		DWGBlockTableRecord						= 0x00100004,		//!< Represents an AcDbBlockTableRecord in RealDWG.
		DWGEntity								= 0x00100005,		//!< Represents an AcDbEntity in RealDWG.
		DWGLayerTable							= 0x00100006,		//!< Represents an AcDbLayerTable in RealDWG.
		DWGLayer								= 0x00100007,		//!< Represents an AcDbLayer in RealDWG.


		UserComponent							= 0x01000000,		//!< Represents a user created component
	};


	/*! The default constructor creates an uninitialized Component object.  The Type() function will return Type::None. */
	Component();

	/*! The copy constructor creates a Component object that shares the underlying smart-pointer of the source Component.
	 *	\param in_that The source Component to copy. */
	Component(Component const & in_that);

	/*! The move constructor creates a Component by transferring the underlying object of the rvalue reference to this Component.
	 * 	\param in_that An rvalue reference to a Component to take the underlying object from. */
	Component(Component && in_that);

	/*! Releases a reference to this Component.  The Component will not be deleted unless the final reference is destroyed. */
	virtual ~Component();

	HPS::Type				ObjectType() const { return HPS::Type::Component; }


	/*! Share the underlying smart-pointer of the Component source.
	 *	\param in_that The Component source of the assignment.
	 *	\return A reference to this Component. */
	Component &				operator=(Component const & in_that);

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this Component.
	 *	\param in_that An rvalue reference to a Component to take the underlying object from.
	 *	\return A reference to this Component. */
	Component &				operator=(Component && in_that);

	/*! Share the underlying smart-pointer of the Component source.
	 *	\param in_that The Component source of the assignment.
	 *	\return A reference to this Component. */
	virtual void			Assign(Component const & in_that);

	/*!	Check if the source Component points to the same underlying impl as this Component.
	 *	\param in_that The source Component to compare to this Component.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					Equals(Component const & in_that) const;

	/*!	Check if the source Component points to a different impl than this Component.
	 *	\param in_that The source Component to compare to this Component.
	 *	\return <span class='code'>true</span> if the objects reference different impls, <span class='code'>false</span> otherwise. */
	bool					operator!=(Component const & in_that) const;

	/*!	Check if the source Component points to the same underlying impl as this Component.
	 *	\param in_that The source Component to compare to this Component.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					operator==(Component const & in_that) const;


	/*! Gets the specific format-dependent type this Component represents.
	 *	\return The specific format-dependent type this Component represents. */
	ComponentType			GetComponentType() const;

	/*! This function indicates whether this Component has the given ComponentType mask.
	 *	\param in_mask The ComponentType mask to check against this Component.
	 *	\return <span class='code'>true</span> if this Component has the given ComponentType mask, <span class='code'>false</span> otherwise. */
	bool					HasComponentType(ComponentType in_mask) const;



	/*! Adds a key to be associated with this Component.  This will additionally ensure this key can be used to find this Component via CADModel::GetComponentFromKey on the owning CADModel.
	 *	In general, this will either be a SegmentKey or a GeometryKey (or a subclass thereof), but it may also be an IncludeKey, which is necessary for an instanced Component.  If an IncludeKey
	 *	is associated with this Component, the underlying target of the include will be associated with this Component, though the include will be stored as well to determine things like the
	 *	associated KeyPath correctly.
	 *	\param in_key The key to associate with this Component. */
	void					AddKey(Key const & in_key);

	/*! Get the underlying keys (if any) which represent this Component.  Generally this will either be a segment or geometry key.
	 *	\return The underlying keys (if any) which represent this Component. */
	KeyArray				GetKeys() const;



	/*! Adds an owner to this Component.  This is intended to facilitate adding additional owners to an instanced Component, and allows specifying the associated IncludeKey for the additional owner.
	 *	It is possible to add an owner without adding an IncludeKey, and, if desired, add the IncludeKey separately via AddKey.
	 *	\param in_owner The Component to act as an additional owner for this Component.
	 *	\param in_include The IncludeKey for this Component for this new owner.  This defaults to an uninitialized IncludeKey. */
	void					AddOwner(Component & in_owner, IncludeKey const & in_include = IncludeKey());
	
	/*! Get the Component objects (if any) which "own" this Component.  Generally this will be a single element or no elements for a root component.  It is possible for there to be multiple owners
	 *	of a Component if it is instanced.
	 *	\return The Component objects (if any) which "own" this Component. */
	ComponentArray			GetOwners() const;

	/*! Get the subcomponent objects (if any) owned or referenced by this Component.  These Component objects would be deleted if this Component is deleted.
	 *	\return The subcomponent objects (if any) owned or referenced by this Component. */
	ComponentArray			GetSubcomponents() const;

	/*! Gets all components under this Component that have the specified ComponentType mask.  This will examine this subcomponents of this Component, the subcomponents
	 *	of the subcomponents, and so on.
	 *	\param in_type Mask of the components to look for.
	 *	\return The Component objects under this Component (if any) which have the specified mask. */
	ComponentArray			GetAllSubcomponents(ComponentType in_type) const;



	/*! Adds a Component as a reference to this Component.
	 *	\param in_reference The Component to add as a reference to this Component. */
	void					AddReference(Component & in_reference);

	/*! Gets the Component objects which refer to this Component (if any).
	 *	\return The Component objects which refer to this Component. */
	ComponentArray			GetReferrers() const;

	/*! Get the Component objects (if any) which are referenced by this Component.  These are Component objects which are associated with this Component, but not "owned" by this Component, and would
	 *	not be deleted if this Component is deleted (though the reference relationship will be removed).
	 *	\return The Component objects (if any) which are referenced by this Component. */
	ComponentArray			GetReferences() const;



	/*! Get all Metadata objects associated with this Component.
	 *	\return All Metadata objects associated with this Component. */
	MetadataArray			GetAllMetadata() const;

	/*! Get the single Metadata object with the given name associated with this Component (if present).
	 *	\param in_name UTF8-encoded name of the Metadata object to find on this Component.
	 *	\return The single Metadata object with the given name associated with this Component (if present). */
	Metadata				GetMetadata(char const * in_name) const;

	/*! Gets the name for this Component.  This is a convenience function equivalent to getting the "Name" metadata for this Component.  If a Component does not
	 *	have a name, an empty UTF8 object will be returned.
	 *	\return The name (if any) for this Component. */
	UTF8					GetName() const;



	/*! Removes all references components have to this Component, removes all references to any subcomponents, and deletes any underlying keys.
	 *	If a subcomponent of the component being deleted has multiple owners, the deletion will simply remove that reference to the subcomponent, not delete
	 *	it as well.  If a subcomponent of the component being deleted has only one owner (the component being deleted), that subcomponent will be deleted.
	 *	Any handles to the deleted components will become invalid. */
	void					Delete();

	/*! Removes all references to the underlying subcomponents, but leaves this Component around. If a subcomponent of the component being flushed has
	 *	multiple owners, the flush will simply remove the reference to that subcomponent.  If a subcomponent of the component being flushed has only one owner
	 *	(the component being flushed), that subcomponent will be deleted.  Any handles to the deleted components will become invalid. */
	void					Flush();


	/*! Gets the list of KeyPath objects that correspond to the given Component.  There will be at least one KeyPath, and possibly more than one if a particular Component is instanced, e.g.,
	 *	if the Component corresponds to the wheel of a car, a KeyPath would be returned for each wheel (driver side front and rear, and passenger side front and rear).  Each KeyPath in the list will
	 *	also include the portion of the path above the Model (associated with the root CADModel) for each View, Layout layer, and Canvas permutation which includes the Model.  This function will not take
	 *	into account any includes or paths which do not come from the Component hierarchy or the View, Layout, and Canvas permutations which include the Model.
	 *	\param in_component The Component for which to compute the corresponding KeyPath objects.
	 *	\return A list of all KeyPath objects which refer to the given Component.
	 *	\sa HighlightControl */
	static KeyPathArray		GetKeyPath(Component const & in_component);

	/*! Gets the list of KeyPath objects that correspond to the given Component path.  This function can be used to get the KeyPath for a particular instance of an instanced Component.  For a non-instanced
	 *	Component, since the Component path can be determined unambiguously, the KeyPath returned will be the same as if only the leaf non-instanced Component was given to this function.  There will be
	 *	at least one KeyPath returned, and possibly more than one for each View, Layout layer, and Canvas which include the Model associated with the root CADModel.  This function will not take
	 *	into account any includes or paths which do not come from the Component hierarchy or the View, Layout, and Canvas permutations which include the Model.
	 *	\param in_count Size of the following array.
	 *	\param in_components The Component path for which to compute the corresponding KeyPath objects.  The path should be defined such that the first element is the leaf Component, and the last
	 *		element is the root CADModel.  Each element in the path should be an owner of the preceding Component, otherwise an exception will be thrown.
	 *	\return A list of all KeyPath objects which refer to the given Component.
	 *	\sa HighlightControl */
	static KeyPathArray		GetKeyPath(size_t in_count, Component const in_components[]);

	/*! Gets the list of KeyPath objects that correspond to the given Component path.  This function can be used to get the KeyPath for a particular instance of an instanced Component.  For a non-instanced
	 *	Component, since the Component path can be determined unambiguously, the KeyPath returned will be the same as if only the leaf non-instanced Component was given to this function.  There will be
	 *	at least one KeyPath returned, and possibly more than one for each View, Layout layer, and Canvas which include the Model associated with the root CADModel.  This function will not take
	 *	into account any includes or paths which do not come from the Component hierarchy or the View, Layout, and Canvas permutations which include the Model.
	 *	\param in_components The Component path for which to compute the corresponding KeyPath objects.  The path should be defined such that the first element is the leaf Component, and the last
	 *		element is the root CADModel.  Each element in the path should be an owner of the preceding Component, otherwise an exception will be thrown.
	 *	\return A list of all KeyPath objects which refer to the given Component.
	 *	\sa HighlightControl */
	static KeyPathArray		GetKeyPath(ComponentArray const & in_components);
};

/*! The ComponentPath contains an array of components, organized from leaf to root.  It allows the user to perform queries and operations along that path. */
class SPRK_API ComponentPath : public Sprocket
{
public:
	/*! Creates an uninitialized Component path that refers to no database objects. */
	ComponentPath();

	/*! Creates a new ComponentPath object based on the supplied path.
	*	\param in_components An array of components arranged from leaf to root. */
	ComponentPath(ComponentArray const & in_components);

	/*! Creates a new ComponentPath object based on the supplied path.
	*	\param in_count The number of keys in the array.
	*	\param in_components An array of components arranged from leaf to root. */
	ComponentPath(size_t in_count, Component const in_components[]);
	
	/*! Copies in_that into this component path. */
	ComponentPath(ComponentPath const & in_that);

	/*! The move constructor creates a ComponentPath by transferring the underlying impl of the rvalue reference to this ComponentPath
	 * 	thereby avoiding a copy and allocation.
	 * 	\param in_that An rvalue reference to a ComponentPath to take the impl from.*/
	ComponentPath(ComponentPath && in_that);

	/*! The move assignment operator transfers the underlying impl of the rvalue reference to this ComponentPath thereby avoiding a copy.
	* \param in_that An rvalue reference to a ComponentPath to take the impl from.
	* \return A reference to this ComponentPath. */
	ComponentPath & operator=(ComponentPath && in_that);

	/*! Releases a reference to the database object this object is tied to. */
	virtual ~ComponentPath();

	HPS::Type				ObjectType() const { return HPS::Type::ComponentPath; }

	/*! Appends the supplied Component into this object.  Same as Append function.
	*	\param in_key The key to be appended to this.
	*	\return A reference to this object. */
	ComponentPath &			operator+=(Component const & in_component);

	/*! Appends the supplied ComponentPath into this object.  Same as Append function.
	*	\param in_key_path The ComponentPath to be appended to this.
	*	\return A reference to this object. */
	ComponentPath &			operator+=(ComponentPath const & in_path);

	/*! Appends the supplied ComponentArray into this object.  Same as Append function.
	*	\param in_components The ComponentArray to be appended to this.
	*	\return A reference to this object. */
	ComponentPath &			operator+=(ComponentArray const & in_components);

	/*! Appends the supplied Component into this object.  Same as operator+= function.
	*	\param in_component The Component to be appended to this.
	*	\return A reference to this object. */
	ComponentPath &			Append(Component const & in_component);

	/*! Appends the supplied key into this object.  Same as operator+= function.
	*	\param in_path The ComponentPath to be appended to this.
	*	\return A reference to this object. */
	ComponentPath &			Append(ComponentPath const & in_path);

	/*! Appends the supplied key into this object.  Same as operator+= function.
	*	\param in_components The ComponentArray to be appended to this.
	*	\return A reference to this object. */
	ComponentPath &			Append(ComponentArray const & in_components);

	/*! Copies the supplied path into this object.
	*	\param in_that The source of the copy.
	*	\return A reference to this object. */
	ComponentPath &			operator=(ComponentPath const & in_that);

	/*! Copies an array of components into this object.
	*	\param in_path The source array, assumed to be arranged from leaf to root.
	*	\return A reference to this object. */
	ComponentPath &			operator=(ComponentArray const & in_path);

	/*! Copies the supplied path into this object.
	*	\param in_that The source of the copy. */
	void					Set(ComponentPath const & in_that);

	/*! Check if the source ComponentPath is equivalent to this object.
	*	\param in_that The source ComponentPath to compare to this object.
	*	\return <span class='code'>true</span> if the objects are equivalent, <span class='code'>false</span> otherwise. */
	bool					Equals(ComponentPath const & in_that) const;
	
	/*! Check if the source ComponentPath is not equivalent to this object.
	*	\param in_that The source ComponentPath to compare to this object.
	*	\return <span class='code'>true</span> if the objects are not equivalent, <span class='code'>false</span> otherwise. */
	bool					operator!= (ComponentPath const & in_that) const;

	/*! Check if the source ComponentPath is equivalent to this object.
	*	\param in_that The source ComponentPath to compare to this object.
	*	\return <span class='code'>true</span> if the objects are equivalent, <span class='code'>false</span> otherwise. */
	bool					operator== (ComponentPath const & in_that) const;


	/*! Copies an array of components into this object.
	*	\param in_components The source array, assumed to be arranged from leaf to root.
	*	\return A reference to this object. */
	ComponentPath &			SetComponents(ComponentArray const & in_components);

	/*! Copies an array of components into this object.
	*	\param in_count the number of components in the array.
	*	\param in_components The source array, assumed to arranged from leaf to root.
	*	\return A reference to this object. */
	ComponentPath &			SetComponents(size_t in_count, Component const in_components[]);

	/*! Gets the components defining this ComponentPath.
	 *	\return The components defining this ComponentPath. */
	ComponentArray			GetComponents() const;

	/*! Gets the KeyPath objects corresponding to this ComponentPath.  These key paths will include all possible paths which include the path components and all Canvas, Layout and View
	 *	paths which include the Model which contains the path components.
	 *	\return All KeyPath objects for this component path. */
	KeyPathArray			GetKeyPaths() const;

	/*! Gets the KeyPath objects corresponding to this ComponentPath which are included by the specified Canvas and layer.
	 *	\return The KeyPath objects for this component path which are included by the specified Canvas and layer.
	 *	\param in_canvas The Canvas the returned KeyPath objects should reference. 
	 *	\param in_layer The layer under the provided Canvas the returned KeyPath objects should reference. */
	KeyPathArray			GetKeyPaths(Canvas const & in_canvas, size_t in_layer = 0) const;


	/*! Highlights this ComponentPath with the provided options under the given Canvas in layer zero.
	 *	\param in_canvas The Canvas in which to perform the highlight.
	 *	\param in_options The options to use to perform the highlight.*/
	void					Highlight(Canvas const & in_canvas, HighlightOptionsKit const & in_options) const;

	/*! Highlights this ComponentPath with the provided options under the given Canvas in the specified layer.
	 *	\param in_canvas The Canvas in which to perform the highlight.
	 *	\param in_layer The layer under the provided Canvas in which to perform the highlight.
	 *	\param in_options The options to use to perform the highlight. */
	void					Highlight(Canvas const & in_canvas, size_t in_layer, HighlightOptionsKit const & in_options) const;

	/*! Unhighlights this ComponentPath with the provided options under the given Canvas in layer zero.
	 *	\param in_canvas The Canvas in which to perform the unhighlight.
	 *	\param in_options The options to use to perform the unhighlight. */
	void					Unhighlight(Canvas const & in_canvas, HighlightOptionsKit const & in_options = HighlightOptionsKit()) const;

	/*! Unhighlights this ComponentPath with the provided options under the given Canvas in the specified layer.
	 *	\param in_canvas The Canvas in which to perform the unhighlight.
	 *	\param in_layer The layer under the provided Canvas in which to perform the unhighlight.
	 *	\param in_options The options to use to perform the unhighlight. */
	void					Unhighlight(Canvas const & in_canvas, size_t in_layer, HighlightOptionsKit const & in_options = HighlightOptionsKit()) const;


	/*! Hides this ComponentPath under the given Canvas in the specified layer.
	 *	\param in_canvas The Canvas in which to hide the ComponentPath.
	 *	\param in_layer The layer under the provided Canvas in which to hide the ComponentPath. */
	void					Hide(Canvas const & in_canvas, size_t in_layer = 0);

	/*! Shows this ComponentPath under the given Canvas in the specified layer.
	 *	\param in_canvas The Canvas in which to show the ComponentPath.
	 *	\param in_layer The layer under the provided Canvas in which to show the ComponentPath. */
	void					Show(Canvas const & in_canvas, size_t in_layer = 0);

	/*! Indicates whether this ComponentPath under the given Canvas in the specified layer is hidden.  Note that even if a path is hidden, it's possible
	 *	that paths descending from this path may not be hidden.
	 *	\param in_canvas The Canvas in which to check whether the specified path is hidden.
	 *	\param in_layer The layer under the provided Canvas in which to check whether the specified path is hidden.
	 *	\return <span class='code'>true</span> if this path is hidden, <span class='code'>false</span> otherwise. */
	bool					IsHidden(Canvas const & in_canvas, size_t in_layer = 0);

	/*! Shows only this ComponentPath and hides all other Component objects under the given Canvas in the specified layer.
	 *	\param in_canvas The Canvas in which to isolate the ComponentPath.
	 *	\param in_layer The layer under the provided Canvas in which to isolate the ComponentPath. */
	void					Isolate(Canvas const & in_canvas, size_t in_layer = 0);

	static void				Isolate(HPS::ComponentPathArray & in_components_to_be_isolated, Canvas const & in_canvas, size_t in_layer = 0);
};


/*! Creates a new ComponentPath object representing the concatenation of two Components.
 *	\param in_lhs The first Component in the returned ComponentPath.
 *	\param in_rhs The last Component in the returned ComponentPath.
 *	\return A new ComponentPath object representing the concatenation of two Components. */
inline ComponentPath operator+(Component const & in_lhs, Component const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of the Component and Component array.
 *	\param in_lhs The first Component in the returned ComponentPath.
 *	\param in_rhs The Components to append to the first Component in the returned ComponentPath.
 *	\return A new ComponentPath object representing the concatenation of the Component and Component array. */
inline ComponentPath operator+(Component const & in_lhs, ComponentArray const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of a Component and Component path.
 *	\param in_lhs The first Component in the returned ComponentPath.
 *	\param in_rhs The ComponentPath to append to the first Component in the returned ComponentPath.  This ComponentPath will be unchanged.
 *	\return A new ComponentPath object representing the concatenation of a Component and Component path. */
inline ComponentPath operator+(Component const & in_lhs, ComponentPath const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of a Component array and a Component.
 *	\param in_lhs The Component array at the beginning of the returned ComponentPath.
 *	\param in_rhs The last Component in the returned ComponentPath.
 *	\return A new ComponentPath object representing the concatenation of a Component array and a Component. */
inline ComponentPath operator+(ComponentArray const & in_lhs, Component const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of two Component arrays.
 *	\param in_lhs The Component array at the beginning of the returned ComponentPath.
 *	\param in_rhs The Component array at the end of the returned ComponentPath.
 *	\return A new ComponentPath object representing the concatenation of two Component arrays. */
inline ComponentPath operator+(ComponentArray const & in_lhs, ComponentArray const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of a Component array and a Component path.
 *	\param in_lhs The Component array at the beginning of the returned ComponentPath.
 *	\param in_rhs The ComponentPath to append to the Component array in the returned ComponentPath.  This ComponentPath will be unchanged.
 *	\return A new ComponentPath object representing the concatenation of a Component array and a Component path. */
inline ComponentPath operator+(ComponentArray const & in_lhs, ComponentPath const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of a Component path and a Component.
 *	\param in_lhs The ComponentPath to begin the returned ComponentPath.  This ComponentPath will be unchanged.
 *	\param in_rhs The last Component in the returned ComponentPath.
 *	\return A new ComponentPath object representing the concatenation of a Component path and a Component. */
inline ComponentPath operator+(ComponentPath const & in_lhs, Component const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of a Component path and Component array.
 *	\param in_lhs The ComponentPath to begin the returned ComponentPath.  This ComponentPath will be unchanged.
 *	\param in_rhs The Component array at the end of the returned ComponentPath.
 *	\return A new ComponentPath object representing the concatenation of a Component path and Component array. */
inline ComponentPath operator+(ComponentPath const & in_lhs, ComponentArray const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}

/*! Creates a new ComponentPath object representing the concatenation of two Component paths.
 *	\param in_lhs The ComponentPath to begin the returned ComponentPath.  This ComponentPath will be unchanged.
 *	\param in_rhs The ComponentPath to end the returned ComponentPath.  This ComponentPath will be unchanged.
 *	\return A new ComponentPath object representing the concatenation of two Component paths. */
inline ComponentPath operator+(ComponentPath const & in_lhs, ComponentPath const & in_rhs)
{
	return HPS::ComponentPath().Append(in_lhs).Append(in_rhs);
}


/*! The ComponentHighlightEvent class is the event that will be triggered for each highlight or unhighlight call. */
class SPRK_API ComponentHighlightEvent : public Event
{
public:
	enum class Action
	{
		None = 0,
		Highlight,
		Unhighlight,
	};

	/*! The default constructor creates an empty ComponentHighlightEvent object. */
	ComponentHighlightEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
		action = Action::None;
	}

	ComponentHighlightEvent(Action in_action,
		ComponentPath const & in_path = ComponentPath(),
		HighlightOptionsKit const & in_options = HighlightOptionsKit())
		: Event(), action(in_action), path(in_path), options(in_options)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a ComponentHighlightEvent object.
	 * 	\param in_event The Event Object to be converted. */
	ComponentHighlightEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<ComponentHighlightEvent>())
		{
			auto that = static_cast<ComponentHighlightEvent const &>(in_event);
			action = that.action;
			path = that.path;
			options = that.options;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~ComponentHighlightEvent();

	/*! Allocates and returns a copy of this ComponentHighlightEvent.
	 * 	\return A copy of this ComponentHighlightEvent. */
	Event * Clone() const
	{
		ComponentHighlightEvent * new_event = new ComponentHighlightEvent(*this);
		return new_event;
	}

	Action action;
	ComponentPath path;
	HighlightOptionsKit options;
};


/*! The Filter class is a smart pointer.  It represents a filter, i.e., a collection of visibility settings for a CADModel. */
class SPRK_API Filter : public Component
{
public:
	/*! The default constructor creates an uninitialized Filter object.  The Type() function will return Type::None. */
	Filter();

	/*! This constructor creates a Filter object that shares the underlying smart-pointer of the source Component.  The copy will only be successful if the source component is really
	 *	an upcast of a Filter object.  Otherwise the copy will fail and the resulting Filter will be invalid.
	 *	\param in_that The source Component to copy. */
	Filter(Component const & in_that);

	/*! The copy constructor creates a Filter object that shares the underlying smart-pointer of the source Filter.
	 *	\param in_that The source Filter to copy. */
	Filter(Filter const & in_that);

	/*! The move constructor creates a Filter by transferring the underlying object of the rvalue reference to this Filter.
	 * 	\param in_that An rvalue reference to a Filter to take the underlying object from. */
	Filter(Filter && in_that);

	virtual ~Filter();

	HPS::Type				ObjectType() const { return HPS::Type::Filter; }


#if !defined(_MSC_VER) || _MSC_VER >= 1900
	Filter &				operator=(Filter const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this Filter.
	 *	\param in_that An rvalue reference to a Filter to take the underlying object from.
	 *	\return A reference to this Filter. */
	Filter &				operator=(Filter && in_that);


	/*! Activates this Filter on a given View.  The View must have an attached CADModel which this Filter belongs to, otherwise an exception will be thrown.
	 *	\param in_view The View on which to activate this Filter. */
	void					Activate(View const & in_view);

	/*! Dectivates this Filter on a given View.  The View must have this Filter activated on it, otherwise an exception will be thrown.
	 *	\param in_view The View on which to deactivate this Filter. */
	void					Deactivate(View const & in_view);
};


class SPRK_API FilterActivationEvent : public Event
{
public:
	enum class Action
	{
		None = 0,
		Activate,
		Deactivate,
	};

	/*! The default constructor creates an empty FilterActivationEvent object. */
	FilterActivationEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	FilterActivationEvent(Filter in_filter, Action in_action, View in_view) : Event(), filter(in_filter), action(in_action), view(in_view)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a FilterActivationEvent object.
	 * 	\param in_event The Event Object to be converted. */
	FilterActivationEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<FilterActivationEvent>())
		{
			auto that = static_cast<FilterActivationEvent const &>(in_event);
			filter = that.filter;
			action = that.action;
			view = that.view;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~FilterActivationEvent();

	/*! Allocates and returns a copy of this FilterActivationEvent.
	 * 	\return A copy of this FilterActivationEvent. */
	Event * Clone() const
	{
		FilterActivationEvent * new_event = new FilterActivationEvent(*this);
		return new_event;
	}

	Filter filter;
	Action action;
	View view;
};


/*! The Capture class is a smart pointer.  It represents a capture, i.e., a collection of visibility settings, material settings, transformation settings, and camera, for a CADModel. */
class SPRK_API Capture : public Component
{
public:
	/*! The default constructor creates an uninitialized Capture object.  The Type() function will return Type::None. */
	Capture();

	/*! This constructor creates a Capture object that shares the underlying smart-pointer of the source Component.  The copy will only be successful if the source component is really
	 *	an upcast of a Capture object.  Otherwise the copy will fail and the resulting Capture will be invalid.
	 *	\param in_that The source Component to copy. */
	Capture(Component const & in_that);

	/*! The copy constructor creates a Capture object that shares the underlying smart-pointer of the source Capture.
	 *	\param in_that The source Capture to copy. */
	Capture(Capture const & in_that);

	/*! The move constructor creates a Capture by transferring the underlying object of the rvalue reference to this Capture.
	 * 	\param in_that An rvalue reference to a Capture to take the underlying object from. */
	Capture(Capture && in_that);

	virtual ~Capture();

	HPS::Type				ObjectType() const { return HPS::Type::Capture; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	Capture &				operator=(Capture const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this Capture.
	 *	\param in_that An rvalue reference to a Capture to take the underlying object from.
	 *	\return A reference to this Capture. */
	Capture &				operator=(Capture && in_that);


	/*! Creates a new View object corresponding to this Capture.  The View object that is returned will have the corresponding visibility settings, material settings, transformation settings,
	 *	and camera from this Capture set on it.
	 *	\return A newly created View object corresponding to this Capture. */
	View					Activate();
};


class SPRK_API CaptureActivationEvent : public Event
{
public:
	/*! The default constructor creates an empty CaptureActivationEvent object. */
	CaptureActivationEvent() : Event()
	{
		channel = GetClassID();
		consumable = false;
	}

	CaptureActivationEvent(Capture in_capture, View in_view) : Event(), capture(in_capture), view(in_view)
	{
		channel = GetClassID();
		consumable = false;
	}

	/*! This constructor converts an Event Object to a CaptureActivationEvent object.
	 * 	\param in_event The Event Object to be converted. */
	CaptureActivationEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<CaptureActivationEvent>())
		{
			auto that = static_cast<CaptureActivationEvent const &>(in_event);
			capture = that.capture;
			view = that.view;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	~CaptureActivationEvent();

	/*! Allocates and returns a copy of this CaptureActivationEvent.
	 * 	\return A copy of this CaptureActivationEvent. */
	Event * Clone() const
	{
		CaptureActivationEvent * new_event = new CaptureActivationEvent(*this);
		return new_event;
	}

	Capture capture;
	View view;
};


/*! The CADModel class is a smart pointer.  It represents the root Component for a structured file format. */
class SPRK_API CADModel : public Component
{
public:
	/*! The default constructor creates an uninitialized CADModel object.  The Type() function will return Type::None. */
	CADModel();

	/*! This constructor creates a CADModel object that shares the underlying smart-pointer of the source Component.  The copy will only be successful if the source component is really
	 *	an upcast of a CADModel object.  Otherwise the copy will fail and the resulting CADModel will be invalid.
	 *	\param in_that The source Component to copy. */
	CADModel(Component const & in_that);

	/*! The copy constructor creates a CADModel object that shares the underlying smart-pointer of the source CADModel.
	 *	\param in_that The source CADModel to copy. */
	CADModel(CADModel const & in_that);

	/*! The move constructor creates a CADModel by transferring the underlying object of the rvalue reference to this CADModel.
	 * 	\param in_that An rvalue reference to a CADModel to take the underlying object from. */
	CADModel(CADModel && in_that);

	/*! This constructor creates a CADModel object that is associated with the given Model.  This will throw an exception if the given Model is not associated with a CADModel.
	 *	\param in_that The source Model to use to find the associated CADModel. */
	CADModel(Model const & in_that);

	virtual ~CADModel();

	HPS::Type				ObjectType() const { return HPS::Type::CADModel; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	CADModel &				operator=(CADModel const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this CADModel.
	 *	\param in_that An rvalue reference to a CADModel to take the underlying object from.
	 *	\return A reference to this CADModel. */
	CADModel &				operator=(CADModel && in_that);


	/*! Find the Component associated with the given Key (if any).
	 *	\param in_key The Key from which to find the associated Component.
	 *	\return The Component associated with the given Key (if any). */
	Component				GetComponentFromKey(Key const & in_key) const;

	/*! Get the Component path which corresponds to the given KeyPath.  The returned path will basically be the Component associated with each key which occurs in the given KeyPath, up to the CADModel.
	 *	Note that since there are some Component objects for which there is no underlying Key (e.g., a Component representing a CoEdge), these objects will not appear in the returned Component list.
	 *	\param in_key_path The KeyPath for which to compute the corresponding Component path.
	 *	\return The Component path corresponding to the given KeyPath. */
	ComponentPath			GetComponentPath(KeyPath const & in_key_path) const;

	/*! Get the Component path which corresponds to the given SelectionItem.  The returned path will basically be the Component associated with each key which occurs in the KeyPath associated with the
	 *	SelectionItem, up to the CADModel.  Note that since there are some Component objects for which there is no underlying Key (e.g., a Component representing a CoEdge), these objects will not appear
	 *	in the returned Component list.
	 *	\param in_item The SelectionItem for which to compute the corresponding Component path.
	 *	\return The Component path corresponding to the given SelectionItem. */
	ComponentPath			GetComponentPath(SelectionItem const & in_item) const;


	/*! Get the Model associated with this CADModel.
	 *	\return The Model associated with this CADModel. */
	Model					GetModel() const;

	/*! Get a list of all filters from all subcomponents for this CADModel.
	 *	\return A list of all filters from all subcomponents for this CADModel. */
	FilterArray				GetAllFilters() const;

	/*! Get a list of all captures from all subcomponents for this CADModel.
	 *	\return A list of all captures from all subcomponents for this CADModel. */
	CaptureArray			GetAllCaptures() const;

	/*! Get a list of the active filters on the given View.
	 *	\param in_view The View for which to get the active filters.
	 *	\return A list of the active filters on the given View. */
	FilterArray				GetActiveFilters(View const & in_view) const;


	/*! Creates a new View object corresponding to the default capture for this CADModel.  The View object that is returned will have the corresponding visibility settings, material settings,
	 *	transformation settings, and camera for the default capture set on it.
	 *	\return A newly created View object corresponding to this Capture. */
	View					ActivateDefaultCapture();


	/*! Resets the visibility in the view specified by the given Canvas and layer.  This will undo any hides, shows or isolates performed on components
	 *	contained within this CADModel via ComponentPath calls.
	 *	\param in_canvas The Canvas in which to perform the the visibility reset.
	 *	\param in_layer The layer in the Canvas in which to perform the visibility reset.
	 *	\sa ComponentPath::Hide 
	 *	\sa ComponentPath::Show
	 *	\sa ComponentPath::Isolate */
	void					ResetVisibility(Canvas & in_canvas, size_t in_layer = 0);
};



/*!	The Metadata class is a smart pointer.  It represents a variety of metadata, i.e., a (name, value) pair, which might be associated with a Component,
 *	e.g., IDs, names, layers, user-defined or format-specific attributes, etc. */
class SPRK_API Metadata : public Sprocket
{
public:
	/*! The default constructor creates an uninitialized Metadata object.  The Type() function will return Type::None. */
	Metadata();

	/*! The copy constructor creates a Metadata object that shares the underlying smart-pointer of the source Metadata.
	 *	\param in_that The source Metadata to copy. */
	Metadata(Metadata const & in_that);

	/*! The move constructor creates a Metadata by transferring the underlying object of the rvalue reference to this Metadata.
	 * 	\param in_that An rvalue reference to a Metadata to take the underlying object from. */
	Metadata(Metadata && in_that);

	/*! This constructor creates a Metadata with a given name.
	 * 	\param in_name The UTF8-encoded name for this Metadata. */
	Metadata(char const * in_name);

	virtual ~Metadata();

	HPS::Type				ObjectType() const { return HPS::Type::Metadata; }


	/*! Share the underlying smart-pointer of the Metadata source.
	 *	\param in_that The Metadata source of the assignment.
	 *	\return A reference to this Metadata. */
	Metadata &				operator=(Metadata const & in_that);

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this Metadata.
	 *	\param in_that An rvalue reference to a Metadata to take the underlying object from.
	 *	\return A reference to this Metadata. */
	Metadata &				operator=(Metadata && in_that);

	/*! Share the underlying smart-pointer of the Metadata source.
	 *	\param in_that The Metadata source of the assignment.
	 *	\return A reference to this Metadata. */
	virtual void			Assign(Metadata const & in_that);

	/*!	Check if the source Metadata points to the same underlying impl as this Metadata.
	 *	\param in_that The source Metadata to compare to this Metadata.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					Equals(Metadata const & in_that) const;

	/*!	Check if the source Metadata points to a different impl than this Metadata.
	 *	\param in_that The source Metadata to compare to this Metadata.
	 *	\return <span class='code'>true</span> if the objects reference different impls, <span class='code'>false</span> otherwise. */
	bool					operator!=(Metadata const & in_that) const;

	/*!	Check if the source Metadata points to the same underlying impl as this Metadata.
	 *	\param in_that The source Metadata to compare to this Metadata.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					operator==(Metadata const & in_that) const;


	/*! Gets the name for this Metadata.
	 *	\return The name for this Metadata. */
	HPS::UTF8				GetName() const;

	/*! Sets the name for this Metadata.
	 *	\param in_name A UTF8-encoded name for this Metadata. */
	void					SetName(char const * in_name);
};

/*!	The IntegerMetadata class is a smart pointer.  It represents a piece of integer Metadata. */
class SPRK_API IntegerMetadata : public Metadata
{
public:
	/*! The default constructor creates an uninitialized IntegerMetadata object.  The Type() function will return Type::None. */
	IntegerMetadata();

	/*! This constructor creates an IntegerMetadata object that shares the underlying smart-pointer of the source Metadata.  The copy will only be successful if the source component is really
	 *	an upcast of an IntegerMetadata object.  Otherwise the copy will fail and the resulting IntegerMetadata will be invalid.
	 *	\param in_that The source Metadata to copy. */
	IntegerMetadata(Metadata const & in_that);

	/*! The copy constructor creates an IntegerMetadata object that shares the underlying smart-pointer of the source IntegerMetadata.
	 *	\param in_that The source IntegerMetadata to copy. */
	IntegerMetadata(IntegerMetadata const & in_that);

	/*! The move constructor creates an IntegerMetadata by transferring the underlying object of the rvalue reference to this IntegerMetadata.
	 * 	\param in_that An rvalue reference to an IntegerMetadata to take the underlying object from. */
	IntegerMetadata(IntegerMetadata && in_that);

	/*! This constructor creates an IntegerMetadata with a given name and value.
	 *	\param in_name The UTF8-encoded name for this IntegerMetadata.
	 *	\param in_value The integer value for this IntegerMetadata. */
	IntegerMetadata(char const * in_name, int in_value);

	virtual ~IntegerMetadata();

	HPS::Type				ObjectType() const { return HPS::Type::IntegerMetadata; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	IntegerMetadata &		operator=(IntegerMetadata const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this IntegerMetadata.
	 *	\param in_that An rvalue reference to an IntegerMetadata to take the underlying object from.
	 *	\return A reference to this IntegerMetadata. */
	IntegerMetadata &		operator=(IntegerMetadata && in_that);


	/*! Gets the integer value for this IntegerMetadata.
	 *	\return The integer value for this IntegerMetadata. */
	int						GetValue() const;

	/*! Sets the integer value for this IntegerMetadata.
	 *	\param in_value The integer value for this IntegerMetadata. */
	void					SetValue(int in_value);
};

/*!	The UnsignedIntegerMetadata class is a smart pointer.  It represents a piece of unsigned integer Metadata. */
class SPRK_API UnsignedIntegerMetadata : public Metadata
{
public:
	/*! The default constructor creates an uninitialized UnsignedIntegerMetadata object.  The Type() function will return Type::None. */
	UnsignedIntegerMetadata();

	/*! This constructor creates an UnsignedIntegerMetadata object that shares the underlying smart-pointer of the source Metadata.  The copy will only be successful if the source component is really
	 *	an upcast of an UnsignedIntegerMetadata object.  Otherwise the copy will fail and the resulting UnsignedIntegerMetadata will be invalid.
	 *	\param in_that The source Metadata to copy. */
	UnsignedIntegerMetadata(Metadata const & in_that);

	/*! The copy constructor creates an UnsignedIntegerMetadata object that shares the underlying smart-pointer of the source UnsignedIntegerMetadata.
	 *	\param in_that The source UnsignedIntegerMetadata to copy. */
	UnsignedIntegerMetadata(UnsignedIntegerMetadata const & in_that);

	/*! The move constructor creates an UnsignedIntegerMetadata by transferring the underlying object of the rvalue reference to this UnsignedIntegerMetadata.
	 * 	\param in_that An rvalue reference to an UnsignedIntegerMetadata to take the underlying object from. */
	UnsignedIntegerMetadata(UnsignedIntegerMetadata && in_that);

	/*! This constructor creates an UnsignedIntegerMetadata with a given name and value.
	 *	\param in_name The UTF8-encoded name for this UnsignedIntegerMetadata.
	 *	\param in_value The unsigned integer value for this UnsignedIntegerMetadata. */
	UnsignedIntegerMetadata(char const * in_name, unsigned int in_value);

	virtual ~UnsignedIntegerMetadata();

	HPS::Type					ObjectType() const { return HPS::Type::UnsignedIntegerMetadata; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	UnsignedIntegerMetadata &	operator=(UnsignedIntegerMetadata const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this UnsignedIntegerMetadata.
	 *	\param in_that An rvalue reference to a UnsignedIntegerMetadata to take the underlying object from.
	 *	\return A reference to this UnsignedIntegerMetadata. */
	UnsignedIntegerMetadata &	operator=(UnsignedIntegerMetadata && in_that);


	/*! Gets the unsigned integer value for this UnsignedIntegerMetadata.
	 *	\return The unsigned integer value for this UnsignedIntegerMetadata. */
	unsigned int				GetValue() const;

	/*! Sets the unsigned integer value for this UnsignedIntegerMetadata.
	 *	\param in_value The unsigned integer value for this UnsignedIntegerMetadata. */
	void						SetValue(unsigned int in_value);
};

/*!	The DoubleMetadata class is a smart pointer.  It represents a piece of double Metadata. */
class SPRK_API DoubleMetadata : public Metadata
{
public:
	/*! The default constructor creates an uninitialized DoubleMetadata object.  The Type() function will return Type::None. */
	DoubleMetadata();

	/*! This constructor creates a DoubleMetadata object that shares the underlying smart-pointer of the source Metadata.  The copy will only be successful if the source component is really
	 *	an upcast of a DoubleMetadata object.  Otherwise the copy will fail and the resulting DoubleMetadata will be invalid.
	 *	\param in_that The source Metadata to copy. */
	DoubleMetadata(Metadata const & in_that);

	/*! The copy constructor creates a DoubleMetadata object that shares the underlying smart-pointer of the source DoubleMetadata.
	 *	\param in_that The source DoubleMetadata to copy. */
	DoubleMetadata(DoubleMetadata const & in_that);

	/*! The move constructor creates a DoubleMetadata by transferring the underlying object of the rvalue reference to this DoubleMetadata.
	 * 	\param in_that An rvalue reference to a DoubleMetadata to take the underlying object from. */
	DoubleMetadata(DoubleMetadata && in_that);

	/*! This constructor creates a DoubleMetadata with a given name and value.
	 *	\param in_name The UTF8-encoded name for this DoubleMetadata.
	 *	\param in_value The double value for this DoubleMetadata. */
	DoubleMetadata(char const * in_name, double in_value);

	virtual ~DoubleMetadata();

	HPS::Type				ObjectType() const { return HPS::Type::DoubleMetadata; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	DoubleMetadata &		operator=(DoubleMetadata const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this DoubleMetadata.
	 *	\param in_that An rvalue reference to a DoubleMetadata to take the underlying object from.
	 *	\return A reference to this DoubleMetadata. */
	DoubleMetadata &		operator=(DoubleMetadata && in_that);


	/*! Gets the double value for this DoubleMetadata.
	 *	\return The double value for this DoubleMetadata. */
	double					GetValue() const;

	/*! Sets the double value for this DoubleMetadata.
	 *	\param in_value The double value for this DoubleMetadata. */
	void					SetValue(double in_value);
};

/*!	The StringMetadata class is a smart pointer.  It represents a piece of string Metadata. */
class SPRK_API StringMetadata : public Metadata
{
public:
	/*! The default constructor creates an uninitialized StringMetadata object.  The Type() function will return Type::None. */
	StringMetadata();

	/*! This constructor creates a StringMetadata object that shares the underlying smart-pointer of the source Metadata.  The copy will only be successful if the source component is really
	 *	an upcast of a StringMetadata object.  Otherwise the copy will fail and the resulting StringMetadata will be invalid.
	 *	\param in_that The source Metadata to copy. */
	StringMetadata(Metadata const & in_that);

	/*! The copy constructor creates a StringMetadata object that shares the underlying smart-pointer of the source StringMetadata.
	 *	\param in_that The source StringMetadata to copy. */
	StringMetadata(StringMetadata const & in_that);

	/*! The move constructor creates a StringMetadata by transferring the underlying object of the rvalue reference to this StringMetadata.
	 * 	\param in_that An rvalue reference to a StringMetadata to take the underlying object from. */
	StringMetadata(StringMetadata && in_that);

	/*! This constructor creates a StringMetadata with a given name and value.
	 *	\param in_name The UTF8-encoded name for this StringMetadata.
	 *	\param in_value The UTF8-encoded string value for this StringMetadata. */
	StringMetadata(char const * in_name, char const * in_value);

	virtual ~StringMetadata();

	HPS::Type				ObjectType() const { return HPS::Type::StringMetadata; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	StringMetadata &		operator=(StringMetadata const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this StringMetadata.
	 *	\param in_that An rvalue reference to a StringMetadata to take the underlying object from.
	 *	\return A reference to this StringMetadata. */
	StringMetadata &		operator=(StringMetadata && in_that);


	/*! Gets the string value for this StringMetadata.
	 *	\return The string value for this StringMetadata. */
	UTF8					GetValue() const;

	/*! Sets the string value for this StringMetadata.
	 *	\param in_value The UTF8-encoded string value for this StringMetadata. */
	void					SetValue(char const * in_value);
};

/*!	The TimeMetadata class is a smart pointer.  It represents a piece of time Metadata, i.e., a 32-bit integer to be interpreted as time_t. */
class SPRK_API TimeMetadata : public Metadata
{
public:
	/*! The default constructor creates an uninitialized TimeMetadata object.  The Type() function will return Type::None. */
	TimeMetadata();

	/*! This constructor creates a TimeMetadata object that shares the underlying smart-pointer of the source Metadata.  The copy will only be successful if the source component is really
	 *	an upcast of a TimeMetadata object.  Otherwise the copy will fail and the resulting TimeMetadata will be invalid.
	 *	\param in_that The source Metadata to copy. */
	TimeMetadata(Metadata const & in_that);

	/*! The copy constructor creates a TimeMetadata object that shares the underlying smart-pointer of the source TimeMetadata.
	 *	\param in_that The source TimeMetadata to copy. */
	TimeMetadata(TimeMetadata const & in_that);

	/*! The move constructor creates a TimeMetadata by transferring the underlying object of the rvalue reference to this TimeMetadata.
	 * 	\param in_that An rvalue reference to a TimeMetadata to take the underlying object from. */
	TimeMetadata(TimeMetadata && in_that);

	/*! This constructor creates a TimeMetadata with a given name and value.
	 *	\param in_name The UTF8-encoded name for this TimeMetadata.
	 *	\param in_value The unsigned integer value (interpreted like time_t) for this TimeMetadata. */
	TimeMetadata(char const * in_name, unsigned int in_value);

	virtual ~TimeMetadata();

	HPS::Type				ObjectType() const { return HPS::Type::TimeMetadata; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	TimeMetadata &			operator=(TimeMetadata const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this TimeMetadata.
	 *	\param in_that An rvalue reference to a TimeMetadata to take the underlying object from.
	 *	\return A reference to this TimeMetadata. */
	TimeMetadata &			operator=(TimeMetadata && in_that);


	/*! Gets the unsigned integer value for this TimeMetadata.
	 *	\return The unsigned integer value for this TimeMetadata. */
	unsigned int			GetValue() const;

	/*! Gets the string represenation of the time value as returned by asctime() for this TimeMetadata.
	 *	\return The string representation of the time value as returned by asctime() for this TimeMetadata. */
	HPS::UTF8				GetValueAsString() const;

	/*! Sets the unsigned integer value (interpreted like time_t) for this TimeMetadata.
	 *	\param in_value The unsigned integer value (interpreted like time_t) for this TimeMetadata. */
	void					SetValue(unsigned int in_value);
};


/*!	The BooleanMetadata class is a smart pointer.  It represents a piece of boolean Metadata. */
class SPRK_API BooleanMetadata : public Metadata
{
public:
	/*! The default constructor creates an uninitialized BooleanMetadata object.  The Type() function will return Type::None. */
	BooleanMetadata();

	/*! This constructor creates a BooleanMetadata object that shares the underlying smart-pointer of the source Metadata.  The copy will only be successful if the source component is really
	 *	an upcast of a BooleanMetadata object.  Otherwise the copy will fail and the resulting BooleanMetadata will be invalid.
	 *	\param in_that The source Metadata to copy. */
	BooleanMetadata(Metadata const & in_that);

	/*! The copy constructor creates a BooleanMetadata object that shares the underlying smart-pointer of the source BooleanMetadata.
	 *	\param in_that The source BooleanMetadata to copy. */
	BooleanMetadata(BooleanMetadata const & in_that);

	/*! The move constructor creates a BooleanMetadata by transferring the underlying object of the rvalue reference to this BooleanMetadata.
	 * 	\param in_that An rvalue reference to a BooleanMetadata to take the underlying object from. */
	BooleanMetadata(BooleanMetadata && in_that);

	/*! This constructor creates a BooleanMetadata with a given name and value.
	 *	\param in_name The UTF8-encoded name for this BooleanMetadata.
	 *	\param in_value The boolean value for this BooleanMetadata. */
	BooleanMetadata(char const * in_name, bool in_value);

	virtual ~BooleanMetadata();

	HPS::Type				ObjectType() const { return HPS::Type::BooleanMetadata; }

#if !defined(_MSC_VER) || _MSC_VER >= 1900
	BooleanMetadata &		operator=(BooleanMetadata const & in_that) = default;
#endif

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this BooleanMetadata.
	 *	\param in_that An rvalue reference to a BooleanMetadata to take the underlying object from.
	 *	\return A reference to this BooleanMetadata. */
	BooleanMetadata &		operator=(BooleanMetadata && in_that);


	/*! Gets the boolean value for this BooleanMetadata.
	 *	\return The boolean value for this BooleanMetadata. */
	bool					GetValue() const;

	/*! Sets the boolean value for this BooleanMetadata.
	 *	\param in_value The boolean value for this BooleanMetadata. */
	void					SetValue(bool in_value);
};



/*! The Factory class is used to create and query Sprocket objects. */
class SPRK_API Factory : public Sprocket
{
public:

	/*! Get a list of all active Canvases (i.e. Canvases that have been created and not deleted) 
	 *	\return A list of active Canvases. */
	static CanvasArray		GetCanvases();

	/*! Get a list of all active Layouts (i.e. Layouts that have been created and not deleted) 
	 *	\return A list of active Layouts. */
	static LayoutArray		GetLayouts();

	/*! Get a list of all active Views (i.e. Views that have been created and not deleted) 
	 *	\return A list of active Views. */
	static ViewArray		GetViews();

	/*! Get a list of all active Models (i.e. Models that have been created and not deleted) 
	 *	\return A list of active Models. */
	static ModelArray		GetModels();

	/*! Get a list of all active CADModels (i.e. CADModels that have been created and not deleted) 
	 *	\return A list of active CADModels. */
	static CADModelArray	GetCADModels();

	/*! Creates a new Canvas.
	 * \param in_window_handle The window handle associated with this Canvas. 
	 * \param in_name An optional name associated with this Canvas.
	 * \param in_options An ApplicationWindowOptionsKit used to specify the driver to be used by the canvas, and whether it will support anti-aliasing. 
	 * \return The newly created Canvas. */
	static Canvas			CreateCanvas(HPS::WindowHandle in_window_handle, char const * in_name = "", HPS::ApplicationWindowOptionsKit const & in_options = HPS::ApplicationWindowOptionsKit());

	/*! Creates a new Canvas.
	 * \param in_name An optional name associated with this Canvas.
	 * \param in_options An StandAloneWindowOptionsKit used to specify the driver to be used by the canvas, and whether it will support anti-aliasing. 
	 * \return The newly created Canvas. */
	static Canvas			CreateCanvas(char const * in_name = "", HPS::StandAloneWindowOptionsKit const & in_options = HPS::StandAloneWindowOptionsKit());

	/*! Creates a new Canvas.
	 * \param in_window_key The window key associated with this Canvas. 
	 * \param in_portfolio_key Key to the portfolio to be associated with this Canvas.
	 * \param in_name An optional name associated with this Canvas.
	 * \return The newly created Canvas. */
	static Canvas			CreateCanvas(HPS::WindowKey const & in_window_key, HPS::PortfolioKey const & in_portfolio_key = HPS::PortfolioKey(), char const * in_name = "");

	/*! Creates a new Layout.
	 * \param in_name An optional name associated with this Layout.
	 * \return The newly created Layout. */
	static Layout			CreateLayout(char const * in_name = "");

	/*! Creates a new View.
	 * \param in_name An optional name associated with this View.
	 * \return The newly created View. */
	static View				CreateView(char const * in_name = "");

	/*! Creates a new Model.
	 * \param in_name An optional name associated with this Model.
	 * \return The newly created Model. */
	static Model			CreateModel(char const * in_name = "");

	static Component		CreateComponent(HPS::Component const & in_owner, char const * in_name = "");

private:
	Factory() {}
};

/*! The AxisTriadOperator class defines an operator which allows the user to interact with the axis triad.
 *  This Operator works for both mouse- and touch-driven devices.
 *  Mouse-Driver Devices:
 *		Left Double Click:							Double clicking on an axis snaps the view to that plane
 *		Left Button Down and Move:					Rotates the scene
 *  Touch-Driven Devices:
 *		Double Tap:									Double tapping on an axis snaps the view to that plane
 *		Tap and Move:								Rotates the scene
 *  */
class SPRK_API AxisTriadOperator : public Operator
{
public:
	AxisTriadOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_AxisTriadOperator"; }

	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was pressed. 
	 *  This function starts the operator and computes the position of the starting point.
	 *  If a double-click is received the view snaps to the appropriate plane.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseDown(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals a mouse button was released. 
	 *  This function stops the operator.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseUp(MouseState const & in_state);
	/*! This function is called whenever HPS receives a MouseEvent that signals the mouse moved
	 *  This function orbits the camera.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 *  This function starts the operator and snaps the camera to a plane, if a double tap was received.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool            OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function orbits the camera as needed.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

	virtual void			OnViewAttached();
	virtual void			OnViewDetached();
	
private:

	bool					AxisOrbit(HPS::WindowPoint const & in_loc);
	void					Transition(HPS::SelectionResults selection_results);
	void					TranslatePoint(HPS::WindowPoint * point);
	bool					IsEventRelevant(HPS::Point const & event_location);

	bool					operator_active;
	HPS::WindowPoint		start_point;
	HPS::Vector				start_sphere_point;
	float					axis_subwindow_width;
	float					axis_subwindow_height;
	HPS::TouchID			tracked_touch_ID;
	HPS::Rectangle			axis_subwindow;
	HPS::IntRectangle		axis_subwindow_offsets;
	HPS::SegmentKey			axis_triad_segment;

};

/*! The SmoothTransitionCompleteEvent class is the event that will be triggered when a Smooth Transition ends. */
class SPRK_API SmoothTransitionCompleteEvent: public HPS::Event
{
public:
	/*! The default constructor creates a SmoothTransitionCompleteEvent object. */
	SmoothTransitionCompleteEvent() : Event(0) {}
	virtual ~SmoothTransitionCompleteEvent();

	/*! This constructor creates a SmoothTransitionCompleteEvent object and associates it with a View.
	* 	\param in_view The View that initiated the smooth transition. */
	SmoothTransitionCompleteEvent(HPS::View const & in_view) : view(in_view)
	{ channel = HPS::Object::ClassID<SmoothTransitionCompleteEvent>(); }

	/*! This constructor converts an Event Object to a SmoothTransitionCompleteEvent object.
	 * 	\param in_event The Event Object to be converted. */
	SmoothTransitionCompleteEvent(Event const & in_event) : Event(in_event)
	{
		if (in_event.GetChannel() == Object::ClassID<SmoothTransitionCompleteEvent>())
		{
			auto that = static_cast<SmoothTransitionCompleteEvent const &>(in_event);
			view = that.view;
		}
		else
			throw HPS::InvalidSpecificationException("Invalid Event Type to Cast From.");
	}

	/*! Allocates and returns a copy of this SmoothTransitionCompleteEvent.
	 * 	\return A copy of this SmoothTransitionCompleteEvent. */
	Event * Clone() const
	{
		SmoothTransitionCompleteEvent * new_event = new SmoothTransitionCompleteEvent(*this);
		return new_event;
	}

	/*! Determines if this SmoothTransitionCompleteEvent can be drop in favor of the following event of the same type.
	 * 	\param in_that_event Event to compare with this SmoothTransitionCompleteEvent.
	 * 	\return true is this event can be dropped. */
	virtual bool Drop(Event const * in_that_event) const
	{	
		HPS_UNREFERENCED(in_that_event);
		return false;
	}

	HPS::View					view;
};

/*! The NavigationCubeOperator class defines an operator which allows the user to interact with the navigation cube.
 *  This Operator works for both mouse- and touch-driven devices.
 *  Mouse-Driver Devices:
 *		Left Button Down:							Snap view to plane
 *		Left Button Down and Move:					Rotates the scene
 *  Touch-Driven Devices:
 *		Tap:										Snaps view to plane
 *		Tap and Move:								Rotates the scene
 *  */
class SPRK_API NavigationCubeOperator : public Operator
{
public:
	NavigationCubeOperator(MouseButtons in_mouse_trigger = MouseButtons::ButtonLeft(), ModifierKeys in_modifier_trigger = ModifierKeys());
	~NavigationCubeOperator() { OnViewDetached(); }

	/*! Returns the name of the operator. */
	virtual HPS::UTF8		GetName() const	{ return "HPS_NavigationCubeOperator"; }

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
	 *  This function orbits the camera.
	 * \param in_state A MouseState object describing the current mouse state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnMouseMove(MouseState const & in_state);

	/*! This function is called whenever HPS receives a TouchEvent that signals the device was touched.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchDown(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has been released.
	 *  This function stops the operator.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool            OnTouchUp(TouchState const & in_state);
	/*! This function is called whenever HPS receives a TouchEvent that signals a point of contact has moved.
	 *  This function orbits the camera as needed.
	 * \param in_state A TouchState object describing the current touch state.
	 * \return <span class='code'>true</span> if the input event was handled, <span class='code'>false</span> otherwise. */
	virtual bool			OnTouchMove(TouchState const & in_state);

	virtual void			OnViewAttached();
	virtual void			OnViewDetached();

	virtual void			OnModelAttached();

	void					UpdateHighlightColor();

	/*! Sets a segment as the reference segment. All rotations happen around the reference segment.
	   By default the reference segment is the View segment.
	   /param in_segment the segment to rotate around. */
	void					SetReferenceSegment(HPS::SegmentKey const & in_segment);

	/*! Sets a group of segments as the reference segments. 
	 * All rotations happen around the combined bounding box of the reference segments.
	   By default the reference segment is the View segment.
	   /param in_segments the segments to rotate around. */
	void					SetReferenceSegment(HPS::SegmentKeyArray const & in_segments);

	/*! Returns the segments used as a reference for rotations
	   By default the reference segment is the View segment.
	   /return the segments to rotate around. */
	HPS::SegmentKeyArray	ShowReferenceSegment();

private:
	void					Transition(HPS::SelectionResults selection_results);
	bool					Orbit(HPS::WindowPoint const & in_loc);
	void					TranslatePoint(HPS::WindowPoint * point);
	bool					IsEventRelevant(HPS::Point const & event_location);
	bool					AreCamerasEqual(HPS::CameraKit const & this_camera, HPS::CameraKit const & that_camera);

	bool					operator_active;
	HPS::TouchID			tracked_touch_ID;
	HPS::WindowPoint		start_point;
	HPS::WindowPoint		down_position;
	HPS::Vector				start_sphere_point;
	HPS::SegmentKeyArray	reference_segments;			//rotation and zooming happen around these segments

	HPS::SegmentKey				nav_cube_segment;
	HPS::SegmentKey				style_segment;
	HPS::PortfolioKey			portfolio;
	HPS::ReferenceKeyArrayArray	groups;
	HPS::HighlightOptionsKit	highlight_options_kit;
	HPS::CameraKit				previous_camera;
	int							previous_face;
	bool						suppress_mouse_over_highlights;

	HPS::Rectangle			nav_cube_subwindow;
	HPS::IntRectangle		nav_cube_subwindow_offsets;
	float					nav_cube_subwindow_width;
	float					nav_cube_subwindow_height;

	int						highlighted_group;

	bool					moving;

	class SmoothTransitionCompleteEventHandler : public EventHandler
	{
	public:
		SmoothTransitionCompleteEventHandler(View const & in_view, bool & in_moving)
			: handler_view(in_view), handler_moving(&in_moving) {}

		~SmoothTransitionCompleteEventHandler() { Shutdown(); }

		virtual HandleResult Handle(HPS::Event const * in_event);

	private:
		View				handler_view;
		bool *				handler_moving;
	};

	SmoothTransitionCompleteEventHandler * handler;
};




class SceneTreeItem;
typedef std::shared_ptr<SceneTreeItem> SceneTreeItemPtr;
class SceneTree;
typedef std::shared_ptr<SceneTree> SceneTreePtr;

/*! The SceneTree class is a smart pointer which corresponds to a tree object in a GUI toolkit, e.g., a TreeView in WPF or a CTreeCtrl in MFC.
 *	This class can be derived from to facilitate tree creation in the various GUI toolkits.  It is used to create a scene graph or segment browser. */
class SPRK_API SceneTree : public Sprocket
{
public:
	/*! \enum ItemType
	 *	Enumerates the types of items that can be contained in a SceneTree. */
	enum class ItemType : uint32_t
	{
		None							= 0x00000000,		//!< No type.
		GenericMask						= 0xffff0000,		//!< Mask for the grouping bits in the enum.

		Segment							= 0x00000001,		//!< Item is a SegmentKey, or some object derived from a SegmentKey.
		Include							= 0x00000002,		//!< Item is an IncludeKey.
		ConditionalExpression			= 0x00000003,		//!< Item is a logical conditional expression.  This is the condition associated with a conditional include, reference or style.
		StaticModelSegment				= 0x00000004,		//!< Item is a SegmentKey which corresponds to the shadow tree for the static model.
		AttributeFilter					= 0x00000005,		//!< Item is an attribute filter on an include or style.

		Geometry						= 0x00010000,		//!< Mask for geometry items.
		CuttingSection					= 0x00010001,		//!< Item is a CuttingSectionKey.
		Shell							= 0x00010002,		//!< Item is a ShellKey.
		Mesh							= 0x00010003,		//!< Item is a MeshKey.
		Grid							= 0x00010004,		//!< Item is a GridKey.
		NURBSSurface					= 0x00010005,		//!< Item is a NURBSSurfaceKey.
		Cylinder						= 0x00010006,		//!< Item is a CylinderKey.
		Sphere							= 0x00010007,		//!< Item is a SphereKey.
		Polygon							= 0x00010008,		//!< Item is a PolygonKey.
		Circle							= 0x00010009,		//!< Item is a CircleKey.
		CircularWedge					= 0x0001000a,		//!< Item is a CircularWedgeKey.
		Ellipse							= 0x0001000b,		//!< Item is an EllipseKey.
		Line							= 0x0001000c,		//!< Item is a LineKey.
		NURBSCurve						= 0x0001000d,		//!< Item is a NURBSCurveKey.
		CircularArc						= 0x0001000e,		//!< Item is a CircularArcKey.
		EllipticalArc					= 0x0001000f,		//!< Item is an EllipticalArcKey.
		InfiniteLine					= 0x00010010,		//!< Item is an InfiniteLineKey of type InfiniteLine::Type::Line.
		InfiniteRay						= 0x00010011,		//!< Item is an InfiniteLineKey of type InfiniteLine::Type::Ray.
		Marker							= 0x00010012,		//!< Item is a MarkerKey.
		Text							= 0x00010013,		//!< Item is a TextKey.
		Reference						= 0x00010014,		//!< Item is a ReferenceKey.
		DistantLight					= 0x00010015,		//!< Item is a DistantLightKey.
		Spotlight						= 0x00010016,		//!< Item is a SpotlightKey.

		Attribute						= 0x00020000,		//!< Mask for attribute items.
		Portfolio						= 0x00020001,		//!< Item is a PortfolioKey that has been pushed onto a SegmentKey.
		SegmentStyle					= 0x00020002,		//!< Item is a StyleKey for a SegmentKey that was styled onto a SegmentKey.
		NamedStyle						= 0x00020003,		//!< Item is a StyleKey for a name of a NamedStyleDefinition that was styled onto a SegmentKey.
		MaterialPalette					= 0x00020004,		//!< Item is a MaterialPalette on a SegementKey.
		Priority						= 0x00020005,		//!< Item is a Priority on a SegmentKey.
		Material						= 0x00020006,		//!< Item is a MaterialMappingKit on a SegmentKey.
		Camera							= 0x00020007,		//!< Item is a CameraKit on a SegmentKey.
		ModellingMatrix					= 0x00020008,		//!< Item is a modelling matrix on a SegmentKey.
		UserData						= 0x00020009,		//!< Item is user data on a SegmentKey.
		TextureMatrix					= 0x0002000a,		//!< Item is a texture matrix on a SegmentKey.
		Culling							= 0x0002000b,		//!< Item is a CullingKit on a SegmentKey.
		CurveAttribute					= 0x0002000c,		//!< Item is a CurveAttributeKit on a SegmentKey.
		CylinderAttribute				= 0x0002000d,		//!< Item is a CylinderAttributeKit on a SegmentKey.
		EdgeAttribute					= 0x0002000e,		//!< Item is an EdgeAttributeKit on a SegmentKey.
		LightingAttribute				= 0x0002000f,		//!< Item is a LightingAttributeKit on a SegmentKey.
		LineAttribute					= 0x00020010,		//!< Item is a LineAttributeKit on a SegmentKey.
		MarkerAttribute					= 0x00020011,		//!< Item is a MarkerAttributeKit on a SegmentKey.
		SurfaceAttribute				= 0x00020012,		//!< Item is a SurfaceAttributeKit on a SegmentKey.
		Selectability					= 0x00020013,		//!< Item is a SelectabilityKit on a SegmentKey.
		SphereAttribute					= 0x00020014,		//!< Item is a SphereAttributeKit on a SegmentKey.
		Subwindow						= 0x00020015,		//!< Item is a SubwindowKit on a SegmentKey.
		TextAttribute					= 0x00020016,		//!< Item is a TextAttributeKit on a SegmentKey.
		Transparency					= 0x00020017,		//!< Item is a TransparencyKit on a SegmentKey.
		Visibility						= 0x00020018,		//!< Item is a VisibilityKit on a SegmentKey.
		VisualEffects					= 0x00020019,		//!< Item is a VisualEffectsKit on a SegmentKey.
		Performance						= 0x00020020,		//!< Item is a PerformanceKit on a SegmentKey.
		DrawingAttribute				= 0x00020021,		//!< Item is a DrawingAttributeKit on a SegmentKey.
		HiddenLineAttribute				= 0x00020022,		//!< Item is a HiddenLineAttributeKit on a SegmentKey.
		ContourLine						= 0x00020023,		//!< Item is a ContourLineKit on a SegmentKey.
		Condition						= 0x00020024,		//!< Item is a Condition on a SegmentKey.
		Bounding						= 0x00020025,		//!< Item is a BoundingKit on a SegmentKey.
		AttributeLock					= 0x00020026,		//!< Item is a AttributeLockKit on a SegmentKey.
		TransformMask					= 0x00020027,		//!< Item is a TransformMaskKit on a SegmentKey.
		ColorInterpolation				= 0x00020028,		//!< Item is a ColorInterpolationKit on a SegmentKey.
		CuttingSectionAttribute			= 0x00020029,		//!< Item is a CuttingSectionAttributeKit on a SegmentKey.

		// window only attributes
		Debugging						= 0x00020030,		//!< Item is a DebuggingKit on a WindowKey.
		PostProcessEffects				= 0x00020031,		//!< Item is a PostProcessEffectsKit on a WindowKey.
		SelectionOptions				= 0x00020032,		//!< Item is a SelectionOptionsKit on a WindowKey.
		UpdateOptions					= 0x00020033,		//!< Item is an UpdateOptionsKit on a WindowKey.

		Definition						= 0x00040000,		//!< Mask for definition items.
		NamedStyleDefinition			= 0x00040001,		//!< Item is a NamedStyleDefintion in a PortfolioKey.
		TextureDefinition				= 0x00040002,		//!< Item is a TextureDefinition in a PortfolioKey.
		LinePatternDefinition			= 0x00040003,		//!< Item is a LinePatternDefinition in a PortfolioKey.
		GlyphDefinition					= 0x00040004,		//!< Item is a GlyphDefinition in a PortfolioKey.
		CubeMapDefinition				= 0x00040005,		//!< Item is a CubeMapDefinition in a PortfolioKey.
		ImageDefinition					= 0x00040006,		//!< Item is a ImageDefinition in a PortfolioKey.
		MaterialPaletteDefinition		= 0x00040007,		//!< Item is a MaterialPaletteDefinition in a PortfolioKey.
		ShaderDefinition				= 0x00040008,		//!< Item is a ShaderDefinition in a PortfolioKey.
		ShapeDefinition					= 0x00040009,		//!< Item is a ShapeDefinition in a PortfolioKey.

		Group							= 0x00080000,		//!< Mask for group items.
		SegmentGroup					= 0x00080100,		//!< Item is a collection of SegmentKey objects.
		GeometryGroup					= 0x00080200,		//!< Item is a collection of specific geometry type groups.
		AttributeGroup					= 0x00080300,		//!< Item is a collection of attributes.
		PortfolioGroup					= 0x00080400,		//!< Item is a collection of PortfolioKey objects.
		StyleGroup						= 0x00080500,		//!< Item is a collection of StyleKey objects.
		IncludeGroup					= 0x00080600,		//!< Item is a collection of IncludeKey objects.
		DefinitionGroup					= 0x00180000,		//!< Mask for definition group items.
		NamedStyleDefinitionGroup		= 0x00180700,		//!< Item is a collection of NamedStyleDefinition objects.
		TextureDefinitionGroup			= 0x00180800,		//!< Item is a collection of TextureDefinition objects.
		LinePatternDefinitionGroup		= 0x00180900,		//!< Item is a collection of LinePatternDefinition objects.
		GlyphDefinitionGroup			= 0x00180a00,		//!< Item is a collection of GlyphDefinition objects.
		CubeMapDefinitionGroup			= 0x00180b00,		//!< Item is a collection of CubeMapDefinition objects.
		ImageDefinitionGroup			= 0x00180c00,		//!< Item is a collection of ImageDefinition objects.
		MaterialPaletteDefinitionGroup	= 0x00180d00,		//!< Item is a collection of MaterialPaletteDefinition objects.
		ShaderDefinitionGroup			= 0x00180e00,		//!< Item is a collection of ShaderDefinition objects.
		ShapeDefinitionGroup			= 0x00180f00,		//!< Item is a collection of ShapeDefinition objects.
		CuttingSectionGroup				= 0x00080001,		//!< Item is a collection of CuttingSectionKey objects.
		ShellGroup						= 0x00080002,		//!< Item is a collection of ShellKey objects.
		MeshGroup						= 0x00080003,		//!< Item is a collection of MeshKey objects.
		GridGroup						= 0x00080004,		//!< Item is a collection of GridKey objects.
		NURBSSurfaceGroup				= 0x00080005,		//!< Item is a collection of NURBSSurfaceKey objects.
		CylinderGroup					= 0x00080006,		//!< Item is a collection of CylinderKey objects.
		SphereGroup						= 0x00080007,		//!< Item is a collection of SphereKey objects.
		PolygonGroup					= 0x00080008,		//!< Item is a collection of PolygonKey objects.
		CircleGroup						= 0x00080009,		//!< Item is a collection of CircleKey objects.
		CircularWedgeGroup				= 0x0008000a,		//!< Item is a collection of CircularWedgeKey objects.
		EllipseGroup					= 0x0008000b,		//!< Item is a collection of EllipseKey objects.
		LineGroup						= 0x0008000c,		//!< Item is a collection of LineKey objects.
		NURBSCurveGroup					= 0x0008000d,		//!< Item is a collection of NURBSCurveKey objects.
		CircularArcGroup				= 0x0008000e,		//!< Item is a collection of CircularArcKey objects.
		EllipticalArcGroup				= 0x0008000f,		//!< Item is a collection of EllipticalArcKey objects.
		InfiniteLineGroup				= 0x00080010,		//!< Item is a collection of InfiniteLineKey (of type InfiniteLine::Type::Line) objects.
		InfiniteRayGroup				= 0x00080011,		//!< Item is a collection of InfiniteLineKey (of type InfiniteLine::Type::Ray) objects.
		MarkerGroup						= 0x00080012,		//!< Item is a collection of MarkerKey objects.
		TextGroup						= 0x00080013,		//!< Item is a collection of TextKey objects.
		ReferenceGroup					= 0x00080014,		//!< Item is a collection of ReferenceKey objects.
		DistantLightGroup				= 0x00080015,		//!< Item is a collection of DistantLightKey objects.
		SpotlightGroup					= 0x00080016,		//!< Item is a collection of SpotlightKey objects.
	};

	/*! The default constructor creates an uninitialized SceneTree object.  The Type() function will return Type::None. */
	SceneTree();

	/*! This constructor creates a SceneTree object associated with a Canvas.  This constructor should be used to create a new SceneTree object.
	 *	\param in_canvas The Canvas to associate this SceneTree with. */
	SceneTree(Canvas const & in_canvas);

	/*! The copy constructor creates a SceneTree object that shares the underlying smart-pointer of the source SceneTree.
	 *	\param in_that The source SceneTree to copy. */
	SceneTree(SceneTree const & in_that);

	/*! The move constructor creates a SceneTree by transferring the underlying object of the rvalue reference to this SceneTree.
	 * 	\param in_that An rvalue reference to a SceneTree to take the underlying object from. */
	SceneTree(SceneTree && in_that);

	virtual ~SceneTree();

	virtual HPS::Type ObjectType() const { return HPS::Type::SceneTree; }

	/*! Share the underlying smart-pointer of the SceneTree source.
	 *	\param in_that The SceneTree source of the assignment.
	 *	\return A reference to this SceneTree. */
	SceneTree &				operator=(SceneTree const & in_that);

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this SceneTree.
	 *	\param in_that An rvalue reference to a SceneTree to take the underlying object from.
	 *	\return A reference to this SceneTree. */
	SceneTree &				operator=(SceneTree && in_that);

	/*! Share the underlying smart-pointer of the SceneTree source.
	 *	\param in_that The SceneTree source of the assignment.
	 *	\return A reference to this SceneTree. */
	virtual void			Assign(SceneTree const & in_that);

	/*!	Check if the source SceneTree points to the same underlying impl as this SceneTree.
	 *	\param in_that The source SceneTree to compare to this SceneTree.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					Equals(SceneTree const & in_that) const;

	/*!	Check if the source SceneTree points to a different impl than this SceneTree.
	 *	\param in_that The source SceneTree to compare to this SceneTree.
	 *	\return <span class='code'>true</span> if the objects reference different impls, <span class='code'>false</span> otherwise. */
	bool					operator!=(SceneTree const & in_that) const;

	/*!	Check if the source SceneTree points to the same underlying impl as this SceneTree.
	 *	\param in_that The source SceneTree to compare to this SceneTree.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					operator==(SceneTree const & in_that) const;


	/*!	Sets the highlight options that will be used by this SceneTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as selected when highlights are performed by a HighlightOperator or HighlightAreaOperator.
	 *  Calling this function replaces any highlight options previously set on this object.
	 *	\param in_options The highlight options that will be used by this SceneTree.
	 *	\sa SceneTreeItem::Highlight
	 *	\sa SceneTreeItem::Unhighlight
	 *	\sa HighlightOperator
	 *	\sa HighlightAreaOperator */
	void					SetHighlightOptions(HighlightOptionsKit const & in_options);

	/*!	Sets the highlight options that will be used by this SceneTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as selected when highlights are performed by a HighlightOperator or HighlightAreaOperator.
	 *  Calling this function replaces any highlight options previously set on this object.
	 *  All elements of the array should be unique. In the case of multiple identical elements, only the first unique one will be added.
	 *	\param in_options An array of highlight options that will be used by this SceneTree.
	 *	\sa SceneTreeItem::Highlight
	 *	\sa SceneTreeItem::Unhighlight
	 *	\sa HighlightOperator
	 *	\sa HighlightAreaOperator */
	void					SetHighlightOptions(HighlightOptionsKitArray const & in_options);

	/*!	Sets the highlight options that will be used by this SceneTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as selected when highlights are performed by a HighlightOperator or HighlightAreaOperator.
	 *  Calling this function replaces any highlight options previously set on this object.
	 *  All elements of the array should be unique. In the case of multiple identical elements, only the first unique one will be added.
	 *  \param in_count the number of highlight options kits in the in_options array
	 *	\param in_options An array of highlight options that will be used by this SceneTree.
	 *	\sa SceneTreeItem::Highlight
	 *	\sa SceneTreeItem::Unhighlight
	 *	\sa HighlightOperator
	 *	\sa HighlightAreaOperator */
	void					SetHighlightOptions(size_t in_count, HighlightOptionsKit const in_options []);

	/*!	Appends one highlight options that will be used by this SceneTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as selected when highlights are performed by a HighlightOperator or HighlightAreaOperator.
	 *	\param in_options highlight options that will be used by this SceneTree.
	 *	\sa SceneTreeItem::Highlight
	 *	\sa SceneTreeItem::Unhighlight
	 *	\sa HighlightOperator
	 *	\sa HighlightAreaOperator */
	void					AddHighlightOptions(HighlightOptionsKit const & in_options);

	/*!	Gets the highlight options that will be used by this SceneTree.
	 *	\return The highlight options that will be used by this SceneTree. */
	HighlightOptionsKitArray		GetHighlightOptions() const;


	/*!	Sets the grouping limit for this SceneTree.  The grouping limit determines when to collect certain items into groups of a particular size.  This limit
	 *	is used to keep the number of children for a particular SceneTreeItem constrained to ensure reasonable performance.  Presently this limit is used for
	 *	segments and geometry SceneTreeItem objects.
	 *	\param in_limit The grouping limit for this SceneTree. */
	void					SetGroupingLimit(size_t in_limit);

	/*!	Gets the grouping limit for this SceneTree.
	 *	\return The grouping limit for this SceneTree. */
	size_t					GetGroupingLimit() const;


	/*!	Sets the root SceneTreeItem for this SceneTree.  This object should be created on the heap and it should have been created with a SceneTreeItem constructor which
	 *	took a Model, View, Canvas or Layout, otherwise an exception will be thrown.  Additionally, if there was already an existing root for this SceneTree, the Flush function
	 *	will be invoked prior to setting this as the new root, and Expand will be invoked on the new root.
	 *	\param in_root The root SceneTreeItem for this SceneTree.
	 *	\sa Flush
	 *	\sa SceneTreeItem::Expand */
	void					SetRoot(SceneTreeItemPtr const & in_root);

	/*!	Gets the root SceneTreeItem for this SceneTree.
	 *	\return The root SceneTreeItem for this SceneTree. */
	SceneTreeItemPtr		GetRoot() const;


	/*!	Resets the root SceneTreeItem for this SceneTree.  This will release the root SceneTreeItem by this SceneTree and set the root to a null pointer.  If this function
	 *	is overridden, the overriding function should invoke this base function. */
	virtual void			Flush();

	/*! Whether the SceneTree should handle highlighting events. 
	 *  This include expanding the tree to the highlighted SceneTreeItem, and making the text for the highlighted SceneTreeItem bold.
	 *  By default the SceneTree will handle highlight events.
	 * \param in_state Whether to handle highlight events.*/
	void					SetHighlightEventHandling(bool in_state);

	/*! Whether this SceneTree is currently handling highlight events
	*   This include expanding the tree to the highlighted SceneTreeItem, and making the text for the highlighted SceneTreeItem bold.
	*	\return Whether this SceneTreeItem is currently handling highlight events. */
	bool					GetHighlightEventHandling() const;

	/*!	Collapses the root SceneTreeItem for this SceneTree and then re-expands the tree.  It will try to preserve the original expansion state, however, if the underlying scene
	*	structure has changed, the resulting expansion may look different.*/
	void					ReExpand();
};


/*! The SceneTreeItem class is a smart pointer which corresponds to an item or node in a tree object in a GUI toolkit, e.g., a TreeViewItem in WPF or an HTREEITEM in MFC.
 *	This class must be derived from to facilitate tree creation in the various GUI toolkits.  It is used to represent items in a scene graph or segment browser. */
class SPRK_API SceneTreeItem : public Sprocket
{
public:
	/*! The default constructor creates an uninitialized SceneTreeItem object.  The Type() function will return Type::None. */
	SceneTreeItem();

	/*! The copy constructor creates a SceneTreeItem object that shares the underlying smart-pointer of the source SceneTreeItem.
	 *	\param in_that The source SceneTreeItem to copy. */
	SceneTreeItem(SceneTreeItem const & in_that);

	/*! The move constructor creates a SceneTreeItem by transferring the underlying object of the rvalue reference to this SceneTreeItem.
	 * 	\param in_that An rvalue reference to a SceneTreeItem to take the underlying object from. */
	SceneTreeItem(SceneTreeItem && in_that);

	/*! This constructor creates a SceneTreeItem suitable for acting as a root in a SceneTree.  This root would start at the Model.  The Model provided to this constructor
	 *	should be accessible from the Canvas associated with the SceneTree, otherwise an exception will be thrown.
	 *	\param in_tree The SceneTree with which to associate this new root SceneTreeItem.
	 *	\param in_model The Model for this root SceneTreeItem.
	 *	\sa SceneTree::SetRoot */
	SceneTreeItem(SceneTreePtr const & in_tree, Model const & in_model);

	/*! This constructor creates a SceneTreeItem suitable for acting as a root in a SceneTree.  This root would start at the View.  The View provided to this constructor
	 *	should be accessible from the Canvas associated with the SceneTree, otherwise an exception will be thrown.
	 *	\param in_tree The SceneTree with which to associate this new root SceneTreeItem.
	 *	\param in_view The View for this root SceneTreeItem.
	 *	\sa SceneTree::SetRoot */
	SceneTreeItem(SceneTreePtr const & in_tree, View const & in_view);

	/*! This constructor creates a SceneTreeItem suitable for acting as a root in a SceneTree.  This root would start at the Layout.  The Layout provided to this constructor
	 *	should be accessible from the Canvas associated with the SceneTree, otherwise an exception will be thrown.
	 *	\param in_tree The SceneTree with which to associate this new root SceneTreeItem.
	 *	\param in_layout The Layout for this root SceneTreeItem.
	 *	\sa SceneTree::SetRoot */
	SceneTreeItem(SceneTreePtr const & in_tree, Layout const & in_layout);

	/*! This constructor creates a SceneTreeItem suitable for acting as a root in a SceneTree.  This root would start at the Canvas.  The Canvas provided to this constructor
	 *	should be the same as the Canvas associated with the SceneTree, otherwise an exception will be thrown.
	 *	\param in_tree The SceneTree with which to associate this new root SceneTreeItem.
	 *	\param in_canvas The Canvas for this root SceneTreeItem.
	 *	\sa SceneTree::SetRoot */
	SceneTreeItem(SceneTreePtr const & in_tree, Canvas const & in_canvas);

	/*! This constructor creates a SceneTreeItem which is not suitable for acting as a root in a SceneTree, but is suitable to be a SceneTreeItem under the root of the SceneTree.
	 *	This constructor should be the one invoked in the overridden AddChild function.
	 *	\param in_tree The SceneTree with which to associate this new SceneTreeItem.
	 *	\param in_key The Key for this SceneTreeItem.  This value should come from the corresponding argument to AddChild.
	 *	\param in_type The type of this SceneTreeItem.  This value should come from the corresponding argument to AddChild.
	 *	\param in_title The title for this SceneTreeItem.  This value should come from the corresponding argument to AddChild.
	 *	\sa AddChild */
	SceneTreeItem(SceneTreePtr const & in_tree, Key const & in_key, SceneTree::ItemType in_type, char const * in_title = nullptr);

	virtual ~SceneTreeItem();

	HPS::Type ObjectType() const { return HPS::Type::SceneTreeItem; }

	/*! Share the underlying smart-pointer of the SceneTreeItem source.
	 *	\param in_that The SceneTreeItem source of the assignment.
	 *	\return A reference to this SceneTreeItem. */
	SceneTreeItem &				operator=(SceneTreeItem const & in_that);

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this SceneTreeItem.
	 *	\param in_that An rvalue reference to a SceneTreeItem to take the underlying object from.
	 *	\return A reference to this SceneTreeItem. */
	SceneTreeItem &				operator=(SceneTreeItem && in_that);

	/*! Share the underlying smart-pointer of the SceneTreeItem source.
	 *	\param in_that The SceneTreeItem source of the assignment.
	 *	\return A reference to this SceneTreeItem. */
	virtual void				Assign(SceneTreeItem const & in_that);

	/*!	Check if the source SceneTreeItem points to the same underlying impl as this SceneTreeItem.
	 *	\param in_that The source SceneTreeItem to compare to this SceneTreeItem.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool						Equals(SceneTreeItem const & in_that) const;

	/*!	Check if the source SceneTreeItem points to a different impl than this SceneTreeItem.
	 *	\param in_that The source SceneTreeItem to compare to this SceneTreeItem.
	 *	\return <span class='code'>true</span> if the objects reference different impls, <span class='code'>false</span> otherwise. */
	bool						operator!=(SceneTreeItem const & in_that) const;

	/*!	Check if the source SceneTreeItem points to the same underlying impl as this SceneTreeItem.
	 *	\param in_that The source SceneTreeItem to compare to this SceneTreeItem.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool						operator==(SceneTreeItem const & in_that) const;


	/*!	Gets the SceneTree associated with this SceneTreeItem.  This provides a means of accessing the derived SceneTree by this SceneTreeItem if necessary.
	 *	\return The SceneTree associated with this SceneTreeItem. */
	SceneTreePtr				GetTree() const;

	/*!	Gets the title for this SceneTreeItem.  This is the text that should be displayed for this SceneTreeItem in the SceneTree.
	 *	\return The title for this SceneTreeItem. */
	UTF8						GetTitle() const;

	/*!	Gets the item type for this SceneTreeItem.  This is used to determine what type of data this SceneTreeItem represents in the SceneTree.
	 *	\return The item type for this SceneTreeItem. */
	SceneTree::ItemType			GetItemType() const;

	/*! Indicates whether this SceneTreeItem has the given ItemType mask.
	 *	\param in_mask The ItemType mask to check against this SceneTreeItem.
	 *	\return <span class='code'>true</span> if this SceneTreeItem has the given ItemType mask, <span class='code'>false</span> otherwise. */
	bool						HasItemType(SceneTree::ItemType in_mask) const;

	/*!	Gets the key for this SceneTreeItem.  The type of the key depends on the SceneTree::ItemType for this SceneTreeItem.
	 *	\return The key for this SceneTreeItem. */
	Key							GetKey() const;

	/*!	Gets the key path for this SceneTreeItem.
	 *	\return The key path for this SceneTreeItem. */
	KeyPath						GetKeyPath() const;

	/*!	Indicates whether this SceneTreeItem has any children.
	 *	\return Whether this SceneTreeItem has any children. */
	bool						HasChildren() const;


	/*!	This function is used to create a new SceneTreeItem to be the child of this SceneTreeItem.  This function will be invoked if this SceneTreeItem has children
	 *	and Expand has been invoked on it.  This function should contain any necessary calls to GUI toolkits to create the appropriate GUI elements, e.g., see the implementations
	 *	in the mfc_sandbox or wpf_sandbox.  Additionally, since this function requires the creation of a derived SceneTreeItem, that derived SceneTreeItem should use the base
	 *	constructor which takes the same arguments this function.
	 *	\param in_key The Key for this SceneTreeItem.  Should be passed to as the corresponding argument to the base constructor.
	 *	\param in_type The type for this SceneTreeItem.  Should be passed to as the corresponding argument to the base constructor.
	 *	\param in_title The title for this SceneTreeItem.  Should be passed to as the corresponding argument to the base constructor.
	 *	\return A newly created object derived from SceneTreeItem. */
	virtual SceneTreeItemPtr	AddChild(Key const & in_key, SceneTree::ItemType in_type, char const * in_title = nullptr) = 0;


	/*!	This function should be invoked when this SceneTreeItem needs to expand.  If this function is overridden, the overriding function should invoke this base function.
	 *	Additionally, this function will always repopulate the children for this SceneTreeItem when invoked. */
	virtual void				Expand();

	/*!	This function should be invoked when this SceneTreeItem needs to collapse.  If this function is overridden, the overriding function should invoke this base function.
	 *	Additionally, this function will always release all children for this SceneTreeItem when invoked. */
	virtual void				Collapse();


	/*! Indicates whether this SceneTreeItem is currently marked as selected.
	 *	\return Whether this SceneTreeItem is currently marked as selected.
	 *	\sa Select
	 *	\sa Unselect */
	bool						IsSelected() const;

	/*!	This function will be invoked when this SceneTreeItem should be marked as selected.  Marking an item as selected means that the key backing this SceneTreeItem has been highlighted
	 *	with the options specified in the associated SceneTree or is along a path in which a highlight has occurred.  This highlighting may have come as a result of the Highlight function
	 *	on a SceneTreeItem being invoked (not necessarily this SceneTreeItem), or potentially due to the HighlightOperator or HighlightAreaOperator highlighting the key backing this SceneTreeItem
	 *	or highlighting some path which includes the key backing this SceneTreeItem. This function is meant to be overridden to allow one to change the appearance of a selected SceneTreeItem,
	 *	e.g., by bolding the text or changing an icon, to signify this new state. If this function is overridden, the overriding function should invoke this base function. */
	virtual void				Select();

	/*!	This function will be invoked when this SceneTreeItem should be marked as unselected.  Marking an item as unselected means that the key backing this SceneTreeItem is no longer highlighted
	 *	with the options specified in the associated SceneTree or is no longer along a path in which a highlight has occurred.  This unhighlighting may have come as a result of the Unhighlight function
	 *	on a SceneTreeItem being invoked (not necessarily this SceneTreeItem), or potentially due to the HighlightOperator or HighlightAreaOperator unhighlighting the key backing this SceneTreeItem.
	 *	or unhighlighting some path which includes the key backing this SceneTreeItem. This function is meant to be overridden to allow one to change the appearance of a selected SceneTreeItem,
	 *	e.g., by no longer bolding the text or changing an icon, to signify this new state. If this function is overridden, the overriding function should invoke this base function. */
	virtual void				Unselect();

	/*! Indicates whether this SceneTreeItem is able to be highlighted.  Some items, like those in static trees, cannot be highlighted.
	 *	\return Whether this SceneTreeItem is highlightable.
	 *	\sa Highlight
	 *	\sa IsHighlighted */
	bool						IsHighlightable() const;

	/*! Indicates whether this SceneTreeItem is currently highlighted with the options specified in the associated SceneTree.  This may mean that the Highlight function on this SceneTreeItem
	 *	was invoked, or that the key backing this SceneTreeItem was highlighted by the HighlightOperator or HighlightAreaOperator.
	 *	\return Whether this SceneTreeItem is currently highlighted with the options specified in the associated SceneTree.
	 *	\sa Highlight
	 *	\sa Unhighlight */
	bool						IsHighlighted() const;

	/*!	This function will cause this SceneTreeItem to be highlighted with the options specified in the associated SceneTree.  This will cause elements along the path to this item (and this item itself)
	 *	to be marked as selected if they aren't already.  Depending on the type of the SceneTreeItem, sometimes a highlight is nonsensical.  In this case, no highlight will be performed.
	 *	\param in_higlight_options_index The index in the highlight options array held by the associated SceneTree. Defaults to 0.
	 *	\sa Select */
	void						Highlight(size_t in_highlight_options_index = 0);

	/*!	This function will cause this SceneTreeItem to be highlighted with the options specified in the associated SceneTree.  This will cause elements along the path to this item (and this item itself)
	 *	to be marked as selected if they aren't already.  Depending on the type of the SceneTreeItem, sometimes a highlight is nonsensical.  In this case, no highlight will be performed.
	 *  If the options requested is not part of the associated SceneTree highlight options, it will be added to them.
	 *	\param in_highlight_options The highlight options used during highlighting
	 *	\sa Select */
	void						Highlight(HighlightOptionsKit const & in_highlight_options);

	/*!	This function will remove the highlight with the options specified in the associated SceneTree from this SceneTreeItem.  This can cause elements along the path to this item (and this item itself)
	 *	to be marked as unselected unless those items lie along a path which is still highlighted.  Depending on the type of the SceneTreeItem, sometimes an unhighlight is nonsensical.
	 *	In this case, no unhighlight will be performed.
	 *	\param in_higlight_options_index The index in the highlight options array held by the associated SceneTree. Defaults to 0.
	 *	\sa Unselect */
	void						Unhighlight(size_t in_highlight_options_index = 0);

	/*!	This function will remove the highlight with the options specified in the associated SceneTree from this SceneTreeItem.  This can cause elements along the path to this item (and this item itself)
	 *	to be marked as unselected unless those items lie along a path which is still highlighted.  Depending on the type of the SceneTreeItem, sometimes an unhighlight is nonsensical.
	 *	In this case, no unhighlight will be performed.
	 *  If the options requested is not part of the associated SceneTree highlight options, it will be added to them.
	 *	\param in_higlight_options_index The index in the highlight options array held by the associated SceneTree. Defaults to 0.
	 *	\sa Unselect */
	void						Unhighlight(HighlightOptionsKit const & in_highlight_options);

	/*!	Indicates whether this SceneTreeItem is currently expanded.
	*	\return Whether this SceneTreeItem is currently expanded. */
	bool						IsExpanded() const;

	/*!	Collapses this SceneTreeItem and then re-expands the tree.  It will try to preserve the original expansion state, however, if the underlying scene structure has changed, the resulting expansion
	*	may look different. */
	void						ReExpand();
};



class ComponentTreeItem;
typedef std::shared_ptr<ComponentTreeItem> ComponentTreeItemPtr;
class ComponentTree;
typedef std::shared_ptr<ComponentTree> ComponentTreePtr;

/*! The ComponentTree class is a smart pointer which corresponds to a tree object in a GUI toolkit, e.g., a TreeView in WPF or a CTreeCtrl in MFC.
 *	This class can be derived from to facilitate tree creation in the various GUI toolkits.  It is used to create a component or model browser. */
class SPRK_API ComponentTree : public Sprocket
{
public:
	/*! \enum ItemType
	 *	Enumerates the types of items that can be contained in a ComponentTree. */
	enum class ItemType
	{
		None,							//!< No type.
		ExchangeComponent,				//!< Item is an Exchange::Component.
		ExchangeModelFile,				//!< Item is an Exchange::CADModel.

		ExchangeViewGroup,				//!< Item is a collection of Exchange::Capture objects.
		ExchangeAnnotationViewGroup,	//!< Item is a collection of Exchange::Capture objects which are annotation captures.
		ExchangePMIGroup,				//!< Item is a collection of Exchange::Component objects representing PMI.
		ExchangeModelGroup,				//!< Item is a collection of root Exchange::Component objects.

		ParasolidComponent,
		ParasolidModelFile,

		DWGComponent,
		DWGModelFile,
	};

	/*! The default constructor creates an uninitialized ComponentTree object.  The Type() function will return Type::None. */
	ComponentTree();

	/*! This constructor creates a ComponentTree object associated with a Canvas and a layer.  This constructor should be used to create a new ComponentTree object.
	 *	\param in_canvas The Canvas to associate this ComponentTree with.
	 *	\param in_layer The layer in the Canvas to associate this ComponentTree with. */
	ComponentTree(Canvas const & in_canvas, size_t in_layer = 0);

	/*! The copy constructor creates a ComponentTree object that shares the underlying smart-pointer of the source ComponentTree.
	 *	\param in_that The source ComponentTree to copy. */
	ComponentTree(ComponentTree const & in_that);

	/*! The move constructor creates a ComponentTree by transferring the underlying object of the rvalue reference to this ComponentTree.
	 * 	\param in_that An rvalue reference to a ComponentTree to take the underlying object from. */
	ComponentTree(ComponentTree && in_that);

	virtual ~ComponentTree();

	virtual HPS::Type ObjectType() const { return HPS::Type::ComponentTree; }

	/*! Share the underlying smart-pointer of the ComponentTree source.
	 *	\param in_that The ComponentTree source of the assignment.
	 *	\return A reference to this ComponentTree. */
	ComponentTree &			operator=(ComponentTree const & in_that);

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this ComponentTree.
	 *	\param in_that An rvalue reference to a ComponentTree to take the underlying object from.
	 *	\return A reference to this ComponentTree. */
	ComponentTree &			operator=(ComponentTree && in_that);

	/*! Share the underlying smart-pointer of the ComponentTree source.
	 *	\param in_that The ComponentTree source of the assignment.
	 *	\return A reference to this ComponentTree. */
	virtual void			Assign(ComponentTree const & in_that);

	/*!	Check if the source ComponentTree points to the same underlying impl as this ComponentTree.
	 *	\param in_that The source ComponentTree to compare to this ComponentTree.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					Equals(ComponentTree const & in_that) const;

	/*!	Check if the source ComponentTree points to a different impl than this ComponentTree.
	 *	\param in_that The source ComponentTree to compare to this ComponentTree.
	 *	\return <span class='code'>true</span> if the objects reference different impls, <span class='code'>false</span> otherwise. */
	bool					operator!=(ComponentTree const & in_that) const;

	/*!	Check if the source ComponentTree points to the same underlying impl as this ComponentTree.
	 *	\param in_that The source ComponentTree to compare to this ComponentTree.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool					operator==(ComponentTree const & in_that) const;


	/*!	Sets the highlight options that will be used by this ComponentTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as highlighted when highlights are performed by ComponentPath::Highlight.
	 *  Calling this function replaces any highlight options previously set on this object.
	 *	\param in_options The highlight options that will be used by this ComponentTree.
	 *	\sa ComponentTreeItem::Highlight
	 *	\sa ComponentTreeItem::Unhighlight
	 *	\sa ComponentPath::Highlight
	 *	\sa ComponentPath::Unhighlight */
	void					SetHighlightOptions(HighlightOptionsKit const & in_options);

	/*!	Sets the highlight options that will be used by this ComponentTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as highlighted when highlights are performed by ComponentPath::Highlight.
	 *  Calling this function replaces any highlight options previously set on this object.
	 *  All elements of the array should be unique. In the case of multiple identical elements, only the first unique one will be added.
	 *	\param in_options An array of highlight options that will be used by this ComponentTree.
	 *	\sa ComponentTreeItem::Highlight
	 *	\sa ComponentTreeItem::Unhighlight
	 *	\sa ComponentPath::Highlight
	 *	\sa ComponentPath::Unhighlight */
	void					SetHighlightOptions(HighlightOptionsKitArray const & in_options);

	/*!	Sets the highlight options that will be used by this ComponentTree.  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as highlighted when highlights are performed by ComponentPath::Highlight.
	 *  Calling this function replaces any highlight options previously set on this object.
	 *  All elements of the array should be unique. In the case of multiple identical elements, only the first unique one will be added.
	 *  \param in_count the number of highlight options kits in the in_options array
	 *	\param in_options An array of highlight options that will be used by this ComponentTree.
	 *	\sa ComponentTreeItem::Highlight
	 *	\sa ComponentTreeItem::Unhighlight
	 *	\sa ComponentPath::Highlight
	 *	\sa ComponentPath::Unhighlight */
	void					SetHighlightOptions(size_t in_count, HighlightOptionsKit const in_options []);

	/*!	Appends one highlight options kit to the array that will be used by this ComponentTree.  
	 *  These options are used to perform highlights of elements within the tree and to
	 *	determine what items within the tree need to be marked as highlighted when highlights are performed by ComponentPath::Highlight.
	 *  If the options kit requested is already part of the ComponentTree highlight options array, it will not be added to it.
	 *	\param in_options The highlight options that will be used by this ComponentTree.
	 *	\sa ComponentTreeItem::Highlight
	 *	\sa ComponentTreeItem::Unhighlight
	 *	\sa ComponentPath::Highlight
	 *	\sa ComponentPath::Unhighlight */
	void					AddHighlightOptions(HighlightOptionsKit const & in_options);

	/*!	Gets the highlight options that will be used by this ComponentTree.
	 *	\return An array of highlight options that will be used by this ComponentTree. */
	HighlightOptionsKitArray		GetHighlightOptions() const;


	/*!	Sets the root ComponentTreeItem for this ComponentTree.  This object should be created on the heap and it should have been created with a ComponentTreeItem constructor which
	 *	took a CADModel, otherwise an exception will be thrown.  Additionally, if there was already an existing root for this ComponentTree, the Flush function
	 *	will be invoked prior to setting this as the new root, and Expand will be invoked on the new root.
	 *	\param in_root The root ComponentTreeItem for this ComponentTree.
	 *	\sa Flush
	 *	\sa ComponentTreeItem::Expand */
	void					SetRoot(ComponentTreeItemPtr const & in_root);

	/*!	Gets the root ComponentTreeItem for this ComponentTree.
	 *	\return The root ComponentTreeItem for this ComponentTree. */
	ComponentTreeItemPtr	GetRoot() const;


	/*!	Resets the root ComponentTreeItem for this ComponentTree.  This will release the root ComponentTreeItem by this ComponentTree and set the root to a null pointer.  If this function
	 *	is overridden, the overriding function should invoke this base function. */
	virtual void			Flush();

	/*!	Collapses the root ComponentTreeItem for this ComponentTree and then re-expands the tree.  It will try to preserve the original expansion state, however, if the underlying component
	 *	structure has changed, the resulting expansion may look different.*/
	void					ReExpand();

	/*! Whether the ComponentTree should handle highlighting events. 
	 *  This include expanding the tree to the highlighted components, and making the text for the highlighted component tree items bold.
	 *  By default the ComponentTree will handle highlight events.
	 * \param in_state Whether to handle highlight events.*/
	void					SetHighlightEventHandling(bool in_state);

	/*! Whether this ComponentTree is currently handling highlight events
	*   This include expanding the tree to the highlighted components, and making the text for the highlighted component tree items bold.
	*	\return Whether this ComponentTree is currently handling highlight events. */
	bool					GetHighlightEventHandling() const;
};


/*! The ComponentTreeItem class is a smart pointer which corresponds to an item or node in a tree object in a GUI toolkit, e.g., a TreeViewItem in WPF or an HTREEITEM in MFC.
 *	This class must be derived from to facilitate tree creation in the various GUI toolkits.  It is used to represent items in a component or model browser. */
class SPRK_API ComponentTreeItem : public Sprocket
{
public:
	/*! The default constructor creates an uninitialized ComponentTreeItem object.  The Type() function will return Type::None. */
	ComponentTreeItem();

	/*! The copy constructor creates a ComponentTreeItem object that shares the underlying smart-pointer of the source ComponentTreeItem.
	 *	\param in_that The source ComponentTreeItem to copy. */
	ComponentTreeItem(ComponentTreeItem const & in_that);

	/*! The move constructor creates a ComponentTreeItem by transferring the underlying object of the rvalue reference to this ComponentTreeItem.
	 * 	\param in_that An rvalue reference to a ComponentTreeItem to take the underlying object from. */
	ComponentTreeItem(ComponentTreeItem && in_that);

	/*! This constructor creates a ComponentTreeItem suitable for acting as a root in a ComponentTree.  The CADModel provided to this constructor
	 *	should be accessible from the Canvas and layer associated with the ComponentTree, otherwise an exception will be thrown.
	 *	\param in_tree The ComponentTree with which to associate this new root ComponentTreeItem.
	 *	\param in_cad_model The CADModel for this root ComponentTreeItem.
	 *	\sa ComponentTree::SetRoot */
	ComponentTreeItem(ComponentTreePtr const & in_tree, CADModel const & in_cad_model);

	/*! This constructor creates a ComponentTreeItem which is not suitable for acting as a root in a ComponentTree, but is suitable to be a ComponentTreeItem under the root of the ComponentTree.
	 *	This constructor should be the one invoked in the overridden AddChild function.
	 *	\param in_tree The ComponentTree with which to associate this new ComponentTreeItem.
	 *	\param in_component The Component for this ComponentTreeItem.  This value should come from the corresponding argument to AddChild.
	 *	\param in_type The type of this ComponentTreeItem.  This value should come from the corresponding argument to AddChild.
	 *	\sa AddChild */
	ComponentTreeItem(ComponentTreePtr const & in_tree, Component const & in_component, ComponentTree::ItemType in_type);

	virtual ~ComponentTreeItem();

	HPS::Type ObjectType() const { return HPS::Type::ComponentTreeItem; }

	/*! Share the underlying smart-pointer of the ComponentTreeItem source.
	 *	\param in_that The ComponentTreeItem source of the assignment.
	 *	\return A reference to this ComponentTreeItem. */
	ComponentTreeItem &				operator=(ComponentTreeItem const & in_that);

	/*! The move assignment operator transfers the underlying object of the rvalue reference to this ComponentTreeItem.
	 *	\param in_that An rvalue reference to a ComponentTreeItem to take the underlying object from.
	 *	\return A reference to this ComponentTreeItem. */
	ComponentTreeItem &				operator=(ComponentTreeItem && in_that);

	/*! Share the underlying smart-pointer of the ComponentTreeItem source.
	 *	\param in_that The ComponentTreeItem source of the assignment.
	 *	\return A reference to this ComponentTreeItem. */
	virtual void					Assign(ComponentTreeItem const & in_that);

	/*!	Check if the source ComponentTreeItem points to the same underlying impl as this ComponentTreeItem.
	 *	\param in_that The source ComponentTreeItem to compare to this ComponentTreeItem.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool							Equals(ComponentTreeItem const & in_that) const;

	/*!	Check if the source ComponentTreeItem points to a different impl than this ComponentTreeItem.
	 *	\param in_that The source ComponentTreeItem to compare to this ComponentTreeItem.
	 *	\return <span class='code'>true</span> if the objects reference different impls, <span class='code'>false</span> otherwise. */
	bool							operator!=(ComponentTreeItem const & in_that) const;

	/*!	Check if the source ComponentTreeItem points to the same underlying impl as this ComponentTreeItem.
	 *	\param in_that The source ComponentTreeItem to compare to this ComponentTreeItem.
	 *	\return <span class='code'>true</span> if the objects reference the same impl, <span class='code'>false</span> otherwise. */
	bool							operator==(ComponentTreeItem const & in_that) const;



	/*!	Gets the ComponentTree associated with this ComponentTreeItem.  This provides a means of accessing the derived ComponentTree by this ComponentTreeItem if necessary.
	 *	\return The ComponentTree associated with this ComponentTreeItem. */
	ComponentTreePtr				GetTree() const;

	/*!	Gets the title for this ComponentTreeItem.  This is the text that should be displayed for this ComponentTreeItem in the ComponentTree.
	 *	\return The title for this ComponentTreeItem. */
	UTF8							GetTitle() const;

	/*!	Gets the item type for this ComponentTreeItem.  This is used to determine what type of data this ComponentTreeItem represents in the ComponentTree.
	 *	\return The item type for this ComponentTreeItem. */
	ComponentTree::ItemType			GetItemType() const;

	/*!	Gets the Component for this ComponentTreeItem.  The type of the Component depends on the ComponentTree::ItemType for this ComponentTreeItem.
	 *	\return The Component for this ComponentTreeItem. */
	Component						GetComponent() const;

	/*!	Indicates whether this ComponentTreeItem has any children.
	 *	\return Whether this ComponentTreeItem has any children. */
	bool							HasChildren() const;

	/*!	Gets the ComponentPath corresponding to this ComponentTreeItem.
	 *	\return The ComponentPath corresponding to this ComponentTreeItem. */
	ComponentPath					GetPath() const;


	/*!	This function is used to create a new ComponentTreeItem to be the child of this ComponentTreeItem.  This function will be invoked if this ComponentTreeItem has children
	 *	and Expand has been invoked on it.  This function should contain any necessary calls to GUI toolkits to create the appropriate GUI elements.
	 *	Additionally, since this function requires the creation of a derived ComponentTreeItem, that derived ComponentTreeItem should use the base
	 *	constructor which takes the same arguments this function.
	 *	\param in_component The Component for this ComponentTreeItem.  Should be passed to as the corresponding argument to the base constructor.
	 *	\param in_type The type for this ComponentTreeItem.  Should be passed to as the corresponding argument to the base constructor.
	 *	\return A newly created object derived from ComponentTreeItem. */
	virtual ComponentTreeItemPtr	AddChild(Component const & in_component, ComponentTree::ItemType in_type) = 0;


	/*!	This function should be invoked when this ComponentTreeItem needs to expand.  If this function is overridden, the overriding function should invoke this base function.
	 *	Additionally, this function will always repopulate the children for this ComponentTreeItem when invoked. */
	virtual void					Expand();

	/*!	This function should be invoked when this ComponentTreeItem needs to collapse.  If this function is overridden, the overriding function should invoke this base function.
	 *	Additionally, this function will always release all children for this ComponentTreeItem when invoked. */
	virtual void					Collapse();


	/*! Indicates whether this ComponentTreeItem is currently highlighted with the options specified in the associated ComponentTree.  This may mean that the Highlight function on this ComponentTreeItem
	 *	was invoked, or that the Component backing this ComponentTreeItem was highlighted by ComponentPath::Highlight.
	 *	\return Whether this ComponentTreeItem is currently highlighted with the options specified in the associated ComponentTree.
	 *	\sa Highlight
	 *	\sa Unhighlight */
	bool							IsHighlighted() const;

	/*!	This function will be invoked when this ComponentTreeItem should be marked as highlighted. This highlighting may have come as a result of the Highlight function on a ComponentTreeItem being invoked,
	 *	or due to ComponentPath::Highlight being invoked on the Component backing this ComponentTreeItem. This function is meant to be overridden to allow one to change the appearance of a
	 *	highlighted ComponentTreeItem, e.g., by bolding the text or changing an icon, to signify this new state. If this function is overridden, the overriding function should invoke this base function.
	 *	\param in_options The HighlightOptionsKit for the associated highlight. */
	virtual void					OnHighlight(HighlightOptionsKit const & in_options);

	/*!	This function will be invoked when this ComponentTreeItem should be marked as unhighlighted. This unhighlighting may have come as a result of the Unhighlight function on a ComponentTreeItem being invoked,
	 *	or due to ComponentPath::Unhighlight being invoked on the Component backing this ComponentTreeItem. This function is meant to be overridden to allow one to change the appearance of a
	 *	unhighlighted ComponentTreeItem, e.g., by no longer bolding the text or changing an icon, to signify this new state. If this function is overridden, the overriding function should invoke this base function.
	 *	\param in_options The HighlightOptionsKit for the associated unhighlight. */
	virtual void					OnUnhighlight(HighlightOptionsKit const & in_options);


	/*!	This function will cause this ComponentTreeItem to be highlighted with the nth option specified in the associated ComponentTree.  This will cause this item
	 *	to be marked as highlighted if it isn't already.  Depending on the type of the ComponentTreeItem, sometimes a highlight is nonsensical.  In this case, no highlight will be performed.
	 *	\param in_higlight_options_index The index in the highlight options array held by the associated ComponentTree. Defaults to 0.
	 *	\sa OnHighlight */
	void							Highlight(size_t in_highlight_options_index = 0);

	/*!	This function will cause this ComponentTreeItem to be highlighted with the options specified.
	 *  If the options requested is not part of the associated ComponentTree highlight options, it will be added to them. This will cause this item
	 *	to be marked as highlighted if it isn't already.  Depending on the type of the ComponentTreeItem, sometimes a highlight is nonsensical.  In this case, no highlight will be performed.
	 *	\param in_highlight_options The highlight options used during highlighting
	 *	\sa OnHighlight */
	void							Highlight(HighlightOptionsKit const & in_highlight_options);

	/*!	This function will cause this ComponentTreeItem to be unhighlighted with the nth option specified in the associated ComponentTree.  This will cause this item
	 *	to be marked as unhighlighted if it isn't already.  Depending on the type of the ComponentTreeItem, sometimes an unhighlight is nonsensical.  In this case, no unhighlight will be performed.
	 *	\param in_higlight_options_index The index in the highlight options array held by the associated ComponentTree. Defaults to 0.
	 *	\sa OnUnhighlight */
	void							Unhighlight(size_t in_highlight_options_index = 0);

	/*!	This function will cause this ComponentTreeItem to be unhighlighted with the options specified.
	 *  If the options requested is not part of the associated ComponentTree highlight options, it will be added to them. This will cause this item
	 *	to be marked as unhighlighted if it isn't already.  Depending on the type of the ComponentTreeItem, sometimes a highlight is nonsensical.  In this case, no unhighlight will be performed.
	 *	\param in_highlight_options The highlight options used during unhighlighting
	 *	\sa OnUnhighlight */
	void							Unhighlight(HighlightOptionsKit const & in_highlight_options);


	/*! Indicates whether this ComponentTreeItem is currently hidden.  This may mean that the Hide function on this ComponentTreeItem was invoked, or that the Component backing this ComponentTreeItem was
	 *	hidden via ComponentPath::Hide or ComponentPath::Isolate.
	 *	\return Whether this ComponentTreeItem is currently hidden.
	 *	\sa Hide
	 *	\sa Show
	 *	\sa Isolate
	 *	\sa ComponentPath::Hide
	 *	\sa ComponentPath::Show
	 *	\sa ComponentPath::Isolate */
	bool							IsHidden() const;

	/*!	This function will be invoked when this ComponentTreeItem should be marked as hidden. This hiding may have come as a result of the Hide or Isolate function on a ComponentTreeItem being invoked,
	 *	or due to ComponentPath::Hide being invoked on the Component backing this ComponentTreeItem. This function is meant to be overridden to allow one to change the appearance of a
	 *	hidden ComponentTreeItem, e.g., by graying the text or changing an icon, to signify this new state. If this function is overridden, the overriding function should invoke this base function. */
	virtual void					OnHide();

	/*!	This function will be invoked when this ComponentTreeItem should be marked as visible. This visibility may have come as a result of the Show or Isolate function on a ComponentTreeItem being invoked,
	 *	or due to ComponentPath::Show or ComponentPath::Isolate being invoked on the Component backing this ComponentTreeItem. This function is meant to be overridden to allow one to change the appearance of a
	 *	visible ComponentTreeItem, e.g., by no longer graying out the text or changing an icon, to signify this new state. If this function is overridden, the overriding function should invoke this base function. */
	virtual void					OnShow();

	/*!	This function will cause this ComponentTreeItem to be hidden if it isn't already.  This will cause this item to be marked as hidden if it isn't already.  Depending on the type of the ComponentTreeItem,
	 *	sometimes a hide is nonsensical.  In this case, no hide will be performed.
	 *	\sa OnHide */
	void							Hide();

	/*!	This function will cause this ComponentTreeItem to be visible if it isn't already.  This will cause this item to be marked as visible if it isn't already.  Depending on the type of the ComponentTreeItem,
	 *	sometimes a show is nonsensical.  In this case, no show will be performed.
	 *	\sa OnShow */
	void							Show();

	/*!	This function will cause this ComponentTreeItem to be isolated, i.e., all other items will be hidden and this item will be shown.  This will cause this item to be marked as visible if it isn't already,
	 *	and all other items to be marked as hidden. Depending on the type of the ComponentTreeItem, sometimes an isolate is nonsensical.  In this case, no isolate will be performed.
	 *	\sa OnHide
	 *	\sa OnShow */
	void							Isolate();

	/*!	Indicates whether this ComponentTreeItem is currently expanded.
	 *	\return Whether this ComponentTreeItem is currently expanded. */
	bool							IsExpanded() const;

	/*!	Collapses this ComponentTreeItem and then re-expands the tree.  It will try to preserve the original expansion state, however, if the underlying component structure has changed, the resulting expansion
	 *	may look different. */
	void							ReExpand();
};




}
#endif



