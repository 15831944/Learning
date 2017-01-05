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
// $Header: /files/homes/master/cvs/hoops_master/hoops_appwizard/VCWizards/HOOPSAppWiz_NET2008/Templates/1033/3daf/HMSFileToolkit.cpp,v 1.2 2009-06-25 14:13:02 mustafa Exp $
//

#include "stdafx.h"
#include <afxtempl.h>
#include <io.h>
#include <sys/stat.h>

#include "HStream.h"
#include "HOpcodeShell.h"
#include "HASFileToolkit.h"
#include "HHOOPSModel.h"
#include "HTools.h"
#include "HUtilityLocaleString.h"

void CHECK_OUTCOME( const outcome& result );

#ifndef nobreak
  #define nobreak   //do nothing
#endif

// chunk size to read/write modeler files embedded in hsfs.
#define PSF_BUFF_CHUNK_SIZE			(64*1024)	// 65K - is that good size?


long CModellerInfo::Compute_Geometry_Keys(long index, long max_count, HC_KEY* keys)
{
	int maxcount = m_objects[index]->GetKeyNum();
	for (int i=0;i<maxcount;i++)
		keys[i] = m_objects[index]->m_keys[i];
	return maxcount;
}

long CModellerInfo::Compute_Entity_Index(HC_KEY key, int eclass)
{
	long entity;
	outcome error = 0;
	int entityClass;
	long body;

	if (HC_Show_Existence_By_Key(key, "self"))
	{
		int found = m_map.Lookup((long &)key,(long &)entity);
		if (!found)
		{
			if (eclass == BODY_TYPE ) // || eclass == PK_CLASS_instance)
			{
				HC_KEY ancestorSegment;
				// be more forgiving if they are interested in a body pointer; 
				// in case this entity has been rendered in merge_faces mode we need
				// to be a bit tricky about obtaining this info, if and only if 
				// they are looking to get the body associated with a given shell.
				char keytype[MVO_BUFFER_SIZE];
				HC_Show_Key_Type(key, keytype);
				if (keytype[0] == 's' && keytype[1] == 'h') //|| eclass == PK_CLASS_instance)
				{
					ancestorSegment = HC_KShow_Owner_By_Key(key);
					found = m_map.Lookup((long &)ancestorSegment,(long &)entity);
					// look to the parent and grandparent segment of this shell in the hopes of finding
					// a body segment, which will have a mapping to the body entity.
					while (!found && ancestorSegment != -1)
					{
						ancestorSegment = HC_KShow_Owner_By_Key(ancestorSegment);
						found = m_map.Lookup((long &)ancestorSegment,(long &)entity);
					}
					if( found )
						return entity;
					else
						return 0;
				}
				else
					return 0;
			}
			else
				return 0; 
		}
	}
	else
	{
		return 0;
	}
	if ( (ENTITY_ask_class(entity, entityClass)) != true )
		return 0;
	if( entityClass == EDGE_TYPE )
	{
		if( eclass == BODY_TYPE )
		{
			int numfaces;
			long* faces;
			error = EDGE_ask_faces(entity, &numfaces, &faces);
			if (numfaces)
				FACE_ask_body(faces[0],&body);
			else
				body=0;
			delete faces;			
			return body;	 	 
		}
		else if( eclass == EDGE_TYPE )
			return entity;
//		case PK_CLASS_ellipse:
		else if( eclass == ENTITY_TYPE )
			return entity; 	  
		else
			return 0;
	}
	else if( entityClass == FACE_TYPE )
	{
		if( eclass == BODY_TYPE )
		{
			error = FACE_ask_body(entity, &body);
			return body; 
		}
		else if( eclass == FACE_TYPE )
			return entity; 	 	 
		else
			return 0;
	}
	else if( entityClass == BODY_TYPE )
	{
		if( eclass == BODY_TYPE )
			return entity;
		else
			return 0;
	}
	else
		return 0;
}

bool CModellerInfo::ask_bodies(int * const nbodies, long** const body_indices)
{
	*nbodies = m_objects[0]->m_numchildren;
	if (body_indices)
	{
		*body_indices = new long[*nbodies];
		for (int i=0;i<(*nbodies);i++)
			(*body_indices)[i] = m_objects[0]->m_children[i];
	}
	return true;
}

