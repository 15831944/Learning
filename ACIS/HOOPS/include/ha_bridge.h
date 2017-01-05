/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HA_BRIDGE_H_
#define __HA_BRIDGE_H_
#include "hc.h"
#include "base.hxx"
#include "dcl_hoops.h"
#include "lists.hxx"
#include "api.hxx"
#include "param.hxx"
#include "idx_mesh.hxx"
#include "LinkedMesh.hxx"
#include "ha_util.h"
#include "ientityconverter.h"
#include "spa_progress_info.hxx"
#include "rgbcolor.hxx"

class ENTITY;
class CURVE;
class SPAtransf;
class rgb_color;
class ha_rendering_options;
class SPApoint_cloud; // deprecated please consider using SPAposition_cloud
class SPAposition_cloud;
class facet_options;
/**
 * @file ha_bridge.h
 * @CAA2Level L1
 * @CAA2Usage U1
 * \defgroup HABRIDGEAPI HOOPS/ACIS Bridge
 * \brief Declared at <ha_bridge.h>
 * \ingroup VISMODULE
 * @{
 */
/**
 * Edge Geometry Type
 */
#define HA_GEOMETRY_TYPE_edges 2
/**
 * Face Geometry Type
 */
#define HA_GEOMETRY_TYPE_faces 4
/**
 * Body Geometry Type
 */
#define HA_GEOMETRY_TYPE_bodies 8
/**
 * Merge Faces Type
 */
#define HA_MERGE_FACES 2
/**
 * Merge Bodies Type
 */
#define HA_MERGE_BODIES 4
/**
 * Create Body Elements
 */
#define HA_CREATE_BODY_SEGMENTS 8
/**
 * Create Color Segments
 */
#define HA_CREATE_COLOR_SEGMENTS 16
/**
 * Tessellate ellipses
 */
#define HA_TESSELLATE_ELLIPSES 32
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
 * <li><tt>api_initialize_hoops_acis_bridge</tt><br>
 * The <tt>api_initialize_hoops_acis_bridge</tt> function has been added to initialize the bridge.
 * This function should be called after <tt>api_start_modeller</tt> and component initialization.
 * <br><br>
 * @param options
 * options.
 */
DECL_HOOPS void HA_Init( const char* options );

/**
 * @deprecated R10
 * <br><br>
 * This API has been deprecated in R10.
 * <br><br>
 * <b>Role:</b> Please implement the termination functions in the sequence described below:<br>
 * <br>
 * <li><tt>api_terminate_hoops_acis_bridge</tt><br>
 * The <tt>api_terminate_hoops_acis_bridge</tt> function has been added to terminate the bridge.
 * This function should be called before calling <tt>api_stop_modeller</tt>.<br>
 * <br>
 * <li>ACIS component termination<br>
 * Each ACIS component used by the user application must be terminated before closing the application.
 * For example, if the Boolean component was used in the application, a call to <tt>api_terminate_boolean</tt>
 * should be made explicitly. ACIS component termination should happen after calling
 * <tt>api_terminate_hoops_acis_bridge</tt> and before calling <tt>api_stop_modeller</tt>.<br>
 * <br>
 * <li><tt>api_stop_modeller</tt><br>
 * Before closing, the user application now needs to stop the modeler explicitly by calling <tt>api_stop_modeller</tt>.<br>
 * <br>
 * <li><tt>terminate_base</tt><br>
 * Finally, terminate the base component by calling <tt>terminate_base</tt>.
 */
DECL_HOOPS void HA_Close( void );

/**
 * Delete the geometric primitives associated with the given ACIS entities.
 * <br><br>
 * <b>Role:</b> Deletes the geometric primitives associated with the
 * given ACIS entities from the HOOPS database and update the mapping between
 * the ACIS pointers and the HOOPS keys.
 * <br><br>
 * @param entitylist
 * List of entities to be deleted from the HOOPS database.
 */
DECL_HOOPS void HA_Delete_Entity_Geometry(ENTITY_LIST const& entitylist);

/**
 * Delete the geometric primitives associated with the given ACIS entity.
 * <br><br>
 * <b>Role:</b> Deletes the geometric primitives associated with the
 * given ACIS entity from the HOOPS database and update the mapping between
 * the ACIS pointer and the HOOPS keys.
 * <br><br>
 * @param entity
 * Entity to be deleted from the HOOPS database.
 */
