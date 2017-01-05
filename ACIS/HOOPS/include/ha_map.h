/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HA_MAP_H_
#define __HA_MAP_H_
#include "hc.h"

#include "dcl_hoops.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#include <map>
#pragma warning(pop)

//#define HA_USE_STL
#ifdef HA_USE_STL
#include <list>
#endif // HA_USE_STL

#include "logical.h"

#include <assert.h>

#ifdef NT
#include <windows.h>
#endif // NT

#include "kernapi.hxx"

// This will set some #defines, such as RWSTD_NO_NAMESPACE, used below
#ifdef hp700
#include <stdcomp.h>
#endif

#ifdef RWSTD_NO_NAMESPACE
#  define STD
#else
#  define STD std::
#endif

#if defined(aix)
#define HA_TYPENAME
#else
#define HA_TYPENAME typename
#endif

template<class T_value> class ha_list: public ACIS_OBJECT
{
public:
	class ha_list_node: public ACIS_OBJECT
	{
	public:
		ha_list_node(T_value value):
		m_Value(value), m_Next(0), m_Prev(0){}

		~ha_list_node()
		{
//			if (m_Next)
//				delete m_Next;
		}

		ha_list_node *&next() { return m_Next;}

		logical operator < (const ha_list_node &node) const {return m_Value < node.m_Value;}
		logical operator > (const ha_list_node &node) const {return m_Value > node.m_Value;}
		logical operator == (const ha_list_node &node) const {return m_Value == node.m_Value;}
		
		T_value	m_Value;
		ha_list_node *m_Next;
		ha_list_node *m_Prev;

		class iterator: public ACIS_OBJECT
		{
			public:
			ha_list_node *m_Node;
			iterator ():m_Node(0){}
			iterator (ha_list_node *node):m_Node(node){}

			iterator &operator ++ () 
			{
				if (m_Node)
					m_Node=m_Node->m_Next;
				return *this;
			}
			iterator &operator -- () 
			{
				if (m_Node)
					m_Node=m_Node->m_Prev;
				return *this;
			}
			void operator =(ha_list_node *node) {m_Node=node;}
			T_value &operator *() { return m_Node->m_Value;}
//			T_value *operator ->() { return &(m_Node->m_Value);}

			logical operator !=(const iterator &iter) const {return m_Node!=iter.m_Node;}
			logical operator ==(const iterator &iter) const {return m_Node==iter.m_Node;}
			logical operator ==(const ha_list_node *node) const {return m_Node==node;}
		};
		class reverse_iterator: public ACIS_OBJECT
		{
			public:
			ha_list_node *m_Node;
			reverse_iterator ():m_Node(0){}
			reverse_iterator (ha_list_node *node):m_Node(node){}

			reverse_iterator &operator ++ () 
			{
				if (m_Node)
					m_Node=m_Node->m_Prev;
				return *this;
			}
			reverse_iterator &operator -- () 
			{
				if (m_Node)
					m_Node=m_Node->m_Next;
				return *this;
			}
			void operator =(ha_list_node *node) {m_Node=node;}
			T_value &operator *() { return m_Node->m_Value;}
//			T_value *operator ->() { return &(m_Node->m_Value);}

			logical operator !=(const reverse_iterator &iter) const {return m_Node!=iter.m_Node;}
			logical operator ==(const reverse_iterator &iter) const {return m_Node==iter.m_Node;}
			logical operator ==(const ha_list_node *node) const {return m_Node==node;}
		};

		class const_iterator: public ACIS_OBJECT
		{
			public:
			ha_list_node *m_Node;
			const_iterator ():m_Node(0){}
			const_iterator (ha_list_node *node):m_Node(node){}
			const_iterator (const iterator &iter):m_Node(iter.m_Node){}

			const_iterator &operator ++ () 
			{
				if (m_Node)
					m_Node=m_Node->m_Next;
				return *this;
			}
			const_iterator &operator -- () 
			{
				if (m_Node)
					m_Node=m_Node->m_Prev;
				return *this;
			}
			void operator =(ha_list_node *node) {m_Node=node;}
			T_value &operator *() { return m_Node->m_Value;}
//			T_value *operator ->() { return &(m_Node->m_Value);}

			logical operator !=(const const_iterator &iter) const {return m_Node!=iter.m_Node;}
			logical operator ==(const const_iterator &iter) const {return m_Node==iter.m_Node;}
			logical operator ==(const ha_list_node *node) const {return m_Node==node;}
		};
	};

	ha_list_node *m_Begin,*m_End;
	unsigned int m_Size;
public:
	typedef HA_TYPENAME ha_list_node::iterator iterator;
	typedef HA_TYPENAME ha_list_node::const_iterator const_iterator;
	typedef HA_TYPENAME ha_list_node::reverse_iterator reverse_iterator;

	ha_list():m_Begin(0),m_End(0),m_Size(0) {};
	~ha_list()
	{
		clear();
	}

	T_value &front() {return m_Begin->m_Value;}
	T_value &back() {return m_End->m_Value;}
	T_value &top() {return back();}

	void push_back(const T_value &value)
	{
		ha_list_node *new_node=new ha_list_node(value);
		if (!m_Begin)
			m_Begin=new_node;
		if (m_End)
		{
			m_End->next()=new_node;
			new_node->m_Prev=m_End;
		}
		m_End=new_node;
		m_Size++;
	}
	void push(const T_value &value)
	{
		push_back(value);
	}

	void push_front(const T_value &value)
	{
		ha_list_node *new_node=new ha_list_node(value);
		if (!m_Begin)
			m_Begin=new_node;
		else
		{
			new_node->next()=m_Begin;
			m_Begin->m_Prev=new_node;
			m_Begin=new_node;
		}
		m_Size++;
	}

	void pop_back()
	{
		erase(iterator(m_End));
	}
	void pop()
	{
		pop_back();
	}

	void pop_front()
	{
		erase(iterator(m_Begin));
	}

	iterator begin() {return iterator(m_Begin);}
	iterator end() {return iterator(0);}

	const_iterator begin() const {return iterator(m_Begin);}
	const_iterator end() const {return const_iterator(0);}

	reverse_iterator rbegin() {return reverse_iterator(m_End);}
	reverse_iterator rend() {return reverse_iterator(0);}

	void clear () 
	{
		while (m_Begin)
		{
			ha_list_node *node_to_delete=m_Begin;
			m_Begin=m_Begin->next();
			node_to_delete->m_Next=0;
			node_to_delete->m_Prev=0;
			delete node_to_delete;
		}

		m_Begin=0;
		m_End=0;
		m_Size=0;
	}
	
	unsigned int size() const {return m_Size;}
	logical empty() const {return m_Begin==0;}
	iterator find(const T_value &value)
	{
		if (!m_Begin) return 0;
		iterator iter=m_Begin;
		while (iter!=end() && (iter.m_Node->m_Value != value))
			++iter;

		return iter;
	}

	void erase(const iterator &i)
	{
		iterator del_iter=i;
		if (del_iter==end()) 
			return;

		if (del_iter.m_Node == m_Begin)
		{
			m_Begin=m_Begin->next();
			if (!m_Begin)
				m_End=0;
		}
		else
		{
			iterator iter=m_Begin;
			while (del_iter.m_Node != iter.m_Node->m_Next)
				++iter;
			if (del_iter.m_Node==m_End)
				m_End=iter.m_Node;
			iter.m_Node->m_Next=del_iter.m_Node->m_Next;
			if (del_iter.m_Node->m_Next)
				del_iter.m_Node->m_Next->m_Prev=iter.m_Node;
		}
		del_iter.m_Node->m_Next=0;
		delete del_iter.m_Node;del_iter.m_Node=0;
		m_Size--;
	}
	void remove(const T_value &value)
	{
		erase(find(value));
	}
	void sort()
	{
		ha_list_node *temp_list_begin=m_Begin;
		m_Begin=m_End=0;

		// Insert the dudes in the right order.
		while (temp_list_begin)
		{
			// Get a node to insert into the sorted list.
			ha_list_node *node_to_insert=temp_list_begin;
			// Iterate to the next node.
			temp_list_begin=temp_list_begin->m_Next;
			// Make the node to insert "stand alone"
			node_to_insert->m_Next=0;
			node_to_insert->m_Prev=0;

			if (!m_Begin)
			{
				m_Begin=m_End=node_to_insert;
			}
			else if (*node_to_insert<*m_Begin || *node_to_insert==*m_Begin)
			{
				node_to_insert->m_Next=m_Begin;
				m_Begin->m_Prev=node_to_insert;
				m_Begin=node_to_insert;
			}
			else
			{
				iterator iter=m_Begin;
				while (iter!=end() && *node_to_insert > *iter)
//				while (iter!=end() && *iter < *node_to_insert )
				{
					++iter;
				}
				// Node needs to be inserted at end.
				if (iter==end())
				{
					m_End->m_Next=node_to_insert;
					node_to_insert->m_Prev=m_End;
					m_End=node_to_insert;
				}
				else
				{
					node_to_insert->m_Next=iter.m_Node;
					node_to_insert->m_Prev=iter.m_Node->m_Prev;
					iter.m_Node->m_Prev->m_Next=node_to_insert;
					iter.m_Node->m_Prev=node_to_insert;
				}
			}
		}
	}
	void unique()
	{
		iterator next_unique=m_Begin;
		iterator next_node=next_unique;
		while (next_unique!=end())
		{
			do
			{
				++next_node;
				if (next_node!=end() && *next_unique==*next_node)
				{
					next_unique.m_Node->m_Next=next_node.m_Node->m_Next;
					if (next_unique.m_Node->m_Next)
						next_unique.m_Node->m_Next->m_Prev=next_unique.m_Node;
					next_node.m_Node->m_Next=0;
					if (m_End==next_node.m_Node)
						m_End=next_unique.m_Node;
					delete next_node.m_Node;next_node.m_Node=0;
					next_node=next_unique;
					m_Size--;
				}
				else
				{
					break;
				}
			} while (next_node!=end());
			++next_unique;
			next_node=next_unique;
		}
	}
};