bool CModellerInfo::BODY_ask_faces(long body_index, int * const nfaces, long** const face_indices)
{
	*nfaces = m_objects[body_index]->m_numchildren;
	if (face_indices)
	{
		*face_indices = new long[*nfaces];
		for (int i=0;i<(*nfaces);i++)
			(*face_indices)[i] = m_objects[body_index]->m_children[i];
	}
	return true;
}

bool CModellerInfo::FACE_ask_body(long face_index, long* const body_index)
{
	*body_index = m_objects[face_index]->m_parents[0];
	return true;
}

bool CModellerInfo::EDGE_ask_faces(long edge_index, int * const nfaces, long ** const face_indices)
{
	*nfaces = ((CEdgeModellerObject *)m_objects[edge_index])->m_numparents;
	if (face_indices)
	{
		*face_indices = new long[*nfaces];
		for (int i=0;i<(*nfaces);i++)
			(*face_indices)[i] = m_objects[edge_index]->m_parents[i];
	}
	return true;
}

bool CModellerInfo::FACE_ask_edges(long face_index, int * const nedges, long** const edge_indices)
{
	*nedges = m_objects[face_index]->m_numchildren;
	if (edge_indices)
	{
		*edge_indices = new long[*nedges];
		for (int i=0;i<(*nedges);i++)
			(*edge_indices)[i] = m_objects[face_index]->m_children[i];
	}
	return true;
}

bool CModellerInfo::ENTITY_ask_class(long entity_index, int   & eclass)
{
	eclass = m_objects[entity_index]->AskClass();
	return true;
}


bool CModellerInfo::ENTITY_ask_identifier(long entity_index, int *ident)
{
	*ident = m_objects[entity_index]->m_identifier;
	return true;
}

 
void CModellerInfo::SetModellerObject (int pos, CModellerObject * object)  
{
	if (m_objects[pos] != 0)
		delete m_objects[pos];
	m_objects[pos] = object;
}

CModellerObject * CModellerInfo::GetModellerObject (int pos)  
{
	return( m_objects[pos]);
}

// uses standard WIN apis to generate a temp filename in a temp directory
static void generate_temp_filename(TCHAR * retFilename)
{
	TCHAR temp_dir[_MAX_DIR];
	DWORD dir_len = GetTempPath( _MAX_DIR,  temp_dir );
	assert( dir_len != 0);	assert( dir_len <= _MAX_DIR);
	UINT res = GetTempFileName( temp_dir, _T("HOOPS"), 0, retFilename);
	assert( res != 0);
}

void	TK_PSUser_Data::putdata(void *data, int size)
{
	if (m_size+size>=m_buffer_size)
	{
		Resize(m_buffer_size+size+8192);
	}
	memcpy(&m_data[m_size],data,size);
	m_size+=size;
}

void TK_PSUser_Data::Reset()
{
	m_pos = 0;
	TK_User_Data::Reset();
}

void	TK_PSUser_Data::getdata(long &data)
{
	data = *((int *)(&m_data[m_pos]));	
	m_pos+=sizeof(long);
}

void	TK_PSUser_Data::getdata(char &data)
{
	data = m_data[m_pos];	
	m_pos++;
}

void	TK_PSUser_Data::getdata(void *data,int len)
{
	memcpy(data, &m_data[m_pos],len);	
	m_pos+=len;
}

TK_PSUser_Data::TK_PSUser_Data (CModellerInfo **mi, ENTITY_LIST& ent_list, bool psf) : TK_User_Data(), m_entityList(ent_list)
{
	m_mi = new CModellerInfo;
	*mi = m_mi;
	m_pos = 0;
	m_mi->m_bPSF = psf;
	m_mystage = 0;
	m_psf_filesize = 0;
	m_psf_filehandle = 0;
	m_psf_file_buff = 0;
	m_psf_buff_count = 0;

}

TK_Status TK_PSClose_Segment::Write(BStreamFileToolkit &tk) 
{	TK_Status status = TK_Normal;

	switch( m_mystage ) {
		case 0: {
			if((status = HTK_Close_Segment::Write(tk)) != TK_Normal )
				return status;
			m_mystage++;
		} nobreak;
		case 1: {
			//segment can have lods, so make sure this one is recorded as LOD 0.
			if((status = Tag( tk, 0 )) != TK_Normal )
				return status;
			m_mystage = 0;
		} break;
	}
	return (status);
}

