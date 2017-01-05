// Standard includes
#include "StdAfx.h"
#include <afxtempl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "ha_part.h"
#include "ha_part_util.h"
#include "roll_hm.hxx"
#include "kernapi.hxx"

// HOOPS includes
#include "hc.h"
#include "HStream.h"
#include "HOpcodeShell.h"
#include "HUtility.h"

#include "HHOOPSModel.h"
#include "HTools.h"

#include "HASFileToolkit.h"

HHOOPSModel::HHOOPSModel()
{
	m_bSolidModel = false;
	SetBRepGeometry(false);
	m_mi=NULL;

	m_entityList.clear();

	initialize_ha_part();
	m_pHAPart = ACIS_NEW HA_Part();

	PART* pPart = m_pHAPart->GetPart();

	//	pPart->attach_history_stream();
	distributed_history(TRUE,TRUE);

	char buffer[1024];
	char pbuffer[POINTER_BUFFER_SIZE];
	sprintf(buffer,"?Include Library/%s",ptoax(pPart->history_stream(), pbuffer));
	m_ModelKey = HA_KCreate_Segment(0, buffer);
	
	HA_Set_Rendering_Options("segment pattern = ?Include Library/history/entity");
}

HHOOPSModel::~HHOOPSModel()
{
	if(m_pHAPart)
	{
		ACIS_DELETE m_pHAPart;
		m_pHAPart = NULL;
	}
	terminate_ha_part();

	if(m_mi)
	{
		delete m_mi;
		m_mi = NULL;
	}

	m_entityList.clear();     //ÊÖ¶¯Ìí¼Ó

	// Model cleanup : delete all the entities from the 
	// partition associated with this model
	DeleteAllEntities();
}

// Delete all the entities in the current model from the 
// modeller and associated HOOPS geometry
void HHOOPSModel::DeleteAllEntities()
{
	HA_Delete_Entity_Geometry(m_entityList);
	api_del_entity_list(m_entityList);
	m_entityList.clear();
}

// 10/20/2010 mwu :  
void HHOOPSModel::AddAcisEntity( ENTITY* entity )
{
	if(m_pHAPart)
	{
		m_pHAPart->AddEntity(entity);
	}

	m_entityList.add(entity);
}

// 10/20/2010 mwu :  
void HHOOPSModel::RemoveAcisEntity( ENTITY* entity )
{
	if(m_pHAPart)
	{
		m_pHAPart->RemoveEntity(entity);
	}

	m_entityList.remove(entity);
}

void HHOOPSModel::RemoveAcisEntities(ENTITY_LIST lst)
{
	if(m_pHAPart)
	{
		ENTITY *entity = lst.first();
		while(entity != NULL)
		{
			m_pHAPart->RemoveEntity(entity);

			m_entityList.remove(entity);          //Add By LYA

			entity = lst.next();
		}
	}

}

// 10/20/2010 mwu :  
void HHOOPSModel::UpdateAcisEntity( ENTITY* entity )
{
	if(m_pHAPart)
	{
		m_pHAPart->UpdateEntity(entity);
	}
}

// 10/20/2010 mwu :  
void HHOOPSModel::AddAcisEntities( const ENTITY_LIST& entityList )
{
	if(m_pHAPart)
	{
		m_pHAPart->AddEntities(entityList);
	}

	//m_entityList.add(entityList);
}

void HHOOPSModel::DeleteAcisEntity( ENTITY* entity)
{
	ENTITY_LIST elist;
	elist.add(entity);

	// delete from HOOPS
	HA_Delete_Entity_Geometry(elist);

	// delete from ACIS 
	api_del_entity(entity);
	m_entityList.remove(entity);
}



// our application-specific read function
HFileInputResult HHOOPSModel::Read(const char * FileName) 
{   
	CWaitCursor show_hourglass_cursor_through_this_function;
	
	HFileInputResult success = InputOK;

	// get the file extension
	char extension[120]; 
	HUtility::FindFileNameExtension(FileName, extension);

	// read the file into the model object's model segment
	HC_Open_Segment_By_Key(m_ModelKey);
	//HC_Set_Visibility ("lines = off, edges = on");
	//HC_Open_Segment("main");

	if  (streq(extension,"sat"))
	{  
		m_bSolidModel = true;
		SetBRepGeometry(true);
		
		m_entityList.clear();

		HA_Read_Sat_File(FileName, m_entityList);	// read a SAT file		 
	} 
	else if (streq(extension,"asf"))
	{
		m_bSolidModel = true;
		SetBRepGeometry(true);

		HStreamFileToolkit tk;
		tk.SetOpcodeHandler (TKE_Comment, new TK_PSComment(this));
		TK_Status status = HTK_Read_Stream_File( FileName, &tk ); 
		if( status != TK_Complete )
			success = InputFail;
	}
	else
	{
		// No special read - let the base class handle
		m_bSolidModel = false;
		success = HBaseModel::Read(FileName);
	}

	//HC_Close_Segment();
	HC_Close_Segment();

	return success;
}

// our application-specific write function
bool HHOOPSModel::Write(const char * FileName, HBaseView * view, 
	int version, int width, int height) 
{   
	CWaitCursor show_hourglass_cursor_through_this_function;
	
	bool success = true;

	// get the file extension
	char extension[120]; 
	HUtility::FindFileNameExtension(FileName, extension);

	if (streq(extension,"sat"))
	{
		if (m_entityList.count() > 0)
			HA_Write_Sat_File(FileName, m_entityList);
	}
	else
	{
		if(!HBaseModel::Write(FileName, view, width, height))
			success = false;
	}

	return success;
}

logical HHOOPSModel::Undo()
{
	return m_pHAPart->RollBack();
}

logical HHOOPSModel::Redo()
{
	return m_pHAPart->RollForward();
}

logical HHOOPSModel::CanUndo()
{
	PART* pPart = m_pHAPart->GetPart();
	HISTORY_STREAM* hs = pPart->history_stream();
	if (hs)
		return (hs->can_roll_back()!=0);
	return false;
}

logical HHOOPSModel::CanRedo()
{
	PART* pPart = m_pHAPart->GetPart();
	HISTORY_STREAM* hs = pPart->history_stream();
	if (hs)
		return (hs->can_roll_forward()!=0);
	return false;
}


