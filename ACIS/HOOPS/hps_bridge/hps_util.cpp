/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: hps_util.cpp,v 1.6 2002/07/18 22:58:35 jhauswir Exp $

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.

#include <stdio.h>

#include "acis.hxx"
#include "kernapi.hxx"
#include "hps_util.h"
#include "rgbcolor.hxx"
#include "rnd_api.hxx"
#include "hps_bridge.h"
#include "hps_rend_options.h"

#include "transf.hxx"
#include "acistol.hxx"

#include "api.hxx"

#include "attrib.hxx"
#include "rh_attr.hxx"
#include "rh_efunc.hxx"
#include "ckoutcom.hxx"

#include "b_strutl.hxx"

#ifndef _MAX_PATH
#ifdef MAXPATHLEN
#define _MAX_PATH MAXPATHLEN
#else
#define _MAX_PATH 1024
#endif
#endif

#include "hps_entity_converter.h"
#include "hps_bridge.err"
#include <iostream>     // std::cout
#include <algorithm>    // std::reverse
#include <vector>       // std::vector
#include "edge.hxx"

void MapColorToString( const rgb_color &in_color, char* out_colorName )
{
	if ( out_colorName )
		sprintf( out_colorName, "%3i%3i%3i\0", (int)( in_color.red()*100.0 ), (int)( in_color.green()*100.0 ), (int)( in_color.blue()*100.0 ) );
}

void transf_to_matrix( float *out_float_mat, const SPAtransf &in_tform )
{
	if ( !out_float_mat )
		return;
	SPAmatrix mat = in_tform.affine();
	out_float_mat[0] = (float)( mat.element( 0, 0 ) * in_tform.scaling() );
	out_float_mat[1] = (float)( mat.element( 0, 1 ) * in_tform.scaling() );
	out_float_mat[2] = (float)( mat.element( 0, 2 ) * in_tform.scaling() );
	out_float_mat[3] = (float)( 0.0 );
	out_float_mat[4] = (float)( mat.element( 1, 0 ) * in_tform.scaling() );
	out_float_mat[5] = (float)( mat.element( 1, 1 ) * in_tform.scaling() );
	out_float_mat[6] = (float)( mat.element( 1, 2 ) * in_tform.scaling() );
	out_float_mat[7] = (float)( 0.0 );
	out_float_mat[8] = (float)( mat.element( 2, 0 ) * in_tform.scaling() );
	out_float_mat[9] = (float)( mat.element( 2, 1 ) * in_tform.scaling() );
	out_float_mat[10] = (float)( mat.element( 2, 2 ) * in_tform.scaling() );
	out_float_mat[11] = (float)( 0.0 );
	SPAvector trans = in_tform.translation();
	out_float_mat[12] = (float)( trans.x() );
	out_float_mat[13] = (float)( trans.y() );
	out_float_mat[14] = (float)( trans.z() );
	out_float_mat[15] = (float)( 1.0 );
}

void ConvertSPAPositionToFloatArray( int in_num_pts, SPAposition *in_pos, float *&out_float_array )
{
	API_SYS_BEGIN
	{
		if ( in_num_pts > 0 )
		{
			out_float_array = ACIS_NEW float[in_num_pts * 3];
			for ( int i = 0; i < in_num_pts; i++ )
			{
				out_float_array[i * 3] = (float)in_pos[i].x();
				out_float_array[i * 3 + 1] = (float)in_pos[i].y();
				out_float_array[i * 3 + 2] = (float)in_pos[i].z();
			}
		}
	} API_SYS_END
}

void ConvertDoubleToFloatArray( int in_count, double *in_double_array, float *&out_float_array )
{
	API_SYS_BEGIN
	{
		if ( in_count > 0 && in_double_array )
		{
			out_float_array = ACIS_NEW float[in_count];
			for ( int i = 0; i < in_count; i++ )
				out_float_array[i] = (float)in_double_array[i];
		}
	} API_SYS_END
}

char * ptoax( void* in_ptr, char* out_buffer )
{
	out_buffer[POINTER_BUFFER_SIZE - 1] = 0;			// Null terminate
	char* cptr = &out_buffer[POINTER_BUFFER_SIZE - 2];	// Address of lowest order nyble of result
	uintptr_t p = (uintptr_t)in_ptr;
	for ( int j = 0; j < POINTER_BUFFER_SIZE - 1; j++ )
		out_buffer[j] = '0';
	out_buffer[1] = 'x';
	if ( p )
	{
		while ( p )
		{
			char c = (char)( ( p & 0xF ) + '0' );
			if ( c > '9' ) c += ( 'a' - '9' ) - 1;
			*cptr-- = c;
			p /= 0x10;
		}
	}
	return out_buffer;
}

// Convert ascii in 0x format to a pointer
void * axtop( char* in_buffer )
{
	uintptr_t p = 0;
	char* cptr = &in_buffer[2];		// Skip the 0x
	while ( *cptr )
	{
		p *= 0x10;
		uintptr_t nyble = *cptr++ - '0';
		if ( nyble > 9 )
			nyble -= ( 'a' - '9' ) - 1;
		if ( nyble > 0xF )
			nyble -= (uintptr_t)( ( 'A' - 'a' ) );		// Just in the case someone strupper'd it.
		p |= nyble;
	}
	return (void*)p;
}

