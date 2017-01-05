/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_BRIDGE_H_
#define __HPS_BRIDGE_H_
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "base.hxx"
#include "dcl_hps.h"
#include "lists.hxx"
#include "api.hxx"
#include "param.hxx"
#include "idx_mesh.hxx"
#include "LinkedMesh.hxx"
#include "hps_util.h"
#include "hps_rend_options.h"
#include "spa_progress_info.hxx"
#include "rgbcolor.hxx"

class ENTITY;
class CURVE;
class SPAtransf;
class rgb_color;
class hps_rendering_options;
class SPApoint_cloud; // deprecated please consider using SPAposition_cloud
class SPAposition_cloud;
class facet_options;
class IEntityConverter;

/**
 * @file hps_bridge.h
 * @CAA2Level L1
 * @CAA2Usage U1
 * \defgroup HPSBRIDGEAPI HPS/ACIS Bridge
 * \brief Declared at <hps_bridge.h>
 * \ingroup VISMODULE
 * @{
 */
/**
 * Edge Geometry Type
 */
#define HPS_GEOMETRY_TYPE_edges 2
/**
 * Face Geometry Type
 */
#define HPS_GEOMETRY_TYPE_faces 4
/**
 * Body Geometry Type
 */
#define HPS_GEOMETRY_TYPE_bodies 8
/**
* Vertex Geometry Type
*/
#define HPS_GEOMETRY_TYPE_vertices	16
/**
 * Merge Faces Type
 */
#define HPS_MERGE_FACES 2
/**
 * Merge Bodies Type
 */
#define HPS_MERGE_BODIES 4
/**
 * Create Body Elements
 */
#define HPS_CREATE_BODY_SEGMENTS 8
/**
 * Create Color Segments
 */
#define HPS_CREATE_COLOR_SEGMENTS 16
/**
 * Tessellate ellipses
 */
#define HPS_TESSELLATE_ELLIPSES 32
/**
 * Render edges
 */
#define HA_RENDER_EDGES 64
/**
 * Render faces
 */
#define HA_RENDER_FACES 128
/**
 * @deprecated R10
 * <br><br>
 * This API has been deprecated in R10.
 * <br><br>
 * <b>Role:</b> Please implement the initialisation functions in the sequence described below:<br>
 * <br>
 * <li><tt>initialize_base</tt><br>
 * The first initialization function is to initialize the base component. This initializes the ACIS memory manager.<br>
 * <br>
 * <li><tt>api_start_modeller</tt><br>
 * After calling <tt>initialize_base</tt>, start the modeler by explicitly calling <tt>api_start_modeller</tt>.<br>
 * <br>
 * <li>ACIS component initialization<br>
 * Each ACIS component used in a user application must be initialized before use.
 * For example, if a user application performs booleans, then a call to <tt>api_initialize_boolean</tt>
 * should be made explicitly. These component initialization functions should be called after <tt>api_start_modeller</tt>.<br>
 * <br>
 * <li><tt>api_initialize_hps_acis_bridge</tt><br>
 * The <tt>api_initialize_hps_acis_bridge</tt> function has been added to initialize the bridge.
 * This function should be called after <tt>api_start_modeller</tt> and component initialization.
 * <br><br>
 * @param options
 * options.
 */
DECL_HPS void HPS_Init( const char* in_options );

/**
 * @deprecated R10
 * <br><br>
 * This API has been deprecated in R10.
 * <br><br>
 * <b>Role:</b> Please implement the termination functions in the sequence described below:<br>
 * <br>
 * <li><tt>api_terminate_hps_acis_bridge</tt><br>
 * The <tt>api_terminate_hps_acis_bridge</tt> function has been added to terminate the bridge.
 * This function should be called before calling <tt>api_stop_modeller</tt>.<br>
 * <br>
 * <li>ACIS component termination<br>
 * Each ACIS component used by the user application must be terminated before closing the application.
 * For example, if the Boolean component was used in the application, a call to <tt>api_terminate_boolean</tt>
 * should be made explicitly. ACIS component termination should happen after calling
 * <tt>api_terminate_hps_acis_bridge</tt> and before calling <tt>api_stop_modeller</tt>.<br>
 * <br>
 * <li><tt>api_stop_modeller</tt><br>
 * Before closing, the user application now needs to stop the modeler explicitly by calling <tt>api_stop_modeller</tt>.<br>
 * <br>
 * <li><tt>terminate_base</tt><br>
 * Finally, terminate the base component by calling <tt>terminate_base</tt>.
 */
