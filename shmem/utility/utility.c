#include <stdint.h>

//---------------------------------------------------------

#include "utility.h"

//=========================================================

int my_swap(void* first_, void* second_, size_t size) 
{
	assert(first_  != NULL);
	assert(second_ != NULL);

	char* first_ptr  = (char*)first_;
	char* second_ptr = (char*)second_;

	while (size >= sizeof(int64_t)) 
    {
		int64_t temp = *(int64_t*)first_ptr;
		*(int64_t*)first_ptr  = *(int64_t*)second_ptr;
		*(int64_t*)second_ptr = temp;

		first_ptr += sizeof(int64_t);
		second_ptr += sizeof(int64_t);

		size -= sizeof(int64_t);
	}

	while (size >= sizeof(int32_t)) 
    {
		int32_t temp = *(int32_t*)first_ptr;
		*(int32_t*)first_ptr = *(int32_t*)second_ptr;
		*(int32_t*)second_ptr = temp;

		first_ptr += sizeof(int32_t);
		second_ptr += sizeof(int32_t);

		size -= sizeof(int32_t);
	}

	while (size >= sizeof(int16_t)) 
    {
		int16_t temp = *(int16_t*)first_ptr;
		*(int16_t*)first_ptr  = *(int16_t*)second_ptr;
		*(int16_t*)second_ptr = temp;

		first_ptr += sizeof(int16_t);
		second_ptr += sizeof(int16_t);

		size -= sizeof(int16_t);
	}

	while (size >= sizeof(int8_t)) 
    {
		int8_t temp = *(int8_t*)first_ptr;
		*(int8_t*)first_ptr  = *(int8_t*)second_ptr;
		*(int8_t*)second_ptr = temp;

		first_ptr += sizeof(int8_t);
		second_ptr += sizeof(int8_t);

		size -= sizeof(int8_t);
	}

	return 0;
}