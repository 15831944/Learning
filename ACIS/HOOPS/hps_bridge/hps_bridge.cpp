/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// Sep-10-15 grf : Copied code from SPAha_bridge and changed it to use HPS
/*******************************************************************/
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.

#include <ctype.h> // needed for tolower()
#include <assert.h>

#ifdef NT
#pragma warning( disable : 4786 )
#endif // NT

#include "acis.hxx"
#include "af_api.hxx"
#include "kernapi.hxx"
#include "api.err"
#include "transfrm.hxx"
#include "ckoutcom.hxx"
#include "bulletin.hxx"
#include "curve.hxx"
#include "pcurve.hxx"
#include "surface.hxx"
#include "face.hxx"
#include "edge.hxx"
#include "wire.hxx"
#include "body.hxx"
#include "surface.hxx"
#include "vertex.hxx"
#include "point.hxx"
#include "initkern.hxx"
#include "acistype.hxx"  
#include "getowner.hxx"
#include "lists.hxx"    
#include "logical.h"
#include "vector_utils.hxx"
#include "ga_api.hxx"
#include "rnd_api.hxx"  
#include "init_rb.hxx"
#include "handlers.hxx"
#include "ptlist.hxx"
#include "facet.err"
#include "initintr.hxx"
#include "intrapi.hxx"  
#include "entity.hxx"    
#include "hist_cb.hxx"
#include "asm_assembly.hxx"
#include "asm_model_ref.hxx"
#include "asm_api.hxx"
#include "init_ga.hxx"
#include "mutex.hxx"
#include "no_rend.hxx"
#include "alltop.hxx"
#include "fct_utl.hxx"
#include "point_cloud.hxx"
#include "SPAposition_cloud.hxx"
#include "b_strutl.hxx"
#include "hps_map.h"
#include "hps_bridge.h"
#include "hps_ientityconverter.h"
#include "hps_bridge_internal.h"
#include "hps_map.h"
#include "hps_util.h"
#include "hps_entity_converter.h"
#include "hps_rend_options.h"
#include "hps_map_asm.h"
#include "hps_rend_context.h"
#include "hps_bridge_state.hxx"
#include "mutex.hxx"

#ifndef NT
#include <strings.h>
#define stricmp(x,y) strcasecmp(x,y)
#endif

typedef struct hapoint {
	float	x, y, z;
} HAPoint;
typedef struct hapoint HAVector;

HPS::SegmentKey		HPS_Include_Library;
HPS::SegmentKey		HPS_Style_Library;
HPS::PortfolioKey	HPS_Portfolio;
HPS::Key			HPS_INVALID_KEY;
HPS::SegmentKey		HPS_INVALID_SEGMENT_KEY;

#define MAX_TYPE_NAME_LENGTH		32

#ifdef THREAD_SAFE_ACIS
SESSION_LOCAL_VAR safe_integral_type<int> init_count;
#else
SESSION_LOCAL_VAR int init_count;
#endif

// kev. Needed a way to call HPS_Init once for the session but
// had to ensure it happened after dependent components had been
// initialized. Couldn't use an instance callback's eSessionInit 
// because it is fired before the dependent components are 
// initialized.
SESSION_LOCAL_VAR int one_hps_init_term_per_session;

// Only one for session - init'ed in call to HPS_Init.
SESSION_LOCAL_VAR hps_state* local_state;

hps_state* get_hps_state()
{
	return local_state;
}

extern mutex_resource hps_bridge_mutex;

#include "comp_handle.hxx"
extern component_handle_list s_HPS_CHandles;

//----------------------------------------------------------------------
// purpose---
//    Initialise the hps_acis_bridge
//----------------------------------------------------------------------

logical initialize_hps_acis_bridge()
{
	logical init_ok = TRUE;
	if ( init_count++ == 0 )
	{
		init_ok &= initialize_faceter();
		init_ok &= initialize_rendering();
		init_ok &= initialize_generic_attributes();
	}
	// kev. Only call HPS_Init once per session (for all threads).
	// Do so after the above components have been initialized.
	{
		CRITICAL_BLOCK( hps_bridge_mutex );
		if ( one_hps_init_term_per_session++ == 0 )
			HPS_Init( 0 );
	}
	return init_ok;
}

outcome api_initialize_hps_acis_bridge()
{
	return outcome( initialize_hps_acis_bridge() ? 0 : API_FAILED );
}

//----------------------------------------------------------------------
// purpose---
//    Shut down the hps_acis_bridge
//----------------------------------------------------------------------
logical terminate_hps_acis_bridge()
{
	logical ok = TRUE;
	// kev. Only call HPS_Close once per session (for all threads).
	// Do so before the below components have been terminated.
	{
		CRITICAL_BLOCK( hps_bridge_mutex );
		if ( --one_hps_init_term_per_session == 0 )
			HPS_Close();
	}
	if ( init_count == 0 )
	{
		ok = FALSE;
	} else if ( --init_count == 0 )
	{
		ok &= terminate_faceter();
		ok &= terminate_rendering();
		ok &= terminate_generic_attributes();
	}
	return ok;
}

outcome api_terminate_hps_acis_bridge()
{
	return outcome( terminate_hps_acis_bridge() ? 0 : API_FAILED );
}

// Jeff Aug09 Moved history callback responsibilities into hps_part.
#if 0
class HPS_History_Callbacks : public history_callbacks
{
	// declare any data related to handling roll.  
public:
	void Before_Roll_States()
	{
		// Do your before roll stuff for a group of DELTA_STATEs
	}
	void Before_Roll_State( DELTA_STATE* )
	{
		// Do your before roll stuff for one DELTA_STATE;
	}
	void get_changed_entitys( BULLETIN_BOARD* bb, ENTITY_LIST &elist )
	{
		for ( BULLETIN *b = bb->start_b; b != NULL; b = b->next_ptr )
		{
			ENTITY* e2 = b->new_entity_ptr();
			ENTITY* e3 = b->old_entity_ptr();
			ENTITY *owner2, *owner3;
			api_get_owner( e2, owner2 );
			api_get_owner( e3, owner3 );
			if ( owner2 && ( !owner3 || owner2 == owner3 ) )
			{
				logical add = FALSE;
				ENTITY_LIST owners;

				if ( is_APOINT( owner2 ) )
				{
					if ( !( (APOINT*)owner2 )->get_owners( owners ) )
						add = TRUE;
				} else if ( is_CURVE( owner2 ) )
				{
					if ( !( (CURVE*)owner2 )->get_owners( owners ) )
						add = TRUE;
				} else if ( is_PCURVE( owner2 ) )
				{
					if ( !( (PCURVE*)owner2 )->get_owners( owners ) )
						add = TRUE;
				} else if ( is_SURFACE( owner2 ) )
				{
					if ( !( (SURFACE*)owner2 )->get_owners( owners ) )
						add = TRUE;
				} else if ( is_toplevel( owner2 ) )
					add = TRUE;

				if ( add )
					elist.add( owner2 );
			} else if ( owner2 && owner2 == owner3 )
			{
				if ( is_toplevel( owner2 ) )
					elist.add( owner2 );
			}
		}
	}

	void Before_Roll_Bulletin_Board( BULLETIN_BOARD* bb, logical discard )
	{
		// Do your before roll stuff for one BULLETIN_BOARD
		// if discard is true, the  roll is due to  error processing and the 
		// BULLETIN_BOARD will be deleted along with all its BULLETINS.

		if ( discard )
			return;

		API_NOP_BEGIN
			ENTITY_LIST DelEntities;
		get_changed_entitys( bb, DelEntities );
		HPS_Delete_Entity_Geometry( DelEntities );
		API_NOP_END
	}

	void After_Roll_Bulletin_Board( BULLETIN_BOARD* bb, logical discard )
	{
		// Do your after roll stuff for one BULLETIN_BOARD;
		// if discard is true, the  roll is due to  error processing and the 
		// BULLETIN_BOARD will be deleted along with all its BULLETINS.

		if ( discard )
			return;

		API_NOP_BEGIN
			ENTITY_LIST NewEntities;
		get_changed_entitys( bb, NewEntities );
		HPS_Render_Entities( NewEntities );
		API_NOP_END
	}

	void After_Roll_State( DELTA_STATE* )
	{
		// Do your after roll stuff for one DELTA_STATE;
	}

	void After_Roll_States()
	{
		// Do your after roll stuff for a group of DELTA_STATES
	}
};
#endif

logical analyze_init_options( const char* in_list )
{
	if ( !in_list )
		return TRUE;
	char token[1025];
	char llist[1025];
	unsigned long i;
	unsigned long token_number = 0;
	/*my canonize chars*/
	for ( i = 0; i < 1024; i++ )
	{
		if ( !in_list[i] ) break;
		llist[i] = (char)tolower( (int)in_list[i] );
	}
	llist[i] = 0;
	/*loop through options*/
	while ( HPS_Parse_String( llist, ",", token_number++, token ) )
	{
		HPS::Database::GetEventDispatcher().InjectEvent( HPS::ErrorEvent( "HPS_Init:  Null option or unknown option" ) );
		return FALSE;
	}
	return TRUE;
}

extern SESSION_GLOBAL_VAR option_header use_asm_highlight_segments;