HPS::SegmentKey HPS_OpenColorSegment( ENTITY* in_entity, HPS::SegmentKey in_segment, logical in_traverse_up )
{
	logical color_exists = FALSE;
	logical texture_exists = FALSE;
	logical material_properties_exist = FALSE;
	char segmentName[100];
	rgb_color color;
	ATTRIB * rh_material = find_attrib( in_entity, ATTRIB_RH_TYPE, ATTRIB_RENDER_TYPE );
	double	transparency = 0.0;
	logical reflection_state = FALSE;
	double	reflect_ambient = 0.0;
	double	reflect_diffuse = 0.0;
	double	reflect_specular = 0.0;
	double	reflect_exponent = 0.0;
	HPS::SegmentKey return_key = HPS_INVALID_SEGMENT_KEY;
	if ( rh_material )
	{
		material_properties_exist = TRUE;
		RH_ENTITY_PROPS new_props;
		new_props.material = ( (ATTRIB_RENDER *)( rh_material ) )->material();
		new_props.sides = ( (ATTRIB_RENDER *)( rh_material ) )->sides();
		new_props.texture_space = ( (ATTRIB_RENDER *)( rh_material ) )->texture_space();
		new_props.local_transf = ( (ATTRIB_RENDER *)( rh_material ) )->local_transf();
		new_props.local_transf_modified = ( (ATTRIB_RENDER *)( rh_material ) )->local_transf_modified();
		//
		// if the entity has sidedness attached ( non 0 ), then used that 
		//
		//if ( new_props.sides )
		//	mp->set_NumSides(new_props.sides);
		if ( new_props.material )
		{
			// Guy. Addresses concerns posed in BTS 87839. Use backplane culling instead of no polygon handedness
			if ( new_props.sides > 1 )
				in_segment.GetCullingControl().SetBackFace( false );
			// base color
			double	color_red = 0.0;
			double	color_green = 0.0;
			double	color_blue = 0.0;
			// Now grab any settings that are present.  If the material does
			// not have a setting for any of these the procedures don't modify
			// the data, so the default will stick. 
			logical has_material_color = FALSE;
			rh_get_material_color( new_props.material, color_red, color_green, color_blue, has_material_color );
			//mp->set_BaseColor(rgb_color(color_red,color_green,color_blue));
			// reflectance parameters
			rh_get_reflect_status( new_props.material, reflection_state );
			rh_get_material_reflection( new_props.material, reflect_ambient, reflect_diffuse, reflect_specular, reflect_exponent );
			// transparency value
			rh_get_material_transparency( new_props.material, transparency );
			//if (transparency!=1.0)
			//	mp->set_Transparency(transparency);
			//if (new_props.sides > 1)
			//	mp->set_NumSides( new_props.sides);
			// texture name
			char *texture_name = 0;
			rh_get_material_texture_name( new_props.material, (const char *&)texture_name );
			//texture_name=0;
			// There can be a texture only if there is a texture file
			// and we care about this only if we are processing a mesh/face
			if ( texture_name )
			{
				texture_exists = TRUE;
				size_t len = strlen( texture_name );
				char * hps_name = (char *)ACIS_ALLOCA( sizeof( char )* ( len + 1 ) );
				strcpy( hps_name, texture_name );
				for ( unsigned int i = 0; i < len; i++ )
				{
					if ( hps_name[i] == '/' || hps_name[i] == '\\' )
						hps_name[i] = '_';
				}
				return_key = in_segment.Subsegment( hps_name );
				logical loaded_texture = FALSE;
				HPS::ImageKit imageKit;
				bool okay = false;
				try
				{
					HPS::UTF8 file_in = texture_name;
					HPS::Image::ImportOptionsKit importOptionsKit;
					importOptionsKit.SetFormat( HPS::Image::Format::Jpeg );
					imageKit = HPS::Image::File::Import( static_cast<const char*>( file_in ), importOptionsKit );
					okay = true;
				} catch ( ... )
				{
#ifdef NT
					okay = LoadBMP( imageKit, texture_name );
#endif // NT
				}
				if ( okay )
				{
					HPS::PortfolioKey pKey = HPS_Portfolio;
					HPS::ImageDefinition imageDefinition = pKey.DefineImage( hps_name, imageKit );
					HPS::TextureOptionsKit textureOptionsKit;
					textureOptionsKit.SetParameterizationSource( HPS::Material::Texture::Parameterization::UV );
					pKey.DefineTexture( hps_name, imageDefinition, textureOptionsKit );
					return_key.GetPortfolioControl().Push( pKey );
					return_key.GetMaterialMappingControl().SetFaceTexture( hps_name );
				}
			}
		}
	}
	check_outcome( api_rh_get_entity_rgb( in_entity, color, in_traverse_up, color_exists ) );
	HPS::RGBColor face_color( (float)color.red(), (float)color.green(), (float)color.blue() );
	if ( ( color_exists || material_properties_exist ) && !texture_exists )
	{
		if ( !color_exists )
		{
			check_outcome( api_rh_get_entity_rgb( in_entity, color, TRUE, color_exists ) );
			face_color = HPS::RGBColor( (float)color.red(), (float)color.green(), (float)color.blue() );;
		}
		char str[500];
		MapColorToString( color, segmentName );
		sprintf( str, "%s %3i", segmentName, (int)( 100.0 * transparency ) );
		return_key = HPS_Open_Segment( str, in_segment );
		return_key.GetMaterialMappingControl().SetLineColor( face_color );
		return_key.GetMaterialMappingControl().SetMarkerColor( face_color );
		return_key.GetMaterialMappingControl().SetFaceColor( face_color );
		double transparency_inherented = 0;
		api_rh_get_material_transp( in_entity, transparency_inherented, TRUE );
		transparency = transparency_inherented;
		if ( 0 < transparency && transparency <= 1.0 )
			return_key.GetMaterialMappingControl().SetFaceAlpha( 1 - (float)transparency );
		// Guy. This recognizes reflect_diffuse and reflect_specular. However, reflect_ambient and reflect_exponenet still don't work in HOOPS.
		if ( rh_material && ( (ATTRIB_RENDER *)( rh_material ) )->material() && reflection_state )
		{
			HPS::MaterialKit mat_kit;
			mat_kit.SetDiffuse( HPS::RGBAColor( (float)( reflect_diffuse * color.red() ), (float)( reflect_diffuse * color.green() ), (float)( reflect_diffuse * color.blue() ) ) );
			mat_kit.SetSpecular( HPS::RGBAColor( (float)reflect_specular, (float)reflect_specular, (float)reflect_specular ) );
			mat_kit.SetGloss( (float)reflect_exponent );
			return_key.GetMaterialMappingControl().SetFaceMaterial( mat_kit );
			return_key.GetMaterialMappingControl().SetAmbientLightColor( HPS::RGBAColor( (float)reflect_ambient, (float)reflect_ambient, (float)reflect_ambient ) );
		}
	}
	hps_rendering_options hps_opts = HPS_Get_Rendering_Options();
	if ( hps_opts.GetFacetStyle() == 1 )
	{	// set the color of facet lines to be the complementary color of the face when the shading is on
		bool face_is_visible = false;
		HPS::SegmentKey  view_key = HPS_Get_Current_View().GetSegmentKey();
		if ( HPS_Is_Valid_Segment_Key( view_key ) )
			view_key.GetVisibilityControl().ShowFaces( face_is_visible );
		HPS::RGBColor facet_edge_color( face_color );
		if ( face_is_visible )
		{	// Facet lines are displayed on shaded faces...
			// If the hue is any shade of grey, use white or black for the facet lines.
			// Else, use the complement shade of the hue.
			if ( ( fabs( color.red() - color.green() ) <= 0.07 )
				 && ( fabs( color.red() - color.blue() ) <= 0.07 )
				 && ( fabs( color.green() - color.blue() ) <= 0.07 ) )
			{	// this is a shade of grey...use black or white facet lines.  red ~= green ~= blue, so we can look at just red
				if ( color.red() < 0.3 ) // 0.5 was the first choice, but white lines get lost in the light reflections
					facet_edge_color.red = facet_edge_color.green = facet_edge_color.blue = 1; // White
				else
					facet_edge_color.red = facet_edge_color.green = facet_edge_color.blue = 0; // BLACK
			} else
			{	// this is not a shade of grey, use the complementary hue for facet lines
				float hue, saturation, value;
				facet_edge_color.ShowHSV( hue, saturation, value );
				facet_edge_color = HPS::RGBColor::HSV( (float)( ( ( (int)hue ) + 180 ) % 360 ), saturation, value );
			}
		}
		if ( HPS_Is_Valid_Segment_Key( return_key ) )
			return_key.GetMaterialMappingControl().SetEdgeColor( facet_edge_color );
	}
	if ( color_exists || material_properties_exist )
		return return_key;
	return in_segment;
} // end of HPS_OpenColorSegment()

HPS::SegmentKey HPS_KOpenPointerSegment( void *in_ptr, HPS::SegmentKey in_segment_key )
{
	char pbuffer[POINTER_BUFFER_SIZE];
	ptoax( in_ptr, pbuffer );
	HPS::SegmentKey key = HPS_KOpen_Segment( pbuffer, in_segment_key );
	return key;
}

bool HPS_Is_Valid_Segment_Key( HPS::SegmentKey in_key )
{
	if ( in_key.Empty() )
		return false;
	if ( in_key.Type() == HPS::Type::None )
		return false;
	static HPS::SegmentKey a_segment_key;
	HPS::Type type = in_key.Type();
	intptr_t key_class_ID = in_key.GetClassID();
	intptr_t a_class_ID = a_segment_key.GetClassID();
	if ( type == HPS::Type::SegmentKey && key_class_ID == a_class_ID )
		return true;
	return false;
}

bool HPS_Is_Valid_Key( HPS::Key in_key )
{
	if ( in_key.Empty() )
		return false;
	static HPS::Key a_key;
	if ( in_key.GetClassID() == a_key.GetClassID() && in_key.Type() != HPS::Type::None )
		return true;
	return false;
}

HPS::Key HPS_Invalid_Key()
{
	static HPS::Key key;
	return key;
}

HPS::SegmentKey HPS_Invalid_Segment_Key()
{
	static HPS::SegmentKey key;
	return key;
}

HPS_BOOLEAN	HPS_Update_Display()
{
	HPS_Get_Current_View().GetOwningLayouts()[0].GetOwningCanvases()[0].Update();
	//HPS_Get_Current_View().GetOwningLayouts()[0].GetOwningCanvases()[0].UpdateWithNotifier().Wait();
	//HPS::View view = HPS_Get_Current_View();
	//if ( !view.Empty() )
	//{
	//	HPS::KeyPath key_path;
	//	HPS_Get_Current_View_Model_KeyPath( key_path );
	//	HPS::KeyArray key_array;
	//	key_path.ShowKeys( key_array );
	//	if ( key_array.size() > 0 )
	//	{
	//		HPS::IncludeKey model_include_key( key_array[0] );
	//		printf( "model_include_key = %d\n", model_include_key.GetInstanceID() );
	//	} else
	//		printf( "*** Error: no model\n" );
	//} else
	//	printf( "*** Error: view is empty\n" );
	return true;
}

HPS_BOOLEAN	HPS_Update_Display( HPS::View & in_view )
{
	if ( in_view.Empty() )
		return false;
	in_view.Update();
	//HPS_Get_Current_View().GetOwningLayouts()[0].GetOwningCanvases()[0].UpdateWithNotifier().Wait();
	return true;
}

#ifdef NT
void BGRToRGB( BITMAPINFO *biInfo, void *biBits, int biWidth );
/* Macro to determine to round off the given value to the closest byte */
#define WIDTHBYTES(i)	((i+31)/32*4)
#define BFT_BITMAP  0x4d42  // 'BM' -- indicates structure is BITMAPFILEHEADER

// struct BITMAPFILEHEADER {
//      WORD  bfType
//      DWORD bfSize
//      WORD  bfReserved1
//      WORD  bfReserved2
//      DWORD bfOffBits
// }
#define OFFSET_bfType			 0
#define OFFSET_bfSize			 2
#define OFFSET_bfReserved1		 6
#define OFFSET_bfReserved2		 8
#define OFFSET_bfOffBits		10
#define SIZEOF_BITMAPFILEHEADER 14

// Read a WORD-aligned DWORD.  Needed because BITMAPFILEHEADER has
// WORD-alignment.
#define READDWORD(pv)   ( (DWORD)((PWORD)(pv))[0] | ( (DWORD)( (PWORD)( pv ) )[1] << 16 ) )   \

// Computes the number of BYTES needed to contain n number of bits.
#define BITS2BYTES(n)   ( ((n) + 7) >> 3 )

typedef struct _RGBIMAGEREC {
	ULONG sizeX;
	ULONG sizeY;
	BYTE *data;
} RGBIMAGEREC;

extern BOOL bVerifyDIB( WCHAR *fileName, ULONG *pWidth, ULONG *pHeight );

//
// RGB color structure that stores colors in R-G-B order in
// memory instead of the B-G-R format used by RGBTRIPLE.
//
typedef struct _CGLRGBTRIPLE {
	BYTE rgbRed;
	BYTE rgbGreen;
	BYTE rgbBlue;
} CGLRGBTRIPLE;

