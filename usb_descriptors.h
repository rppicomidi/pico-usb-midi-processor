#pragma once
#include <stdint.h>
#include "tusb.h"
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif
void start_cloning(uint8_t dev_addr);
void set_cloning_required();
bool cloning_is_required();
bool clone_next_string_is_required();
void clone_next_string();
bool descriptors_are_cloned(void);
void set_descriptors_uncloned(void);
/**
 * @brief Get the device string object
 *
 * @param devstr 
 * @param max_prodstr 
 * @return true if the device string is successfully copied
 * @return false if the parameters not OK or if the device string is not available
 *
 * @note this function assumes the first language ID is default and it assumes
 * that no character in the string is > 255 (e.g. US ASCII)
 */
bool get_product_string(char* prodstr, uint8_t max_prodstr);

uint8_t get_product_string_length(void);

uint16_t get_vid();
uint16_t get_pid();

TU_ATTR_WEAK void device_clone_complete_cb(void);
#ifdef __cplusplus
}
#endif