void HPS_Init( const char* in_options )
{
	local_state = ACIS_NEW hps_state;
	get_hps_state()->s_pHPS_Map = ACIS_NEW HPS_Map;
	get_hps_state()->s_pHPS_MapAsm = ACIS_NEW HPS_Map;
	get_hps_state()->s_pHPS_ModelMap = ACIS_NEW HPS_ModelMap;
	get_hps_state()->s_pHPS_CHandleMap = ACIS_NEW HPS_CHandleMap;
	get_hps_state()->s_pFacetOptions = ACIS_NEW facet_options_visualization;
	analyze_init_options( in_options );
	// Jeff Aug09 Moved history callback responsibilities into hps_part.
	//HPS_History_Callbacks *cb = ACIS_NEW HPS_History_Callbacks;
	//get_history_callbacks_list().add(cb);

	// Mimic the old 3DF system for now...
	//HPS_Include_Library = HPS::Database::CreateRootSegment();

	// create a portfolio to store styles and the texture
	HPS_Style_Library = HPS::Database::CreateRootSegment().SetName( "Style Library" );
	HPS_Portfolio = HPS::Database::CreatePortfolio();

	// Set the ENTITY converter.
	HPS_Set_Entity_Converter( ACIS_NEW hps_acis_entity_converter );
	HPS::SegmentKey key;

	key = HPS_Style_Library.Subsegment().SetName( "AcisShellsOn" );
	key.GetVisibilityControl().SetFaces( true ).SetLines( false ).SetMarkers( false );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisShellsOffSilsOn" );
	key.GetVisibilityControl().SetFaces( false ).SetLines( false ).SetMarkers( false ).SetInteriorSilhouetteEdges( true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisShellsOffSilsOff" );
	key.GetVisibilityControl().SetFaces( false ).SetLines( false ).SetLines( false ).SetMarkers( false ).SetInteriorSilhouetteEdges( false );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisPolylinesOn" );
	key.GetVisibilityControl().SetLines( true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisPolylinesOff" );
	key.GetVisibilityControl().SetLines( false );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisMarkersOn" );
	key.GetVisibilityControl().SetMarkers( true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisMarkersOff" );
	key.GetVisibilityControl().SetMarkers( false );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	if ( use_asm_highlight_segments.on() )
	{
		key = HPS_Style_Library.Subsegment().SetName( "AcisAsmHighlightFacesEdges" );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Everything, false );
		key.GetLineAttributeControl().SetWeight( 3.0 );
		key.GetVisibilityControl().SetFaces( true ).SetLines( true ).SetMarkers( false );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility, true );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

		key = HPS_Style_Library.Subsegment().SetName( "AcisAsmHighlightVerts" );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Everything, false );
		key.GetLineAttributeControl().SetWeight( 3.0 );
		key.GetMarkerAttributeControl().SetSize( 1.25 );
		key.GetVisibilityControl().SetFaces( true ).SetLines( true ).SetMarkers( true );
	} else
	{
		key = HPS_Style_Library.Subsegment().SetName( "AcisAsmHighlightFaces" );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility, true );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::MaterialBackFaceDiffuseColor, true );
		key.GetLineAttributeControl().SetWeight( 3.0 );
		key.GetVisibilityControl().SetFaces( true ).SetLines( true ).SetMarkers( false );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

		key = HPS_Style_Library.Subsegment().SetName( "AcisAsmHighlightEdges" );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Everything, false );
		key.GetLineAttributeControl().SetWeight( 3.0 );
		key.GetVisibilityControl().SetFaces( true ).SetLines( true ).SetMarkers( false );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

		key = HPS_Style_Library.Subsegment().SetName( "AcisAsmHighlightVerts" );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Everything, false );
		key.GetLineAttributeControl().SetWeight( 3.0 );
		key.GetMarkerAttributeControl().SetSize( 1.25 );
		key.GetVisibilityControl().SetFaces( true ).SetLines( true ).SetMarkers( true );
		//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );
	}

	key = HPS_Style_Library.Subsegment().SetName( "AcisTextOn" );
	key.GetVisibilityControl().SetText( true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility, true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisTextOff" );
	key.GetVisibilityControl().SetText( false );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility, true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisTextLinesOn" );
	key.GetVisibilityControl().SetText( true ).SetLines( true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility, true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );

	key = HPS_Style_Library.Subsegment().SetName( "AcisTextLinesOff" );
	key.GetVisibilityControl().SetText( false ).SetLines( false );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility, true );
	//key.GetAttributeLockControl().SetLock( HPS::AttributeLock::Type::Visibility );
}

char * HPS_Build_Segment_String( ENTITY* in_entity, char* out_buffer, const char* in_pattern )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	if ( !in_pattern || !*in_pattern )
		in_pattern = get_hps_state()->s_RenderingOptions.GetPattern();
	if ( !in_pattern )
	{
		*out_buffer = 0;
		return out_buffer;
	}
	char pbuffer[POINTER_BUFFER_SIZE];
	char* buffer = out_buffer;
	unsigned int token_number = 0;
	char token[1025];
	while ( HPS_Parse_String( in_pattern, "/", token_number++, token ) )
	{
		if ( strcmp( token, "history" ) == 0 )
		{
			if ( in_entity )
				strcpy( buffer, ptoax( in_entity->history(), pbuffer ) );
			else
			{
				HISTORY_STREAM *default_hs = NULL;
				api_get_default_history( default_hs );
				strcpy( buffer, ptoax( default_hs, pbuffer ) );
			}
		} else if ( strcmp( token, "entity" ) == 0 )
			strcpy( buffer, ptoax( in_entity, pbuffer ) );
		else if ( strcmp( token, "type" ) == 0 && in_entity )
			strcpy( buffer, in_entity->type_name() );
		else if ( is_ASM_ASSEMBLY( in_entity ) && strcmp( token, "handle" ) == 0 )
		{
			ASM_ASSEMBLY* assembly = (ASM_ASSEMBLY*)in_entity;
			asm_model* owning_model = NULL;
			check_outcome( api_asm_assembly_get_owning_model( assembly, owning_model ) );
			entity_handle* assembly_eh = NULL;
			check_outcome( api_asm_model_get_entity_handle( assembly, owning_model, assembly_eh ) );
			strcpy( buffer, ptoax( assembly_eh, pbuffer ) );
		} else
		{
			strcpy( buffer, token ); // Just accept the token as a constant
		}
		buffer += strlen( buffer );
		*buffer++ = '/';
	}
	*--buffer = 0;	// Null terminate and eliminate trailing slash
	return out_buffer;
}

void HPS_Set_Rendering_Options( const char * in_list )
{
	if ( in_list && ( in_list[0] == '?' || in_list[0] == '/' ) )
		in_list++;
	get_hps_state()->s_RenderingOptions.Set_Rendering_Options( in_list );
}

void HPS_Set_Rendering_Options( const hps_rendering_options &in_rendering_options )
{
	get_hps_state()->s_RenderingOptions = in_rendering_options;
}

hps_rendering_options & HPS_Get_Rendering_Options()
{
	return get_hps_state()->s_RenderingOptions;
}

void HPS_Set_Facet_Options( facet_options *in_facet_opts )
{
	if ( get_hps_state()->s_pFacetOptions )
		ACIS_DELETE get_hps_state()->s_pFacetOptions;

	get_hps_state()->s_pFacetOptions = in_facet_opts;
}

facet_options * HPS_Get_Facet_Options()
{
	return get_hps_state()->s_pFacetOptions;
}

void HPS_Close( void )
{
	fp_sentry fps;

	IEntityConverter *icvrt = HPS_Get_Entity_Converter();
	HPS_Set_Entity_Converter( 0 );
	ACIS_DELETE icvrt;

	if ( get_hps_state()->s_pHPS_Map )
	{
		ACIS_DELETE get_hps_state()->s_pHPS_Map;
		get_hps_state()->s_pHPS_Map = 0;
	}
	if ( get_hps_state()->s_pHPS_MapAsm )
	{
		ACIS_DELETE get_hps_state()->s_pHPS_MapAsm;
		get_hps_state()->s_pHPS_MapAsm = 0;
	}
	if ( get_hps_state()->s_pHPS_ModelMap )
	{
		ACIS_DELETE get_hps_state()->s_pHPS_ModelMap;
		get_hps_state()->s_pHPS_ModelMap = 0;
	}
	if ( get_hps_state()->s_pHPS_CHandleMap )
	{
		ACIS_DELETE get_hps_state()->s_pHPS_CHandleMap;
		get_hps_state()->s_pHPS_CHandleMap = 0;
	}
	if ( get_hps_state()->s_pFacetOptions )
	{
		ACIS_DELETE get_hps_state()->s_pFacetOptions;
		get_hps_state()->s_pFacetOptions = 0;
	}
	ACIS_DELETE local_state;
	local_state = NULL;
	//HPS_Reset_System();
}