// Code to convert from a DIB to a Hoops image
unsigned char * InsertRGBImage( int in_width, int in_height, unsigned char* in_buffer )
{
	fp_sentry fps;
	// Windows dword-aligns the rows, HOOPS wants them packed...
	int nbRowIn = 3 * in_width;
	//	if (nbRowIn % 4 != 0)
	//		nbRowIn += 4 - (nbRowIn % 4);
	// But there is a bug in the HOOPS OpenGL texture mapping so
	// odd widths don't work, so pad it too...
	int wHoops = in_width;
	if ( wHoops % 4 != 0 )
		wHoops += 4 - ( wHoops % 4 );
	int nbRowOut = 3 * wHoops;
	unsigned char* pImage = ACIS_NEW unsigned char[3 * wHoops * in_height];
	unsigned char* pImgRow = pImage;
	// And the byte orderings are different.
	for ( int j = 0; j < in_height; ++j, in_buffer += nbRowIn, pImgRow += nbRowOut )
	{
		int i;
		for ( i = 0; i < 3 * in_width; i += 3 )
		{
			pImgRow[i + 0] = in_buffer[i + 2];
			pImgRow[i + 1] = in_buffer[i + 1];
			pImgRow[i + 2] = in_buffer[i + 0];
		}
		int iLast = i - 3;			// Pad with last valid RGB on input row
		for ( ; i < nbRowOut; i += 3 )
		{
			pImgRow[i + 0] = in_buffer[iLast + 2];
			pImgRow[i + 1] = in_buffer[iLast + 1];
			pImgRow[i + 2] = in_buffer[iLast + 0];
		}
	}
	return pImage;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DibNumColors(VOID FAR * pv)                                *
 *                                                                          *
 *  PURPOSE    : Determines the number of colors in the DIB by looking at   *
 *               the BitCount filed in the info block.                      *
 *                                                                          *
 *  RETURNS    : The number of colors in the DIB.                           *
 *                                                                          *
 * Take from SDK ShowDIB example.                                           *
 ****************************************************************************/

WORD DibNumColors( VOID FAR * in_bitmap_header )
{
	WORD                bits;
	LPBITMAPINFOHEADER  lpbi;
	LPBITMAPCOREHEADER  lpbc;
	lpbi = ( (LPBITMAPINFOHEADER)in_bitmap_header );
	lpbc = ( (LPBITMAPCOREHEADER)in_bitmap_header );
	/*  With the BITMAPINFO format headers, the size of the palette
	 *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
	 *  is dependent on the bits per pixel ( = 2 raised to the power of
	 *  bits/pixel).
	 *
	 *  Because of the way we use this call, BITMAPINFOHEADER may be out
	 *  of alignment if it follows a BITMAPFILEHEADER.  So use the macro
	 *  to safely access DWORD fields.
	 */
	if ( READDWORD( &lpbi->biSize ) != sizeof( BITMAPCOREHEADER ) )
	{
		if ( READDWORD( &lpbi->biClrUsed ) != 0 )
			return (WORD)READDWORD( &lpbi->biClrUsed );
		bits = lpbi->biBitCount;
	} else
		bits = lpbc->bcBitCount;
	switch ( bits )
	{
	case 1:
		return 2;
	case 4:
		return 16;
	case 8:
		return 256;
	default:
		/* A 24 bitcount DIB has no color table */
		return 0;
	}
}

/******************************Public*Routine******************************\
* tkDIBImageLoadAW
*
* Loads a DIB file and converts it into a TK image format.
*
* The technique used is based on CreateDIBSection and SetDIBits.
* CreateDIBSection is used to create a DIB with a format easily converted
* into the TK image format (packed 24BPP RGB)--namely 32BPP RGB.  The
* resulting bitmap is selected into a memory DC.
*
* The DIB file is mapped into memory and SetDIBits called to initialize
* the memory DC bitmap.  It is during this step that GDI converts the
* arbitrary DIB file format to our 32BPP RGB format.
*
* Finally, the 32BPP RGB data in the DIB section is read out and repacked
* as 24BPP RGB.
*
* Returns:
*	TRUE if loaded ok, FALSE otherwise.
*
\**************************************************************************/
bool tkDIBImageLoadAW( const char *in_fileName, RGBIMAGEREC &in_image )
{
	bool			result = FALSE;				// presume failure
	WORD             wNumColors;    // Number of colors in color table
	BITMAPFILEHEADER *pbmf;         // Ptr to file header
	BITMAPINFOHEADER *pbmihFile;    // Ptr to file's info header (if it exists)
	BITMAPCOREHEADER *pbmchFile;    // Ptr to file's core header (if it exists)
	PVOID            pvBitsFile = NULL;    // Ptr to bitmap bits in file
	PBYTE            pjBitsRGB;     // Ptr to 32BPP RGB image in DIB section
	PBYTE            pjTKBits;      // Ptr to final TK image bits
	PBYTE            pjSrc;         // Ptr to image file used for conversion
	PBYTE            pjDst;         // Ptr to TK image used for conversion
	// These need to be cleaned up when we exit:
	HANDLE     hFile = INVALID_HANDLE_VALUE;        // File handle
	HANDLE     hMap = (HANDLE)NULL;                // Mapping object handle
	PVOID      pvFile = (PVOID)NULL;               // Ptr to mapped file
	HDC        hdcMem = (HDC)NULL;                 // 32BPP mem DC
	HBITMAP    hbmRGB = (HBITMAP)NULL;             // 32BPP RGB bitmap
	BITMAPINFO *pbmiSource = (BITMAPINFO *)NULL;   // Ptr to 32BPP BITMAPINFO
	BITMAPINFO *pbmiRGB = (BITMAPINFO *)NULL;      // Ptr tp file's BITMAPINFO
	int i, j;
	// Map the DIB file into memory.
	hFile = CreateFile( in_fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
	if ( hFile == INVALID_HANDLE_VALUE )
		goto tkDIBLoadImage_cleanup;
	hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
	if ( !hMap )
		goto tkDIBLoadImage_cleanup;
	pvFile = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
	if ( !pvFile )
		goto tkDIBLoadImage_cleanup;
	// Check the file header.  If the BFT_BITMAP magic number is there,
	// then the file format is a BITMAPFILEHEADER followed immediately
	// by either a BITMAPINFOHEADER or a BITMAPCOREHEADER.  The bitmap
	// bits, in this case, are located at the offset bfOffBits from the
	// BITMAPFILEHEADER.
	//
	// Otherwise, this may be a raw BITMAPINFOHEADER or BITMAPCOREHEADER
	// followed immediately with the color table and the bitmap bits.
	pbmf = (BITMAPFILEHEADER *)pvFile;
	if ( pbmf->bfType == BFT_BITMAP )
	{
		pbmihFile = (BITMAPINFOHEADER *)( (PBYTE)pbmf + SIZEOF_BITMAPFILEHEADER );
		// BITMAPFILEHEADER is WORD aligned, so use safe macro to read DWORD
		// bfOffBits field.
		pvBitsFile = (PVOID *)( (PBYTE)pbmf
								+ READDWORD( (PBYTE)pbmf + OFFSET_bfOffBits ) );
	} else
	{
		pbmihFile = (BITMAPINFOHEADER *)pvFile;
		// Determination of where the bitmaps bits are needs to wait until we
		// know for sure whether we have a BITMAPINFOHEADER or a BITMAPCOREHEADER.
	}
	// Determine the number of colors in the DIB palette.  This is non-zero
	// only for 8BPP or less.
	wNumColors = DibNumColors( pbmihFile );
	// Create a BITMAPINFO (with color table) for the DIB file.  Because the
	// file may not have one (BITMAPCORE case) and potential alignment problems,
	// we will create a new one in memory we allocate.
	//
	// We distinguish between BITMAPINFO and BITMAPCORE cases based upon
	// BITMAPINFOHEADER.biSize.
	pbmiSource = (BITMAPINFO *)LocalAlloc( LMEM_FIXED, sizeof( BITMAPINFO ) + wNumColors * sizeof( RGBQUAD ) );
	if ( !pbmiSource )
		goto tkDIBLoadImage_cleanup;
	// Note: need to use safe READDWORD macro because pbmihFile may
	// have only WORD alignment if it follows a BITMAPFILEHEADER.
	switch ( READDWORD( &pbmihFile->biSize ) )
	{
	case sizeof( BITMAPINFOHEADER ) :
		// Convert WORD-aligned BITMAPINFOHEADER to aligned BITMAPINFO.
		pbmiSource->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		pbmiSource->bmiHeader.biWidth = READDWORD( &pbmihFile->biWidth );
		pbmiSource->bmiHeader.biHeight = READDWORD( &pbmihFile->biHeight );
		pbmiSource->bmiHeader.biPlanes = pbmihFile->biPlanes;
		pbmiSource->bmiHeader.biBitCount = pbmihFile->biBitCount;
		pbmiSource->bmiHeader.biCompression = READDWORD( &pbmihFile->biCompression );
		pbmiSource->bmiHeader.biSizeImage = READDWORD( &pbmihFile->biSizeImage );
		pbmiSource->bmiHeader.biXPelsPerMeter = READDWORD( &pbmihFile->biXPelsPerMeter );
		pbmiSource->bmiHeader.biYPelsPerMeter = READDWORD( &pbmihFile->biYPelsPerMeter );
		pbmiSource->bmiHeader.biClrUsed = READDWORD( &pbmihFile->biClrUsed );
		pbmiSource->bmiHeader.biClrImportant = READDWORD( &pbmihFile->biClrImportant );
		// Copy color table.  It immediately follows the BITMAPINFOHEADER.
		memcpy( (PVOID)&pbmiSource->bmiColors[0], (PVOID)( pbmihFile + 1 ),
				wNumColors * sizeof( RGBQUAD ) );
		// If we haven't already determined the SPAposition of the image bits,
		// we may now assume that they immediately follow the color table.
		if ( !pvBitsFile )
			pvBitsFile = (PVOID)( (PBYTE)( pbmihFile + 1 ) + wNumColors * sizeof( RGBQUAD ) );
		break;
	case sizeof( BITMAPCOREHEADER ) :
		pbmchFile = (BITMAPCOREHEADER *)pbmihFile;
		// Convert BITMAPCOREHEADER to BITMAPINFOHEADER.
		pbmiSource->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		pbmiSource->bmiHeader.biWidth = (DWORD)pbmchFile->bcWidth;
		pbmiSource->bmiHeader.biHeight = (DWORD)pbmchFile->bcHeight;
		pbmiSource->bmiHeader.biPlanes = pbmchFile->bcPlanes;
		pbmiSource->bmiHeader.biBitCount = pbmchFile->bcBitCount;
		pbmiSource->bmiHeader.biCompression = BI_RGB;
		pbmiSource->bmiHeader.biSizeImage = 0;
		pbmiSource->bmiHeader.biXPelsPerMeter = 0;
		pbmiSource->bmiHeader.biYPelsPerMeter = 0;
		pbmiSource->bmiHeader.biClrUsed = wNumColors;
		pbmiSource->bmiHeader.biClrImportant = wNumColors;
		// Convert RGBTRIPLE color table into RGBQUAD color table.
		{
			RGBQUAD *rgb4 = pbmiSource->bmiColors;
			RGBTRIPLE *rgb3 = (RGBTRIPLE *)( pbmchFile + 1 );
			for ( i = 0; i < wNumColors; i++ )
			{
				rgb4->rgbRed = rgb3->rgbtRed;
				rgb4->rgbGreen = rgb3->rgbtGreen;
				rgb4->rgbBlue = rgb3->rgbtBlue;
				rgb4->rgbReserved = 0;
				rgb4++;
				rgb3++;
			}
		}
		// If we haven't already determined the SPAposition of the image bits,
		// we may now assume that they immediately follow the color table.
		if ( !pvBitsFile )
			pvBitsFile = (PVOID)( (PBYTE)( pbmihFile + 1 ) + wNumColors * sizeof( RGBTRIPLE ) );
		break;
	default:
		goto tkDIBLoadImage_cleanup;
	}
	// Fill in default values (for fields that can have defaults).
	if ( pbmiSource->bmiHeader.biSizeImage == 0 )
		pbmiSource->bmiHeader.biSizeImage = BITS2BYTES( (DWORD)pbmiSource->bmiHeader.biWidth * pbmiSource->bmiHeader.biBitCount )
		* pbmiSource->bmiHeader.biHeight;
	if ( pbmiSource->bmiHeader.biClrUsed == 0 )
		pbmiSource->bmiHeader.biClrUsed = wNumColors;
	// Create memory DC.
	hdcMem = CreateCompatibleDC( NULL );
	if ( !hdcMem )
		goto tkDIBLoadImage_cleanup;
	// Create a 32BPP RGB DIB section and select it into the memory DC.
	pbmiRGB = (BITMAPINFO *)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, sizeof( BITMAPINFO ) + 2 * sizeof( RGBQUAD ) );
	if ( !pbmiRGB )
		goto tkDIBLoadImage_cleanup;
	pbmiRGB->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	pbmiRGB->bmiHeader.biWidth = pbmiSource->bmiHeader.biWidth;
	pbmiRGB->bmiHeader.biHeight = pbmiSource->bmiHeader.biHeight;
	pbmiRGB->bmiHeader.biPlanes = 1;
	pbmiRGB->bmiHeader.biBitCount = 32;
	pbmiRGB->bmiHeader.biCompression = BI_RGB;
	pbmiRGB->bmiHeader.biSizeImage = pbmiRGB->bmiHeader.biWidth
		* abs( pbmiRGB->bmiHeader.biHeight ) * 4;
	// This may look backwards, but because of the ordering of RGBQUADS,
	// this will yield an RGB format (vs. the BGR format that will result
	// from the "natural" ordering).
	pbmiRGB->bmiColors[0].rgbBlue = 0xff;  // red mask
	pbmiRGB->bmiColors[1].rgbGreen = 0xff;  // green mask
	pbmiRGB->bmiColors[2].rgbRed = 0xff;  // blue mask
	hbmRGB = CreateDIBSection( hdcMem, (BITMAPINFO *)pbmiRGB, DIB_RGB_COLORS, (VOID **)&pjBitsRGB, NULL, 0 );
	if ( !hbmRGB )
		goto tkDIBLoadImage_cleanup;
	if ( !SelectObject( hdcMem, hbmRGB ) )
		goto tkDIBLoadImage_cleanup;
	// Slam the DIB file image into the memory DC.  GDI will do the work of
	// translating whatever format the DIB file has into our 32BPP RGB format.
	if ( !SetDIBits( hdcMem, hbmRGB, 0, pbmiSource->bmiHeader.biHeight, pvBitsFile, pbmiSource, DIB_RGB_COLORS ) )
		goto tkDIBLoadImage_cleanup;
	// Convert to TK image format (packed RGB format).
	pjTKBits = (PBYTE)LocalAlloc( LMEM_FIXED, pbmiRGB->bmiHeader.biSizeImage );
	if ( !pjTKBits )
		goto tkDIBLoadImage_cleanup;
	pjSrc = pjBitsRGB;
	pjDst = pjTKBits;
	for ( i = 0; i < pbmiSource->bmiHeader.biHeight; i++ )
	{
		for ( j = 0; j < pbmiSource->bmiHeader.biWidth; j++ )
		{
			*pjDst++ = *pjSrc++;
			*pjDst++ = *pjSrc++;
			*pjDst++ = *pjSrc++;
			pjSrc++;
		}
	}
	// If we get to here, we have suceeded!
	result = TRUE;
	in_image.sizeX = pbmiSource->bmiHeader.biWidth;
	in_image.sizeY = pbmiSource->bmiHeader.biHeight;
	in_image.data = pjTKBits;
	//in_image.data = pjBitsRGB;
	// Cleanup objects.
tkDIBLoadImage_cleanup:
	{
		if ( hdcMem )
			DeleteDC( hdcMem );
		if ( hbmRGB )
			DeleteObject( hbmRGB );
		if ( pbmiRGB )
			LocalFree( pbmiRGB );
		if ( pbmiSource )
			LocalFree( pbmiSource );
		if ( pvFile )
			UnmapViewOfFile( pvFile );
		if ( hMap )
			CloseHandle( hMap );
		if ( hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );
	}
	return result;
}

/******************************Public*Routine******************************\
* bVerifyDIB
*
* Stripped down version of tkDIBImageLoadAW that verifies that a bitmap
* file is valid and, if so, returns the bitmap dimensions.
*
* Returns:
*   TRUE if valid bitmap file; otherwise, FALSE.
*
\**************************************************************************/

BOOL bVerifyDIB( WCHAR *in_fileName, ULONG *in_pWidth, ULONG *in_pHeight )
{
	BOOL bRet = FALSE;
	BITMAPFILEHEADER *pbmf;         // Ptr to file header
	BITMAPINFOHEADER *pbmihFile;    // Ptr to file's info header (if it exists)
	BITMAPCOREHEADER *pbmchFile;    // Ptr to file's core header (if it exists)
	// These need to be cleaned up when we exit:
	HANDLE     hFile = INVALID_HANDLE_VALUE;        // File handle
	HANDLE     hMap = (HANDLE)NULL;                // Mapping object handle
	PVOID      pvFile = (PVOID)NULL;               // Ptr to mapped file
	// Map the DIB file into memory.
	hFile = CreateFileW( (LPWSTR)in_fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0 );
	if ( hFile == INVALID_HANDLE_VALUE )
		goto bVerifyDIB_cleanup;
	hMap = CreateFileMappingW( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
	if ( !hMap )
		goto bVerifyDIB_cleanup;
	pvFile = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
	if ( !pvFile )
		goto bVerifyDIB_cleanup;
	// Check the file header.  If the BFT_BITMAP magic number is there,
	// then the file format is a BITMAPFILEHEADER followed immediately
	// by either a BITMAPINFOHEADER or a BITMAPCOREHEADER.  The bitmap
	// bits, in this case, are located at the offset bfOffBits from the
	// BITMAPFILEHEADER.
	//
	// Otherwise, this may be a raw BITMAPINFOHEADER or BITMAPCOREHEADER
	// followed immediately with the color table and the bitmap bits.
	pbmf = (BITMAPFILEHEADER *)pvFile;
	if ( pbmf->bfType == BFT_BITMAP )
		pbmihFile = (BITMAPINFOHEADER *)( (PBYTE)pbmf + SIZEOF_BITMAPFILEHEADER );
	else
		pbmihFile = (BITMAPINFOHEADER *)pvFile;
	// Get the width and height from whatever header we have.
	//
	// We distinguish between BITMAPINFO and BITMAPCORE cases based upon
	// BITMAPINFOHEADER.biSize.
	// Note: need to use safe READDWORD macro because pbmihFile may
	// have only WORD alignment if it follows a BITMAPFILEHEADER.
	switch ( READDWORD( &pbmihFile->biSize ) )
	{
	case sizeof( BITMAPINFOHEADER ) :
		*in_pWidth = READDWORD( &pbmihFile->biWidth );
		*in_pHeight = READDWORD( &pbmihFile->biHeight );
		bRet = TRUE;
		break;
	case sizeof( BITMAPCOREHEADER ) :
		pbmchFile = (BITMAPCOREHEADER *)pbmihFile;
		// Convert BITMAPCOREHEADER to BITMAPINFOHEADER.
		*in_pWidth = (DWORD)pbmchFile->bcWidth;
		*in_pHeight = (DWORD)pbmchFile->bcHeight;
		bRet = TRUE;
		break;
	default:
		break;
	}
bVerifyDIB_cleanup:
	if ( pvFile )
		UnmapViewOfFile( pvFile );
	if ( hMap )
		CloseHandle( hMap );
	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );
	return bRet;
}

bool LoadBMP( HPS::ImageKit & in_imageKit, char *in_texture_name )
{
	RGBIMAGEREC image = { 0, 0, 0 };
	bool okay = tkDIBImageLoadAW( in_texture_name, image );
	if ( !okay )
		return false;
	unsigned char * temp = InsertRGBImage( image.sizeX, image.sizeY, image.data );
	in_imageKit.SetData( image.sizeX * image.sizeY * 3, temp );
	in_imageKit.SetSize( image.sizeX, image.sizeY );
	in_imageKit.SetFormat( HPS::Image::Format::RGB );
	ACIS_DELETE[] STD_CAST temp;
	return true;
}
#endif // NT

void MakeArrow( SPAposition			const &	in_arrow_base,
				SPAvector			const &	in_arrow_direction,
				double				const	in_arrow_length,
				hps_arrow_head_type const	in_arrow_type_o_head,
				hps_arrow_size_type const	in_arrow_size_o_head,
				HPS::SegmentKey				in_segment_key )
{
	SPAunit_vector	in_arrow_vector = normalise( in_arrow_direction );
	SPAposition		in_arrow_line[2];
	if ( in_arrow_length <= 0.0f )
	{
		in_arrow_line[0] = in_arrow_base;
		in_arrow_line[1] = in_arrow_base + in_arrow_direction;
	} else
	{
		in_arrow_line[0] = in_arrow_base;
		in_arrow_line[1] = in_arrow_base + in_arrow_vector * in_arrow_length;
	}
	float cone_length;
	switch ( in_arrow_size_o_head )
	{
	case hps_tiny:		cone_length = (float)in_arrow_length * 0.025f;	break;
	case hps_little:	cone_length = (float)in_arrow_length * 0.050f;	break;
	default:
	case hps_average:	cone_length = (float)in_arrow_length * 0.100f;	break;
	case hps_large:		cone_length = (float)in_arrow_length * 0.250f;	break;
	case hps_enormous:	cone_length = (float)in_arrow_length * 0.500f;	break;
	}
	SPAvector cone_vector = in_arrow_vector;
	cone_vector *= cone_length;
	SPAposition		cone_pos[3];
	cone_pos[0] = in_arrow_line[1];
	cone_pos[1] = in_arrow_line[1] - cone_vector;
	cone_pos[2] = in_arrow_line[1] - cone_vector * 2;
	float cone_radii[2];
	cone_radii[0] = cone_length * 0.25f;
	cone_radii[1] = 0.0F;

	// Insert arrow line
	fp_sentry fps;
	in_segment_key.InsertLine( HPS_Cast_Point( in_arrow_line[0] ), HPS_Cast_Point( in_arrow_line[1] ) );

	HPS::Point endPoints[] = { HPS_Cast_Point( cone_pos[1] ), HPS_Cast_Point( cone_pos[0] ) };
	float radii[] = { cone_radii[0], cone_radii[1] };
	HPS::CylinderKit cyl_kit;
	cyl_kit.SetPoints( 2, endPoints );
	cyl_kit.SetRadii( 2, radii );
	cyl_kit.SetCaps( HPS::Cylinder::Capping::Both );
	in_segment_key.InsertCylinder( cyl_kit );
}

logical Parse_YesNo_And_Mutate_Options_Using_Bitmask( char* in_token, unsigned long in_bitmask, unsigned long* io_options )
{
	// warning: this function will often mutate argument "token"
	char token2[1025];
	if ( 0 == strncmp( in_token, "no", 2 ) )
	{
		in_bitmask = ~in_bitmask;
		*io_options &= in_bitmask;
	} else if ( strstr( in_token, "=" ) && HPS_Parse_String( in_token, "=", -1, token2 ) )
	{
		if ( 0 == strncmp( token2, "off", 3 ) )
		{
			in_bitmask = ~in_bitmask;
			*io_options &= in_bitmask;
		} else if ( 0 == strncmp( token2, "on", 2 ) )
		{
			*io_options |= in_bitmask;
		} else if ( 0 == strncmp( token2, "1", 1 ) )
		{
			*io_options |= in_bitmask;
		} else if ( 0 == strncmp( token2, "0", 1 ) )
		{
			in_bitmask = ~in_bitmask;
			*io_options &= in_bitmask;
		} else
		{
			HPS::Database::GetEventDispatcher().InjectEvent( HPS::ErrorEvent( "HPS_Parse: bad value" ) );
			return FALSE;
		}
	} else /*plain option*/
		*io_options |= in_bitmask;
	return TRUE;
}

void		hps_acis_entity_converter::BuildSegmentPatternString( const ENTITY * in_entity, const char*in_pattern, char* out_buffer )
{
	if ( !out_buffer )
		return;
	if ( !in_pattern || !*in_pattern )
		in_pattern = m_Pattern;
	if ( !in_pattern )
	{
		*out_buffer = 0;
		return;
	}
	char	pbuffer[POINTER_BUFFER_SIZE];
	char *	buffer = out_buffer;
	char*	token = NULL;
	char	_Pattern[POINTER_BUFFER_SIZE];
	strcpy( _Pattern, in_pattern );
	token = strtok( _Pattern, "/" );
	while ( token != NULL )
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
			sprintf( buffer, "ent_%d_", in_entity->tag() );	// strcpy( buffer, ptoax( (void*)in_entityentity, pbuffer ) );
		else if ( ( strcmp( token, "type" ) == 0 ) && in_entity )
			strcpy( buffer, in_entity->type_name() );
		else
			strcpy( buffer, token );                                                                                                                                                                                                                                                                                                                                                                                                                                                     // Just accept the token as a constant
		buffer += strlen( buffer );
		*buffer++ = '/';
		token = strtok( NULL, "/" );
	}
	*--buffer = 0;                                                                                                                                                                                      // Null terminate and eliminate trailing slash
} // end of BuildSegmentPatternString

