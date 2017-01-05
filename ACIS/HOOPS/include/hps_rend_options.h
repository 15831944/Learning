/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_REND_OPTIONS_HXX__
#define __HPS_REND_OPTIONS_HXX__
#include "dcl_hps.h"
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "logical.h"
#include "entity.hxx"
#include "hps_bridge.h"
#define DEF_CURVE_FACET_TOL 0.003
/**
* @file hps_rend_options.h
* @CAA2Level L1
* @CAA2Usage U1
*! \addtogroup HPSBRIDGEAPI
*  \brief Declared at <hps_rend_options.h>
*  @{
*/

/**
 * Curve Facetting Level.
 * @param cfl_world
 * Facet curves to world units tolerance.
 * @param cfl_screen
 * Facet curves to screen (pixel) tolerance.
 * @param cfl_bounding_box
 * Facet curves to BB diagonal as unit size tolerance.
 * @param cfl_face_facets
 * Facet curves using nodes from faceted faces.
 * @param cfl_hps_primitives
 * Display curves as HOOPS primitives when possible.
 */
enum CurveFacetLevel
{
	cfl_world,			// Facet curves to world units tolerance.
	cfl_screen,			// Facet curves to screen (pixel) tolerance.
	cfl_bounding_box,	// Facet curves to BB diagonal as unit size tolerance.
	cfl_face_facets,	// Facet curves using nodes from faceted faces.
	cfl_hps_primitives// Display curves as HOOPS primitives when possible.
};
/**
 * Rendering Options
 */
class DECL_HPS hps_rendering_options : public ACIS_OBJECT
{
	logical m_lMappingEnabled;	// Indicates whether to map the converted ENTITYs.  
	// Mapping may not make sense when temporarily displaying 
	// the ENTITY- i.e. TempRenderEntity in Scheme AIDE.
	logical m_lColorSegmentMode;
	logical m_lBodySegmentMode;
	logical m_lRenderVerticesMode;
	logical m_lRenderEdgesMode;
	logical m_lRenderFacesMode;
	logical m_lRenderTCoedges;
	logical m_lRenderCoedges;
	logical m_lRenderAPOINTs;
	logical m_lRenderWCSs;
	logical m_lRenderText;

	logical m_lTessellateEllipsesMode;

	logical m_lMergeBodiesMode;
	logical m_lMergeFacesMode;

	logical m_view_is_iconified;

	double m_curveFacetTol;
	double m_saveCurveFacetTol;
	CurveFacetLevel m_CurveFacetLevel;

	char* m_Pattern;
	char* m_GeomPattern;
	char  m_FacetStyle;  // Facets lines colors are: 0 == same color as FACE, 1 == complimentary of FACE color

public:

	/**
	 * Constructor
	 */
	hps_rendering_options();
	/**
	 * Copy Constructor
	 */
	hps_rendering_options( const hps_rendering_options &in_rendering_options )
	{
		m_Pattern = 0;
		m_GeomPattern = 0;
		m_FacetStyle = in_rendering_options.m_FacetStyle;
		*this = in_rendering_options;
	}
	/**
	 * Destructor
	 */
	virtual ~hps_rendering_options()
	{
		if ( m_Pattern )
			ACIS_DELETE[] STD_CAST m_Pattern; m_Pattern = 0;
		if ( m_GeomPattern )
			ACIS_DELETE[] STD_CAST m_GeomPattern; m_GeomPattern = 0;
	}
	/**
	 * Set iconified state
	 */
	void SetIconifiedState( logical in_iconify );
	/**
	 * Set mapping flag
	 * @param m
	 * New state
	 */
	void SetMappingFlag( logical in_mapping_enabled ) { m_lMappingEnabled = in_mapping_enabled; }
	/**
	 * Get mapping flag
	 */
	logical GetMappingFlag() { return m_lMappingEnabled; }
	/**
	 * Set tessellate ellipse mode
	 * @param mode
	 * mode
	 */
	void SetTessellateEllipsesMode( logical in_mode ) { m_lTessellateEllipsesMode = in_mode; }
	/**
	 * .
	 */
	void SetBodySegmentMode( logical in_mode ) { m_lBodySegmentMode = in_mode; }
	/**
	 * .
	 */
	void SetColorSegmentMode( logical in_mode ) { m_lColorSegmentMode = in_mode; }
	/**
	 * .
	 */
	void SetAllRenderModes( logical in_mode )
	{
		m_lRenderVerticesMode =
			m_lRenderEdgesMode =
			m_lRenderFacesMode =
			m_lRenderCoedges =
			m_lRenderTCoedges =
			m_lRenderAPOINTs =
			m_lRenderWCSs =
			m_lRenderText =
			in_mode;
	}
	/**
	 * .
	 */
	void SetRenderVerticesMode( logical in_mode ) { m_lRenderVerticesMode = in_mode; }
	/**
	 * .
	 */
	void SetRenderEdgesMode( logical in_mode )
	{
		m_lRenderEdgesMode = in_mode;
		/**
		 * .
		 */
	}
	void SetRenderFacesMode( logical in_mode )
	{
		m_lRenderFacesMode = in_mode;
	}
	/**
	 * .
	 */
	void SetRenderCoedgeMode( logical in_mode )
	{
		m_lRenderCoedges = in_mode;
	}
	/**
	 * .
	 */
	void SetRenderTCoedgeMode( logical in_mode )
	{
		m_lRenderTCoedges = in_mode;
	}
	/**
	 * .
	 */
	void SetRenderAPOINTsMode( logical in_mode ) { m_lRenderAPOINTs = in_mode; }
	/**
	 * .
	 */
	void SetRenderWCSsMode( logical in_mode ) { m_lRenderWCSs = in_mode; }
	/**
	 * .
	 */
	void SetRenderTextMode( logical in_mode ) { m_lRenderText = in_mode; }
	/**
	 * .
	 */
	void SetMergeBodiesMode( logical in_mode ) { m_lMergeBodiesMode = in_mode; }
	/**
	 * .
	 */
	void SetMergeFacesMode( logical in_mode ) { m_lMergeFacesMode = in_mode; }
	/**
	 * .
	 */
	void SetPattern( const char * in_pattern );
	/**
	 * .
	 */
	void SetGeomPattern( const char * in_pattern );
	/**
	 * .
	 */
	void SetFacetStyle( char style ) { m_FacetStyle = style; }
	/**
	 * .
	 */
	char GetFacetStyle() const { return m_FacetStyle; }
	/**
	 * .
	 */
	const char* GetPattern() const
	{
		return m_Pattern;
	}
	/**
	 * .
	 */
	const char* GetGeomPattern() const
	{
		return m_GeomPattern;
	}
	/**
	 * .
	 */
	logical GetTessellateEllipsesMode( void ) const { return m_lTessellateEllipsesMode; }
	/**
	 * .
	 */
	logical GetBodySegmentMode( void ) const { return m_lBodySegmentMode; }
	/**
	 * .
	 */
	logical GetColorSegmentMode( void ) const { return m_lColorSegmentMode; }
	/**
	 * .
	 */
	logical GetRenderVerticesMode( void ) const { return m_lRenderVerticesMode; }
	/**
	 * .
	 */
	logical GetRenderEdgesMode( void ) const { return m_lRenderEdgesMode; }
	/**
	 * .
	 */
	logical GetRenderFacesMode( void ) const { return m_lRenderFacesMode; }
	/**
	 * .
	 */
	logical RenderEntityWires( ENTITY* in_entity );
	/**
	 * .
	 */
	logical GetRenderTCoedgeMode() const { return m_lRenderTCoedges; }
	/**
	 * .
	 */
	logical GetRenderCoedgeMode() const { return m_lRenderCoedges; }
	/**
	 * .
	 */
	logical GetRenderAPOINTsMode() const { return m_lRenderAPOINTs; }
	/**
	 * .
	 */
	logical GetRenderWCSsMode()	const { return m_lRenderWCSs; }
	/**
	 * .
	 */
	logical GetRenderTextMode() const { return m_lRenderText; }
	/**
	 * .
	 */
	logical GetMergeBodiesMode() { return m_lMergeBodiesMode; }
	/**
	 * .
	 */
	logical GetMergeFacesMode() { return m_lMergeFacesMode; }
	/**
	 * .
	 */
	CurveFacetLevel GetCurveFacetLevel() const { return m_CurveFacetLevel; }
	/**
	 * .
	 */
	void SetCurveFacetLevel( CurveFacetLevel in_cfl ) { m_CurveFacetLevel = in_cfl; }
	/**
	 * .
	 */
	double SetCurveFacetTol( double in_tol ) { m_curveFacetTol = ( in_tol < SPAresabs ) ? (double)SPAresabs : in_tol;  return m_curveFacetTol; }
	/**
	 * .
	 */
	double GetCurveFacetTol() const;
	/**
	 * .
	 */
	double SetSaveCurveFacetTol( double in_tol ) { m_saveCurveFacetTol = in_tol;  return m_saveCurveFacetTol; }
	/**
	 * .
	 */
	double GetSaveCurveFacetTol() const { return m_saveCurveFacetTol; }
	/**
	 * .
	 */
	void Set_Rendering_Options( const char* in_list );
	/**
	 * .
	 */
	void Show_Rendering_Options( char* out_list );
	/**
	 * .
	 */
	void Show_One_Rendering_Option( const char* in_type, char* out_text );
	/**
	 * .
	 */
	hps_rendering_options& operator |=( const hps_rendering_options& in_rendering_options );
	/**
	 * .
	 */
	hps_rendering_options& operator =( const hps_rendering_options& in_rendering_options );
};
/*! @} */
#endif //__HPS_REND_OPTIONS_HXX__