unsigned long HPS_Internal_Compute_Geometry_Keys( ENTITY* in_entity, HPS::KeyArray& out_key_array, unsigned long in_geomTypes )
{
	ENTITY_LIST entities;
	ENTITY* edgeOrFace;
	outcome o;
	unsigned long numEntities;
	unsigned long num_keys = 0;
	unsigned long i;
	if ( is_BODY( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_vertices )
		{
			int temp_key_count;
			o = api_get_vertices( in_entity, entities );
			check_outcome( o );
			numEntities = entities.count();
			for ( i = 0; i < numEntities; i++ )
			{
				edgeOrFace = entities[i];
				temp_key_count = HPS_Internal_Compute_Geometry_Keys( edgeOrFace, out_key_array, in_geomTypes );
				assert( temp_key_count >= 0 );  // check for errors
				if ( temp_key_count < 0 )
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_edges )
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_edges( in_entity, entities );
			check_outcome( o );
			numEntities = entities.count();
			for ( i = 0; i < numEntities; i++ )
			{
				edgeOrFace = entities[i];
				temp_key_count = HPS_Internal_Compute_Geometry_Keys( edgeOrFace, out_key_array, in_geomTypes );
				assert( temp_key_count >= 0 );  // check for errors
				if ( temp_key_count < 0 )
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_faces )
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_faces( in_entity, entities );
			check_outcome( o );
			numEntities = entities.count();
			for ( i = 0; i < numEntities; i++ )
			{
				edgeOrFace = entities[i];
				temp_key_count = HPS_Internal_Compute_Geometry_Keys( edgeOrFace, out_key_array, in_geomTypes );
				assert( temp_key_count >= 0 );  // check for errors
				if ( temp_key_count < 0 )
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_bodies )
		{
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, out_key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, out_key_array );
		}
	} else if ( is_FACE( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_faces )
		{
			// there may be more than one hoops key mapped to a particular face.
			// so use the findmapping function which can return more than one key.
			// the last arg ensures that we don't stomp past the end of the array. rlw
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, out_key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, out_key_array );
		}
	} else if ( is_EDGE( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_edges )
		{
			// there may be more than one hoops key mapped to a particular edge.
			// so use the findmapping function which can return more than one key.
			// the last arg ensures that we don't stomp past the end of the array. rlw
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, out_key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, out_key_array );
		}
	} else if ( is_VERTEX( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_vertices )
		{
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, out_key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, out_key_array );
		}
	}
	return num_keys;
}

unsigned long HPS_Compute_Geometry_Keys( ENTITY* in_entity, HPS::KeyArray& out_key_array, const char * in_geomTypes )
{
	char token[1025];
	unsigned long options = 0;
	unsigned long bitmask = 0;
	unsigned long token_number = 0;
	/*my canonize chars*/
	/*loop through options*/
	while ( HPS_Parse_String( in_geomTypes, ",", token_number++, token ) )
	{
		if ( HPS_stristr( token, "any" ) )
			bitmask = HPS_GEOMETRY_TYPE_edges | HPS_GEOMETRY_TYPE_faces | HPS_GEOMETRY_TYPE_bodies | HPS_GEOMETRY_TYPE_vertices;
		else if ( HPS_stristr( token, "edges" ) )
			bitmask = HPS_GEOMETRY_TYPE_edges;
		else if ( HPS_stristr( token, "faces" ) )
			bitmask = HPS_GEOMETRY_TYPE_faces;
		else if ( HPS_stristr( token, "bodies" ) )
			bitmask = HPS_GEOMETRY_TYPE_bodies;
		else if ( HPS_stristr( token, "vertices" ) )
			bitmask = HPS_GEOMETRY_TYPE_vertices;
		else
		{	/*error*/
			HPS::Database::GetEventDispatcher().InjectEvent( HPS::WarningEvent( "HPS_Compute_Geometry_Keys: Null option or unknown option" ) );
			return 0;
		}
		if ( !Parse_YesNo_And_Mutate_Options_Using_Bitmask( token, bitmask, &options ) )
			return 0; // parse error, don't set the options.
	}
	return HPS_Internal_Compute_Geometry_Keys( in_entity, out_key_array, options );
}

unsigned long HPS_Compute_Geometry_Key_Count( ENTITY* in_entity )
{
	HPS::KeyArray key_array;
	unsigned long retval = get_hps_state()->s_pHPS_Map->FindMapping( in_entity, key_array );
	//if (0 == retval)
	{	// jkf Feb02 R20 Inspect ASM map as well 88512
		retval += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, key_array );
	}
	return retval;
}

unsigned long HPS_Compute_Geometry_Keys( ENTITY* in_entity, HPS::KeyArray& out_key_array )
{
	unsigned long retval = 0;
	// there may be more than one hoops key mapped to a particular entity.
	// so use the findmapping function which can return more than one key.
	// the last arg ensures that we don't stomp past the end of the array. rlw
	retval = get_hps_state()->s_pHPS_Map->FindMapping( in_entity, out_key_array );
	//if (0 == retval)
	{	// jkf Feb02 R20 Inspect ASM map as well 88512
		retval += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, out_key_array );
	}
	return retval;
}

unsigned long HPS_Internal_Compute_Geometry_Key_Count( ENTITY* in_entity, unsigned long in_geomTypes )
{
	HPS::KeyArray key_array;
	ENTITY_LIST entities;
	ENTITY* edgeOrFace;
	outcome o;
	unsigned long numEntities;
	unsigned long num_keys = 0;
	unsigned long i;
	if ( is_BODY( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_vertices )
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_vertices( in_entity, entities );
			check_outcome( o );
			numEntities = entities.count();
			for ( i = 0; i < numEntities; i++ )
			{
				edgeOrFace = entities[i];
				temp_key_count = HPS_Internal_Compute_Geometry_Key_Count( edgeOrFace, in_geomTypes );
				assert( temp_key_count >= 0 );  // check for errors
				if ( temp_key_count < 0 )
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_edges )
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_edges( in_entity, entities );
			check_outcome( o );
			numEntities = entities.count();
			for ( i = 0; i < numEntities; i++ )
			{
				edgeOrFace = entities[i];
				temp_key_count = HPS_Internal_Compute_Geometry_Key_Count( edgeOrFace, in_geomTypes );
				assert( temp_key_count >= 0 );  // check for errors
				if ( temp_key_count < 0 )
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_faces )
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_faces( in_entity, entities );
			check_outcome( o );
			numEntities = entities.count();
			for ( i = 0; i < numEntities; i++ )
			{
				edgeOrFace = entities[i];
				temp_key_count = HPS_Internal_Compute_Geometry_Key_Count( edgeOrFace, in_geomTypes );
				assert( temp_key_count >= 0 );  // check for errors
				if ( temp_key_count < 0 )
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_bodies )
		{
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, key_array );
		}
	} else if ( is_FACE( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_faces )
		{	// there may be more than one hoops key mapped to a particular face.
			// so use the findmapping function which can return more than one key.
			// the last arg ensures that we don't stomp past the end of the array. rlw
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, key_array );
		}
	} else if ( is_EDGE( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_edges )
		{	// there may be more than one hoops key mapped to a particular edge.
			// so use the findmapping function which can return more than one key.
			// the last arg ensures that we don't stomp past the end of the array. rlw
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, key_array );
		}
	} else if ( is_VERTEX( in_entity ) )
	{
		if ( in_geomTypes & HPS_GEOMETRY_TYPE_vertices )
		{
			num_keys += get_hps_state()->s_pHPS_Map->FindMapping( in_entity, key_array );
			num_keys += get_hps_state()->s_pHPS_MapAsm->FindMapping( in_entity, key_array );
		}
	}
	return num_keys;
}

unsigned long HPS_Compute_Geometry_Key_Count( ENTITY* in_entity, const char * in_geomtypes )
{
	char token[1025];
	unsigned long options = 0;
	unsigned long bitmask = 0;
	unsigned long token_number = 0;
	/*my canonize chars*/
	/*loop through options*/
	while ( HPS_Parse_String( in_geomtypes, ",", token_number++, token ) )
	{
		if ( HPS_stristr( token, "any" ) )
			bitmask = HPS_GEOMETRY_TYPE_edges | HPS_GEOMETRY_TYPE_faces | HPS_GEOMETRY_TYPE_bodies | HPS_GEOMETRY_TYPE_vertices;
		else if ( HPS_stristr( token, "edges" ) )
			bitmask = HPS_GEOMETRY_TYPE_edges;
		else if ( HPS_stristr( token, "faces" ) )
			bitmask = HPS_GEOMETRY_TYPE_faces;
		else if ( HPS_stristr( token, "bodies" ) )
			bitmask = HPS_GEOMETRY_TYPE_bodies;
		else if ( HPS_stristr( token, "vertices" ) )
			bitmask = HPS_GEOMETRY_TYPE_vertices;
		else
		{
			HPS::Database::GetEventDispatcher().InjectEvent( HPS::WarningEvent( "HPS_Compute_Geometry_Key_Count: Null option or unknown option" ) );
			return 0;
		}
		if ( !Parse_YesNo_And_Mutate_Options_Using_Bitmask( token, bitmask, &options ) )
			return 0; // parse error, don't set the options.
	}
	return HPS_Internal_Compute_Geometry_Key_Count( in_entity, options );
}

ENTITY * HPS_Compute_Entity_Pointer( HPS::Key in_key )
{
	ENTITY* entity = 0;
	entity = get_hps_state()->s_pHPS_Map->FindMapping( in_key );
	if ( NULL == entity )
	{
		// jkf Feb02 R20 Inspect ASM map as well 88512
		//		entity = get_hps_state()->s_pHPS_MapAsm->FindMapping( key );
	}
	return entity;

}

