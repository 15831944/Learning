/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: hps_util.h,v 1.4 2002/08/09 17:36:18 jeff Exp $
#ifndef __HPS_UTIL_HXX__
#define __HPS_UTIL_HXX__

#include <float.h>

#include "dcl_hps.h"
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "base.hxx"
#include "logical.h"
#include "entity.hxx"
#include "position.hxx"
#include "hps_point.h"
#include "fpsentry.hxx"

#ifdef NT
#include <windows.h>
#endif

class rgb_color;
class SPAtransf;

#ifdef NT
#define LStringize( s )			L#s
#else
#define LStringize( s )			#s
#endif
#define Stringize( s )			#s

#define MakeString( M, L )		M(L)
#define SPAHALine					\
	MakeString( Stringize, __LINE__ )
#define SPAHAReminder				\
	__FILE__ "(" SPAHALineMacro ") : "
// Example:
// #pragma message (SPAHAReminder "TODO: ")

/* note that the string colorName must have room for 10 + 1(\0) characters */
DECL_HPS void				MapColorToString( const rgb_color &in_color, char* out_colorName );
DECL_HPS HPS::SegmentKey	HPS_OpenColorSegment( ENTITY* in_entity, HPS::SegmentKey in_segment, logical in_traverse_up = FALSE );
//DECL_HPS void				OpenColorSegment( const rgb_color &color );
DECL_HPS HPS::SegmentKey	HPS_KOpenPointerSegment( void *in_ptr, HPS::SegmentKey in_segment_key );
DECL_HPS void				transf_to_matrix( float *out_float_mat, const SPAtransf &in_tform );
DECL_HPS void				ConvertSPAPositionToFloatArray( int in_num_pts, SPAposition *in_pos, float *&out_float_array );
DECL_HPS void				ConvertDoubleToFloatArray( int in_count, double *in_double_array, float *&out_float_array );

// The next two functions will be used to convert between pointers and strings used for 
// segment names.  The intent is to be portable across all variations of 64/32 bit pointers, longs
// uintptr_t is the ansi standard for an int large enough to hold an unsigned pointer

// Convert pointer to ascii in 0x format -- buffer should be at least POINTER_BUFFER_SIZE bytes
// return points to the 0 of 0x, not necessarily the start of the buffer.
#define POINTER_BUFFER_SIZE ((sizeof(uintptr_t) * 2) + 3)
DECL_HPS char*			ptoax( void* in_ptr, char* out_buffer );
// Convert ascii in 0x format to a pointer
DECL_HPS void*			axtop( char* in_buffer );

DECL_HPS HPS_BOOLEAN		HPS_Update_Display();
DECL_HPS HPS_BOOLEAN		HPS_Update_Display( HPS::View & in_view );

DECL_HPS logical			Parse_YesNo_And_Mutate_Options_Using_Bitmask( char* token, unsigned long bitmask, unsigned long* options );

#ifdef NT
// Loads a BMP
bool LoadBMP( HPS::ImageKit & imageKit, char *texture_name );
#endif // NT


#ifdef _DEBUG
#define HPS_CHECK_FOR_OPEN_SEGMENT

#else
#define HPS_CHECK_FOR_OPEN_SEGMENT
#endif //_DEBUG

enum hps_arrow_head_type
{
	hps_double_outside = -1,  // double arrows  -->->
	hps_simple = 0,  // a simple arrow ---->
	hps_double_inside = 1,  // double arrows  --<->
	hps_headless = 2,  // obviously this -----
	hps_conic = 3   // a "tee-pee" with 16 edges
};

// The drawing method in takes these values and divides them by 5 
// to get a scaling factor for the arrow head
enum hps_arrow_size_type
{
	hps_tiny = 1,
	hps_little = 3,
	hps_average = 5,
	hps_large = 10,
	hps_enormous = 25
};

// MakeArrow assumes there's an open segment to place the arrow into.
DECL_HPS void MakeArrow( SPAposition		const &	in_arrow_base,
						 SPAvector          const &	in_arrow_direction,
						 double             const	in_arrow_length,
						 hps_arrow_head_type const  in_arrow_type_o_head,
						 hps_arrow_size_type const  in_arrow_size_o_head,
						 HPS::SegmentKey			in_segment_key );

DECL_HPS void		ConvertSPAPositionToPointArray( int num_pts, SPAposition *pos, HPS::PointArray &PointArray, const SPAtransf* bodyTransf = 0 );
DECL_HPS void		ConvertFloatsToPointArray( int num_floats, float * pFloats, HPS::PointArray & PointArray, SPAtransf* bodyTransf = 0 );
DECL_HPS void		HPS_Debug( char const * comment, HPS::SegmentKey in_segment_key );

char* HPS_stristr( const char* in_string, const char* in_substring );

/**
* @nodoc
*/
DECL_HPS void HPS_Print_Segment_Tree( const char* in_base_seg, FILE * in_file_pointer = stdout );

/**
* @nodoc
*/
DECL_HPS void HPS_Print_Segment_Tree( HPS::SegmentKey in_key, FILE * in_file_pointer );


#endif // __HPS_UTIL_HXX__
