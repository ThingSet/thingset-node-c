/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_INTERNAL_H_
#define THINGSET_INTERNAL_H_

#include "thingset/thingset.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INDEX_NONE (-1)
#define INDEX_NEW  (-2) /* non-existent element behind the last array element */

struct thingset_endpoint
{
    /** Pointer to the data object in memory (must never be NULL) */
    struct thingset_data_object *object;
    /** Index number or INDEX_NONE or INDEX_NEW */
    int index;
};

/**
 * Fill the rsp buffer with a CBOR response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Numeric status code.
 * @param msg Optional diagnostic error message (pass NULL if not used)
 *
 * @return Length of response
 */
int thingset_bin_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg,
                                    ...);

/**
 * Fill the rsp buffer with a JSON response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Numeric status code.
 * @param msg Optional diagnostic error message (pass NULL if not used)
 *
 * @return Length of response
 */
int thingset_txt_serialize_response(struct thingset_context *ts, uint8_t code, const char *msg,
                                    ...);

/**
 * Get the endpoint from a provided path.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the struct thingset_endpoint to store the result.
 * @param path Relative path with multiple object names separated by forward slash.
 * @param len Length of the entire path.
 *
 * @return 0 if successful or negative ThingSet error code to be reported
 */
int thingset_endpoint_by_path(struct thingset_context *ts, struct thingset_endpoint *endpoint,
                              const char *path, size_t len);

/**
 * Get the endpoint from a provided ID.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the struct thingset_endpoint to store the result.
 * @param id Numeric ID of the ThingSet object.
 *
 * @return 0 if successful or negative ThingSet error code to be reported
 */
int thingset_endpoint_by_id(struct thingset_context *ts, struct thingset_endpoint *endpoint,
                            uint16_t id);

/**
 * Get the child object from a provided parent ID and the child name.
 *
 * @param ts Pointer to ThingSet context.
 * @param parent_id ID of the parent object.
 * @param name Child name.
 * @param len Length of the entire path.
 *
 * @return Pointer to the data object or NULL in case of error
 */
struct thingset_data_object *thingset_get_child_by_name(struct thingset_context *ts,
                                                        uint16_t parent_id, const char *name,
                                                        size_t len);

/**
 * Get the object by ID.
 *
 * @param ts Pointer to ThingSet context.
 * @param id ID of the object.
 *
 * @return Pointer to the data object or NULL in case of error
 */
struct thingset_data_object *thingset_get_object_by_id(struct thingset_context *ts, uint16_t id);

/**
 * Serialize the relative path of an object into the provided buffer
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer.
 * @param size Size of the buffer.
 * @param id Pointer to the object to get the path of.
 *
 * @return Length of the path or negative ThingSet response code in case of error
 */
int thingset_serialize_path(struct thingset_context *ts, char *buf, size_t size,
                            const struct thingset_data_object *obj);

/**
 * Process text mode GET/FETCH request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_txt_get_fetch(struct thingset_context *ts);

/**
 * Process text mode UPDATE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_txt_update(struct thingset_context *ts);

/**
 * Process text mode EXEC request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_txt_exec(struct thingset_context *ts);

/**
 * Process text mode CREATE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_txt_create(struct thingset_context *ts);

/**
 * Process text mode DELETE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_txt_delete(struct thingset_context *ts);

/**
 * Process text mode desire.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return 0 for success or negative ThingSet response code in case of error
 */
int thingset_txt_desire(struct thingset_context *ts);

/**
 * Process binary mode GET request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_bin_get(struct thingset_context *ts);

/**
 * Process binary mode FETCH request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_bin_fetch(struct thingset_context *ts);

/**
 * Process binary mode UPDATE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_bin_update(struct thingset_context *ts);

/**
 * Process binary mode EXEC request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_bin_exec(struct thingset_context *ts);

/**
 * Process binary mode CREATE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_bin_create(struct thingset_context *ts);

/**
 * Process binary mode DELETE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_bin_delete(struct thingset_context *ts);

/**
 * Process binary mode desire.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return 0 for success or negative ThingSet response code in case of error
 */
int thingset_bin_desire(struct thingset_context *ts);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_INTERNAL_H_ */
