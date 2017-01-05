/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#if !defined( _hps_acis_entity_converter_hxx_)
#define _hps_acis_entity_converter_hxx_hxx_

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.

#include "dcl_hps.h"

#include "hps_ientityconverter.h"
#include "hps_rend_options.h"

#include "position.hxx"
#include "box.hxx"
#include "transf.hxx"

#include "ellipse.hxx"

#include "sg_bs3c.err"    // For FAC_MAX_NPTS    // For FAC_MAX_NPTS

class HPS_Map;
class HPS_CHandleMap;
class HPS_ModelMap;
class hps_rendering_context;
class asm_model;
class component_handle;

class DECL_HPS hps_acis_entity_converter : public IEntityConverter
{
public:

	enum FacetLevel {
		fl_body,
		fl_face
	};

	hps_acis_entity_converter() :
		m_lSilhouette( TRUE ), m_lplane_sil( FALSE ), m_lcone_sil( TRUE ),
		m_lsphere_sil( TRUE ), m_ltorus_sil( TRUE ), m_lspline_sil( TRUE ),
		m_pEntity( 0 ), m_ModelGeometryMode( FALSE )
	{
		m_Map = 0;
		m_CHandleMap = 0;
		m_ModelMap = 0;
		m_EnableMapping = TRUE;
		m_Pattern = 0;
		m_Max_num_pts_so_far = 20;
		m_pEdge_points = ACIS_NEW SPAposition[m_Max_num_pts_so_far];
		m_pParam_vals = ACIS_NEW double[m_Max_num_pts_so_far];
		curve_pixel_tol = -1;
	}

	virtual ~hps_acis_entity_converter()
	{
		// Capture the sys_warning about exceeding the facet limit
		err_mess_type* warnings;
		int nwarnings = get_warnings( warnings );
		if ( nwarnings == 1 && warnings[0] == FAC_MAX_NPTS )
			init_warnings();

		if ( m_pEdge_points )
			ACIS_DELETE[] m_pEdge_points;
		if ( m_pParam_vals )
			ACIS_DELETE[] STD_CAST m_pParam_vals;
	};

	ENTITY*	m_pEntity;				// ENTITY being converted

	int m_Max_num_pts_so_far;
	SPAposition* m_pEdge_points;
	double* m_pParam_vals;
	SPAbox m_box;

	hps_rendering_options m_RenderingOptions;

	//HoopsIndexedMeshManager m_Mesher;

public:

	const char* m_Pattern;

	logical m_ModelGeometryMode;

	logical m_lSilhouette;
	logical m_lplane_sil;
	logical m_lcone_sil;
	logical m_lsphere_sil;
	logical m_ltorus_sil;
	logical m_lspline_sil;

	FacetLevel m_FacetLevel;

	SPAtransf m_BodyTransf;

	virtual void set_Silhouette( const logical& s ) { m_lSilhouette = s; }
	virtual void set_SilhouettePlane( const logical& s ) { m_lplane_sil = s; }
	virtual void set_SilhouetteCone( const logical& s ) { m_lcone_sil = s; }
	virtual void set_SilhouetteSphere( const logical& s ) { m_lsphere_sil = s; }
	virtual void set_SilhouetteTorus( const logical& s ) { m_ltorus_sil = s; }
	virtual void set_SilhouetteSpline( const logical& s ) { m_lspline_sil = s; }

	virtual logical get_Silhouette() const { return m_lSilhouette; }
	virtual logical get_SilhouettePlane() const { return m_lplane_sil; }
	virtual logical get_SilhouetteCone() const { return m_lcone_sil; }
	virtual logical get_SilhouetteSphere() const { return m_lsphere_sil; }
	virtual logical get_SilhouetteTorus() const { return m_ltorus_sil; }
	virtual logical get_SilhouetteSpline() const { return m_lspline_sil; }

	//virtual HPS::SegmentKey ConvertEntity(
	//	ENTITY *						in_entity,
	//	const hps_rendering_options &	in_ro,
	//	HPS_Map *						in_map,
	//	const char *					in_pattern
	//	);
	virtual HPS::SegmentKey ConvertEntity(
		ENTITY *						in_entity,
		const hps_rendering_options &	in_ro,
		HPS_Map *						in_map,
		HPS::SegmentKey					in_segment_key
		);
	virtual HPS::SegmentKey ConvertEntityAsm(
		ENTITY*,
		const hps_rendering_options& ro,
		const hps_rendering_context& rc );

	virtual HPS::SegmentKey get_Key( ENTITY * )
	{
		return HPS_INVALID_SEGMENT_KEY;
	}

	virtual void AddMapping( HPS::Key in_key, ENTITY * in_entity, ENTITY * in_owner = NULL );
	virtual void AddMapping( HPS::Key key, component_handle* comp );
	virtual void AddMapping( HPS::Key key, asm_model* model );
	virtual HPS::Key FindMapping( asm_model* model );
	virtual HPS::Key FindMapping( component_handle* comp );

