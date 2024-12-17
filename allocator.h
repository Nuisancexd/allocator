#ifndef _ALLOC_H_
#define _ALLOC_H_


#include <assert.h>
#include <stdio.h>

#ifndef _SYS_QUEUE_H_
#include "queue.h"
#endif


#if defined __GNUC__
#include <cstring>
#include <cstddef>
#include <cstdint>

#elif defined _MSC_VER

#else
error("Compiler of gcc or msc");
#endif

#ifndef ALLOCATED_CAPACITY
#define ALLOCATED_CAPACITY 10000
#endif


namespace alloc
{
#if defined _M_X64
	
#elif defined _M_IX86 && defined _MSC_VER
	void* m_memset(void* szBuffer, size_t dwSym, size_t dwLen)
	{
		if (!szBuffer)
		{
			return NULL;
		}

		__asm
		{
			pushad
			mov		edi, [szBuffer]
			mov		ecx, [dwLen]
			mov		eax, [dwSym]
			rep		stosb
			popad
		}

		return NULL;
	}

	void* m_memcpy(void* szBuf, const void* szStr, int nLen)
	{
		if (!szBuf || !szStr)
		{
			return NULL;
		}

		__asm
		{
			pushad
			mov		esi, [szStr]
			mov		edi, [szBuf]
			mov		ecx, [nLen]
			rep		movsb
			popad
		}

		return NULL;
	}

#endif

	typedef uint8_t BYTE;
	
	
	typedef struct heap_fragment
	{
		void* ptr;
		size_t size;
	} FRAGMENT, *PFRAGMENT;
	
	typedef struct list_
	{		
		FRAGMENT fragment;
		LIST_ENTRY(list_) Entries;
	}LIST, *PLIST;

	typedef struct asd
	{
		//PLIST list;
		size_t count = 0;		
		uintptr_t heap_size = 0;
	}data_mark;
	
	BSD_LIST_HEAD(, list_) FragmentsList;
	PLIST allocated_fragments = (PLIST)operator new(sizeof(struct list_));
	data_mark* data = new data_mark;
	uintptr_t heap[ALLOCATED_CAPACITY] = { 0 };
	
	
	void* mem_alloc(uintptr_t bytes)
	{				
		assert(bytes <= ALLOCATED_CAPACITY && bytes > 0);				
		std::cout << data->heap_size << std::endl;
		void* ptr_ret = heap + data->heap_size;
		data->heap_size += (bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
		//data->heap_size += bytes;
		PLIST LFragment = (PLIST)operator new(sizeof(struct list_));		
		{
			LFragment->fragment.size = bytes;
			LFragment->fragment.ptr = ptr_ret;				
			data->count += 1;		
		}		
		LIST_INSERT_HEAD(&FragmentsList, LFragment, Entries);
		return ptr_ret;
	}	


	void mem_free(void* ptr);
	void* mem_realloc(void* ptr, size_t new_size)
	{
    assert(false && "not implemented\n");
		size_t size_end = 0;		
		assert(ptr != NULL);
		LIST_FOREACH(allocated_fragments, &FragmentsList, Entries)
		{
			if (allocated_fragments->fragment.ptr == ptr)
			{
				assert(size_end + new_size <= ALLOCATED_CAPACITY);				
				size_t old_size = allocated_fragments->fragment.size;
				if (old_size > new_size) 
				{
					printf_s("old_size(%zu) > new_size(%zu)\n", old_size, new_size);
					return NULL;
				}
				void* ptr_new = mem_alloc(new_size);
								
#if defined _M_IX86 && defined _MSC_VER
				m_memcpy(heap, &heap[old_size], old_size);
				//m_memset(&heap[old_size], 0, new_size - old_size); no need memset in mem_free()
#else
				memmove_s(heap, old_size, &heap[old_size], old_size); // бля ваще я тупой тут крч у нас не с начала массива а с конца массива я дебил индексы список != массив
				//memset(&heap[old_size], 0, 20);//new_size - old_size
				//memset((void*)heap, 0, 20);//new_size - old_size
				heap[8] = 2;
#endif

				mem_free(ptr);
				return ptr_new;
			}		
			size_end += allocated_fragments->fragment.size;
		}
		assert(false && "miss the pointer address\n");
		return NULL;
	}

	/*RECOMMENDED FREE BLOCK FROM TAIL*/
	void mem_free(void* ptr)
	{
		assert(ptr != NULL);
		
		char check = 0;
		size_t szf = 0;
		LIST_FOREACH(allocated_fragments, &FragmentsList, Entries)
		{
			check += 1;
			szf += allocated_fragments->fragment.size;
			if (allocated_fragments->fragment.ptr == ptr)
			{				
				size_t size_hstart = 0;
				size_t size_block = allocated_fragments->fragment.size;				
				data->heap_size -= (size_block + sizeof(uintptr_t) - 1)/ sizeof(uintptr_t);
				LIST_REMOVE(allocated_fragments, Entries);
				allocated_fragments = LIST_NEXT(allocated_fragments, Entries);
				for (; allocated_fragments; allocated_fragments = LIST_NEXT(allocated_fragments, Entries))	/*get size*/
				{
					size_hstart += allocated_fragments->fragment.size;
					szf += allocated_fragments->fragment.size;
				}
				
#if defined _M_IX86 && defined _MSC_VER
				if (check != 0)
				{
					m_memcpy
					(
						&heap[(size_hstart + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)],
						&heap[((size_hstart + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)) + ((size_block + sizeof(uintptr_t) - 1) / sizeof(uintptr_t))],
						szf - size_block
					);
					m_memset(&heap[(szf - size_block + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)], 0, size_block);
				}
				else
					m_memset(&heap[(size_hstart + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)], 0, size_block);
				
#else
				if (check != 1)
				{
					memmove_s
					(
						&heap[(size_hstart + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)],
						szf - size_block,						
						&heap[((size_hstart + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)) + ((size_block + sizeof(uintptr_t) - 1) / sizeof(uintptr_t))],						
						szf - size_block
					);
					memset(&heap[(szf - size_block + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)], 0, size_block);
				}
				else
					memset(&heap[(size_hstart + sizeof(uintptr_t) - 1) / sizeof(uintptr_t)], 0, size_block);
#endif
				--data->count;				
				
				return;

			}			
		}
		
		assert(false && "miss the pointer address\n");
	}

	void mem_full_free()
	{		
		while (LIST_FIRST(&FragmentsList) != NULL)
		{
			LIST_REMOVE(LIST_FIRST(&FragmentsList), Entries);
		}
		data->count = 0;
		data->heap_size = 0;
		memset(heap, 0, ALLOCATED_CAPACITY);
	}

	void dump_allocated_fragments()
	{			
		printf_s("Allocated fragments of %zu\n", data->count);
		LIST_FOREACH(allocated_fragments, &FragmentsList, Entries)
		{
			printf_s("\tmem_ptr: %p\tsize: %zu\n", allocated_fragments->fragment.ptr, allocated_fragments->fragment.size);			
		}		
	}
#endif