ENTITY * HPS_Compute_Entity_Pointer( HPS::Key in_key, int in_acisClass )
{
	ENTITY* entity;
	// TODO: A in_key may not just represent a built in ACIS entity type
	//       like face/edge/vert, it may be a customer entity.
	entity = get_hps_state()->s_pHPS_Map->FindMapping( in_key );
	if ( !entity )
	{
		if ( in_acisClass == BODY_TYPE )
		{
			HPS::Key ancestorSegment = in_key;
			if ( in_key.Type() == HPS::Type::ShellKey )
			{
				while ( ( !entity ) && HPS_Is_Valid_Key( ancestorSegment ) )
				{
					ancestorSegment = ancestorSegment.Owner();
					if ( HPS_Is_Valid_Key( ancestorSegment ) )
						entity = get_hps_state()->s_pHPS_Map->FindMapping( ancestorSegment );
				}
				if ( entity )
				{
					assert( is_BODY( entity ) );
					return entity;
				}
			}
		}
	}
	//if ( !entity )
	//	entity = get_hps_state()->s_pHPS_MapAsm->FindMapping( in_key );
	if ( !entity )
	{
		if ( in_acisClass == BODY_TYPE )
		{
			HPS::Type type = in_key.Type();
			if ( type == HPS::Type::ShellKey )
			{
				HPS::Key ancestorSegment = in_key;
				while ( ( !entity ) && HPS_Is_Valid_Key( ancestorSegment ) )
				{
					ancestorSegment = ancestorSegment.Owner();
					if ( HPS_Is_Valid_Key( ancestorSegment ) )
						entity = get_hps_state()->s_pHPS_MapAsm->FindMapping( ancestorSegment );
				}
				if ( entity )
					assert( is_BODY( entity ) );
				return entity;
			} else
				return 0;
		} else
			return 0;
	}
	if ( is_EDGE( entity ) )
	{
		if ( in_acisClass == EDGE_TYPE )
			return entity;
		else if ( in_acisClass == BODY_TYPE )
		{
			ENTITY* owning_body;
			outcome o;
			o = api_get_owner( entity, owning_body );
			assert( o.ok() );
			return owning_body;
		} else
			return 0;
	} else if ( is_FACE( entity ) )
	{
		if ( in_acisClass == FACE_TYPE )
			return entity;
		else if ( in_acisClass == BODY_TYPE )
		{
			ENTITY* owning_body = 0;
			outcome o;
			o = api_get_owner( entity, owning_body );
			assert( o.ok() );
			return owning_body;
		} else
			return 0;
	} else if ( is_VERTEX( entity ) )
	{
		if ( in_acisClass == VERTEX_TYPE )
			return entity;
		else if ( in_acisClass == BODY_TYPE )
		{
			ENTITY* owning_body = NULL;
			outcome o;
			o = api_get_owner( entity, owning_body );
			assert( o.ok() );
			return owning_body;
		} else
			return 0;
	} else if ( is_BODY( entity ) )
	{
		if ( in_acisClass == BODY_TYPE )
			return entity;
		else
			return 0;
	} else
		return 0;
	return 0;
}

// This function takes all the shells it finds in a segment ( filtered by the in_bodies_to_merge list )
// and combines them into a single shell for rendering performance purposes.
static void merge_body_faces_in_currently_open_segment( BODY** in_bodies_to_merge, unsigned long in_num_bodies )
{
}

static void merge_body_faces_recurse_subsegments( BODY** in_bodies_to_merge, unsigned long in_num_bodies )
{
}

// merge_body_faces iterates through the segments that represent a body, and gathers
// together any shells that are in the same subsegment into a single shell.  Usually
// shells are split into seperate segments by color, and usually a seperate shell
// represents a seperate face.  When shells are combined, their mappings to faces
// will be lost, so you gain rendering efficiency but lost selection capability.
void merge_body_faces( BODY** in_bodies_to_merge, unsigned long in_num_bodies )
{
	// Lets look for a body segment mapping.  If on, then there should only be one body in the list
	// and we will open it's segment up for processing.
	if ( get_hps_state()->s_RenderingOptions.GetBodySegmentMode() )
	{
		assert( in_num_bodies == 1 );
		if ( in_num_bodies != 1 )
			return;
		HPS::KeyArray key_array;
		HPS_Internal_Compute_Geometry_Keys( in_bodies_to_merge[0], key_array, HPS_GEOMETRY_TYPE_bodies );
		assert( HPS_Is_Valid_Key( key_array[0] ) );
		if ( !HPS_Is_Valid_Key( key_array[0] ) )
			return;
	}
}
//
// Begin Point Cloud Code
//

#define PC_PARENT_SEGEMENT "AcisPointClouds"
#define MAX_PC_PATH 256

char * getPointCloudName( SPApoint_cloud* in_cloud, logical create_parent_segment = FALSE )
{
	return 0;
}

HPS_BOOLEAN HPS_Erase_PointCloud( SPApoint_cloud* in_cloud )
{
	return FALSE;
}

HPS::SegmentKey HPS_Highlight_PointCloud( SPApoint_cloud* in_cloud, const rgb_color& inColor, logical turnOn )
{
	return HPS_INVALID_SEGMENT_KEY;
}

#ifdef THREAD_SAFE_ACIS
SESSION_LOCAL_VAR safe_integral_type<int> pos_cloud_seg_num;
#else
static int pos_cloud_seg_num;
#endif

HPS::SegmentKey HPS_Render_PositionCloud( SPAposition_cloud const& cloud )
{
	const int max_buffer_pts = 10000;
	SPAposition pts[max_buffer_pts];
	char segment_name[256];
	sprintf( segment_name, "position cloud %d", pos_cloud_seg_num++ );
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	EXCEPTION_BEGIN // ensure segment is closed even on error.
		key = HPS_KOpen_Segment( segment_name );
	EXCEPTION_TRY;
	{
	}
	EXCEPTION_CATCH_TRUE
		EXCEPTION_END
		return key;
}

HPS::SegmentKey HPS_Render_PointCloud( SPApoint_cloud* in_cloud, const rgb_color& inColor )
{
	return HPS_INVALID_SEGMENT_KEY;
}

HPS_BOOLEAN HPS_Set_PointCloud_Color( SPApoint_cloud* in_cloud, rgb_color in_Color )
{
	HPS_BOOLEAN status = TRUE;
	return status;
}

SPApoint_cloud * HPS_Select_Point_Cloud( HPS::SegmentKey in_scene_key, HPS_Point in_pick_point )
{
	return NULL;
}

/**
* The following two functions are for point cloud picking. qsort_compare is a comparator
* function for qsort, which we need in the second function.
* get_position_indices takes an array of indices to a compacted
* index list (which may or may not actually be compacted)
* and returns a sortable integer list stream (that should be sorted on return)
* of indexes into a position list.
*
* The function works by keeping track of three indices. The first, list_index,
* is the literal location in the index list - it may be a tombstone, it may be a real value.
* Every loop, we try to get the information from this particular location in the list.
* If the location is real (not a tombstone), we increment the real_index. This represents
* the nubmer of actual (non-tombstone) values read.
* Lastly, if we did get a real index, we compare it to the index we're currently looking
* for (given by input_array[input_index]. If it's an index we're looking for, we add the
* position list index to our sortable_integer_ls of position list indices, and increment
* input_index.
*
**/
int qsort_compare( const void * a, const void * b )
{
	return ( *(int*)a - *(int*)b );
}

sortable_integer_ls * get_position_indices( int* in_array, int in_size, SPApoint_cloud* in_pc )
{
	sortable_integer_ls* position_indices = NULL;
	position_indices = ACIS_NEW sortable_integer_ls();
	qsort( in_array, in_size, sizeof( int ), qsort_compare );
	int input_index = 0;
	int list_index = 0; //The element of the list we're at, INCLUDING tombstones
	int real_index = 0; //The element of the list we're at, NOT including tombstones
	int new_value = 0;
	SPAposition temp_position;
	while ( input_index < in_size && input_index < in_pc->size() )
	{
		if ( in_pc->get( list_index, temp_position, new_value ) )
		{
			if ( in_array[input_index] == real_index )
			{
				position_indices->add( new_value );
				input_index++;
			}
			real_index++;
		}
		list_index++;
	}
	return position_indices;
}

SPApoint_cloud * HPS_Select_Point_Cloud_Marker( HPS::SegmentKey in_scene_key, HPS_Point in_pick_point )
{
	SPApoint_cloud* point_cloud = NULL;
	return point_cloud;
}

SPApoint_cloud * HPS_Select_Point_Cloud_Markers( HPS::SegmentKey in_scene_key, HPS_Point in_pick_point1, HPS_Point in_pick_point2 )
{
	return NULL;
}

HPS::SegmentKey HPS_Render_Entity( ENTITY* in_entity, const char* in_pattern )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	if ( !in_entity )
		return HPS_INVALID_SEGMENT_KEY;
	// Caller paused the rendering opration. So skip.
	option_header *pr_opt = find_option( "pr" );
	if ( pr_opt && pr_opt->on() )
		return HPS_INVALID_SEGMENT_KEY;
	if ( is_ASM_ASSEMBLY( in_entity ) || is_ASM_MODEL_REF( in_entity ) )
		return HPS_INVALID_SEGMENT_KEY;
	const char * pattern = ( !in_pattern || !*in_pattern )
		? get_hps_state()->s_RenderingOptions.GetPattern()
		: in_pattern;
	HPS::SegmentKey pattern_key = HPS_Open_Segment( in_entity, pattern );

	HPS_Map* map = 0;
	if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
		map = get_hps_state()->s_pHPS_Map;
	HPS::SegmentKey key = HPS_INVALID_SEGMENT_KEY;
	ENTITY *owner = NULL;
	api_get_owner( in_entity, owner );
	if ( HPS_Get_Entity_Converter() )
		key = HPS_Get_Entity_Converter()->ConvertEntity( owner, get_hps_state()->s_RenderingOptions, map, pattern_key );
	if ( get_hps_state()->s_RenderingOptions.GetMergeFacesMode() && !get_hps_state()->s_RenderingOptions.GetMergeBodiesMode() )
	{
		// merge bodies can't be done on Wire bodies 
		if ( ( in_entity->identity() == BODY_TYPE ) && ( is_wire_body( in_entity ) == FALSE ) )
			merge_body_faces( (BODY**)&in_entity, 1 );
	}
	if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
		return HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_Map->FindMapping( in_entity ) );
	return key;
}

