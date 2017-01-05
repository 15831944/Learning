/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: ha_util.cpp,v 1.6 2002/07/18 22:58:35 jhauswir Exp $

#include <vector>
#include <string>
//	Required for all ACIS functions
#include "acis.hxx"
#include "hc.h"
#include "ha_util.h"
#include "rgbcolor.hxx"
#include "rnd_api.hxx"
#include "ha_bridge.h"
#include "ha_rend_options.h"
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

void MapColorToString(const rgb_color &color, char* colorName)
{
	if (colorName)
		sprintf(colorName, "%3i%3i%3i\0", (int)(color.red()*100.0),(int)(color.green()*100.0),(int)(color.blue()*100.0));
}

void transf_to_matrix(float *float_mat,const SPAtransf &tform)
{
	SPAmatrix mat = tform.affine();

	float_mat[0] = (float)(mat.element( 0, 0) * tform.scaling());
	float_mat[1] = (float)(mat.element( 0, 1) * tform.scaling());
	float_mat[2] = (float)(mat.element( 0, 2) * tform.scaling());
	float_mat[3] = (float)(0.0);
	float_mat[4] = (float)(mat.element( 1, 0) * tform.scaling());
	float_mat[5] = (float)(mat.element( 1, 1) * tform.scaling());
	float_mat[6] = (float)(mat.element( 1, 2) * tform.scaling());
	float_mat[7] = (float)(0.0);
	float_mat[8] = (float)(mat.element( 2, 0) * tform.scaling());
	float_mat[9] = (float)(mat.element( 2, 1) * tform.scaling());
	float_mat[10] = (float)(mat.element( 2, 2) * tform.scaling());
	float_mat[11] = (float)(0.0);

	SPAvector trans = tform.translation();
	float_mat[12] = (float)(trans.x());
	float_mat[13] = (float)(trans.y());
	float_mat[14] = (float)(trans.z());
	
	float_mat[15] = (float)(1.0);
}

void ConvertSPAPositionToFloatArray(int num_pts, SPAposition *pos, float *&float_points)
{
	API_SYS_BEGIN
	if (num_pts>0)
	{
		float_points=ACIS_NEW float[num_pts*3];

		for (int i=0;i<num_pts;i++)
		{
			float_points[i*3]  =(float)pos[i].x();
			float_points[i*3+1]=(float)pos[i].y();
			float_points[i*3+2]=(float)pos[i].z();
		}
	}
	API_SYS_END
}

void ConvertDoubleToFloatArray(int num_pts, double *d, float *&float_points)
{
	API_SYS_BEGIN
	if (num_pts>0 && d)
	{
		float_points=ACIS_NEW float[num_pts];

		for (int i=0;i<num_pts;i++)
		{
			float_points[i] = (float)d[i];
		}
	}
	API_SYS_END
}

char* ptoax(void* SPAptr, char* buffer)
{
	buffer[POINTER_BUFFER_SIZE-1] = 0;				// Null terminate
	char* cptr = &buffer[POINTER_BUFFER_SIZE-2];	// Address of lowest order nyble of result
	uintptr_t p = (uintptr_t) SPAptr;

	for (int j=0;j<POINTER_BUFFER_SIZE-1;j++)
	{
		buffer[j]='0';
	}
	buffer[1]='x';

	if (p)
	{
		while(p)
		{
			char c = (char) ((p & 0xF) + '0');
			if(c > '9') c += ('a'-'9')-1;
			*cptr-- = c;
			p /= 0x10;
		}
	}

	return buffer;
}

// Convert ascii in 0x format to a pointer
void* axtop(char* buffer)
{
	uintptr_t p = 0;
	char* cptr = &buffer[2];						// Skip the 0x
	
	while(*cptr) 
	{
		p *= 0x10;
		uintptr_t nyble = *cptr++ - '0';
		if(nyble > 9) 
			nyble -= ('a'-'9')-1;
		if(nyble > 0xF) 
			nyble -= (uintptr_t)(('A'-'a'));		// Just in the case someone strupper'd it.
		p |= nyble;
	}

	return (void*) p;
}

void OpenColorSegment(const rgb_color &color)
{
	char segmentName[10];
	MapColorToString(color, segmentName);
	HC_Open_Segment(segmentName);
	HC_Set_Color_By_Value("everything", "RGB", (float)color.red(), (float)color.green(), (float)color.blue());
}