DECL_HPS void HPS_Close( void );

/**
 * Delete the geometric primitives associated with the given ACIS entities.
 * <br><br>
 * <b>Role:</b> Deletes the geometric primitives associated with the
 * given ACIS entities from the HPS database and update the mapping between
 * the ACIS pointers and the HPS keys.
 * <br><br>
 * @param in_entitylist
 * List of entities to be deleted from the HPS database.
 */
DECL_HPS void HPS_Delete_Entity_Geometry( ENTITY_LIST const& in_entitylist );

/**
 * Delete the geometric primitives associated with the given ACIS entity.
 * <br><br>
 * <b>Role:</b> Deletes the geometric primitives associated with the
 * given ACIS entity from the HPS database and update the mapping between
 * the ACIS pointer and the HPS keys.
 * <br><br>
 * @param in_entity
 * Entity to be deleted from the HPS database.
 */
DECL_HPS void HPS_Delete_Entity_Geometry( ENTITY *in_entity );

/**
 * Generate the tessellation of a set of ACIS entities.
 * <br><br>
 * <b>Role:</b> Generates the tessellation of a set of ACIS entities and stores it in the
 * HPS database using the HPS/ACIS Bridge.<br><br>
 * <i><b>Note:</b> <tt>HPS_Render_Entities</tt> will use default pattern set by "segment pattern"
 * option sent to <tt>HPS_Set_Rendering_Options<tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HPS_Set_Rendering_Options(buffer);</pre>
 * @param in_entitylist
 * List of entities to be rendered.
 * @param in_pattern
 * Pattern.
 */
DECL_HPS void HPS_Render_Entities(
	const ENTITY_LIST& in_entitylist,
	const char* in_pattern = 0 );

/**
 * HPS_ReRender_Entities regenerates the rendering data for the given ENTITYs.
 * <br><br>
 * <b>Role:</b> The rendering data in the HPS database is first deleted and then regenerated..<br><br>
 * <i><b>Note:</b> <tt>HPS_ReRender_Entities</tt> will use default pattern set by "segment pattern"
 * option sent to <tt>HPS_Set_Rendering_Options<tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HPS_Set_Rendering_Options(buffer);</pre>
 * @param in_entitylist
 * List of entities to be rendered.
 * @param in_pattern
 * Pattern.
 */
DECL_HPS void HPS_ReRender_Entities(
	const ENTITY_LIST& in_entitylist,
	const char* in_pattern = 0 );

/**
 * Generate the tessellation of an ACIS entity.
 * <br><br>
 * <b>Role:</b> Generates the tessellation of an ACIS entity and stores it in the
 * HPS database using the HPS/ACIS Bridge.<br><br>
 * <i><b>Note:</b> <tt>HPS_Render_Entity</tt> will use default pattern set by "segment pattern" option
 * sent to <tt>HPS_Set_Rendering_Options</tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HPS_Set_Rendering_Options(buffer); </pre>
 * @param entity
 * ACIS entity.
 * @param pattern
 * Pattern.
 * <br><br>
 * @return
 * Key for HPS segment.
 */
DECL_HPS HPS::SegmentKey HPS_Render_Entity(
	ENTITY*		in_entity,
	const char* in_pattern = 0 );
DECL_HPS HPS::SegmentKey HPS_Render_Entity(
	ENTITY*		in_entity,
	HPS::SegmentKey in_segment_key );

/**
* Renders a SPAposition_cloud into its own hoops segment.  The returned value
* is the hoops key for the cloud's segment.
**/

