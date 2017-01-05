/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// Jul-30-09 bhd : R20 fixed UX and NT_DLL issues for getPointCloudName
// Apr-17-09 jkf : R20 moved progress info class to fct
// Feb-03-09 jkf : R20 Check ine ASM map when looking for an ENTITY
//                 to key
// Dec-10-03 jkf : api_facet_curve needs the tolerance to be bigger
//                 than SPAresabs
/*******************************************************************/
///----------------------------------------------------------//
//	Standard C++ Header File(s)
//-----------------------------------------------------------//
//#include <stdio.h>						// standard i/o
#include <ctype.h> // needed for tolower()

//-----------------------------------------------------------//
// HOOPS-specific Header File(s)
//-----------------------------------------------------------//
#include <hc.h>
#ifdef HC_STANDALONE__H
#include HC_STANDALONE__H
#endif

//-----------------------------------------------------------//
// ACIS-specific Header File(s)
//-----------------------------------------------------------//

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
//#include <assert.h>

//-----------------------------------------------------------//
//	User-Specified Header File(s)
//-----------------------------------------------------------//
#include "ha_bridge_internal.h"
#include "ha_map.h"
#include "ha_util.h"
#include "entity_converter.h"
#include "ha_rend_options.h"

#include "ha_map_asm.h"
#include "ha_rend_context.h"
#include "ha_bridge_state.hxx"
#include "mutex.hxx"

#ifndef NT
#include <strings.h>
#define stricmp(x,y) strcasecmp(x,y)
#endif

typedef struct hapoint {
    float	x, y, z;
} HAPoint;
typedef struct hapoint HAVector; 

#define HA_GEOMETRY_TYPE_edges		2 
#define HA_GEOMETRY_TYPE_faces		4
#define HA_GEOMETRY_TYPE_bodies		8
#define HA_GEOMETRY_TYPE_vertices	16

#define HA_MERGE_FACES				2 
#define HA_MERGE_BODIES				4 
#define HA_CREATE_BODY_SEGMENTS		8
#define HA_CREATE_COLOR_SEGMENTS	16
#define HA_TESSELLATE_ELLIPSES		32
#define HA_RENDER_EDGES				64
#define HA_RENDER_FACES				128

#define MAX_ERROR_STRING_LENGTH		1024
#define MAX_SEGMENT_NAME_LENGTH		1024
#define MAX_TYPE_NAME_LENGTH		32

#ifdef THREAD_SAFE_ACIS
SESSION_LOCAL_VAR safe_integral_type<int> init_count;
#else
SESSION_LOCAL_VAR int init_count;
#endif

// kev. Needed a way to call HA_Init once for the session but
// had to ensure it happened after dependent components had been
// initialized. Couldn't use an instance callback's eSessionInit 
// because it is fired before the dependent components are 
// initialized.
SESSION_LOCAL_VAR int one_ha_init_term_per_session;


// Only one for session - init'ed in call to HA_Init.
SESSION_LOCAL_VAR ha_state* local_state;

ha_state* get_ha_state() {
	return local_state;
}

extern mutex_resource ha_bridge_mutex;

#include "comp_handle.hxx"
extern component_handle_list s_HA_CHandles;


//----------------------------------------------------------------------
// purpose---
//    Initialise the hoops_acis_bridge
//----------------------------------------------------------------------

logical
initialize_hoops_acis_bridge()
{
	logical init_ok = TRUE;

	if (init_count++ == 0) {
		init_ok &= initialize_faceter();
		init_ok &= initialize_rendering();
		init_ok &= initialize_generic_attributes();
	}

	// kev. Only call HA_Init once per session (for all threads).
	// Do so after the above components have been initialized.
	{
		CRITICAL_BLOCK( ha_bridge_mutex );
		if ( one_ha_init_term_per_session++ == 0 )
			HA_Init(0);
	}


	return init_ok;
}

outcome api_initialize_hoops_acis_bridge()
{
	return outcome(initialize_hoops_acis_bridge() ? 0 : API_FAILED);
}

//----------------------------------------------------------------------
// purpose---
//    Shut down the hoops_acis_bridge
//----------------------------------------------------------------------
logical
terminate_hoops_acis_bridge()
{
	logical ok = TRUE;

	// kev. Only call HA_Close once per session (for all threads).
	// Do so before the below components have been terminated.
	{
		CRITICAL_BLOCK( ha_bridge_mutex );
		if ( --one_ha_init_term_per_session == 0 )
			HA_Close();
	}

	if (init_count == 0) {
		ok = FALSE;
	} else if (--init_count == 0) {
		ok &= terminate_faceter();
		ok &= terminate_rendering();
		ok &= terminate_generic_attributes();
	}

	return ok;
}

outcome api_terminate_hoops_acis_bridge()
{
	return outcome(terminate_hoops_acis_bridge() ? 0 : API_FAILED);
}

// Jeff Aug09 Moved history callback responsibilities into ha_part.
#if 0
class HA_History_Callbacks : public history_callbacks
{
	// declare any data related to handling roll.  
public:
	void Before_Roll_States() 
	{
		// Do your before roll stuff for a group of DELTA_STATEs
	}
	void Before_Roll_State(DELTA_STATE*) 
	{
		// Do your before roll stuff for one DELTA_STATE;
	}
	void get_changed_entitys(BULLETIN_BOARD* bb, ENTITY_LIST &elist)
	{
		for (BULLETIN *b = bb->start_b; b != NULL; b = b->next_ptr) 
		{
			ENTITY* e2 = b->new_entity_ptr();
			ENTITY* e3 = b->old_entity_ptr();
			ENTITY *owner2, *owner3;
			api_get_owner(e2, owner2);
			api_get_owner(e3, owner3);
			if (owner2 && (!owner3 || owner2 == owner3))
			{
				logical add=FALSE;
				ENTITY_LIST owners;

				if (is_APOINT(owner2))
				{
					if (!((APOINT*)owner2)->get_owners(owners))
						add=TRUE;
				}
				else if (is_CURVE(owner2))
				{
					if (!((CURVE*)owner2)->get_owners(owners))
						add=TRUE;
				}
				else if (is_PCURVE(owner2))
				{
					if (!((PCURVE*)owner2)->get_owners(owners))
						add=TRUE;
				}
				else if (is_SURFACE(owner2))
				{
					if (!((SURFACE*)owner2)->get_owners(owners))
						add=TRUE;
				}
				else if (is_toplevel(owner2))
					add=TRUE;

				if (add)
						elist.add(owner2);
					}
			else if (owner2 && owner2 == owner3)
			{
				if (is_toplevel(owner2))
					elist.add(owner2);
			}
		}
	}

	void Before_Roll_Bulletin_Board(BULLETIN_BOARD* bb, logical discard) 
	{
		// Do your before roll stuff for one BULLETIN_BOARD
		// if discard is true, the  roll is due to  error processing and the 
		// BULLETIN_BOARD will be deleted along with all its BULLETINS.

		if (discard)
			return;
		
		API_NOP_BEGIN
		ENTITY_LIST DelEntities;
		get_changed_entitys(bb,DelEntities);
		HA_Delete_Entity_Geometry(DelEntities);
		API_NOP_END
	}

	void After_Roll_Bulletin_Board(BULLETIN_BOARD* bb, logical discard) 
	{
		// Do your after roll stuff for one BULLETIN_BOARD;
		// if discard is true, the  roll is due to  error processing and the 
		// BULLETIN_BOARD will be deleted along with all its BULLETINS.

		if (discard)
			return;

		API_NOP_BEGIN
		ENTITY_LIST NewEntities;
		get_changed_entitys(bb,NewEntities);
		HA_Render_Entities(NewEntities);
		API_NOP_END
	}

	void After_Roll_State(DELTA_STATE*) 
	{
		// Do your after roll stuff for one DELTA_STATE;
	}

	void After_Roll_States() 
	{
		// Do your after roll stuff for a group of DELTA_STATES
	}
};
#endif


logical analyze_init_options( const char* list )
{
	if( !list )
 	return TRUE;

	char token[1025];
	char llist[1025];
	unsigned long i;
	unsigned long token_number = 0;

	/*my canonize chars*/
	for(i=0; i<1024; i++){

		if(!list[i]) break;	
		llist[i] = (char) tolower((int)list[i]);
	}
	llist[i] = 0;


	/*loop through options*/
	while(HC_Parse_String(llist,",",token_number++,token))
	{
		/*error*/
		const char * mes = "Null option or unknown option";
		const char * fun = "HA_Init";
		HC_Report_Error( 50, 309, 1, 1, &mes, 1, &fun);
		return FALSE;
	}
 return TRUE;
}

extern SESSION_GLOBAL_VAR option_header use_asm_highlight_segments;

void HA_Init( const char* options )
{
	local_state = ACIS_NEW ha_state;

	get_ha_state()->s_pHA_Map = ACIS_NEW HA_Map;
	get_ha_state()->s_pHA_MapAsm = ACIS_NEW HA_Map;
	get_ha_state()->s_pHA_ModelMap = ACIS_NEW HA_ModelMap;
	get_ha_state()->s_pHA_CHandleMap = ACIS_NEW HA_CHandleMap;
	get_ha_state()->s_pFacetOptions = ACIS_NEW facet_options_visualization;

	analyze_init_options( options );

	// Jeff Aug09 Moved history callback responsibilities into ha_part.
	//HA_History_Callbacks *cb = ACIS_NEW HA_History_Callbacks;
	//get_history_callbacks_list().add(cb);
	
	// Set the ENTITY converter.
	HA_Set_Entity_Converter(ACIS_NEW hoops_acis_entity_converter);

	HC_Open_Segment("?Style Library/AcisShellsOn");
		HC_Set_Visibility("faces = on, edges = off, vertices = off");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisShellsOffSilsOn");
		HC_Set_Visibility("faces = off, edges = off, vertices = off");
		HC_Set_Visibility("edges=(interior silhouettes=on)");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisShellsOffSilsOff");
		HC_Set_Visibility("faces = off, edges = off, vertices = off");
		HC_Set_Visibility("edges=(interior silhouettes=off)");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisPolylinesOn");
		HC_Set_Visibility("polylines = on");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisPolylinesOff");
		HC_Set_Visibility("polylines = off");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisMarkersOn");
		HC_Set_Visibility("markers only = on");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisMarkersOff");
		HC_Set_Visibility("markers only = off");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

    if (use_asm_highlight_segments.on())
    {
		HC_Open_Segment("?Style Library/AcisAsmHighlightFacesEdges");
			HC_Set_Rendering_Options("no attribute lock");
			HC_Set_Line_Weight(3.0);
			HC_Set_Visibility("faces, lines = on, markers = off");
			HC_Set_Rendering_Options("attribute lock = visibility");
		HC_Close_Segment();

		HC_Open_Segment("?Style Library/AcisAsmHighlightVerts");
			HC_Set_Rendering_Options("no attribute lock");
			HC_Set_Line_Weight(3.0);
			HC_Set_Marker_Size(1.25);
			HC_Set_Visibility("faces, lines, markers = on");
		HC_Close_Segment();
    }
    else
    {
		HC_Open_Segment("?Style Library/AcisAsmHighlightFaces");
			HC_Set_Rendering_Options("no attribute lock");
			HC_Set_Line_Weight(3.0);
			HC_Set_Visibility("faces, lines = on, markers = off");
			HC_Set_Rendering_Options("attribute lock = (visibility, line weight, color)");
		HC_Close_Segment();

		HC_Open_Segment("?Style Library/AcisAsmHighlightEdges");
			HC_Set_Rendering_Options("no attribute lock");
			HC_Set_Line_Weight(3.0);
			HC_Set_Visibility("faces, lines = on, markers = off");
			HC_Set_Rendering_Options("attribute lock = (visibility, line weight, color)");
		HC_Close_Segment();

		HC_Open_Segment("?Style Library/AcisAsmHighlightVerts");
			HC_Set_Rendering_Options("no attribute lock");
			HC_Set_Line_Weight(3.0);
			HC_Set_Marker_Size(1.25);
			HC_Set_Visibility("faces, lines, markers = on");
			HC_Set_Rendering_Options("attribute lock = (visibility, marker size, line weight, color)");
		HC_Close_Segment();
    }

	HC_Open_Segment("?Style Library/AcisTextOn");
		HC_Set_Visibility("text = on");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisTextOff");
		HC_Set_Visibility("text = off");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisTextLinesOn");
		HC_Set_Visibility("text, lines = on");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();

	HC_Open_Segment("?Style Library/AcisTextLinesOff");
		HC_Set_Visibility("text, lines = off");
		HC_Set_Rendering_Options("attribute lock = visibility");
	HC_Close_Segment();
}

