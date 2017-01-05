#ifndef HPS_ACIS_H_DEFINED
#define HPS_ACIS_H_DEFINED

#pragma warning(push)
#pragma warning(disable: 4100)
#include "hps.h"
#include "sprk.h"
#include "sprk_ops.h"
#pragma warning(pop)

#define HPS2016

#include <string>
#include "dcl_hps.h"
#include "b_strutl.hxx"
#include "position.hxx"
#include "unitvec.hxx"
#include "wndo_dt.hxx"

#define HPS_Version_String "HPS_2016_SP2"
extern DECL_HPS HPS::SegmentKey		HPS_Include_Library;
extern DECL_HPS HPS::SegmentKey		HPS_Style_Library;
extern DECL_HPS HPS::PortfolioKey	HPS_Portfolio;
extern DECL_HPS HPS::Key			HPS_INVALID_KEY;
extern DECL_HPS HPS::SegmentKey		HPS_INVALID_SEGMENT_KEY;

DECL_HPS HPS::Key			HPS_Invalid_Key();
DECL_HPS HPS::SegmentKey	HPS_Invalid_Segment_Key();
DECL_HPS	bool			HPS_Is_Valid_Key( HPS::Key in_key );
DECL_HPS	bool			HPS_Is_Valid_Segment_Key( HPS::SegmentKey in_key );
DECL_HPS	void			HPS_Delete_By_Key( HPS::Key in_key );

DECL_HPS HPS::SegmentKey	HPS_Open_Segment( const char * in_segment_name );
DECL_HPS HPS::SegmentKey	HPS_Open_Segment_Key_By_Key( HPS::SegmentKey in_parent_segment, char const *in_child_segment_name );
DECL_HPS HPS::Key			HPS_Include_Segment_Key_By_Key( HPS::SegmentKey in_included, HPS::SegmentKey in_includer );

#define HPS_KOpen_Segment	HPS_Open_Segment
DECL_HPS std::string		HPS_Show_Segment( HPS::SegmentKey in_key );
DECL_HPS HPS::UTF8			HPS_Cast_UTF8( HPS::Line::SizeUnits in_size_units );

inline SPAposition			HPS_Cast_SPAposition( HPS::Point& in_position ) { return SPAposition( in_position.x, in_position.y, in_position.z ); };
inline HPS::Point			HPS_Cast_Point( SPAposition& in_point ) { return HPS::Point( float( in_point.x() ), float( in_point.y() ), float( in_point.z() ) ); };
inline HPS::Point			HPS_Cast_Point( SPAvector& in_vector ) { return HPS::Point( float( in_vector.x() ), float( in_vector.y() ), float( in_vector.z() ) ); };

inline SPAunit_vector		HPS_Cast_SPAvector( HPS::Vector& in_vector ) { return SPAunit_vector( in_vector.x, in_vector.y, in_vector.z ); };
inline HPS::Vector			HPS_Cast_Vector( SPAunit_vector& in_vector ) { return HPS::Vector( float( in_vector.x() ), float( in_vector.y() ), float( in_vector.z() ) ); };
inline HPS::Vector			HPS_Cast_Vector( SPAvector& in_vector ) { return HPS::Vector( float( in_vector.x() ), float( in_vector.y() ), float( in_vector.z() ) ); };

inline bool strequal( const char * a, const char * b ) { if ( strcmp( ( a ), ( b ) ) == 0 ) return 1; else return false; }
inline bool striequal( const char * a, const char * b ) { if ( STRICMP( ( a ), ( b ) ) == 0 ) return 1; else return false; }
inline bool strnequal( const char * a, const char * b, size_t n ) { if ( strncmp( ( a ), ( b ), n ) == 0 ) return 1; else return false; }
inline bool strniequal( const char * a, const char * b, size_t n ) { if ( STRNICMP( ( a ), ( b ), n ) == 0 ) return 1; else return false; }

#ifdef NO_PROTOTYPES
#   define HPS_PROTO(a) ()
#else
#   define HPS_PROTO(a) a
#endif

#ifndef HPS_CDECL
#   ifdef _MSC_VER
#       define HPS_CDECL void
#   else
#       define HPS_CDECL
#   endif
#endif