DECL_HPS HPS::SegmentKey HPS_Render_PositionCloud( SPAposition_cloud const& cloud );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Set the rendering color for a SPApoint_cloud object
 * <br><br>
 * <b>Role:</b> Changes the stored value for the color of a point cloud.<br><br>
 * <i><b>Note:</b> <tt>HPS_Set_PointCloud_Color</tt> will only take effect after the point cloud
 * is redrawn.</i><br><br>
 * @param inputCloud
 * Pointer to a SPApoint_cloud to be modified.
 * @param inColor
 * RGB color specifying the new color of the point cloud.
 * <br><br>
 * @return
 * TRUE if successful.
 */
DECL_HPS HPS_BOOLEAN HPS_Set_PointCloud_Color( SPApoint_cloud* inputCloud, rgb_color inColor );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Get the rendering color for a SPApoint_cloud object
 * <br><br>
 * <b>Role:</b> Returns the stored value for the color of a point cloud.<br><br>
 * @param inputCloud
 * Pointer to a SPApoint_cloud to be queried.
 * @param outColor
 * RGB color specifying the current color of the point cloud.
 * <br><br>
 * @return
 * TRUE if successful.
 */
DECL_HPS HPS_BOOLEAN HPS_Get_PointCloud_Color( SPApoint_cloud* inputCloud, rgb_color& outputColor );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Turn highlighting for a point cloud on or off.
 * <br><br>
 * <b>Role:</b> Generates a temporary segment and renders highlighted points in it, or deletes
 * the temporary segment if highlighting is being disabled.<br><br>
 * @param inputCloud
 * SPApoint_cloud object to be highlighted
 * @param inColor
 * Highlighting color.
 * @param turnOn
 * Logical specifying TRUE if highlighting is being turned on, FALSE if not.
 * <br><br>
 * @return
 * Key for HPS segment.
 */
DECL_HPS HPS::SegmentKey HPS_Highlight_PointCloud( SPApoint_cloud* inputCloud, const rgb_color& inColor, logical turnOn );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Erases a point cloud from the display.
 * <br><br>
 * <b>Role:</b> Finds and deletes the segment displaying the given point cloud, if one exists.<br><br>
 * @param inputCloud
 * SPApoint_cloud object to be erased
 * <br><br>
 * @return
 * Boolean indicating success or failure.
 */
DECL_HPS HPS_BOOLEAN HPS_Erase_PointCloud( SPApoint_cloud* inputCloud );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Render a point cloud to the active display.
 * <br><br>
 * <b>Role:</b> Constructs appropriate HPS segment then stores the
 * points of the cloud in a shell inside the segment.<br><br>
 * @param inputCloud
 * SPApoint_cloud object to be rendered
 * @param color
 * Rendering color.
 * <br><br>
 * @return
 * Key for HPS segment the point cloud was placed in.
 */
DECL_HPS HPS::SegmentKey HPS_Render_PointCloud( SPApoint_cloud* inputCloud, const rgb_color& color );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Select an entire point cloud.
 * <br><br>
 * <b>Role:</b> Determines what point cloud is closest to the selection location and returns it.<br><br>
 * @param scene_key
 * Key to HPS scene to pick in.
 * @param pick_point
 * HPS_Point giving the pick location in screen coordinates.
 * <br><br>
 * @return
 * Point cloud picked by the user.
 */
DECL_HPS SPApoint_cloud* HPS_Select_Point_Cloud( HPS::SegmentKey scene_key, HPS_Point pick_point );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Select a single point within a cloud.
 * <br><br>
 * <b>Role:</b> Picks the closest point within the point cloud closest to the selection location.<br><br>
 * @param scene_key
 * Key to HPS scene to pick in.
 * @param pick_point
 * HPS_Point giving the pick location in screen coordinates.
 * <br><br>
 * @return
 * Point cloud containing a single point.
 */
DECL_HPS SPApoint_cloud* HPS_Select_Point_Cloud_Marker( HPS::SegmentKey scene_key, HPS_Point pick_point );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Select zero or more points within a point cloud.
 * <br><br>
 * <b>Role:</b> Selects all points contained within a rectangular area, using the closest point cloud to the center of the
 * selection area as the "parent" point cloud for the subset.<br><br>
 * @param scene_key
 * Key to HPS scene to pick in.
 * @param pick_point1
 * HPS_Point giving the first corner of the rectangular pick area.
 * @param pick_point2
 * HPS_Point giving the second corner of the rectanglar pick area.
 * <br><br>
 * @return
 * Point cloud subset specified by the rectangular area.
 */