DECL_HOOPS void HA_Delete_Entity_Geometry(ENTITY *entity);

/**
 * Generate the tessellation of a set of ACIS entities.
 * <br><br>
 * <b>Role:</b> Generates the tessellation of a set of ACIS entities and stores it in the
 * HOOPS database using the HOOPS/ACIS Bridge.<br><br>
 * <i><b>Note:</b> <tt>HA_Render_Entities</tt> will use default pattern set by "segment pattern"
 * option sent to <tt>HA_Set_Rendering_Options<tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HA_Set_Rendering_Options(buffer);</pre>
 * @param entitylist
 * List of entities to be rendered.
 * @param pattern
 * Pattern.
*/
DECL_HOOPS void HA_Render_Entities(
	const ENTITY_LIST& entitylist, 
	const char* pattern = 0);

/**
 * HA_ReRender_Entities regenerates the rendering data for the given ENTITYs.
 * <br><br>
 * <b>Role:</b> The rendering data in the HOOPS database is first deleted and then regenerated..<br><br>
 * <i><b>Note:</b> <tt>HA_ReRender_Entities</tt> will use default pattern set by "segment pattern"
 * option sent to <tt>HA_Set_Rendering_Options<tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HA_Set_Rendering_Options(buffer);</pre>
 * @param entitylist
 * List of entities to be rendered.
 * @param pattern
 * Pattern.
*/
DECL_HOOPS void HA_ReRender_Entities(
	const ENTITY_LIST& entitylist, 
	const char* pattern = 0);

/**
 * Generate the tessellation of an ACIS entity.
 * <br><br>
 * <b>Role:</b> Generates the tessellation of an ACIS entity and stores it in the
 * HOOPS database using the HOOPS/ACIS Bridge.<br><br>
 * <i><b>Note:</b> <tt>HA_Render_Entity</tt> will use default pattern set by "segment pattern" option
 * sent to <tt>HA_Set_Rendering_Options</tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HA_Set_Rendering_Options(buffer); </pre>
 * @param entity
 * ACIS entity.
 * @param pattern
 * Pattern.
 * <br><br>
 * @return
 * Key for HOOPS segment.
*/
DECL_HOOPS HC_KEY HA_Render_Entity(
	ENTITY*		entity, 
	const char* pattern = 0);

/**
* Renders a SPAposition_cloud into its own hoops segment.  The returned value
* is the hoops key for the cloud's segment.
**/

DECL_HOOPS HC_KEY HA_Render_PositionCloud( SPAposition_cloud const& cloud );

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Set the rendering color for a SPApoint_cloud object
 * <br><br>
 * <b>Role:</b> Changes the stored value for the color of a point cloud.<br><br>
 * <i><b>Note:</b> <tt>HA_Set_PointCloud_Color</tt> will only take effect after the point cloud
 * is redrawn.</i><br><br>
 * @param inputCloud
 * Pointer to a SPApoint_cloud to be modified.
 * @param inColor
 * RGB color specifying the new color of the point cloud.
 * <br><br>
 * @return
 * TRUE if successful.
*/
DECL_HOOPS HC_BOOLEAN HA_Set_PointCloud_Color(SPApoint_cloud* inputCloud, rgb_color inColor);

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
DECL_HOOPS HC_BOOLEAN HA_Get_PointCloud_Color(SPApoint_cloud* inputCloud,rgb_color& outputColor);

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
 * Key for HOOPS segment.
*/
DECL_HOOPS HC_KEY HA_Highlight_PointCloud(SPApoint_cloud* inputCloud, const rgb_color& inColor, logical turnOn);

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
DECL_HOOPS HC_BOOLEAN HA_Erase_PointCloud(SPApoint_cloud* inputCloud);

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Render a point cloud to the active display.
 * <br><br>
 * <b>Role:</b> Constructs appropriate HOOPS segment then stores the 
 * points of the cloud in a shell inside the segment.<br><br>
 * @param inputCloud
 * SPApoint_cloud object to be rendered
 * @param color
 * Rendering color.
 * <br><br>
 * @return
 * Key for HOOPS segment the point cloud was placed in.
*/
DECL_HOOPS HC_KEY HA_Render_PointCloud(SPApoint_cloud* inputCloud, const rgb_color& color);

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Select an entire point cloud.
 * <br><br>
 * <b>Role:</b> Determines what point cloud is closest to the selection location and returns it.<br><br>
 * @param scene_key
 * Key to HOOPS scene to pick in.
 * @param pick_point
 * HA_Point giving the pick location in screen coordinates.
 * <br><br>
 * @return
 * Point cloud picked by the user.
