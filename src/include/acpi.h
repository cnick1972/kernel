#pragma once

/**
 * @brief Scan legacy address ranges to locate the ACPI RSDP.
 *
 * @return Pointer to the RSDP if found, NULL otherwise.
 */
void* find_rsdp();
