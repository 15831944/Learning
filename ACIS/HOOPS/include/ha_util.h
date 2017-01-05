/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: ha_util.h,v 1.4 2002/08/09 17:36:18 jeff Exp $
#ifndef __HA_UTIL_HXX__
#define __HA_UTIL_HXX__

#include <float.h>

#include "dcl_hoops.h"
#include "hc.h"
#include "base.hxx"
#include "logical.h"
#include "entity.hxx"
#include "position.hxx"
#include "ha_point.h"
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
DECL_HOOPS void MapColorToString(const rgb_color &color, char* colorName);
DECL_HOOPS logical OpenColorSegment(ENTITY* entity,logical traverse_up=FALSE);
DECL_HOOPS void OpenColorSegment(const rgb_color &color);
DECL_HOOPS HC_KEY KOpenPointerSegment(void *SPAptr);

DECL_HOOPS void transf_to_matrix(float *float_mat,const SPAtransf &tform);

DECL_HOOPS void ConvertSPAPositionToFloatArray(int num_pts, SPAposition *pos,float *&float_points);
DECL_HOOPS void ConvertDoubleToFloatArray(int num_pts, double *pos,float *&float_points);

// The next two functions will be used to convert between pointers and strings used for 
// segment names.  The intent is to be portable across all variations of 64/32 bit pointers, longs
// uintptr_t is the ansi standard for an int large enough to hold an unsigned pointer

// Convert pointer to ascii in 0x format -- buffer should be at least POINTER_BUFFER_SIZE bytes
// return points to the 0 of 0x, not necessarily the start of the buffer.
#define POINTER_BUFFER_SIZE ((sizeof(uintptr_t) * 2) + 3)
DECL_HOOPS char* ptoax(void* SPAptr, char* buffer);
// Convert ascii in 0x format to a pointer
DECL_HOOPS void* axtop(char* buffer);

DECL_HOOPS logical valid_segment(HC_KEY key);

DECL_HOOPS logical Parse_YesNo_And_Mutate_Options_Using_Bitmask(char* token, unsigned long bitmask, unsigned long* options);

#ifdef NT
// Loads a BMP
int LoadBMP(LPCTSTR lpszName, LPCTSTR hoops_name);
#endif // NT


#ifdef _DEBUG
#define HA_CHECK_FOR_OPEN_SEGMENT \
	HC_Begin_Open_Segment_Search();\
	{int count;\
	HC_Show_Open_Segment_Count(&count);\
	assert(!count);}\
	HC_End_Open_Segment_Search();\

#else
#define HA_CHECK_FOR_OPEN_SEGMENT
#endif //_DEBUG

enum ha_arrow_head_type
{
	ha_double_outside = -1,  // double arrows  -->->
	ha_simple         =  0,  // a simple arrow ---->
	ha_double_inside  =  1,  // double arrows  --<->
	ha_headless       =  2,  // obviously this -----
	ha_conic          =  3   // a "tee-pee" with 16 edges
};

// The drawing method in takes these values and divides them by 5 
// to get a scaling factor for the arrow head
enum ha_arrow_size_type
{
	ha_tiny       =   1,
	ha_little     =   3,
	ha_average    =   5,
	ha_large      =  10,
	ha_enormous   =  25
};

// MakeArrow assumes there's an open segment to place the arrow into.
DECL_HOOPS void MakeArrow(SPAposition const &arrow_base,
			   SPAvector          const &arrow_direction,
			   double             const  arrow_length,
			   ha_arrow_head_type const  arrow_type_o_head,
			   ha_arrow_size_type const  arrow_size_o_head);

#ifdef NT
#include <xstring>
DECL_HOOPS std::string HA_Show_Segment( HC_KEY in_key );
#endif // NT


#endif // __HA_UTIL_HXX__
