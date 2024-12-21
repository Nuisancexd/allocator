#include <iostream>
#include "queue.h"
//#define WIN32_VIRTUALALLOC
#include "allocator.h"

#if defined WIN32_VIRTUALALLOC
void test_wvm()
{
	int* p = (int*)alloc::new_region(10);
	for (int i = 0; i < 10; ++i)
	{
		p[i] = 1;
		printf_s("%d ", p[i]);
	}

	alloc::free_region(p);
}
#else

void test_realloc()
{	
	int* ptr1 = (int*)alloc::mem_alloc(BYTES(8));
	for (int i = 0; i < 8; ++i)
	{
		ptr1[i] = 1;
	}
	int* ptr2 = (int*)alloc::mem_alloc(BYTES(16));
	for (int i = 0; i < 16; ++i)
	{
		ptr2[i] = 2;
	}
	int* ptr3 = (int*)alloc::mem_alloc(BYTES(32));
	for (int i = 0; i < 32; ++i)
	{
		ptr3[i] = 3;
	}
	
	alloc::dump_allocated_fragments();
	printf_s("\ndump heap\n");
	for (int i = 0; i < 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}
	
	ptr1 =(int*)alloc::mem_realloc(ptr1, BYTES(16));
	for (int i = 0; i < 16/2; ++i)
	{
		ptr1[i] = 4;
	}
	printf_s("\ndump heap after realloc\n");
	int c1 = 0;
	int c2 = 0;
	int c3 = 0;
	int c4 = 0;
	for (int i = 0; i < 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
		if ((int)alloc::heap[i] == 1)
			c1++;
		else if ((int)alloc::heap[i] == 2)
			c2++;
		else if ((int)alloc::heap[i] == 3)
			c3++;
		else if ((int)alloc::heap[i] == 4)
			c4++;
	}
	printf_s("\n '1' - %d\t '2' - %d\t '3' - %d\t '4' - %d\n", c1, c2, c3, c4);
		
	std::cout << "\n";
	alloc::dump_allocated_fragments();
}

void test_alloc()
{
	unsigned char* ptr_ch = (unsigned char*)alloc::mem_alloc(10 * sizeof(unsigned char));
	printf_s("ptr_\t");
	for (int i = 0; i < 2; ++i)
	{
		ptr_ch[i] = 255;
		printf_s("%d ", ptr_ch[i]);
	}
	int* ptr1 = (int*)alloc::mem_alloc(BYTES(8));
	for (int i = 0; i < 8; ++i)
	{
		ptr1[i] = 1;
	}
	int* ptr2 = (int*)alloc::mem_alloc(BYTES(8));
	for (int i = 0; i < 16; ++i)
	{
		ptr2[i] = 2;
	}
	int* ptr3 = (int*)alloc::mem_alloc(BYTES(32));
	for (int i = 0; i < 32; ++i)
	{
		ptr3[i] = 3;
	}
	int* ptr4 = (int*)alloc::mem_alloc(BYTES(64));
	for (int i = 0; i < 64; ++i)
	{
		ptr4[i] = 4;
	}
	printf_s("\nptr1\t");
	for (int i = 0; i < 9; ++i)
	{
		printf_s("%d ", ptr1[i]);
	}	
	printf_s("\nptr2\t");
	for (int i = 0; i < 16; ++i)
	{
		printf_s("%d ", ptr2[i]);
	}
	printf_s("\nptr3\t");
	for (int i = 0; i < 32; ++i)
	{
		printf_s("%d ", ptr3[i]);
	}
	printf_s("\nptr4\t");
	for (int i = 0; i < 64; ++i)
	{
		printf_s("%d ", ptr4[i]);
	}printf_s("\n");
	
	
	alloc::dump_allocated_fragments();
	printf_s("dump heap\n");
	for (int i = 0; i < 256+128+64+32+5; ++i)
	{
		std::cout <<(int) alloc::heap[i] << " ";
	}

	std::cout << "\n\n" << (int)alloc::heap[256 + 128 + 64 + 32 + 1] << std::endl;
}