logical OpenColorSegment(ENTITY* entity,logical traverse_up)
{
	logical color_exists=FALSE;
	logical texture_exists=FALSE;
	logical material_properties_exist=FALSE;

	char segmentName[100];
	rgb_color color;

	ATTRIB * rh_material = find_attrib( entity, ATTRIB_RH_TYPE, ATTRIB_RENDER_TYPE );
	double	transparency=0.0;
	logical reflection_state = FALSE;
	double	reflect_ambient=0.0;
	double	reflect_diffuse=0.0;
	double	reflect_specular=0.0;
	double	reflect_exponent=0.0;

	if (rh_material)
	{
		material_properties_exist = TRUE;

		RH_ENTITY_PROPS new_props;
		new_props.material      = (( ATTRIB_RENDER *)(rh_material))->material();
		new_props.sides         = (( ATTRIB_RENDER *)(rh_material))->sides();
		new_props.texture_space = (( ATTRIB_RENDER *)(rh_material))->texture_space(); 
		new_props.local_transf  = (( ATTRIB_RENDER *)(rh_material))->local_transf(); 
		new_props.local_transf_modified  = (( ATTRIB_RENDER *)(rh_material))->local_transf_modified(); 

	    //
	    // if the entity has sidedness attached ( non 0 ), then used that 
	    //
	    //if ( new_props.sides )
		//	mp->set_NumSides(new_props.sides);

		if (new_props.material)
		{
			// Guy. Addresses concerns posed in BTS 87839. Use backplane culling instead of no polygon handedness
			if (new_props.sides>1)
				HC_Set_Heuristics( "no backplane cull" );

			// base color
			double	color_red=0.0;
			double	color_green=0.0;
			double	color_blue=0.0;
			// Now grab any settings that are present.  If the material does
			// not have a setting for any of these the procedures don't modify
			// the data, so the default will stick. 
			logical has_material_color = FALSE;
			rh_get_material_color( new_props.material, color_red, color_green, color_blue, has_material_color );
			//mp->set_BaseColor(rgb_color(color_red,color_green,color_blue));

			// reflectance parameters
			rh_get_reflect_status( new_props.material, reflection_state );
			rh_get_material_reflection(	new_props.material, reflect_ambient, reflect_diffuse, reflect_specular, reflect_exponent );
				
			// transparency value
			rh_get_material_transparency( new_props.material, transparency );

			//if (transparency!=1.0)
			//	mp->set_Transparency(transparency);

			//if (new_props.sides > 1)
			//	mp->set_NumSides( new_props.sides);

			// texture name
			char *texture_name=0;
			rh_get_material_texture_name( new_props.material, (const char *&)texture_name );

			//texture_name=0;
			// There can be a texture only if there is a texture file
			// and we care about this only if we are processing a mesh/face
			if ( texture_name )
			{
				texture_exists=TRUE;
				// The local SPAtransf should only be used if there is a texture map.
				//if (new_props.local_transf && !new_props.local_transf->identity())
				//	mp->set_LocalTransform(new_props.local_transf);

		
				//tex->set_TextureName(texture_name);
				//HC_Open_Segment(texture_name);
				// Load up the HOOPS image.
				// We are assuming a *.bmp file.  That's all GI/GL handled.
				size_t len = strlen(texture_name);
				char * hoops_name = (char *)ACIS_ALLOCA( sizeof(char) * (len+1) );
				strcpy( hoops_name, texture_name );

				for (unsigned int i=0; i<len; i++)
				{
					if (hoops_name[i]=='/' || hoops_name[i]=='\\')
						hoops_name[i]='_';
				}
				HC_Open_Segment(hoops_name);
				
				logical loaded_texture=FALSE;
#ifdef NT
				loaded_texture=LoadBMP(texture_name,hoops_name);
#endif // NT

				if ( new_props.texture_space )
				{
					//texture space 
					//RH_TEXTURE_SPACE *texture_space=0;
					const char *texture_space_name=0;
					int texture_space_nargs=0;
					const char **texture_space_arg_names=0;
					Render_Arg * texture_space_arg_vals=0;

					rh_get_texture_space_args(  new_props.texture_space,
												texture_space_name,
												texture_space_nargs,
												texture_space_arg_names,
												texture_space_arg_vals);

					if (!strcmp(texture_space_name, "zebra"))
					{
						char str[_MAX_PATH+200];
						//sprintf(str,"source =%s, layout=spherical, parameterization source = reflection vector",hoops_name);
						sprintf(str,"source =%s, transform=., parameterization source = sphere",hoops_name);
						//int u=0, v=-30, w=0;
						//HC_Rotate_Texture(u, v, w);
					    HC_Define_Texture (hoops_name, str);
					}

					//tex->set_TextureSpaceName( _TEMP_BSTR(texture_space_name));
					//for (int i = 0; i < texture_space_nargs; i++)
					//	tex->AddTextureSpaceArgName(_TEMP_BSTR(texture_space_arg_names[i]));
					//tex->set_TextureSpaceArgVals( texture_space_arg_vals);
					char color_str[_MAX_PATH+10];
					//sprintf(color_str,"faces =%s",hoops_name);
					sprintf(color_str, "faces=(diffuse=%s, mirror=white)", hoops_name); 
					//sprintf(color_str, "faces=(environment=%s)", hoops_name);
					if (loaded_texture)
					{
						if ( rh_material && (( ATTRIB_RENDER *)(rh_material))->material() && reflection_state )  {
							sprintf( color_str, "Faces = (ambient=(r=%g g=%g b=%g), diffuse=%s, specular=(r=%g g=%g b=%g), gloss=4.1%lf)",
								reflect_ambient, reflect_ambient, reflect_ambient,
								hoops_name, reflect_specular, reflect_specular, reflect_specular, reflect_exponent );
						}
						HC_Set_Color(color_str);
					}
				}
				else
				{

					char color_str[_MAX_PATH+10];
					sprintf(color_str,"faces =%s",hoops_name);
					if (loaded_texture)
					{
						if ( rh_material && (( ATTRIB_RENDER *)(rh_material))->material() && reflection_state )  {
							sprintf( color_str, "Faces = (ambient=(r=%g g=%g b=%g), diffuse=%s, specular=(r=%g g=%g b=%g), gloss=4.1%lf)",
								reflect_ambient, reflect_ambient, reflect_ambient,
								hoops_name, reflect_specular, reflect_specular, reflect_specular, reflect_exponent );
						}
						HC_Set_Color(color_str);
					}
				}
				if ( 0 < transparency && transparency <= 1.0)
				{
					char str[ 500 ];
					sprintf( str, "faces = (transmission = (r=%f g=%f b=%f))", transparency, transparency, transparency );
					HC_Set_Color( str );
				}
			}
		}
	}

	check_outcome( api_rh_get_entity_rgb(entity, color, traverse_up, color_exists) );
 
	if ( (color_exists || material_properties_exist) && !texture_exists )
	{
		if ( !color_exists )
			check_outcome( api_rh_get_entity_rgb( entity, color, TRUE, color_exists ) );
		char str[500];
		MapColorToString(color, segmentName);
		sprintf(str,"%s %3i",segmentName,(int)(100.0*transparency));
		
		HA_Point rgb( color.red(), color.green(),color.blue() );

		HC_Open_Segment(str);
		HC_Set_Color_By_Value("geometry", "RGB", color.red(), color.green(), color.blue());

		// Guy. This recognizes reflect_diffuse and reflect_specular. However, reflect_ambient and reflect_exponenet still don't work in HOOPS.
		if ( rh_material && (( ATTRIB_RENDER *)(rh_material))->material() && reflection_state )  {
			sprintf( str, "Faces = (ambient=(r=%g g=%g b=%g), diffuse=(r=%g g=%g b=%g), specular=(r=%g g=%g b=%g), gloss=4.1%lf)",
				reflect_ambient, reflect_ambient, reflect_ambient, 
				reflect_diffuse * color.red(), reflect_diffuse * color.green(), reflect_diffuse * color.blue(), 
				reflect_specular, reflect_specular, reflect_specular, reflect_exponent ); 
			HC_Set_Color( str );
		}

		ha_rendering_options ha_opts = HA_Get_Rendering_Options();
		if ( ha_opts.GetFacetStyle() == 1 ) {
			// set the color of facet lines to be the complementary color of the face when the shading is on
			char visibility[32] = "";
			HC_KEY view_key = HA_Get_Current_View_Segment();
			if( view_key )
			{
				HC_Open_Segment_By_Key( view_key );
					HC_Show_One_Visibility( "faces", visibility );
				HC_Close_Segment();
			}

			HA_Point hsv;
			logical okay;
			if( STRICMP( visibility, "off" ) != 0 )
			{	// Facet lines are displayed with shading...
				// If the hue is any shade of grey, use white or black for the facet lines.
				// Else, use the complement shade of the hue.
				if ( (fabs( color.red() - color.green() ) <= 0.07 )
				  && (fabs( color.red() - color.blue() ) <= 0.07 )
				  && (fabs( color.green() - color.blue() ) <= 0.07 ) )
				{	// this is a shade of grey...use black or white facet lines
					if ( color.red() < 0.3 ) // 0.5 was the first choice, but white lines get lost in the light reflections
					{
						HA_Point temp_rgb( 1.0, 1.0, 1.0 ); // white
						okay = HC_Compute_Color_By_Value( "RGB", temp_rgb, "HSV", hsv ); 
					} else {
						HA_Point temp_rgb( 0.0, 0.0, 0.0 ); // black
						okay = HC_Compute_Color_By_Value( "RGB", temp_rgb, "HSV", hsv ); 
					}
				} else
				{	// this is not a shade of grey, use the complementary hue for facet lines
					okay = HC_Compute_Color_By_Value( "RGB", rgb, "HSV", hsv ); 
				}
				hsv.Set( (float) ((((int)hsv.x()) + 180 ) % 360), hsv.y(), hsv.z() );
			} else // Facet lines are displayed without shading...use the face color for facet lines
				okay = HC_Compute_Color_By_Value( "RGB", rgb, "HSV", hsv ); 

			if (okay)
				HC_Set_Color_By_Value("edges", "HSV", hsv.x(), hsv.y(), hsv.z() );
			else
				HC_Set_Color_By_Value("edges", "RGB", color.red(), color.green(), color.blue());
		}
			
		// I don't like how I get the value for transparency, but....
		if ( 0 < transparency && transparency <= 1.0)
		{
			sprintf(str,"faces = (transmission = (r=%f g=%f b=%f))",transparency,transparency,transparency);
			HC_Set_Color (str);
		}
	}

	return color_exists || material_properties_exist;
}

