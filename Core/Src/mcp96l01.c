/*
 * mcp96l01.c
 *
 *  Created on: 15.02.2026 г.
 *      Author: Valio
 */

#include "mcp96l01.h"

#define PWM_WINDOW_MS 1000 // 1 Hz base frequency (1000ms window)
#define PROP_BAND     10.0f // Proportional band (start dimming 10C before target)

// --- PI Tuning Parameters ---
#define KP 100.0f        // Proportional gain (100ms ON-time per 1°C error)
#define KI 2.0f          // Integral gain (adds 2ms ON-time per second per 1°C error)
#define PWM_WINDOW_MS 1000
#define I_TERM_MAX 500.0f // Limit the integral term to 50% max power contribution

// --- State Variables ---
uint32_t pwm_window_start = 0;
uint32_t pwm_on_time = 0;
float    integral_sum = 0.0f;

void Process_Heater_PI(void) {
    uint32_t now = HAL_GetTick();

    // 1. Safety First: Hardware Fault Override
    if (oc_detected || sc_detected){
    	Handle_OC_SC_Error();
    	return;
    }

    uint32_t time_in_window = now - pwm_window_start;

    // 2. Calculate new PI output once per window (1Hz update rate)
    if (time_in_window >= PWM_WINDOW_MS || pwm_window_start == 0) {
        pwm_window_start = now;
        time_in_window = 0; // Reset for the new cycle

        float error = target_temp - current_temp;

        // Proportional Term
        float p_term = KP * error;

        // Integral Term with Anti-Windup
        // Only integrate when we are within +/- 10°C of the target.
        if (error < 10.0f && error > -10.0f) {
            integral_sum += (KI * error);
        } else {
            // If we make a large setpoint change, clear the integral memory
            integral_sum = 0.0f;
        }

        // Constrain the Integral term (Anti-windup limits)
        if (integral_sum > I_TERM_MAX) integral_sum = I_TERM_MAX;
        if (integral_sum < 0.0f) integral_sum = 0.0f; // Heater can't actively cool

        // Calculate total required ON time
        float total_output = p_term + integral_sum;

        // Constrain total output to our 0-1000ms window
        if (total_output > PWM_WINDOW_MS) {
            pwm_on_time = PWM_WINDOW_MS;
        } else if (total_output < 0.0f) {
            pwm_on_time = 0;
        } else {
            pwm_on_time = (uint32_t)total_output;
        }
    }

    // 3. Execute the Slow PWM
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
		LCD_Print("Грешка Липсва   Термодвойка     ");

	}
	if ( HAL_GPIO_ReadPin(SCALERT_GPIO_Port, SCALERT_Pin) ){
		pwm_on_time = 0;
        HAL_GPIO_WritePin(TEMP_CONTROL_GPIO_Port, TEMP_CONTROL_Pin, GPIO_PIN_RESET);
		LCD_SetCursor(0, 0);
		LCD_Print("Грешка Късо     Съединение      ");
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
HAL_StatusTypeDef MCP96L01_Init(MCP96L01_HandleTypeDef *dev, I2C_HandleTypeDef *hi2c, uint8_t addr, MCP96_TCType_t type) {
    dev->hi2c = hi2c;
    dev->i2c_addr = addr;

    // Check if device is alive
    if (HAL_I2C_IsDeviceReady(dev->hi2c, dev->i2c_addr, 5, 100) != HAL_OK) {
        return HAL_ERROR;
    }

    // Set Type and Filter (Defaulting filter to Mid-range 4 for stability)
    // Register 0x05: [Filter 2:0] [Type 6:4]
    uint8_t config = (type << 4) | 4;

    if (WriteReg(dev, REG_SENSOR_CONFIG, config) != HAL_OK) {
        return HAL_ERROR;
    }

    // Set Device Config (Resolution/Cold Junction resolution)
    // 0x00 = Cold Junction Res 0.0625°C, ADC 18-bit (Best resolution)
    if (WriteReg(dev, REG_DEVICE_CONFIG, 0x00) != HAL_OK) {
        return HAL_ERROR;
    }

    dev->tc_type = type;
    return HAL_OK;
}

/**
 * @brief  Change Thermocouple Type on the fly
 */
HAL_StatusTypeDef MCP96L01_SetThermocoupleType(MCP96L01_HandleTypeDef *dev, MCP96_TCType_t type) {
    // We must read existing filter config to not overwrite it
    uint8_t current_conf;
    if (ReadRegs(dev, REG_SENSOR_CONFIG, &current_conf, 1) != HAL_OK) return HAL_ERROR;

    // Clear bits 4,5,6 (Type) and set new type
    current_conf &= ~(0x70);
    current_conf |= (type << 4);

    dev->tc_type = type;
    return WriteReg(dev, REG_SENSOR_CONFIG, current_conf);
}

/**
 * @brief  Read the Hot Junction (Thermocouple) Temperature
 * @return Temperature in Celsius (float)
 */
float MCP96L01_ReadHotJunction(MCP96L01_HandleTypeDef *dev) {
    uint8_t buff[2];

    // Read Register 0x00 (Hot Junction)
    if (ReadRegs(dev, REG_HOT_JUNCTION, buff, 2) != HAL_OK) {
        return -999.0f; // Error value
    }

    // Convert: Big Endian int16
    int16_t raw = (int16_t)((buff[0] << 8) | buff[1]);

    // Scale: LSB is 0.0625 degrees C
    return (float)raw * 0.0625f;
}

/**
 * @brief  Read the Cold Junction (Chip) Temperature
 */
float MCP96L01_ReadColdJunction(MCP96L01_HandleTypeDef *dev) {
    uint8_t buff[2];
    if (ReadRegs(dev, REG_COLD_JUNCTION, buff, 2) != HAL_OK) return -999.0f;
    int16_t raw = (int16_t)((buff[0] << 8) | buff[1]);
    return (float)raw * 0.0625f; // Same scaling for standard config
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

    // 1. Calculate Limit Register Value (int16 / 0.25 resolution for Limit register? Check datasheet)
    // Note: Alert Limits in MCP9600 are often resolution 0.25°C or matching T_H
    // MCP9600 Datasheet: Alert Limit is 16-bit, resolution matches device config (usually 0.0625)
    int16_t limitVal = (int16_t)(tempLimit / 0.0625f);
    uint8_t limitBytes[2];
    limitBytes[0] = (limitVal >> 8) & 0xFF;
    limitBytes[1] = limitVal & 0xFF;

    if (HAL_I2C_Mem_Write(dev->hi2c, dev->i2c_addr, limitReg, I2C_MEMADD_SIZE_8BIT, limitBytes, 2, 100) != HAL_OK) return HAL_ERROR;

    // 2. Configure Alert Logic
    // Bit 0: Alert Enable
    // Bit 1: Interrupt Mode (0 = Comparator)
    // Bit 2: Active High/Low
    // Bit 3: Fall/Rise
    // Bit 4: Cold/Hot Junction (0 = Hot Junction)
    uint8_t configVal = 0x01; // Enable
    if (activeHigh) configVal |= (1 << 2);

    return WriteReg(dev, configReg, configVal);
}
