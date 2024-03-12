/*
 * Copyright (c) The ThingSet Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_INTERNAL_H_
#define THINGSET_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Internal functions that have to be implemented separately for text and binary mode.
 *
 * The correct API struct is assigned to the ThingSet context at the beginning of processing an
 * incoming message.
 *
 * All serialize functions return 0 or negative error code. If a negative error code is
 * returned, an error message may have been stored already. If ts->rsp_pos == 0, the error
 * message has to be generated at the end.
 *
 * Also deserialize functions return 0 or negative error code, but never store any error response
 * in the buffer.
 */
struct thingset_api
{
    /**
     * Store a response with the specified error code in the response buffer.
     *
     * @param ts Pointer to ThingSet context
     * @param code Response code
     * @param msg Optional diagnostic payload for errors, otherwise NULL
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_response)(struct thingset_context *ts, uint8_t code, const char *msg, ...);

    /**
     * Serialize the key (name or ID) of the specified data object. For binary mode, the use_ids
     * parameter of the endpoint determines whether IDs or names should be used.
     *
     * @param ts Pointer to ThingSet context
     * @param object Pointer to data object
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_key)(struct thingset_context *ts, const struct thingset_data_object *object);

    /**
     * Serialize the value of the specified data object.
     *
     * @param ts Pointer to ThingSet context
     * @param object Pointer to data object
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_value)(struct thingset_context *ts, const struct thingset_data_object *object);

    /**
     * Serialize the path for the specified data object.
     *
     * @param ts Pointer to ThingSet context
     * @param object Pointer to data object
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_path)(struct thingset_context *ts, const struct thingset_data_object *object);

#ifdef CONFIG_THINGSET_METADATA_ENDPOINT
    /**
     * Serialize the metadata (including the type) for the specified data object.
     *
     * @param ts Pointer to ThingSet context
     * @param object Pointer to data object
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_metadata)(struct thingset_context *ts,
                              const struct thingset_data_object *object);
#endif /* CONFIG_THINGSET_METADATA_ENDPOINT */

    /**
     * Serialize the key and value of the specified data object.
     *
     * @param ts Pointer to ThingSet context
     * @param object Pointer to data object
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_key_value)(struct thingset_context *ts,
                               const struct thingset_data_object *object);

    /**
     * Serialize the start of a map (`{` for text mode).
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_map_start)(struct thingset_context *ts);

    /**
     * Serialize the end of a map (`}` for text mode).
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_map_end)(struct thingset_context *ts);

    /**
     * Serialize the start of a list/array (`[` for text mode).
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_list_start)(struct thingset_context *ts);

    /**
     * Serialize the end of a list/array (`]` for text mode).
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_list_end)(struct thingset_context *ts);

    /**
     * Serialize the payload data for the specified subset.
     *
     * The implementation of this function is very different for text and binary mode, so it cannot
     * be implemented as a common function.
     *
     * @param ts Pointer to ThingSet context
     * @param subsets Subset(s) to be considered
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_subsets)(struct thingset_context *ts, uint16_t subsets);

    /**
     * Serialize the start of a report message.
     *
     * The path parameter is redundant, as it could be determined from the endpoint. However,
     * providing it as a parameter reduces calculation effort.
     *
     * @param ts Pointer to ThingSet context
     * @param path Path string or NULL if using IDs is desired for binary mode
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*serialize_report_header)(struct thingset_context *ts, const char *path);

    /**
     * Finalize serialization
     *
     * @param ts Pointer to ThingSet context
     */
    void (*serialize_finish)(struct thingset_context *ts);

    /**
     * Reset payload deserialization to start parsing at beginning of payload.
     *
     * @param ts Pointer to ThingSet context
     */
    void (*deserialize_payload_reset)(struct thingset_context *ts);