char* HA_Build_Segment_String(ENTITY* entity, char* inbuffer, const char* pattern)
{
	if (!pattern || !*pattern) 
		pattern = get_ha_state()->s_RenderingOptions.GetPattern();

	if (!pattern) 
	{
		*inbuffer = 0;
		return inbuffer;
	}

	char pbuffer[POINTER_BUFFER_SIZE];

	char* buffer = inbuffer;
	unsigned int token_number = 0;
	char token[1025];

	while ( HC_Parse_String( pattern, "/", token_number++, token ) )
	{
		if ( strcmp( token, "history" ) == 0 )
		{
			if ( entity )
				strcpy(buffer, ptoax(entity->history(), pbuffer));
			else
			{
				HISTORY_STREAM *default_hs = NULL;
				api_get_default_history( default_hs );
				strcpy( buffer, ptoax( default_hs, pbuffer ) );
				//sprintf( buffer, "HIST_%d", default_hs->tag( entity ) );
			}
		} else if ( strcmp( token, "entity" ) == 0 )
			strcpy( buffer, ptoax( entity, pbuffer ) );
		else if ( strcmp( token, "type" ) == 0 && entity )
			strcpy( buffer, entity->type_name() );
		else if ( is_ASM_ASSEMBLY( entity ) && strcmp( token, "handle" ) == 0 )
		{
			ASM_ASSEMBLY* assembly = (ASM_ASSEMBLY*)entity;
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

	return inbuffer;
}

HC_KEY HA_KOpen_Segment(ENTITY* entity, const char* pattern)
{
	char buffer[1025];
	HA_Build_Segment_String(entity, buffer, pattern);
	return HC_KOpen_Segment(buffer);
}

HC_KEY HA_KCreate_Segment(ENTITY* entity, const char* pattern)
{
	HC_KEY key = HA_KOpen_Segment(entity, pattern);
	HC_Close_Segment();
	return key;
}

void HA_Set_Rendering_Options(const char * list)
{
	get_ha_state()->s_RenderingOptions.Set_Rendering_Options(list);
}

void HA_Set_Rendering_Options(const ha_rendering_options &rendering_options)
{
	get_ha_state()->s_RenderingOptions=rendering_options;
}

ha_rendering_options &HA_Get_Rendering_Options()
{
	return get_ha_state()->s_RenderingOptions;
}

void HA_Set_Facet_Options(facet_options *facet_opts)
{
	if (get_ha_state()->s_pFacetOptions)
		ACIS_DELETE get_ha_state()->s_pFacetOptions;

	get_ha_state()->s_pFacetOptions = facet_opts;
}

facet_options *HA_Get_Facet_Options()
{
	return get_ha_state()->s_pFacetOptions;
}

void HA_Close( void )
{
	fp_sentry fps;

	s_HA_CHandles.clear();

	if (HC_QShow_Existence("?Style Library/AcisShellsOn", "self"))
		HC_Delete_Segment("?Style Library/AcisShellsOn");

	if (HC_QShow_Existence("?Style Library/AcisShellsOffSilsOn", "self"))
		HC_Delete_Segment("?Style Library/AcisShellsOffSilsOn");

	if (HC_QShow_Existence("?Style Library/AcisShellsOffSilsOff", "self"))
		HC_Delete_Segment("?Style Library/AcisShellsOffSilsOff");

	if (HC_QShow_Existence("?Style Library/AcisPolylinesOn", "self"))
		HC_Delete_Segment("?Style Library/AcisPolylinesOn");

	if (HC_QShow_Existence("?Style Library/AcisPolylinesOff", "self"))
		HC_Delete_Segment("?Style Library/AcisPolylinesOff");

	if (HC_QShow_Existence("?Style Library/AcisMarkersOn", "self"))
		HC_Delete_Segment("?Style Library/AcisMarkersOn");

	if (HC_QShow_Existence("?Style Library/AcisMarkersOff", "self"))
		HC_Delete_Segment("?Style Library/AcisMarkersOff");

	if (HC_QShow_Existence("?Style Library/AcisAsmHighlightFaces", "self"))
		HC_Delete_Segment("?Style Library/AcisAsmHighlightFaces");

	if (HC_QShow_Existence("?Style Library/AcisAsmHighlightEdges", "self"))
		HC_Delete_Segment("?Style Library/AcisAsmHighlightEdges");

	if (HC_QShow_Existence("?Style Library/AcisAsmHighlightVerts", "self"))
		HC_Delete_Segment("?Style Library/AcisAsmHighlightVerts");

	IEntityConverter *icvrt=HA_Get_Entity_Converter();
	HA_Set_Entity_Converter(0);
	ACIS_DELETE icvrt;

	if (get_ha_state()->s_pHA_Map)
	{
		ACIS_DELETE get_ha_state()->s_pHA_Map;
		get_ha_state()->s_pHA_Map=0;
	}

	if (get_ha_state()->s_pHA_MapAsm)
	{
		ACIS_DELETE get_ha_state()->s_pHA_MapAsm;
		get_ha_state()->s_pHA_MapAsm=0;
	}

	if (get_ha_state()->s_pHA_ModelMap)
	{
		ACIS_DELETE get_ha_state()->s_pHA_ModelMap;
		get_ha_state()->s_pHA_ModelMap=0;
	}

	if (get_ha_state()->s_pHA_CHandleMap)
	{
		ACIS_DELETE get_ha_state()->s_pHA_CHandleMap;
		get_ha_state()->s_pHA_CHandleMap=0;
	}

	if (get_ha_state()->s_pFacetOptions)
	{
		ACIS_DELETE get_ha_state()->s_pFacetOptions;
		get_ha_state()->s_pFacetOptions=0;
	}

	ACIS_DELETE local_state;
	local_state = NULL;

    HC_Reset_System();
}

unsigned long HA_Internal_Compute_Geometry_Keys(ENTITY* entity, HC_KEY* keys, unsigned long count, unsigned long geomTypes )
{
	ENTITY_LIST entities;
	ENTITY* edgeOrFace;
	outcome o;
	unsigned long numEntities;
	unsigned long num_keys = 0;
	unsigned long i;

	if (is_BODY(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_vertices)
		{
			int temp_key_count;
			o = api_get_vertices(entity, entities);
			check_outcome(o);
			numEntities = entities.count(); 
			for (i = 0; i < numEntities; i++)
			{
				edgeOrFace = entities[i];
				temp_key_count = HA_Internal_Compute_Geometry_Keys(edgeOrFace, &keys[num_keys], count-num_keys, geomTypes );
				assert(temp_key_count >= 0);  // check for errors
				if (temp_key_count < 0)
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if (geomTypes & HA_GEOMETRY_TYPE_edges)
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_edges(entity, entities);
			check_outcome(o);
			numEntities = entities.count(); 
			for (i = 0; i < numEntities; i++)
			{
				edgeOrFace = entities[i];
				temp_key_count = HA_Internal_Compute_Geometry_Keys(edgeOrFace, &keys[num_keys], count-num_keys, geomTypes);
				assert(temp_key_count >= 0);  // check for errors
				if (temp_key_count < 0)
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if (geomTypes & HA_GEOMETRY_TYPE_faces)
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_faces(entity, entities);
			check_outcome(o);
			numEntities = entities.count(); 
			for (i = 0; i < numEntities; i++)
			{
				edgeOrFace = entities[i];
				temp_key_count = HA_Internal_Compute_Geometry_Keys(edgeOrFace, &keys[num_keys], count-num_keys, geomTypes );
				assert(temp_key_count >= 0);  // check for errors
				if (temp_key_count < 0)
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if (geomTypes & HA_GEOMETRY_TYPE_bodies)
		{
			if (num_keys < count)
			{
				num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, &keys[num_keys], count-num_keys);
				num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, &keys[num_keys], count-num_keys);
			}
		}
	}
	else if (is_FACE(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_faces)
		{
			if (num_keys < count)
			{
				// there may be more than one hoops key mapped to a particular face.
				// so use the findmapping function which can return more than one key.
				// the last arg ensures that we don't stomp past the end of the array. rlw
				num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, &keys[num_keys], count-num_keys);
				num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, &keys[num_keys], count-num_keys);
			}
		}
	}
	else if (is_EDGE(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_edges)
		{
			if (num_keys < count)
			{
				// there may be more than one hoops key mapped to a particular edge.
				// so use the findmapping function which can return more than one key.
				// the last arg ensures that we don't stomp past the end of the array. rlw
				num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, &keys[num_keys], count-num_keys);
				num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, &keys[num_keys], count-num_keys);
			}
		}
	}
	else if (is_VERTEX(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_vertices)
		{
			if (num_keys < count)
			{
				num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, &keys[num_keys], count-num_keys);
				num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, &keys[num_keys], count-num_keys);
			}
		}
	}
	return num_keys;
}

unsigned long HA_Compute_Geometry_Keys(ENTITY* entity, unsigned long count, HC_KEY* keys, const char * geomtypes)
{
	char token[1025];
	char llist[1025];
	unsigned long options = 0;
	unsigned long bitmask = 0;
	unsigned long token_number = 0;

	int i;
		
	/*my canonize chars*/
	for(i=0; i<1024; i++){

		if(!geomtypes[i]) break;	
		llist[i] = (char) tolower((int)geomtypes[i]);
	}
	llist[i] = 0;

	/*loop through options*/
	while(HC_Parse_String(llist,",",token_number++,token))
	{
		if (strstr(token, "any"))
			bitmask = HA_GEOMETRY_TYPE_edges | HA_GEOMETRY_TYPE_faces | HA_GEOMETRY_TYPE_bodies | HA_GEOMETRY_TYPE_vertices;
		else if (strstr(token,"edges"))
			bitmask = HA_GEOMETRY_TYPE_edges;
		else if (strstr(token,"faces"))
			bitmask = HA_GEOMETRY_TYPE_faces;
		else if (strstr(token,"bodies"))
			bitmask = HA_GEOMETRY_TYPE_bodies;
		else if (strstr(token,"vertices"))
			bitmask = HA_GEOMETRY_TYPE_vertices;
		else
		{
			/*error*/
			char * mes = "Null option or unknown option";
			
			const char * fun = "HA_Set_Rendering_Options";

			HC_Report_Error( 50, 309, 1, 1, &mes, 1, &fun);
			return 0;
		}

		if (!Parse_YesNo_And_Mutate_Options_Using_Bitmask(token, bitmask, &options))
			return 0; // parse error, don't set the options.
	}
	return HA_Internal_Compute_Geometry_Keys(entity, keys, count, options);
}

unsigned long HA_Compute_Geometry_Key_Count(ENTITY* entity)
{
	HC_KEY	keys[40000];
    unsigned long retval = get_ha_state()->s_pHA_Map->FindMapping(entity, keys, 40000);
//    if (0 == retval)
    {
        // jkf Feb02 R20 Inspect ASM map as well 88512
        retval += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, keys, 40000);
    }

    return retval;
}

unsigned long HA_Compute_Geometry_Keys(ENTITY* entity, unsigned long count, HC_KEY* keys)
{
    unsigned long retval = 0;
	// there may be more than one hoops key mapped to a particular entity.
	// so use the findmapping function which can return more than one key.
	// the last arg ensures that we don't stomp past the end of the array. rlw
    retval = get_ha_state()->s_pHA_Map->FindMapping(entity, keys, count);

//    if (0 == retval)
    {
        // jkf Feb02 R20 Inspect ASM map as well 88512
        retval += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, &keys[retval], 40000);
    }
    return retval;
}

unsigned long HA_Internal_Compute_Geometry_Key_Count(ENTITY* entity, unsigned long geomTypes )
{
	HC_KEY keys[3];
	ENTITY_LIST entities;
	ENTITY* edgeOrFace;
	outcome o;
	unsigned long numEntities;
	unsigned long num_keys = 0;
	unsigned long i;
	
	if (is_BODY(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_vertices)
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_vertices(entity, entities);
			check_outcome(o);

			numEntities = entities.count(); 
			for (i = 0; i < numEntities; i++)
			{
				edgeOrFace = entities[i];
				temp_key_count = HA_Internal_Compute_Geometry_Key_Count(edgeOrFace, geomTypes );
				assert(temp_key_count >= 0);  // check for errors
				if (temp_key_count < 0)
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if (geomTypes & HA_GEOMETRY_TYPE_edges)
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_edges(entity, entities);
			check_outcome(o);
			numEntities = entities.count(); 
			for (i = 0; i < numEntities; i++)
			{
				edgeOrFace = entities[i];
				temp_key_count = HA_Internal_Compute_Geometry_Key_Count(edgeOrFace, geomTypes);
				assert(temp_key_count >= 0);  // check for errors
				if (temp_key_count < 0)
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if (geomTypes & HA_GEOMETRY_TYPE_faces)
		{
			int temp_key_count;
			// must actually get the geometry here to check for circles
			o = api_get_faces(entity, entities);
			check_outcome(o);
			numEntities = entities.count(); 
			for (i = 0; i < numEntities; i++)
			{
				edgeOrFace = entities[i];
				temp_key_count = HA_Internal_Compute_Geometry_Key_Count(edgeOrFace, geomTypes );
				assert(temp_key_count >= 0);  // check for errors
				if (temp_key_count < 0)
					return temp_key_count;  // return error code
				num_keys += temp_key_count;
			}
			entities.clear();
		}
		if (geomTypes & HA_GEOMETRY_TYPE_bodies)
		{
			num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, keys, 3);
			num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, keys, 3);
		}
	}
	else if (is_FACE(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_faces)
		{
			// there may be more than one hoops key mapped to a particular face.
			// so use the findmapping function which can return more than one key.
			// the last arg ensures that we don't stomp past the end of the array. rlw
			num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, keys, 3);
			num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, keys, 3);
		}
	}
	else if (is_EDGE(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_edges)
		{
			// there may be more than one hoops key mapped to a particular edge.
			// so use the findmapping function which can return more than one key.
			// the last arg ensures that we don't stomp past the end of the array. rlw
			num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, keys, 3);
			num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, keys, 3);
		}
	}
	else if (is_VERTEX(entity))
	{
		if (geomTypes & HA_GEOMETRY_TYPE_vertices)
		{
			num_keys += get_ha_state()->s_pHA_Map->FindMapping(entity, keys, 3);
			num_keys += get_ha_state()->s_pHA_MapAsm->FindMapping(entity, keys, 3);
		}
	}

	return num_keys;
}

