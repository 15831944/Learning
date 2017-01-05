#ifndef _HHOOPS10Model_H
#define _HHOOPS10Model_H

#include "HDB.h"
#include "HBaseModel.h"
#include "HTools.h"

#include "ha_bridge.h"
class CModellerInfo;

class HA_Part;

class HHOOPSModel : public HBaseModel
{
public:

	HHOOPSModel();
	virtual ~HHOOPSModel();

	// Overrides
	HFileInputResult Read(const char * FileName);
	bool Write(const char * FileName, HBaseView * pHView, int version, int width = 0, int height = 0);

	void	DeleteAllEntities();

	bool	IsSolidModel(){ return m_bSolidModel; };

	void DeleteAcisEntity( ENTITY* entity);
	void AddAcisEntity( ENTITY* entity);
	void RemoveAcisEntity( ENTITY* entity);
	void UpdateAcisEntity( ENTITY* entity);
	void RemoveAcisEntities(ENTITY_LIST lst);
	void AddAcisEntities( const ENTITY_LIST& entityList);
	ENTITY_LIST& GetEntityList() { return m_entityList; }

	logical Undo();
	logical Redo();
	logical CanUndo();
	logical CanRedo();
	CModellerInfo *m_mi;

public:

	// list of entities (bodies) in this model	
	ENTITY_LIST m_entityList; 

	HA_Part* m_pHAPart;

public:
	// do we have any solid modeler entities
	bool	m_bSolidModel;
};

#endif