HPS::SegmentKey HPS_Render_Entity( ENTITY * in_entity, HPS::SegmentKey in_segment_key )
{
	IEntityConverter * cur_converter = HPS_Get_Entity_Converter();
	if ( !in_entity )
		return HPS_INVALID_SEGMENT_KEY;
	// Caller paused the rendering opration. So skip.
	option_header *pr_opt = find_option( "pr" );
	if ( pr_opt && pr_opt->on() )
		return HPS_INVALID_SEGMENT_KEY;
	if ( is_ASM_ASSEMBLY( in_entity ) || is_ASM_MODEL_REF( in_entity ) )
		return HPS_INVALID_SEGMENT_KEY;
	const char * pattern = get_hps_state()->s_RenderingOptions.GetPattern();
	if ( pattern && ( pattern[0] == '?' || pattern[0] == '/' ) )
		pattern++;
	if ( pattern == NULL && strncmp( "0x", in_segment_key.Name().GetBytes(), 2 ) )
		pattern = "entity";
	HPS::SegmentKey entity_key = HPS_Open_Segment( in_entity, pattern, in_segment_key );
	HPS_Map* map = 0;
	if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
		map = get_hps_state()->s_pHPS_Map;
	HPS::SegmentKey return_key = HPS_INVALID_SEGMENT_KEY;
	ENTITY *owner = NULL;
	api_get_owner( in_entity, owner );
	if ( cur_converter ) 
		return_key = cur_converter->ConvertEntity( owner, get_hps_state()->s_RenderingOptions, map, entity_key );
	if ( get_hps_state()->s_RenderingOptions.GetMergeFacesMode() && !get_hps_state()->s_RenderingOptions.GetMergeBodiesMode() )
	{
		// merge bodies can't be done on Wire bodies 
		if ( ( in_entity->identity() == BODY_TYPE ) && ( is_wire_body( in_entity ) == FALSE ) )
			merge_body_faces( (BODY**)&in_entity, 1 );
	}
	if ( get_hps_state()->s_RenderingOptions.GetMappingFlag() )
		return HPS_Cast_SegmentKey( get_hps_state()->s_pHPS_Map->FindMapping( in_entity ) );
	return return_key;
}

void HPS_Internal_Flush_Entity_Geometry( ENTITY* in_entity );

logical HPS_Update_Entity( ENTITY* in_entity )
{
	// This api right now only works for bodies and 'create body segments' is on
	HPS::Key body_key = get_hps_state()->s_pHPS_Map->FindMapping( in_entity );
	bool no_segment = true;
	if ( HPS_Is_Valid_Key( body_key ) )
	{
		HPS::Type type = body_key.Type();
		if ( type == HPS::Type::SegmentKey )
			no_segment = false;
	}
	if ( no_segment )
	{
		HPS::Database::GetEventDispatcher().InjectEvent( HPS::ErrorEvent( "HPS_Update_Entity:  "
			"Could not determine segment corresponding to given entity. "
			"Please ensure entity is 'body' type and 'create body segments' option is on" ) );
		return false;
	}
	// Delete the ENTITY data.  This will delete all the maps, including the top
	// level entity, but they will be put back in by Render Entity.
	HPS_Internal_Flush_Entity_Geometry( in_entity );
	HPS_Render_Entity( in_entity, HPS::SegmentKey( body_key ) );
	return true;
}

HPS::SegmentKey HPS_ReRender_Entity( ENTITY* in_entity, const char* in_pattern )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	HPS_Delete_Entity_Geometry( in_entity );
	return HPS_Render_Entity( in_entity, in_pattern );
}

#define MAX_FIXED_KEY_ARRAY 1000
void HPS_Internal_Delete_Entity_Geometry( ENTITY* in_entity )
{
	// hh:combine causes FACEs that were toplevel to not be toplevel
	// but we still need to be able to delete the rendering data.
	// assert(is_toplevel(in_entity));
	HPS::KeyArray key_array;
	long num_keys = HPS_Compute_Geometry_Keys( in_entity, key_array );
	if ( num_keys <= 0 )
		return;
	long j;
	for ( j = 0; j < num_keys; j++ )
	{
		HPS::SegmentKey key = HPS_Cast_SegmentKey( key_array[j] );
		if ( HPS_Is_Valid_Segment_Key( key ) )
		{
			get_hps_state()->s_pHPS_Map->DeleteMapping( key );
			// jkf Feb02 R20 Delete from ASM map as well 88512
			get_hps_state()->s_pHPS_MapAsm->DeleteMapping( key );
			// Check if you have a face and you happen to delete the segment containing the mesh before
			// the mesh segement is nuked.
			HPS_Delete_By_Key( key );
		}
	}
	get_hps_state()->s_pHPS_Map->DeleteMapping( in_entity );
	// jkf Feb02 R20 Delete from ASM map as well 88512
	get_hps_state()->s_pHPS_MapAsm->DeleteMapping( in_entity );
}

void HPS_Internal_Flush_Entity_Geometry( ENTITY* in_entity )
{
	// hh:combine causes FACEs that were toplevel to not be toplevel
	// but we still need to be able to delete the rendering data.
	// assert(is_toplevel(in_entity));
	HPS::KeyArray key_array;
	long num_keys = HPS_Compute_Geometry_Keys( in_entity, key_array );
	if ( num_keys <= 0 )
		return;
	long j;
	for ( j = 0; j < num_keys; j++ )
	{
		HPS::SegmentKey key = HPS_Cast_SegmentKey( key_array[j] );
		// This routine is cleaning all the data, so we will kill all the maps.  If
		// this segment is being reused for an ENTITY then the maps will have to be
		// put back in. This is the easiest way to prevent duplicate maps.
		get_hps_state()->s_pHPS_Map->DeleteMapping( key );
		get_hps_state()->s_pHPS_MapAsm->DeleteMapping( key );
		// Clear the contents of the segment mapped by the key.
		if ( HPS_Is_Valid_Segment_Key( key ) )
			key.Flush();
	}
	get_hps_state()->s_pHPS_Map->DeleteMapping( in_entity );
	get_hps_state()->s_pHPS_MapAsm->DeleteMapping( in_entity );
}

void HPS_Delete_Entity_Geometry( ENTITY_LIST const& in_entitylist )
{
	// there is not part and all that stuff here,
	// just call the internal deletion routene for each entity
	ENTITY* entity = 0;
	in_entitylist.init(); // reset next() function to give first element in "list"
	while ( ( entity = in_entitylist.next() ) != 0 )
		HPS_Delete_Entity_Geometry( entity );
}

void HPS_Delete_Entity_Geometry( ENTITY *in_entity )
{
	HPS_Internal_Delete_Entity_Geometry( in_entity );
}

logical face_has_no_render_attrib( FACE* in_face )
{
	SHELL * fs = (SHELL *)in_face->owner();
	LUMP * fl = ( fs != NULL ) ? (LUMP *)fs->owner() : NULL;
	BODY * fb = ( fl != NULL ) ? (BODY *)fl->owner() : NULL;
	logical face_has_norender_attrib = ( in_face ? find_NORENDER_ATTRIB( in_face ) != NULL : FALSE );
	logical shell_has_norender_attrib = ( fs ? find_NORENDER_ATTRIB( fs ) != NULL : FALSE );
	logical lump_has_norender_attrib = ( fl ? find_NORENDER_ATTRIB( fl ) != NULL : FALSE );
	logical body_has_norender_attrib = ( fb ? find_NORENDER_ATTRIB( fb ) != NULL : FALSE );
	logical has_norender_attrib = ( face_has_norender_attrib || shell_has_norender_attrib || lump_has_norender_attrib || body_has_norender_attrib );
	return has_norender_attrib;
}
/**
 * Make a new render progress callback, based on the input entity list.
 * Largely, this reverse engineers the workflow that would happen when rendering - so you
 * see conditions like ( is_BODY(ent) || is_FACE(ent) ) and all the NORENDER_ATTRIB
 * logic.
 *
 * The in_skip_rendered_faces flag is a bit inelegant, but allows the progress metering to be
 * shared for model rendering and part rendering - model rendering skips already rendered faces,
 * part rendering doesn't. A possible future efficiency.
 */
HA_RENDER_progress_info * make_progress_meter( const ENTITY_LIST & in_entitylist, logical in_skip_rendered_faces )
{
	// Count the total # of faces - for rendering progress.
	int total_faces = 0;
	ENTITY * ent = NULL;
	in_entitylist.init();
	ENTITY_LIST faces, unrendered_faces;
	while ( ENTITY * ent = in_entitylist.next() )
	{
		if ( is_BODY( ent ) || is_FACE( ent ) )
			api_get_faces( ent, faces );
	}
	while ( ENTITY * face = faces.next() )
	{
		logical has_norender_attrib = face_has_no_render_attrib( (FACE*)face );
		logical skip = FALSE;
		if ( in_skip_rendered_faces )
		{
			int nkeys = HPS_Compute_Geometry_Key_Count( face );
			skip = ( nkeys != 0 );
		}
		if ( !skip && !has_norender_attrib )
			unrendered_faces.add( face );
	}
	int total_unrendered_faces = unrendered_faces.iteration_count();
	return ( total_unrendered_faces > 0 ) ? ACIS_NEW HA_RENDER_progress_info( total_unrendered_faces ) : NULL;
}

