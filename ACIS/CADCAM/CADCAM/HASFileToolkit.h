//
// Copyright (c) 2000 by Tech Soft 3D, LLC.
// The information contained herein is confidential and proprietary to
// Tech Soft 3D, LLC., and considered a trade secret as defined under
// civil and criminal statutes.  Tech Soft 3D shall pursue its civil
// and criminal remedies in the event of unauthorized use or misappropriation
// of its trade secrets.  Use of this information by anyone other than
// authorized employees of Tech Soft 3D, LLC. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
// $Header: /files/homes/master/cvs/hoops_master/hoops_appwizard/VCWizards/HOOPSAppWiz_NET2008/Templates/1033/3daf/HMSFileToolkit.h,v 1.1 2008-04-16 23:14:27 stage Exp $
//

#ifndef _HMSFILETOOLKIT_H_
#define _HMSFILETOOLKIT_H_

#include "afxtempl.h"
#include "HStream.h"
#include "HOpcodeShell.h"
#include "HPolyPoly.h"


class HBaseModel;

//! The CModellerObject class is used to store and manage information about an solid modeler object
/*! 
Assuming a BRep model, this class stores the information about it's (body, face, edge etc.) and also about 
its parents and childern. This class is expected to be used as a base class for different type of modeller 
objects your geometry kernel may have.
*/
class CModellerObject
{

public:

  	HC_KEY *m_keys;		/*!< Array of HOOPS keys associated with this modeller object */
	int  m_numchildren; /*!< Number of children this object has */
	long *m_children;	/*!< Array of ids of children of this object */
 	long *m_parents;	/*!< Array of ids of parents of this object */
	int m_identifier;	/*!< Solid modeler identifier (tag/pointer) of this object */
 
	/*! Constructs CModellerObject object */
	CModellerObject() { m_keys = 0; m_numchildren=0; m_children = 0; m_parents = 0; m_identifier = 0; } 

	/*! Destroys the CModeller object and frees up memory associated with it */
	~CModellerObject()
	{
		delete [] m_keys;
		delete [] m_children;
		delete [] m_parents;
	}

	/*! Returns the class of the object (body/face/edge). Should be overridden by the derived class */
	virtual  int AskClass() { return 0; }

	/*! Returns the number of keys associated with this model. Should be overridden */
	virtual long GetKeyNum() { 
		if (!m_keys)
			return 0;
		else
			return 1;
	}

	/*! Associates the given key with the modeller object. It erases any previously associated keys */
	virtual void SetKey(HC_KEY key)
	{
		if (m_keys!=0)
			delete m_keys;
		if (key!=0)
		{
			m_keys = new HC_KEY;
			m_keys[0] = key;
		}
	}
	
	/*! Associates the given id as a parent of this modeller object. It erases any previously associated parent */
	virtual void SetParent(long parent)
	{
		if (m_parents!=0)
			delete m_parents;
		m_parents = new long;
		m_parents[0] = parent;
	}

	/*! Sets the children count and allocated memory to store the ids. It erases any previously associated children */
	void SetChildNum(int numchildren)
	{
		m_numchildren = numchildren;
		if (m_children)
			delete [] m_children;
		m_children = new long[numchildren];
	}
	
};

//! The CBaseModellerObject class is used to store and manage information about an solid modeler object
/*! 
This class can be use to extend the base class and create a more specific modeller object (e.g. BRep object).
Currently, there it is not utilised in that way.
*/
class CBaseModellerObject : public CModellerObject
{

public:

	/*! Constructor */
	CBaseModellerObject()  : CModellerObject() {}

	/*! Returns the class of the object (body/face/edge). Should be overridden by the derived class */
	virtual  int AskClass() { return 0; }
};

//! The CBodyModellerObject class represents a BODY type of solid modeller object
/*! 
The CBodyModellerObject class represents a BODY type of solid modeller object
*/
class CBodyModellerObject : public CModellerObject
{
  	
public:
	/*! Constructor */
	CBodyModellerObject()  : CModellerObject() {}

	/*! Returns the class of the object - which in this case is BODY */
	virtual  int AskClass() { return BODY_TYPE; }
 
};

