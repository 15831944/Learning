/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef __HPS_Map_ASM_H_
#define __HPS_Map_ASM_H_
#include "hps_acis.h" // Include this first, to avoid error C4244: '=' : conversion from 'double' to 'float', possible loss of data. Also avoids XOR macro warning.

#include "dcl_hps.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#include <map>
#pragma warning(pop)

//#define HPS_USE_STL
#ifdef HPS_USE_STL
#include <list>
#endif // HPS_USE_STL

#include "logical.h"

#include <assert.h>

#ifdef NT
#include <windows.h>
#endif // NT

#include "kernapi.hxx"

class asm_model;

// This will set some #defines, such as RWSTD_NO_NAMESPACE, used below
#ifdef hp700
#include <stdcomp.h>
#endif

#ifdef RWSTD_NO_NAMESPACE
#  define STD
#else
#  define STD std::
#endif

#include "hps_map.h"
class entity_handle;
class component_handle;

template<class T_value> class hps_list : public ACIS_OBJECT
{
public:
	class hps_list_node : public ACIS_OBJECT
	{
	public:
		hps_list_node( T_value value ) :
			m_Value( value ), m_Next( 0 ), m_Prev( 0 ) {}

		~hps_list_node()
		{
			//			if (m_Next)
			//				delete m_Next;
		}

		hps_list_node *&next() { return m_Next; }

		logical operator < ( const hps_list_node &node ) const { return m_Value < node.m_Value; }
		logical operator >( const hps_list_node &node ) const { return m_Value > node.m_Value; }
		logical operator == ( const hps_list_node &node ) const { return m_Value == node.m_Value; }

		T_value	m_Value;
		hps_list_node *m_Next;
		hps_list_node *m_Prev;

		class iterator : public ACIS_OBJECT
		{
		public:
			hps_list_node *m_Node;
			iterator() :m_Node( 0 ) {}
			iterator( hps_list_node *node ) :m_Node( node ) {}

			iterator &operator ++ ( )
			{
				if ( m_Node )
					m_Node = m_Node->m_Next;
				return *this;
			}
			iterator &operator -- ( )
			{
				if ( m_Node )
					m_Node = m_Node->m_Prev;
				return *this;
			}
			void operator =( hps_list_node *node ) { m_Node = node; }
			T_value &operator *( ) { return m_Node->m_Value; }
			//			T_value *operator ->() { return &(m_Node->m_Value);}

			logical operator !=( const iterator &iter ) const { return m_Node != iter.m_Node; }
			logical operator ==( const iterator &iter ) const { return m_Node == iter.m_Node; }
			logical operator ==( const hps_list_node *node ) const { return m_Node == node; }
		};
		class reverse_iterator : public ACIS_OBJECT
		{
		public:
			hps_list_node *m_Node;
			reverse_iterator() :m_Node( 0 ) {}
			reverse_iterator( hps_list_node *node ) :m_Node( node ) {}

			reverse_iterator &operator ++ ( )
			{
				if ( m_Node )
					m_Node = m_Node->m_Prev;
				return *this;
			}
			reverse_iterator &operator -- ( )
			{
				if ( m_Node )
					m_Node = m_Node->m_Next;
				return *this;
			}
			void operator =( hps_list_node *node ) { m_Node = node; }
			T_value &operator *( ) { return m_Node->m_Value; }
			//			T_value *operator ->() { return &(m_Node->m_Value);}

			logical operator !=( const reverse_iterator &iter ) const { return m_Node != iter.m_Node; }
			logical operator ==( const reverse_iterator &iter ) const { return m_Node == iter.m_Node; }
			logical operator ==( const hps_list_node *node ) const { return m_Node == node; }
		};

		class const_iterator : public ACIS_OBJECT
		{
		public:
			hps_list_node *m_Node;
			const_iterator() :m_Node( 0 ) {}
			const_iterator( hps_list_node *node ) :m_Node( node ) {}
			const_iterator( const iterator &iter ) :m_Node( iter.m_Node ) {}

			const_iterator &operator ++ ( )
			{
				if ( m_Node )
					m_Node = m_Node->m_Next;
				return *this;
			}
			const_iterator &operator -- ( )
			{
				if ( m_Node )
					m_Node = m_Node->m_Prev;
				return *this;
			}
			void operator =( hps_list_node *node ) { m_Node = node; }
			T_value &operator *( ) { return m_Node->m_Value; }
			//			T_value *operator ->() { return &(m_Node->m_Value);}

			logical operator !=( const const_iterator &iter ) const { return m_Node != iter.m_Node; }
			logical operator ==( const const_iterator &iter ) const { return m_Node == iter.m_Node; }
			logical operator ==( const hps_list_node *node ) const { return m_Node == node; }
		};
	};

	hps_list_node *m_Begin, *m_End;
	unsigned int m_Size;
public:
	typedef HPS_TYPENAME hps_list_node::iterator iterator;
	typedef HPS_TYPENAME hps_list_node::const_iterator const_iterator;
	typedef HPS_TYPENAME hps_list_node::reverse_iterator reverse_iterator;

	hps_list() :m_Begin( 0 ), m_End( 0 ), m_Size( 0 ) {};
	~hps_list()
	{
		clear();
	}

	T_value &front() { return m_Begin->m_Value; }
	T_value &back() { return m_End->m_Value; }
	T_value &top() { return back(); }

	void push_back( const T_value &value )
	{
		hps_list_node *new_node = new hps_list_node( value );
		if ( !m_Begin )
			m_Begin = new_node;
		if ( m_End )
		{
			m_End->next() = new_node;
			new_node->m_Prev = m_End;
		}
		m_End = new_node;
		m_Size++;
	}
	void push( const T_value &value )
	{
		push_back( value );
	}

	void push_front( const T_value &value )
	{
		hps_list_node *new_node = new hps_list_node( value );
		if ( !m_Begin )
			m_Begin = new_node;
		else
		{
			new_node->next() = m_Begin;
			m_Begin->m_Prev = new_node;
			m_Begin = new_node;
		}
		m_Size++;
	}

	void pop_back()
	{
		erase( iterator( m_End ) );
	}
	void pop()
	{
		pop_back();
	}

	void pop_front()
	{
		erase( iterator( m_Begin ) );
	}

	iterator begin() { return iterator( m_Begin ); }
	iterator end() { return iterator( 0 ); }

	const_iterator begin() const { return iterator( m_Begin ); }
	const_iterator end() const { return const_iterator( 0 ); }

	reverse_iterator rbegin() { return reverse_iterator( m_End ); }
	reverse_iterator rend() { return reverse_iterator( 0 ); }

	void clear()
	{
		while ( m_Begin )
		{
			hps_list_node *node_to_delete = m_Begin;
			m_Begin = m_Begin->next();
			node_to_delete->m_Next = 0;
			node_to_delete->m_Prev = 0;
			delete node_to_delete;
		}

		m_Begin = 0;
		m_End = 0;
		m_Size = 0;
	}

	unsigned int size() const { return m_Size; }
	logical empty() const { return m_Begin == 0; }
	iterator find( const T_value &value )
	{
		if ( !m_Begin ) return 0;
		iterator iter = m_Begin;
		while ( iter != end() && ( iter.m_Node->m_Value != value ) )
			++iter;

		return iter;
	}

	void erase( const iterator &i )
	{
		iterator del_iter = i;
		if ( del_iter == end() )
			return;

		if ( del_iter.m_Node == m_Begin )
		{
			m_Begin = m_Begin->next();
			if ( !m_Begin )
				m_End = 0;
		} else
		{
			iterator iter = m_Begin;
			while ( del_iter.m_Node != iter.m_Node->m_Next )
				++iter;
			if ( del_iter.m_Node == m_End )
				m_End = iter.m_Node;
			iter.m_Node->m_Next = del_iter.m_Node->m_Next;
			if ( del_iter.m_Node->m_Next )
				del_iter.m_Node->m_Next->m_Prev = iter.m_Node;
		}
		del_iter.m_Node->m_Next = 0;
		delete del_iter.m_Node; del_iter.m_Node = 0;
		m_Size--;
	}
	void remove( const T_value &value )
	{
		erase( find( value ) );
	}
	void sort()
	{
		hps_list_node *temp_list_begin = m_Begin;
		m_Begin = m_End = 0;

		// Insert the dudes in the right order.
		while ( temp_list_begin )
		{
			// Get a node to insert into the sorted list.
			hps_list_node *node_to_insert = temp_list_begin;
			// Iterate to the next node.
			temp_list_begin = temp_list_begin->m_Next;
			// Make the node to insert "stand alone"
			node_to_insert->m_Next = 0;
			node_to_insert->m_Prev = 0;

			if ( !m_Begin )
			{
				m_Begin = m_End = node_to_insert;
			} else if ( *node_to_insert<*m_Begin || *node_to_insert == *m_Begin )
			{
				node_to_insert->m_Next = m_Begin;
				m_Begin->m_Prev = node_to_insert;
				m_Begin = node_to_insert;
			} else
			{
				iterator iter = m_Begin;
				while ( iter != end() && *node_to_insert > *iter )
					//				while (iter!=end() && *iter < *node_to_insert )
				{
					++iter;
				}
				// Node needs to be inserted at end.
				if ( iter == end() )
				{
					m_End->m_Next = node_to_insert;
					node_to_insert->m_Prev = m_End;
					m_End = node_to_insert;
				} else
				{
					node_to_insert->m_Next = iter.m_Node;
					node_to_insert->m_Prev = iter.m_Node->m_Prev;
					iter.m_Node->m_Prev->m_Next = node_to_insert;
					iter.m_Node->m_Prev = node_to_insert;
				}
			}
		}
	}
	void unique()
	{
		iterator next_unique = m_Begin;
		iterator next_node = next_unique;
		while ( next_unique != end() )
		{
			do
			{
				++next_node;
				if ( next_node != end() && *next_unique == *next_node )
				{
					next_unique.m_Node->m_Next = next_node.m_Node->m_Next;
					if ( next_unique.m_Node->m_Next )
						next_unique.m_Node->m_Next->m_Prev = next_unique.m_Node;
					next_node.m_Node->m_Next = 0;
					if ( m_End == next_node.m_Node )
						m_End = next_unique.m_Node;
					delete next_node.m_Node; next_node.m_Node = 0;
					next_node = next_unique;
					m_Size--;
				} else
				{
					break;
				}
			} while ( next_node != end() );
			++next_unique;
			next_node = next_unique;
		}
	}
};