TK_Status TK_PSLine::Write(BStreamFileToolkit &tk) 
{
	TK_Status status = TK_Normal;

	switch( m_mystage ) {
		case 0: {
			if((status = HTK_Line::Write(tk)) != TK_Normal )
				return status;
			m_mystage++;
		} nobreak;
		case 1: {
			if((status = Tag( tk, -1 )) != TK_Normal )
				return status;
			m_mystage = 0;
		} break;
	}
	return (status);
}

TK_Status TK_PSPolypoint::Write(BStreamFileToolkit &tk) 
{
	TK_Status status = TK_Normal;

	switch( m_mystage ) {
		case 0: {
			if((status = HTK_Polypoint::Write(tk)) != TK_Normal )
				return status;
			m_mystage++;
		} nobreak;
		case 1: {
			if((status = Tag( tk, -1 )) != TK_Normal )
				return status;
			m_mystage = 0;
		} break;
	}
	return (status);
}

TK_Status TK_PSPolyPolypoint::Write(BStreamFileToolkit &tk) 
{
	TK_Status status = TK_Normal;

	switch( m_mystage ) {
		case 0: {
			if((status = HTK_PolyPolypoint::Write(tk)) != TK_Normal )
				return status;
			m_mystage++;
		} nobreak;
		case 1: {
			if((status = Tag( tk, -1 )) != TK_Normal )
				return status;
			m_mystage = 0;
		} break;
	}
	return (status);
}

TK_Status TK_PSEllipse::Write(BStreamFileToolkit &tk) 
{
	TK_Status status = TK_Normal;

	switch( m_mystage ) {
		case 0: {
			if((status = HTK_Ellipse::Write(tk)) != TK_Normal )
				return status;
			m_mystage++;
		} nobreak;
		case 1: {
			if((status = Tag( tk, -1 )) != TK_Normal )
				return status;
			m_mystage = 0;
		} break;
	}
	return (status);
}

TK_Status TK_PSCircle::Write(BStreamFileToolkit &tk) 
{
	TK_Status status = TK_Normal;

	switch( m_mystage ) {
		case 0: {
			if((status = HTK_Circle::Write(tk)) != TK_Normal )
				return status;
			m_mystage++;
		} nobreak;
		case 1: {
			if((status = Tag( tk, -1 )) != TK_Normal )
				return status;
			m_mystage = 0;
		} break;
	}
	return (status);
}

TK_Status TK_PSComment::Execute (BStreamFileToolkit & tk) 
{
	// check if this is us, if yes, we should install our 
	// user data opcode handler
	if (!strcmp (m_comment, FILE_COMMENT_ID))
	{
		tk.SetOpcodeHandler( TKE_Start_User_Data,new TK_PSUser_Data(&((HHOOPSModel *)m_pHModel)->m_mi, ((HHOOPSModel *)m_pHModel)->GetEntityList()));
	}
	return TK_Comment::Execute(tk);
};