DECL_HPS SPApoint_cloud* HPS_Select_Point_Cloud_Markers( HPS::SegmentKey scene_key, HPS_Point pick_point1, HPS_Point pick_point2 );

/**
 * Updates the given entity by re-tessellating it. The HPS segment hierarchy will be retained
 * to the extent possible.  The top level HPS::SegmentKey returned from HPS_Render_Entity
 * will be maintained.
 * <br><br>
 * <b>Role:</b> Regenerates the tessellation of an ACIS entity and updates the
 * previously created HPS segment where the entity exists.<br><br>
 * <i><b>Note:</b> <tt>HPS_Render_Entity</tt> Should have already been used to create this Entity
 * in the HPS database.  Topology changes in the ENTITY between HPS_Render_Entity
 * and HPS_Update_Entity can force HPS_Update_Entity to make changes in the underlying
 * HPS subsegments that may be generated.</i><br><br>
 * @param entity
 * ACIS entity.
 * <br><br>
 * @return
 * TRUE if successful, FALSE otherwise.
 */
DECL_HPS logical HPS_Update_Entity( ENTITY* in_entity );

/**
 * HPS_ReRender_Entity regenerates the rendering data for the given ENTITY.
 * <br><br>
 * <b>Role:</b> The rendering data in the HPS database is first deleted and then regenerated.<br><br>
 * <i><b>Note:</b> <tt>HPS_ReRender_Entity</tt> will use default pattern set by "segment pattern" option
 * sent to <tt>HPS_Set_Rendering_Options</tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HPS_Set_Rendering_Options(buffer); </pre>
 * @param entity
 * ACIS entity.
 * @param pattern
 * Pattern.
 * <br><br>
 * @return
 * Key for HPS segment.  This is probably different than the ENTITY originally used.
 */
DECL_HPS HPS::SegmentKey HPS_ReRender_Entity(
	ENTITY*		in_entity,
	const char* in_pattern = 0 );

/**
 * Returns the current <tt>IEntityConverter</tt>.
 * <br><br>
 * @return
 * The current IEntityConverter.
 */
DECL_HPS IEntityConverter *HPS_Get_Entity_Converter();

/**
 * Sets the <tt>IEntityConverter</tt> used by <tt>HPS_Render_Entity</tt>/<tt>HPS_Render_Entities</tt>.<br>
 * <br>
 * @param ent_conv
 * IEntityConverter used by HPS_Render_Entity/HPS_Render_Entities.
 */
DECL_HPS void HPS_Set_Entity_Converter( IEntityConverter* in_converter );

/**
 * Highlights or un-highlights an <tt>ENTITY</tt> with an <tt>rgb_color</tt>.
 * <br><br>
 * @param e
 * Given ENTITY.
 * @param on
 * Indicates if the ENTITY should be highlighted or not.
 * @param color
 * Highlighting color to be used.
 */
DECL_HPS void HPS_Highlight_Entity( ENTITY* in_entity, logical in_highlight, const rgb_color& in_color );

/**
 * Determines whether an <tt>ENTITY</tt> is highlighted.<br>
 * <br>
 * @param e
 * Given ENTITY.
 * <br><br>
 * @return
 * Indicates if the ENTITY is highlighted or not.
 */
DECL_HPS logical HPS_Show_Entity_Highlight( ENTITY* in_entity );

/**
 * Generates the tessellation of an ACIS <tt>CURVE</tt>.
 * <br><br>
 * <b>Role:</b> Stores the tessellation in the HPS database using the HPS/ACIS Bridge.
 * The ACIS <tt>CURVE</tt> is bounded by the given parameters and transformed by the given transform.
 * <br><br>
 * <b>Precondition:</b> A segment is already open.
 * <br><br>
 * @param this_curve
 * Given ACIS CURVE.
 * @param start_param
 * Starting parameter on the CURVE.
 * @param end_param
 * Ending parameter on the CURVE.
 * @param sketch_transform
 * Transformation.
 * <br><br>
 * @return
 * Key for HPS segment.
 */
