/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
// $Id: ha_point.h,v 1.3 2002/08/20 21:14:52 invscout Exp $

#ifndef _HA_BRIDGE_HA_POINT_
#define _HA_BRIDGE_HA_POINT_

#include "position.hxx"
#include "vector.hxx"
#include "unitvec.hxx"

// This class allows for easy conversion between ACIS positions and vectors,
// which are arrays of three doubles and HOOPS points, which are arrays of 
// three floats.

class HA_Point : public ACIS_OBJECT
{
	float m_Coords[3];
public:
	HA_Point(double x=0, double y=0, double z=0)
	{
		m_Coords[0] = (float) x;
		m_Coords[1] = (float) y;
		m_Coords[2] = (float) z;
	}

	HA_Point(const SPAposition& p)
	{
		m_Coords[0] = (float) p.x();
		m_Coords[1] = (float) p.y();
		m_Coords[2] = (float) p.z();
	}
	HA_Point(const SPAvector& v)
	{
		m_Coords[0] = (float) v.x();
		m_Coords[1] = (float) v.y();
		m_Coords[2] = (float) v.z();
	}
	operator SPAposition() 
	{
		return SPAposition(m_Coords[0], m_Coords[1], m_Coords[2]);
	}
	operator SPAvector()
	{
		return SPAvector(m_Coords[0], m_Coords[1], m_Coords[2]);
	}
	operator SPAunit_vector()
	{
		return SPAunit_vector(m_Coords[0],m_Coords[1],m_Coords[2]);
	}
	float x() { return m_Coords[0]; }
	float y() { return m_Coords[1]; }
	float z() { return m_Coords[2]; }
	
	void Set(float x, float y, float z=0.0f) {m_Coords[0]=x; m_Coords[1]=y; m_Coords[2]=z;}
	void Set(HA_Point *p) {if (!p) return; m_Coords[0]=p->x(); m_Coords[1]=p->y(); m_Coords[2]=p->z();}

	operator float*() 
	{
		return &m_Coords[0];
	}
};

#endif /* _HA_BRIDGE_HA_POINT_ */