*/
DECL_HOOPS SPApoint_cloud* HA_Select_Point_Cloud(HC_KEY scene_key, HA_Point pick_point);

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Select a single point within a cloud.
 * <br><br>
 * <b>Role:</b> Picks the closest point within the point cloud closest to the selection location.<br><br>
 * @param scene_key
 * Key to HOOPS scene to pick in.
 * @param pick_point
 * HA_Point giving the pick location in screen coordinates.
 * <br><br>
 * @return
 * Point cloud containing a single point.
*/
DECL_HOOPS SPApoint_cloud* HA_Select_Point_Cloud_Marker(HC_KEY scene_key, HA_Point pick_point);

/**
 * SPApoint_cloud is deprecated.  Please consider using SPAposition_cloud.
 *
 * Select zero or more points within a point cloud.
 * <br><br>
 * <b>Role:</b> Selects all points contained within a rectangular area, using the closest point cloud to the center of the
 * selection area as the "parent" point cloud for the subset.<br><br>
 * @param scene_key
 * Key to HOOPS scene to pick in.
 * @param pick_point1
 * HA_Point giving the first corner of the rectangular pick area.
 * @param pick_point2
 * HA_Point giving the second corner of the rectanglar pick area.
 * <br><br>
 * @return
 * Point cloud subset specified by the rectangular area.
*/
DECL_HOOPS SPApoint_cloud* HA_Select_Point_Cloud_Markers(HC_KEY scene_key, HA_Point pick_point1, HA_Point pick_point2);

/**
 * Updates the given entity by re-tessellating it. The HOOPS segment hierarchy will be retained
 * to the extent possible.  The top level HC_KEY returned from HA_Render_Entity
 * will be maintained.
 * <br><br>
 * <b>Role:</b> Regenerates the tessellation of an ACIS entity and updates the 
 * previously created HOOPS segment where the entity exists.<br><br>
 * <i><b>Note:</b> <tt>HA_Render_Entity</tt> Should have already been used to create this Entity
 * in the HOOPS database.  Topology changes in the ENTITY between HA_Render_Entity
 * and HA_Update_Entity can force HA_Update_Entity to make changes in the underlying
 * HOOPS subsegments that may be generated.</i><br><br>
 * @param entity
 * ACIS entity.
 * <br><br>
 * @return
 * TRUE if successful, FALSE otherwise.
*/
DECL_HOOPS logical HA_Update_Entity(ENTITY* entity);           

/**
 * HA_ReRender_Entity regenerates the rendering data for the given ENTITY.
 * <br><br>
 * <b>Role:</b> The rendering data in the HOOPS database is first deleted and then regenerated.<br><br>
 * <i><b>Note:</b> <tt>HA_ReRender_Entity</tt> will use default pattern set by "segment pattern" option
 * sent to <tt>HA_Set_Rendering_Options</tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HA_Set_Rendering_Options(buffer); </pre>
 * @param entity
 * ACIS entity.
 * @param pattern
 * Pattern.
 * <br><br>
 * @return
 * Key for HOOPS segment.  This is probably different than the ENTITY originally used.
*/
DECL_HOOPS HC_KEY HA_ReRender_Entity(
	ENTITY*		entity, 
	const char* pattern = 0);

/**
 * Returns the current <tt>IEntityConverter</tt>.
 * <br><br>
 * @return
 * The current IEntityConverter.
 */
DECL_HOOPS IEntityConverter *HA_Get_Entity_Converter();

/**
 * Sets the <tt>IEntityConverter</tt> used by <tt>HA_Render_Entity</tt>/<tt>HA_Render_Entities</tt>.<br>
 * <br>
 * @param ent_conv
 * IEntityConverter used by HA_Render_Entity/HA_Render_Entities.
 */
DECL_HOOPS void HA_Set_Entity_Converter(IEntityConverter* ent_conv);

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
DECL_HOOPS void HA_Highlight_Entity(ENTITY* e, logical on, const rgb_color& color);

/**
 * Determines whether an <tt>ENTITY</tt> is highlighted.<br>
 * <br>
 * @param e
 * Given ENTITY.
 * <br><br>
 * @return
 * Indicates if the ENTITY is highlighted or not.
 */
