/**
 * @file include/binary.h
 * @brief Binary utility macros and helpers.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)