HC_KEY KOpenPointerSegment(void *SPAptr)
{
	char pbuffer[POINTER_BUFFER_SIZE];
	ptoax(SPAptr, pbuffer);
	HC_KEY key=HC_KOpen_Segment(pbuffer);
	return key;
}

logical valid_segment(HC_KEY key)
{
	char status[16];
	status[0]='0';
	EXCEPTION_BEGIN
	EXCEPTION_TRY
		HC_Show_Key_Status(key, status);
		if(status[0] == 'i')
			return FALSE;
	EXCEPTION_CATCH_FALSE
		return FALSE;
	EXCEPTION_END

	return TRUE;
}

#ifdef NT
void BGRToRGB( BITMAPINFO *biInfo, void *biBits, int biWidth);
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
#define OFFSET_bfType       0
#define OFFSET_bfSize       2
#define OFFSET_bfReserved1  6
#define OFFSET_bfReserved2  8
#define OFFSET_bfOffBits    10
#define SIZEOF_BITMAPFILEHEADER 14

// Read a WORD-aligned DWORD.  Needed because BITMAPFILEHEADER has
// WORD-alignment.
#define READDWORD(pv)   ( (DWORD)((PWORD)(pv))[0]               \
                          | ((DWORD)((PWORD)(pv))[1] << 16) )   \

// Computes the number of BYTES needed to contain n number of bits.
#define BITS2BYTES(n)   ( ((n) + 7) >> 3 )

typedef struct _RGBIMAGEREC {
    ULONG sizeX;
    ULONG sizeY;
    BYTE *data;
} RGBIMAGEREC;

extern RGBIMAGEREC *tkDIBImageLoadAW(char *fileName, BOOL bUnicode);
extern BOOL bVerifyDIB(WCHAR *fileName, ULONG *pWidth, ULONG *pHeight);

//
// RGB color structure that stores colors in R-G-B order in
// memory instead of the B-G-R format used by RGBTRIPLE.
//
typedef struct _CGLRGBTRIPLE { 
    BYTE rgbRed;
    BYTE rgbGreen;
  	BYTE rgbBlue;
} CGLRGBTRIPLE;

// convertGMP
// returns: 0 for success, anything else is error.

