/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#include "entity_converter.h"
#include "ckoutcom.hxx"
#include "ha_bridge.h"
#include "fct_utl.hxx"
#include "acistype.hxx"
#include "face.hxx"
#include "direct_render_mesh_manager.hxx"
#include "get_top.hxx"

direct_render_mesh_manager::direct_render_mesh_manager(hoops_acis_entity_converter* converter,logical ModelGeometryMode,  HA_Map* Map):
	  m_current_node_index(0), m_node_count(0), m_position_coords(NULL),
	  m_normal_coords(NULL), m_triangle_indices(NULL ), m_polygon_count( 0 ),
	  m_converter( converter ), m_ModelGeometryMode( ModelGeometryMode ), m_Map( Map )
	{
	}

direct_render_mesh_manager::direct_render_mesh_manager():
	  m_current_node_index(0), m_node_count(0), m_position_coords(NULL),
	  m_normal_coords(NULL), m_triangle_indices(NULL ), m_polygon_count( 0 ),
	  m_converter( 0 ), m_ModelGeometryMode( 0 ), m_Map( 0 )
{
}
 
direct_render_mesh_manager::~direct_render_mesh_manager()
	 {
		 flush_data();
	 }

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

void direct_render_mesh_manager::flush_data()
{
	m_current_node_index = 0;
	m_node_count = 0;
	m_polynode_count = 0;
	m_polygon_count = 0;
	m_current_index_in_triangles_array = 0;

	if( m_position_coords)
	{
		ACIS_DELETE [] STD_CAST m_position_coords;
		m_position_coords= NULL;
	}
	if( m_normal_coords)
	{
		ACIS_DELETE [] STD_CAST m_normal_coords;
		m_normal_coords= NULL;
	}
	if( m_triangle_indices)
	{
		ACIS_DELETE [] STD_CAST m_triangle_indices;
		m_triangle_indices= NULL;
	}
}

void direct_render_mesh_manager::allocate_arrays()
{
	m_position_coords	= ACIS_NEW float[ 3*m_node_count ];
	m_normal_coords		= ACIS_NEW double[ 3*m_node_count ];
	m_triangle_indices	= ACIS_NEW int[m_polygon_count + m_polynode_count];
}

logical direct_render_mesh_manager::already_visible( ENTITY* this_entity )
{
	int count = HA_Compute_Geometry_Key_Count( this_entity );
	return count > 0 ;
}

void direct_render_mesh_manager::delete_rendered_triangles( ENTITY* this_entity )
{
	HA_Delete_Entity_Geometry( this_entity );
}

void direct_render_mesh_manager::update_entity_display_map( HC_KEY key, ENTITY* this_entity )
{
	if( m_converter )
		m_converter->AddMapping(key, this_entity );
}

void direct_render_mesh_manager::render_triangles( ENTITY* this_entity)
{
	if( is_FACE( this_entity ) )
	{
		FACE* face = (FACE*)this_entity;
		if( face_has_no_render_attrib( face ) )
			return ;
	}
	fp_sentry fps;
	if (m_ModelGeometryMode)
	{
		if (use_asm_highlight_segments.on())
		{
		    HA_KOpen_Segment(this_entity, "entity");
		    // In assembly mode, different parts can share geometry.  To avoid
		    // having duplications, always clear the existing geometry.
			if( m_Map )
				Delete_Geometry_Mappings(m_Map, this_entity);
		    char buffer[64];
		    HA_Build_Segment_String(this_entity, buffer, "entity");
		    HC_Conditional_Style("?Style Library/AcisAsmHighlightFacesEdges", buffer);
	    }
	}

	// note hoops needs a named segment open before you should call Open Color Segment
	logical color_segment_open=OpenColorSegment(this_entity);

	// If this is a double sided face set polygon handeness to none.
	// Guy. Addresses concerns posed in BTS 87839. Use backplane culling instead of no polygon handedness
	if( is_FACE(this_entity ) && ((FACE*)this_entity)->sides() == DOUBLE_SIDED)
		HC_Set_Heuristics( "no backplane cull" );

	HC_KEY key = HC_KInsert_Shell (m_node_count, m_position_coords, m_current_index_in_triangles_array, m_triangle_indices);

	if (key == 0 || key == -1)
		return;

	// Set the normals.
	HC_Open_Geometry(key);
		for (int i = 0; i < m_node_count; i++)
		{
			int x_index = 3*i;
			int y_index = 3*i +1;
			int z_index = 3*i +2;

			HC_Open_Vertex(i);
				HC_Set_Normal( m_normal_coords[x_index],m_normal_coords[y_index], m_normal_coords[z_index] );
			HC_Close_Vertex();
		}
	HC_Close_Geometry();

	update_entity_display_map( key, this_entity );

	if ( progress_meter && !m_already_announced)
		progress_meter->update();

	if (m_ModelGeometryMode)
	{
		if (use_asm_highlight_segments.on())
        {
            HC_Close_Segment();
        }
	}

	if (color_segment_open)
		HC_Close_Segment();

}


	// end renderer specific methods.

 void direct_render_mesh_manager::begin_mesh_output(
	ENTITY *faceted_entity,				//	entity being faceted
	ENTITY *,							//	applicable REFINEMENT, don't care
	ENTITY *							//	output format entity, don't care
)
 {
	 m_already_announced = FALSE;
	 if( already_visible( faceted_entity ) )
	 {
		delete_rendered_triangles( faceted_entity ) ;
		m_already_announced = TRUE;
	 }

	 flush_data(); // make sure any data already in mesh manager is deleted.  (Redundant but better to be sure).
 }

