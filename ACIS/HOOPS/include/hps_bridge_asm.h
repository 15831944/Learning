/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_BRIDGE_ASM_H_
#define __HPS_BRIDGE_ASM_H_
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "dcl_hps.h"
#include "hps_util.h"
/**
 * @file hps_bridge_asm.h
 * @CAA2Level L1
 * @CAA2Usage U1
 *! \addtogroup HPSBRIDGEAPI
 *  \brief Declared at <hps_bridge_asm.h>
 *  @{
 */
class ASM_ASSEMBLY;
class ASM_MODEL_REF;
class asm_model;
class asm_model_list;
class component_handle;
class component_entity_handle;
class entity_handle;
class entity_handle_list;

/**
 * Delete the HOOPS segments associated with the given ACIS assembly model reference.
 * <br><br>
 * <b>Role:</b> Deletes the segments associated with the
 * given ACIS assembly model reference from the HOOPS database and updates the mapping between
 * the ACIS pointers and the HOOPS keys.
 * <br><br>
 * @param mref
 * Model-ref whose segments are to be deleted from the HOOPS database.
 */
DECL_HPS void
HPS_Delete_Model_Ref( ASM_MODEL_REF* mref );

/**
 * Delete the HOOPS segments associated with the given ACIS assembly model.
 * <br><br>
 * <b>Role:</b> Deletes the segments associated with the
 * given ACIS assembly model from the HOOPS database and updates the mapping between
 * the ACIS pointers and the HOOPS keys.
 * <br><br>
 * @param model
 * Model whose segments are to be deleted from the HOOPS database.
 */
DECL_HPS void
HPS_Delete_Model( asm_model* model );

/**
 * Delete the HOOPS segments containing the geometry data associated with the given ACIS
 * assembly model.
 * <br><br>
 * <b>Role:</b> Deletes the geometry segments associated with the
 * given ACIS assembly model from the HOOPS database and updates the mapping between
 * the ACIS pointers and the HOOPS keys.  Segments bearing component information (e.g.,
 * model-reference transforms or component color overrides) are left alone.
 * <br><br>
 * @param model
 * Model whose geometry segments are to be deleted from the HOOPS database.
 */
DECL_HPS void
HPS_Delete_Model_Geometry( asm_model* model );

/**
 * Returns an array of HOOPS keys associated with the given assembly model, with the count.
 * <br><br>
 * @param model
 * Pointer to an ACIS assembly model.
 * @param count
 * Maximum number of keys that should be returned, see @href HPS_Compute_Model_Key_Count.
 * @param keys
 * Array of associated HOOPS keys. Returned to user.
 * <br><br>
 * @return
 * Actual number of keys returned to user in keys array. Always less than or equal to count.
 */
DECL_HPS unsigned long
HPS_Compute_Model_Keys( asm_model* model, unsigned long count, HPS::SegmentKey* keys );

/**
 * Returns the number of HOOPS segments in the HOOPS database that represent the given ACIS assembly model.
 * <br><br>
 * <b>Role:</b> This allows allocating an appropriate sized keys array for a subsequent <tt>HPS_Compute_Model_Keys</tt> call.
 * <br><br>
 * @param model
 * Pointer to an ACIS assembly model.
 * <br><br>
 * @return
 * Number of HOOPS keys associated with the given ACIS assembly model.
 */
DECL_HPS unsigned long
HPS_Compute_Model_Key_Count( asm_model* model );

/**
 * Returns the pointer for the ACIS assembly model associated with the specified HOOPS
 * segment key.
 * <br><br>
 * @param key
 * HOOPS key provided for computing associated ACIS assembly-model pointer.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HPS asm_model*
HPS_Compute_Model_Pointer( HPS::SegmentKey key );

/**
 * Associates the model with key in the model map.
 * <br><br>
 * <b>Role:</b> This is required if an ACIS model is rendered in a HOOPS scene by a
 * non-bridge routine and still needs to be considered by the Bridge.
 * <br><br>
 * @param model
 * Given asm_model.
 * @param key
 * Key for HOOPS segment.
 * <br><br>
 * @return
 * Returns true if the given key is associated with the assembly model.
 */
DECL_HPS logical
HPS_Associate_Key_To_Model( asm_model* model, HPS::SegmentKey key );

/**
 * Disassociates a key in the model map from an assembly model.
 * <br><br>
 * @param model (can be NULL)
 * Given asm_model.
 * @param key
 * Key for HOOPS segment.
 * <br><br>
 * @return
 * Returns true if the given key is disassociated from the model.
 */
DECL_HPS logical
HPS_Disassociate_Key_From_Model( asm_model* model, HPS::SegmentKey key );