unsigned long HA_Compute_Geometry_Key_Count(ENTITY* entity, const char * geomtypes)
{
	char token[1025];
	char llist[1025];
	unsigned long options = 0;
	unsigned long bitmask = 0;
	unsigned long token_number = 0;

	int i;
		
	/*my canonize chars*/
	for(i=0; i<1024; i++){

		if(!geomtypes[i]) break;	
		llist[i] = (char) tolower((int)geomtypes[i]);
	}
	llist[i] = 0;


	/*loop through options*/
	while(HC_Parse_String(llist,",",token_number++,token))
	{
		if (strstr(token, "any"))
			bitmask = HA_GEOMETRY_TYPE_edges | HA_GEOMETRY_TYPE_faces | HA_GEOMETRY_TYPE_bodies | HA_GEOMETRY_TYPE_vertices;
		else if (strstr(token,"edges"))
			bitmask = HA_GEOMETRY_TYPE_edges;
		else if (strstr(token,"faces"))
			bitmask = HA_GEOMETRY_TYPE_faces;
		else if (strstr(token,"bodies"))
			bitmask = HA_GEOMETRY_TYPE_bodies;
		else if (strstr(token,"vertices"))
			bitmask = HA_GEOMETRY_TYPE_vertices;
		else
		{
			/*error*/
			char * mes = "Null option or unknown option";
			
			const char * fun = "HA_Set_Rendering_Options";

			HC_Report_Error( 50, 309, 1, 1, &mes, 1, &fun);
			return 0;
		}

		if (!Parse_YesNo_And_Mutate_Options_Using_Bitmask(token, bitmask, &options))
			return 0; // parse error, don't set the options.
	}
	return HA_Internal_Compute_Geometry_Key_Count(entity, options);
}

ENTITY* HA_Compute_Entity_Pointer (HC_KEY key)
{
	ENTITY* entity=0;
	entity = get_ha_state()->s_pHA_Map->FindMapping(key);
    if (NULL == entity)
    {
        // jkf Feb02 R20 Inspect ASM map as well 88512
        entity = get_ha_state()->s_pHA_MapAsm->FindMapping(key);
    }
    
	return entity;

}

ENTITY* HA_Compute_Entity_Pointer (HC_KEY key, int acisClass)
{
	ENTITY* entity;

	// TODO: A key may not just represent a built in ACIS entity type
	//       like face/edge/vert, it may be a customer entity.
	entity = get_ha_state()->s_pHA_Map->FindMapping(key);

	/*
	if (!entity)
	{
		if (acisClass == BODY_TYPE)
		{
			HC_KEY ancestorSegment;
			// be more forgiving if they are interested in a body pointer; 
			// in case this entity has been rendered in merge_faces mode we need
			// to be a bit tricky about obtaining this info, if and only if 
			// they are looking to get the body associated with a given shell.
			char keytype[120];
			HC_Show_Key_Type(key, keytype);
			if (keytype[0] == 's' && keytype[1] == 'h')
			{
				// look to the parent and grandparent segment of this shell in the hopes of finding
				// a body segment, which will ahve a mapping to the body entity.
				ancestorSegment = HC_KShow_Owner_By_Key(key);
				entity = s_pHA_Map->FindMapping(ancestorSegment);
				if (!entity)
				{
					ancestorSegment = HC_KShow_Owner_By_Key(ancestorSegment);
					entity = s_pHA_Map->FindMapping(ancestorSegment);

					if( !entity )
					{
						ancestorSegment = HC_KShow_Owner_By_Key(ancestorSegment);
						entity = s_pHA_Map->FindMapping(ancestorSegment);
						if( entity )
							assert( is_BODY(entity));
					}
					else
					{
						assert(is_BODY(entity));
					}
					return entity; // if entity is 0, that's an appropriate return value here.
				}
			}
			else
				return 0;
		}
		else
			return 0; 
	}
	
	*/

	if( !entity )
	{
		if( acisClass == BODY_TYPE )
		{
			HC_KEY ancestorSegment = key;
			char keytype[120];
			HC_Show_Key_Type( key, keytype );
			if( keytype[0] == 's' && keytype[1] == 'h' )
			{
				while( (!entity) && (ancestorSegment != -1 ) )
				{
					ancestorSegment = HC_KShow_Owner_By_Key(ancestorSegment);
					if (ancestorSegment != -1)
						entity = get_ha_state()->s_pHA_Map->FindMapping(ancestorSegment);
				}
				if( entity )
				{
					assert( is_BODY(entity) );
					return entity;
				}
//				return entity;
			}
//			else
//				return 0;
		}
//		else
//			return 0;
	}
	
	if (!entity)
		entity = get_ha_state()->s_pHA_MapAsm->FindMapping(key);
	
	if (!entity)
	{
		if( acisClass == BODY_TYPE )
		{
			HC_KEY ancestorSegment = key;
			char keytype[120];
			HC_Show_Key_Type( key, keytype );
			if( keytype[0] == 's' && keytype[1] == 'h' )
			{
				while( (!entity) && (ancestorSegment != -1 ) )
				{
					ancestorSegment = HC_KShow_Owner_By_Key(ancestorSegment);
					if (ancestorSegment != -1)
						entity = get_ha_state()->s_pHA_MapAsm->FindMapping(ancestorSegment);
				}
				if( entity )
				{
					assert( is_BODY(entity) );
				}
				return entity;
			}
			else
				return 0;
		}
		else
			return 0;
	}
	

	if (is_EDGE(entity))
	{
		if (acisClass == EDGE_TYPE)
			return entity;
		else if (acisClass == BODY_TYPE)
		{
			ENTITY* owning_body;
			outcome o;
			o = api_get_owner(entity, owning_body);
			assert(o.ok());

			return owning_body;
		}
		else
			return 0;
	}
	else if (is_FACE(entity))
	{
		if (acisClass == FACE_TYPE)
			return entity;
		else if (acisClass == BODY_TYPE)
		{
			ENTITY* owning_body=0;
			outcome o;
			o = api_get_owner(entity, owning_body);
			assert(o.ok());

			return owning_body;
		}
		else
			return 0;
	}
	else if (is_VERTEX(entity))
	{
		if (acisClass == VERTEX_TYPE)
			return entity;
		else if (acisClass == BODY_TYPE)
		{
			ENTITY* owning_body = NULL;
			outcome o;
			o = api_get_owner(entity, owning_body);
			assert(o.ok());

			return owning_body;
		}
		else
			return 0;
	}
	else if (is_BODY(entity))
	{
		if (acisClass == BODY_TYPE)
		{
			return entity;
		}
		else
		{
			return 0;
		}
	}
	else
		return 0;
	return 0;
}


// This function takes all the shells it finds in a segment ( filtered by the bodies_to_merge list )
// and combines them into a single shell for rendering performance purposes.
static void merge_body_faces_in_currently_open_segment(BODY** bodies_to_merge, unsigned long num_bodies)
{
	int shell_point_list_len = 0;
	int shell_face_list_len = 0;
	int max_shell_point_list_len = 0;
	int max_shell_face_list_len = 0;
	int total_point_list_len = 0;
	int total_face_list_len = 0;
	int optimized_point_list_len = 0;
	int optimized_face_list_len = 0;
	int total_point_list_offset = 0;
	int total_face_list_offset = 0;
	int i = 0;

	char type[ MAX_TYPE_NAME_LENGTH ];
	HC_KEY key = 0;
	ENTITY* face_pointer = 0;
	logical result = FALSE;

	fp_sentry fps;

	// search for shells.  if the shell belongs to the body being merged, increment the size of the 
	// working arrays.
	HC_Begin_Contents_Search (".", "shells");
	while (HC_Find_Contents (type, &key)) 
	{
		unsigned long i;
		ENTITY* face_parent_body;
		face_parent_body = HA_Compute_Entity_Pointer(key, BODY_TYPE);
		for (i = 0; i < num_bodies; i++)
		{
			if (face_parent_body == bodies_to_merge[i])
			{
				HC_Show_Shell_Size(key, &shell_point_list_len, &shell_face_list_len );
				total_point_list_len += shell_point_list_len;
				total_face_list_len += shell_face_list_len;
				if (max_shell_point_list_len < shell_point_list_len)
					max_shell_point_list_len = shell_point_list_len;
				if (max_shell_face_list_len < shell_face_list_len)
					max_shell_face_list_len = shell_face_list_len;
				break; // found the body in the array; no need to search more
			}
		}
	}
	HC_End_Contents_Search();
	// search for shells.  if the shell belongs to the body being merged, build the 
	// working arrays.
	if (total_point_list_len && total_face_list_len)
	{
		HAPoint *shell_point_list;
		int *shell_face_list;

		HAPoint *total_point_list;
		HAVector *total_normal_list;
		int *total_face_list;

		HAPoint *optimized_point_list;
		int *optimized_face_list;

		
		int *point_mapping_list = 0;
		int *face_mapping_list = 0;
		
		shell_point_list = (HAPoint*) malloc (max_shell_point_list_len * sizeof(HAPoint));
		shell_face_list = (int*) malloc (max_shell_face_list_len * sizeof(int));

		total_point_list = (HAPoint*) malloc (total_point_list_len * sizeof(HAPoint));
		total_normal_list = (HAVector*) malloc (total_point_list_len * sizeof(HAVector));
		total_face_list = (int*) malloc (total_face_list_len * sizeof(int));

		optimized_point_list = (HAPoint*) malloc (total_point_list_len * sizeof(HAPoint));
		optimized_face_list = (int*) malloc (total_face_list_len * sizeof(int));

	    point_mapping_list = (int *) malloc (total_point_list_len * sizeof (int));

		face_mapping_list = (int *) malloc (total_face_list_len * sizeof (int));

		assert(shell_point_list);
		assert(shell_face_list);

		assert(total_point_list);
		assert(total_normal_list);
		assert(total_face_list);

		assert(optimized_point_list);
		assert(optimized_face_list);

		HC_Begin_Contents_Search (".", "shells");
		while (HC_Find_Contents (type, &key)) 
		{
			int i;
			unsigned long j;
			ENTITY* face_parent_body;
			face_parent_body = HA_Compute_Entity_Pointer(key, BODY_TYPE);
			for (j = 0; j < num_bodies; j++)
			{
				if (face_parent_body == bodies_to_merge[j])
				{
					// the shell we've found belongs to the body we're merging,
					// absorb it.
					HC_Show_Shell(key, &shell_point_list_len, shell_point_list, &shell_face_list_len, shell_face_list );
					HC_Open_Geometry( key );  // we'll be querying normals soon
					for (i = 0; i < shell_point_list_len; i++)
					{
						total_point_list[total_point_list_offset + i].x = shell_point_list[i].x;
						total_point_list[total_point_list_offset + i].y = shell_point_list[i].y;
						total_point_list[total_point_list_offset + i].z = shell_point_list[i].z;
						HC_Open_Vertex( i );
						HC_Show_Net_Normal( &total_normal_list[total_point_list_offset + i].x,
											&total_normal_list[total_point_list_offset + i].y,
											&total_normal_list[total_point_list_offset + i].z);
						HC_Close_Vertex();
					}
					HC_Close_Geometry();

					for (i = 0; i < shell_face_list_len; )
					{
						int j;
						assert(shell_face_list[i] > 0); // if negative, means subtract this poly from shell.
						// this integration should never result in us seeing that case.

						total_face_list[total_face_list_offset + i] = shell_face_list[i];
						for (j = 1; j <= shell_face_list[i]; j++)
						{
							total_face_list[total_face_list_offset + i + j] = shell_face_list[i+j] + total_point_list_offset;
						}
						i += 1 + shell_face_list[i];
					}

					total_face_list_offset += shell_face_list_len;
					total_point_list_offset += shell_point_list_len;

					// find out the pointer of the FACE entity of the shell we're deleting.
					// when we map the created shell to an entity, we will map it to 
					// the BODY entity
					face_pointer = HA_Compute_Entity_Pointer(key, FACE_TYPE);
					// now, delete the found shell and de-map it; there will no longer be any mappings between the shell key and any ACIS entity
					get_ha_state()->s_pHA_Map->DeleteMapping(key);
					get_ha_state()->s_pHA_Map->DeleteMapping( face_pointer);
					get_ha_state()->s_pHA_MapAsm->DeleteMapping(key);
					get_ha_state()->s_pHA_MapAsm->DeleteMapping( face_pointer);
					HC_Delete_By_Key(key);
				}
			}
		}
		HC_End_Contents_Search();

		// list is assembled, now put it together into a single optimized shell.
		HC_Compute_Optimized_Shell(total_point_list_len, total_point_list,
                              (float *)total_normal_list,
                              total_face_list_len, total_face_list,
                              "tolerance = 50% fru, normal tolerance = 1degrees",
                              &optimized_point_list_len,optimized_point_list,
                              &optimized_face_list_len, optimized_face_list,
                              point_mapping_list, face_mapping_list );

		key = HC_KInsert_Shell (optimized_point_list_len, 
								optimized_point_list,
								optimized_face_list_len, 
								optimized_face_list );

		// set the normals
		HC_Open_Geometry(key);
		for( i = 0 ; i < total_point_list_len ; i++ )
		{
			if( point_mapping_list[i] >= 0 )
			{
				HC_Open_Vertex( point_mapping_list[i] );
					HC_Set_Normal( total_normal_list[i].x,
									total_normal_list[i].y,
									total_normal_list[i].z );
				HC_Close_Vertex();
			}
		}
		HC_Close_Geometry();
		
		assert(face_pointer != 0);

		if (shell_point_list)
			free(shell_point_list);
		if (shell_face_list)
			free(shell_face_list);

		if (total_point_list)
			free(total_point_list);
		if (total_normal_list)
			free(total_normal_list);
		if (total_face_list)
			free(total_face_list);

		if (optimized_point_list)
			free(optimized_point_list);
		if (optimized_face_list)
			free(optimized_face_list);

		if (point_mapping_list)
			free(point_mapping_list);
		if (face_mapping_list)
			free(face_mapping_list);
	}
	
}