//! The CFaceModellerObject class represents a FACE type of solid modeller object
/*! 
The CFaceModellerObject class represents a FACE type of solid modeller object
*/
class CFaceModellerObject : public CModellerObject
{
  	
public:
	/*! Constructor */
	CFaceModellerObject()  : CModellerObject() {}

	/*! Returns the class of the object - which in this case is FACE */
	virtual  int AskClass() { return FACE_TYPE; }
};


//! The CEdgeModellerObject class represents a EDGE type of solid modeller object
/*! 
The CEdgeModellerObject class represents a EDGE type of solid modeller object
*/
class CEdgeModellerObject : public CModellerObject
{

public:

	int m_numkeys;		/*!< Number of keys associated with this object */
	int m_numparents;	/*!< Number of parents associated with this object */

	/*! Constructor */
	CEdgeModellerObject() : CModellerObject() {m_numparents=0;}

	/*! Returns the class of the object - which in this case is EDGE */
	virtual  int AskClass() { return EDGE_TYPE; }
	/*! Associates a key to the object. The key is inserted in the list at the given position */
	void SetKey(int pos, int key)
	{
		m_keys[pos] = key;
	}

	/*! Sets the associated keys count and allocated memory to store the ids. It erases any previously associated keys */
	void SetKeyNum(int numkeys)
	{
		if (m_keys!=0)
			delete m_keys;
		m_keys = new HC_KEY[numkeys];
		m_numkeys = numkeys;
	}

	/*! Adds the given id to the parents array of the object. The parent is ignored if the object has already more
	    than one parent associated
	 */
	void AddParent(int parent)
	{
		if (m_numparents == 0)
		{
			delete m_parents;
			m_parents = new long[2];
		}
		if (m_numparents<2)
			m_parents[m_numparents++] = parent;
	}

	/*! Returns the number of keys associated */
	long GetKeyNum() { return m_numkeys; }

};

//! The CModellerInfo class manages the associativity information
/*! 
The CModellerInfo uses a hash map to store the associativity information between the solid modeller and HOOPS
*/
class CModellerInfo
{
public:
	int m_objectnum;						/*!< Number of objects */
	bool m_bIsValid;						/*!< Is the object valid for use */
	CModellerObject **m_objects;			/*!< Solid modeler object information */
 	CMap <long,long&,long,long&> m_map;		/*!< Hash map storing the associativity information */
	bool m_bTesselatedFaces;				/*!< true if the body is tessleated at face level */
	bool m_bPSF;							/*!< true if the modeler file is embedded into the hsf file */

	/*! Constructor */
	CModellerInfo()
	{
		m_objects=0;
		m_objectnum = 0;
		m_map.InitHashTable(1201);
		m_bIsValid = false;
		m_bTesselatedFaces = false;
		m_bPSF = false;
 	}

	/*! Destroys the CModellerInfo object and frees up memory associated with it */
	~CModellerInfo()
	{
		if( m_objects )
		{
			for (int i=0;i<m_objectnum;i++)
				delete m_objects[i];
			delete [] m_objects;
		}
	}

	/*! Initializes the CModellerInfo. Allocates memory to store information about given number of objects */
	bool Init(int number_of_objects)
	{
 		if (m_objects!=0)
			delete [] m_objects;
		m_objects = new  CModellerObject *[number_of_objects];
		for (int i=0;i<number_of_objects;i++)
			m_objects[i]=0;
 		m_objectnum = number_of_objects;
		return 1;
	}

	/*! Returns the number of bodies and their ids this modeller info has stored */
	bool ask_bodies(int * const nbodies, long** const body_indices);

	/*! Returns the number of faces and their ids for the given body */
	bool BODY_ask_faces(long body_index, int * const nfaces, long** const face_indices);

	/*! Returns the number of edges and their ids for the given face */
	bool FACE_ask_edges(long face_index, int * const nedges, long** const edge_indices);

	/*! Returns the number of owner faces and their ids for the given edge */
	bool EDGE_ask_faces(long edge_index, int * const nfaces, long** const face_indices);