void ConvertSPAPositionToPointArray( int in_num_pts, SPAposition * in_pos, HPS::PointArray & out_PointArray, const SPAtransf* in_body_transf )
{
	if ( ( in_num_pts == 0 ) || !in_pos )
		return;
	for ( int i = 0; i < in_num_pts; ++i )
	{
		if ( in_body_transf )
			in_pos[i] *= *in_body_transf;
		out_PointArray.push_back( HPS::Point( (float)in_pos[i].x(), (float)in_pos[i].y(), (float)in_pos[i].z() ) );
	}
} // end of SPAPositionToPointArray

void ConvertFloatsToPointArray( int in_num_floats, float * in_floats, HPS::PointArray & in_point_array, SPAtransf* in_body_transf )
{
	if ( ( in_num_floats == 0 ) || !in_floats )
		return;
	for ( int i = 0; i < in_num_floats; i += 3 )
	{
		SPAposition pos( in_floats[i], in_floats[i + 1], in_floats[i + 2] );
		//if ( ( !m_ModelGeometryMode ) & bUseBodyTransf )
		if ( in_body_transf )
			pos *= *in_body_transf;
		in_point_array.push_back( HPS::Point( (float)pos.x(), (float)pos.y(), (float)pos.z() ) );
	}
} // end of ConvertFloatsToPointArray

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include "fct_utl.hxx"
bool HPS_Parse_String(
	char const *	in_string,
	char const *	in_delim,
	int				in_offset,
	char *			out_tok )
{
	return get_arguments( in_string, in_delim, in_offset, out_tok, -1 );
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void HPS_Delete_By_Key( HPS::Key in_key )
{
	in_key.Delete();
}

HPS::UTF8 HPS_Cast_UTF8( HPS::Line::SizeUnits in_size_units )
{
	HPS::UTF8 str;
	switch ( in_size_units )
	{
	case HPS::Line::SizeUnits::ScaleFactor: str = "ScaleFactor"; break;
	case HPS::Line::SizeUnits::ObjectSpace: str = "ObjectSpace"; break;
	case HPS::Line::SizeUnits::SubscreenRelative: str = "SubscreenRelative"; break;
	case HPS::Line::SizeUnits::WindowRelative: str = "WindowRelative"; break;
	case HPS::Line::SizeUnits::WorldSpace: str = "WorldSpace"; break;
	case HPS::Line::SizeUnits::Points: str = "Points"; break;
	case HPS::Line::SizeUnits::Pixels: str = "Pixels"; break;
	default: str = "";  break;
	}
	return str;
}

std::string HPS_Show_Segment( HPS::SegmentKey in_key )
{
	std::string return_value;
	if ( !HPS_Is_Valid_Segment_Key( in_key ) )
		return return_value;
	std::vector<std::string> names;
	HPS::SegmentKey owner_key = in_key;
	while ( HPS_Is_Valid_Segment_Key( owner_key ) && owner_key.HasOwner() )
	{
		owner_key = owner_key.Owner();
		names.push_back( owner_key.Name().GetBytes() );
	}
	if ( names.size() )
	{
		std::vector<std::string>::reverse_iterator it = names.rbegin();
		std::vector<std::string>::reverse_iterator it_end = names.rend()--;
		for ( ; it != it_end; ++it )
		{
			return_value.append( "/" );
			return_value.append( it->c_str() );
		}
		return_value.append( "/" );
	}
	return return_value + in_key.Name().GetBytes();
}

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

HPS::SegmentKey HPS_Open_Recurse( const char * in_pattern, HPS::SegmentKey in_segment_key = HPS_INVALID_SEGMENT_KEY )
{
	char buffer[MAX_PATTERN_SIZE];
	char * pattern = buffer;
	strcpy( pattern, in_pattern );
	HPS::SegmentKey key = in_segment_key;
	size_t buffer_length = strlen( pattern );
	if ( buffer_length && ( pattern[0] == '/' || pattern[0] == '?' ) )
	{
		pattern++;
		buffer_length--;
	}
	if ( buffer_length > 1 && strniequal( pattern, "Include Library", 15 ) )
	{
		key = HPS_Include_Library;
		pattern += 16;
		buffer_length -= 16;
	} else if ( buffer_length > 1 && strniequal( pattern, "Style Library", 13 ) )
	{
		key = HPS_Style_Library;
		pattern += 14;
		buffer_length -= 14;
	}
	if ( !HPS_Is_Valid_Segment_Key( key ) )
	{
		//sys_error( HPS_MSG_SEGMENT_DOES_NOT_EXIST );
		if ( HPS_Is_Valid_Segment_Key( in_segment_key ) )
			key = in_segment_key.Subsegment( pattern );
		else
		{
			key = HPS::Database::CreateRootSegment();
			key.SetName( in_pattern );
		}
	}
	if ( buffer_length > 0 && buffer_length < MAX_PATTERN_SIZE )
	{
		char *token = strchr( pattern, '/' );
		if ( token == NULL )
			key = key.Subsegment( pattern );
		else
		{
			*token = 0;
			key = key.Subsegment( pattern );
			key = HPS_Open_Recurse( token + 1, key );
		}
	}
	return key;
}

HPS::SegmentKey	HPS_Open_Segment( const char * in_segment_name )
{
	HPS::SegmentKey key = HPS_Open_Recurse( in_segment_name );
	return key;
}

HPS::SegmentKey HPS_Open_Segment( const char * in_pattern, HPS::SegmentKey in_segment_key )
{
	HPS::SegmentKey key = HPS_Open_Recurse( in_pattern, in_segment_key );
	return key;
}

HPS::SegmentKey HPS_Open_Segment_Key_By_Key( HPS::SegmentKey in_segment_key, char const *in_pattern )
{
	//	HPS::SegmentKey return_value = in_segment_key.Subsegment( in_pattern );
	HPS::SegmentKey return_value = HPS_Open_Recurse( in_pattern, in_segment_key );
	return return_value;
}

HPS::SegmentKey HPS_KOpen_Segment( ENTITY* in_entity, const char* in_pattern, HPS::SegmentKey in_segment_key )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	char pattern[1025] = "\0";
	HPS_Build_Segment_String( in_entity, pattern, in_pattern );
	const char * buffer = pattern;
	return HPS_Open_Segment( buffer, in_segment_key );
}