TK_Status TK_PSUser_Data::Read (BStreamFileToolkit & tk) alter 
{
	//auto        TK_Status       status = TK_Normal;
	TK_Status       status = TK_Normal;
	switch (m_stage) {
		case 0: {
			// get the total opcode size - which we really don't need 
			if ((status = GetData (tk, m_size)) != TK_Normal)
				return status;

			m_stage++;
		}   nobreak;
		case 1: {
			// get the topology block size
			if ((status = GetData (tk, m_size)) != TK_Normal)
				return status;
			set_data (m_size);      // allocate space

			m_stage++;
		}   nobreak;

		case 2: {
			// get the topology data
			if ((status = GetData (tk, m_data, m_size)) != TK_Normal)
				return status;
			m_stage++;
		}   nobreak;

		case 3: {
			// check if we have got the modeler file attached. It is the 5th byte from start
			if( m_data[4] )
			{
				int filesize = 0;
				if ((status = GetData (tk, filesize )) != TK_Normal)
					return status;

				m_psf_filesize = filesize;

				// create a temporary file to write the modeler file
				generate_temp_filename(m_psf_tempfilename.GetBuffer(_MAX_PATH));
				m_psf_tempfilename.ReleaseBuffer();

				// prepare to write
				m_psf_filehandle = _tfopen(m_psf_tempfilename, _T("wb"));
				if (!m_psf_filehandle)
				{
					TCHAR msg[MVO_BUFFER_SIZE];
					_stprintf(msg, _T("Failed to create a temporary modeler file - %s"), m_psf_tempfilename );
					AfxMessageBox(msg);
					return TK_Error;
				}
				m_psf_buff_count = 0;
				m_psf_file_buff = new char[PSF_BUFF_CHUNK_SIZE];

			}
			m_stage++;

		} nobreak;

		case 4: {
			if( m_data[4] )
			{
				// write the file out in chunks. As the file could get too big to read it in a 
				// single buffer
				while (m_psf_buff_count < m_psf_filesize)
				{
					int next_block = m_psf_filesize - m_psf_buff_count;
					if( next_block > PSF_BUFF_CHUNK_SIZE )
						next_block = PSF_BUFF_CHUNK_SIZE;

					if ((status = GetData (tk, m_psf_file_buff, next_block)) != TK_Normal)
						return status;
					fwrite(m_psf_file_buff, sizeof(char), next_block, m_psf_filehandle);
					m_psf_buff_count += next_block;
				}
				fclose(m_psf_filehandle);
				H_SAFE_DELETE_ARRAY(m_psf_file_buff);
			}
			m_stage++;
		} nobreak

		case 5: {
			//auto        unsigned char       stop_code;
			unsigned char       stop_code;
			if ((status = GetData (tk, stop_code)) != TK_Normal)
				return status;

			if (stop_code != TKE_Stop_User_Data)    // sanity check
				return tk.Error();

			m_stage = -1;
		}   break;

		default:
			return tk.Error();
	}

	return status;
}

TK_Status TK_PSUser_Data::Execute (BStreamFileToolkit & tk) 
{
 
	long numbodies,numentities;
	getdata(numentities);
	m_mi->Init(numentities+1);
	getdata((char &)m_mi->m_bPSF);
	getdata(numbodies);
 
	long counter=0;
	long index;
	HC_KEY key;
	 
	CBaseModellerObject *baseobj = new CBaseModellerObject;
	m_mi->SetModellerObject(counter++,baseobj);
	int basechildnum=0;
	baseobj->SetChildNum(numbodies);
	
	for (int i=0;i<numbodies;i++)
	{
		CBodyModellerObject *bodyobj = new CBodyModellerObject;
		if (m_mi->m_bPSF)
			getdata(&bodyobj->m_identifier,sizeof(int));

		getdata(index);
		tk.IndexToKey(index,key);
		m_mi->m_map.SetAt((long&)key,counter);
		int bodyparent = counter;
		baseobj->m_children[basechildnum++] = counter;	
		m_mi->SetModellerObject(counter++,bodyobj);
		bodyobj->SetKey(key);
		long numfaces;
		getdata(numfaces);
		bodyobj->SetChildNum(numfaces);
		int bodychildnum=0;
		for (int j=0;j<numfaces;j++)
		{
			CFaceModellerObject *faceobj = new CFaceModellerObject;
			bodyobj->m_children[bodychildnum++] = counter;	
			faceobj->SetParent(bodyparent);		
			if (m_mi->m_bPSF)
				getdata(&faceobj->m_identifier,sizeof(int));
			getdata(index);
			if (index)
			{
				m_mi->m_bTesselatedFaces = true;			
				tk.IndexToKey(index,key);
			}
			else
				key=0;

			m_mi->m_map.SetAt((long&)key,counter);
			int faceparent = counter;

			m_mi->SetModellerObject(counter++,faceobj);
		
			faceobj->SetKey(key);
			long numedges;
			getdata(numedges);
			faceobj->SetChildNum(numedges);
			int facechildnum=0;
			for (int k=0;k<numedges;k++)
			{
				char keynum;
				getdata(keynum);
				if (keynum != -1)
				{
				
					CEdgeModellerObject *edgeobj = new CEdgeModellerObject;
					if (m_mi->m_bPSF)
						getdata(&edgeobj->m_identifier,sizeof(int));

					edgeobj->AddParent(faceparent);		

					faceobj->m_children[facechildnum++] = counter;
					m_mi->SetModellerObject(counter,edgeobj);
					edgeobj->SetKeyNum(keynum);
					for (int l=0;l<keynum;l++)
					{
						getdata(index);
						if (index)
							tk.IndexToKey(index,key);
						else
							key=0;
						m_mi->m_map.SetAt((long&)key,counter);
						edgeobj->SetKey(l,key);
					}
					counter++;
				}
				else
				{
					long edgepointer;
					getdata(edgepointer);
					CEdgeModellerObject * temp = (CEdgeModellerObject *)m_mi->GetModellerObject(edgepointer+1);
					temp->AddParent(faceparent);
					faceobj->m_children[facechildnum++] = edgepointer+1;

				}
			}
		}

	}
	if (m_mi->m_bPSF)
		ExecutePS(tk);
	m_mi->m_bIsValid= true;
	 return TK_Normal;       // we don't do anything with it by default

}


