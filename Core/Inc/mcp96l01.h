/*
 * mcp96l01.h
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#ifndef INC_MCP96L01_H_
#define INC_MCP96L01_H_


#include "main.h" // Includes your GPIO definitions and HAL

extern signed short current_temp;

// Default I2C Address (A0=0) - Shifted for HAL (0x60 << 1) or (0x67 << 1)
// Check your schematic for A0 pin status.
#define MCP96L01_I2C_ADDR          (0x60 << 1)

// --- Register Map ---
#define REG_HOT_JUNCTION           0x00
#define REG_JUNCTION_TEMP_DELTA    0x01
#define REG_COLD_JUNCTION          0x02
#define REG_RAW_DATA               0x03
#define REG_STATUS                 0x04
#define REG_SENSOR_CONFIG          0x05
#define REG_DEVICE_CONFIG          0x06
#define REG_ALERT1_CONFIG          0x08
#define REG_ALERT2_CONFIG          0x09
#define REG_ALERT3_CONFIG          0x0A
#define REG_ALERT4_CONFIG          0x0B
#define REG_ALERT1_HYST            0x0C
#define REG_ALERT2_HYST            0x0D
#define REG_ALERT3_HYST            0x0E
#define REG_ALERT4_HYST            0x0F
#define REG_ALERT1_LIMIT           0x10
#define REG_ALERT2_LIMIT           0x11
#define REG_ALERT3_LIMIT           0x12
#define REG_ALERT4_LIMIT           0x13
#define REG_DEVICE_ID              0x20

// --- Thermocouple Types (for Register 0x05) ---
typedef enum {
    TC_TYPE_K = 0x00,
    TC_TYPE_J = 0x01,
    TC_TYPE_T = 0x02,
    TC_TYPE_N = 0x03,
    TC_TYPE_S = 0x04,
    TC_TYPE_E = 0x05,
    TC_TYPE_B = 0x06,
    TC_TYPE_R = 0x07
} MCP96_TCType_t;

// --- Filter Coefficients ---
typedef enum {
    FILTER_OFF = 0,
    FILTER_MIN = 1,
    FILTER_MID = 4,
    FILTER_MAX = 7
} MCP96_FilterCoeff_t;

// --- Alert Pad Selection ---
typedef enum {
    ALERT_1 = 1,
    ALERT_2 = 2,
    ALERT_3 = 3,
    ALERT_4 = 4
} MCP96_AlertPad_t;

// --- Main Struct ---
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t i2c_addr;
    MCP96_TCType_t tc_type;
} MCP96L01_HandleTypeDef;

// --- Function Prototypes ---
void Handle_OC_SC_Error();
void Process_Heater_PI(void);
HAL_StatusTypeDef MCP96L01_Init(MCP96L01_HandleTypeDef *dev, I2C_HandleTypeDef *hi2c, uint8_t addr, MCP96_TCType_t type);
HAL_StatusTypeDef MCP96L01_SetThermocoupleType(MCP96L01_HandleTypeDef *dev, MCP96_TCType_t type);
float MCP96L01_ReadHotJunction(MCP96L01_HandleTypeDef *dev);
float MCP96L01_ReadColdJunction(MCP96L01_HandleTypeDef *dev);
uint8_t MCP96L01_GetStatus(MCP96L01_HandleTypeDef *dev);

// Configuration for your Alert Pins
HAL_StatusTypeDef MCP96L01_ConfigureAlertLimit(MCP96L01_HandleTypeDef *dev, MCP96_AlertPad_t alertNum, float tempLimit, uint8_t activeHigh);


#endif /* INC_MCP96L01_H_ */
