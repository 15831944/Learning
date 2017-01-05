/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#ifndef acis_ha_part_entity_mgr_FACTORY_CLASS
#define acis_ha_part_entity_mgr_FACTORY_CLASS

#include "dcl_ha_part.h"
#include "acis_pm_entity_mgr_factory.hxx"

class ENTITY_LIST;
class asm_model_entity_mgr;
class asm_model_info;

/**
 * @file acis_ha_part_entity_mgr_factory.hxx
 * @CAA2Level L1
 * @CAA2Usage U1
 *! \addtogroup HABRIDGEAPI
 *  \brief Declared at <acis_ha_part_entity_mgr_factory.hxx>
 *  @{
 */

/**
 * Concrete entity manager factory class for creating <tt>acis_scm_entity_mgr</tt> objects.
 * <br><br>
 * <b>Role:</b> The <tt>acis_scm_entity_mgr</tt> (and its corresponding factory) is used to
 * integrate ACIS Assembly Modeling with the Scheme example application.  The implementations of <tt>acis_scm_entity_mgr</tt>
 * and <tt>acis_ha_part_entity_mgr_factory</tt> are provided as example code.  
 * Please refer to the assembly modeling technical articles for further details.
 * <br>
 * @see acis_pm_entity_mgr, acis_scm_entity_mgr
 */

class DECL_HA_PART acis_ha_part_entity_mgr_factory : public acis_pm_entity_mgr_factory
{
protected:
/**
 * @nodoc
 */
	virtual asm_model_entity_mgr* create_entity_mgr_vf(asm_model_info const& model_info, 
	                                                   HISTORY_STREAM* share_stream = NULL); 
public:
/**
 * Default constructor.
 * <br><br>
 * <b>Role:</b> Constructs an <tt>acis_ha_part_entity_mgr_factory</tt> object.  A list of <tt>PART</tt>
 * objects may be passed in; the factory will check the list for empty parts that
 * are not already bound to an entity manager and re-use those, if any.
 * <br><br>
 * @param seed_parts
 * list of seed parts to check for re-use.
 */
	acis_ha_part_entity_mgr_factory(const VOID_LIST& seed_parts = *(VOID_LIST*)NULL_REF);
};

/*! @} */
#endif // acis_ha_part_entity_mgr_FACTORY_CLASS