DECL_HOOPS logical HA_Show_Entity_Highlight(ENTITY* e);

/**
 * Generates the tessellation of an ACIS <tt>CURVE</tt>.
 * <br><br>
 * <b>Role:</b> Stores the tessellation in the HOOPS database using the HOOPS/ACIS Bridge.
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
 * Key for HOOPS segment.
 */
DECL_HOOPS HC_KEY HA_Render_Curve(CURVE *this_curve, SPAparameter start_param, SPAparameter end_param, const SPAtransf* sketch_transform);

/**
 * Provides control over how ACIS objects are rendered to HOOPS objects via the HOOPS/ACIS Mesh Manager.
 * <br><br>
 * @param rendering_options
 * A quoted string or a string variable containing a list of the desired settings.
 */
DECL_HOOPS void HA_Set_Rendering_Options(const char* rendering_options);

/**
 * Provides control over how ACIS objects are rendered to HOOPS objects via the HOOPS/ACIS Mesh Manager.
 * <br><br>
 * @param rendering_options
 * ha_rendering_options class containing desired settings.
 */
DECL_HOOPS void HA_Set_Rendering_Options(const ha_rendering_options &rendering_options);

/**
 * Returns the current <tt>ha_rendering_options</tt>.
 * <br><br>
 * @return
 * The current ha_rendering_options.
 */
DECL_HOOPS ha_rendering_options &HA_Get_Rendering_Options();

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
DECL_HOOPS void HA_Set_Facet_Options(facet_options *facet_opts);

/**
 * Returns the current <tt>facet_options</tt>.
 * <br><br>
 * @return
 * The current facet_options.
 */
DECL_HOOPS facet_options *HA_Get_Facet_Options();

/**
 * Shows the list of rendering options.
 * <br><br>
 * @param list
 * List of rendering options.
 */
DECL_HOOPS void HA_Show_Rendering_Options(char * list);

/**
 * Shows a rendering option and the value set to it.
 * <br><br>
 * @param type
 * Rendering option.
 * @param value
 * Value set to the rendering option.
 */
DECL_HOOPS void HA_Show_One_Rendering_Option(const char * type, char * value);

/**
 * Returns the array of HOOPS keys for the geometry in the HOOPS database generated by the HOOPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> <tt>HA_Render_Entity</tt> must have tessellated the <tt>ENTITY</tt>.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * @param count
 * Maximum number of keys that should be returned, see @href HA_Compute_Geometry_Key_Count.
 * @param keys
 * Array of associated HOOPS keys. Returned to user.
 * <br><br>
 * @return
 * Actual number of keys returned to user in keys array. Always less than or equal to count.
 */
DECL_HOOPS unsigned long HA_Compute_Geometry_Keys(ENTITY* entity, unsigned long count, HC_KEY* keys, const char* geomTypes);

/**
 * Returns the array of HOOPS keys for the geometry in the HOOPS database generated by the HOOPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> <tt>HA_Render_Entity</tt> must have tessellated the <tt>ENTITY</tt>.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * @param count
 * Maximum number of keys that should be returned, see @href HA_Compute_Geometry_Key_Count.
 * @param keys
 * Array of associated HOOPS keys. Returned to user.
 * @param geomTypes
 * A quoted string or string variable specifying the particular types of ACIS geometry that should be converted.
 * <br><br>
 * @return
 * Actual number of keys returned to user in keys array. Always less than or equal to count.
 */
DECL_HOOPS unsigned long HA_Compute_Geometry_Keys(ENTITY* entity, unsigned long count, HC_KEY* keys);

/**
 * Returns the number of HOOPS geometric primitives in the HOOPS database that graphically represent the ACIS entity.
 * <br><br>
 * <b>Role:</b> This allows allocating an appropriate sized keys array for a subsequent <tt>HA_Compute_Geometry_Keys</tt> call.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * @param geomTypes
 * A quoted string or string variable specifying the particular types of ACIS geometry that should be converted.
 * <br><br>
 * @return
 * Number of HOOPS keys associated with the given ACIS entity.
 */
DECL_HOOPS unsigned long HA_Compute_Geometry_Key_Count(ENTITY* entity, const char* geomTypes);

