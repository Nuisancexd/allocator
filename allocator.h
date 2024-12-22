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
#error("Compiler of gcc or msc");
#endif

#ifndef ALLOCATED_CAPACITY
//#define ALLOCATED_CAPACITY 10000
#endif

#define BYTES(x) x * sizeof(x)
#define STATIC static

namespace alloc
{
#if !defined WIN32_VIRTUALALLOC		
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
	#endif
	namespace detail
	{
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

		typedef struct data
		{	
			size_t count = 0;			
			uintptr_t heap_size = 0;
			size_t freed_count = 0;
		}data_mark;
		
		typedef struct freed_frag_list
		{
			void* ptr;
			size_t bytes;
			TAILQ_ENTRY(freed_frag_list) Entries;
		}FREED, *PFREED;
		
		
		BSD_LIST_HEAD(, list_) FragmentsList;
		TAILQ_HEAD(, freed_frag_list) FreedsList;
		PLIST allocated_fragments = (PLIST)operator new(sizeof(struct list_));
		PFREED freed_fragments = (PFREED)operator new(sizeof(struct freed_frag_list));
		data_mark* data = new data_mark;
	}
	uintptr_t heap[ALLOCATED_CAPACITY] = { 0 };
	
	#define det using namespace detail
	
	
	void* mem_alloc(uintptr_t bytes)
	{
		det;		
		assert(data->heap_size + bytes <= ALLOCATED_CAPACITY && bytes > 0);

		TAILQ_FOREACH(freed_fragments, &FreedsList, Entries)
		{
			if (freed_fragments->bytes >= bytes)
			{				
				void* ptr_freed = freed_fragments->ptr;
				PLIST LFragment = (PLIST)operator new(sizeof(struct list_));
				{
					LFragment->fragment.size = bytes;
					LFragment->fragment.ptr = ptr_freed;
					++data->count;					
				}
				LIST_INSERT_HEAD(&FragmentsList, LFragment, Entries);
				
				
				/* Fragmentation */
				if (freed_fragments->bytes - bytes > 0)
				{
					freed_fragments->bytes -= bytes;					
					auto pos = [&ptr_freed]()
						{
							size_t i = 0;
							for (; i < data->heap_size; ++i)
							{
								if (&heap[i] == ptr_freed)
									return i;
							}
							/*size_t i = 0;
							size_t j = data->heap_size;
							for (; i < data->heap_size && j != i; ++i, --j)
							{
								if (&heap[i] == ptr_freed)
								{
									return i;
								}
								else if (&heap[j] == ptr_freed)
								{
									return j;
								}
							}*/
							return static_cast<size_t>(0);
						};

					freed_fragments->ptr = heap + pos() + (bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
				}
				else
				{
					TAILQ_REMOVE(&FreedsList, freed_fragments, Entries);
					--data->freed_count;
				}
				return ptr_freed;
			}
		}

		void* ptr_ret = heap + data->heap_size;
		data->heap_size += (bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
		
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
		det;
		void* ptr_new = NULL;
		LIST_FOREACH(allocated_fragments, &FragmentsList, Entries)
		{
			if (allocated_fragments->fragment.ptr == ptr)
			{
				size_t old_size = allocated_fragments->fragment.size;				
				if (old_size > new_size)
				{
					printf_s("old_size(%zu) < new_size(%zu)\n", old_size, new_size);					
					return ptr;
				}
				ptr_new = mem_alloc(new_size);
				memmove_s(ptr_new, old_size, ptr, old_size);
			}
		}
		mem_free(ptr);
		return ptr_new;
	}

	/*RECOMMENDED FREE BLOCK FROM TAIL*/
	void mem_free(void* ptr)
	{
		det;
		allocated_fragments = LIST_FIRST(&FragmentsList);
		if (ptr == allocated_fragments->fragment.ptr)
		{
			
#if defined _M_IX86 && defined _MSC_VER
			m_memset(allocated_fragments->fragment.ptr, 0, allocated_fragments->fragment.size);
#else			
			memset(allocated_fragments->fragment.ptr, 0, allocated_fragments->fragment.size);
#endif
			data->heap_size -= (allocated_fragments->fragment.size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
			LIST_REMOVE(allocated_fragments, Entries);
			return;
		}

		for (; allocated_fragments; allocated_fragments = LIST_NEXT(allocated_fragments, Entries))
		{
			if (ptr == allocated_fragments->fragment.ptr)
			{
				PFREED FFragment = (PFREED) operator new(sizeof(struct freed_frag_list));				
				{					
					FFragment->bytes = allocated_fragments->fragment.size;
					FFragment->ptr = allocated_fragments->fragment.ptr;
					--data->count;
					++data->freed_count;
				}
				
				TAILQ_INSERT_TAIL(&FreedsList, FFragment, Entries);				
				memset(allocated_fragments->fragment.ptr, 0, allocated_fragments->fragment.size);
				LIST_REMOVE(allocated_fragments, Entries);		
				return;
			}
		}

		assert(false && "miss the pointer address\n");
	}


	void mem_full_free()
	{		
		det;
		while (LIST_FIRST(&FragmentsList) != NULL)
		{
			LIST_REMOVE(LIST_FIRST(&FragmentsList), Entries);
		}
		while (TAILQ_FIRST(&FreedsList) != NULL)
		{
			TAILQ_REMOVE(&FreedsList, TAILQ_FIRST(&FreedsList), Entries);
		}
		data->count = 0;
		data->heap_size = 0;
		data->freed_count = 0;		
#if defined _M_IX86 && defined _MSC_VER
		m_memset(heap, 0, ALLOCATED_CAPACITY);
#else
		memset(heap, 0, ALLOCATED_CAPACITY);
#endif
	}
	
	void dump_allocated_fragments()
	{			
		det;
		printf_s("Allocated fragments of %zu\n", data->count);
		LIST_FOREACH(allocated_fragments, &FragmentsList, Entries)
		{
			printf_s("\tmem_ptr: %p\tsize: %zu\n", allocated_fragments->fragment.ptr, allocated_fragments->fragment.size);			
		}		
	}

	void dump_freed_fragments()
	{
		det;
		printf_s("Freed fragments of %zu\n", data->freed_count);
		TAILQ_FOREACH(freed_fragments, &FreedsList, Entries)
		{
			printf_s("\tmem_ptr: %p\tbytes: %zu\n", freed_fragments->ptr, freed_fragments->bytes);
		}
	}

	void init_alloc()
	{		
		det;
		TAILQ_INIT(&FreedsList);
		LIST_INIT(&FragmentsList);
	}

	#else

#if !defined(_WINDOWS_)
	#include <Windows.h>
#endif
		void* wmalloc(size_t bytes)
		{
			size_t size_bytes = (bytes + sizeof(uintptr_t) - 1)/sizeof(uintptr_t);
			void* ptr = VirtualAllocEx
			(
				GetCurrentProcess(),      /* Allocate in current process address space */
				NULL,                     /* Unknown position */
				size_bytes,               /* Bytes to allocate */
				MEM_COMMIT | MEM_RESERVE, /* Reserve and commit allocated page */
				PAGE_READWRITE            /* Permissions ( Read/Write )*/
			);
			if (ptr == NULL || ptr == INVALID_HANDLE_VALUE)
				assert(0 && "VirtualAllocEx() failed.");
	
			
			return ptr;
		}
	
		void wmfree(void* ptr)
		{
			if (ptr == NULL || ptr == INVALID_HANDLE_VALUE)
				return;
	
			BOOL free_result = VirtualFreeEx(
				GetCurrentProcess(),        /* Deallocate from current process address space */
				(LPVOID)ptr,                  /* Address to deallocate */
				0,                          /* Bytes to deallocate ( Unknown, deallocate entire page ) */
				MEM_RELEASE                 /* Release the page ( And implicitly decommit it ) */
			);
	
			if (FALSE == free_result)
				assert(0 && "VirtualFreeEx() failed.");
		}
	#endif	
}
#endif
