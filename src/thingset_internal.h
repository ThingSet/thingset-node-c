/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_INTERNAL_H_
#define THINGSET_INTERNAL_H_

#include "thingset/thingset.h"

#define JSMN_HEADER
#include "jsmn.h"

#include <zcbor_common.h>

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
    /** Use names or IDs (relevant for binary mode) */
    bool use_ids;
};

/*
 * All serialize functions return 0 or negative error code. If a negative error code is
 * returned, an error message may have been stored already. If ts->rsp_pos == 0, the error
 * message has to be generated at the end.
 */
struct thingset_api
{
    int (*serialize_response)(struct thingset_context *ts, uint8_t code, const char *msg, ...);
    int (*serialize_key)(struct thingset_context *ts, const struct thingset_data_object *object);
    int (*serialize_value)(struct thingset_context *ts, const struct thingset_data_object *object);
    int (*serialize_key_value)(struct thingset_context *ts,
                               const struct thingset_data_object *object);
    int (*serialize_map_start)(struct thingset_context *ts);
    int (*serialize_map_end)(struct thingset_context *ts);
    int (*serialize_list_start)(struct thingset_context *ts);
    int (*serialize_list_end)(struct thingset_context *ts);

    int (*serialize_subsets)(struct thingset_context *ts, uint16_t subsets);

    /**
     * @param path Path string or NULL if using IDs is desired for binary mode
     */
    int (*serialize_report_header)(struct thingset_context *ts, const char *path);

    void (*serialize_finish)(struct thingset_context *ts);

    /**
     * Deserialize string with zero-copy
     *
     * @param str_start Pointer to store start of string
     * @param str_len Pointer to store length of string in the buffer EXCLUDING null-termination
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_string)(struct thingset_context *ts, const char **str_start, size_t *str_len);
};

/**
 * ThingSet context.
 *
 * Stores and handles all data objects exposed to different communication interfaces.
 */
struct thingset_context
{
    /**
     * Array of objects database provided during initialization
     */
    struct thingset_data_object *data_objects;

    /**
     * Number of objects in the data_objects array
     */
    size_t num_objects;

    /**
     * Pointer to the incoming message buffer (request or desire, provided by process function)
     */
    const uint8_t *msg;

    /**
     * Length of the incoming message
     */
    size_t msg_len;

    /**
     * Position in the message currently being parsed
     */
    size_t msg_pos;

    /**
     * Pointer to the response buffer (provided by process function)
     */
    uint8_t *rsp;

    /**
     * Size of response buffer (i.e. maximum length)
     */
    size_t rsp_size;

    /**
     * Current position inside the response (equivalent to length of the response at end of
     * processing)
     */
    size_t rsp_pos;

    /**
     * Function pointers to mode-specific implementation (text or binary)
     */
    struct thingset_api *api;

    /* State information for data processing. */
    union {
        /* Text mode */
        struct
        {
            /** Pointer to the start of JSON payload in the request */
            char *json_str;

            /** JSON tokens in json_str parsed by JSMN */
            jsmntok_t tokens[CONFIG_THINGSET_NUM_JSON_TOKENS];

            /** Number of JSON tokens parsed by JSMN */
            size_t tok_count;

            /** Current position of the parsing process */
            size_t tok_pos;
        };
        /* Binary mode */
        struct
        {
            /** CBOR encoder states for binary mode */
            zcbor_state_t encoder[4];

            /** CBOR decoder states for binary mode */
            zcbor_state_t decoder[4];
        };
    };

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint8_t auth_flags;

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint8_t update_subsets;

    /**
     * Callback to be called from patch function if a value belonging to update_subsets
     * was changed
     */
    void (*update_cb)(void);

    /**
     * Endpoint used for the current message
     */
    struct thingset_endpoint endpoint;
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
 * Process message in text mode.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return see thingset_process_message.
 */
int thingset_txt_process(struct thingset_context *ts);

void thingset_txt_setup(struct thingset_context *ts);

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

/**
 * Process message in binary mode.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return see thingset_process_message.
 */
int thingset_bin_process(struct thingset_context *ts);

void thingset_bin_setup(struct thingset_context *ts, size_t buf_offset);

int thingset_common_serialize_group(struct thingset_context *ts,
                                    const struct thingset_data_object *object);

int thingset_common_serialize_record(struct thingset_context *ts,
                                     const struct thingset_data_object *object, int record_index);

int thingset_common_get(struct thingset_context *ts);

int thingset_common_create(struct thingset_context *ts);

int thingset_common_delete(struct thingset_context *ts);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_INTERNAL_H_ */