/**
 * Returns the number of HOOPS geometric primitives in the HOOPS database that graphically represent the ACIS entity.
 * <br><br>
 * <b>Role:</b> This allows allocating an appropriate sized keys array for a subsequent <tt>HA_Compute_Geometry_Keys</tt> call.
 * <br><br>
 * @param entity
 * Pointer to an ACIS entity.
 * <br><br>
 * @return
 * Number of HOOPS keys associated with the given ACIS entity.
 */
DECL_HOOPS unsigned long HA_Compute_Geometry_Key_Count(ENTITY* entity);

/**
 * Returns the pointer for the ACIS entity that contains the specified HOOPS geometric entity.
 * <br><br>
 * @param key
 * HOOPS key provided for computing associated ACIS entity pointer.
 * @param acisClass
 * ACIS Class for desired entity associated with the input HOOPS key.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HOOPS ENTITY* HA_Compute_Entity_Pointer (HC_KEY key, int acisClass);

/**
 * Returns the pointer for the ACIS entity that contains the specified HOOPS geometric entity.
 * <br><br>
 * @param key
 * HOOPS key provided for computing associated ACIS entity pointer.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HOOPS ENTITY* HA_Compute_Entity_Pointer (HC_KEY key);

/**
 * Associates the entity with key in the map.
 * <br><br>
 * <b>Role:</b> This is required if an ACIS ENTITY is rendered in HOOPS scene by a non-bridge routine and still needs to be considered by the Bridge.
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param key
 * Key for HOOPS segment.
 * <br><br>
 * @return
 * Returns TRUE if the given key is associated with the entity.
 */
DECL_HOOPS logical HA_Associate_Key_To_Entity(ENTITY* entity, HC_KEY key);

/**
 * Disassociates a key in the map from an entity.
 * <br><br>
 * <b>Role:</b> This allows the removal of HOOPS geometry without removing the complete entity and the rest of its geometry from the Bridge.
 * <br><br>
 * @param entity (can be NULL)
 * Given ENTITY.
 * @param key
 * Key for HOOPS segment.
 * <br><br>
 * @return
 * Returns TRUE if the given key is disassociated from the entity.
 */
DECL_HOOPS logical HA_Disassociate_Key_From_Entity(ENTITY* entity, HC_KEY key);

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
DECL_HOOPS char* HA_Build_Segment_String(ENTITY* entity, char* inbuffer, const char* pattern=0);

/**
 * Returns the value from <tt>HC_KOpen_Segment(HA_Build_Segment_String(entity,pattern))</tt>.
 * <br><br>
 * <b>Role:</b> Calls <tt>HA_Build_Segment_String</tt>.
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param pattern
 * Segment pattern string.
 * <br><br>
 * @return
 * Returns HC_KOpen_Segment.
 */
DECL_HOOPS HC_KEY HA_KOpen_Segment(ENTITY* entity, const char* pattern = 0);

/**
 * Returns the value from <tt>HC_KOpen_Segment(HA_Build_Segment_String(entity,pattern))</tt>.
 * <br><br>
 * <b>Role:</b> Calls <tt>HA_Build_Segment_String</tt> and then <tt>HC_KOpen_Segment</tt>/<tt>HC_KClose_Segment</tt>.
 * <br><br>
 * @param entity
 * Given ENTITY.
 * @param pattern
 * Segment pattern string.
 * <br><br>
 * @return
 * Returns HC_KOpen_Segment.
 */
DECL_HOOPS HC_KEY HA_KCreate_Segment(ENTITY* entity, const char* pattern = 0);

