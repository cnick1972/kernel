/**
 * @file include/tss.h
 * @brief Task State Segment helpers.
 */

#pragma once

#include <stdint.h>

void tss_init(uint32_t kernel_stack_top);
void tss_set_kernel_stack(uint32_t kernel_stack_top);
