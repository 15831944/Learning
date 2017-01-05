/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HA_REND_OPTIONS_HXX__
#define __HA_REND_OPTIONS_HXX__
#include "dcl_hoops.h"
#include "logical.h"
#include "entity.hxx"
#include "ha_bridge.h"
#define DEF_CURVE_FACET_TOL 0.003
/**
* @file ha_rend_options.h
 * @CAA2Level L1
 * @CAA2Usage U1
 *! \addtogroup HABRIDGEAPI
 *  \brief Declared at <ha_rend_options.h>
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
 * @param cfl_hoops_primitives
 * Display curves as HOOPS primitives when possible.
*/
enum CurveFacetLevel 
{
	cfl_world,			// Facet curves to world units tolerance.
	cfl_screen,			// Facet curves to screen (pixel) tolerance.
	cfl_bounding_box,	// Facet curves to BB diagonal as unit size tolerance.
	cfl_face_facets,	// Facet curves using nodes from faceted faces.
	cfl_hoops_primitives// Display curves as HOOPS primitives when possible.
};
/**
 * Rendering Options
 */
class DECL_HOOPS ha_rendering_options: public ACIS_OBJECT
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
	ha_rendering_options();
    /**
     * Copy Constructor
     */
	ha_rendering_options(const ha_rendering_options &opt)
	{
		m_Pattern = 0;
        m_GeomPattern = 0;
		m_FacetStyle = opt.m_FacetStyle;
		*this = opt;
	}
    /**
     * Destructor
     */
	virtual ~ha_rendering_options()
	{
		if (m_Pattern)
			ACIS_DELETE [] STD_CAST m_Pattern ; m_Pattern = 0;
		if (m_GeomPattern)
			ACIS_DELETE [] STD_CAST m_GeomPattern; m_GeomPattern = 0;
	}
    /**
     * Set iconified state
     */
	void SetIconifiedState(logical);
    /**
     * Set mapping flag
     * @param m
     * New state
     */
	void SetMappingFlag(logical m)					{m_lMappingEnabled = m;}
    /**
     * Get mapping flag
     */
	logical GetMappingFlag()						{return m_lMappingEnabled;}
    /**
     * Set tessellate ellipse mode
     * @param mode
     * mode
     */
	void SetTessellateEllipsesMode( logical mode )	{ m_lTessellateEllipsesMode = mode; }
    /**
     * .
     */
	void SetBodySegmentMode( logical mode )			{ m_lBodySegmentMode = mode; }
    /**
     * .
     */
	void SetColorSegmentMode( logical mode )		{ m_lColorSegmentMode = mode; }
    /**
     * .
     */
	void SetAllRenderModes( logical mode)			{ m_lRenderVerticesMode = 
													  m_lRenderEdgesMode = 
													  m_lRenderFacesMode = 
													  m_lRenderCoedges = 
													  m_lRenderTCoedges = 
													  m_lRenderAPOINTs = 
													  m_lRenderWCSs = 
													  m_lRenderText = 
													  mode; }
    /**
     * .
     */
	void SetRenderVerticesMode( logical mode )		{ m_lRenderVerticesMode = mode; }
    /**
     * .
     */
	void SetRenderEdgesMode( logical mode )			
	{ 
		m_lRenderEdgesMode = mode; 
    /**
     * .
     */
	}
	void SetRenderFacesMode( logical mode )			
	{ 
		m_lRenderFacesMode = mode; 
	}
    /**
     * .
     */
	void SetRenderCoedgeMode(logical mode)			
	{ 
		m_lRenderCoedges = mode; 
	}
    /**
     * .
     */
	void SetRenderTCoedgeMode(logical mode)			
	{ 
		m_lRenderTCoedges = mode; 
	}
    /**
     * .
     */
	void SetRenderAPOINTsMode(logical mode)			{ m_lRenderAPOINTs = mode; }
    /**
     * .
     */
	void SetRenderWCSsMode(logical mode)			{ m_lRenderWCSs = mode; }
    /**
     * .
     */
	void SetRenderTextMode(logical mode)			{ m_lRenderText = mode; }
    /**
     * .
     */
	void SetMergeBodiesMode(logical mode)			{ m_lMergeBodiesMode = mode; }
    /**
     * .
     */
	void SetMergeFacesMode(logical mode)			{ m_lMergeFacesMode = mode; }
    /**
     * .
     */
	void SetPattern(const char *);
    /**
     * .
     */
    void SetGeomPattern(const char *);
    /**
     * .
     */
	void SetFacetStyle(char style)	{ m_FacetStyle = style; }
    /**
     * .
     */
	char GetFacetStyle() const 		{ return m_FacetStyle;	}
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
	logical GetTessellateEllipsesMode( void ) const	{ return m_lTessellateEllipsesMode; }
    /**
     * .
     */
	logical GetBodySegmentMode( void ) const		{ return m_lBodySegmentMode; }
    /**
     * .
     */
	logical GetColorSegmentMode( void ) const		{ return m_lColorSegmentMode; }
    /**
     * .
     */
	logical GetRenderVerticesMode( void ) const		{ return m_lRenderVerticesMode; }
    /**
     * .
     */
	logical GetRenderEdgesMode( void ) const		{ return m_lRenderEdgesMode; }
    /**
     * .
     */
	logical GetRenderFacesMode( void ) const		{ return m_lRenderFacesMode; }
    /**
     * .
     */
	logical RenderEntityWires(ENTITY* entity);
    /**
     * .
     */
	logical GetRenderTCoedgeMode() const			{ return m_lRenderTCoedges; }
    /**
     * .
     */
	logical GetRenderCoedgeMode() const				{ return m_lRenderCoedges; }
    /**
     * .
     */
	logical GetRenderAPOINTsMode() const			{ return m_lRenderAPOINTs; }
    /**
     * .
     */
	logical GetRenderWCSsMode()	const				{ return m_lRenderWCSs; }
    /**
     * .
     */
	logical GetRenderTextMode() const				{ return m_lRenderText; }
    /**
     * .
     */
	logical GetMergeBodiesMode()					{ return m_lMergeBodiesMode; }
    /**
     * .
     */
	logical GetMergeFacesMode()						{ return m_lMergeFacesMode; }
    /**
     * .
     */
	CurveFacetLevel GetCurveFacetLevel() const		{ return m_CurveFacetLevel;}
    /**
     * .
     */
	void SetCurveFacetLevel(CurveFacetLevel cfl)	{ m_CurveFacetLevel = cfl;}
    /**
     * .
     */
	double SetCurveFacetTol(double d)				{ m_curveFacetTol = (d < SPAresabs) ? (double)SPAresabs : d;  return m_curveFacetTol; }
    /**
     * .
     */
	double GetCurveFacetTol() const;
    /**
     * .
     */
	double SetSaveCurveFacetTol(double d)			{ m_saveCurveFacetTol = d;  return m_saveCurveFacetTol; }
    /**
     * .
     */
    double GetSaveCurveFacetTol() const				{ return m_saveCurveFacetTol; }
    /**
     * .
     */
	void Set_Rendering_Options(const char* list);
    /**
     * .
     */
	void Show_Rendering_Options(char* list);
    /**
     * .
     */
	void Show_One_Rendering_Option(const char* type, char* value);
    /**
     * .
     */
	ha_rendering_options& operator |=(const ha_rendering_options& ro);
    /**
     * .
     */
	ha_rendering_options& operator =(const ha_rendering_options& ro);
};
/*! @} */
#endif //__HA_REND_OPTIONS_HXX__