#ifdef HA_USE_STL
typedef ha_list<HC_KEY> T_segment_list;
#else
typedef ha_list<HC_KEY> T_segment_list;
#endif // HA_USE_STL

typedef T_segment_list::const_iterator T_segment_list_const_iterator;
typedef T_segment_list::iterator T_segment_list_iterator;

typedef STD map<ENTITY*,T_segment_list> T_entity_to_segment_list;
typedef T_entity_to_segment_list::const_iterator T_entity_to_segment_list_const_iterator;
typedef T_entity_to_segment_list::iterator T_entity_to_segment_list_iterator;

// TL (Top Level)
class HA_TL_Map: public ACIS_OBJECT
{
public:
	T_entity_to_segment_list m_ACIS_to_HOOPS;

	~HA_TL_Map()
	{
		Clear();
	}

	void Clear() 
	{
		m_ACIS_to_HOOPS.clear();
	}
	
	void DeleteMapping(ENTITY* entity)
	{
		m_ACIS_to_HOOPS.erase(entity);
	} // Does this ever happen?
	
	void AddMapping(HC_KEY key, ENTITY* entity) 
	{
		m_ACIS_to_HOOPS[entity].push_back(key);
	}
	
	void DeleteMapping(HC_KEY key, ENTITY* entity) 
	{
//		SPAUNUSED(key)
//		DeleteMapping(entity);
		m_ACIS_to_HOOPS[entity].remove(key);
	}
	
