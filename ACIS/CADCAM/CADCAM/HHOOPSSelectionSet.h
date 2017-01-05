
#ifndef _HHOOPS10SelectionSet_H__20010203__0953__
#define _HHOOPS10SelectionSet_H__20010203__0953__

#include "HSelectionSet.h"

class HHOOPSSelectionSet :  public HSelectionSet 
{
public:
	HHOOPSSelectionSet(HBaseView* view);
	virtual ~HHOOPSSelectionSet();

	virtual void Select( HC_KEY key, int num_include_keys, HC_KEY * include_keys, bool emit_message = true);
	virtual void DeSelect(HC_KEY key, bool emit_message = true);

	virtual void SelectFromMessage( HC_KEY key, int num_include_keys, HC_KEY * include_keys, bool emit_message = false);
	virtual void DeSelectFromMessage(HC_KEY key, bool emit_message = false);

	virtual void Init();
	virtual void Reset();

	virtual void DeSelectAll ();
	virtual bool IsSelected(HC_KEY key); 	
	virtual long GetAtSolidEntity(int index);
	virtual int  GetSolidListSize();

	void Select(ENTITY* entity, bool emit_message = true);
	void DeSelectEntity(ENTITY* entity, bool emit_message = true);

	void SetSelectLevel(int new_level) { m_SelectLevel = new_level; }
	int GetSelectLevel() { return (m_SelectLevel);}

protected:
	int				m_SelectLevel;
	struct vlist_s *m_pSolidSelection;

};

#endif	// _HHOOPS08SelectionSet_H__20010203__0953__