#ifndef HPS_POINT
#	ifdef SWIG
typedef float HPS_POINT;
#	else
#   	define	HPS_POINT	void
/*
* NOTE: if you want a real definition for "HPS_POINT", put
*		typedef	struct {float x, y, z;}	Point;
*	        #define	HPS_POINT	Point
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_DPOINT
#	ifdef SWIG
typedef double HPS_DPOINT;
#	else
#   	define	HPS_DPOINT	void
/*
* NOTE: if you want a real definition for "HPS_DPOINT", put
*		typedef	struct {double x, y, z;}	DPoint;
*	        #define	HPS_DPOINT	DPoint
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_VECTOR
#	ifdef SWIG
typedef float HPS_VECTOR;
#	else
#   	define	HPS_VECTOR	void
/*
* NOTE: if you want a real definition for "HPS_VECTOR", put
*		typedef	struct {float x, y, z;}	Vector;
*	        #define	HPS_VECTOR	Vector
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_DVECTOR
#	ifdef SWIG
typedef double HPS_DVECTOR;
#	else
#   	define	HPS_DVECTOR	void
/*
* NOTE: if you want a real definition for "HPS_DVECTOR", put
*		typedef	struct {double x, y, z;}	DVector;
*	        #define	HPS_DVECTOR	DVector
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_PLANE
#	ifdef SWIG
typedef float HPS_PLANE;
#	else
#   define	HPS_PLANE	void
/*
* NOTE: if you want a real definition for "HPS_PLANE", put
*		typedef	struct {float a, b, c, d;} Plane;
*	        #define	HPS_PLANE	Plane
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_DPLANE
#	ifdef SWIG
typedef double HPS_DPLANE;
#	else
#   define	HPS_DPLANE	void
/*
* NOTE: if you want a real definition for "HPS_DPLANE", put
*		typedef	struct {double a, b, c, d;} DPlane;
*	        #define	HPS_DPLANE	DPlane
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_RGB
#	ifdef SWIG
typedef float HPS_RGB;
#	else
#   	define	HPS_RGB	void
/*
* NOTE: if you want a real definition for "HPS_RGB", put
*		typedef	struct {float red, green, blue;}	RGB;
*	        #define	HPS_RGB	RGB
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_RGBA
#	ifdef SWIG
typedef float HPS_RGBA;
#	else
#   	define	HPS_RGBA	void
/*
* NOTE: if you want a real definition for "HPS_RGB", put
*		typedef	struct {float red, green, blue, alpha;}	RGBA;
*	        #define	HPS_RGBA	RGBA
*	     in your program before including <hc.h>.
*/
#	endif
#endif

#ifndef HPS_BOOLEAN
#	ifdef __cplusplus
#	define	HPS_BOOLEAN	bool
#	else
#	define	HPS_BOOLEAN	char
#endif
#endif

#ifndef HPS_POINTER_SIZED_INT
#if defined(_M_IA64) || defined(_M_AMD64) || defined(WIN64) || defined(_WIN64) || defined(_M_X64)
#   define  HPS_POINTER_SIZED_INT    __int64
#else
#   define  HPS_POINTER_SIZED_INT    long
#endif
#endif

#ifndef HPS_ERROR_KEY
#   define  HPS_ERROR_KEY      (HPS::Key)-1
#endif

#ifndef HPS_PIXEL
/* (This definition probably shouldn't be changed) */
#   define	HPS_PIXEL	void
#endif

class HPS_EventHandler : public HPS::EventHandler
{
public:
	HPS_EventHandler() : HPS::EventHandler() {}
	virtual ~HPS_EventHandler() {}
	HPS::Canvas * _canvas;
	virtual HandleResult Handle( HPS::Event const * in_event )
	{
		if ( in_event != NULL )
		{
			HPS::MouseEvent const * mouse_input = static_cast<HPS::MouseEvent const *>( in_event );
			HPS::KeyboardEvent const * input = static_cast<HPS::KeyboardEvent const *>( in_event );
			if ( input->CurrentAction == HPS::KeyboardEvent::Action::KeyDown )
			{
			}
			if ( mouse_input->CurrentAction == HPS::MouseEvent::Action::ButtonDown )
			{
			}
		}
		return HandleResult::NotHandled;
	}
};

namespace std
{
	template<>
	struct less < HPS::Key >
	{
		bool operator()( HPS::Key const& a, HPS::Key const& b ) const
		{
			if ( HPS_Is_Valid_Key( a ) )
			{
				if ( HPS_Is_Valid_Key( b ) )
					return a.GetInstanceID() < b.GetInstanceID();
				else
					return true;
			} else return false;
		}
	};

	template<>
	struct less < HPS::SegmentKey >
	{
		bool operator()( HPS::SegmentKey const& a, HPS::SegmentKey const& b ) const
		{
			if ( HPS_Is_Valid_Segment_Key( a ) )
			{
				if ( HPS_Is_Valid_Segment_Key( b ) )
					return a.GetInstanceID() < b.GetInstanceID();
				else
					return true;
			} else return false;
		}
	};
}

inline HPS::SegmentKey HPS_Cast_SegmentKey( HPS::Key in_key )
{
	if ( in_key.Type() == HPS::Type::SegmentKey )
		return HPS::SegmentKey( in_key );
	return HPS_INVALID_SEGMENT_KEY;
}

typedef struct mergepoint {
	float	x, y, z;
}     MergePoint;
class HPS_Point;

#define MAX_PATTERN_SIZE 260	/* based on MS Windows MAX_PATH */

DECL_HPS HPS_BOOLEAN		HPS_Parse_String( const char * in_string, const char * in_delimiter, int in_offset, char * out_token );

#ifdef _MSC_VER
#define HPS_MACRO_DEBUG_PRINTF
#ifdef HPS_MACRO_DEBUG_PRINTF
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
void DbgPrintf(
	const char* fmt, // Format string (printf style).
	...              // Variable number of arguments.
	);
#define DBGPRINTF(s, ...);
#else
#define inline DBGPRINTF(s, ...) { }
#endif
#endif

#endif // HPS_ACIS_H_DEFINED