/**
 * Search and close any open segments
 */
int
HPS_Close_All_Open_Segments()
{
	int count = 0;
	return count;
}

SESSION_GLOBAL_VAR HA_RENDER_progress_info * progress_meter;
#include "af_serializable_mesh.hxx"
#include "af_api.hxx"
#include "thmgr.hxx"
void facet_unfaceted_bodies( ENTITY_LIST const& in_ents )
{
	if ( faceter_allow_multithreading.on() && ( thread_work_base::thread_count() > 0 ) )
	{
		ENTITY_LIST bodies_with_no_facets;
		{for ( ENTITY* e = in_ents.first(); e; e = in_ents.next() )
		{
			ENTITY_LIST efaces;
			api_get_faces( e, efaces );
			logical has_facets = FALSE;
			{for ( ENTITY* f = efaces.first(); f; f = efaces.next() )
			{
				if ( GetSerializableMesh( (FACE*)f ) )
				{
					has_facets = TRUE;
					break;
				}
			}}
			if ( !has_facets && is_BODY( e ) )
			{
				bodies_with_no_facets.add( e );
			}
		}}
		facet_options* fo = HPS_Get_Facet_Options();
		api_facet_bodies( bodies_with_no_facets, fo );
	}
}

extern option_header hps_direct_render;
void HPS_Render_Entities( ENTITY_LIST const& in_entitylist, const char* in_pattern )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	ENTITY* entity = 0;
	// *FIX need to handle case where user renders an individual face,
	// but they have merge_bodies mode ON.  Probably should forbid this (report error here)
	// *TODO determine method of error reporting for this type of error
	EXCEPTION_BEGIN
		progress_meter = NULL;
	EXCEPTION_TRY;
	{
		hps_rendering_options * ro = &get_hps_state()->s_RenderingOptions;
		if ( ro && ro->GetRenderFacesMode() )
			progress_meter = make_progress_meter( in_entitylist, hps_direct_render.on() );
		facet_unfaceted_bodies( in_entitylist );
		in_entitylist.init(); // initialize next() to return first elt in "list"
		while ( ( entity = in_entitylist.next() ) != 0 )
			HPS_Render_Entity( entity, in_pattern );
		if ( get_hps_state()->s_RenderingOptions.GetMergeBodiesMode() )
		{
			// need to create a pruned array which only contains body entities in it.
			BODY** bodies;
			unsigned long num_bodies = 0;
			bodies = ACIS_NEW BODY*[in_entitylist.iteration_count()];
			if ( !bodies )
				return;
			in_entitylist.init(); // initialize next() to return first elt in "list"
			while ( ( entity = in_entitylist.next() ) != 0 )
			{
				if ( is_BODY( entity ) )
					bodies[num_bodies++] = (BODY*)entity;
			}
			if ( num_bodies > 0 )
				merge_body_faces( bodies, num_bodies );
			if ( bodies )
				ACIS_DELETE[] STD_CAST bodies;
		}
	} EXCEPTION_CATCH_TRUE;
	{
		if ( progress_meter )
		{
			ACIS_DELETE progress_meter;
			progress_meter = NULL;
		}
		if ( error_no != 0 )
			HPS_Close_All_Open_Segments();
	} EXCEPTION_END;
}

void HPS_ReRender_Entities( ENTITY_LIST const& in_entitylist, const char* in_pattern )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	ENTITY* entity = 0;
	in_entitylist.init(); // initialize next() to return first elt in "list"
	while ( ( entity = in_entitylist.next() ) != 0 )
		HPS_ReRender_Entity( entity, in_pattern );
}

void HPS_Show_Rendering_Options( char * out_list )
{
	get_hps_state()->s_RenderingOptions.Show_Rendering_Options( out_list );
}

void HPS_Show_One_Rendering_Option( const char * in_type, char * out_value )
{
	get_hps_state()->s_RenderingOptions.Show_One_Rendering_Option( in_type, out_value );
}

HPS::Key HPS_Render_Curve(
	CURVE *				in_curve_entity,
	SPAparameter		in_start_param,
	SPAparameter		in_end_param,
	const SPAtransf*	in_transform,
	HPS::SegmentKey		in_segment_key
	)
{
	if ( !in_curve_entity )
		return HPS_INVALID_SEGMENT_KEY;
	fp_sentry fps;
	double tolerance = HPS_World_Coords_Per_Pixel();
	static option_header * render_curve_factor = 0;
	static int render_curve_factor_requested = FALSE;
	if ( render_curve_factor == 0 && !render_curve_factor_requested )
	{
		render_curve_factor = find_option( "render_curve_factor" );
		render_curve_factor_requested = TRUE;
	}
	if ( render_curve_factor )
		tolerance /= render_curve_factor->count();
	int num_points = 0;
	AF_POINT* first_point = 0;
	tolerance /= in_transform->scaling();
	if ( tolerance < SPAresabs )
		tolerance = SPAresabs * 10;
	outcome o = api_facet_curve( in_curve_entity->equation(), in_start_param, in_end_param, tolerance, 0.0, 0.0, num_points, first_point );
	assert( o.ok() );
	HPS::Key key = HPS_INVALID_SEGMENT_KEY;
	float *pHoopsPoints = 0;
	if ( num_points > 0 )
		pHoopsPoints = ACIS_NEW float[3 * num_points];
	if ( pHoopsPoints )
	{
		AF_POINT* curr = first_point;
		int i;
		for ( i = 0; i < num_points; i++ )
		{
			SPAposition pos = curr->get_position() * in_transform;
			pHoopsPoints[3 * i + 0] = (float)pos.x();
			pHoopsPoints[3 * i + 1] = (float)pos.y();
			pHoopsPoints[3 * i + 2] = (float)pos.z();
			curr = curr->next( 0 );
		}
	}
	api_delete_all_AF_POINTs( first_point );
	if ( pHoopsPoints )
	{
		HPS::PointArray line;
		for ( int i = 0; i < num_points; i++ )
			line.push_back( HPS::Point( pHoopsPoints[3 * i + 0], pHoopsPoints[3 * i + 1], pHoopsPoints[3 * i + 2] ) );
		in_segment_key.InsertLine( line );
		ACIS_DELETE[] STD_CAST pHoopsPoints; pHoopsPoints = 0; num_points = 0;
	}
	return key;
}

#define HIGHLIGHT_SEGMENT "ACIShighlightSegment"
void HPS_Highlight_Entity( ENTITY * in_entity, logical in_highlight, const rgb_color & in_color )
{
	HPS::KeyArray key_array;
	int count = HPS_Compute_Geometry_Keys( in_entity, key_array, "any" );
	for ( int i = 0; i < count; i++ )
	{
		HPS::Key selected_key = key_array[i];
		HPS::SegmentKey owning_seg_key = selected_key.Owner();
		if ( in_highlight )
		{
			HPS::SegmentKey highlight_key = owning_seg_key.Subsegment( HIGHLIGHT_SEGMENT );
			//intptr_t seg_key_ID = owning_seg_key.GetInstanceID();
			//highlight_key.SetUserData( 0, sizeof( intptr_t ), (byte*)&owning_seg_key ); // not really necessary???
			HPS::RGBColor hps_color( (float)in_color.red(), (float)in_color.green(), (float)in_color.blue() );
			highlight_key.GetMaterialMappingControl().SetFaceColor( hps_color ).SetLineColor( hps_color );
			highlight_key.GetLineAttributeControl().SetWeight( 3.0 );
			highlight_key.GetVisibilityControl().SetFaces( true ).SetLines( true ).SetMarkers( false );
			if ( is_VERTEX( in_entity ) || is_APOINT( in_entity ) )
			{
				highlight_key.GetMarkerAttributeControl().SetSize( 1.25 );
				highlight_key.GetVisibilityControl().SetMarkers( true );
			}
			selected_key.MoveTo( highlight_key );
		} else
			selected_key.MoveTo( owning_seg_key.Owner() );
	}
}

logical HPS_Show_Entity_Highlight( ENTITY* in_entity )
{
	// An entity is highlighted if all of its parts are in a HIGHLIGHT_SEGMENT
	// It is not good enough to have only some parts highlighted.  For example
	// one FACE of a body.
	HPS::KeyArray key_array;
	int count = HPS_Compute_Geometry_Keys( in_entity, key_array, "any" );
	for ( int i = 0; i < count; i++ )
	{
		HPS::SegmentKey key = HPS_Cast_SegmentKey( key_array[i] );
		if ( key.Type() == HPS::Type::SegmentKey )
		{
			std::string owner( HPS_Show_Segment( key ).c_str() );
			std::string::size_type str1 = owner.find( HIGHLIGHT_SEGMENT );
			logical hilite = FALSE;
			if ( str1 != std::string::npos )
				hilite = TRUE;
			if ( !hilite ) // If we could not parse the string, or the segment did not have the special name, this piece of the entity is not highlighted.
				return FALSE;
		}
	}
	return TRUE;
}

IEntityConverter * HPS_Get_Entity_Converter()
{
	return get_hps_state()->s_pIEntityConverter;
}

void HPS_Set_Entity_Converter( IEntityConverter *in_converter )
{
	get_hps_state()->s_pIEntityConverter = in_converter;
}