/**
 * Returns the number of HOOPS segments in the HOOPS database that represent the given ACIS assembly component.
 * <br><br>
 * <b>Role:</b> This allows allocating an appropriate sized keys array for a subsequent <tt>HPS_Compute_Component_Keys</tt> call.
 * <br><br>
 * @param comp
 * Pointer to an ACIS assembly component handle.
 * <br><br>
 * @return
 * Number of HOOPS keys associated with the given ACIS assembly component.
 */
DECL_HPS unsigned long
HPS_Compute_Component_Key_Count( component_handle* comp );

/**
 * Returns an array of HOOPS keys associated with the given assembly component, with the count.
 * <br><br>
 * @param comp
 * Pointer to an ACIS assembly component handle.
 * @param count
 * Maximum number of keys that should be returned, see @href HPS_Compute_Component_Key_Count.
 * @param keys
 * Array of associated HOOPS keys. Returned to user.
 * <br><br>
 * @return
 * Actual number of keys returned to user in keys array. Always less than or equal to count.
 */
DECL_HPS unsigned long
HPS_Compute_Component_Keys( component_handle* comp, unsigned long count, HPS::SegmentKey* keys );

/**
 * Returns the handle pointer for the ACIS assembly component associated with the
 * specified HOOPS segment key.
 * <br><br>
 * @param key
 * HOOPS keys provided for computing associated component-handle pointer.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HPS component_handle*
HPS_Compute_Component_Pointer( HPS::SegmentKey key );

/**
 * Returns the handle pointer for the ACIS assembly component-entity associated with the
 * specified HOOPS segment-key path.
 * <br><br>
 * <b>Role:</b> The first component key and entity key found in the key array are used
 * to determine the component-entity.
 * <br><br>
 * @param count
 * the number of keys in the path.
 * @param keys
 * HOOPS keys provided for computing associated component-entity handle pointer.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HPS component_entity_handle*
HPS_Compute_Component_Entity_Pointer( unsigned long count, HPS::SegmentKey* keys );

/**
 * Returns the handle pointer for the ACIS assembly component-entity associated with the
 * specified HOOPS segments.
 * <br><br>
 * @param comp_key
 * The HOOPS key associated with the component.
 * @param ent_key
 * The HOOPS key associated with the entity.
 * <br><br>
 * @return
 * The requested ACIS pointer.
 */
DECL_HPS component_entity_handle*
HPS_Compute_Component_Entity_Pointer( HPS::SegmentKey comp_key, HPS::SegmentKey ent_key );

/**
 * Returns the HOOPS keys associated with the given component-entity.
 * <br><br>
 * @param comp_ent
 * Pointer to an ACIS component-entity.
 * @param comp_key
 * The HOOPS key associated with the component.  Returned to user.
 * @param ent_key
 * The HOOPS key associated with the entity.  Returned to user.
 * <br><br>
 * @return
 * Returns TRUE if both keys were found.
 */
DECL_HPS logical
HPS_Compute_Component_Entity_Keys( component_entity_handle* comp_ent, HPS::SegmentKey& comp_key, HPS::SegmentKey& ent_key );

/**
 * Associates the component with key in the component map.
 * <br><br>
 * <b>Role:</b> This is required if an ACIS assembly compoment is rendered in a HOOPS scene by a
 * non-bridge routine and still needs to be considered by the Bridge.
 * <br><br>
 * @param comp
 * Given component handle.
 * @param key
 * Key for HOOPS segment.
 * <br><br>
 * @return
 * Returns TRUE if the given key is associated with the assembly component.
 */
DECL_HPS logical
HPS_Associate_Key_To_Component( component_handle* comp, HPS::SegmentKey key );

/**
 * Disassociates a key in the component map from an assembly component.
 * <br><br>
 * @param comp (can be NULL)
 * Given component handle.
 * @param key
 * Key for HOOPS segment.
 * <br><br>
 * @return
 * Returns true if the given key is disassociated from the component.
 */
DECL_HPS logical
HPS_Disassociate_Key_From_Component( component_handle* comp, HPS::SegmentKey key );

/**
 * Highlights or un-highlights a component entity with the given color.
 * <br><br>
 * @param ce
 * The given component entity.
 * @param on
 * Indicates if the component entity should be highlighted or not.
 * @param color
 * Highlighting color to be used.
 */
DECL_HPS void
HPS_Highlight_Component_Entity( component_entity_handle* ce, logical on, const rgb_color& color );

/**
 * Generate the tessellation of an ACIS assembly model.
 * <br><br>
 * <b>Role:</b> Generates the tessellation of an ACIS assembly model and stores it in the
 * HOOPS database using the HPS/ACIS Bridge.<br><br>
 * <i><b>Note:</b> <tt>HPS_Render_Model</tt> will use default pattern set by "segment pattern" option
 * sent to <tt>HPS_Set_Rendering_Options</tt>.</i><br><br>
 * For example:
 * <pre>
 * char buffer[1024];
 * char pbuffer[POINTER_BUFFER_SIZE];
 * sprintf(buffer,"segment pattern = ?Include Library/%s",ptoax(m_pPart.GetPart(), pbuffer));
 * HPS_Set_Rendering_Options(buffer); </pre>
 * @param asm_model
 * pointer to an ACIS assembly model.
 * @param pattern
 * Pattern.
 * <br><br>
 * @return
 * Key for HOOPS segment.
 */