DECL_HPS HPS::Key HPS_Render_Curve(
	CURVE *				in_curve_entity,
	SPAparameter		in_start_param,
	SPAparameter		in_end_param,
	const SPAtransf*	in_transform,
	HPS::SegmentKey		in_segment_key
	);

/**
 * Provides control over how ACIS objects are rendered to HPS objects via the HPS/ACIS Mesh Manager.
 * <br><br>
 * @param rendering_options
 * A quoted string or a string variable containing a list of the desired settings.
 */
DECL_HPS void HPS_Set_Rendering_Options( const char * in_list );

/**
 * Provides control over how ACIS objects are rendered to HPS objects via the HPS/ACIS Mesh Manager.
 * <br><br>
 * @param rendering_options
 * hps_rendering_options class containing desired settings.
 */
DECL_HPS void HPS_Set_Rendering_Options( const hps_rendering_options &in_rendering_options );

/**
 * Returns the current <tt>hps_rendering_options</tt>.
 * <br><br>
 * @return
 * The current hps_rendering_options.
 */
DECL_HPS hps_rendering_options &HPS_Get_Rendering_Options();

/**
 * Shows the list of rendering options.
 * <br><br>
 * @param list
 * List of rendering options.
 */
/**
 * Provides control over how ACIS objects are faceted during rendering.
 * <br><br>
 * @param facet_opts
 * facet_options class containing desired settings.
 */
DECL_HPS void HPS_Set_Facet_Options( facet_options *in_facet_opts );

/**
 * Returns the current <tt>facet_options</tt>.
 * <br><br>
 * @return
 * The current facet_options.
 */
DECL_HPS facet_options *HPS_Get_Facet_Options();

/**
 * Shows the list of rendering options.
 * <br><br>
 * @param list
 * List of rendering options.
 */
DECL_HPS void HPS_Show_Rendering_Options( char * out_list );

/**
 * Shows a rendering option and the value set to it.
 * <br><br>
 * @param type
 * Rendering option.
 * @param value
 * Value set to the rendering option.
 */
DECL_HPS void HPS_Show_One_Rendering_Option( const char * in_type, char * out_value );

/**
 * Returns the array of HPS keys for the geometry in the HPS database generated by the HPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> <tt>HPS_Render_Entity</tt> must have tessellated the <tt>ENTITY</tt>.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * @param count
 * Maximum number of keys that should be returned, see @href HPS_Compute_Geometry_Key_Count.
 * @param keys
 * Array of associated HPS keys. Returned to user.
 * <br><br>
 * @return
 * Actual number of keys returned to user in keys array. Always less than or equal to count.
 */
DECL_HPS unsigned long HPS_Compute_Geometry_Keys( ENTITY* in_entity, HPS::KeyArray& out_key_array, const char* in_geomTypes );

/**
 * Returns the array of HPS keys for the geometry in the HPS database generated by the HPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> <tt>HPS_Render_Entity</tt> must have tessellated the <tt>ENTITY</tt>.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * @param count
 * Maximum number of keys that should be returned, see @href HPS_Compute_Geometry_Key_Count.
 * @param keys
 * Array of associated HPS keys. Returned to user.
 * @param geomTypes
 * A quoted string or string variable specifying the particular types of ACIS geometry that should be converted.
 * <br><br>
 * @return
 * Actual number of keys returned to user in keys array. Always less than or equal to count.
 */
DECL_HPS unsigned long HPS_Compute_Geometry_Keys( ENTITY* in_entity, HPS::KeyArray& out_key_array );

/**
 * Returns the number of HPS geometric primitives in the HPS database that graphically represent the ACIS entity.
 * <br><br>
 * <b>Role:</b> This allows allocating an appropriate sized keys array for a subsequent <tt>HPS_Compute_Geometry_Keys</tt> call.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * @param geomTypes
 * A quoted string or string variable specifying the particular types of ACIS geometry that should be converted.
 * <br><br>
 * @return
 * Number of HPS keys associated with the given ACIS entity.
 */
DECL_HPS unsigned long HPS_Compute_Geometry_Key_Count( ENTITY* in_entity, const char* in_geomTypes );

