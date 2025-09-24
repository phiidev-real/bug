
#ifndef MAN_H
#define MAN_H

#include <utility>
#include <iostream>

//I should have used a std::list, but I was scared of iterator stuff. 
//So anyways, this is a diy linked list memory management system for large pieces of data. 
//I probably should work up some implimentation that doesn't require a new manager for each type,
//but that kinda sounds like a pain. So yeah. 

namespace man {
	
	template <typename T> class manager;
	
	template <typename T>
	class node {
		
	public:
		template <class... Args>
		 node( Args&& ...args  ) : m_target( std::forward<Args>(args)... ) { }; //Modern C++ :/
		
		void Delete() {
			
			mp_last->mp_next = mp_last;
			mp_next->mp_last = mp_next;
			
		};
		
		     T	 m_target;
		node<T>* mp_last = nullptr;
		node<T>* mp_next = nullptr;
		
	};
	
	//Just a factory that comes bundled with a linked list for memory management. 
	template <typename T>
	class manager {
		
	public:
		template <typename... Args>
		T* Create(Args&& ...args)
		{
			
			node<T>* p_node = new node<T>( std::forward<Args>(args)... );
			
			//If there's no head, this becomes the new head
			if (mp_head == nullptr)
				mp_head = p_node;
			
			//Push this onto the tail. 
			if (mp_tail != nullptr)
				mp_tail->mp_next = p_node;
			
			p_node->mp_last = mp_tail;
			mp_tail = p_node;
			
			return &p_node->m_target;
			
		}
		
		void Delete(T* ap_handle)
		{
			
			//I love ptr hack YIPPIE YIPPIE YIPPIE YIPPIE YIPPIE YIPPIE
			node<T>* p_node = (node<T>*)( (void**)ap_handle + 2 );
			
			if (mp_head == p_node) mp_head = mp_head->mp_next;
			if (mp_tail == p_node) mp_tail = mp_tail->mp_last;
			
			if (p_node->mp_last != nullptr) p_node->mp_last->mp_next = p_node->mp_next;
			if (p_node->mp_next != nullptr) p_node->mp_next->mp_last = p_node->mp_last;
			
			delete p_node;
			
		}
		
		void Empty()
		{
			
			//If mp_head == nullptr, there's nothing to detele (I hope)
			if (mp_head == nullptr) return;
			
			//Very simple node traversal stuff
			node<T>* p_node = mp_head;
			for (;;)
			{
				
				if (p_node->mp_next == nullptr) break;
				p_node = mp_head->mp_next;
				delete p_node->mp_last;
				
			}
			
			delete p_node;
			mp_head = nullptr;
			mp_tail = nullptr;
			
		}
		
	private:
		node<T>* mp_head = nullptr;
		node<T>* mp_tail = nullptr;
		
	};
	
};

#endif //MAN_H