static void merge_body_faces_recurse_subsegments(BODY** bodies_to_merge, unsigned long num_bodies)
{
	// Merge shells in the current segment
	merge_body_faces_in_currently_open_segment(bodies_to_merge, num_bodies);

	// now iterate through all the child segments
	HC_KEY child;
	char type[ MAX_TYPE_NAME_LENGTH ];

	HC_Begin_Contents_Search (".","segment");
	while( HC_Find_Contents (type, &child))
	{
		HC_Open_Segment_By_Key (child);
		merge_body_faces_recurse_subsegments( bodies_to_merge, num_bodies);
		HC_Close_Segment();  // close previously opened color segment
	}

	HC_End_Contents_Search();
}


// merge_body_faces iterates through the segments that represent a body, and gathers
// together any shells that are in the same subsegment into a single shell.  Usually
// shells are split into seperate segments by color, and usually a seperate shell
// represents a seperate face.  When shells are combined, their mappings to faces
// will be lost, so you gain rendering efficiency but lost selection capability.
void merge_body_faces(BODY** bodies_to_merge, unsigned long num_bodies, const char* pattern)
{
	// Lets look for a body segment mapping.  If on, then there should only be one body in the list
	// and we will open it's segment up for processing.
	if( get_ha_state()->s_RenderingOptions.GetBodySegmentMode())
	{
		assert(num_bodies == 1);
		if (num_bodies != 1)
			return;
		HC_KEY key = 0;
		HA_Internal_Compute_Geometry_Keys(bodies_to_merge[0], &key, 1, HA_GEOMETRY_TYPE_bodies);
		assert(key);
		if( !key )
			return;
		HC_Open_Segment_By_Key(key);
	}
	else
	{
		// try to open the segment where the pattern points.  This is presumed to be the opening segment
		// where all the entities we are interested in is stored. 
		HC_KEY key = 0;
		HC_KOpen_Segment( pattern);
	}

	// Merge the appropriate shells in the current subtree
	merge_body_faces_recurse_subsegments(bodies_to_merge, num_bodies);

	// We always have some segment open
	HC_Close_Segment();
}

//
// Begin Point Cloud Code
//

#define PC_PARENT_SEGEMENT "AcisPointClouds"
#define MAX_PC_PATH 256

char* getPointCloudName( SPApoint_cloud* inputCloud, logical create_parent_segment = FALSE )
{
	if (inputCloud == NULL) 
	{
		return 0;
	}

	HC_KEY viewSegmentKey = HA_Get_Current_View_Segment(); // Gets the root view segment key
	if (viewSegmentKey == 0)
	{
		return 0;
	}

	// Jeff - Probably don't need to open and close the segment if we cannot set the driver option.
	// We should ifdef this code to hoops versions >= 17
	//char ownerName[1024];
	//HC_Show_Owner_By_Key( viewSegmentKey, ownerName);
	//HC_Open_Segment(ownerName);
		// bhd 30Jul09 - this doesn't work in NT_DLL (presumably because we link against HOOPS 16)
		// and we cannot measure a performance improvement, anyway
		//HC_Set_Driver_Options("marker drawing = fastest");
	//HC_Close_Segment();

	char parentPath[256];
	HC_Show_Segment( viewSegmentKey, parentPath);

	if (create_parent_segment)
	{
		char pointCloudCollection[256]; // Name of the Parent Segment that collects all point clouds
		sprintf( pointCloudCollection,"%s/%s", parentPath, PC_PARENT_SEGEMENT);

		HC_Begin_Segment_Search (pointCloudCollection);
		HC_BOOLEAN segFound = HC_Find_Segment(NULL);
		HC_End_Segment_Search();
		if (!segFound)
		{
			HC_Open_Segment(pointCloudCollection);
				HC_Set_Marker_Symbol(".");
				HC_Set_Visibility("markers=on");  
				HC_Set_Variable_Marker_Size("1 px");
				HC_Set_Rendering_Options("display lists = on, lod = on");
			HC_Close_Segment();
		}
	}

	char pointCloudName[MAX_PC_PATH];
	sprintf( pointCloudName, "%s/%s/0x%X", parentPath, PC_PARENT_SEGEMENT, inputCloud); // Create a unique name for the point cloud segment

	return ACIS_STRDUP(pointCloudName);
}

HC_BOOLEAN HA_Erase_PointCloud(SPApoint_cloud* inputCloud)
{
	if (inputCloud == NULL)
	{
		return FALSE;
	}

	char* pointCloudName = getPointCloudName( inputCloud);
	if (pointCloudName == NULL)
	{
		return FALSE;
	}

	HC_Begin_Segment_Search( pointCloudName);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();

	if (segFound)
	{
		HC_Delete_Segment( pointCloudName);
		HC_Update_Display();
	}

	ACIS_FREE(pointCloudName);
	
	return segFound;
}

HC_KEY HA_Highlight_PointCloud(SPApoint_cloud* inputCloud, const rgb_color& inColor, logical turnOn)
{
	
	if (!inputCloud) 
		return 0;

	HC_KEY viewSegmentKey = HA_Get_Current_View_Segment(); // Gets the root view segment key

	if (viewSegmentKey == 0)
		return 0;

	char parentPath[256];
	HC_Show_Segment(viewSegmentKey,parentPath);
	HC_Open_Segment_By_Key(viewSegmentKey); // Open View Segment Root

	char pointCloudCollection[256]; // Name of the Parent Segment that collects all point clouds
	sprintf(pointCloudCollection,"%s/%s",parentPath, PC_PARENT_SEGEMENT);
	HC_Open_Segment(pointCloudCollection);

	char pointCloudHighLight[256];
	sprintf(pointCloudHighLight,"%s/%s",pointCloudCollection,"HighLight"); // Create a unique name for the HighLightSegment segment


	fp_sentry fps;
	HC_KEY highLightSegmentKey = 0;
	if (turnOn)
	{
		// Create a HighLight Segment and Add Points to It
		highLightSegmentKey=HC_KOpen_Segment(pointCloudHighLight);
		HC_Set_Marker_Symbol("*");
		HC_Set_Color_By_Value("marker","RGB",inColor.red(),inColor.green(),inColor.blue());
		HC_Set_Visibility("markers=on");  
		HC_Set_Marker_Size(0.5);
		HC_Set_Driver_Options ("marker drawing = fastest");


		SPApoint_cloud_iterator pci( *inputCloud );
		SPAposition nextPos;
		while ( pci.next( nextPos ) )
		{
			HC_Insert_Marker( nextPos.x(),nextPos.y(), nextPos.z() );
		}

		HC_Close_Segment();  // Current Highlight segment.
	}
	else
	{
		// Remove the HighLight Segment
		HC_Begin_Segment_Search (pointCloudHighLight);
		HC_BOOLEAN segFound = HC_Find_Segment(NULL);
		HC_End_Segment_Search();

		if (segFound)
			HC_Delete_Segment(pointCloudHighLight);
	}
	
	HC_Close_Segment(); // Point Cloud Collection Segment
	HC_Close_Segment();  // Root View Segment

	HC_Update_Display();

	return highLightSegmentKey;
}

#ifdef THREAD_SAFE_ACIS
SESSION_LOCAL_VAR safe_integral_type<int> pos_cloud_seg_num;
#else
static int pos_cloud_seg_num;
#endif

HC_KEY HA_Render_PositionCloud(	SPAposition_cloud const& cloud )
{
	const int max_buffer_pts=10000;
	SPAposition pts[max_buffer_pts];

	char segment_name[256];
	sprintf(segment_name, "position cloud %d",pos_cloud_seg_num++);
	HC_KEY key =0;
	EXCEPTION_BEGIN // ensure segment is closed even on error.
	key=HC_KOpen_Segment(segment_name);
	EXCEPTION_TRY
	//HC_QSet_Color_By_Value(segment_name,"marker","rgb",in_color.red(),in_color.green(),in_color.blue());
	//HC_Set_Marker_Symbol( marker_style );		
	//HC_Set_Marker_Size( marker_size );


	int npts=0;
	SPAbox bx;
	SPAposition_cloud_iterator itr=cloud.get_iterator();
	for( itr.init(); npts=itr.next_points( max_buffer_pts, pts ); )
	{
		float* shellPts = NULL;
		EXCEPTION_BEGIN
		EXCEPTION_TRY
		{
			fp_sentry fps;
			shellPts = ACIS_NEW float[3*npts];
			for( int ii=0; ii<npts; ++ii)
			{
				shellPts[3*ii]		=(float) pts[ii].x();
				shellPts[3*ii+1]	=(float) pts[ii].y();
				shellPts[3*ii+2]	=(float) pts[ii].z();
			}

			// insert all points into a shell with no polygons.
			HC_Insert_Shell( npts, shellPts, 0, NULL );
		}
		EXCEPTION_CATCH_TRUE
			ACIS_DELETE [] STD_CAST shellPts;
			shellPts=NULL;
		EXCEPTION_END
	}
	EXCEPTION_CATCH_TRUE 
	HC_Close_Segment();
	EXCEPTION_END
	return key;
}

HC_KEY HA_Render_PointCloud(SPApoint_cloud* inputCloud, const rgb_color& inColor)
{
	if (inputCloud == NULL) 
	{
		return 0;
	}

	char* pointCloudName = getPointCloudName( inputCloud, TRUE);
	if (pointCloudName == NULL)
	{
		return 0;
	}
	fp_sentry fps;
	//1. find if segment exists
	//2. If it exists, delete it
	//3. Recreate it
	HC_Begin_Segment_Search (pointCloudName);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();

	if (segFound)
	{
		HC_Delete_Segment( pointCloudName);
	}

	HC_KEY key = HC_KOpen_Segment( pointCloudName);

	//Identity matrix is the default transform.
	float modelling_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	const SPAtransf* transform = inputCloud->get_transform();
	if (transform != NULL) //Convert ACIS transform to hoops-compatible matrix
	{
		transf_to_matrix(modelling_matrix, *transform);
	}

	HC_Set_Modelling_Matrix(modelling_matrix);

	HC_QSet_Color_By_Value(pointCloudName,"marker","rgb",inColor.red(),inColor.green(),inColor.blue());

	int numPoints = inputCloud->size();

	HAPoint *shellPts = ACIS_NEW HAPoint[numPoints];

	SPApoint_cloud_iterator pci( *inputCloud );
	logical didItWork = TRUE;
	for(int i = 0;((i<numPoints) && didItWork);i++)
	{
		SPAposition nextPos;
		logical didItWork = pci.next (nextPos);
		if (didItWork)
		{
			shellPts[i].x = (float)nextPos.x();
			shellPts[i].y = (float)nextPos.y();
			shellPts[i].z = (float)nextPos.z();
		}
	}

	//HC_Control_Update(pointCloudName,"compile only");
    //HC_Set_Rendering_Options("display lists = geometry");
	//HC_Set_Heuristics("static model=on");
	
	// Insert the shell and renumber its key to the pointcloud address.
	EXCEPTION_BEGIN
	EXCEPTION_TRY
	HC_KEY original_shell_key = HC_KInsert_Shell(numPoints,shellPts,0,NULL);
	IDC_ASSERT( (((intptr_t)inputCloud) & 0x01) == 0);
	HC_Renumber_Key( original_shell_key, (HC_KEY)(((uintptr_t)inputCloud)>>1), "local");
	// We shift the address by one bit to make sure we never add negative keys.
	// This is harmless because valid addresses will never have this bit set.
	

	EXCEPTION_CATCH_TRUE
		HC_Close_Segment();  // Current PointCloud segment.
		ACIS_FREE(pointCloudName);
		ACIS_DELETE[] STD_CAST shellPts;
	EXCEPTION_END
	HC_Update_Display();

	return key;
}

