/**
 * @file include/acpi.h
 * @brief ACPI table structures and discovery routines.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

/**
 * @brief Scan legacy address ranges to locate the ACPI RSDP.
 *
 * @return Pointer to the RSDP if found, NULL otherwise.
 */
void* find_rsdp();