#ifdef HPS_USE_STL
typedef hps_list<HPS::Key> T_segment_list;
#else
typedef hps_list<HPS::Key> T_segment_list;
#endif // HPS_USE_STL

typedef T_segment_list::const_iterator T_segment_list_const_iterator;
typedef T_segment_list::iterator T_segment_list_iterator;

typedef STD map<ENTITY*, HPS::KeyArray> T_entity_to_segment_list;
typedef T_entity_to_segment_list::const_iterator T_entity_to_segment_list_const_iterator;
typedef T_entity_to_segment_list::iterator T_entity_to_segment_list_iterator;

typedef STD map<entity_handle*, T_segment_list> T_ehandle_to_segment_list;
typedef T_ehandle_to_segment_list::const_iterator T_ehandle_to_segment_list_const_iterator;
typedef T_ehandle_to_segment_list::iterator T_ehandle_to_segment_list_iterator;

// TL (Top Level)
class HPS_TL_EHandleMap : public ACIS_OBJECT
{
public:
	T_ehandle_to_segment_list m_ACIS_to_HOOPS;

	~HPS_TL_EHandleMap()
	{
		Clear();
	}

	void Clear()
	{
		m_ACIS_to_HOOPS.clear();
	}

	void DeleteMapping( entity_handle* entity )
	{
		m_ACIS_to_HOOPS.erase( entity );
	} // Does this ever happen?

