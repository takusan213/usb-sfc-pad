/*******************************************************************************
Copyright 2025 Custom USB Gamepad Project

Mapping functionality for button-to-usage configuration
*******************************************************************************/

#ifndef _MAPPING_H
#define _MAPPING_H

#include <stdint.h>

#define NUM_BUTTONS 8  // SFCは8ボタン（十字キー除く）

// 物理ボタンインデックス
enum {
    PHYS_BTN_A = 0,      // A
    PHYS_BTN_B = 1,      // B  
    PHYS_BTN_X = 2,      // X
    PHYS_BTN_Y = 3,      // Y
    PHYS_BTN_L = 4,      // L(TL)
    PHYS_BTN_R = 5,      // R(TR)
    PHYS_BTN_START = 6,  // Start
    PHYS_BTN_SELECT = 7  // Select
};

/**
 * convert mapping data structure to row buffer for flash write
 */
void map_to_rowbuf(void);

/**
 * Load mapping from High-Endurance Flash to RAM
 */
void Mapping_Load(void);

/**
 * Save mapping from RAM to High-Endurance Flash
 * @param normal_tbl Pointer to normal mode button-to-usage mapping table (at least NUM_BUTTONS bytes)
 * @param special_tbl Pointer to special mode button-to-usage mapping table (at least NUM_BUTTONS bytes)
 */
void Mapping_Save(const uint8_t *normal_tbl, const uint8_t *special_tbl);

/**
 * Get the usage value for a physical button
 * @param physBtn Physical button index (0-7)
 * @param mode Mode selection (0=normal, 1=special)
 * @return Usage value (1-14)
 */
uint8_t Mapping_GetUsage(uint8_t physBtn, uint8_t mode);

/**
 * Copy mapping data from Feature Report buffer to the mapping table
 * @param featureReport The feature report buffer received from the host
 * @param length Length of the feature report data
 */
void Mapping_SetFromFeatureReport(uint8_t* featureReport, uint16_t length);

/**
 * Copy mapping data from the mapping table to the Feature Report buffer
 * @param featureReport The feature report buffer to be sent to the host
 */
void Mapping_GetAsFeatureReport(uint8_t* featureReport);

#endif /* _MAPPING_H */