	virtual HPS::SegmentKey ConvertModelGeometry(
		asm_model*					model,
		const hps_rendering_options&	ro,
		const hps_rendering_context&	rc );
	virtual HPS::SegmentKey ConvertModelComponents(
		component_handle*			comp,
		const hps_rendering_options&	ro,
		const hps_rendering_context&	rc,
		HPS::SegmentKey in_segment_key );

	// Ambient entities are things that affect the view of the model without being part of the model -
	// currently lights, text, points, and WCSs.
	virtual void ConvertAmbientEntity( HPS::SegmentKey CurrentSegment );
	virtual void ConvertLumps( HPS::SegmentKey in_segment_key );
	virtual void ConvertShells( ENTITY * in_entity, HPS::SegmentKey in_segment_key );
	virtual void ConvertFaces( ENTITY * in_entity /*most likely a shell*/, HPS::SegmentKey in_segment_key );
	virtual void ConvertSilhouettes(
		FACE *				in_face,
		const SPAtransf *	in_tform,
		SPAposition &		in_eye,
		SPAposition &		in_target,
		int					in_projection,
		bool				in_mapping,
		HPS::SegmentKey		in_segment_key );
	virtual void ConvertEdges( ENTITY * in_entity, ENTITY * in_owner, HPS::SegmentKey in_segment_key );
	virtual void ConvertCoedges( ENTITY * in_entity, HPS::SegmentKey in_segment_key );
	virtual void ConvertTCoedges( ENTITY * in_entity, HPS::SegmentKey in_segment_key );
	virtual void ConvertCURVE(
		CURVE*			in_CUR,
		ENTITY*			in_ent,
		SPAparameter	in_start_param,
		SPAparameter	in_end_param,
		CurveFacetLevel	in_facet_level,
		ENTITY*			in_owner,
		HPS::SegmentKey	in_segment_key );
	virtual void ConvertELLIPSE(
		ELLIPSE*		in_ellipse,  // Could be actual CURVE or the CURVE of an EDGE.
		ENTITY*			in_ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
		SPAparameter	in_start_param,
		SPAparameter	in_end_param,
		HPS::SegmentKey	in_segment_key );
	virtual void Convert_intcurve(
		CURVE*			in_cur,  // Could be actual CURVE or the CURVE of an EDGE.
		ENTITY*			in_ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
		SPAparameter	in_start_param,
		SPAparameter	in_end_param,
		CurveFacetLevel	in_facet_level,
		HPS::SegmentKey	in_segment_key );
	virtual void ConvertVertices( ENTITY * in_entity, HPS::SegmentKey in_segment_key );
	virtual void ReRenderVisibilityAsm( const hps_rendering_options& in_ro );

protected:
	//	private:
	HPS_Map* m_Map;
	HPS_CHandleMap* m_CHandleMap;
	HPS_ModelMap* m_ModelMap;
	logical m_EnableMapping;
	// unset is -1.  cached to avoid repetitively calling on many edged parts.
	double curve_pixel_tol;

	virtual HPS::Key		MakeHoopsPolyline( int in_num_pts, SPAposition* in_points, bool in_bUseBodyTransf, HPS::SegmentKey in_segment_key );
	virtual HPS::ShellKey	SequentialMeshToHOOPS( HPS::SegmentKey in_segment_key, af_serializable_mesh *in_mesh, FACE * in_face = NULL );
	virtual void			MeshFaces( HPS::SegmentKey in_segment_key, ENTITY* in_entity );

	virtual void	DoubleToFloatArray( int in_num_pts, double * in_doubles, HPS::FloatArray & out_floats );
	void			BuildSegmentPatternString( const ENTITY*entity, const char*pattern, char* outbuffer );
	char*			BuildSegmentString( void* SPAptr, char* inbuffer, const char* pattern );
	int				m_Options;
	logical			m_HasTransform;

private:
	//	virtual HPS::SegmentKey	OpenColorSegment( ENTITY* entity, logical traverse_up = FALSE, HPS::SegmentKey CurSegment = HPS_INVALID_SEGMENT_KEY );
	const SPAtransf&	GetTransfMatrix() const { return( m_BodyTransf ); }
	logical				HasTransform() const { return( m_HasTransform ); }
	void				SetOption( int EnumVal ) { m_Options |= EnumVal; }
	void				RemoveOptions( int EnumVal ) { m_Options &= ~EnumVal; }
	void				ToggleOptions( int EnumVal ) { m_Options ^= EnumVal; }
	logical				HasOptions( int EnumVal ) { return( ( m_Options & EnumVal ) ? TRUE : FALSE ); }
	ENTITY*				GetEntity() { return( m_pEntity ); }

public:
	//	virtual void			Delete_Geometry( ENTITY* pEntity );
};

#endif
