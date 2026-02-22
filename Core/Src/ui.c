#include "ui.h"

// --- Configuration Definitions ---
#define UNLOCK_HOLD_MS       3000   // 3 seconds to unlock
#define AUTO_LOCK_MS         60000  // 1 minute of inactivity to lock
#define SCROLL_DELAY_MS      250    // Speed of temperature change when holding (250ms = 4 ticks per second)
#define TEMP_INCREMENT       2.0f   // Change by 2 degrees

// --- UI State Variables ---
uint8_t  ui_unlocked = 0;
uint32_t last_activity_time = 0;
uint32_t both_pressed_start_time = 0;
uint32_t last_scroll_time = 0;

void Process_UI(void) {
    uint32_t now = HAL_GetTick();

    // Read your button states (Assuming Active-Low / Pull-Ups used)
    uint8_t btn_up = (HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_RESET);
    uint8_t btn_dn = (HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin) == GPIO_PIN_RESET);

    // 1. Check for Auto-Lock timeout
    if (ui_unlocked && (now - last_activity_time > AUTO_LOCK_MS)) {
        ui_unlocked = 0; // Lock the UI
        // Optional: Trigger a beep or UI update here to indicate locking
    }

    // 2. Handling the Locked State (Checking for 3-second hold)
    if (!ui_unlocked) {
        if (btn_up && btn_dn) {
            // Both buttons are being held
            if (both_pressed_start_time == 0) {
                both_pressed_start_time = now; // Start the 3-second timer
            }
            else if (now - both_pressed_start_time >= UNLOCK_HOLD_MS) {
                // 3 seconds have passed! Unlock the UI.
                ui_unlocked = 1;
                last_activity_time = now;
                both_pressed_start_time = 0; // Reset timer so it doesn't re-trigger
            }
        } else {
            // Buttons were released before 3 seconds
            both_pressed_start_time = 0;
        }
        return; // Exit function early since we are locked
    }

    // 3. Handling the Unlocked State (Adjusting Temperature)
    if (btn_up || btn_dn) {
        last_activity_time = now; // Keep the 1-minute timer alive

        // Prevent action if both are pressed while unlocked
        if (btn_up && btn_dn) return;

        // Execute increment/decrement based on scroll delay
        if (now - last_scroll_time >= SCROLL_DELAY_MS) {
            if (btn_up) target_temp += TEMP_INCREMENT;
            if (btn_dn) target_temp -= TEMP_INCREMENT;

            // Optional: Clamp target_temp to a max/min safe value
            if (target_temp > 400.0f) target_temp = 400.0f;
            if (target_temp < 0.0f) target_temp = 0.0f;

            last_scroll_time = now;
        }
    } else {
        // If no buttons are pressed, reset the scroll timer.
        // Subtracting SCROLL_DELAY_MS ensures that the VERY NEXT press is registered instantly,
        // rather than waiting 250ms for the first click.
        last_scroll_time = now - SCROLL_DELAY_MS;
    }
}