void test_free()
{
	int* ptr1 = (int*)alloc::mem_alloc(BYTES(8));
	int* ptr2 = (int*)alloc::mem_alloc(BYTES(16));
	int* ptr3 = (int*)alloc::mem_alloc(BYTES(32));
	int* ptr4 = (int*)alloc::mem_alloc(BYTES(64));
	
	for (int i = 0; i < 8; ++i)
	{
		ptr1[i] = 1;
	}
	for (int i = 0; i < 16; ++i)
	{
		ptr2[i] = 2;
	}
	for (int i = 0; i < 32; ++i)
	{
		ptr3[i] = 3;
	}
	for (int i = 0; i < 64; ++i)
	{
		ptr4[i] = 4;
	}
	
	
	printf_s("ptr1\t");
	for (int i = 0; i < 8; ++i)
	{
		printf_s("%d ", ptr1[i]);
	}
	printf_s("\nptr2\t");
	for (int i = 0; i < 16; ++i)
	{
		printf_s("%d ", ptr2[i]);
	}
	printf_s("\nptr3\t");
	for (int i = 0; i < 32; ++i)
	{
		printf_s("%d ", ptr3[i]);
	}
	printf_s("\nptr4\t");
	for (int i = 0; i < 64; ++i)
	{
		printf_s("%d ", ptr4[i]);
	}printf_s("\n");

	alloc::dump_allocated_fragments();
	printf_s("\ndump heap\n");
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}
	alloc::mem_free(ptr2);
	printf_s("\ndump heap free\n");
	int c1 = 0;
	int c2 = 0;
	int c3 = 0;
	int c4 = 0;
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
		if ((int)alloc::heap[i] == 1)
			c1++;
		else if ((int)alloc::heap[i] == 2)
			c2++;
		else if ((int)alloc::heap[i] == 3)
			c3++;
		else if ((int)alloc::heap[i] == 4)
			c4++;
	}
	printf_s("\n");
	alloc::dump_allocated_fragments();
	printf_s("\n '1' - %d\t '2' - %d\t '3' - %d\t '4' - %d\n", c1,c2,c3,c4);

	int* ptr5 = (int*)alloc::mem_alloc(256 * sizeof(int));
	for (int i = 0; i < 256; ++i)
	{
		ptr5[i] = 5;
	}
	alloc::dump_allocated_fragments();
	printf_s("\ndump heap\n");
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}
	printf_s("\n");
	alloc::mem_free(ptr1);
	alloc::mem_free(ptr4);
	printf_s("\ndump heap ffree\n");
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}printf_s("\n");
	alloc::mem_free(ptr3);
	alloc::mem_free(ptr5);
	alloc::dump_allocated_fragments();
	printf_s("\ndump heap ffree\n");
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}
}

void test()
{
	int* ptr1 = (int*)alloc::mem_alloc(BYTES(10));
	int* ptr3 = (int*)alloc::mem_alloc(BYTES(30));
	int* ptr2 = (int*)alloc::mem_alloc(BYTES(20));
	int* ptr4 = (int*)alloc::mem_alloc(BYTES(40));

	for (int i = 0; i < 10; ++i)
	{
		ptr1[i] = 1;
	}
	for (int i = 0; i < 20; ++i)
	{
		ptr2[i] = 2;
	}
	for (int i = 0; i < 30; ++i)
	{
		ptr3[i] = 3;
	}
	for (int i = 0; i < 40; ++i)
	{
		ptr4[i] = 4;
	}
	printf_s("\ndump heap\n");
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}
	printf_s("\n");
	alloc::dump_allocated_fragments();
	alloc::mem_free(ptr3);
	alloc::mem_free(ptr2);
	alloc::dump_freed_fragments();
	int* ptr_ = (int*)alloc::mem_alloc(BYTES(30));
	for (int i = 0; i < 30; ++i)
	{
		ptr_[i] = 5;
	}
	int* ptr__ = (int*)alloc::mem_alloc(BYTES(5));
	for (int i = 0; i < 5; ++i)
	{
		ptr__[i] = 6;
	}
	int* ptr_f = (int*)alloc::mem_alloc(BYTES(5));
	for (int i = 0; i < 5; ++i)
	{
		ptr_f[i] = 7;
	}
	int* ptr_f_ = (int*)alloc::mem_alloc(BYTES(5));
	for (int i = 0; i < 5; ++i)
	{
		ptr_f_[i] = 9;
	}
	alloc::mem_realloc(ptr_f_, BYTES(7));
	ptr_f_[0] = 999;
	for (int i = 5; i < 7; ++i)
	{
		ptr_f_[i] = 99;
	}
	printf_s("\nptr_f_\t");
	for (int i = 0; i < 7; ++i)
	{
		printf_s("%d ", ptr_f_[i]);
	}
	alloc::mem_realloc(ptr_f, BYTES(10));
	for (int i = 5; i < 10; ++i)
	{
		ptr_f[i] = 77;
	}
	printf_s("\nptr_f\t");
	for (int i = 0; i < 10; ++i)
	{
		printf_s("%d ", ptr_f[i]);
	}
	printf_s("\nptr_f_\t");
	for (int i = 0; i < 7; ++i)
	{
		printf_s("%d ", ptr_f_[i]);
	}
	printf_s("\ndump heap\n");
	for (int i = 0; i < 128 + 64 + 32; ++i)
	{
		std::cout << (int)alloc::heap[i] << " ";
	}
	printf_s("\n");
	alloc::dump_allocated_fragments();
	printf_s("\nFREED DUMP\n");
	alloc::dump_freed_fragments();
	alloc::mem_full_free();
}

#endif

int main()
{
	alloc::init_alloc();
	test();
	std::cout << std::string(30, '-') << "TEST_ALLOC_START" << std::string(30, '-') << std::endl;
	test_alloc();
	alloc::mem_full_free();
	std::cout << std::string(30, '-') << "TEST_ALLOC_END" << std::string(30, '-') << std::endl;

	std::cout << std::endl << std::string(30, '-') << "TEST_FREE_START" << std::string(30, '-') << std::endl;
	test_free();
	std::cout << std::endl << std::string(30, '-') << "TEST_FREE_END" << std::string(30, '-') << std::endl;
	
	std::cout << std::endl << std::string(30, '-') << "TEST_REALLOC_START" << std::string(30, '-') << std::endl;
	test_realloc();
	std::cout << std::endl << std::string(30, '-') << "TEST_REALLOC_END" << std::string(30, '-') << std::endl;
	
}