logical HPS_Read_Sat_File( const char *in_file_name, HPS::SegmentKey in_segment_key, ENTITY_LIST& out_entity_list )
{
	FILE * fp = NULL;
	ENTITY* entity = 0;
	outcome oc;
	logical res = TRUE;
	fp = fopen( in_file_name, "r" );
	res = ( fp != NULL );
	if ( res )
	{
		oc = api_restore_entity_list( fp, TRUE, out_entity_list );
		res = oc.ok();
	}
	if ( fp )
		fclose( fp );
	if ( res )
	{
		out_entity_list.init(); // initialize next() to return first elt in "list"
		//std::string owner_string( HPS_Show_Segment( in_segment_key ) );
		//HPS::SegmentKey owner_segkey = ( HPS_Open_Segment( owner_string.c_str() ) );
		while ( ( entity = out_entity_list.next() ) != 0 )
			HPS_Render_Entity( entity, in_segment_key );
	}
	return res;
}

logical HPS_Write_Sat_File( const char *in_file_name, ENTITY_LIST const& in_entitylist )
{
	FILE * fp = NULL;
	outcome oc;
	logical res = TRUE;
	fp = fopen( in_file_name, "w" );
	res = ( fp != NULL );
	// ACIS 6.3 requires this thing for saving files
	char id_string[128];
	sprintf( id_string, "HPS-ACIS Part Viewer" );
	if ( res )
	{
		FileInfo info;
		info.set_product_id( id_string );
		info.set_units( 1.0 );
		oc = api_set_file_info( ( FileIdent | FileUnits ), info );
		info.reset();
		res = oc.ok();
	}
	if ( res )
		oc = api_save_entity_list( fp, TRUE, in_entitylist );
	if ( fp )
		fclose( fp );
	return res;
}

logical HPS_Read_Sab_File( const char *in_file_name, HPS::SegmentKey in_segment_key, ENTITY_LIST& out_entity_list )
{
	FILE * fp = NULL;
	ENTITY* entity = 0;
	outcome oc;
	logical res = TRUE;
	char * flag = "r";
#if defined( NT ) || defined( mac )
	flag = "rb";
#endif
	fp = fopen( in_file_name, flag );
	res = ( fp != NULL );
	if ( res )
	{
		oc = api_restore_entity_list( fp, FALSE, out_entity_list );
		res = oc.ok();
	}
	if ( fp )
		fclose( fp );
	if ( res )
	{
		out_entity_list.init(); // initialize next() to return first elt in "list"
		while ( ( entity = out_entity_list.next() ) != 0 )
			HPS_Render_Entity( entity, in_segment_key );
	}
	return res;
}

logical HPS_Write_Sab_File( const char *in_file_name, ENTITY_LIST const & out_entity_list )
{
	FILE * fp = NULL;
	outcome oc;
	logical res = TRUE;
	char * flag = "w";
#if defined( NT ) || defined( mac )
	flag = "wb";
#endif
	fp = fopen( in_file_name, flag );
	res = ( fp != NULL );
	// ACIS 6.3 requires this thing for saving files
	char id_string[128];
	sprintf( id_string, "HPS-ACIS Part Viewer" );
	if ( res )
	{
		FileInfo info;
		info.set_product_id( id_string );
		info.set_units( 1.0 );
		oc = api_set_file_info( ( FileIdent | FileUnits ), info );
		info.reset();
		res = oc.ok();
	}
	if ( res )
		oc = api_save_entity_list( fp, FALSE, out_entity_list );
	if ( fp )
		fclose( fp );
	return res;
}

logical HPS_Write_Sat_File_2(
	const char *		in_file_name,
	ENTITY_LIST const &	out_entity_list,
	float				version )
{
	// get the current save version
	int cur_major_ver;
	int cur_minor_ver;
	outcome oc = api_get_save_version( cur_major_ver, cur_minor_ver );
	logical res = oc.ok();
	// extract the major and minor versions from the user specified version 
	// e.g. if version is v.w then v is major and w is minor
	if ( res )
	{
		assert( version > ( double )0.0 );
		int user_major_ver = (int)floor( (double)version );
		int user_minor_ver = (int)( ( (double)( (double)version - (double)user_major_ver ) ) * ( double )10.0 );

		// set this as current version for save
		oc = api_save_version( user_major_ver, user_minor_ver );
		res = oc.ok();
	}
	// call the vanilla write sat api
	if ( res )
		res = HPS_Write_Sat_File( in_file_name, out_entity_list );
	// reset the save version
	if ( res )
	{
		oc = api_save_version( cur_major_ver, cur_minor_ver );
		res = oc.ok();
	}
	return res;
}

HPS::Key HPS_KINDEXED_MESH_to_HOOPS( INDEXED_MESH *in_mesh, FACE * /*face*/ )
{
	if ( !in_mesh )
		return HPS_INVALID_SEGMENT_KEY;
	HPS::Key key = HPS_INVALID_SEGMENT_KEY;
	fp_sentry fps;
	// First find the number of polys.
	int num_polys = in_mesh->get_num_polygon();
	// Get the number of vertices off all the polys.
	int nnode = in_mesh->get_num_vertex();
	int num_polynodes = in_mesh->get_num_polynode();
	EXCEPTION_BEGIN;
	float *points = 0;
	int *pFacetConnectivity = 0;
	EXCEPTION_TRY;
	{
		// Get the verts array.
		points = ACIS_NEW float[nnode * 3];
		// Build the conectivity array.
		pFacetConnectivity = ACIS_NEW int[num_polys + num_polynodes];
		// Now fill in the conectivity array.
		int polyIdx = 0;
		int current_array_index = 0;
		while ( polyIdx < num_polys )
		{
			// grab the polygon and find out how many vertices it has.
			indexed_polygon *poly = in_mesh->get_polygon( polyIdx++ );
			int numVertices = poly->num_vertex();

			pFacetConnectivity[current_array_index++] = numVertices;
			for ( int i = 0; i < numVertices; i++ )
			{
				polygon_vertex* vert = poly->get_vertex( i );
				int index = in_mesh->get_vertex_index( vert );
				pFacetConnectivity[current_array_index++] = index;
			}
		}
		for ( int j = 0; j < nnode; j++ )
		{
			SPAposition pos = in_mesh->get_position( j );
			points[j * 3] = (float)pos.x();
			points[j * 3 + 1] = (float)pos.y();
			points[j * 3 + 2] = (float)pos.z();
		}
		// Create the HOOPS rep.
		if ( num_polys > 0 )
		{
		}
	} EXCEPTION_CATCH_TRUE;
	{
		ACIS_DELETE[] STD_CAST points; points = 0;
		ACIS_DELETE[] STD_CAST pFacetConnectivity; pFacetConnectivity = 0;
	} EXCEPTION_END;
	return key;
}

#include "surdef.hxx"
#include <vector>
#include <algorithm>

typedef struct {
	float red, green, blue;
} HOT_COLD_COLOR;
HOT_COLD_COLOR get_hot_cold_color( float in_value, float in_min_value, float in_max_value )
{
	HOT_COLD_COLOR color = { 1.0, 1.0, 1.0 }; // white
	if ( in_value < in_min_value )
		in_value = in_min_value;
	if ( in_value > in_max_value )
		in_value = in_max_value;
	float range = in_max_value - in_min_value;
	if ( in_value < ( in_min_value + 0.25 * range ) )
	{
		color.red = 0;
		color.green = 4 * ( in_value - in_min_value ) / range;
	} else if ( in_value < ( in_min_value + 0.50 * range ) )
	{
		color.red = 0;
		color.blue = (float)( 1 + 4 * ( in_min_value + 0.25 * range - in_value ) / range );
	} else if ( in_value < ( in_min_value + 0.75 * range ) )
	{
		color.red = (float)( 4 * ( in_value - in_min_value - 0.5 * range ) / range );
		color.blue = 0;
	} else
	{
		color.green = (float)( 1 + 4 * ( in_min_value + 0.75 * range - in_value ) / range );
		color.blue = 0;
	}
	return color;
}

typedef double( *HighlightWeightFn )( FACE *in_face, float pos3d[3], float pos_uv[2] );
HighlightWeightFn wt_function = NULL;
bool get_colors_for_FACE( af_serializable_mesh *in_mesh, FACE * in_face, float *in_points, float* in_texture_uvs, int in_vertexCount, float *out_colors )
{
	std::vector<double> weights;
	int i = 0;
	double min_elem = DBL_MAX;
	double max_elem = DBL_MIN;
	for ( i = 0; i < in_vertexCount; i++ )
	{
		double xxx = wt_function( in_face, &in_points[3 * i], &in_texture_uvs[2 * 1] );
		weights.push_back( xxx );
		if ( xxx < min_elem ) min_elem = xxx;
		if ( xxx > max_elem ) max_elem = xxx;
	}
	//min_elem = *min_element( weights.begin(), weights.end() );
	//max_elem = *max_element( weights.begin(), weights.end() );
	double range = max_elem - min_elem;
	if ( range < SPAresabs )
		range = SPAresabs;
	for ( i = 0; i < in_vertexCount; i++ )
	{
		float r = 0;
		r = (float)( weights[i] - min_elem ) / (float)range;
		out_colors[3 * i + 2] = 0.0f;
		HOT_COLD_COLOR colour = get_hot_cold_color( (float)( weights[i] ), (float)min_elem, (float)max_elem );
		out_colors[3 * i + 0] = colour.red;
		out_colors[3 * i + 1] = colour.green;
		out_colors[3 * i + 2] = colour.blue;

		//if ( r <= 0.5 )
		//{
		//	out_colors[3 * i] = 2 * r;
		//	out_colors[3 * i + 1] = 1;
		//} else
		//{
		//	out_colors[3 * i] = r;
		//	out_colors[3 * i + 1] = 2 * ( 1 - r );
		//}

		//{
		//	out_colors[3 * i] = r;
		//	out_colors[3 * i + 1] = 1 - r;
		//	out_colors[3 * i + 2] = 0.0f;
		//}
	}
	return true;
}