HC_BOOLEAN HA_Set_PointCloud_Color(SPApoint_cloud* inputCloud, rgb_color inColor)
{
	HC_BOOLEAN status = TRUE;
	char* pointCloudName = getPointCloudName( inputCloud);

	HC_Begin_Segment_Search (pointCloudName);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();

	if(!segFound)
		status = FALSE;

	if (status)
	{	
		HC_QSet_Color_By_Value( pointCloudName,"marker","RGB",inColor.red(),inColor.green(),inColor.blue());
		HC_Update_Display();
	}

	ACIS_FREE(pointCloudName);

	return status;
}

SPApoint_cloud* HA_Select_Point_Cloud(HC_KEY scene_key, HA_Point pick_point)
{
	fp_sentry fps;
	HC_Open_Segment_By_Key(scene_key);
	HC_Set_Selectability("markers");

	HC_Begin_Segment_Search (PC_PARENT_SEGEMENT);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();
	if (!segFound)
	{
		HC_Close_Segment();
		return NULL;
	}

	char ownerName[1024];
	HC_Show_Owner_By_Key(scene_key, ownerName);

	double x, y;
	HC_KEY key;
	SPApoint_cloud* key_address = NULL;

	// Convert mouse coords to hoops coordinates.
	HA_Point out(0,0,0);
	HC_Compute_Coordinates( ".", "local pixels", &pick_point, "local window", &out);
	x = out.x();
	y = out.y();

	int count = HC_Compute_Selection( ownerName, PC_PARENT_SEGEMENT, "v", x, y);
	if (count)
	{
		do {
			int o1, o2, o3;
			HC_Show_Selection_Element(&key, &o1, &o2, &o3);

			HC_KEY original_key;
			HC_Show_Selection_Original_Key( &original_key);

			char tempName[16];
			HC_Show_Key_Type( original_key, tempName);
			if ( strcmp( tempName, "shell") == 0 ) // Its a shell
			{
				// See bit shift comments where we renumber the shell key.
				key_address = (SPApoint_cloud*)(key<<1);
				break;
			}
		} while ( HC_Find_Related_Selection() ); 
	}
	HC_Close_Segment();
	
	return key_address;
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
int qsort_compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

sortable_integer_ls* get_position_indices( int* input_array, int size, SPApoint_cloud* pc)
{
	sortable_integer_ls* position_indices = NULL;

	position_indices = ACIS_NEW sortable_integer_ls();

	qsort(input_array, size, sizeof(int), qsort_compare);
	
	int input_index = 0;
	int list_index = 0; //The element of the list we're at, INCLUDING tombstones
	int real_index = 0; //The element of the list we're at, NOT including tombstones
	int new_value = 0;
	SPAposition temp_position;
	while(input_index < size && input_index < pc->size())
	{
		if (pc->get(list_index, temp_position, new_value))
		{
			if (input_array[input_index] == real_index)
			{
				position_indices->add(new_value);
				input_index++;
			}
			real_index++;
		}

		list_index++;
	}

	return position_indices;
}

SPApoint_cloud* HA_Select_Point_Cloud_Marker(HC_KEY scene_key, HA_Point pick_point)
{
	fp_sentry fps;
	HC_Open_Segment_By_Key(scene_key);
	HC_Set_Selectability("markers");

	HC_Begin_Segment_Search (PC_PARENT_SEGEMENT);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();
	if (!segFound)
	{
		HC_Close_Segment();
		return NULL;
	}

	char ownerName[1024];
	HC_Show_Owner_By_Key(scene_key, ownerName);

	double x, y;
	HC_KEY key;
	int index = -1;
	SPApoint_cloud* key_address = NULL;

	// Convert mouse coords to hoops coordinates.
	HA_Point out(0,0,0);
	HC_Compute_Coordinates( ".", "local pixels", &pick_point, "local window", &out);
	x = out.x();
	y = out.y();
	if ( x < -1 || x > 1 || y < -1 || y > 1 )
		return NULL;

	int count = HC_Compute_Selection( ownerName, PC_PARENT_SEGEMENT, "v", x, y);
	if (count)
	{
		do {
			int o1, o2, o3;
			HC_Show_Selection_Element(&key, &o1, &o2, &o3);

			HC_KEY original_key;
			HC_Show_Selection_Original_Key( &original_key);

			// Find the selection of a marker.
			char tempName[16];
			HC_Show_Key_Type( original_key, tempName);
			if ( !strcmp(tempName,"shell")) // Its a shell
			{
				// The index of the marker.
				index = o1;
				IDC_ASSERT( index >= 0);

				// See bit shift comments where we renumber the shell key.
				key_address = (SPApoint_cloud*)(key<<1);
				break;
			}
		} while ( HC_Find_Related_Selection() ); 
	}
	HC_Close_Segment();

	SPApoint_cloud* point_cloud = NULL;
	if (key_address)
	{
		int list[1] = { index };
		point_cloud = key_address->subset(get_position_indices( list, 1, key_address ) );
	}

	return point_cloud;

}

SPApoint_cloud* HA_Select_Point_Cloud_Markers(HC_KEY scene_key, HA_Point pick_point1, HA_Point pick_point2)
{
	fp_sentry fps;
	HC_Open_Segment_By_Key(scene_key);
	HC_Set_Selectability("everything = off, markers = on, vertices = on");
	HC_Set_Heuristics( "no related selection limit, no internal selection limit");

	HC_Begin_Segment_Search (PC_PARENT_SEGEMENT);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();
	if (!segFound)
	{
		HC_Close_Segment();
		return NULL;
	}

	char ownerName[1024];
	HC_Show_Owner_By_Key(scene_key, ownerName);

	// Convert mouse coords to hoops coordinates.
	double x1, y1, x2, y2;
	HC_KEY key;
	HA_Point out1(0,0,0);
	HA_Point out2(0,0,0);
	HC_Compute_Coordinates( ".", "local pixels", &pick_point1, "local window", &out1);
	HC_Compute_Coordinates( ".", "local pixels", &pick_point2, "local window", &out2);
	x1 = out1.x();
	y1 = out1.y();
	x2 = out2.x();
	y2 = out2.y();
	if ( x1 < -1 || x1 > 1 || y1 < -1 || y1 > 1 
		|| x2 < -1 || x2 > 1 || y2 < -1 || y2 > 1 )
		return NULL;

	// Convert window coordinates into lrbt
	double left = x1, right = x2, bottom = y1, top = y2;
	if (x1 > x2)
	{
		left = x2;
		right = x1;
	}
	if (y1 > y2)
	{
		bottom = y2;
		top = y1;
	}
	int* offsetarray = NULL;
	int numelements = 0;
	SPApoint_cloud* key_address = NULL;
	int count = HC_Compute_Selection_By_Area(ownerName, PC_PARENT_SEGEMENT, "v", left, right, bottom, top);
	if ( count )
	{
		HC_Show_Selection_Elements_Coun(&key, &numelements);
		offsetarray = ACIS_NEW int[numelements];
		HC_Show_Selection_Elements(&key, &numelements, offsetarray, NULL, NULL);
		HC_KEY original_key;
		HC_Show_Selection_Original_Key( &original_key);
		char tempName[16];
		HC_Show_Key_Type( original_key, tempName);

		if ( strcmp(tempName, "shell") == 0 )
		{
			HC_Flush_All_Events();
			// See bit shift comments where we renumber the shell key.
			key_address = (SPApoint_cloud*)(key<<1);
		}
	}
	HC_Close_Segment();

	SPApoint_cloud* point_cloud = NULL;
	if (key_address)
	{
		sortable_integer_ls* list = get_position_indices(offsetarray, numelements, key_address);
		point_cloud = key_address->subset(list);
	}

	ACIS_DELETE[] STD_CAST offsetarray; 

	return point_cloud;
}

/*HC_KEY HA_Render_PointCloud(SPApoint_cloud* inputCloud, const rgb_color& inColor)
{
	if (!inputCloud) 
		return 0;

	char* pointCloudName = getPointCloudName(inputCloud);

	if (pointCloudName == NULL)
		return 0;

	HC_Begin_Segment_Search (pointCloudName);
	HC_BOOLEAN segFound = HC_Find_Segment(NULL);
	HC_End_Segment_Search();

	if(segFound)
		HC_Delete_Segment(pointCloudName);

	
	HC_KEY key = HC_KOpen_Segment(pointCloudName);
	HC_QSet_Color_By_Value(pointCloudName,"marker","rgb",inColor.red(),inColor.green(),inColor.blue());

	ACIS_DELETE[] STD_CAST pointCloudName;

	HC_Renumber_Key(key,(long)inputCloud,"local"); // Renumber the key to be the input Cloud Pointer


	int max_indx = inputCloud->max_index();
	for ( int indx = 0; indx <= max_indx; indx++ )
	{
		SPAposition nextPos;
		int pos_list_indx = -1;
		if ( inputCloud->get( indx, nextPos, pos_list_indx ) )
		{
			
			HC_KEY markerKey = HC_KInsert_Marker( nextPos.x(),nextPos.y(), nextPos.z() );
			HC_Renumber_Key(markerKey,pos_list_indx,"local"); // Renumber the key to be the index of the point in the position list
		}
	}
	
	HC_Close_Segment();  // Current PointCloud segment.
	HC_Update_Display();

	return key;
}*/

HC_KEY HA_Render_Entity(ENTITY* entity, const char* pattern)
{
	if (!entity) 
		return 0;

	// Caller paused the rendering opration. So skip.
	option_header *pr_opt = find_option( "pr" );
	if( pr_opt && pr_opt->on() )
		return 0;

	if (is_ASM_ASSEMBLY(entity) || is_ASM_MODEL_REF(entity))
		return 0;

	if(!pattern || !*pattern) 
		pattern = get_ha_state()->s_RenderingOptions.GetPattern();

	HA_Map* map = 0;
	if (get_ha_state()->s_RenderingOptions.GetMappingFlag())
		map = get_ha_state()->s_pHA_Map;
	
	HC_KEY key=0;

	ENTITY *owner = NULL;
	api_get_owner( entity, owner );

	if (get_ha_state()->s_pIEntityConverter)
	{
		key=get_ha_state()->s_pIEntityConverter->ConvertEntity(owner,get_ha_state()->s_RenderingOptions,map,pattern);
	}

	if (get_ha_state()->s_RenderingOptions.GetMergeFacesMode() && !get_ha_state()->s_RenderingOptions.GetMergeBodiesMode())
	{
		// merge bodies can't be done on Wire bodies 
		if ( (entity->identity() == BODY_TYPE) && (is_wire_body(entity) == FALSE) )
			merge_body_faces((BODY**)&entity, 1, pattern);
	}

	if (get_ha_state()->s_RenderingOptions.GetMappingFlag())
		return get_ha_state()->s_pHA_Map->FindMapping(entity);

	return key;
}


void HA_Internal_Flush_Entity_Geometry(ENTITY* entity);

logical HA_Update_Entity(ENTITY* entity)  
{
	// This api right now only works for bodies and 'create body segments' is on
	HC_KEY body_key = get_ha_state()->s_pHA_Map->FindMapping( entity);
	bool no_segment = true;
	if( body_key)
	{
		char key_type[4096];
		HC_Show_Key_Type(body_key, key_type);
		if (strstr(key_type, "segment"))
			no_segment = false;
	}

	if( no_segment )
	{
		const char * mes = "Could not determine segment corresponding to given entity."
						   " Please ensure entity is 'body' type and 'create body segments' option is on";
		const char * mesv[1]; 
		const char * fun = "HA_Update_Entity";
		const char * funv[1];
		mesv[0]=mes;
		funv[0]=fun;

		HC_Report_Error(50,309,2,1,mesv,1,funv);
		return false;
	}

	// Delete the ENTITY data.  This will delete all the maps, including the top
	// level entity, but they will be put back in by Render Entity.
	HA_Internal_Flush_Entity_Geometry( entity);
	HA_Render_Entity(entity);

	return true;
}

HC_KEY HA_ReRender_Entity(ENTITY* entity, const char* pattern)
{
	HA_Delete_Entity_Geometry(entity);
	return HA_Render_Entity(entity, pattern);
}

#define MAX_FIXED_KEY_ARRAY 1000
void HA_Internal_Delete_Entity_Geometry(ENTITY* entity)
{
	// hh:combine causes FACEs that were toplevel to not be toplevel
	// but we still need to be able to delete the rendering data.
	// assert(is_toplevel(entity));

	long j;
	long num_keys = 0;
	long max_allocated_key_array_size = MAX_FIXED_KEY_ARRAY;
	HC_KEY fixed_size_key_array[MAX_FIXED_KEY_ARRAY];
	HC_KEY* dynamic_size_key_array = 0;
	HC_KEY* keys = 0;
	keys = fixed_size_key_array;

	num_keys = HA_Compute_Geometry_Key_Count(entity);

	if (num_keys <= 0)
		return;

	if (num_keys >= max_allocated_key_array_size) // the fixed_size array was not big enough
	{
		max_allocated_key_array_size = (num_keys);  // avoid excessive re-allocation later
	
		dynamic_size_key_array = ACIS_NEW HC_KEY[max_allocated_key_array_size];
		if (!dynamic_size_key_array)
		{
			assert(0);
			return;
		}
		keys = dynamic_size_key_array;
	}

	num_keys = HA_Compute_Geometry_Keys(entity, max_allocated_key_array_size, keys);

	for (j = 0; j < num_keys; j++)
	{
		HC_KEY key = keys[j];
		get_ha_state()->s_pHA_Map->DeleteMapping(key);
        // jkf Feb02 R20 Delete from ASM map as well 88512
		get_ha_state()->s_pHA_MapAsm->DeleteMapping(key);
		// Check if you have a face and you happen to delete the segment containing the mesh before
		// the mesh segement is nuked.
		if (valid_segment(key))
			HC_Delete_By_Key(key);
	}

	get_ha_state()->s_pHA_Map->DeleteMapping(entity);
    // jkf Feb02 R20 Delete from ASM map as well 88512
	get_ha_state()->s_pHA_MapAsm->DeleteMapping(entity);

	if (dynamic_size_key_array)
		ACIS_DELETE [] STD_CAST dynamic_size_key_array;
}

void HA_Internal_Flush_Entity_Geometry(ENTITY* entity)
{
	// hh:combine causes FACEs that were toplevel to not be toplevel
	// but we still need to be able to delete the rendering data.
	// assert(is_toplevel(entity));

	long j;
	long num_keys = 0;
	long max_allocated_key_array_size = MAX_FIXED_KEY_ARRAY;
	HC_KEY fixed_size_key_array[MAX_FIXED_KEY_ARRAY];
	HC_KEY* dynamic_size_key_array = 0;
	HC_KEY* keys = 0;

	keys = fixed_size_key_array;

	num_keys = HA_Compute_Geometry_Key_Count(entity);

	if (num_keys <= 0)
		return;

	if (num_keys >= max_allocated_key_array_size) // the fixed_size array was not big enough
	{
		max_allocated_key_array_size = (num_keys);  // avoid excessive re-allocation later
	
		dynamic_size_key_array = ACIS_NEW HC_KEY[max_allocated_key_array_size];
		if (!dynamic_size_key_array)
		{
			assert(0);
			return;
		}
		keys = dynamic_size_key_array;
	}

	num_keys = HA_Compute_Geometry_Keys(entity, max_allocated_key_array_size, keys);

	for (j = 0; j < num_keys; j++)
	{
		HC_KEY key = keys[j];

		// This routine is cleaning all the data, so we will kill all the maps.  If
		// this segment is being reused for an ENTITY then the maps will have to be
		// put back in. This is the easiest way to prevent duplicate maps.
		get_ha_state()->s_pHA_Map->DeleteMapping(key);
		get_ha_state()->s_pHA_MapAsm->DeleteMapping(key);

		// Clear the contents of the segment mapped by the key.
		if (valid_segment(key))
		{
			HC_Open_Segment_By_Key( key);
			HC_Flush_Contents("...", "everything, no subsegments");
			HC_Close_Segment();
		}
	}

	get_ha_state()->s_pHA_Map->DeleteMapping(entity);
	get_ha_state()->s_pHA_MapAsm->DeleteMapping(entity);

	if (dynamic_size_key_array)
		ACIS_DELETE [] STD_CAST dynamic_size_key_array;
}

void HA_Delete_Entity_Geometry(ENTITY_LIST const& entitylist)
{
	// there is not part and all that stuff here,
	// just call the internal deletion routene for each entity
	ENTITY* entity = 0;
	entitylist.init(); // reset next() function to give first element in "list"
	while ((entity = entitylist.next()) != 0)
	{
		HA_Delete_Entity_Geometry( entity );
	}
}

void HA_Delete_Entity_Geometry(ENTITY *entity)
{
	// knt 3 Aug 2006. BTS #82057. Added the check here to see if the
	// given entity is in the top map. The bug was that an EDGE was lost
	// and found itself in this routine. We called api_get_owner on the
	// given EDGE and it's owning COEDGE, too, was already lost. Trying
	// to ask for the COEDGE's owner caused a crash. My thinking behind the
	// design of my fix is as follows: originally, the EDGE in question was
	// a top-level entity - so it was put in the Top-Map. Now the EDGE
	// in question is no longer top-level, but we need to clear it from 
	// the Top-Map. So the point is this: if we know it is in the Top-Map,
	// then we don't need to ask for it's owner. From the perspective
	// of this routine, we just need to make sure we clean things up -
	// so we treat input entities that are in the Top-Map as if they are
	// top-level entities (i.e., set their top_level owner to themselves).
	// I had to put this logic in a handful of places related to the delete
	// logic.

//	ENTITY *owner = NULL;
//	if ( Is_In_Top_Map( entity ) )
//		owner = entity;
//	else
//		api_get_owner( entity, owner );
//
//	if ( s_pHA_Map->FindMapping( entity ) )
//		HA_Internal_Delete_Entity_Geometry( entity );
//
//	if ( ( owner != entity ) && s_pHA_Map->FindMapping( owner ) )
//		HA_Internal_Delete_Entity_Geometry( owner );


	// knt 2 Nov 2006. BTS #82145. Commented out all of the above logic and
	// replaced with single line below. The bug was that when using the
	// entity:erase command with a single face of a block, the entire block
	// was removed from the view. The reason was that above logic would find
	// the top-level ent of the input entity and call HA_Internal_Delete_Entity_Geometry
	// on the top-level ent - so of course the entire block was removed
	// from the view. To fix this we just call HA_Internal_Delete_Entity_Geometry
	// in the given input entity. With regard to the bug, that ensures that
	// only the face given to entity:erase will be removed from the view.

	HA_Internal_Delete_Entity_Geometry( entity );
}

logical face_has_no_render_attrib( FACE* face)
{
	SHELL * fs = (SHELL *)face->owner();
	LUMP * fl = ( fs != NULL ) ? (LUMP *)fs->owner() : NULL;
	BODY * fb = ( fl != NULL ) ? (BODY *)fl->owner() : NULL;
	logical face_has_norender_attrib = ( face ? find_NORENDER_ATTRIB( face ) != NULL : FALSE );
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
 * The skip_rendered_faces flag is a bit inelegant, but allows the progress metering to be
 * shared for model rendering and part rendering - model rendering skips already rendered faces,
 * part rendering doesn't. A possible future efficiency.
 */
HA_RENDER_progress_info * make_progress_meter( const ENTITY_LIST & entitylist, logical skip_rendered_faces )
{
	// Count the total # of faces - for rendering progress.
	int total_faces = 0;
	ENTITY * ent = NULL;
	entitylist.init();
	ENTITY_LIST faces, unrendered_faces;
	while ( ENTITY * ent = entitylist.next() )
	{
		if ( is_BODY(ent) || is_FACE(ent) )
			api_get_faces( ent, faces );
	}

	while ( ENTITY * face = faces.next() )
	{
		logical has_norender_attrib = face_has_no_render_attrib((FACE*)face );
		logical skip = FALSE;
		if ( skip_rendered_faces )
		{
			int nkeys = HA_Compute_Geometry_Key_Count( face );
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
int HA_Close_All_Open_Segments()
{
    int count = 0;
    HC_Begin_Open_Segment_Search();
        HC_Show_Open_Segment_Count(&count);
        for (int i = 0; i < count; i++)
            HC_Close_Segment();
    HC_End_Open_Segment_Search();
    return count;
}

SESSION_GLOBAL_VAR HA_RENDER_progress_info * progress_meter;
#include "af_serializable_mesh.hxx"
#include "af_api.hxx"
#include "thmgr.hxx"
void facet_unfaceted_bodies( ENTITY_LIST const& ents )
{
	if(faceter_allow_multithreading.on() && (thread_work_base::thread_count() > 0 ) )
	{
		ENTITY_LIST bodies_with_no_facets;
		{for( ENTITY* e=ents.first(); e; e=ents.next() )
		{
			ENTITY_LIST efaces;
			api_get_faces(e,efaces);

			logical has_facets=FALSE;
			{for( ENTITY* f=efaces.first(); f; f=efaces.next() )
			{
				if( GetSerializableMesh((FACE*)f) )
				{
					has_facets=TRUE;
					break;
				}
			}}
			if( !has_facets && is_BODY(e) )
			{
				bodies_with_no_facets.add(e);
			}
		}}

		facet_options* fo = HA_Get_Facet_Options();
		api_facet_bodies(bodies_with_no_facets,fo);
	}
}

extern option_header hoops_direct_render;
void HA_Render_Entities(ENTITY_LIST const& entitylist, const char* pattern)
{
	ENTITY* entity = 0;

	// *FIX need to handle case where user renders an individual face,
	// but they have merge_bodies mode ON.  Probably should forbid this (report error here)
	// *TODO determine method of error reporting for this type of error

	EXCEPTION_BEGIN

		progress_meter = NULL;

	EXCEPTION_TRY

		ha_rendering_options * ro = &get_ha_state()->s_RenderingOptions;
		if ( ro && ro->GetRenderFacesMode() )
		{
			progress_meter = make_progress_meter( entitylist, hoops_direct_render.on() );
		}

		facet_unfaceted_bodies( entitylist );

		entitylist.init(); // initialize next() to return first elt in "list"
		while ((entity = entitylist.next()) != 0)
		{
			HA_Render_Entity(entity, pattern);
		}

	if (get_ha_state()->s_RenderingOptions.GetMergeBodiesMode())
	{
		// need to create a pruned array which only contains body entities in it.
		BODY** bodies;
		unsigned long num_bodies = 0;
		bodies = ACIS_NEW BODY*[entitylist.iteration_count()];
		if (!bodies)
			return;

		entitylist.init(); // initialize next() to return first elt in "list"
		while ((entity = entitylist.next()) != 0)
		{
			if (is_BODY(entity))
				bodies[num_bodies++] = (BODY*)entity;
		}
		if (num_bodies > 0)
			merge_body_faces(bodies, num_bodies, pattern);
		if (bodies)
			ACIS_DELETE [] STD_CAST bodies;
	}

	EXCEPTION_CATCH_TRUE
	
		if ( progress_meter )
		{
			ACIS_DELETE progress_meter;
			progress_meter = NULL;
		}
		if (error_no != 0)
			HA_Close_All_Open_Segments();

	EXCEPTION_END

}

void HA_ReRender_Entities(ENTITY_LIST const& entitylist, const char* pattern)
{
	ENTITY* entity = 0;
	entitylist.init(); // initialize next() to return first elt in "list"
	while ((entity = entitylist.next()) != 0)
	{
		HA_ReRender_Entity(entity, pattern);
	}
}

void HA_Show_Rendering_Options(char * list)
{
	get_ha_state()->s_RenderingOptions.Show_Rendering_Options(list);
}

void HA_Show_One_Rendering_Option(const char * type, char * value)
{
	get_ha_state()->s_RenderingOptions.Show_One_Rendering_Option(type, value);
}

HC_KEY HA_Render_Curve(
    CURVE *curve_entity,
    SPAparameter start_param,
    SPAparameter end_param,
	const SPAtransf* transform
    )
{
	if (!curve_entity)
		return 0;

	fp_sentry fps;
	double tolerance=HA_World_Coords_Per_Pixel();
	static option_header * render_curve_factor = 0;
	static int render_curve_factor_requested = FALSE;
	if ( render_curve_factor == 0 && ! render_curve_factor_requested )
	{
		render_curve_factor = find_option("render_curve_factor");
		render_curve_factor_requested = TRUE;
	}
	if ( render_curve_factor )
		tolerance /= render_curve_factor->count();

	int num_points = 0;
	AF_POINT* first_point = 0;
	tolerance /= transform->scaling();
	if (tolerance<SPAresabs)
		tolerance = SPAresabs*10;
	outcome o = api_facet_curve(curve_entity->equation(), start_param, end_param, tolerance, 0.0, 0.0,
		num_points, first_point);

	assert(o.ok());

	HC_KEY key = -1;
	float *pHoopsPoints = 0;
	if (num_points > 0)
		pHoopsPoints = ACIS_NEW float[3*num_points];

	if (pHoopsPoints)
	{
		AF_POINT* curr = first_point;
		int i;
		for (i = 0; i < num_points; i++)
		{
			SPAposition pos = curr->get_position() * transform;
			pHoopsPoints[3*i+0] = (float)pos.x();
			pHoopsPoints[3*i+1] = (float)pos.y();
			pHoopsPoints[3*i+2] = (float)pos.z();
			curr = curr->next(0);
		}
	}
	api_delete_all_AF_POINTs(first_point);
	if (pHoopsPoints)
	{
		key = HC_KInsert_Polyline(num_points, pHoopsPoints);
		ACIS_DELETE [] STD_CAST pHoopsPoints;pHoopsPoints=0;
	}

	return key;
}

#define HIGHLIGHT_SEGMENT "ACIShighlightSegment"
void HA_Highlight_Entity(ENTITY* e, logical on, const rgb_color& color)
{
	HC_KEY keys[1000];
	int count = HA_Compute_Geometry_Keys(e, 1000, keys);
	for(int i = 0; i<count; i++) 
	{
		HC_KEY key = keys[i];
		
		HC_KEY seg_key = HC_KShow_Owner_By_Key(key);
		if(on)
		{
			HC_Open_Segment_By_Key(seg_key);
				HC_Open_Segment(HIGHLIGHT_SEGMENT);   
					HC_Set_User_Index(0, (void *)seg_key);
					
					HC_Set_Color_By_Value("geometry", "RGB", color.red(), color.green(), color.blue());
					HC_Set_Line_Weight(3.0);
					// vertex highlight
					HC_Set_Visibility("faces, lines = on, markers = off");
					if(is_VERTEX(e) || is_APOINT(e))
					{
						HC_Set_Marker_Size(1.25);
						HC_Set_Visibility("markers = on");
					}

					HC_Move_By_Key(key, ".");
				HC_Close_Segment();
			HC_Close_Segment();	
		}
		else
		{
			HC_KEY orig_key;
			HC_Open_Segment_By_Key(seg_key);
			HC_Show_One_Net_User_Index(0, &orig_key);
			HC_Close_Segment();
			
			if(orig_key)
			{
				HC_Open_Segment_By_Key(orig_key);
				HC_Move_By_Key(key, ".");
				HC_Close_Segment();
				// TODO -- Should we now delete the "highlight" segment?
			}
		}
	}
}

logical HA_Show_Entity_Highlight(ENTITY* e)
{
	// An entity is highlighted if all of its parts are in a HIGHLIGHT_SEGMENT
	// It is not good enough to have only some parts highlighted.  For example
	// one FACE of a body.
	HC_KEY keys[1000];
	int count = HA_Compute_Geometry_Keys(e, 1000, keys, "any");

	for(int i = 0; i<count; i++) 
	{
		HC_KEY key = keys[i];
		
		char keytype[120];
		HC_Show_Key_Type(key, keytype);

		if(keytype[0] != 's' || keytype[1] != 'e') // keytype is not s e gment
		{
			char owner[1024];	
			HC_Show_Owner_By_Key(key, owner);
			logical hilite = FALSE;
			char token[1024];
			int  i = -1;

			while ( HC_Parse_String(owner, "/", i, token) )
			{
				if (STRICMP(token, HIGHLIGHT_SEGMENT) == 0)
				{
					hilite = TRUE;
					break;
				}

				i--;
			}

			if (!hilite)
			{
				// If we could not parse the string, or the segment did not have the special
				// name, this piece of the entity is not highlighted.
				return FALSE;
			}
		}
	}

	return TRUE;
}

IEntityConverter *HA_Get_Entity_Converter()
{
	return get_ha_state()->s_pIEntityConverter;
}

void HA_Set_Entity_Converter(IEntityConverter *icvrt)
{
	get_ha_state()->s_pIEntityConverter=icvrt;
}

logical HA_Read_Sat_File(const char *fname, ENTITY_LIST& entitylist)
{
	FILE * fp = NULL;
	ENTITY* entity = 0;
	outcome oc;
	logical res = TRUE;

	fp = fopen(fname, "r");
	res = (fp != NULL);

	if (res)
	{
		oc = api_restore_entity_list(fp, TRUE, entitylist);
		res = oc.ok();
	}

	if (fp)
		fclose(fp);

	if (res)
	{
		entitylist.init(); // initialize next() to return first elt in "list"
		while ((entity = entitylist.next()) != 0)
			HA_Render_Entity(entity);
	}

	return res;
}

logical HA_Write_Sat_File(const char *fname, ENTITY_LIST const& entitylist)
{
	FILE * fp = NULL;
	outcome oc;
	logical res = TRUE;

	fp = fopen(fname, "w");
	res = (fp != NULL);

	// ACIS 6.3 requires this thing for saving files
	char id_string[128];
	sprintf(id_string, "HOOPS-ACIS Part Viewer");

	if (res)
	{
		FileInfo info;
		info.set_product_id(id_string);
		info.set_units(1.0);
		oc = api_set_file_info((FileIdent | FileUnits), info);
		info.reset();
		res = oc.ok();
	}

	if (res)
		oc = api_save_entity_list(fp, TRUE, entitylist);

	if (fp)
		fclose(fp);
	
	return res;
}

logical HA_Read_Sab_File(const char *fname, ENTITY_LIST& entitylist)
{
	FILE * fp = NULL;
	ENTITY* entity = 0;
	outcome oc;
	logical res = TRUE;

	char * flag = "r";
#if defined( NT ) || defined( mac )
	flag = "rb";
#endif
	fp = fopen( fname, flag );
	res = (fp != NULL);

	if (res)
	{
		oc = api_restore_entity_list(fp, FALSE, entitylist);
		res = oc.ok();
	}

	if (fp)
		fclose(fp);

	if (res)
	{
		entitylist.init(); // initialize next() to return first elt in "list"
		while ((entity = entitylist.next()) != 0)
			HA_Render_Entity(entity);
	}

	return res;
}

logical HA_Write_Sab_File( const char *fname, ENTITY_LIST const & entitylist )
{
	FILE * fp = NULL;
	outcome oc;
	logical res = TRUE;

	char * flag = "w";
#if defined( NT ) || defined( mac )
	flag = "wb";
#endif
	fp = fopen( fname, flag );
	res = (fp != NULL);

	// ACIS 6.3 requires this thing for saving files
	char id_string[128];
	sprintf(id_string, "HOOPS-ACIS Part Viewer");

	if (res)
	{
		FileInfo info;
		info.set_product_id(id_string);
		info.set_units(1.0);
		oc = api_set_file_info((FileIdent | FileUnits), info);
		info.reset();
		res = oc.ok();
	}

	if (res)
		oc = api_save_entity_list(fp, FALSE, entitylist);

	if (fp)
		fclose(fp);
	
	return res;
}

logical HA_Write_Sat_File_2(const char *fname, ENTITY_LIST const& entitylist, float version) 
{
	// get the current save version
	int cur_major_ver;
	int cur_minor_ver;
	
	outcome oc = api_get_save_version( cur_major_ver, cur_minor_ver );
	logical res = oc.ok();
	
	// extract the major and minor versions from the user specified version 
	// e.g. if version is v.w then v is major and w is minor
	if (res)
	{
		assert( version > (double)0.0 );
		int user_major_ver = (int) floor( (double)version );
		int user_minor_ver = (int) (((double)((double)version - (double)user_major_ver)) * (double)10.0);

		// set this as current version for save
		oc = api_save_version( user_major_ver, user_minor_ver );
		res = oc.ok();
	}

	// call the vanilla write sat api
	if (res)
		res = HA_Write_Sat_File( fname, entitylist );

	// reset the save version
	if (res)
	{
		oc = api_save_version( cur_major_ver, cur_minor_ver );
		res = oc.ok();
	}

 	return res;
}

HC_KEY HA_KINDEXED_MESH_to_HOOPS(INDEXED_MESH *idx_mesh, FACE * /*face*/)
{
	if (!idx_mesh)
		return 0;

	HC_KEY key = 0;

	fp_sentry fps;
	// First find the number of polys.
	int num_polys=idx_mesh->get_num_polygon();
	// Get the number of vertices off all the polys.
	int nnode = idx_mesh->get_num_vertex();
	int num_polynodes=idx_mesh->get_num_polynode();

	EXCEPTION_BEGIN
	float *points=0;
	int *pFacetConnectivity=0;
	EXCEPTION_TRY
	// Get the verts array.
	points=ACIS_NEW float[nnode*3];
	// Build the conectivity array.
	pFacetConnectivity=ACIS_NEW int[num_polys+num_polynodes];

	// Now fill in the conectivity array.
	int polyIdx = 0;
	int current_array_index=0;
	while(polyIdx < num_polys)
	{
		// grab the polygon and find out how many vertices it has.
		indexed_polygon *poly = idx_mesh->get_polygon( polyIdx++ );
		int numVertices = poly->num_vertex();
		
		pFacetConnectivity[current_array_index++]=numVertices;
		for(int i=0; i<numVertices; i++) 
		{
			polygon_vertex* vert = poly->get_vertex(i);
			int index = idx_mesh->get_vertex_index(vert);
			pFacetConnectivity[current_array_index++]=index;
		}
	}

	for(int j=0; j<nnode; j++) 
	{
		SPAposition pos = idx_mesh->get_position(j);

		points[j*3] =	(float)pos.x();
		points[j*3+1] = (float)pos.y();
		points[j*3+2] = (float)pos.z();
	}

	// Create the HOOPS rep.
	if (num_polys > 0)
	{
		key = HC_KInsert_Shell (nnode, points, current_array_index, pFacetConnectivity);

		if (key == 0 || key == -1)
			return 0;

		float uv[3];

		// Set the normals.
		HC_Open_Geometry(key);
			for (int i = 0; i < nnode; i++)
			{
				SPAunit_vector normal=idx_mesh->get_normal(i);
				HC_Open_Vertex(i);
					HC_Set_Normal(normal.x(),normal.y(),normal.z());
					// Set the UV
					const SPApar_pos &pp=idx_mesh->get_uv_as_scaled(i);
					uv[0]=(float)pp.u;
					uv[1]=(float)pp.v;
					uv[2]=0.0;
					HC_Set_Parameter( 3, uv);
				HC_Close_Vertex();
			}
		HC_Close_Geometry();
	}

    EXCEPTION_CATCH_TRUE
	ACIS_DELETE [] STD_CAST points;points=0;
	ACIS_DELETE [] STD_CAST pFacetConnectivity;pFacetConnectivity=0;
	EXCEPTION_END

	return key;
}

#include "surdef.hxx"
#include <vector>
#include <algorithm>

typedef struct {
	float red, green, blue;
} HOT_COLD_COLOR;
HOT_COLD_COLOR get_hot_cold_color( float value, float min_value, float max_value )
{
	HOT_COLD_COLOR color = {1.0, 1.0, 1.0}; // white
	if ( value < min_value )
		value = min_value;
	if ( value > max_value )
		value = max_value;
	float range = max_value - min_value;
	if ( value < ( min_value + 0.25 * range ) )
	{
		color.red = 0;
		color.green = 4 * ( value - min_value ) / range;
	}
	else if ( value < ( min_value + 0.50 * range ) )
	{
		color.red = 0;
		color.blue = (float)( 1 + 4 * ( min_value + 0.25 * range - value ) / range );
	}
	else if ( value < ( min_value + 0.75 * range ) )
	{
		color.red = (float)( 4 * ( value - min_value - 0.5 * range ) / range );
		color.blue = 0;
	}
	else
	{
		color.green = (float)( 1 + 4 * ( min_value + 0.75 * range - value ) / range );
		color.blue = 0;
	}
	return color;
}

typedef double (*HighlightWeightFn)( FACE *face, float pos3d[3], float pos_uv[2]);

DECL_HOOPS HighlightWeightFn wt_function = NULL;
bool get_colors_for_FACE( af_serializable_mesh *mesh, FACE * face, float *points, float* texture_uvs, int vertexCount, float *colors )
{
	std::vector<double> weights;
	int i=0;
	double min_elem = DBL_MAX;
	double max_elem = DBL_MIN;
	for ( i = 0; i < vertexCount; i++ )
	{
		double xxx = wt_function( face, &points[3 * i], &texture_uvs[2 * 1] );
		weights.push_back( xxx );
		if ( xxx < min_elem ) min_elem = xxx;
		if ( xxx > max_elem ) max_elem = xxx;
	}

	//min_elem = *min_element( weights.begin(), weights.end() );
	//max_elem = *max_element( weights.begin(), weights.end() );

	double range = max_elem - min_elem;
	if (range < SPAresabs)
		range = SPAresabs;
	for (i=0; i<vertexCount; i++)
	{
		float r = 0;
		r = (float)(weights[i]-min_elem) / (float)range;

		colors[3*i+2] = 0.0f;
		HOT_COLD_COLOR colour = get_hot_cold_color( (float)(weights[i]), (float)min_elem, (float)max_elem );
		colors[3 * i + 0] = colour.red;
		colors[3 * i + 1] = colour.green;
		colors[3 * i + 2] = colour.blue;

/*		if ( r <= 0.5 )
		{
			colors[3*i] = 2*r;
			colors[3*i+1] = 1;
		}
		else
		{
			colors[3*i] = r;
			colors[3*i+1] = 2*(1-r);
		}

		{
			colors[3*i] = r;
			colors[3*i+1] = 1-r;
			colors[3*i+2] = 0.0f;
		}*/
	}
	return true;
}


#include "af_serializable_mesh.hxx"
#ifndef _MSC_VER
HC_KEY HA_K_SEQUENTIAL_MESH_to_HOOPS(af_serializable_mesh *mesh, FACE * face)
{
	if (!mesh)
		return 0;

	HC_KEY key = 0;

	fp_sentry fps;
	// First find the number of polys.
	int polygonCount = mesh->number_of_polygons();
	// Get the number of vertices off all the polys.
	int vertexCount = mesh->number_of_vertices();
	//int polynodeCount = mesh->get_num_polynode();
	// Guy. Fixes 89417. 
	if ( polygonCount < 1 || vertexCount < 3 )
		return 0;

	EXCEPTION_BEGIN
	float *points = 0;
	int *pFacetConnectivity = 0;
	double* normals=0;
	float* texture_uvs=0;
	EXCEPTION_TRY
	points = ACIS_NEW float[vertexCount*3];
	mesh->serialize_positions(points);
	int polynodeCount = mesh->number_of_polygon_coedges();

	// Build the conectivity array.
	pFacetConnectivity = ACIS_NEW int[polygonCount + polynodeCount];
	mesh->serialize_polygons_for_hoops(pFacetConnectivity);
	// Create the HOOPS rep.
	if (polygonCount > 0)
	{
		{
			// kev. Added fp-sentry. Hoops was causing fp-exceptions.
			fp_sentry fps;
			key = HC_KInsert_Shell (vertexCount, points, polygonCount + polynodeCount, pFacetConnectivity);
		}

		if (key == 0 || key == -1)
			return 0;

		normals = ACIS_NEW double[3*vertexCount];
		mesh->serialize_normals(normals);
		texture_uvs = ACIS_NEW float[2*vertexCount];
		mesh->serialize_uv_data(texture_uvs, TRUE );
		// Set the normals.
		HC_Open_Geometry(key);
		int i = 0;
			{
				float *colors = ACIS_NEW float[3*vertexCount];
				bool use_colors = (NULL == wt_function) ? false :
									(face ? get_colors_for_FACE( mesh, face, points, texture_uvs, vertexCount, colors ) : false);

				for (i = 0; i < vertexCount; i++)
				{
					float uv[3];
					uv[0] = texture_uvs[2*i];
					uv[1] = texture_uvs[2*i+1];
					uv[2]=0.0;
					// todo: may need to readd parameter settings here for texture mapping folks.
					HC_Open_Vertex(i);
						HC_Set_Normal(normals[3*i],normals[3*i+1],normals[3*i+2]);
						HC_Set_Parameter(2, uv);
						if ( use_colors )
							HC_Set_Color_By_Value("geometry", "RGB", colors[3*i],colors[3*i+1],colors[3*i+2]);
					HC_Close_Vertex();
				}
				ACIS_DELETE [] STD_CAST colors;
			}
		HC_Close_Geometry();
	}

    EXCEPTION_CATCH_TRUE
	ACIS_DELETE [] STD_CAST normals; normals=0;
	ACIS_DELETE [] STD_CAST points;points=0;
	ACIS_DELETE [] STD_CAST pFacetConnectivity;pFacetConnectivity=0;
	ACIS_DELETE [] STD_CAST texture_uvs;texture_uvs=0;
	EXCEPTION_END

	return key;
}
#else
#include <vector>
HC_KEY HA_K_SEQUENTIAL_MESH_to_HOOPS(af_serializable_mesh *mesh, FACE * face)
{
	if (!mesh)
		return 0;

	fp_sentry fps;
	const int polygonCount = mesh->number_of_polygons();
	const int vertexCount = mesh->number_of_vertices();
	const int polynodeCount = mesh->number_of_polygon_coedges();
	if ( polygonCount < 1 ) // Guy. Crashes when a view:gl window is opened before running the 88911 script
		return 0;
	std::vector<int> connectivity;
	connectivity.resize(polygonCount+polynodeCount);
	mesh->serialize_polygons_for_hoops(&connectivity[0]);

	std::vector<float> coords;
	coords.resize(3*vertexCount);
	mesh->serialize_positions(&(coords[0]));
	HC_KEY key = HC_KInsert_Shell (vertexCount, &coords[0], polygonCount + polynodeCount, &connectivity[0]);
	connectivity.clear();

	if( mesh->has_normals() )
	{
		std::vector<float> ncoords;
		ncoords.resize(3*vertexCount);
		mesh->serialize_normals(&ncoords[0]);
		HC_MSet_Vertex_Normals(key,0,vertexCount,&ncoords[0]);
	}
	if( mesh->has_uv() )
	{
		std::vector<float> uvs;
		uvs.resize(2*vertexCount);
		mesh->serialize_uv_data(&uvs[0],TRUE);
		HC_MSet_Vertex_Parameters(key,0,vertexCount,2,&uvs[0]);
		if( wt_function )
		{
			std::vector<float> colorsPerVertex;
			colorsPerVertex.resize( 3*vertexCount );
			get_colors_for_FACE( mesh, face, &coords[0], &uvs[0], vertexCount, &colorsPerVertex[0] );
			HC_MSet_Vertex_Colors_By_Value(key,"face",0,"rgb",vertexCount,&colorsPerVertex[0]);
		}
	}
	return key;
}
#endif
void HA_Set_Current_View_Segment(HC_KEY key)
{
	get_ha_state()->s_Current_View_Key=key;
}

HC_KEY HA_Get_Current_View_Segment()
{
	return get_ha_state()->s_Current_View_Key;
}

double HA_World_Coords_Per_Pixel()
{
	HC_KEY key=HA_Get_Current_View_Segment();
	if (!key)
		return -1;

	HC_Open_Segment_By_Key(key);

		fp_sentry fps;
		float pt1[3] = {0.0f,0.0f,0.0f};  // World SPAposition.
		float pt2[3] = {0.0f,0.0f,0.0f};  // World SPAposition.
		float in_pt[3];      // Screen SPAposition.
		logical success;
		in_pt[0]=0.0;
		in_pt[1]=0.0;
		in_pt[2]=0.0;
		success = HC_Compute_Coordinates(".", "local pixels", in_pt, "world", pt1);

		if (success)
		{
			in_pt[0]=1.0;
			success = HC_Compute_Coordinates(".", "local pixels", in_pt, "world", pt2);
		}

	HC_Close_Segment();

	if (!success)
		return 0.0;

	SPAposition p1(pt1[0],pt1[1],pt1[2]);
	SPAposition p2(pt2[0],pt2[1],pt2[2]);
	double dist=distance_to_point(p1,p2);

	return dist;
}

logical HA_Associate_Key_To_Entity(ENTITY* entity, HC_KEY key) 
{
	assert(entity);
	HC_KEY keys[1024];
	int count = get_ha_state()->s_pHA_Map->FindMapping(entity,keys,1024);
	logical mapping_exists = FALSE;
	for (int i=0;i < count; i++)
	{
		if (keys[i] == key)
		{
			mapping_exists = TRUE;
			break;
		}
	}
	if (!mapping_exists)
	{
		get_ha_state()->s_pHA_Map->AddMapping(key, entity);
		return TRUE;
	}
	return TRUE;
}

logical HA_Disassociate_Key_From_Entity(ENTITY* entity, HC_KEY key) 
{
	if (!entity)
	{
		entity = get_ha_state()->s_pHA_Map->FindMapping(key);
	}

	if (entity)
	{
		get_ha_state()->s_pHA_Map->DeleteMapping(key, entity);
		return TRUE;
	}

	return FALSE;
}

// Return TRUE if anything was done
logical HA_ReRender_Body_Transforms(ENTITY_LIST& bodies)
{
	logical answer(FALSE);

	ENTITY* p_entity = bodies.first();
    while ( NULL != p_entity )
    {
        if ( is_BODY(p_entity) )
        {
            // Get the body transform
            float matrix[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
            BODY* p_body = (BODY*)p_entity;
            if ( p_body->transform() )
            {
				transf_to_matrix(matrix, p_body->transform()->transform());
            }

            // Update the modelling transform for the top level segments
            unsigned long keys_size = 1024;
            HC_KEY keys[1024];
            int count = HA_Compute_Geometry_Keys(p_entity, keys_size, keys);
            for ( int i=0; i<count; i++ )
            {
                // Update the modelling transform for the top level segments
                HC_Open_Segment_By_Key (keys[i]);
                HC_Set_Modelling_Matrix(matrix);
                HC_Close_Segment();
				answer = TRUE;
            }
        }
        p_entity = bodies.next();
    }

	return answer;
}

int Get_Map_Entries()
{
	int answer = get_ha_state()->s_pHA_Map->HA_Internal_Get_Map_Entries();
//	answer +=  get_ha_state()->s_pHA_MapAsm->HA_Internal_Get_Map_Entries();
	return answer;
}

logical Is_In_Top_Map( ENTITY* ent )
{
	return get_ha_state()->s_pHA_Map->HA_Is_In_Top_Map( ent );
}


#include "facet_body.hxx"

void opengl_face_list_to_hoops_format(int ntri, const int* const connectivity, std::vector<int>& face_list)
{
	face_list.clear();
	face_list.reserve(4 * ntri);
	for (int ii = 0; ii < ntri; ++ii)
	{
		face_list.push_back(3);
		face_list.push_back(connectivity[3 * ii]);
		face_list.push_back(connectivity[3 * ii+1]);
		face_list.push_back(connectivity[3 * ii+2]);
	}
}

void convert_double_array_to_floats(int const npt, double const* const pt_coords, std::vector<float>& pt_coords_flt)
{
	pt_coords_flt.clear();
	pt_coords_flt.reserve(3 * npt);
	for (int ii = 0; ii < 3 * npt; ++ii)
	{
		pt_coords_flt.push_back(static_cast<float>(pt_coords[ii]));
	}
}

#if 1
void HA_Render_FACET_BODY(FACET_BODY* pb, logical renderFaces, logical renderEdges)
{
	if (NULL == pb)
	{
		return;
	}

	if (renderFaces)
	{
		int const nf = pb->num_faces();
		{for (int ii = 0; ii < nf; ++ii)
		{
			Spa_raw_mesh const& rm = pb->get_face_mesh(ii);
			std::vector<int> face_list;
			std::vector<float> coords;
			convert_double_array_to_floats(rm.num_vertices(), rm.vertex_coordinates(), coords);

			int const * const con = rm.triangle_connectivity();
			int const ntri = rm.num_triangles();
			opengl_face_list_to_hoops_format(ntri, con, face_list);
			HC_Set_Visibility("markers=off");
			HC_Insert_Shell(rm.num_vertices(), &(coords[0]), static_cast<int>(face_list.size()), &face_list[0]);
		}}
	}
}
#endif

#ifdef INTERNAL_DEBUG_CHECKS

//#include "ut.hxx"
//#include "cstrapi.hxx"
//
//LOCAL_VAR int num_callbacks;
//
//LOCAL_PROC int my_cbfn( SPA_progress_info * )
//{
//	num_callbacks++;
//	return 0;
//}
//
//UT_BEGIN(rndr_prog_cbk)
//{
//	EXCEPTION_BEGIN
//
//		num_callbacks = 0;
//		SPA_progress_callback orig_cbfn = get_progress_callback( SPA_progress_info_HA_RENDER );
//		ha_rendering_options orig_ro( HA_Get_Rendering_Options() );
//		BODY * block = NULL;
//
//	EXCEPTION_TRY
//
//		set_progress_callback( my_cbfn, SPA_progress_info_HA_RENDER );
//
//		ha_rendering_options & ro = HA_Get_Rendering_Options();
//		ro.SetRenderFacesMode( TRUE );
//
//		check_outcome( api_solid_block( SPAposition(0,0,0), SPAposition(1,1,1), block ) );
//
//		ENTITY_LIST list;
//		list.add( block );
//		HA_Render_Entities( list );
//
//		// Expect 1 callback for start and 1 for finish, plus 6 more for each face == 8.
//		UT_ASSERT_MESSAGE( "Wrong number of render callbacks", num_callbacks == 8 );
//
//	EXCEPTION_CATCH_TRUE
//
//		set_progress_callback( orig_cbfn, SPA_progress_info_HA_RENDER );
//		HA_Set_Rendering_Options( orig_ro );
//		HA_Delete_Entity_Geometry( block );
//		check_outcome( api_del_entity( block ) );
//
//	EXCEPTION_END
//}
//UT_END

#endif