	HC_KEY FindMapping(ENTITY* entity)
	{
		if (m_ACIS_to_HOOPS.size()>0)
		{
			if (m_ACIS_to_HOOPS[entity].size()>0)
			{
				return m_ACIS_to_HOOPS[entity].front();
			}
		}
		return 0;
	}
	
	int NumSegments(ENTITY* entity)
	{
		return m_ACIS_to_HOOPS[entity].size();
	}
};

typedef STD map <ENTITY*,HA_TL_Map> T_entity_to_HA_TL_Map;
typedef T_entity_to_HA_TL_Map::const_iterator T_entity_to_HA_TL_Map_const_iterator;
typedef T_entity_to_HA_TL_Map::iterator T_entity_to_HA_TL_Map_iterator;

class DECL_HOOPS HA_Map: public ACIS_OBJECT
{
public:
	~HA_Map()
	{
		m_HOOPS_to_ACIS.clear();
		m_Top_Maps.clear();
	}

	void AddMapping(HC_KEY key, ENTITY* entity, ENTITY* owner=NULL);

	// delete functions return FALSE if item to be deleted could not be found.
	void DeleteMapping(HC_KEY key);
	void DeleteMapping(ENTITY* entity);
	void DeleteMapping(HC_KEY key, ENTITY* entity);

	// returns zero if entity not found
	HC_KEY FindMapping(ENTITY* entity);

	// returns 0 if key not found
	ENTITY* FindMapping(HC_KEY key);

	// returns number of keys found (for multiple mappings)
	unsigned long FindMapping(ENTITY* entity, HC_KEY* keys, unsigned long count);

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings(ENTITY* entity);

	void DebugMaps(void);
	int HA_Internal_Get_Map_Entries(void);
	logical HA_Is_In_Top_Map( ENTITY* entity );

private:
	STD map<HC_KEY,ENTITY*> m_HOOPS_to_ACIS;
	STD map <ENTITY*,HA_TL_Map> m_Top_Maps;
};
#endif