/**
 * Returns the number of HPS geometric primitives in the HPS database that graphically represent the ACIS entity.
 * <br><br>
 * <b>Role:</b> This allows allocating an appropriate sized keys array for a subsequent <tt>HPS_Compute_Geometry_Keys</tt> call.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * <br><br>
 * @return
 * Number of HPS keys associated with the given ACIS entity.
 */
DECL_HPS unsigned long HPS_Compute_Geometry_Key_Count( ENTITY* in_entity );

/**
 * Returns the pointer for the ACIS entity that contains the specified HPS geometric entity.
 * <br><br>
 * @param key
 * HPS key provided for computing associated ACIS entity pointer.
 * @param acisClass
 * ACIS Class for desired entity associated with the input HPS key.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HPS ENTITY* HPS_Compute_Entity_Pointer( HPS::Key in_key, int in_acisClass );

/**
 * Returns the pointer for the ACIS entity that contains the specified HPS geometric entity.
 * <br><br>
 * @param key
 * HPS key provided for computing associated ACIS entity pointer.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HPS ENTITY* HPS_Compute_Entity_Pointer( HPS::Key in_key );

/**
 * Associates the entity with key in the map.
 * <br><br>
 * <b>Role:</b> This is required if an ACIS ENTITY is rendered in HPS scene by a non-bridge routine and still needs to be considered by the Bridge.
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param key
 * Key for HPS segment.
 * <br><br>
 * @return
 * Returns TRUE if the given key is associated with the entity.
 */
DECL_HPS logical HPS_Associate_Key_To_Entity( ENTITY* in_entity, HPS::SegmentKey in_key );

/**
 * Disassociates a key in the map from an entity.
 * <br><br>
 * <b>Role:</b> This allows the removal of HPS geometry without removing the complete entity and the rest of its geometry from the Bridge.
 * <br><br>
 * @param entity (can be NULL)
 * Given ENTITY.
 * @param key
 * Key for HPS segment.
 * <br><br>
 * @return
 * Returns TRUE if the given key is disassociated from the entity.
 */
DECL_HPS logical HPS_Disassociate_Key_From_Entity( ENTITY* in_entity, HPS::SegmentKey in_key );

/**
 * Converts the pattern input to an expanded string and places into a buffer.
 * <br><br>
 * <b>Role:</b> The pattern should be a string with '/' separating keywords and static strings.
 * <br><br>
 * Keywords:
 * <ul>
 *	<li>history (this is replaced with entity->history())</li>
 *	<li>entity (this is replaced with the ENTITY's pointer value)</li>
 *	<li>type (this is replaced with entity->type_name())</li>
 * </ul>
 * <br><br>
 * For example: <i>"?Include Library/entity"</i> will become <i>"?Include Library/0x12345678"</i> if entity=0x12345678
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param inbuffer
 * Expanded string, should be large enough to hold the expanded string.
 * @param pattern
 * Segment pattern string.
 * <br><br>
 * @return
 * Value of inbuffer.
 */
DECL_HPS char* HPS_Build_Segment_String( ENTITY* in_entity, char* out_buffer, const char* in_pattern );

/**
 * Returns the value from <tt>HPS_KOpen_Segment(HPS_Build_Segment_String(entity,pattern))</tt>.
 * <br><br>
 * <b>Role:</b> Calls <tt>HPS_Build_Segment_String</tt>.
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param pattern
 * Segment pattern string.
 * <br><br>
 * @return
 * Returns HPS_KOpen_Segment.
 */
DECL_HPS HPS::SegmentKey HPS_KOpen_Segment( ENTITY* entity, const char* pattern = 0, HPS::SegmentKey in_segment_key = HPS_INVALID_SEGMENT_KEY );
DECL_HPS HPS::SegmentKey HPS_Open_Segment( const char * in_pattern, HPS::SegmentKey in_segment_key );

/**
 * Returns the value from <tt>HC_KOpen_Segment(HPS_Build_Segment_String(entity,pattern))</tt>.
 * <br><br>
 * <b>Role:</b> Calls <tt>HPS_Build_Segment_String</tt> and then <tt>HC_KOpen_Segment</tt>/<tt>HC_KClose_Segment</tt>.
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param pattern
 * Segment pattern string.
 * <br><br>
 * @return
 * Returns HC_KOpen_Segment.
 */