void direct_render_mesh_manager::end_mesh_output(
		ENTITY *faceted_entity,				// entity being faceted
		ENTITY *,							// applicable REFINEMENT, don't care
		ENTITY *							// output format entity, don't care
	)
{
	render_triangles( faceted_entity );
	flush_data();
}

void direct_render_mesh_manager::announce_counts(
		int npoly,		// Number of polygons to follow.
		int nnode,		// Number of nodes to follow.
		int npolynode	// Number of nodes when counted each time
						// they are used by a polygon.
	)
{
	m_current_node_index = 0;
	m_node_count		= nnode;
	m_polynode_count	= npolynode;
	m_polygon_count		= npoly;

	allocate_arrays();
}

logical direct_render_mesh_manager::need_counts()
{
	return TRUE;
}

logical direct_render_mesh_manager::need_indexed_polygons()
{
	return TRUE;
}

	// Return value from this function is used as identifier (index ) for
	// announce_indexed_polynode.
void *direct_render_mesh_manager::announce_indexed_node(
		int ,						// 0-based index of the node
		const SPApar_pos &,			// parametric coordinates
		const SPAposition &iX,		// cartesian coordinates
		const SPAunit_vector &N)	// surface normal
{
	// Layout of information in arrays has m_node_count blocks of
	// 3 numbers. Each block has x first, then y, then z.
	int x_index = 3*m_current_node_index;
	int y_index = 3*m_current_node_index +1;
	int z_index = 3*m_current_node_index +2;

	// copy position information
	m_position_coords[ x_index ] =(float)iX.x();
	m_position_coords[ y_index ] =(float)iX.y();
	m_position_coords[ z_index ] =(float)iX.z();

	// copy normals.
	m_normal_coords[ x_index ] =N.x();
	m_normal_coords[ y_index ] =N.y();
	m_normal_coords[ z_index ] =N.z();

	// for simplicity we have ignored par positions but one could easily copy them here.

	int retval = m_current_node_index;
	m_current_node_index++;
	return (void*)INTEXTEND(retval);
}

void direct_render_mesh_manager::start_indexed_polygon(
	int /*ipoly*/,			// 0-based polygon index
	int npolynode,			// Number of nodes around the polygon
	int /* ishare */		//Which edge of previous poly is shared with this
							//used for opengl orders
	)
{
	m_triangle_indices[ m_current_index_in_triangles_array ] = npolynode;

	m_current_index_in_triangles_array++;
}

void direct_render_mesh_manager::announce_indexed_polynode(
	int ,					// 0-based polygon index. This is the
							//	same as the immediately preceding call to
							//  start_indexed_polygon().
	int ,					// 0-based counter within the polygon.  This
							//	increments sequentially on successive calls.
	void *id				// Node identifer as previously received from
							// announce_indexed_node
	)
{
	size_t this_node_idx = (size_t)id;
	m_triangle_indices[ m_current_index_in_triangles_array ] = (int)this_node_idx;

	m_current_index_in_triangles_array++;
}



void direct_render_mesh_manager::end_indexed_polygon(
	int //ipoly				// 0-based polygon index.  This matches the
							// immediately preceding call to start_indexed_polygon(..)
							// and the (multiple) calls to announce_indexed_polynode(..)
	)
{
}

MESH_MANAGER* get_hoops_direct_render_mesh_manager( hoops_acis_entity_converter* converter, logical m_ModelGeometryMode,  HA_Map* m_Map )
{
	return ACIS_NEW direct_render_mesh_manager( converter, m_ModelGeometryMode, m_Map );
}

void direct_render_entity( hoops_acis_entity_converter* converter,ENTITY* entity, logical m_ModelGeometryMode,  HA_Map* m_Map)
{
	ENTITY_LIST faces;
	ENTITY_LIST unrendered_faces;
	get_faces( entity, faces );
	ENTITY* itr = NULL;
	for( faces.init(); itr=faces.next(); )
	{
		if( HA_Compute_Geometry_Key_Count( itr ) == 0 )
		{
			unrendered_faces.add( itr );
		}
	}

	MESH_MANAGER* old_mm = NULL;
	MESH_MANAGER* direct_mm = get_hoops_direct_render_mesh_manager( converter, m_ModelGeometryMode, m_Map );
	EXCEPTION_BEGIN
	EXCEPTION_TRY
	{
		check_outcome( api_get_mesh_manager( old_mm ) );
		check_outcome( api_set_mesh_manager( direct_mm ) );
		check_outcome( api_facet_entities( entity, &unrendered_faces,HA_Get_Facet_Options() ) );
	}
	EXCEPTION_CATCH_TRUE
		api_set_mesh_manager( old_mm );
		ACIS_DELETE direct_mm;
	EXCEPTION_END
}