	void AddMapping( HPS::Key key, entity_handle* entity )
	{
		m_ACIS_to_HOOPS[entity].push_back( key );
	}

	void DeleteMapping( HPS::Key key, entity_handle* entity )
	{
		//		SPAUNUSED(key)
		//		DeleteMapping(entity);
		m_ACIS_to_HOOPS[entity].remove( key );
	}

	HPS::Key FindMapping( entity_handle* entity )
	{
		if ( m_ACIS_to_HOOPS.size() > 0 )
		{
			if ( m_ACIS_to_HOOPS[entity].size() > 0 )
			{
				return m_ACIS_to_HOOPS[entity].front();
			}
		}
		return HPS_INVALID_KEY;
	}

	int NumSegments( entity_handle* entity )
	{
		return m_ACIS_to_HOOPS[entity].size();
	}
};

typedef STD map <entity_handle*, HPS_TL_EHandleMap> T_ehandle_to_HPS_TL_EHandleMap;
typedef T_ehandle_to_HPS_TL_EHandleMap::const_iterator T_ehandle_to_HPS_TL_EHandleMap_const_iterator;
typedef T_ehandle_to_HPS_TL_EHandleMap::iterator T_ehandle_to_HPS_TL_EHandleMap_iterator;

class DECL_HPS HPS_EHandleMap : public ACIS_OBJECT
{
public:
	~HPS_EHandleMap()
	{
		m_HPS_to_ACIS.clear();
		m_Top_Maps.clear();
	}

