/*******************************************************************************
Copyright 2025 Custom USB Gamepad Project

Mapping functionality for button-to-usage configuration
*******************************************************************************/

#include "mapping.h"
#include <xc.h>
#include <string.h>
#include "mcc_generated_files/nvm/nvm.h"
#include "demo_src/hid_rpt_map.h"

/* RAM working copy of the mapping data */
static struct {
    // Bytes 0-7: Global settings (8 bytes)
    uint8_t report_id;                // Report ID (Feature Report送信時に0x00設定)
    uint8_t ver;                      // Version for compatibility checking
    uint8_t crc;                      // CRC8 checksum for data integrity
    uint8_t global_reserved[5];       // Reserved for future global settings
    
    // Bytes 8-23: Normal mode mapping (16 bytes)
    uint8_t normal_tbl[NUM_BUTTONS];  // Normal mode button-to-usage mapping table (8 bytes)
    uint8_t normal_reserved[8];       // Reserved for normal mode expansion (8 bytes)
    
    // Bytes 24-39: Special mode mapping (16 bytes)
    uint8_t special_tbl[NUM_BUTTONS]; // Special mode button-to-usage mapping table (8 bytes)
    uint8_t special_reserved[8];      // Reserved for special mode expansion (8 bytes)
    
    // Bytes 40-63: Future expansion (24 bytes)
    uint8_t future_reserved[24];      // Reserved for future features
} map;            

#define MAP_VER 0x01           // Current data structure version
#define HEF_ADDR 0x1F80        // High-Endurance Flash starting address (row0)

#define ROW_WORDS   32                  // 64B / 2B
static flash_data_t rowBuf[ROW_WORDS];  // uint16_t[32]

/**
 * Calculate CRC8 checksum (0x07 polynomial)
 * @param d Pointer to data
 * @param l Data length in bytes
 * @return CRC8 checksum
 */
static uint8_t crc8(const uint8_t *d, uint8_t l) {
    uint8_t c = 0;
    while (l--) {
        c ^= *d++;
        for (uint8_t i = 0; i < 8; i++) {
            c = (uint8_t)((c & 0x80) ? ((c << 1) ^ 0x07) : (c << 1));
        }
    }
    return c;
}

void map_to_rowbuf(void)
{
    /* 0x3FFF で初期化（未使用上位バイトは 0x3F） */
    for (uint8_t i = 0; i < ROW_WORDS; i++) rowBuf[i] = 0x3FFF;

    /* uint8_t map 構造体をuint16_t rowBufにコピー */
    for (uint8_t b = 0; b < sizeof(map); b++) {
        rowBuf[b] = 0x3F00 | ((uint8_t*) &map)[b]; 
    }
}

/**
 * Load mapping from High-Endurance Flash to RAM
 * If invalid data detected, initialize with default mapping
 */
void Mapping_Load(void) {
    // Read mapping data from flash to RAM via FLASH_Read
    for(uint8_t i = 0; i < sizeof(map); i ++){
        // Read 14-bit words from flash and convert to 8-bit
        flash_data_t data = FLASH_Read(HEF_ADDR + i);
        ((uint8_t*)&map)[i] = (uint8_t)(data & 0x00FF); // Use lower byte
    }
    
    // Validate data (version and CRC)
    if (map.ver != MAP_VER || map.crc != crc8((uint8_t*)&map, sizeof(map) - 1)) {
        // Invalid data, initialize with standardized default mapping
        
        // Normal mode mapping - SFC standard layout
        map.normal_tbl[PHYS_BTN_A] = 1;      // A -> Button 1 (A)
        map.normal_tbl[PHYS_BTN_B] = 2;      // B -> Button 2 (B)
        map.normal_tbl[PHYS_BTN_X] = 3;      // X -> Button 3 (X)
        map.normal_tbl[PHYS_BTN_Y] = 4;      // Y -> Button 4 (Y)
        map.normal_tbl[PHYS_BTN_L] = 5;      // L -> Button 5 (L1)
        map.normal_tbl[PHYS_BTN_R] = 6;      // R -> Button 6 (R1)
        map.normal_tbl[PHYS_BTN_SELECT] = 7; // Select -> Button 7 (Select)
        map.normal_tbl[PHYS_BTN_START] = 8;  // Start -> Button 8 (Start)
        
        // Special mode mapping - Switch-like layout
        map.special_tbl[PHYS_BTN_A] = 1;      // A -> Button 1 (A)
        map.special_tbl[PHYS_BTN_B] = 11;     // B -> Button 11 (Home)
        map.special_tbl[PHYS_BTN_X] = 12;     // X -> Button 12 (Right Stick)
        map.special_tbl[PHYS_BTN_Y] = 13;     // Y -> Button 13 (Left Stick)
        map.special_tbl[PHYS_BTN_L] = 9;      // L -> Button 9 (L2)
        map.special_tbl[PHYS_BTN_R] = 10;     // R -> Button 10 (R2)
        map.special_tbl[PHYS_BTN_SELECT] = 7; // Select -> Button 7 (Select)
        map.special_tbl[PHYS_BTN_START] = 8;  // Start -> Button 8 (Start)
        
        // Clear all reserved areas
        map.report_id = 0x00;  // Initialize report ID
        memset(map.global_reserved, 0, sizeof(map.global_reserved));
        memset(map.normal_reserved, 0, sizeof(map.normal_reserved));
        memset(map.special_reserved, 0, sizeof(map.special_reserved));
        memset(map.future_reserved, 0, sizeof(map.future_reserved));

        map.ver = MAP_VER;  // Set version
        map.crc = crc8((uint8_t*)&map, sizeof(map) - 1); // Calculate CRC
    }
}