#include "af_serializable_mesh.hxx"
#ifndef _MSC_VER
HPS::Key 
HPS_K_SEQUENTIAL_MESH_to_HOOPS( af_serializable_mesh *in_mesh, FACE * in_face )
{
}
#else
#include <vector>
HPS::Key HPS_K_SEQUENTIAL_MESH_to_HOOPS( af_serializable_mesh *in_mesh, FACE * in_face )
{
	return HPS_INVALID_SEGMENT_KEY;
}
#endif

void HPS_Set_Current_View( HPS::View in_view )
{
	get_hps_state()->s_Current_View_Key = in_view;
}

HPS::View HPS_Get_Current_View()
{
	HPS::View view = get_hps_state()->s_Current_View_Key;
	return view;
}

HPS::SegmentKey HPS_Get_Current_View_Model_Segment_Key()
{
	HPS::View view = get_hps_state()->s_Current_View_Key;
	if ( !view.Empty() )
	{
		HPS::Model model = view.GetAttachedModel();
		if ( !model.Empty() )
			return model.GetSegmentKey();
	}
	return HPS_Invalid_Segment_Key();
}

void HPS_Get_Current_View_Model_KeyPath( HPS::KeyPath & in_key_path )
{
	in_key_path.Reset();
	HPS::View view = get_hps_state()->s_Current_View_Key;
	HPS::Canvas canvas = view.GetOwningLayouts()[0].GetOwningCanvases()[0];
	//keyArray.push_back( view.GetAttachedModel().GetSegmentKey() );
	HPS::IncludeKey model_include_key = view.GetAttachedModelIncludeLink();
	//keyArray.push_back( view.GetSegmentKey() );
	HPS::IncludeKey view_include_key = canvas.GetAttachedLayout().GetAttachedViewIncludeLink( 0 );
	//keyArray.push_back( canvas.GetAttachedLayout().GetSegmentKey() );
	HPS::IncludeKey layout_include_key = canvas.GetAttachedLayoutIncludeLink();
	if ( !model_include_key.Empty() )
		in_key_path.Append( model_include_key );
	if ( !view_include_key.Empty() )
		in_key_path.Append( view_include_key );
	if ( !layout_include_key.Empty() )
		in_key_path.Append( layout_include_key );
	if ( !canvas.GetWindowKey().Empty() )
		in_key_path.Append( canvas.GetWindowKey() );
	return;
}

double HPS_World_Coords_Per_Pixel( /* HPS::Canvas const & in_canvas */ )
{
	HPS::View view( HPS_Get_Current_View() );
	if ( view.Empty() )
		return -1;
	HPS::Point screen_point( 0, 0, 0 );		// Screen position.
	HPS::Point world_point_1( 0, 0, 0 );		// World position 1.
	HPS::Point world_point_2( 0, 0, 0 );		// World position 2.
	HPS::KeyPath key_path;
	HPS_Get_Current_View_Model_KeyPath( key_path );
	bool status = key_path.ConvertCoordinate( HPS::Coordinate::Space::Pixel, screen_point, HPS::Coordinate::Space::World, world_point_1 );
	screen_point[0] = 1.0;
	status = key_path.ConvertCoordinate( HPS::Coordinate::Space::Pixel, screen_point, HPS::Coordinate::Space::World, world_point_2 );
	double dist = distance_to_point( HPS_Point( world_point_1 ), HPS_Point( world_point_2 ) ); // cstr's are converting float array to a double array
	// Using our HPS_Point class to convert between HPS::Point and SPAposition. Not really efficient, unless compiler can optimize. Revisit.
	// Functional cast (C++ style, not C style casts). Can't use a reinterpret_cast.
	return dist;
}

logical HPS_Associate_Key_To_Entity( ENTITY* in_entity, HPS::SegmentKey in_key )
{
	assert( in_entity );
	HPS::KeyArray key_array;
	int count = get_hps_state()->s_pHPS_Map->FindMapping( in_entity, key_array );
	logical mapping_exists = FALSE;
	for ( int i = 0; i < count; i++ )
	{
		if ( key_array[i] == in_key )
		{
			mapping_exists = TRUE;
			break;
		}
	}
	if ( !mapping_exists )
	{
		get_hps_state()->s_pHPS_Map->AddMapping( in_key, in_entity );
		return TRUE;
	}
	return TRUE;
}

logical HPS_Disassociate_Key_From_Entity( ENTITY* in_entity, HPS::SegmentKey in_key )
{
	if ( !in_entity )
		in_entity = get_hps_state()->s_pHPS_Map->FindMapping( in_key );
	if ( in_entity )
	{
		get_hps_state()->s_pHPS_Map->DeleteMapping( in_key, in_entity );
		return TRUE;
	}
	return FALSE;
}

logical HPS_ReRender_Body_Transforms( ENTITY_LIST& in_bodies )
{	// Return TRUE if anything was done
	logical answer( FALSE );
	ENTITY* p_entity = in_bodies.first();
	while ( NULL != p_entity )
	{
		if ( is_BODY( p_entity ) )
		{	// Get the body transform
			float matrix[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
			BODY* p_body = (BODY*)p_entity;
			if ( p_body->transform() )
				transf_to_matrix( matrix, p_body->transform()->transform() );
			// Update the modelling transform for the top level segments
			HPS::KeyArray key_array;
			int count = HPS_Compute_Geometry_Keys( p_entity, key_array );
			for ( int i = 0; i < count; i++ )
			{	// Update the modelling transform for the top level segments
				if ( key_array[i].Type() == HPS::Type::ReferenceKey )
					HPS::ReferenceKey( key_array[i] ).GetModellingMatrixControl().SetElements( 16, matrix );
				else if ( key_array[i].Type() == HPS::Type::SegmentKey )
					HPS::SegmentKey( key_array[i] ).GetModellingMatrixControl().SetElements( 16, matrix );
				answer = TRUE;
			}
		}
		p_entity = in_bodies.next();
	}
	return answer;
}

int Get_Map_Entries()
{
	int answer = get_hps_state()->s_pHPS_Map->HPS_Internal_Get_Map_Entries();
	//	answer +=  get_hps_state()->s_pHPS_MapAsm->HPS_Internal_Get_Map_Entries();
	return answer;
}

logical Is_In_Top_Map( ENTITY* in_ent )
{
	return get_hps_state()->s_pHPS_Map->HPS_Is_In_Top_Map( in_ent );
}

#include "facet_body.hxx"

void opengl_face_list_to_hps_format( int in_ntri, const int* const in_connectivity, std::vector<int>& out_face_list )
{
	out_face_list.clear();
	out_face_list.reserve( 4 * in_ntri );
	for ( int ii = 0; ii < in_ntri; ++ii )
	{
		out_face_list.push_back( 3 );
		out_face_list.push_back( in_connectivity[3 * ii] );
		out_face_list.push_back( in_connectivity[3 * ii + 1] );
		out_face_list.push_back( in_connectivity[3 * ii + 2] );
	}
}

void convert_double_array_to_floats( int const in_npt, double const* const in_pt_coords, std::vector<float>& out_pt_coords_flt )
{
	out_pt_coords_flt.clear();
	out_pt_coords_flt.reserve( 3 * in_npt );
	for ( int ii = 0; ii < 3 * in_npt; ++ii )
		out_pt_coords_flt.push_back( static_cast<float>( in_pt_coords[ii] ) );
}

#if 1
void HPS_Render_FACET_BODY( FACET_BODY* in_body, logical in_renderFaces, logical in_renderEdges, HPS::SegmentKey in_segment_key )
{
	if ( NULL == in_body )
		return;
	if ( in_renderFaces )
	{
		int const nf = in_body->num_faces();
		for ( int face_index = 0; face_index < nf; ++face_index )
		{
			Spa_raw_mesh const& rm = in_body->get_face_mesh( face_index );
			int const * const con = rm.triangle_connectivity();
			int const ntri = rm.num_triangles();
			HPS::PointArray point_array;
			point_array.reserve( 3 * rm.num_vertices() );
			for ( int point_index = 0; point_index < rm.num_vertices() * 3; point_index += 3 )
			{
				point_array.push_back( HPS::Point(
					static_cast<float>( rm.vertex_coordinates()[point_index + 0] ),
					static_cast<float>( rm.vertex_coordinates()[point_index + 1] ),
					static_cast<float>( rm.vertex_coordinates()[point_index + 2] ) ) );
			}
			HPS::IntArray	FaceList;
			FaceList.reserve( 4 * ntri );
			for ( int face_list_index = 0; face_list_index < ntri; ++face_list_index )
			{
				FaceList.push_back( 3 );
				FaceList.push_back( con[3 * face_list_index + 0] );
				FaceList.push_back( con[3 * face_list_index + 1] );
				FaceList.push_back( con[3 * face_list_index + 2] );
			}
			fp_sentry		fps;
			in_segment_key.GetVisibilityControl().SetMarkers( false );
			HPS::ShellKey ShellKey = in_segment_key.InsertShell( point_array, FaceList );
		}
	}
}
#endif