DECL_HPS HPS::SegmentKey HPS_KCreate_Segment( ENTITY* in_entity, const char* in_pattern = 0, HPS::SegmentKey in_segment_key = HPS_INVALID_SEGMENT_KEY );

/**
 * Parses the input ACIS file, populates the ACIS kernel with its contents,
 * and creates the corresponding geometry in the HPS database using the ACIS renderer and HPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> This function also fills the given entity list with the entities loaded from the file.
 * Calling this function is equivalent to calling <tt>api_restore_entity_list</tt> and <tt>HPS_Render_Entities</tt>, except
 * that a file name is used instead of a file pointer.
 * <br><br>
 * @param fname
 * Name of the ACIS File to be parsed.
 * @param entitylist
 * List of entities read in.
 * <br><br>
 * @return
 * Returns TRUE when successful.
 */
DECL_HPS logical HPS_Read_Sat_File( const char *in_file_name, HPS::SegmentKey in_segment_key, ENTITY_LIST& out_entity_list );

/**
 * Saves all entities in the given entity list with the entities loaded from to the given file.
 * <br><br>
 * <b>Role:</b> Calling this function is equivalent to calling <tt>api_save_entity_list</tt>, except
 * that a file name is used instead of a file pointer.
 * <br><br>
 * @param fname
 * Name of the ACIS File to be written.
 * @param entitylist
 * List of entities to be written.
 * <br><br>
 * @return
 * Returns TRUE when successful.
 */
DECL_HPS logical HPS_Write_Sat_File( const char *in_file_name, ENTITY_LIST const& in_entity_list );

/**
 * Saves all entities in the given entity list with the entities loaded from to the given file.
 * <br><br>
 * <b>Role:</b> Calling this function is equivalent to calling <tt>api_save_entity_list</tt>, except
 * that a file name is used instead of a file pointer.
 * <br><br>
 * @param fname
 * Name of the ACIS File to be written.
 * @param entitylist
 * List of entities to be written.
 * @param version
 * ACIS version number. Use float values (e.g. 6.0, 6.3)for indicating the ACIS version number to use for archiving the file.
 * <br><br>
 * @return
 * Returns TRUE when successful.
 */
DECL_HPS logical HPS_Write_Sat_File_2( const char *in_file_name, ENTITY_LIST const& in_entity_list, float version );

/**
 * Parses the input ACIS file, populates the ACIS kernel with its contents,
 * and creates the corresponding geometry in the HPS database using the ACIS renderer and HPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> This function also fills the given entity list with the entities loaded from the file.
 * Calling this function is equivalent to calling <tt>HPS_Read_Sat_File</tt>, except
 * that information in the file is expected to be in binary format.
 * <br><br>
 * @param fname
 * Name of the ACIS File to be parsed.
 * @param entitylist
 * List of entities read in.
 * <br><br>
 * @return
 * Returns TRUE when successful.
 */
DECL_HPS logical HPS_Read_Sab_File( const char *in_file_name, HPS::SegmentKey in_segment_key, ENTITY_LIST& out_entity_list );

/**
 * Saves all entities in the given entity list with the entities loaded from to the given file.
 * <br><br>
 * <b>Role:</b> Calling this function is equivalent to calling <tt>HPS_Write_Sat_File</tt>, except
 * that information is written in binary format.
 * <br><br>
 * @param fname
 * Name of the ACIS File to be written.
 * @param entitylist
 * List of entities to be written.
 * <br><br>
 * @return
 * Returns TRUE when successful.
 */
DECL_HPS logical HPS_Write_Sab_File( const char *in_file_name, ENTITY_LIST const& in_entity_list );

/**
 * Converts an <tt>INDEXED_MESH</tt> into a HPS shell.
 * <br><br>
 * <b>Role:</b> A segment should already be open, where the shell should be placed.
 * <br><br>
 * @param mesh
 * Given INDEXED MESH.
 * <br><br>
 * @return
 * Key for HPS segment.
 */