TK_Status   TK_PSUser_Data::ExecutePS(BStreamFileToolkit & tk)
{

	// we should already have a temporary file written out in the Read function
	// read this file using ACIS api and populate entity list
	FILE* fp;
	ENTITY* entity = 0;
	outcome o;
	fp = _tfopen(m_psf_tempfilename, _T("rb"));
	if( !fp )
	{
		AfxMessageBox(_T("Failed to read ACIS data. The data may be corrupt"));
		return TK_Error;
	}

	// Let ACIS know that we need the tags here to re-create our ACIS-HOOPS entity co-relation
	restore_tags.set(TRUE);

	HISTORY_STREAM_LIST hslist;
	DELTA_STATE_LIST dslist;
	o = api_restore_entity_list_with_history ( fp, FALSE, m_entityList, hslist, dslist);
	check_outcome(o);
	o = api_set_default_history( hslist[0] );
	check_outcome(o);

	if( m_entityList.iteration_count() < 1)
	{
		AfxMessageBox(_T("No ACIS entities found in the ACIS data"));
		return TK_Error;
	}
	fclose(fp);
	// get rid of temp file
	int iRes = _tunlink( m_psf_tempfilename );	assert( iRes != -1 );

	// for each body, face and edge, set the bridge associativity
	m_entityList.init();
	ENTITY_LIST bodiesPS = m_entityList;
	bodiesPS.init();
	long *bodies;
	int numbodies;
	int ident;
	m_mi->ask_bodies(&numbodies,&bodies);
	for (int i=0;i<numbodies;i++)
	{
		int body_id;
		m_mi->ENTITY_ask_identifier(bodies[i],&body_id);
		ENTITY * body;
		outcome o = api_get_entity_from_id ( body_id, body, NULL);
		check_outcome(o);
		assert(body);
		HC_KEY body_key;
		int numkeys = m_mi->Compute_Geometry_Keys(bodies[i], 1, &body_key);
		if( numkeys <= 0 )
			continue;

		HA_Associate_Key_To_Entity(body,body_key);

		long *faces;
		int numfaces;
		m_mi->BODY_ask_faces(bodies[i],&numfaces,&faces);
		for (int j=0;j<numfaces;j++)
		{
			m_mi->ENTITY_ask_identifier(faces[j],&ident);
			ENTITY * face;
			outcome o = api_get_entity_from_id ( ident, face, NULL);
			check_outcome(o);
			assert(face);
			HC_KEY keys[1000];
			int numkeys = m_mi->Compute_Geometry_Keys(faces[j], 1000, keys);
			for (int knum=0;knum<numkeys;knum++)
				HA_Associate_Key_To_Entity(face,keys[knum]);
			
			long *edges;
			int numedges;
			m_mi->FACE_ask_edges(faces[j],&numedges,&edges);
			for (int k=0;k<numedges;k++)
			{
				m_mi->ENTITY_ask_identifier(edges[k],&ident);
				ENTITY * edge;
				outcome o = api_get_entity_from_id (  ident, edge, NULL);
				HC_KEY keys[1000];
				int numkeys = m_mi->Compute_Geometry_Keys(edges[k], 1000, keys);
				for (int knum=0;knum<numkeys;knum++)
					HA_Associate_Key_To_Entity(edge,keys[knum]);
			}
			delete edges;
		}
		delete faces;
	}
	delete bodies;
 
	return TK_Normal;						
}
 
