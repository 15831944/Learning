/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: hps_point.h,v 1.3 2002/08/20 21:14:52 invscout Exp $

#ifndef _HPS_BRIDGE_HPS_POINT_
#define _HPS_BRIDGE_HPS_POINT_

#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.
#include "position.hxx"
#include "vector.hxx"
#include "unitvec.hxx"

// This class allows for easy conversion between ACIS positions and vectors,
// which are arrays of three doubles and HOOPS points, which are arrays of 
// three floats.

class HPS_Point : public ACIS_OBJECT
{
	float m_Coords[3];
public:
	HPS_Point( double x = 0, double y = 0, double z = 0 )
	{
		m_Coords[0] = (float)x;
		m_Coords[1] = (float)y;
		m_Coords[2] = (float)z;
	}

	HPS_Point( const SPAposition& p )
	{
		m_Coords[0] = (float)p.x();
		m_Coords[1] = (float)p.y();
		m_Coords[2] = (float)p.z();
	}
	HPS_Point( const SPAvector& v )
	{
		m_Coords[0] = (float)v.x();
		m_Coords[1] = (float)v.y();
		m_Coords[2] = (float)v.z();
	}
	HPS_Point( const HPS::Point& p )
	{
		m_Coords[0] = (float)p.x;
		m_Coords[1] = (float)p.y;
		m_Coords[2] = (float)p.z;
	}
	operator HPS::Point()
	{
		return HPS::Point( m_Coords[0], m_Coords[1], m_Coords[2] );
	}
	operator SPAposition()
	{
		return SPAposition( m_Coords[0], m_Coords[1], m_Coords[2] );
	}
	operator SPAvector()
	{
		return SPAvector( m_Coords[0], m_Coords[1], m_Coords[2] );
	}
	operator SPAunit_vector()
	{
		return SPAunit_vector( m_Coords[0], m_Coords[1], m_Coords[2] );
	}
	float x() const { return m_Coords[0]; }
	float y() const { return m_Coords[1]; }
	float z() const { return m_Coords[2]; }

	void Set( float x, float y, float z = 0.0f ) { m_Coords[0] = x; m_Coords[1] = y; m_Coords[2] = z; }
	void Set( HPS_Point *p ) { if ( !p ) return; m_Coords[0] = p->x(); m_Coords[1] = p->y(); m_Coords[2] = p->z(); }

	operator float*( )
	{
		return &m_Coords[0];
	}
	//	inline HPS::Point operator=( HPS_Point const &in_point )
	//	{
	//		return HPS::Point( (float)in_point.x(), (float)in_point.y(), (float)in_point.z() );
	//	};
};

inline HPS::Vector operator-( HPS_Point const &v1, HPS_Point const &v2 )
{
	return HPS::Vector( v1.x() - v2.x(),
						v1.y() - v2.y(),
						v1.z() - v2.z() );
};

#endif /* _HPS_BRIDGE_HPS_POINT_ */
