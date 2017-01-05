/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#if !defined( _hoops_acis_entity_converter_hxx_)
#define _hoops_acis_entity_converter_hxx_hxx_

#include <hc.h>

#include "dcl_hoops.h"

#include "ha_rend_options.h"
#include "ientityconverter.h"

#include "position.hxx"
#include "box.hxx"
#include "transf.hxx"

#include "ellipse.hxx"

#include "sg_bs3c.err"    // For FAC_MAX_NPTS    // For FAC_MAX_NPTS

class HA_Map;
class HA_CHandleMap;
class HA_ModelMap;
class ha_rendering_context;
class asm_model;
class component_handle;

class DECL_HOOPS hoops_acis_entity_converter : public IEntityConverter
{
public:

enum FacetLevel {
	fl_body,
	fl_face		
};

	hoops_acis_entity_converter():
						m_lSilhouette(TRUE), m_lplane_sil(FALSE), m_lcone_sil(TRUE),
						m_lsphere_sil(TRUE), m_ltorus_sil(TRUE), m_lspline_sil(TRUE),
						m_pEntity(0), m_ModelGeometryMode(FALSE)
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

	virtual ~hoops_acis_entity_converter()
	{
		// Capture the sys_warning about exceeding the facet limit
		err_mess_type* warnings;
		int nwarnings = get_warnings( warnings );
		if (nwarnings == 1 && warnings[0] == FAC_MAX_NPTS)
			init_warnings();

		if (m_pEdge_points)
			ACIS_DELETE [] m_pEdge_points;
		if (m_pParam_vals)
			ACIS_DELETE [] STD_CAST m_pParam_vals;
	};

	ENTITY*	m_pEntity;				// ENTITY being converted

	int m_Max_num_pts_so_far;
	SPAposition* m_pEdge_points;
	double* m_pParam_vals;
	SPAbox m_box;

	ha_rendering_options m_RenderingOptions;

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

	virtual void set_Silhouette(const logical& s)		{m_lSilhouette = s;}
	virtual void set_SilhouettePlane(const logical& s)	{m_lplane_sil = s;}
	virtual void set_SilhouetteCone(const logical& s)	{m_lcone_sil = s;}
	virtual void set_SilhouetteSphere(const logical& s)	{m_lsphere_sil = s;}
	virtual void set_SilhouetteTorus(const logical& s)	{m_ltorus_sil = s;}
	virtual void set_SilhouetteSpline(const logical& s)	{m_lspline_sil = s;}

	virtual logical get_Silhouette() const			{return m_lSilhouette;}
	virtual logical get_SilhouettePlane() const		{return m_lplane_sil;}
	virtual logical get_SilhouetteCone() const		{return m_lcone_sil;}
	virtual logical get_SilhouetteSphere() const	{return m_lsphere_sil;}
	virtual logical get_SilhouetteTorus() const		{return m_ltorus_sil;}
	virtual logical get_SilhouetteSpline() const	{return m_lspline_sil;}

	HC_KEY ConvertEntity(
		ENTITY*,
		const ha_rendering_options &ro,
		HA_Map *map,
		const char* pattern);

	HC_KEY ConvertEntityAsm(
		ENTITY*,
		const ha_rendering_options& ro,
		const ha_rendering_context& rc);

	virtual HC_KEY get_Key(ENTITY *)
	{
		return 0;
	}

	void AddMapping(HC_KEY key, ENTITY* entity, ENTITY* owner = NULL);
	void AddMapping(HC_KEY key, component_handle* comp);
	void AddMapping(HC_KEY key, asm_model* model);
	HC_KEY FindMapping(asm_model* model);
	HC_KEY FindMapping(component_handle* comp);

	virtual HC_KEY ConvertModelGeometry(
		asm_model*					model,
		const ha_rendering_options&	ro,
		const ha_rendering_context&	rc);
	virtual HC_KEY ConvertModelComponents(
		component_handle*			comp,
		const ha_rendering_options&	ro,
		const ha_rendering_context&	rc);

	// Ambient entities are things that affect the view of the model without being part of the model -
	// currently lights, text, points, and WCSs.
	virtual void ConvertAmbientEntity();
	virtual void ConvertLumps();
	virtual void ConvertShells(ENTITY*);
	virtual void ConvertFaces(ENTITY*);
	virtual void ConvertSilhouettes(
						FACE*						face, 
						const SPAtransf*			tform,
						SPAposition&				eye,
						SPAposition&				target,
						int							projection,
						bool						mapping = FALSE);
	virtual void ConvertEdges(ENTITY*, ENTITY* owner = NULL);
	virtual void ConvertCoedges(ENTITY*);
	virtual void ConvertTCoedges(ENTITY*);
	virtual void ConvertCURVE(
						CURVE*			cur,  // Could be actual CURVE or the CURVE of an EDGE.
						ENTITY*			ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
						SPAparameter	start_param, 
						SPAparameter	end_param, 
						CurveFacetLevel	facet_level,
						ENTITY*			owner = NULL);
	virtual void ConvertELLIPSE(
						ELLIPSE*		cur,  // Could be actual CURVE or the CURVE of an EDGE.
						ENTITY*			ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
						SPAparameter	start_param, 
						SPAparameter	end_param);
	virtual void Convert_intcurve(
						CURVE*			cur,  // Could be actual CURVE or the CURVE of an EDGE.
						ENTITY*			ent,	// Actual entity to add mappings to- could be EDGE||CURVE.
						SPAparameter	start_param, 
						SPAparameter	end_param, 
						CurveFacetLevel	facet_level);
	virtual void ConvertVertices(ENTITY*);
	virtual void ReRenderVisibilityAsm(const ha_rendering_options& ro);

protected:
//	private:
	HA_Map* m_Map;
	HA_CHandleMap* m_CHandleMap;
	HA_ModelMap* m_ModelMap;
	logical m_EnableMapping;
	// unset is -1.  cached to avoid repetitively calling on many edged parts.
	double curve_pixel_tol;

	virtual HC_KEY MakeHoopsPolyline(int num_points, SPAposition* points);
};


#endif