TK_Status   TK_PSUser_Data::Write (BStreamFileToolkit & tk)
{
	//auto        TK_Status       status = TK_Normal;
	TK_Status       status = TK_Normal;
	switch (m_stage) {
		case 0: {
			// user data coming
			if ((status = PutOpcode (tk, 0)) != TK_Normal)
				return status;
			m_stage++;
		}   nobreak;

		case 1: {
			// do we need to write modeler file as well? Yes, then get going
			if (m_mi->m_bPSF)
				InterpretPS(tk);
			m_stage++;
		}   nobreak;

		case 2: {
			// write total opcode size which really two blocks
			// block 1 is topology information plus it's size
			// block 2 is modeler file (if required) it's size
			long size = m_size + sizeof(int);
			if( m_psf_filesize )
				size += m_psf_filesize + sizeof(int);
			if ((status = PutData (tk, size)) != TK_Normal)
				return status;
			m_progress = 0;
			m_stage++;
		}   nobreak;

		case 3: {
			// write out the size of first block which is topology info
			if ((status = PutData (tk, m_size)) != TK_Normal)
				return status;
			m_stage++;
		}   nobreak;

		case 4: {
			// write out the topology info itself
			if ((status = PutData (tk, m_data, m_size)) != TK_Normal)
				return status;
			m_stage++;
		}   nobreak;

		case 5: {
			if( m_psf_filesize ) {
				// write out the size of second block which is modeler file
				if ((status = PutData (tk, m_psf_filesize)) != TK_Normal)
					return status;

				// prepare to read the temporary file
				m_psf_filehandle = _tfopen(m_psf_tempfilename, _T("rb"));
				if( !m_psf_filehandle )
					return tk.Error("Unable to open temporary modeler file");

				m_psf_file_buff = new char[PSF_BUFF_CHUNK_SIZE];
				m_psf_buff_count = 0;
			}
			m_stage++;
		}   nobreak;
		case 6: {
			if( m_psf_filesize ) {
				// read it in chunks - we have seen upto 200MB monster modeler files 
				// we can't swallow it in one bite at all - Rajesh B (22-Jun-04)
				if(m_psf_buff_count)
				{
					// we reached eof but were unable to PutData last time.
					if ((status = PutData (tk, m_psf_file_buff, m_psf_buff_count)) != TK_Normal)
						return status;

					m_psf_buff_count = 0;
				}

				// read file in chunsk and dump it
				while( !feof( m_psf_filehandle ) )
				{
					if(!m_psf_buff_count)
					{
						m_psf_buff_count = fread( m_psf_file_buff, sizeof( char ), PSF_BUFF_CHUNK_SIZE, m_psf_filehandle);
						if(ferror( m_psf_filehandle ) != 0)
							return tk.Error("Failed to read temporary modeler file");
					}

					if ((status = PutData (tk, m_psf_file_buff, m_psf_buff_count)) != TK_Normal)
						return status;

					m_psf_buff_count = 0;
				}
				
				fclose( m_psf_filehandle );
				H_SAFE_DELETE_ARRAY(m_psf_file_buff);

				// get rid of the temporary file
				int iRes = _tunlink( m_psf_tempfilename );				assert( iRes != -1 );
			}
			m_stage++;
		}   nobreak;


		case 7: {
			// huf! done with that....
			if ((status = PutData (tk, (unsigned char)TKE_Stop_User_Data)) != TK_Normal)
				return status;
 
			m_stage = -1;
		}   break;

		default:
			return tk.Error();
	}

	return status;
}