/**
 * Save mapping from RAM to High-Endurance Flash
 * @param normal_tbl Pointer to normal mode button-to-usage mapping table
 * @param special_tbl Pointer to special mode button-to-usage mapping table
 */
void Mapping_Save(const uint8_t *normal_tbl, const uint8_t *special_tbl) {
    // Copy new mapping tables to RAM structure
    memcpy(map.normal_tbl, normal_tbl, NUM_BUTTONS);
    memcpy(map.special_tbl, special_tbl, NUM_BUTTONS);
    
    // Update version and CRC, ensure report ID is set
    map.report_id = 0x00;  // Set report ID
    map.ver = MAP_VER;
    map.crc = crc8((uint8_t*)&map, sizeof(map) - 1);
    
    // Convert map structure to row buffer for flash write
    map_to_rowbuf();

    // Save to flash via FLASH_RowWrite
    uint8_t gie = INTCONbits.GIE;
    INTCONbits.GIE = 0;
    NVM_UnlockKeySet(UNLOCK_KEY);
    FLASH_PageErase(HEF_ADDR);  // Erase the page before writing
    while(NVM_IsBusy());  // Wait for erase to complete
    FLASH_RowWrite(HEF_ADDR, rowBuf);    // Write the row buffer to flash
    while(NVM_IsBusy());     
    NVM_UnlockKeyClear();
    INTCONbits.GIE = gie;
}

/**
 * Get the usage value for a physical button
 * @param physBtn Physical button index (0-7)
 * @param mode Mode selection (0=normal, 1=special)
 * @return Usage value
 */
uint8_t Mapping_GetUsage(uint8_t physBtn, uint8_t mode) {
    if (physBtn >= NUM_BUTTONS) return 0; // Invalid button index
    
    if (mode == 0) {
        return map.normal_tbl[physBtn];
    } else {
        return map.special_tbl[physBtn];
    }
}

/**
 * Copy mapping data from Feature Report buffer to the mapping table
 * @param featureReport The feature report buffer received from the host
 * @param length Length of the feature report data
 */
void Mapping_SetFromFeatureReport(uint8_t* featureReport, uint16_t length) {
    // Feature report structure: [Report ID + 63 bytes data] = 64 bytes total
    // Byte 0: Report ID, Byte 1: version, Byte 2: crc, Bytes 8-15: normal, Bytes 24-31: special
    
    // Ensure we have enough data for complete structure
    if (length < 64) {
        return; // Not enough data
    }
    
    uint8_t newNormalMapping[NUM_BUTTONS];
    uint8_t newSpecialMapping[NUM_BUTTONS];
    
    // Copy normal mode mapping (bytes 8-15 in feature report)
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        newNormalMapping[i] = featureReport[8 + i];
    }
    
    // Copy special mode mapping (bytes 24-31 in feature report)
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        newSpecialMapping[i] = featureReport[24 + i];
    }
    
    // Save both mapping tables to flash
    Mapping_Save(newNormalMapping, newSpecialMapping);
}

/**
 * Copy mapping data from the mapping table to the Feature Report buffer
 * @param featureReport The feature report buffer to be sent to the host
 */
void Mapping_GetAsFeatureReport(uint8_t* featureReport) {
    // Copy entire map structure (64 bytes) directly
    memcpy(featureReport, &map, sizeof(map));
    
    // Ensure Report ID is set correctly
    featureReport[0] = 0x00;  // Report ID 0
}