DECL_HPS HPS::SegmentKey
HPS_Render_Model(
asm_model*	model,
const char*					pattern = 0 );

/**
 * HPS_ReRender_Model_Geometry regenerates the B-Rep rendering data for the given assembly model.
 * <br><br>
 * <b>Role:</b> The rendering data associated with an ACIS assembly model is organized in
 * two segment trees.  One contains geometric data (e.g., meshes) as well as body-specific
 * properties, such as colors.  The other contains component-related data (e.g., the transforms
 * associated with model references and component property overrides), and references the
 * geometry tree as a include segment.  This function updates only the first of these two trees.
 * <br><br>
 * The rendering data in the HOOPS database is first deleted and then regenerated.<br><br>
 * @param model
 * ACIS assembly model.
 * <br><br>
 * @return
 * Key for HOOPS segment.  This is probably different than the model originally used.
 */
DECL_HPS HPS::SegmentKey
HPS_ReRender_Model_Geometry(
asm_model*	model );

/**
 * HPS_ReRender_Model_Entity regenerates the B-Rep rendering data for specified entity within
 * its assembly model.
 * <br><br>
 * <b>Role:</b> The rendering data associated with an ACIS assembly model is organized in
 * two segment trees.  One contains geometric data (e.g., meshes) as well as body-specific
 * properties, such as colors.  The other contains component-related data (e.g., the transforms
 * associated with model references and component property overrides), and references the
 * geometry tree as a include segment.  This function updates only the first of these two trees.
 * <br><br>
 * The rendering data in the HOOPS database is first deleted and then regenerated.<br><br>
 * @param ent_handle
 * entity handle associated with the entity to be rerendered.
 * <br><br>
 * @return
 * Key for HOOPS segment.  This is probably different than the entity originally used.
 */
DECL_HPS HPS::SegmentKey
HPS_ReRender_Model_Entity(
entity_handle*	ent_handle );

/**
 * HPS_ReRender_Model_Components regenerates the component-related rendering data for the given
 * assembly model.
 * <br><br>
 * <b>Role:</b> The rendering data associated with an ACIS assembly model is organized in
 * two segment trees.  One contains geometric data (e.g., meshes) as well as entity-specific
 * properties, such as colors.  The other contains component-related data (e.g., the transforms
 * associated with model references and component property overrides), and references the
 * geometry tree as a include segment.  This function updates the second of these two trees, and
 * only affects the second if it is missing.
 * <br><br>
 * The rendering data in the HOOPS database is first deleted and then regenerated.
 * <br><br>
 * @param model
 * ACIS assembly model.
 * <br><br>
 * @return
 * Key for HOOPS segment.  This is probably different than the component originally used.
 */
DECL_HPS HPS::SegmentKey
HPS_ReRender_Model_Components(
asm_model*	model );

/**
 * HPS_ReRender_Component regenerates the component-related rendering data associated with the given
 * component and its subcomponents.
 * <br><br>
 * <b>Role:</b> The rendering data associated with an ACIS assembly model is organized in
 * two segment trees.  One contains geometric data (e.g., meshes) as well as entity-specific
 * properties, such as colors.  The other contains component-related data (e.g., the transforms
 * associated with model references and component property overrides), and references the
 * geometry tree as a include segment.  This function updates the second of these two trees, and
 * only affects the second if it is missing.
 * <br><br>
 * The rendering data in the HOOPS database is first deleted and then regenerated.
 * <br><br>
 * @param comp
 * component handle associated with the component to be rerendered.
 * <br><br>
 * @return
 * Key for HOOPS segment.  This is probably different than the component originally used.
 */
DECL_HPS HPS::SegmentKey
HPS_ReRender_Component(
component_handle*	comp );

/**
 * Update the visibility of faces, edges, etc., based upon the current rendering options.
 * <br><br>
 * <b>Role:</b> Implemented only for assembly modeling.
 * <br><br>
 */
DECL_HPS void
HPS_ReRender_Visibility_ASM();

/**
 * @nodoc
 */
DECL_HPS void
HPS_Show_Visibility(
asm_model*	model,
const char*	geomtype,
char*		visibility );

/**
 * @nodoc
 */
DECL_HPS void
HPS_Show_Conditions(
asm_model*	model,
const char*	geomtype,
char*		conditions );


/*! @} */
#endif /* __HPS_BRIDGE_ASM_H_ */