    /**
     * Deserialize string with zero-copy.
     *
     * @param ts Pointer to ThingSet context
     * @param str_start Pointer to store start of string
     * @param str_len Pointer to store length of string in the buffer EXCLUDING null-termination
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_string)(struct thingset_context *ts, const char **str_start, size_t *str_len);

    /**
     * Deserialize null value.
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_null)(struct thingset_context *ts);

    /**
     * Deserialize the start of a list/array (`[` for text mode).
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_list_start)(struct thingset_context *ts);

    /**
     * Deserialize the start of a map (`{` for text mode).
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_map_start)(struct thingset_context *ts);

    /**
     * Deserialize a child object by name or ID for a given parent ID.
     *
     * @param ts Pointer to ThingSet context
     * @param object Pointer to store the pointer to the found child object
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_child)(struct thingset_context *ts,
                             const struct thingset_data_object **object);

    /**
     * Deserialize any value for the given data object
     *
     * Setting the check_only parameter allows to check the type and size of the data items prior
     * to applying the changes of an entire UPDATE request.
     *
     * @param ts Pointer to ThingSet context
     * @param object Data object to use
     * @param check_only If set to true, buffers are not actually deserialized and it is only
     *                   checked if their size would fit.
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_value)(struct thingset_context *ts, const struct thingset_data_object *object,
                             bool check_only);

    /**
     * Deserialize the next object and skip it
     *
     * @param ts Pointer to ThingSet context
     *
     * @returns 0 for success or negative ThingSet response code in case of error
     */
    int (*deserialize_skip)(struct thingset_context *ts);

    /**
     * Finalize deserialization
     *
     * @param ts Pointer to ThingSet context
     */
    int (*deserialize_finish)(struct thingset_context *ts);
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
 * Get an object by its path.
 *
 * @param ts Pointer to ThingSet context.
 * @param path Path to the object
 * @param path_len Length of path
 * @param index Pointer to an index which may be decoded as part of the path
 */
struct thingset_data_object *thingset_get_object_by_path(struct thingset_context *ts,
                                                         const char *path, size_t path_len,
                                                         int *index);

/**
 * Get the relative path of an object
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer to store the path.
 * @param size Size of the buffer.
 * @param obj Pointer to the object to get the path of.
 *
 * @return Length of the path or negative ThingSet response code in case of error
 */
int thingset_get_path(struct thingset_context *ts, char *buf, size_t size,
                      const struct thingset_data_object *obj);

/**
 * Gets the type of a given object as a string.
 *
 * @param ts Pointer to ThingSet context.
 * @param obj Pointer to the object to get the path of.
 * @param buf Pointer to the buffer to store the path.
 * @param size Size of the buffer.
 */
int thingset_get_type_name(struct thingset_context *ts, const struct thingset_data_object *obj,
                           char *buf, size_t size);

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

int thingset_bin_import_data(struct thingset_context *ts, uint8_t auth_flags,
                             enum thingset_data_format format);

int thingset_bin_import_data_progressively(struct thingset_context *ts, uint8_t auth_flags,
                                           size_t size, uint32_t *last_id, size_t *consumed);

int thingset_bin_export_subsets_progressively(struct thingset_context *ts, uint16_t subsets,
                                              unsigned int *index, size_t *len);

int thingset_common_serialize_group(struct thingset_context *ts,
                                    const struct thingset_data_object *object);

int thingset_common_serialize_record(struct thingset_context *ts,
                                     const struct thingset_data_object *object, int record_index);

typedef int (*thingset_common_record_element_action)(
    struct thingset_context *ts, const struct thingset_data_object *item_offset);

int thingset_common_prepare_record_element(struct thingset_context *ts,
                                           const struct thingset_data_object *item,
                                           uint8_t *record_ptr,
                                           thingset_common_record_element_action callback);

/**
 * Process GET request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_common_get(struct thingset_context *ts);

/**
 * Process FETCH request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_common_fetch(struct thingset_context *ts);

/**
 * Process UPDATE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_common_update(struct thingset_context *ts);

/**
 * Process EXEC request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_common_exec(struct thingset_context *ts);

/**
 * Process CREATE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_common_create(struct thingset_context *ts);

/**
 * Process DELETE request.
 *
 * @param ts Pointer to ThingSet context.
 *
 * @return Length of response or negative ThingSet response code in case of error
 */
int thingset_common_delete(struct thingset_context *ts);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_INTERNAL_H_ */
