//******************************************************************************
//
// File Name : events.h
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

#include <string.h>
#include "common.h"
#include "../core/jparse.h"

int on_object_update(DB_OBJECT *object)
{
	JKEY key;

	jparse_extract_key((unsigned char *)"dev_id", 6, (unsigned char *)object->core_data, object->core_data_size, &key);

	if (key.value_type == JPARSE_VALUE_TYPE_STRING) {

		for (size_t i = object_id.size(); i--; ) {

			if (object->id == object_id[i]) {

				unsigned char *ptr = dev_id + i * 8;
				unsigned char id[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

				memcpy(id + 16 - key.str_len, key.value.str_val, key.str_len);

				*ptr++ = ((id[0]  - '0') << 4) | (id[1]  - '0');
				*ptr++ = ((id[2]  - '0') << 4) | (id[3]  - '0');
				*ptr++ = ((id[4]  - '0') << 4) | (id[5]  - '0');
				*ptr++ = ((id[6]  - '0') << 4) | (id[7]  - '0');
				*ptr++ = ((id[8]  - '0') << 4) | (id[9]  - '0');
				*ptr++ = ((id[10] - '0') << 4) | (id[11] - '0');
				*ptr++ = ((id[12] - '0') << 4) | (id[13] - '0');
				*ptr++ = ((id[14] - '0') << 4) | (id[15] - '0');

				break;
			}
		}
	}

	return 0;
}

int on_object_remove(DB_OBJECT *object)
{
	for (size_t i = object_id.size(); i--; ) {

		if (object->id == object_id[i]) {

			memcpy(dev_id + i * 8, dev_id + (i + 1) * 8, (object_id.size() - i - 1) * 8);

			object_id.erase(object_id.begin() + i);

			break;
		}
	}

	return 0;
}

int on_object_create(DB_OBJECT *object)
{
	JKEY key;

	jparse_extract_key((unsigned char *)"dev_id", 6, (unsigned char *)object->core_data, object->core_data_size, &key);

	if (key.value_type == JPARSE_VALUE_TYPE_STRING) {

		unsigned char *ptr = dev_id + object_id.size() * 8;
		unsigned char id[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

		memcpy(id + 16 - key.str_len, key.value.str_val, key.str_len);

		*ptr++ = ((id[0]  - '0') << 4) | (id[1]  - '0');
		*ptr++ = ((id[2]  - '0') << 4) | (id[3]  - '0');
		*ptr++ = ((id[4]  - '0') << 4) | (id[5]  - '0');
		*ptr++ = ((id[6]  - '0') << 4) | (id[7]  - '0');
		*ptr++ = ((id[8]  - '0') << 4) | (id[9]  - '0');
		*ptr++ = ((id[10] - '0') << 4) | (id[11] - '0');
		*ptr++ = ((id[12] - '0') << 4) | (id[13] - '0');
		*ptr++ = ((id[14] - '0') << 4) | (id[15] - '0');

		object_id.push_back(object->id);
	}

	return 0;
}

int on_timer()
{
	return 0;
}
