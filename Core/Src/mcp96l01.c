/*
 * mcp96l01.c
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#include "mcp96l01.h"

// --- PI Tuning Parameters ---
#define KP 100        // Proportional gain (100ms ON-time per 1°C error)
#define KI 2          // Integral gain (adds 2ms ON-time per second per 1°C error)
#define PWM_WINDOW_MS 1000 // 1 Hz base frequency (1000ms window)
#define I_TERM_MAX 500 // Limit the integral term to 50% max power contribution

// --- State Variables ---
uint32_t pwm_window_start = 0;
uint32_t pwm_on_time = 0;
uint32_t    integral_sum = 0;

// "Късо съединение" (Short circuit)
const char STR_ERR_SHORT[] = "K\xC1" "co c" "\xC1" "e" "\xE3\xB6\xBB" "e" "\xBB\xB6" "e";

// "Липсва термодвойка" (Missing thermocouple)
const char STR_ERR_OC[]  = "\xA7\xB6\xBC" "c" "\xB2" "a " "\xBD" "ep" "\xBA" "o" "\xE3\xB2" "o" "\xB7\xB8" "a";


void Process_Heater_PI(void) {
    uint32_t now = HAL_GetTick();

    // 1. Safety First: Hardware Fault Override
    if (oc_detected || sc_detected){
    	Handle_OC_SC_Error();
    	return;
    }

    uint32_t time_in_window = now - pwm_window_start;

    if (time_in_window >= PWM_WINDOW_MS || pwm_window_start == 0) {
        pwm_window_start = now;
        time_in_window = 0;

        // Integer error calculation
        int16_t error = target_temp - current_temp;

        int32_t p_term = KP * error;

        // Integral Anti-Windup (only integrate within +/- 10 degrees)
        if (error < 10 && error > -10) {
            integral_sum += (KI * error);
        } else {
            integral_sum = 0;
        }

        // Clamp Integral term
        if (integral_sum > I_TERM_MAX) integral_sum = I_TERM_MAX;
        if (integral_sum < 0) integral_sum = 0;

        int32_t total_output = p_term + integral_sum;

        // Clamp total output to window size
        if (total_output > PWM_WINDOW_MS) {
            pwm_on_time = PWM_WINDOW_MS;
        } else if (total_output < 0) {
            pwm_on_time = 0;
        } else {
            pwm_on_time = (uint32_t)total_output;
        }
    }

    if (time_in_window < pwm_on_time) {
        HAL_GPIO_WritePin(TEMP_CONTROL_GPIO_Port, TEMP_CONTROL_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(TEMP_CONTROL_GPIO_Port, TEMP_CONTROL_Pin, GPIO_PIN_RESET);
    }
}

void Handle_OC_SC_Error(){
	if ( HAL_GPIO_ReadPin(MDI_OCALERT_GPIO_Port, MDI_OCALERT_Pin) ){
		pwm_on_time = 0;
        HAL_GPIO_WritePin(TEMP_CONTROL_GPIO_Port, TEMP_CONTROL_Pin, GPIO_PIN_RESET);
		LCD_SetCursor(0, 0);
		LCD_PrintRaw(STR_ERR_OC);

	}
	if ( HAL_GPIO_ReadPin(SCALERT_GPIO_Port, SCALERT_Pin) ){
		pwm_on_time = 0;
        HAL_GPIO_WritePin(TEMP_CONTROL_GPIO_Port, TEMP_CONTROL_Pin, GPIO_PIN_RESET);
		LCD_SetCursor(0, 0);
		LCD_PrintRaw(STR_ERR_SHORT);
	}
}


// Helper to write register
static HAL_StatusTypeDef WriteReg(MCP96L01_HandleTypeDef *dev, uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(dev->hi2c, dev->i2c_addr, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

// Helper to read register
static HAL_StatusTypeDef ReadRegs(MCP96L01_HandleTypeDef *dev, uint8_t reg, uint8_t *data, uint16_t len) {
    return HAL_I2C_Mem_Read(dev->hi2c, dev->i2c_addr, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

/**
 * @brief  Initialize the MCP96L01 sensor
 */