// Code to convert from a DIB to a Hoops image
void InsertRGBImage(int width, int height, unsigned char* pPxl, LPCTSTR lpszName, LPCTSTR hoops_name)
{
	SPAUNUSED(lpszName)
	fp_sentry fps;

	// Windows dword-aligns the rows, HOOPS wants them packed...
	int nbRowIn = 3 * width;
//	if (nbRowIn % 4 != 0)
//		nbRowIn += 4 - (nbRowIn % 4);

	// But there is a bug in the HOOPS OpenGL texture mapping so
	// odd widths don't work, so pad it too...
	int wHoops = width;
	if (wHoops % 4 != 0)
		wHoops += 4 - (wHoops % 4);
	int nbRowOut = 3 * wHoops;

	unsigned char* pImage = ACIS_NEW unsigned char[3*wHoops*height];
	unsigned char* pImgRow = pImage;

	// And the byte orderings are different.
	for (int j=0; j<height; ++j, pPxl+=nbRowIn, pImgRow+=nbRowOut)
	{
		int i;
		for (i=0; i<3*width; i+=3)
		{
			pImgRow[i+0] = pPxl[i+2];
			pImgRow[i+1] = pPxl[i+1];
			pImgRow[i+2] = pPxl[i+0];
		}
		int iLast = i-3;			// Pad with last valid RGB on input row
		for ( ; i<nbRowOut; i+=3)
		{
			pImgRow[i+0] = pPxl[iLast+2];
			pImgRow[i+1] = pPxl[iLast+1];
			pImgRow[i+2] = pPxl[iLast+0];
		}
	}

	char str[_MAX_PATH+100];
	sprintf(str,"RGB, name=%s",hoops_name);

	HC_Insert_Image(0, 0, 0, str, wHoops, height, pImage);
	//HC_Insert_Image(0, 0, 0, "RGB, name = E:image_test", wHoops, height, pImage);

	ACIS_DELETE [] STD_CAST pImage;
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

WORD DibNumColors(VOID FAR * pv)
{
    WORD                bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

    /*  With the BITMAPINFO format headers, the size of the palette
     *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
     *  is dependent on the bits per pixel ( = 2 raised to the power of
     *  bits/pixel).
     *
     *  Because of the way we use this call, BITMAPINFOHEADER may be out
     *  of alignment if it follows a BITMAPFILEHEADER.  So use the macro
     *  to safely access DWORD fields.
     */
    if (READDWORD(&lpbi->biSize) != sizeof(BITMAPCOREHEADER)){
        if (READDWORD(&lpbi->biClrUsed) != 0)
        {
            return (WORD) READDWORD(&lpbi->biClrUsed);
        }
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits){
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
* Loads a DIB file (specified as either an ANSI or Unicode filename,
* depending on the bUnicode flag) and converts it into a TK image format.
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
bool tkDIBImageLoadAW(const char *fileName, BOOL bUnicode, RGBIMAGEREC &image)
{
	SPAUNUSED(bUnicode)

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
    HANDLE     hMap = (HANDLE) NULL;                // Mapping object handle
    PVOID      pvFile = (PVOID) NULL;               // Ptr to mapped file
    HDC        hdcMem = (HDC) NULL;                 // 32BPP mem DC
    HBITMAP    hbmRGB = (HBITMAP) NULL;             // 32BPP RGB bitmap
    BITMAPINFO *pbmiSource = (BITMAPINFO *) NULL;   // Ptr to 32BPP BITMAPINFO
    BITMAPINFO *pbmiRGB = (BITMAPINFO *) NULL;      // Ptr tp file's BITMAPINFO

    int i, j;

// Map the DIB file into memory.
    hFile =CreateFile(fileName, GENERIC_READ,FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
                       				
    if (hFile == INVALID_HANDLE_VALUE)
        goto tkDIBLoadImage_cleanup;

    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMap)
        goto tkDIBLoadImage_cleanup;

    pvFile = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!pvFile)
        goto tkDIBLoadImage_cleanup;

// Check the file header.  If the BFT_BITMAP magic number is there,
// then the file format is a BITMAPFILEHEADER followed immediately
// by either a BITMAPINFOHEADER or a BITMAPCOREHEADER.  The bitmap
// bits, in this case, are located at the offset bfOffBits from the
// BITMAPFILEHEADER.
//
// Otherwise, this may be a raw BITMAPINFOHEADER or BITMAPCOREHEADER
// followed immediately with the color table and the bitmap bits.

    pbmf = (BITMAPFILEHEADER *) pvFile;

    if ( pbmf->bfType == BFT_BITMAP )
    {
        pbmihFile = (BITMAPINFOHEADER *) ((PBYTE) pbmf + SIZEOF_BITMAPFILEHEADER);

    // BITMAPFILEHEADER is WORD aligned, so use safe macro to read DWORD
    // bfOffBits field.

        pvBitsFile = (PVOID *) ((PBYTE) pbmf
                                + READDWORD((PBYTE) pbmf + OFFSET_bfOffBits));
    }
    else
    {
        pbmihFile = (BITMAPINFOHEADER *) pvFile;

    // Determination of where the bitmaps bits are needs to wait until we
    // know for sure whether we have a BITMAPINFOHEADER or a BITMAPCOREHEADER.
    }

// Determine the number of colors in the DIB palette.  This is non-zero
// only for 8BPP or less.

    wNumColors = DibNumColors(pbmihFile);

// Create a BITMAPINFO (with color table) for the DIB file.  Because the
// file may not have one (BITMAPCORE case) and potential alignment problems,
// we will create a new one in memory we allocate.
//
// We distinguish between BITMAPINFO and BITMAPCORE cases based upon
// BITMAPINFOHEADER.biSize.

    pbmiSource = (BITMAPINFO *)
        LocalAlloc(LMEM_FIXED, sizeof(BITMAPINFO)
                               + wNumColors * sizeof(RGBQUAD));
    if (!pbmiSource)
        goto tkDIBLoadImage_cleanup;

    // Note: need to use safe READDWORD macro because pbmihFile may
    // have only WORD alignment if it follows a BITMAPFILEHEADER.

    switch (READDWORD(&pbmihFile->biSize))
    {
    case sizeof(BITMAPINFOHEADER):

    // Convert WORD-aligned BITMAPINFOHEADER to aligned BITMAPINFO.

        pbmiSource->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmiSource->bmiHeader.biWidth         = READDWORD(&pbmihFile->biWidth);
        pbmiSource->bmiHeader.biHeight        = READDWORD(&pbmihFile->biHeight);
        pbmiSource->bmiHeader.biPlanes        = pbmihFile->biPlanes;
        pbmiSource->bmiHeader.biBitCount      = pbmihFile->biBitCount;
        pbmiSource->bmiHeader.biCompression   = READDWORD(&pbmihFile->biCompression);
        pbmiSource->bmiHeader.biSizeImage     = READDWORD(&pbmihFile->biSizeImage);
        pbmiSource->bmiHeader.biXPelsPerMeter = READDWORD(&pbmihFile->biXPelsPerMeter);
        pbmiSource->bmiHeader.biYPelsPerMeter = READDWORD(&pbmihFile->biYPelsPerMeter);
        pbmiSource->bmiHeader.biClrUsed       = READDWORD(&pbmihFile->biClrUsed);
        pbmiSource->bmiHeader.biClrImportant  = READDWORD(&pbmihFile->biClrImportant);

    // Copy color table.  It immediately follows the BITMAPINFOHEADER.

        memcpy((PVOID) &pbmiSource->bmiColors[0], (PVOID) (pbmihFile + 1),
               wNumColors * sizeof(RGBQUAD));

    // If we haven't already determined the SPAposition of the image bits,
    // we may now assume that they immediately follow the color table.

        if (!pvBitsFile)
            pvBitsFile = (PVOID) ((PBYTE) (pbmihFile + 1)
                         + wNumColors * sizeof(RGBQUAD));
        break;

    case sizeof(BITMAPCOREHEADER):
        pbmchFile = (BITMAPCOREHEADER *) pbmihFile;

    // Convert BITMAPCOREHEADER to BITMAPINFOHEADER.

        pbmiSource->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmiSource->bmiHeader.biWidth         = (DWORD) pbmchFile->bcWidth;
        pbmiSource->bmiHeader.biHeight        = (DWORD) pbmchFile->bcHeight;
        pbmiSource->bmiHeader.biPlanes        = pbmchFile->bcPlanes;
        pbmiSource->bmiHeader.biBitCount      = pbmchFile->bcBitCount;
        pbmiSource->bmiHeader.biCompression   = BI_RGB;
        pbmiSource->bmiHeader.biSizeImage     = 0;
        pbmiSource->bmiHeader.biXPelsPerMeter = 0;
        pbmiSource->bmiHeader.biYPelsPerMeter = 0;
        pbmiSource->bmiHeader.biClrUsed       = wNumColors;
        pbmiSource->bmiHeader.biClrImportant  = wNumColors;

    // Convert RGBTRIPLE color table into RGBQUAD color table.

        {
            RGBQUAD *rgb4 = pbmiSource->bmiColors;
            RGBTRIPLE *rgb3 = (RGBTRIPLE *) (pbmchFile + 1);

            for (i = 0; i < wNumColors; i++)
            {
                rgb4->rgbRed   = rgb3->rgbtRed  ;
                rgb4->rgbGreen = rgb3->rgbtGreen;
                rgb4->rgbBlue  = rgb3->rgbtBlue ;
                rgb4->rgbReserved = 0;

                rgb4++;
                rgb3++;
            }
        }

    // If we haven't already determined the SPAposition of the image bits,
    // we may now assume that they immediately follow the color table.

        if (!pvBitsFile)
            pvBitsFile = (PVOID) ((PBYTE) (pbmihFile + 1)
                         + wNumColors * sizeof(RGBTRIPLE));
        break;

    default:
        goto tkDIBLoadImage_cleanup;
    }

// Fill in default values (for fields that can have defaults).

    if (pbmiSource->bmiHeader.biSizeImage == 0)
    {
        pbmiSource->bmiHeader.biSizeImage = 
        		BITS2BYTES((DWORD) 	pbmiSource->bmiHeader.biWidth * 
        							pbmiSource->bmiHeader.biBitCount) * 
        							pbmiSource->bmiHeader.biHeight;
    }
       							
    if (pbmiSource->bmiHeader.biClrUsed == 0)
        pbmiSource->bmiHeader.biClrUsed = wNumColors;

// Create memory DC.

    hdcMem = CreateCompatibleDC(NULL);
    if (!hdcMem)
        goto tkDIBLoadImage_cleanup;

// Create a 32BPP RGB DIB section and select it into the memory DC.

    pbmiRGB = (BITMAPINFO *)
              LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(BITMAPINFO)
                                                   + 2 * sizeof(RGBQUAD));
    if (!pbmiRGB)
        goto tkDIBLoadImage_cleanup;

    pbmiRGB->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pbmiRGB->bmiHeader.biWidth         = pbmiSource->bmiHeader.biWidth;
    pbmiRGB->bmiHeader.biHeight        = pbmiSource->bmiHeader.biHeight;
    pbmiRGB->bmiHeader.biPlanes        = 1;
    pbmiRGB->bmiHeader.biBitCount      = 32;
    pbmiRGB->bmiHeader.biCompression   = BI_RGB;
    pbmiRGB->bmiHeader.biSizeImage     = pbmiRGB->bmiHeader.biWidth
                                         * abs(pbmiRGB->bmiHeader.biHeight) * 4;

    // This may look backwards, but because of the ordering of RGBQUADS,
    // this will yield an RGB format (vs. the BGR format that will result
    // from the "natural" ordering).

    pbmiRGB->bmiColors[0].rgbBlue  = 0xff;  // red mask
    pbmiRGB->bmiColors[1].rgbGreen = 0xff;  // green mask
    pbmiRGB->bmiColors[2].rgbRed   = 0xff;  // blue mask

    hbmRGB = CreateDIBSection(hdcMem, (BITMAPINFO *) pbmiRGB, DIB_RGB_COLORS, 
    							(VOID **) &pjBitsRGB, NULL, 0);
    if (!hbmRGB)
        goto tkDIBLoadImage_cleanup;

    if (!SelectObject(hdcMem, hbmRGB))
        goto tkDIBLoadImage_cleanup;

// Slam the DIB file image into the memory DC.  GDI will do the work of
// translating whatever format the DIB file has into our 32BPP RGB format.

    if (!SetDIBits(hdcMem, hbmRGB, 0, pbmiSource->bmiHeader.biHeight, 
    				pvBitsFile, pbmiSource, DIB_RGB_COLORS))
	{
        goto tkDIBLoadImage_cleanup;
	}

// Convert to TK image format (packed RGB format).

    pjTKBits = (PBYTE) LocalAlloc(LMEM_FIXED, pbmiRGB->bmiHeader.biSizeImage);
    if (!pjTKBits)
        goto tkDIBLoadImage_cleanup;

    pjSrc = pjBitsRGB;
    pjDst = pjTKBits;
    for (i = 0; i < pbmiSource->bmiHeader.biHeight; i++)
    {
        for (j = 0; j < pbmiSource->bmiHeader.biWidth; j++)
        {
            *pjDst++ = *pjSrc++;
            *pjDst++ = *pjSrc++;
            *pjDst++ = *pjSrc++;
            pjSrc++;
        }
    }

    // If we get to here, we have suceeded!
    result = TRUE;
    image.sizeX = pbmiSource->bmiHeader.biWidth;
    image.sizeY = pbmiSource->bmiHeader.biHeight;
    image.data = pjTKBits;
	//image.data = pjBitsRGB;

// Cleanup objects.

tkDIBLoadImage_cleanup:
    {
        if (hdcMem)
            DeleteDC(hdcMem);

        if (hbmRGB)
            DeleteObject(hbmRGB);

        if (pbmiRGB)
            LocalFree(pbmiRGB);

        if (pbmiSource)
            LocalFree(pbmiSource);

        if (pvFile)
            UnmapViewOfFile(pvFile);

        if (hMap)
            CloseHandle(hMap);

        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);
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

BOOL bVerifyDIB(WCHAR *fileName, ULONG *pWidth, ULONG *pHeight)
{
    BOOL bRet = FALSE;
    BITMAPFILEHEADER *pbmf;         // Ptr to file header
    BITMAPINFOHEADER *pbmihFile;    // Ptr to file's info header (if it exists)
    BITMAPCOREHEADER *pbmchFile;    // Ptr to file's core header (if it exists)

    // These need to be cleaned up when we exit:
    HANDLE     hFile = INVALID_HANDLE_VALUE;        // File handle
    HANDLE     hMap = (HANDLE) NULL;                // Mapping object handle
    PVOID      pvFile = (PVOID) NULL;               // Ptr to mapped file

// Map the DIB file into memory.

    hFile = CreateFileW((LPWSTR) fileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
    							OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        goto bVerifyDIB_cleanup;

    hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMap)
        goto bVerifyDIB_cleanup;

    pvFile = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!pvFile)
        goto bVerifyDIB_cleanup;

// Check the file header.  If the BFT_BITMAP magic number is there,
// then the file format is a BITMAPFILEHEADER followed immediately
// by either a BITMAPINFOHEADER or a BITMAPCOREHEADER.  The bitmap
// bits, in this case, are located at the offset bfOffBits from the
// BITMAPFILEHEADER.
//
// Otherwise, this may be a raw BITMAPINFOHEADER or BITMAPCOREHEADER
// followed immediately with the color table and the bitmap bits.

    pbmf = (BITMAPFILEHEADER *) pvFile;

    if ( pbmf->bfType == BFT_BITMAP )
        pbmihFile = (BITMAPINFOHEADER *)((PBYTE)pbmf + SIZEOF_BITMAPFILEHEADER);
    else
        pbmihFile = (BITMAPINFOHEADER *) pvFile;

// Get the width and height from whatever header we have.
//
// We distinguish between BITMAPINFO and BITMAPCORE cases based upon
// BITMAPINFOHEADER.biSize.

    // Note: need to use safe READDWORD macro because pbmihFile may
    // have only WORD alignment if it follows a BITMAPFILEHEADER.

    switch (READDWORD(&pbmihFile->biSize))
    {
    case sizeof(BITMAPINFOHEADER):

        *pWidth  = READDWORD(&pbmihFile->biWidth);
        *pHeight = READDWORD(&pbmihFile->biHeight);
        bRet = TRUE;

        break;

    case sizeof(BITMAPCOREHEADER):
        pbmchFile = (BITMAPCOREHEADER *) pbmihFile;

    // Convert BITMAPCOREHEADER to BITMAPINFOHEADER.

        *pWidth  = (DWORD) pbmchFile->bcWidth;
        *pHeight = (DWORD) pbmchFile->bcHeight;
        bRet = TRUE;

        break;

    default:
        break;
    }

bVerifyDIB_cleanup:

    if (pvFile)
        UnmapViewOfFile(pvFile);

    if (hMap)
        CloseHandle(hMap);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    return bRet;
}

int LoadBMP( LPCTSTR lpszName, LPCTSTR hoops_name)
{
	int worked = 1;
	RGBIMAGEREC image;

	HC_Open_Segment("?Include Library/images");
		HC_KEY key=HC_KOpen_Segment(hoops_name);
		//HC_Flush_Contents (".", "everything");
		if (!HC_Show_Existence("everything"))
		{
			if (tkDIBImageLoadAW(lpszName, FALSE, image))
				InsertRGBImage( image.sizeX, image.sizeY, image.data, lpszName, hoops_name);
			else
				worked=0;
		}
		HC_Close_Segment();
	HC_Close_Segment();

	return worked;
}
#endif // NT

void MakeArrow(SPAposition const &arrow_base,
			   SPAvector          const &arrow_direction,
			   double             const  arrow_length,
			   ha_arrow_head_type const  arrow_type_o_head,
			   ha_arrow_size_type const  arrow_size_o_head)
{
	SPAunit_vector arrow_vector = normalise(arrow_direction);
	SPAvector      cone_vector  = arrow_vector;
	SPAposition    arrow_line[2];
	SPAposition    cone_pos[3];
	HA_Point       cone_points[2];
	float          cone_radii[2];
	float          cone_length;
	float          arrow_len;

	if (arrow_length <= 0.0f)
	{
		arrow_line[0] = arrow_base;
		arrow_line[1] = arrow_base + arrow_direction;
		arrow_len     = (float)arrow_direction.len();
	}
	else
	{
		arrow_line[0] = arrow_base;
		arrow_line[1] = arrow_base + arrow_vector * arrow_length;
		arrow_len     = (float)arrow_length;
	}

	switch (arrow_size_o_head)
	{
		case ha_tiny:
			cone_length =(float) arrow_length * 0.025f;
			break;
		case ha_little:
			cone_length =(float) arrow_length * 0.05f;
			break;
		case ha_large:
			cone_length =(float) arrow_length * 0.25f;
			break;
		case ha_enormous:
			cone_length =(float) arrow_length * 0.5f;
			break;
		case ha_average:
		default:
			cone_length =(float) arrow_length * 0.1f;
			break;
	}

	cone_vector *= cone_length;

	cone_pos[0] = arrow_line[1];
	cone_pos[1] = arrow_line[1] - cone_vector;
	cone_pos[2] = arrow_line[1] - cone_vector * 2;

	cone_radii[0] = cone_length * 0.25f;
	cone_radii[1] = 0.0F;

	// Insert arrow line
	fp_sentry fps;
	HC_Insert_Line(arrow_line[0].x(), arrow_line[0].y(), arrow_line[0].z(), arrow_line[1].x(), arrow_line[1].y(), arrow_line[1].z());

	if ( !is_equal(cone_pos[0], cone_pos[1]) )
	{
		switch (arrow_type_o_head)
		{
			case ha_simple:          // a simple arrow ---->
			default:
				// Insert arrow head...
				cone_points[0].Set((float)cone_pos[1].x(), (float)cone_pos[1].y(), (float)cone_pos[1].z());
				cone_points[1].Set((float)cone_pos[0].x(), (float)cone_pos[0].y(),(float) cone_pos[0].z());
				HC_Insert_PolyCylinder(2, cone_points, 2, cone_radii, "both");
				break;
			case ha_double_outside:  // double arrows  -->->
				// Insert arrow head...
				cone_points[0].Set((float)cone_pos[1].x(), (float)cone_pos[1].y(), (float)cone_pos[1].z());
				cone_points[1].Set((float)cone_pos[0].x(), (float)cone_pos[0].y(), (float)cone_pos[0].z());
				HC_Insert_PolyCylinder(2, cone_points, 2, cone_radii, "both");

				// Insert 2nd arrow head...
				cone_points[0].Set((float)cone_pos[2].x(), (float)cone_pos[2].y(), (float)cone_pos[2].z());
				cone_points[1].Set((float)cone_pos[1].x(), (float)cone_pos[1].y(), (float)cone_pos[1].z());
				HC_Insert_PolyCylinder(2, cone_points, 2, cone_radii, "both");
				break;
			case ha_double_inside:   // double arrows  --<->
				// Insert arrow head...
				cone_points[0].Set((float)cone_pos[1].x(),(float)cone_pos[1].y(),(float)cone_pos[1].z());
				cone_points[1].Set((float)cone_pos[0].x(),(float)cone_pos[0].y(),(float)cone_pos[0].z());
				HC_Insert_PolyCylinder(2, cone_points, 2, cone_radii, "both");

				// Insert 2nd (reversed) arrow head...
				cone_points[1].Set((float)cone_pos[2].x(),(float) cone_pos[2].y(),(float) cone_pos[2].z());
				cone_points[0].Set((float)cone_pos[1].x(),(float) cone_pos[1].y(),(float) cone_pos[1].z());
				HC_Insert_PolyCylinder(2, cone_points, 2, cone_radii, "both");
				break;
			case ha_headless:        // obviously this -----
				// Nothing else to insert
				break;
		}
	}
}

logical Parse_YesNo_And_Mutate_Options_Using_Bitmask(char* token, unsigned long bitmask, unsigned long* options)
{
	// warning: this function will often mutate argument "token"
	
	char token2[1025];

	if ( 0 == strncmp( token, "no", 2))
	{
		bitmask = ~bitmask;
		*options &= bitmask;
	}
	else if (strstr(token,"=")&&HC_Parse_String(token,"=",-1,token2))
	{

		if (0==strncmp(token2,"off",3))
		{
			bitmask = ~bitmask;
			*options &= bitmask;
		}
		else if (0==strncmp(token2,"on",2))
		{
			*options |= bitmask;
		}
		else if (0==strncmp(token2,"1",1))
		{
			*options |= bitmask;
		}
		else if (0==strncmp(token2,"0",1))
		{
			bitmask = ~bitmask;
			*options &= bitmask;
		}
		else
		{
			/*error*/
			char * mes = "bad value:";
			char * mesv[2]; 

			const char * fun = "HA_Set_Rendering_Options";

			mesv[0]=mes;
			mesv[1]=token2;

			HC_Report_Error( 50, 309, 1, 2, mesv, 1, &fun );
			return FALSE;
		}			
	}
	else{
		/*plain option*/
		*options |= bitmask;
	}
 return TRUE;
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

void print_segment_leaf( const char * in_leaf_string )
{
	HC_KEY key = 0;
	char type[64];
	fprintf( debug_file_ptr, "%s", in_leaf_string );
	// Refer to E:\Guy\HOOPS\HOOPS_3DF_2120\demo\qt\qthoopsrefapp_4\HQSegmentBrowserFrame.cpp segTreeListViewItem::addAttributeLeaves() line 427 to eof for how to gather info.
	int total_face_count = 0, total_facet_count = 0, total_vertex_count = 0, total_polyline_count = 0, total_marker_count = 0, total_point_count = 0;
	bool firsttime = true;
	while ( HC_Find_Contents( type, &key ) )
	{
		char buffer[1024] = "";
		if ( firsttime )
			printf( "\\ ", firsttime = false );
		if ( 0 == strncmp( type, "include", 7 ) )
		{
			HC_Show_Include_Segment( key, buffer );
			fprintf( debug_file_ptr, "\n\t\tInc\"%s\"", buffer );
		} else if ( 0 == strncmp( type, "style", 5 ) )
		{
			HC_Show_Style_Segment( key, buffer );
			fprintf( debug_file_ptr, " (style:%s) ", buffer );
		} else if ( 0 == strncmp( type, "condition", 9 ) )
		{
			HC_Open_Segment_By_Key( key );
				HC_Show_Conditions( buffer );
			HC_Close_Segment();
			fprintf( debug_file_ptr, " (conds:" );
			fprintf( debug_file_ptr, "%s,", buffer );
			fprintf( debug_file_ptr, ") " );
			HC_Show_Net_Conditions( buffer );
			fprintf( debug_file_ptr, " (net_conds:" );
			fprintf( debug_file_ptr, "%s,", buffer );
			fprintf( debug_file_ptr, ") " );
		} else if ( 0 == strncmp( type, "visibility", 9 ) )
		{
			HC_Show_Visibility( buffer );
			fprintf( debug_file_ptr, " (vis:%s) ", buffer );
		} else if ( 0 == strncmp( type, "segment", 7 ) )
		{
			HC_Show_Segment( key, buffer );
			HC_Open_Segment_By_Key( key );
			{
				HC_Show_Segment( key, buffer );
				//HC_Show_Pathname_Expansion( ".", buffer );
				char * last_name = strrchr( buffer, '/' );
				//fprintf( debug_file_ptr, " (%#x:last_name:%s) ", key, last_name );
			} HC_Close_Segment();
		} else if ( 0 == strncmp( type, "polyline", 6 ) )
		{
			total_polyline_count++;
		} else if ( 0 == strncmp( type, "marker", 6 ) )
		{
			total_marker_count++;
		} else if ( 0 == strncmp( type, "shell", 5 ) )
		{
			int facet_count = 0, vertex_count = 0, triangle_count = 0, ts_length = 0, numtriangles = 0, numvertices = 0, point_count = 0, face_list_count = 0;
			HC_Show_Shell_Size( key, &point_count, &face_list_count );
			HC_Show_Shell_By_Tristrips_Size( key, &vertex_count, &ts_length, &triangle_count );
			HC_Show_Shell_Face_Count( key, &facet_count );
			total_face_count++;
			total_facet_count += facet_count;
			total_point_count += point_count;
			total_vertex_count += vertex_count;
			//fprintf( debug_file_ptr, "(%#x:#faces=%d,#vertices=%d)", key, face_count, vertex_count );
		} else if ( 0 == strncmp( type, "color", 5 ) )
		{
			HC_Show_Color( buffer );
			char * everything_ptr = strstr( buffer, "everything=white" );
			if ( everything_ptr )
				memcpy( everything_ptr, "R=1 G=1 B=1     ", 16 );
			char * geom_ptr = strstr( buffer, "geometry=R=" );
			fprintf( debug_file_ptr, " (Color<%s>) ", geom_ptr ? geom_ptr + 9 : buffer );
		//} else if
		//	( 0 == strncmp( type, "special", 7 )
		//	|| 0 == strncmp( type, "window", 6 )
		//	|| 0 == strncmp( type, "heuristics", 6 )
		//	|| 0 == strncmp( type, "line pattern", 6 )
		//	|| 0 == strncmp( type, "line weight", 6 )
		//	|| 0 == strncmp( type, "marker size", 11 )
		//	|| 0 == strncmp( type, "marker symbol", 13 )
		//	|| 0 == strncmp( type, "modelling matrix", 6 )
		//	|| 0 == strncmp( type, "text alignment", 6 )
		//	|| 0 == strncmp( type, "text font", 6 )
		//	|| 0 == strncmp( type, "user options", 6 )
		//	//|| 0 == strncmp( type, "visibility", 7 )
		//	)
		//{
		//	fprintf( debug_file_ptr, "*" ); // Ignore for now.
		} else
			fprintf( debug_file_ptr, " (%s) ", type );
	}
	if ( total_face_count )
		fprintf( debug_file_ptr, " (%ld_face%s) ", total_face_count, total_face_count == 1 ? "" : "s" );
	if ( total_facet_count )
		fprintf( debug_file_ptr, " (%ld_facet%s) ", total_facet_count, total_facet_count == 1 ? "" : "s" );
	if ( total_point_count )
		fprintf( debug_file_ptr, " (%ld_point%s) ", total_point_count, total_point_count == 1 ? "" : "s" );
	if ( total_marker_count )
		fprintf( debug_file_ptr, " (%ld_marker%s) ", total_marker_count, total_marker_count == 1 ? "" : "s" );
	if ( total_polyline_count )
		fprintf( debug_file_ptr, " (%ld_line%s) ", total_polyline_count, total_polyline_count == 1 ? "" : "s" );
	if ( firsttime )
		printf( "\\ ", firsttime = false );
	fprintf( debug_file_ptr, "\n" );
}
// Prints to debug_file_ptr.  The base_key argument is where we start;
// leaves_only indicates whether to print the entire tree, or only its
// leaves; the level argument is used in recursion, and is set to zero
// by default.
bool print_segment_tree( const char* in_base_seg, int in_level, std::vector<void*>& in_hex_values, std::vector<void*>& in_amp_values )
{
	std::string base_string = in_base_seg;
	if ( hoops_tree_ptrs.on() )	for ( int counter = 4; counter; counter-- )
	{
		char x_or_amp = 'X';
		int bytes_to_replace = byte_count + 2;
		int index = 0;
		size_t start_pos = base_string.find( "0x0", 0 );
		void * hex = NULL;
		if ( start_pos != std::string::npos )
		{
			hex = axtop( const_cast<char*>( base_string.substr( start_pos, byte_count + 2 ).c_str() ) ); // axtop() skips the "0x"
			std::vector<void*>::iterator it;
			for ( it = in_hex_values.begin(); it != in_hex_values.end(); ++it, index++ )
				if ( *it == hex )
					break;
			if ( it == in_hex_values.end() )
				in_hex_values.push_back( hex );
		} else
		{
			x_or_amp = '@';
			bytes_to_replace = byte_count + 1;
			index = 0;
			start_pos = base_string.find( "@8", 0 );
			if ( start_pos != std::string::npos )
			{
				hex = axtop( const_cast<char*>( base_string.substr( start_pos - 1, byte_count + 1 ).c_str() ) ); // axtop skips the first 2 chars, which are usually "0x", but in this case it is "@"
				std::vector<void*>::iterator it;
				for ( it = in_amp_values.begin(); it != in_amp_values.end(); ++it, index++ )
					if ( *it == hex )
						break;
				if ( it == in_amp_values.end() )
					in_amp_values.push_back( hex );
			}
		}
		if ( start_pos != std::string::npos )
		{
			char buffer[50];
			size_t acis_pos = base_string.find( "acis ", 0 );
			if ( x_or_amp == 'X' && acis_pos != std::string::npos && acis_pos < start_pos )
			{
				ENTITY * ent = (ENTITY*)hex;
				tag_id_type tag = ent->tag();
				sprintf_s( buffer, 50, "ent_%d_", ( (ENTITY*)hex )->tag() );
			} else
				sprintf_s( buffer, 50, "%c_%d", x_or_amp, index );
			base_string.replace( start_pos, bytes_to_replace, buffer );
		}
	}
	HC_Open_Segment( in_base_seg );
	{	// Print this leaf...
		HC_Begin_Contents_Search( ".", "everything" );
		print_segment_leaf( base_string.c_str() );
		HC_End_Contents_Search();
		// Recurrsive cyle thru the leaf's leaves...
		HC_Begin_Segment_Search( "*" );
		char child_seg[256];
		while ( HC_Find_Segment( child_seg ) )
			print_segment_tree( child_seg, in_level + 1, in_hex_values, in_amp_values );
		HC_End_Segment_Search();
	} HC_Close_Segment();
	return true;
}

// Prints to debug_file_ptr.  The base_key argument is where we start;
// leaves_only indicates whether to print the entire tree, or only its
// leaves; the in_level argument is used in recursion, and is set to zero
// by default.
bool HA_Print_Segment_Tree( const char* in_base_seg, int in_level )
{
	std::vector<void*> hex_values;
	std::vector<void*> amp_values;
	logical return_value = print_segment_tree( in_base_seg, in_level, hex_values, amp_values );
	return return_value != 0 ? true : false;
}
