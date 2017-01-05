/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#if !defined( _ientity_converter_hxx_)
#define _ientity_converter_hxx_
#include "dcl_hoops.h"
#include "mmgr.hxx"
#include "hc.h"
/**
 * @file ientityconverter.h
 * @CAA2Level L1
 * @CAA2Usage U2
 *! \addtogroup HABRIDGEAPI
 *  \brief Declared at <ientityconverter.h>
 *  @{
 */

class ENTITY;
class ha_rendering_options;
class ha_rendering_context;
class HA_Map;
class asm_model;
class component_handle;

/**
 * Interface to be implemented by custom <tt>ENTITY</tt> conversion classes.
 * <br><br>
 * <b>Role:</b> An implementation of this class should convert ACIS
 * <tt>ENTITY</tt>s into shaded representations based on polygon meshes.
 * It should also output the correct material types to the display
 * objects so that the user's color and texture settings are handled
 * correctly.
 */
class DECL_HOOPS IEntityConverter: public ACIS_OBJECT
{
public:
	/**
	 * Convert the given ACIS entity to HOOPS display-object data.
	 * <br><br>
	 * <b>Role:</b> Converts the geometric primitives associated with
	 * the given ACIS <tt>ENTITY</tt> to HOOPS display-object data and updates
	 * the mapping between the ACIS <tt>ENTITY</tt> pointers and the HOOPS keys.
	 * <br><br>
	 * This is the method that should be called to initially create the 
	 * data for an entity; it should not attempt a limited rebuild
	 * of the object.  However, it should at least look to see if there
	 * are existing data created by this converter and remove them from the
	 * display object before creating the new data.
	 * <br><br>
	 * If the entity to be converted may be an <tt>ASM_ASSEMBLY</tt>, or belong to an
	 * assembly model, the <tt>ConvertEntityAsm</tt> method should be
	 * implemented and used instead.  (That method uses additional segment patterns
	 * and maps associated with assembly objects.)  The default implementation of
	 * this method issues a system error when called on an <tt>ASM_ASSEMBLY</tt>.
	 * <br><br>
	 * @param entity
	 * Entity to be converted. This will most likely be a top-level <tt>ENTITY</tt> such
	 * as <tt>BODY</tt>, but could conceivably be a lower topological type as well.
	 * However, it should preferably be the topmost <tt>ENTITY</tt> pointer available.
	 * @param ro
	 * Set of <tt>ha_rendering_options</tt> to be used for the <tt>ENTITY</tt> conversion.
	 * @param map
	 * Pointer to the <tt>HA_Map</tt> containing the mapping between ACIS <tt>ENTITY</tt> pointers and
	 * HOOPS segment keys.  This mapping should be updated by custom converter
	 * classes during conversion.
	 * @param pattern
	 * Segment pattern string (see <tt>HA_Build_Segment_String</tt>).
	 * @return
	 * HOOPS segment key for the specified <tt>ENTITY</tt>.
	 */
	virtual HC_KEY ConvertEntity(ENTITY* entity,
								 const ha_rendering_options& ro,
								 HA_Map* map,
								 const char* pattern);

	/**
	 * Convert the given ACIS entity to HOOPS display-object data.
	 * <br><br>
	 * <b>Role:</b> Converts the geometric primitives associated with
	 * the given ACIS <tt>ENTITY</tt> to HOOPS display-object data and updates
	 * the mapping between the ACIS <tt>ENTITY</tt> pointers and the HOOPS keys.
	 * <br><br>
	 * This is the method that should be called to initially create the 
	 * data for an entity; it should not attempt a limited rebuild
	 * of the object.  However, it should at least look to see if there
	 * are existing data created by this converter and remove them from the
	 * display object before creating the new data.
	 * <br><br>
	 * This is the method, rather than <tt>ConvertEntity</tt>, that must be implemented
	 * when the entity to be rendered may be an <tt>ASM_ASSEMBLY</tt> or belong to an
	 * assembly model.  To signal this, its default behavior is to issue
	 * a system error when the given entity is an <tt>ASM_ASSEMBLY</tt>; 
	 * when it is not, the method merely calls <tt>ConvertEntity</tt>
	 * with the appropriate parameter values.
	 * <br><br>
	 * @param entity
	 * Entity to be converted. This will most likely be a top-level <tt>ENTITY</tt> such
	 * as <tt>BODY</tt>, but could conceivably be a lower topological type as well.
	 * However, it should preferably be the topmost <tt>ENTITY</tt> pointer available.
	 * @param ro
	 * Set of <tt>ha_rendering_options</tt> to be used for the <tt>ENTITY</tt> conversion.
	 * @param rc
	 * The rendering context, including all segment pattern strings and HOOPS/ACIS
	 * mappings.
	 * @return
	 * HOOPS segment key for the specified ENTITY.
	 */
	virtual HC_KEY ConvertEntityAsm(ENTITY* entity,
								 const ha_rendering_options& ro,
								 const ha_rendering_context& rc); // internal use only

	/**
	 * Convert the geometry of the given assembly model to HOOPS display-object data.
	 * <br><br>
	 * <b>Role:</b> Within an assembly, the geometry associated with a particular model
	 * may be used multiple times.  The purpose of this method is to create HOOPS
	 * segments for such a model so that the geometry is available for inclusion by
	 * the model components that require it.  The default implementation calls
	 * <tt>ConvertEntityAsm</tt> on all top-level entities associated with the model.
	 * <br><br>
	 * @param model
	 * Model whose geometry is to be converted.
	 * @param ro
	 * Set of <tt>ha_rendering_options</tt> to be used for the conversion.
	 * @param rc
	 * The rendering context, including all segment pattern strings and HOOPS/ACIS
	 * mappings.
	 * @return
	 * HOOPS segment key for the specified <tt>asm_model</tt>.
	 */
	virtual HC_KEY ConvertModelGeometry(
								asm_model*					model,
								const ha_rendering_options&	ro,
								const ha_rendering_context&	rc);

	/**
	 * Convert a component of an assembly model to HOOPS display-object data.
	 * <br><br>
	 * <b>Role:</b> The purpose of this method is, first, to create the HOOPS segments 
	 * corresponding to the specified model component.  In addition, it should locate
	 * the segments corresponding to the model geometry, if any, and connect them (via the HOOPS
	 * Include mechanism) to the associated components.  (If it fails to locate the geometry,
	 * is should call <tt>ConvertModelGeometry</tt> in order to create it, then connect
	 * the results to the component.) The default implementation does nothing.
	 * <br><br>
	 * @param comp
	 * Component to be converted.
	 * @param ro
	 * Set of <tt>ha_rendering_options</tt> to be used for the conversion.
	 * @param rc
	 * The rendering context, including all segment pattern strings and HOOPS/ACIS
	 * mappings.
	 * @return
	 * HOOPS segment key for the specified <tt>component_handle</tt>.
	 */
	virtual HC_KEY ConvertModelComponents(
								component_handle*			comp,
								const ha_rendering_options& ro,
								const ha_rendering_context&	rc);

	/**
	 * Change the visibility of faces, edges, etc. of an assembly model based upon
	 * the specified rendering options.
	 * <br><br>
	 * @param ro
	 * Set of <tt>ha_rendering_options</tt> to be used for determining visibility.
	 */
	 virtual void ReRenderVisibilityAsm(const ha_rendering_options&) {}

	/**
	 * Default destructor.
	 */
	virtual ~IEntityConverter(){}
};
/*! @} */
#endif //_ientity_converter_hxx_