HPS::SegmentKey HPS_KCreate_Segment( ENTITY* in_entity, const char* in_pattern, HPS::SegmentKey in_segment_key )
{
	if ( in_pattern && ( in_pattern[0] == '?' || in_pattern[0] == '/' ) )
		in_pattern++;
	HPS::SegmentKey key = HPS_KOpen_Segment( in_entity, in_pattern, in_segment_key );
	return key;
}

HPS::Key HPS_Include_Segment_Key_By_Key( HPS::SegmentKey in_included, HPS::SegmentKey in_includer )
{
	HPS::Key return_value = in_includer.IncludeSegment( in_included );
	return return_value;
}

void HPS_Debug( char const * comment, HPS::SegmentKey in_segment_key )
{
	bool valid;
	float weight;
	HPS::Line::SizeUnits units;
	valid = in_segment_key.GetLineAttributeControl().ShowWeight( weight, units );
	printf( "%s line weight: \"%s\"", comment, HPS_Show_Segment( in_segment_key ).c_str() );
	if ( valid )
		printf( "   %g %s", weight, HPS_Cast_UTF8( units ).GetBytes() );
	else
		printf( " NOT SET! " );
	printf( "\n" );
}

#ifdef NT
#include <cstdint>
#else
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#define sprintf_s snprintf
#endif
#if INTPTR_MAX == INT32_MAX
static int byte_count = 8;
#elif INTPTR_MAX == INT64_MAX
static int byte_count = 16;
#else
#error "Environment not 32 or 64-bit."
#endif
option_header hoops_tree_ptrs( "hoops_tree_ptrs", false ); // If true, HOOPS segment names use an index rather than entity_ptrs.

