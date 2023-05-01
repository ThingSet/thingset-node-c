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

/**
 * Fill the rsp buffer with a CBOR response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Numeric status code.
 * @param msg Optional diagnostic error message (pass NULL if not used)
 *
 * @return Length of response
 */
int thingset_bin_serialize_response(struct thingset_context *ts, int code, const char *msg, ...);

/**
 * Fill the rsp buffer with a JSON response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Numeric status code.
 * @param msg Optional diagnostic error message (pass NULL if not used)
 *
 * @return Length of response
 */
int thingset_txt_serialize_response(struct thingset_context *ts, int code, const char *msg, ...);

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