/**
 * Parses the input ACIS file, populates the ACIS kernel with its contents,
 * and creates the corresponding geometry in the HOOPS database using the ACIS renderer and HOOPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> This function also fills the given entity list with the entities loaded from the file.
 * Calling this function is equivalent to calling <tt>api_restore_entity_list</tt> and <tt>HA_Render_Entities</tt>, except
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
DECL_HOOPS logical HA_Read_Sat_File(const char *fname, ENTITY_LIST& entitylist) ;

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
DECL_HOOPS logical HA_Write_Sat_File(const char *fname, ENTITY_LIST const& entitylist) ;

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
DECL_HOOPS logical HA_Write_Sat_File_2(const char *fname, ENTITY_LIST const& entitylist, float version) ;

/**
 * Parses the input ACIS file, populates the ACIS kernel with its contents,
 * and creates the corresponding geometry in the HOOPS database using the ACIS renderer and HOOPS/ACIS Bridge.
 * <br><br>
 * <b>Role:</b> This function also fills the given entity list with the entities loaded from the file.
 * Calling this function is equivalent to calling <tt>HA_Read_Sat_File</tt>, except
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
DECL_HOOPS logical HA_Read_Sab_File(const char *fname, ENTITY_LIST& entitylist) ;

/**
 * Saves all entities in the given entity list with the entities loaded from to the given file.
 * <br><br>
 * <b>Role:</b> Calling this function is equivalent to calling <tt>HA_Write_Sat_File</tt>, except
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
DECL_HOOPS logical HA_Write_Sab_File(const char *fname, ENTITY_LIST const& entitylist) ;

/**
 * Converts an <tt>INDEXED_MESH</tt> into a HOOPS shell.
 * <br><br>
 * <b>Role:</b> A segment should already be open, where the shell should be placed.
 * <br><br>
 * @param mesh
 * Given INDEXED MESH.
 * <br><br>
 * @return
 * Key for HOOPS segment.
 */
DECL_HOOPS HC_KEY HA_KINDEXED_MESH_to_HOOPS(INDEXED_MESH *mesh, FACE *face=0);

/**
 * Converts an <tt>SEQUENTIAL_MESH</tt> into a HOOPS shell.
 * <br><br>
 * <b>Role:</b> A segment should already be open, where the shell should be placed.
 * <br><br>
 * @param mesh
 * Given SEQUENTIAL MESH.
 * <br><br>
 * @return
 * Key for HOOPS segment.
 */
class af_serializable_mesh;
DECL_HOOPS HC_KEY HA_K_SEQUENTIAL_MESH_to_HOOPS(af_serializable_mesh *mesh, FACE *face=0);

/**
 * Set the current view segment.
 * <br><br>
 * <b>Role:</b> Used for <tt>HA_Get_Facet_Tolerance()</tt>.
 * <br><br>
 * @param key
 * Key for HOOPS segment.
 */
DECL_HOOPS void HA_Set_Current_View_Segment(HC_KEY);

/**
 * Returns the current view segment.
 * <br><br>
 * <b>Role:</b> Used for <tt>HA_Get_Facet_Tolerance()</tt>.
 * <br><br>
 * @return
 * Key for HOOPS segment.
 */
DECL_HOOPS HC_KEY HA_Get_Current_View_Segment();

/**
 * Returns world coordinates per pixel.
 * <br><br>
 * <b>Role:</b> Uses the current view key set by <tt>HA_Set_Current_View_Segment</tt>.
 * <br><br>
 * @return
 * World co-ordinates per pixel.
 */
DECL_HOOPS double HA_World_Coords_Per_Pixel();

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
DECL_HOOPS logical HA_ReRender_Body_Transforms(ENTITY_LIST& bodies);


/**
 * ACIS style initialization.
 * <br><br>
 * <b>Role:</b> The following calls are made inside this routine:
 * <br><pre>
 * api_initialize_faceter();
 * api_initialize_rendering();
 * api_initialize_generic_attributes();
 * g_History_Callbacks = ACIS_NEW HA_History_Callbacks;
 * get_history_callbacks_list().add(g_History_Callbacks);
 * // Set the ENTITY converter.
 * HA_Set_Entity_Converter(ACIS_NEW hoops_acis_entity_converter); </pre>
 * @return
 * Indicates success or failure.
**/
DECL_HOOPS outcome api_initialize_hoops_acis_bridge();

/**
 * ACIS style termination.
 * <br><br>
 * <b>Role:</b> The following calls are made inside this routine:
 * <pre>
 * IEntityConverter *icvrt=HA_Get_Entity_Converter();
 * HA_Set_Entity_Converter(0);
 * ACIS_DELETE icvrt;
 * api_terminate_generic_attributes();
 * api_terminate_rendering();
 * api_terminate_faceter(); </pre>
 * @return
 * Indicates success or failure.
 **/
DECL_HOOPS outcome api_terminate_hoops_acis_bridge();

/**
 * @nodoc
 */
DECL_HOOPS int Get_Map_Entries();

/**
 * @nodoc
 */
DECL_HOOPS logical Is_In_Top_Map(ENTITY* ent);
/**
 * @nodoc
 */
DECL_HOOPS bool HA_Print_Segment_Tree(const char* in_base_seg, int in_level = 0);

/** @} */
#endif /* __HA_BRIDGE_H_ */