void HPS_Print_Portfolio( FILE * in_file_pointer, HPS::PortfolioKey in_portfolio_key )
{
	if ( !HPS_Is_Valid_Key( in_portfolio_key ) )
		return;
	if ( in_file_pointer == NULL )
		in_file_pointer = stdout;
	printf( "\t Portfolio = 0x%p", in_portfolio_key.GetInstanceID() );
	// Display all defined glyphs...
		HPS::GlyphDefinitionArray glyphs;
	in_portfolio_key.ShowAllGlyphDefinitions( glyphs );
	for ( auto const glyph : glyphs )
		fprintf( in_file_pointer, "\t Glyph=%s,", glyph.Name().GetBytes() );
		// Display all defined line patterns...
		HPS::LinePatternDefinitionArray line_patterns;
	in_portfolio_key.ShowAllLinePatternDefinitions( line_patterns );
	for ( auto const line_pattern : line_patterns )
		fprintf( in_file_pointer, "\t Line=%s,", line_pattern.Name().GetBytes() );
		// Display all defined images...
		HPS::ImageDefinitionArray images;
	in_portfolio_key.ShowAllImageDefinitions( images );
	for ( auto const image : images )
		fprintf( in_file_pointer, "\t Image=%s,", image.Name().GetBytes() );
	printf( "\n" );
	}

void HPS_Print_Portfolio( FILE * in_file_pointer, HPS::SegmentKey in_segkey )
{
	HPS::PortfolioKeyArray portfolio_keys;
	in_segkey.GetPortfolioControl().Show( portfolio_keys );
	if ( portfolio_keys.size() )
		for ( auto portfolio_key : portfolio_keys )
			HPS_Print_Portfolio( in_file_pointer, portfolio_key );
}