	/*! Returns the id of the owner body for the given face */
	bool FACE_ask_body(long face_index, long* const body_index);

	/*! Returns the type (body, face, edge) for the given entity */
	bool ENTITY_ask_class(long entity_index, int   & eclass);

	/*! Returns the id of the given entity */
	bool ENTITY_ask_identifier(long entity_index, int *ident);

	/*! Adds the modeler object information at the given position */
	void SetModellerObject (int pos, CModellerObject * object);

	/*! Returns the modeller object at the given position */
	CModellerObject * GetModellerObject (int pos);  

	/*! Returns the id of the given HOOPS key */
	long Compute_Entity_Index(HC_KEY key, int    eclass);

	/*! Returns the HOOPS keys for the given modeler object */
	long  Compute_Geometry_Keys(long index, long max_count, HC_KEY* keys);

};

 
//! The TK_PSLine is a custom object handler
/*! 
This class helps to tag the line objects which are not tagged by the HOOPS StreamToolkit by default.
*/
class TK_PSLine : public HTK_Line {
  private:
	int m_mystage;
  public:
	TK_PSLine() : HTK_Line() { m_mystage = 0; }
	TK_Status   Write (BStreamFileToolkit & tk) alter;
};

//! The TK_PSPolypoint is a custom object handler
/*! 
This class helps to tag the TK_PSPolypoint objects which are not tagged by the HOOPS StreamToolkit by default.
*/
class TK_PSPolypoint : public HTK_Polypoint {
  private:
	int m_mystage;
  public:
	TK_PSPolypoint(unsigned char opcode) : HTK_Polypoint(opcode) { m_mystage = 0; }
	TK_Status   Write (BStreamFileToolkit & tk) alter;
};

//! The TK_PSPolyPolypoint is a custom object handler
/*! 
This class helps to tag the TK_PSPolyPolypoint objects which are not tagged by the HOOPS StreamToolkit by default.
*/
class TK_PSPolyPolypoint : public HTK_PolyPolypoint {
  private:
	int m_mystage;
  public:
	TK_PSPolyPolypoint(unsigned char opcode) : HTK_PolyPolypoint(opcode) { m_mystage = 0; }
	TK_Status   Write (BStreamFileToolkit & tk) alter;
};

//! The TK_PSEllipse is a custom object handler
/*! 
This class helps to tag the TK_PSEllipse objects which are not tagged by the HOOPS StreamToolkit by default.
*/
class TK_PSEllipse : public HTK_Ellipse {
  private:
	int m_mystage;
  public:
	TK_PSEllipse(unsigned char opcode) : HTK_Ellipse(opcode) { m_mystage = 0; }
	TK_Status   Write (BStreamFileToolkit & tk) alter;
};

//! The TK_PSCircle is a custom object handler
/*! 
This class helps to tag the TK_PSCircle objects which are not tagged by the HOOPS StreamToolkit by default.
*/
class TK_PSCircle : public HTK_Circle {
  private:
	int m_mystage;
  public:
	TK_PSCircle(unsigned char opcode) : HTK_Circle(opcode) { m_mystage = 0; }
	TK_Status   Write (BStreamFileToolkit & tk) alter;
	
};

//! The TK_PSClose_Segment is a custom object handler
/*! 
This class helps to tag the TK_PSClose_Segment objects which are not tagged by the HOOPS StreamToolkit by default.
*/
class TK_PSClose_Segment : public HTK_Close_Segment {
  private:
	int m_mystage;
  public:
	TK_PSClose_Segment() : HTK_Close_Segment() { m_mystage = 0; }
  	TK_Status Write(BStreamFileToolkit &tk); 
		
};

//! The TK_PSUser_Data is a custom object handler
/*! 
This is the main class which handler storing and restoring of solid modeler information in an hsf file as a user data.
*/
class  TK_PSUser_Data : public TK_User_Data {
    private:
      
