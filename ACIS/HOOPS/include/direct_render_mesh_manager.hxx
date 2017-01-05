/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

// the following is example code only.

#ifndef _direct_render_hoops_mesh_manager_hxx
#define _direct_render_hoops_mesh_manager_hxx
#include "dcl_hoops.h"
#include "meshmg.hxx"

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
void Delete_Geometry_Mappings(HA_Map* map, ENTITY* ent);
/**
* @nodoc
**/
logical face_has_no_render_attrib( FACE* face);

class hoops_acis_entity_converter;
class ENTITY;
class HA_Map;
/**
* @nodoc
**/
void direct_render_entity( hoops_acis_entity_converter* converter,ENTITY* entity, logical m_ModelGeometryMode,  HA_Map* m_Map);
/**
* @nodoc
**/
DECL_HOOPS MESH_MANAGER* get_hoops_direct_render_mesh_manager( hoops_acis_entity_converter* converter, logical m_ModelGeometryMode=FALSE,  HA_Map* m_Map=NULL );

/**
* @nodoc
**/
class DECL_HOOPS direct_render_mesh_manager: public MESH_MANAGER
{
	int		m_current_node_index;
	int		m_current_index_in_triangles_array;

	int		m_node_count;
	int		m_polynode_count;
	int		m_polygon_count;

	float*	m_position_coords;
	double* m_normal_coords;
	int*	m_triangle_indices;

	hoops_acis_entity_converter* m_converter;	//alias only
	logical m_ModelGeometryMode;				// alias only. only used for acis assembly modeling support
	HA_Map* m_Map;								// alias only. only used for acis assembly modeling support

	logical m_already_announced;				// used to support progress meter.

public:
direct_render_mesh_manager(hoops_acis_entity_converter* converter,logical ModelGeometryMode,  HA_Map* Map);
direct_render_mesh_manager();

~direct_render_mesh_manager();

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

virtual void render_triangles( ENTITY* this_entity);

virtual void update_entity_display_map( HC_KEY key, ENTITY* this_entity ); 

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
		int ,						// 0-based index of the node
		const SPApar_pos &,			// parametric coordinates
		const SPAposition &iX,		// cartesian coordinates
		const SPAunit_vector &N);	// surface normal

void start_indexed_polygon(
	int /*ipoly*/,			// 0-based polygon index
	int npolynode,			// Number of nodes around the polygon
	int /* ishare */		//Which edge of previous poly is shared with this
							//used for opengl orders
	);

void announce_indexed_polynode(
	int ,					// 0-based polygon index. This is the
							//	same as the immediately preceding call to
							//  start_indexed_polygon().
	int ,					// 0-based counter within the polygon.  This
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