// Prints to in_file_pointer.  The base_key argument is where we start;
// in_are_only_leaves_printed indicates whether to print the entire tree, or only its
// leaves; the level argument is used in recursion, and is set to zero
// by default.
void traverse_tree(
	FILE *			in_file_pointer,
	HPS::SegmentKey	in_segmentKey,
	const char *	in_base_name,
	std::vector<void*>& in_hex_values )
{
	const char *sep = "/";
	size_t sep_length = strlen( sep );

	//printf( "ID=0x%p: ", in_segmentKey.GetInstanceID() );
	const char* name = in_segmentKey.Name();
	size_t name_length = strlen( name );
	if ( name_length == 0 )
	{	// Print out something
		name = "null"; name_length = 4;
	}
	//	if ( strequal( name, "geometry" ) || strequal( name, "segment" ) )
	//		return;
	size_t buffer_length = strlen( in_base_name ) + sep_length + name_length;
	char * buffer = new char[buffer_length + 1]; // add for trailing zero
	sprintf_s( buffer, buffer_length + 1, "%s%s%s", in_base_name, sep, name );
	//_strlwr( buffer );
	HPS::Type type = in_segmentKey.Type();
	if ( in_segmentKey.Type() == HPS::Type::Model )
		int x = 5;
	HPS::SearchResults results;
	size_t all_count = in_segmentKey.Find( HPS::Search::Type::Everything, HPS::Search::Space::SegmentOnly, results );
	//	if ( !in_are_only_leaves_printed )
	size_t start_pos = 0;
	std::string base_string = buffer;
	char x_or_amp = 'X';
	if ( hoops_tree_ptrs.on() )	for ( int counter = 4; counter; counter-- ) // Bail after 4 tries
	{
		size_t start_pos = 0;
		int bytes_to_replace = byte_count + 2;
		start_pos = base_string.find( "0x0", 0 );
		if ( start_pos == std::string::npos )
		{
			start_pos = base_string.find( "ent_", 0 );
			if ( start_pos == std::string::npos )
				break;
//			base_string.replace( start_pos, 4, "E_" );
			continue;
		}
		void* hex = axtop( const_cast<char*>( base_string.substr( start_pos + bytes_to_replace - byte_count - 2, bytes_to_replace ).c_str() ) ); // axtop skips the "0x"
		int index = 0;
		std::vector<void*>::iterator it;
		for ( it = in_hex_values.begin(); it != in_hex_values.end(); ++it, index++ )
			if ( *it == hex )
				break;
		if ( it == in_hex_values.end() )
			in_hex_values.push_back( hex );
		char buff[50];
		sprintf_s( buff, 50, "%c_%d", x_or_amp, index );
		base_string.replace( start_pos, bytes_to_replace, buff );
	}
	fprintf( in_file_pointer, "%s\\   ", base_string.c_str() );

	HPS::StyleKeyArray style_key_array;
	in_segmentKey.GetStyleControl().Show( style_key_array );
	if ( style_key_array.size() > 0 )
	{
		fprintf( in_file_pointer, "(style:" );
		for ( auto key_it = style_key_array.begin(), e = style_key_array.end(); key_it != e; ++key_it )
		{
			if ( key_it != style_key_array.begin() )
				fprintf( in_file_pointer, "," );
			HPS::StyleKey style_key = ( HPS::StyleKey )( *key_it );
			HPS::Style::Type type;	HPS::SegmentKey segment;	HPS::UTF8 name;
			style_key.ShowSource( type, segment, name );
			if ( type == HPS::Style::Type::Segment )
				fprintf( in_file_pointer, segment.Name().GetBytes() );
			if ( type == HPS::Style::Type::Named )
				fprintf( in_file_pointer, name.GetBytes() );
		}
		fprintf( in_file_pointer, ") " );
	}

	HPS::VisibilityControl visibility_control = in_segmentKey.GetVisibilityControl();
	HPS::VisibilityKit vis_kit;
	if ( in_segmentKey.ShowVisibility( vis_kit ) )
	{
		bool okay = true, faces_vis, lines_vis, markers_vis, vertices_vis, text_vis, edges_vis;
		okay &= vis_kit.ShowFaces( faces_vis );
		okay &= vis_kit.ShowLines( lines_vis );
		okay &= vis_kit.ShowMarkers( markers_vis );
		okay &= visibility_control.ShowVertices( vertices_vis );
		okay &= vis_kit.ShowGenericEdges( edges_vis );
		okay &= vis_kit.ShowText( text_vis );
		if ( okay )
			fprintf( in_file_pointer, " (vis:%c%c%c%c%c%c) ",
			faces_vis ? 'F' : 'f', lines_vis ? 'L' : 'l', markers_vis ? 'M' : 'm',
			vertices_vis ? 'V' : 'v', edges_vis ? 'E' : 'e', text_vis ? 'T' : 't' );
	}

	HPS::Material::Type mat_type;
	HPS::MaterialKit mat_kit;
	HPS::RGBAColor color;
	float mat_index;
	if ( in_segmentKey.GetMaterialMappingControl().ShowFaceMaterial( mat_type, mat_kit, mat_index ) && !mat_kit.Empty() && mat_kit.ShowDiffuseColor( color ) )
		fprintf( in_file_pointer, "(face<R=%g G=%g B=%g>) ", color.red, color.green, color.blue );
	if ( in_segmentKey.GetMaterialMappingControl().ShowMarkerColor( mat_type, color, mat_index ) && mat_type == HPS::Material::Type::RGBAColor )
		fprintf( in_file_pointer, "(marker<R=%g G=%g B=%g>) ", color.red, color.green, color.blue );
	if ( in_segmentKey.GetMaterialMappingControl().ShowLineColor( mat_type, color, mat_index ) && mat_type == HPS::Material::Type::RGBAColor )
		fprintf( in_file_pointer, "(line<R=%g G=%g B=%g>) ", color.red, color.green, color.blue );
	if ( in_segmentKey.GetMaterialMappingControl().ShowTextColor( mat_type, color, mat_index ) && mat_type == HPS::Material::Type::RGBAColor )
		fprintf( in_file_pointer, "(text<R=%g G=%g B=%g>) ", color.red, color.green, color.blue );
	if ( in_segmentKey.GetMaterialMappingControl().ShowEdgeMaterial( mat_type, mat_kit, mat_index ) && !mat_kit.Empty() && mat_kit.ShowDiffuseColor( color ) )
		fprintf( in_file_pointer, "(edge<R=%g G=%g B=%g>) ", color.red, color.green, color.blue );

	HPS::ConditionControl cond_control = in_segmentKey.GetConditionControl();
	if ( !cond_control.Empty() && cond_control.GetCount() != 0 )
	{
		HPS::UTF8Array conds;
		cond_control.ShowConditions( conds );
		fprintf( in_file_pointer, "(conds:" );
		size_t size = cond_control.GetCount();
		for ( size_t i = 0; i < size; i++ )
			fprintf( in_file_pointer, "%s%s", conds[i].GetBytes(), i + 1 < size ? "," : "" );
		fprintf( in_file_pointer, ") " );
	}

	HPS::MatrixKit matrix_kit;
	if ( in_segmentKey.ShowModellingMatrix( matrix_kit ) )
		fprintf( in_file_pointer, "(modelling matrix) " );

	if ( strcmp(name, "Scene") == 0 )
		{
		HPS::KeyPath key_path;
		HPS::VisibilityKit vis_kit;
		if ( key_path.ShowNetVisibility( vis_kit ) )
		{
			bool okay = true, faces_vis, lines_vis, markers_vis, vertices_vis, text_vis, edges_vis;
			okay &= vis_kit.ShowFaces( faces_vis );
			okay &= vis_kit.ShowLines( lines_vis );
			okay &= vis_kit.ShowMarkers( markers_vis );
			okay &= visibility_control.ShowVertices( vertices_vis );
			okay &= vis_kit.ShowGenericEdges( edges_vis );
			okay &= vis_kit.ShowText( text_vis );
			if ( okay )
				fprintf( in_file_pointer, " (vis:%c%c%c%c%c%c) ",
					faces_vis ? 'F' : 'f', lines_vis ? 'L' : 'l', markers_vis ? 'M' : 'm',
					vertices_vis ? 'V' : 'v', edges_vis ? 'E' : 'e', text_vis ? 'T' : 't' );
		}
	}
	// HPS::SceneTree scene_tree; // How can I use this??????????????????????????????????????????????????????

	size_t total_face_count = 0;		// a count of HOOPS shells
	size_t total_facet_count = 0;		// a count of HOOPS faces
	size_t total_polyline_count = 0;	// a count of HOOPS lines
	size_t total_marker_count = 0;		// a count of HOOPS markers
	size_t total_point_count = 0;		// a count of HOOPS points
	size_t total_unknown = 0;			// a count of unknowns
	if ( all_count >= 1 )
	{
		int inc_count = 0;
		HPS::SearchResultsIterator it = results.GetIterator();
		size_t result_iter;
		for ( result_iter = 0; result_iter < all_count; result_iter++ )
		{	// Print out key info attached to in_segmentKey...
			HPS::Key key = it.GetItem();
			HPS::Type type = key.Type();
			if ( key.Type() == HPS::Type::ShellKey )
			{
				HPS::ShellKit kit;
				HPS::ShellKey( key ).Show( kit );
				HPS::PointArray face_points;
				size_t other_point_count = kit.GetPointCount(); // ??
				if ( kit.ShowPoints( face_points ) )
					total_point_count += face_points.size();
				total_face_count++;
				total_facet_count += kit.GetFaceCount();
			} else if ( key.Type() == HPS::Type::LineKey )
			{
				total_polyline_count++;
			} else if ( key.Type() == HPS::Type::MarkerKey )
			{
				total_marker_count++;
			} else if ( key.Type() == HPS::Type::IncludeKey )
			{
				//if ( ( ++inc_count ) % 2 == 0 )
				//	fprintf( in_file_pointer, "\n\t" );
				fprintf( in_file_pointer, "\n\t\tInc\"%s\" ", HPS_Show_Segment( HPS::IncludeKey( key ).GetTarget() ).c_str() );
			} else if ( key.Type() == HPS::Type::SegmentKey )
			{
				// Gets printed in a recursive call.  
			} else if ( key.Type() == HPS::Type::ReferenceKey )
			{
				HPS::ReferenceKey ref_key( key );
				HPS::Type type = ref_key.GetTarget().Type();
				if ( type == HPS::Type::SegmentKey )
					fprintf( in_file_pointer, "Ref_Seg\"%s\" ", HPS_Show_Segment( ( HPS::SegmentKey )ref_key.GetTarget() ).c_str() );
				else if ( type == HPS::Type::ShellKey )
				{
					HPS::ShellKey shell_key( ref_key.GetTarget() );
					fprintf( in_file_pointer, "Ref_Shell=0x%p ", shell_key.GetInstanceID() );
				} else if ( type == HPS::Type::GeometryKey )
					fprintf( in_file_pointer, "Ref_Geom=0x%p ", ref_key.GetTarget().GetInstanceID() );
			else
					fprintf( in_file_pointer, "Ref_Type=0x%x ", key.Type() );
			} else if ( key.Type() == HPS::Type::TextKey )
			{
				HPS::UTF8 text;
				( ( HPS::TextKey )key ).ShowText( text );
				fprintf( in_file_pointer, "Text\"%s\" ", text.GetBytes() );
			} else if ( key.Type() == HPS::Type::DistantLightKey )
			{
				fprintf( in_file_pointer, "DistantLightKey " );
			} else
			{
				fprintf( in_file_pointer, "Type=0x%x ", key.Type() );
				total_unknown++;
			}
			it.Next();
		}
	}
	if ( total_unknown && ( total_face_count || total_facet_count || total_marker_count || total_polyline_count ) )
		for ( size_t ii = 0; ii < total_unknown; ii++ ) 
			fprintf( in_file_pointer, "? " );
	if ( total_face_count )
		fprintf( in_file_pointer, "(%ld_face%s) ", total_face_count, total_face_count == 1 ? "" : "s" );
	if ( total_facet_count )
		fprintf( in_file_pointer, "(%ld_facet%s) ", total_facet_count, total_facet_count == 1 ? "" : "s" );
	if ( total_marker_count )
		fprintf( in_file_pointer, "(%ld_marker%s) ", total_marker_count, total_marker_count == 1 ? "" : "s" );
	if ( total_point_count )
		fprintf( in_file_pointer, "(%ld_point%s) ", total_point_count, total_point_count == 1 ? "" : "s" );
	if ( total_polyline_count )
		fprintf( in_file_pointer, "(%ld_line%s) ", total_polyline_count, total_polyline_count == 1 ? "" : "s" );

	HPS::PortfolioKeyArray portfolio_keys;
	if ( in_segmentKey.GetPortfolioControl().Show( portfolio_keys ) )
	{
		fprintf( in_file_pointer, "Portfolios:" );
		for ( auto portfolio_key : portfolio_keys )
			fprintf( in_file_pointer, "0x%p,", portfolio_key.GetInstanceID() );
		fprintf( in_file_pointer, " " );
	}
	HPS::ReferenceKeyArray ref_keys;
	in_segmentKey.ShowReferrers( ref_keys );
	for ( auto const & key : ref_keys )
	{
		fprintf( in_file_pointer, "Referrers:" );
		HPS::IncludeKey ref_key( key );
		printf( "0x%p,", ref_key.GetInstanceID() );
	}

	fprintf( in_file_pointer, "\n" );

	if ( all_count >= 1 )
	{
		HPS::SearchResultsIterator it = results.GetIterator();
		size_t result_iter;
		for ( result_iter = 0; result_iter < all_count; result_iter++ )
		{	// Recurse one branch deeper...
			HPS::Key key = it.GetItem();
			HPS::Type type = key.Type();
			if ( key.Type() == HPS::Type::SegmentKey && !key.Empty() && key.GetInstanceID() != in_segmentKey.GetInstanceID() )
			{
				if ( HPS::SegmentKey( key ).Name() == "hps_navigation" )
					fprintf( in_file_pointer, "skip_navigation_tree " );
				else
				traverse_tree( in_file_pointer, HPS::SegmentKey( key ), buffer, in_hex_values );
			}
			it.Next();
		}
	}
	delete[] buffer;
	return;
}

