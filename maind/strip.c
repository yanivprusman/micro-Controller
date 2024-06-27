#include "myRemoteDevice.h"

void setLedStripColor(int index, int red, int green, int blue){
    static bool initialized = false;
    static led_strip_handle_t led_strip;
    if (!initialized){
        initialized = true;
        int BLINK_GPIO = 8;
        led_strip_config_t strip_config = {
            .strip_gpio_num = BLINK_GPIO,
            .max_leds = 1, // at least one LED on board
        };
        led_strip_rmt_config_t rmt_config = {
            .resolution_hz = 10 * 1000 * 1000, // 10MHz
            .flags.with_dma = false,
        };
        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    }
    // led_strip_clear(led_strip);
    led_strip_set_pixel(led_strip, index, red, green, blue);
    led_strip_refresh(led_strip);
}