HAL_StatusTypeDef MCP96L01_Init(MCP96L01_HandleTypeDef *dev, I2C_HandleTypeDef *hi2c, uint8_t addr) {
    dev->hi2c = hi2c;
    dev->i2c_addr = addr;

    if (HAL_I2C_IsDeviceReady(dev->hi2c, dev->i2c_addr, 5, 100) != HAL_OK) return HAL_ERROR;

    // Hardcoded: Type K (0x00) | Filter Mid (0x04)
    if (WriteReg(dev, REG_SENSOR_CONFIG, 0x04) != HAL_OK) return HAL_ERROR;

    // Device Config: 18-bit ADC
    if (WriteReg(dev, REG_DEVICE_CONFIG, 0x00) != HAL_OK) return HAL_ERROR;

    return HAL_OK;
}

/**
 * @brief  Read the Hot Junction (Thermocouple) Temperature
 * @return Temperature in Celsius (integer)
 */
int16_t MCP96L01_ReadHotJunction(MCP96L01_HandleTypeDef *dev) {
    uint8_t buff[2];

    if (ReadRegs(dev, 0x00, buff, 2) != HAL_OK) {
        return -999; // Error state
    }

    int16_t raw = (int16_t)((buff[0] << 8) | buff[1]);

    // Divide by 16 to convert the 0.0625 LSB to a flat integer degree
    return raw / 16;
}


/**
 * @brief  Read the Cold Junction (Chip) Temperature
 */
int16_t MCP96L01_ReadColdJunction(MCP96L01_HandleTypeDef *dev) {
    uint8_t buff[2];
    if (ReadRegs(dev, REG_COLD_JUNCTION, buff, 2) != HAL_OK) return -999;
    int16_t raw = (int16_t)((buff[0] << 8) | buff[1]);
    // Divide by 16 to convert the 0.0625 LSB to a flat integer degree
    return raw / 16;
}

/**
 * @brief  Get Status Register (Check for Shorts/Opens)
 * @return Byte: Bit 6=Open Circuit, Bit 5=Short Circuit (on L01 variants)
 */
uint8_t MCP96L01_GetStatus(MCP96L01_HandleTypeDef *dev) {
    uint8_t status = 0;
    ReadRegs(dev, REG_STATUS, &status, 1);
    return status;
}

/**
 * @brief  Configure an hardware ALERT pin
 * @param  alertNum: ALERT_1 through ALERT_4
 * @param  tempLimit: Temperature threshold
 * @param  activeHigh: 1 for Active High, 0 for Active Low
 */
HAL_StatusTypeDef MCP96L01_ConfigureAlertLimit(MCP96L01_HandleTypeDef *dev, MCP96_AlertPad_t alertNum, float tempLimit, uint8_t activeHigh) {
    uint8_t configReg, limitReg;

    switch(alertNum) {
        case ALERT_1: configReg = REG_ALERT1_CONFIG; limitReg = REG_ALERT1_LIMIT; break;
        case ALERT_2: configReg = REG_ALERT2_CONFIG; limitReg = REG_ALERT2_LIMIT; break;
        case ALERT_3: configReg = REG_ALERT3_CONFIG; limitReg = REG_ALERT3_LIMIT; break;
        case ALERT_4: configReg = REG_ALERT4_CONFIG; limitReg = REG_ALERT4_LIMIT; break;
        default: return HAL_ERROR;
    }

    // The Alert Limit register expects a resolution of 0.25C per LSB.
        // To convert our integer tempLimit to 0.25C formatting, we multiply by 4.
        int16_t limitVal = tempLimit * 4;

        uint8_t limitBytes[2];
        limitBytes[0] = (limitVal >> 8) & 0xFF;
        limitBytes[1] = limitVal & 0xFF;

        if (HAL_I2C_Mem_Write(dev->hi2c, dev->i2c_addr, limitReg, I2C_MEMADD_SIZE_8BIT, limitBytes, 2, 100) != HAL_OK) return HAL_ERROR;

        uint8_t configVal = 0x01; // Enable Alert
        if (activeHigh) configVal |= (1 << 2);

        return WriteReg(dev, configReg, configVal);
}