TK_Status   TK_PSUser_Data::Interpret (BStreamFileToolkit & tk, long dummy, int lod)
{ 
	outcome o;
	int numbodies,numfaces,num_edges;
	int index = -1;
	HC_KEY key = INVALID_KEY;
	TK_Status tk_stat = TK_Normal;
	ENTITY_LIST bodies = m_entityList;
	numbodies = bodies.iteration_count();
	m_size = 0;								
	int counter = 0;
	putdata(&counter,sizeof(long));						//leave room for number of entities
	putdata(&m_mi->m_bPSF,sizeof(m_mi->m_bPSF));
	putdata(&numbodies,sizeof(long));					//store:number of bodies
	
	for (int i=0;i<numbodies;i++)
	{		
		// ACIS bridge supports things like light, text. So, handle those differently.
		if( is_BODY( bodies[i] ) )
		{
			if( HA_Compute_Geometry_Keys(bodies[i], 1, &key, "bodies") )
			{
				if( key >= 0 )
				{
					HC_KEY orig_key_no = key;
					key = HC_KRenumber_Key(key, -1, "global");
					tk_stat = tk.KeyToIndex(key,index);
					assert( tk_stat == TK_Normal );
					HC_Renumber_Key(key, orig_key_no, "global");
				}
				else
				{
					tk_stat = tk.KeyToIndex(key,index);
					assert( tk_stat == TK_Normal );
				}
			}
			else
				index = 0;
			
			if (m_mi->m_bPSF)
			{
				int ident;
				outcome o = api_get_entity_id ( bodies[i], ident ); 
				check_outcome(o);
				putdata(&ident,sizeof(int));					//store:identifier number if PSF file
			}
			putdata(&index,sizeof(long));					//store:index to HOOPS entity for body
			
			ENTITY_LIST faces;
			o = api_get_faces( bodies[i], faces );
			CHECK_OUTCOME(o);
			numfaces = faces.count();
			putdata(&numfaces,sizeof(long));				//store:number of faces in body
			counter++;
			for (int j=0;j<numfaces;j++)
			{
				
				if (HA_Compute_Geometry_Keys(faces[j], 1, &key, "faces"))
				{
					if( key >= 0 )
					{
						HC_KEY orig_key_no = key;
						key = HC_KRenumber_Key(key, -1, "global");
						tk_stat = tk.KeyToIndex(key,index);
						assert( tk_stat == TK_Normal );
						HC_Renumber_Key(key, orig_key_no, "global");
					}
					else
					{
						tk_stat = tk.KeyToIndex(key,index);
						assert( tk_stat == TK_Normal );
					}
				}
				else
					index = 0;
				if (m_mi->m_bPSF)
				{
					int ident;
					outcome o = api_get_entity_id ( faces[j], ident ); 
					check_outcome(o);
					putdata(&ident,sizeof(int));					//store:identifier number if PSF file
				}
				
				putdata(&index,sizeof(long));					//store:index to HOOPS entity for face			
				
				ENTITY_LIST	edges;
				o = api_get_edges( faces[j], edges);
				num_edges = edges.count();
				putdata(&num_edges,sizeof(long));
				counter++;
				for (int k=0;k<num_edges;k++)
				{
					{
						HC_KEY keys[128];
						char num_keys = (char)HA_Compute_Geometry_Keys(edges[k], 128, keys, "edges");					
						putdata(&num_keys,sizeof(char));
						if (m_mi->m_bPSF)
						{
							int ident;
							outcome o = api_get_entity_id ( edges[k], ident ); 
							check_outcome(o);
							
							putdata(&ident,sizeof(int));					//store:identifier number if PSF file
						}
						
						for (int l=0;l<num_keys;l++)
						{
							if( keys[l] >= 0 )
							{
								HC_KEY orig_key_no = keys[l];
								keys[l] = HC_KRenumber_Key(keys[l], -1, "global");
								if ((tk_stat = tk.KeyToIndex(keys[l],index)) != TK_Normal)
								{
									assert( tk_stat == TK_Normal );
									index = 0;
								}
								HC_Renumber_Key(keys[l], orig_key_no, "global");
							}
							else
							{
								if ((tk_stat = tk.KeyToIndex(keys[l],index)) != TK_Normal)
								{
									assert( tk_stat == TK_Normal );
									index = 0;
								}
							}
							
							putdata(&index,sizeof(long));
						}
						
						counter++;
					}
					
				}
			}
			
		}
		// Fang.Chen@Spatial: 03-24-05
		// If the ACIS entity is not a body (it could be LIGHT, WCS and TEXT), 
		// then use the HA_Compute_Geometry_Keys function without geom_type argument
		// to get the keys associated with the entity
		else
		{
			if( HA_Compute_Geometry_Keys(bodies[i], 1, &key) )
			{
				if( key >= 0 )
				{
					HC_KEY orig_key_no = key;
					key = HC_KRenumber_Key(key, -1, "global");
					tk_stat = tk.KeyToIndex(key,index);
					assert( tk_stat == TK_Normal );
					HC_Renumber_Key(key, orig_key_no, "global");
				}
				else
				{
					tk_stat = tk.KeyToIndex(key,index);
					assert( tk_stat == TK_Normal );
				}
			}
			else
				index = 0;
			
			if (m_mi->m_bPSF)
			{
				int ident;
				outcome o = api_get_entity_id ( bodies[i], ident ); 
				check_outcome(o);
				putdata(&ident,sizeof(int));					//store:identifier number if PSF file
			}
			putdata(&index,sizeof(long));					//store:index to HOOPS entity
			
			// set the numfaces to zero to indicate to read that this is not a body and do not
			// expect faces coming
			numfaces = 0;
			putdata(&numfaces,sizeof(long));				
			counter++;
		}
	}
	
	// rewrite the counter at the begining where we left the place holder
	int temp = m_size;
	m_size =0;
	putdata(&counter,sizeof(long));
	m_size = temp;
	
	return TK_Normal;
}

