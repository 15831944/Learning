/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#ifndef acis_ha_part_entity_mgr_CLASS
#define acis_ha_part_entity_mgr_CLASS

#include "dcl_ha_part.h"
#include "acis_pm_entity_mgr.hxx"
#include "asm_event_type.hxx"

class PART;
class ENTITY_LIST;
class ASM_ASSEMBLY;
class outcome;
class acis_scm_entity_mgr;
class asm_event_info;

/**
 * @file acis_ha_part_entity_mgr.hxx
 * @CAA2Level L1
 * @CAA2Usage U1
 *! \addtogroup HABRIDGEAPI
 *  \brief Declared at <acis_ha_part_entity_mgr.hxx>
 *  @{
 */

/**
 * Concrete entity manager class for ACIS Assembly Modeling with the Scheme example application.
 * <br><br>
 * <b>Role:</b> The <tt>acis_ha_part_entity_mgr</tt> (and its corresponding factory) is used to
 * integrate ACIS Assembly Modeling with the Scheme example application.  The implementations of <tt>acis_ha_part_entity_mgr</tt>
 * and <tt>acis_ha_part_entity_mgr_factory</tt> are provided as example code.  
 * Please refer to the assembly modeling technical articles for further details.
 * <br>
 * @see acis_pm_entity_mgr, acis_ha_part_entity_mgr
 */
class DECL_HA_PART acis_ha_part_entity_mgr : public acis_pm_entity_mgr
{
	logical is_part_viewed;

protected:
/**
 * @nodoc
 */
//	virtual void set_assembly_ptr_vf( ASM_ASSEMBLY* assembly_ptr );
/**
 * @nodoc
 */
	virtual void entities_changed_vf( outcome& result, asm_event_type& ev_type, asm_event_info* ev_info );
/**
 * @nodoc
 */
	virtual void sub_model_changed_vf( asm_event_type ev_type, asm_model* changed_model );
/**
 * @nodoc
 */
	virtual void done_restoring_vf();
//	virtual void register_restored_entities_vf(ENTITY_LIST& ents);
/**
 * @nodoc
 */
//	virtual void nested_activate_vf(asm_model const* from_model) const;
/**
 * @nodoc
 */
//	virtual void nested_deactivate_vf(asm_model const* to_model) const;
/**
 * @nodoc
 */
	virtual void unbind_vf(asm_model* model);
/**
 * @nodoc
 */
	virtual acis_ha_part_entity_mgr* acis_ha_part_cast_vf();
/**
 * @nodoc
 */
	virtual acis_scm_entity_mgr* acis_scm_cast_vf();
/**
 * @nodoc
 */
	virtual void* ha_part_customer_cast_vf();

public:
	
/**
 * Default constructor.
 * <br><br>
 * <b>Role:</b> Constructs an unbound <tt>acis_ha_part_entity_mgr</tt> object.
 * If a <tt>PART</tt> object is specified, it will be bound to the new entity manager.
 * <br><br>
 * @param part
 * part to be bound.
 */
	acis_ha_part_entity_mgr(PART* part = NULL);

	// overrides of virtual methods

/**
 * Dynamic down-cast to <tt>acis_scm_entity_mgr</tt>.
 * <br><br>
 * <b>Role:</b> Downcasts <tt>this</tt> pointer to an <tt>acis_scm_entity_mgr</tt> if the
 * concrete type of <tt>this</tt> is <tt>acis_scm_entity_mgr</tt> or a sub-class.
 * <br><br>
 * Returns <tt>NULL</tt> if this is not an <tt>acis_scm_entity_mgr</tt>.  This routine must not be overridden by customers.
 */
	acis_scm_entity_mgr* acis_scm_cast();

	void* ha_part_customer_cast();

	void set_viewed(logical is);

	logical is_viewed() const;

};
/*! @} */
#endif // acis_ha_part_entity_mgr_CLASS
