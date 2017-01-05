#pragma once
#include "hbasemodel.h"

#include "HDB.h"
#include "HBaseModel.h"
#include "HTools.h"

#include "ha_bridge.h"

class CModellerInfo;
class  HA_Part;
class ACISHoopsModel : public HBaseModel
{
public:
	ACISHoopsModel(void);
	~ACISHoopsModel(void);


	// Overrides
	HFileInputResult Read(const char * FileName);
	bool Write(const char * FileName, HBaseView * pHView, int version, int width = 0, int height = 0);

	void	DeleteAllEntities();

	bool	IsSolidModel(){ return m_bSolidModel; };

	void DeleteAcisEntity( ENTITY* entity);
	void AddAcisEntity( ENTITY* entity);
	void RemoveAcisEntity( ENTITY* entity);
	void UpdateAcisEntity( ENTITY* entity);
	void AddAcisEntities( const ENTITY_LIST& entityList);
	ENTITY_LIST& GetEntityList() { return m_entityList; }

	logical Undo();
	logical Redo();
	logical CanUndo();
	logical CanRedo();
	CModellerInfo *m_mi;

protected:

	// list of entities (bodies) in this model	
	ENTITY_LIST m_entityList; 

	HA_Part* m_pHAPart;

public:
	// do we have any solid modeler entities
	bool	m_bSolidModel;
};

