/**
 * @file include/usermode.h
 * @brief Helpers to transition into user mode.
 */

#pragma once

void usermode_enter(void (*entry)(void));