TK_Status   TK_PSUser_Data::InterpretPS(BStreamFileToolkit & tk, long dummy, int lod)
{
	// for now set up a limitation of being able to store only one model
	if( m_entityList.iteration_count() < 1 )
	{
		AfxMessageBox(_T("No entities present in this model"));
		return TK_Error;
	}

	// output a temporary file for the given model
	generate_temp_filename(m_psf_tempfilename.GetBuffer(_MAX_PATH));
	m_psf_tempfilename.ReleaseBuffer();

	FILE* fp;
	outcome o;
	fp = _tfopen(m_psf_tempfilename, _T("wb"));
	assert(fp);
	if (!fp)
	{
		TCHAR msg[MVO_BUFFER_SIZE];
		_stprintf(msg, _T("Failed to create a temporary SAT file - %s"), m_psf_tempfilename );
		AfxMessageBox(msg);
		return TK_Error;
	}

	// NOTE: WE MUST SAVE THE FILE WITH HISTORY TO HAVE PERSISTANT IDS STORED
	 HISTORY_STREAM* default_hs;
	 DELTA_STATE_LIST dslist;
	 o = api_get_default_history( default_hs );  
	 check_outcome(o);
	 HISTORY_STREAM_LIST hs_list;
	 hs_list.add(default_hs);

	// ACIS 6.3 requires this thing for saving files
	char id_string[MVO_BUFFER_SIZE];
	sprintf(id_string, "HOOPS-ACIS Part Viewer");
	FileInfo info;
	info.set_product_id(id_string);
	info.set_units(1.0);
	api_set_file_info((FileIdent | FileUnits), info);
	info.reset();
	o = api_save_entity_list_with_history(fp, FALSE, m_entityList, hs_list, dslist);
	check_outcome( o );
	hs_list.clear();
	fclose(fp);

	// get the size of the temp modeler file - we will dump it during the Write
	FILE * tmp_fh = 0;
	tmp_fh = _tfopen(m_psf_tempfilename, _T("rb"));		assert( tmp_fh);
	int int_fh = _fileno( tmp_fh );
	struct _stat buf;
	int iRes = _fstat( int_fh, &buf );		assert(iRes == 0);
	m_psf_filesize = buf.st_size;			assert(m_psf_filesize != -1L);
	fclose( tmp_fh );

	return TK_Normal;
}
void CHECK_OUTCOME( const outcome& result )
{
	if( !result.ok() ) 
	{
		char default_msg[MVO_BUFFER_SIZE];
		const char* error_string = find_err_mess( result.error_number() );
		if( (error_string == NULL ) || (*error_string == '\0')) 
		{
			sprintf(default_msg,"%d",result.error_number());
			error_string = default_msg;
		}
		MessageBox(NULL, H_TEXT(error_string), _T("ACIS Error"), MB_OK|MB_ICONSTOP|MB_APPLMODAL);
	}
}