void HPS_Print_Segment_Tree( HPS::SegmentKey in_key, FILE * in_file_pointer )
{
	std::vector<void*> hex_values;
	HPS::UTF8 debug_str = in_key.Name();
	if ( ( HPS_Is_Valid_Segment_Key( in_key ) ) )
		traverse_tree( in_file_pointer, in_key, "", hex_values );
}

void HPS_Print_Segment_Tree( const char* in_base_seg, FILE * in_file_pointer )
{
	HPS::SegmentKeyArray roots = HPS::Database::GetRootSegments();
	size_t len = strlen( in_base_seg );
	if ( len == 0 || ( len == 1 && in_base_seg[0] == '/' ) )
	{
		auto it = roots.begin();
		auto const end = roots.end();
	for ( ; it != end; ++it )
			HPS_Print_Segment_Tree( *it, in_file_pointer );
		return;
	}
	char * node_names = new char[len + 1];
#ifdef NT
	memcpy_s( node_names, len + 1, in_base_seg, len + 1 );
#else
	memcpy( node_names, in_base_seg, len + 1 );
#endif
	char * next_token = NULL, *seps = "/?";
#ifdef NT
	char * token = strtok_s( node_names, seps, &next_token );
	for ( ; token != NULL; token = strtok_s( NULL, seps, &next_token ) )
#else
	char * token = strtok_r( node_names, seps, &next_token );
	for ( ; token != NULL; token = strtok_r( NULL, seps, &next_token ) )
#endif
	{
		auto root_it = roots.begin();
		auto const end = roots.end();
		for ( ; root_it != end; ++root_it )
		{
			std::vector<void*> hex_values;
			HPS::SegmentKey root = *root_it;
			HPS::UTF8 debug_str = root.Name();
			if ( striequal( root.Name().GetBytes(), token ) )
	{
				if ( root.HasOwner() )
					traverse_tree( in_file_pointer, root, HPS_Show_Segment( root.Owner() ).c_str(), hex_values );
				else
					HPS_Print_Segment_Tree( root, in_file_pointer );
			}
			HPS::SearchResults results;
			size_t result_count = root.Find( HPS::Search::Type::Segment, HPS::Search::Space::Subsegments, results );
			HPS::SearchResultsIterator seg_it = results.GetIterator();
			size_t result_iter;  for ( result_iter = 0; result_iter < result_count; result_iter++ )
			{
				HPS::SegmentKey key = HPS::SegmentKey( seg_it.GetItem() );
				if ( HPS_Is_Valid_Segment_Key( key ) && key.HasOwner() )
				{
					if ( striequal( key.Name().GetBytes(), token ) )
					{
						if ( key.HasOwner() )
							traverse_tree( in_file_pointer, key, HPS_Show_Segment( key.Owner() ).c_str(), hex_values );
						else
							HPS_Print_Segment_Tree( key, in_file_pointer );
					}
				}
				seg_it.Next();
			}
		}
	}
	delete[] node_names;
}

char* HPS_stristr( const char* in_string, const char* in_substring )
{
	do
	{
		const char* h = in_string;
		const char* n = in_substring;
		while ( tolower( (unsigned char)*h ) == tolower( (unsigned char)*n ) && *n )
		{
			h++;
			n++;
		}
		if ( *n == 0 )
			return (char *)in_string;
	} while ( *in_string++ );
	return 0;
}

#ifdef _MSC_VER
#define HPS_MACRO_DEBUG_DELETES
#ifdef HPS_MACRO_DEBUG_DELETES
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
void DbgPrintf(
	const char* fmt, // Format string (printf style).
	...              // Variable number of arguments.
	)
{
	va_list marker;
	char szBuf[256];
	va_start( marker, fmt );
	vsprintf( szBuf, fmt, marker );
	va_end( marker );
#ifdef NT
	OutputDebugString( szBuf );
	//OutputDebugString( TEXT( "\r\n" ) );
#else
	fprintf( stderr, "%s", szBuf );
#endif
}
#define DBGPRINTF(s, ...) { DbgPrintf( s, __VA_ARGS__ ); }
#else
#define DBGPRINTF(s, ...) { }
#endif
#endif

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

//#include <iomanip>
//#include <sstream>
//
//char * HPS_Show_Segment( HPS::SegmentKey in_segment_key  /*, size_t in_string_size, char * out_string */ )
//{
//	HPS::KeyPath selectionPath;
//	selectionPath.Append( in_segment_key );
//	HPS::KeyArray keys;
//	selectionPath.ShowKeys( keys );
//	size_t count = keys.size( );
//	count = 0;
//	std::stringbuf buf;
//	std::ostream os( &buf );
//	for ( auto key_it = keys.begin( ),	e = keys.end( ); key_it != e; ++key_it )
//	{
//		HPS::Key key = ( *key_it );
//		if ( key.Type( ) == HPS::Type::SegmentKey )
//		{
//			HPS::SegmentKey segment_key = ( HPS::SegmentKey )key;
//			if ( count++ != 0 )
//				os << "~";
//			if ( segment_key.Name( ).GetLength( ) )
//				os << segment_key.Name( );
//			segment_key.Name( );
//		}
//	}
//	//strncpy_s( out_string, in_string_size, buf.str( ).c_str( ), buf.str( ).size( ) );
//	//printf( "string = \"%s\"\n", out_string );
//	//HPS::VisibilityKit vis_kit;
//	//selectionPath.ShowNetVisibility( vis_kit );
//	return out_string;
//}

/* Some helpful debugging statements to cut and paste...

#ifdef _MSC_VER
#include <windows.h>
#define STRING_MSG "@@@@@                                  Revisit: whatever"
#pragma message(STRING_MSG)
static bool output_it = true; if ( output_it ) { OutputDebugString( STRING_MSG ); OutputDebugString( "\n" ); output_it = false; }
#endif

#pragma message( "@@@@@                                  Revisit: whatever" )

printf( "\n\n\nHoopsView::HoopsView::Init 1\n" );	HPS_Print_Segment_Tree( "Include Library", TRUE ); // acis apoints

HPS::Layout layout( view.GetOwningLayouts()[0] );
HPS::LayoutArray layout_array = view.GetOwningLayouts();
layout_array.size();
canvas.AttachLayout( layout );
canvas.AttachViewAsLayout( view );
canvas.GetAttachedLayout();
canvas.GetAttachedLayoutIncludeLink();
canvas.GetPortfolioKey();
layout.AttachViewBack( view );
layout.AttachViewFront( view );
layout.GetAttachedView( 9 );
layout.GetAttachedViewIncludeLink( 9 );
layout.GetFrontView();
layout.GetLayerCount();
layout.GetOwningCanvases();

HPS::KeyPath keypath;
keypath += view.GetAttachedModel().GetSegmentKey();
keypath += view.GetAttachedModelIncludeLink();
keypath += view.GetSegmentKey();
keypath += view.GetOwningLayouts()[0].GetAttachedViewIncludeLink( 0 );
keypath += view.GetSegmentKey();

HPS::LayoutArray layouts = view.GetOwningLayouts();
HPS::CanvasArray canvases;
for ( auto const & layout : layouts )
{
HPS::CanvasArray some_canvases = layout.GetOwningCanvases();
canvases.insert( canvases.end(), some_canvases.begin(), some_canvases.end() );
}
for ( auto & canvas : canvases )
canvas.GetWindowKey();

canvases.push_back( canvas );

HPS::SprocketPath sprkPath( canvas, canvas.GetAttachedLayout(),
canvas.GetAttachedLayout().GetAttachedView(),
canvas.GetAttachedLayout().GetAttachedView().GetAttachedModel() );

HPS_Get_Current_View().Update( HPS::Window::UpdateControl::DefaultUpdate );
if ( !HPS_Get_Current_View().Empty() )		HPS_Get_Current_View().GetOwningLayouts()[0].GetOwningCanvases()[0].UpdateWithNotifier().Wait();
HPS_Get_Current_View().GetOwningLayouts()[0].GetOwningCanvases()[0].UpdateWithNotifier().Wait();

printf( "FUNCTION_XXX(): SEGMENT_KEY=\"%s\"\n", HPS_Show_Segment( segment_key ).c_str( ) );

char buffer[200];
sprintf( buffer, "HPS_Render_Entity(): entity_key=\"%s\"\n", HPS_Show_Segment( entity_key ).c_str() );
OutputDebugString( buffer );

*/