DECL_HPS HPS::Key HPS_KINDEXED_MESH_to_HOOPS( INDEXED_MESH *in_mesh, FACE *in_face = 0 );

/**
 * Converts an <tt>SEQUENTIAL_MESH</tt> into a HPS shell.
 * <br><br>
 * <b>Role:</b> A segment should already be open, where the shell should be placed.
 * <br><br>
 * @param mesh
 * Given SEQUENTIAL MESH.
 * <br><br>
 * @return
 * Key for HPS segment.
 */
class af_serializable_mesh;
DECL_HPS HPS::Key HPS_K_SEQUENTIAL_MESH_to_HOOPS( af_serializable_mesh *in_mesh, FACE * in_face = 0 );

/**
 * Set the current view segment.
 * <br><br>
 * <b>Role:</b> Used for <tt>HPS_Get_Facet_Tolerance()</tt>.
 * <br><br>
 * @param key
 * Key for HPS segment.
 */
DECL_HPS void HPS_Set_Current_View( HPS::View in_key );

/**
 * Returns the current view segment.
 * <br><br>
 * <b>Role:</b> Used for <tt>HPS_Get_Facet_Tolerance()</tt>.
 * <br><br>
 * @return
 * Key for HPS segment.
 */
DECL_HPS HPS::View HPS_Get_Current_View();

DECL_HPS HPS::SegmentKey	HPS_Get_Current_View_Model_Segment_Key();
DECL_HPS void				HPS_Get_Current_View_Model_KeyPath( HPS::KeyPath & in_key_path );


/**
 * Returns world coordinates per pixel.
 * <br><br>
 * <b>Role:</b> Uses the current view key set by <tt>HPS_Set_Current_View_Segment</tt>.
 * <br><br>
 * @return
 * World co-ordinates per pixel.
 */
DECL_HPS double HPS_World_Coords_Per_Pixel();

/**
 * Updates the transforms attached to the segments associated with the specified bodies.
 * <br><br>
 * <b>Role:</b> The existing transforms attached to the body segments are replaced by the
 * ones currently attached to each BODY.  Entities contained in <tt>bodies</tt> are ignored if they are not actually
 * bodies.
 * <br><br>
 * @param bodies
 * List of bodies for which an update is requested.
 * <br><br>
 * @return
 * TRUE if any updates are performed, FALSE otherwise.
 */
DECL_HPS logical HPS_ReRender_Body_Transforms( ENTITY_LIST& in_bodies );


/**
 * ACIS style initialization.
 * <br><br>
 * <b>Role:</b> The following calls are made inside this routine:
 * <br><pre>
 * api_initialize_faceter();
 * api_initialize_rendering();
 * api_initialize_generic_attributes();
 * g_History_Callbacks = ACIS_NEW HPS_History_Callbacks;
 * get_history_callbacks_list().add(g_History_Callbacks);
 * // Set the ENTITY converter.
 * HPS_Set_Entity_Converter(ACIS_NEW hps_acis_entity_converter); </pre>
 * @return
 * Indicates success or failure.
 **/
DECL_HPS outcome api_initialize_hps_acis_bridge();

/**
 * ACIS style termination.
 * <br><br>
 * <b>Role:</b> The following calls are made inside this routine:
 * <pre>
 * IEntityConverter *icvrt=HPS_Get_Entity_Converter();
 * HPS_Set_Entity_Converter(0);
 * ACIS_DELETE icvrt;
 * api_terminate_generic_attributes();
 * api_terminate_rendering();
 * api_terminate_faceter(); </pre>
 * @return
 * Indicates success or failure.
 **/
DECL_HPS outcome api_terminate_hps_acis_bridge();

/**
 * @nodoc
 */
DECL_HPS int Get_Map_Entries();

/**
 * @nodoc
 */
DECL_HPS logical Is_In_Top_Map( ENTITY* ent );

/**
* @nodoc
*/
DECL_HPS void HPS_Print_Portfolio( FILE * in_file_pointer, HPS::PortfolioKey in_key );
DECL_HPS void HPS_Print_Portfolio( FILE * in_file_pointer, HPS::SegmentKey in_segkey );

/** @} */
#endif /* __HPS_BRIDGE_H_ */