	void AddMapping( HPS::Key key, entity_handle* entity, entity_handle* owner = NULL );

	// delete functions return FALSE if item to be deleted could not be found.
	void DeleteMapping( HPS::Key key );
	void DeleteMapping( entity_handle* entity );
	void DeleteMapping( HPS::Key key, entity_handle* entity );

	// returns zero if entity not found
	HPS::Key FindMapping( entity_handle* entity );

	// returns 0 if key not found
	entity_handle* FindMapping( HPS::Key key );

	// returns number of keys found (for multiple mappings)
	unsigned long FindMapping( entity_handle* entity, HPS::Key* keys, unsigned long count );

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings( entity_handle* entity );

	//	void DebugMaps(void);
	int HPS_Internal_Get_Map_Entries( void );
	logical HPS_Is_In_Top_Map( entity_handle* entity );

private:
	STD map<HPS::Key, entity_handle*> m_HPS_to_ACIS;
	STD map <entity_handle*, HPS_TL_EHandleMap> m_Top_Maps;
};

class DECL_HPS HPS_ModelMap : public ACIS_OBJECT
{
public:
	~HPS_ModelMap()
	{
		m_HPS_to_ACIS.clear();
	}

	void AddMapping( HPS::Key key, asm_model* model );

	void DeleteMapping( HPS::Key key );
	void DeleteMapping( asm_model* model );
	void DeleteMapping( HPS::Key key, asm_model* model );

	void Clear();

	// returns zero if entity not found
	HPS::Key FindMapping( asm_model* model );

	// returns 0 if key not found
	asm_model* FindMapping( HPS::Key key );

	// returns number of keys found (for multiple mappings)
	// Passing a NULL model results in all keys being returned
	unsigned long FindMapping( asm_model* model, HPS::Key* keys, unsigned long count );

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings( asm_model* model );

private:
	STD map<HPS::Key, asm_model*> m_HPS_to_ACIS;
};

class DECL_HPS HPS_CHandleMap : public ACIS_OBJECT
{
public:
	~HPS_CHandleMap()
	{
		m_HPS_to_ACIS.clear();
	}

	void AddMapping( HPS::Key key, component_handle* comp );

	void DeleteMapping( HPS::Key key );
	void DeleteMapping( component_handle* comp );
	void DeleteMapping( HPS::Key key, component_handle* comp );

	void Clear();

	// returns zero if entity not found
	HPS::Key FindMapping( component_handle* comp );

	// returns 0 if key not found
	component_handle* FindMapping( HPS::Key key );

	// returns number of keys found (for multiple mappings)
	// Passing a NULL model results in all keys being returned
	unsigned long FindMapping( component_handle* comp, HPS::Key* keys, unsigned long count );

	// returns number of keys found (for multiple mappings)
	unsigned long FindNumMappings( component_handle* comp );

private:
	STD map<HPS::Key, component_handle*> m_HPS_to_ACIS;
};

#endif // __HPS_Map_ASM_H_