		int				m_pos;				/*!< Variable which keeps track of current position in the data */
		CModellerInfo	*m_mi;				/*!< Solid modeler objects information */
		int				m_mystage;			/*!< Internal variable to track the current stage in reading/writing */
		CString			m_psf_tempfilename;	/*!< Filename of temporary solid modeler file */
		long			m_psf_filesize;		/*!< Size of solid modeler file */
		FILE			*m_psf_filehandle;	/*!< File handle of solid modeler file */
		char			*m_psf_file_buff;	/*!< Buffer to read solid modeler file into */
		int				m_psf_buff_count;	/*!< Size of the buffer */


		ENTITY_LIST&		m_entityList;	/*!< List of ACIS entities to be embedded as .sat file */

    public:
		/*!
		Constructor
		\param mi Solid modeler information
		\param ENTITY_LIST ACIS entitylist to export/receive ACIS entities
		\param psf Is the solid modeler file embedded
		*/
        TK_PSUser_Data (CModellerInfo **mi, ENTITY_LIST& ent_list, bool psf = false);


		/*! Physically reads the topology information and the modeler file (if m_bPSF is true). 
		The topology information is read into the member (m_data). 
		The modeler file may be too large to hold into the memory, hence 
		it is read and stored as a temporary disk file.
		*/
		TK_Status	Read (BStreamFileToolkit & tk) alter;

		/*! Converts the raw topology information into CModellerInfo.
		    Calls geometry kernel api to read the solid modeler file (by calling ExecutePS function)
			Restores the associativity information by converting the streamtoolkit indices to actual HOOPS keys.
		*/
        TK_Status	Execute (BStreamFileToolkit & tk) alter;

		/*! Generates and topology information from the solid modeler entities the kernel apis.
		*/
        TK_Status	Interpret (BStreamFileToolkit & tk, long key, int lod=0) alter;

		/*! Physically writes the topology information and the modeler file (if m_bPSF is true). 
		The modeler file may be too large to hold into the memory, hence 
		it is written to a temporary disk file (by calling InterpretPS).
		Note: Special code is required to handle large modeler files since they can't be read all 
		at once into a memory buffer and provided to the streamtoolkit.
		*/
        TK_Status	Write (BStreamFileToolkit & tk) alter;

		/*! Creates a temporary physical solid modeler file from the entity list which is later 
		    embedded into the stream file.
		*/
		TK_Status	InterpretPS(BStreamFileToolkit & tk, long key = -1, int lod=0) alter;

		/*! Uses geometry kernel api to read the solid modeler file which was embedded in the modeler 
		stream file.
		*/
		TK_Status	ExecutePS(BStreamFileToolkit & tk);

		/*! Accumulates the data in the member variable */
		void	putdata(void *data, int size);

		/*! returns the data (long) at the current position */
		void	getdata(long &data);

		/*! returns the data (char) at the current position */
		void	getdata(char &data);

		/*! Returns the given amount (len) of data into the variable (data) */
		void	getdata(void *data,int len);	

		void	Reset();

};

// Add an identifier to hsf files written out by modeler based partviewer as a comment.
// This will help us distinguish files which have our TK_User_Data from other applications.
#define FILE_COMMENT_ID "Exported by TSA Partviewer"

class TK_PSComment : public TK_Comment 
{
  private:
	int m_mystage;
	HBaseModel * m_pHModel;
  public:

	/*! Constructor */
	TK_PSComment(HBaseModel * pHModel) : TK_Comment()
	{ 
		m_mystage = 0; 
		m_pHModel = pHModel;
	}

	/*! Inserts a comment into the file so that it can be identified as a modeler stream file while reading
	 */
  	TK_Status Write(BStreamFileToolkit &tk)
	{
		// stamp our id
		SetComment(FILE_COMMENT_ID);
		return TK_Comment::Write(tk);
	};

	/*! Checks if the comment in the file contains FILE_COMMENT_ID string.
	    If it does, it means this is modeler stream file and sets the custom user data (TK_PSUser_Data)
		opcode handler to handle it.
		This is required during the reading of modeler stream file.
	 */
	TK_Status Execute (BStreamFileToolkit & tk);

	// need to have this overridden for prewalk handlers.
	TK_Status Interpret (BStreamFileToolkit & tk, ID_Key key, int variant = 0) alter{ return TK_Normal; };

};


#endif //_HMSFILETOOLKIT_H_

