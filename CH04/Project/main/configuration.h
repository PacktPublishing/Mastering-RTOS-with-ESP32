/**
 * @file configuration.h
 * @author Bhupinderjeet Singh
 * @brief
 * @version 0.1
 * @date 2023-11-04
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "inttypes.h"

typedef enum
{
	PARAM_TYPE_INT8 = 0,
	PARAM_TYPE_INT16,
	PARAM_TYPE_INT32,
	PARAM_TYPE_UINT8,
	PARAM_TYPE_UINT16,
	PARAM_TYPE_UINT32,
	PARAM_TYPE_STRING,
	PARAM_TYPE_TOTAL
} param_type_t;

#define P_LIST                                \
	PARAM(PARAM_TYPE_INT8, PTST1, 1, TEST1)   \
	PARAM(PARAM_TYPE_UINT8, PTST2, 1, TEST2)  \
	PARAM(PARAM_TYPE_INT16, PTST3, 2, TEST3)  \
	PARAM(PARAM_TYPE_UINT16, PTST4, 2, TEST4) \
	PARAM(PARAM_TYPE_INT32, PTST5, 4, TEST5)  \
	PARAM(PARAM_TYPE_UINT32, PTST6, 4, TEST6) \
	PARAM(PARAM_TYPE_STRING, PTST7, 24, TEST7)

#define PARAM(type, key, len, name) \
	name,
typedef enum
{
	P_LIST
		P_LIST_TOTAL
} param_t;

#undef PARAM
#define PARAM(type, key, len, name) \
	key,
enum
{
	P_LIST
		P_IDS_TOTAL
};

typedef struct config
{
	const char *key;
	const char *name;
	param_type_t type;
	void *array;
	size_t array_len;
	size_t array_max;
} params_t;

bool configuration_init(void);
bool configuration_process(void);

int8_t p_params_read_int8(param_t i);
int16_t p_params_read_int16(param_t i);
int32_t p_params_read_int32(param_t i);
uint8_t p_params_read_uint8(param_t i);
uint16_t p_params_read_uint16(param_t i);
uint32_t p_params_read_uint32(param_t i);
bool p_params_read_string(param_t i, char *value, uint16_t *len);

bool p_params_write_int8(param_t i, int8_t value);
bool p_params_write_int16(param_t i, int16_t value);
bool p_params_write_int32(param_t i, int32_t value);
bool p_params_write_uint8(param_t i, uint8_t value);
bool p_params_write_uint16(param_t i, uint16_t value);
bool p_params_write_uint32(param_t i, uint32_t value);
bool p_params_write_string(param_t i, char *value, uint16_t size);

#endif /* __CONFIGURATION_H__ */