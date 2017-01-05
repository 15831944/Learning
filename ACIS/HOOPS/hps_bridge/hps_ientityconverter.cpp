/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#ifdef NT
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif // NT

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "hps_ientityconverter.h"
#include "matrix.hxx"
#include "transf.hxx"
#include "hps_rend_options.h"
#include "hps_rend_context.h"
#include "hps_map.h"
#include "hps_map_asm.h"
#include "asm_assembly.hxx"
#include "asm_model.hxx"
#include "asm_model_ref.hxx"
#include "hps_bridge.err"
#include "asm_api.hxx"


//HPS::SegmentKey IEntityConverter::ConvertEntity(
//	ENTITY*						entity,
//	const hps_rendering_options&	ro,
//	HPS_Map*						map,
//	const char*					pat )
//{
//	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;
//
//	if ( is_ASM_ASSEMBLY( entity ) || is_ASM_MODEL_REF( entity ) )
//	{
//		sys_error( HPS_MSG_ASM_NOT_SUP );
//	} else
//	{
//		m_pEntity = entity;
//		m_Pattern = pat;
//	}
//
//	return answer;
//}

HPS::SegmentKey IEntityConverter::ConvertEntity(
	ENTITY*						entity,
	const hps_rendering_options&	ro,
	HPS_Map*						map,
	HPS::SegmentKey				in_segment_key )
{
	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;

	if ( is_ASM_ASSEMBLY( entity ) || is_ASM_MODEL_REF( entity ) )
		sys_error( HPS_MSG_ASM_NOT_SUP );
	else
		m_pEntity = entity;
	return answer;
}

HPS::SegmentKey IEntityConverter::ConvertEntityAsm(
	ENTITY*						entity,
	const hps_rendering_options& ro,
	const hps_rendering_context& rc )
{
	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;

	if ( is_ASM_ASSEMBLY( entity ) || is_ASM_MODEL_REF( entity ) )
	{
		sys_error( HPS_MSG_ASM_NOT_OVR );
	} else
	{
		answer = ConvertEntity( entity, ro, rc.GetEntityMap(), HPS_Open_Segment( entity, rc.GetPattern() ) );
	}

	return answer;
}

HPS::SegmentKey IEntityConverter::ConvertModelGeometry(
	asm_model*					model,
	const hps_rendering_options& ro,
	const hps_rendering_context& rc )
{

	HPS::SegmentKey answer = HPS_INVALID_SEGMENT_KEY;

	MODEL_BEGIN( model )

		ENTITY_LIST ents;
	model->get_top_level_entities( ents, FALSE );
	ENTITY* this_ent = NULL;

	for ( this_ent = ents.first(); this_ent; this_ent = ents.next() )
	{
		ConvertEntityAsm( this_ent, ro, rc );
	}

	MODEL_END( ASM_NO_CHANGE )

	if ( ( (hps_rendering_options&)ro ).GetMappingFlag() )
	{
		HPS_ModelMap* model_map = rc.GetModelMap();
		if ( model_map )
			answer = HPS_Cast_SegmentKey( model_map->FindMapping( model ) );
	}

	return answer;
}

HPS::SegmentKey IEntityConverter::ConvertModelComponents(
	component_handle*,
	const hps_rendering_options&,
	const hps_rendering_context&,
	HPS::SegmentKey in_segment_key )
{
	return HPS_INVALID_SEGMENT_KEY;
}

void IEntityConverter::TransfToModelingMatrix( const SPAtransf &tform, float *float_mat )
{
	if ( !float_mat )
		return;

	SPAmatrix mat = tform.affine();

	float_mat[0] = (float)( mat.element( 0, 0 ) * tform.scaling() );
	float_mat[1] = (float)( mat.element( 0, 1 ) * tform.scaling() );
	float_mat[2] = (float)( mat.element( 0, 2 ) * tform.scaling() );
	float_mat[3] = (float)( 0.0 );
	float_mat[4] = (float)( mat.element( 1, 0 ) * tform.scaling() );
	float_mat[5] = (float)( mat.element( 1, 1 ) * tform.scaling() );
	float_mat[6] = (float)( mat.element( 1, 2 ) * tform.scaling() );
	float_mat[7] = (float)( 0.0 );
	float_mat[8] = (float)( mat.element( 2, 0 ) * tform.scaling() );
	float_mat[9] = (float)( mat.element( 2, 1 ) * tform.scaling() );
	float_mat[10] = (float)( mat.element( 2, 2 ) * tform.scaling() );
	float_mat[11] = (float)( 0.0 );

	SPAvector trans = tform.translation();
	float_mat[12] = (float)( trans.x() );
	float_mat[13] = (float)( trans.y() );
	float_mat[14] = (float)( trans.z() );

	float_mat[15] = (float)( 1.0 );
}
