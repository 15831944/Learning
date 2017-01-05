
// the following is example code only.

#ifndef _direct_render_hps_mesh_manager_hxx
#define _direct_render_hps_mesh_manager_hxx
#include "dcl_hps.h"
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "meshmg.hxx"

class hps_acis_entity_converter;
class ENTITY;
class HPS_Map;

/**
* @nodoc
**/
extern HA_RENDER_progress_info * progress_meter;
/**
* @nodoc
**/
extern option_header use_asm_highlight_segments;
/**
* @nodoc
**/
void Delete_Geometry_Mappings( HPS_Map* map, ENTITY* ent );
/**
* @nodoc
**/
logical face_has_no_render_attrib( FACE* face );

/**
* @nodoc
**/
void direct_render_entity( hps_acis_entity_converter* converter, ENTITY* entity, logical m_ModelGeometryMode, HPS_Map* m_Map );
/**
* @nodoc
**/
DECL_HPS MESH_MANAGER* get_hps_direct_render_mesh_manager( hps_acis_entity_converter* converter, logical m_ModelGeometryMode = FALSE, HPS_Map* m_Map = NULL );

/**
* @nodoc
**/
class DECL_HPS hps_direct_render_mesh_manager : public MESH_MANAGER
{
	int		m_current_node_index;
	int		m_current_index_in_triangles_array;

	int		m_node_count;
	int		m_polynode_count;
	int		m_polygon_count;

	float*	m_position_coords;
	double* m_normal_coords;
	int*	m_triangle_indices;

	hps_acis_entity_converter* m_converter;	//alias only
	logical m_ModelGeometryMode;				// alias only. only used for acis assembly modeling support
	HPS_Map* m_Map;								// alias only. only used for acis assembly modeling support

	logical m_already_announced;				// used to support progress meter.

public:
	hps_direct_render_mesh_manager( hps_acis_entity_converter* converter, logical ModelGeometryMode, HPS_Map* Map );
	hps_direct_render_mesh_manager();

	~hps_direct_render_mesh_manager();

	// the following methods:
	// ** flush_data
	// ** allocate_arrays
	// ** already_visible
	// ** delete_rendered_triangles
	// ** render_triangles
	// depend heavily on the rendering system being used.
	//
	// Depending on the data format used by the rendering system, announce_indexed_node,
	// start_indexed_polygon, announce_indexed_polynode, and end_indexed_polygon,
	// may also require changes.
	//
	// The other methods are reasonably renderer independant.
private:
	void flush_data();

	void allocate_arrays();

	virtual logical already_visible( ENTITY* this_entity );

	virtual void delete_rendered_triangles( ENTITY* this_entity );

	virtual void render_triangles( ENTITY* this_entity );

	virtual void update_entity_display_map( HPS::SegmentKey key, ENTITY* this_entity );

public:
	void begin_mesh_output(
		ENTITY *faceted_entity,				//	entity being faceted
		ENTITY *,							//	applicable REFINEMENT, don't care
		ENTITY *							//	output format entity, don't care
		);

	void end_mesh_output(
		ENTITY *faceted_entity,				// entity being faceted
		ENTITY *,							// applicable REFINEMENT, don't care
		ENTITY *							// output format entity, don't care
		);

	void announce_counts(
		int npoly,		// Number of polygons to follow.
		int nnode,		// Number of nodes to follow.
		int npolynode	// Number of nodes when counted each time
		// they are used by a polygon.
		);

	logical need_counts();

	logical need_indexed_polygons();

	// Return value from this function is used as identifier (index ) for
	// announce_indexed_polynode.
	void *announce_indexed_node(
		int,						// 0-based index of the node
		const SPApar_pos &,			// parametric coordinates
		const SPAposition &iX,		// cartesian coordinates
		const SPAunit_vector &N );	// surface normal

	void start_indexed_polygon(
		int /*ipoly*/,			// 0-based polygon index
		int npolynode,			// Number of nodes around the polygon
		int /* ishare */		//Which edge of previous poly is shared with this
		//used for opengl orders
		);

	void announce_indexed_polynode(
		int,					// 0-based polygon index. This is the
		//	same as the immediately preceding call to
		//  start_indexed_polygon().
		int,					// 0-based counter within the polygon.  This
		//	increments sequentially on successive calls.
		void *id				// Node identifer as previously received from
		// announce_indexed_node
		);



	void end_indexed_polygon(
		int //ipoly				// 0-based polygon index.  This matches the
		// immediately preceding call to start_indexed_polygon(..)
		// and the (multiple) calls to announce_indexed_polynode(..)
		);
};

#endif
