/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdbool.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "bootloader_init.h"
#include "bootloader_utility.h"
#include "bootloader_common.h"
static const char* TAG = "boot";

static int select_partition_number(bootloader_state_t *bs);
void printBS(bootloader_state_t *bs){
    
    esp_rom_printf("boot state:\n");
    esp_rom_printf("ota_info\n");
    esp_rom_printf("-offset: %d\n", bs->ota_info.offset);
    esp_rom_printf("-size: %d\n", bs->ota_info.size);
    esp_rom_printf("factory\n");
    esp_rom_printf("-offset: %d\n", bs->factory.offset);
    esp_rom_printf("-size: %d\n", bs->factory.size);
    esp_rom_printf("test\n");
    esp_rom_printf("-offset: %d\n", bs->test.offset);
    esp_rom_printf("-size: %d\n;", bs->test.size);
    esp_rom_printf("ota[MAX_OTA_SLOTS]: %d\n",MAX_OTA_SLOTS);
    esp_rom_printf("app_count: %d\n", bs->app_count);
    esp_rom_printf("selected_subtype %d\n;", bs->selected_subtype);


}
/*
 * We arrive here after the ROM bootloader finished loading this second stage bootloader from flash.
 * The hardware is mostly uninitialized, flash cache is down and the app CPU is in reset.
 * We do have a stack, so we can do the initialization in C.
 */
void __attribute__((noreturn)) call_start_cpu0(void)
{
    // 1. Hardware initialization
    if (bootloader_init() != ESP_OK) {
        bootloader_reset();
    }

#ifdef CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP
    // If this boot is a wake up from the deep sleep then go to the short way,
    // try to load the application which worked before deep sleep.
    // It skips a lot of checks due to it was done before (while first boot).
    bootloader_utility_load_boot_image_from_deep_sleep();
    // If it is not successful try to load an application as usual.
#endif

    // 2. Select the number of boot partition
    bootloader_state_t bs = {0};
    int boot_index = select_partition_number(&bs);
    if (boot_index == INVALID_INDEX) {
        bootloader_reset();
    }
    printBS(&bs);
    // 3. Load the app image for booting
    bootloader_utility_load_boot_image(&bs, boot_index);
}

// Select the number of boot partition
static int select_partition_number(bootloader_state_t *bs)
{
    // 1. Load partition table
    if (!bootloader_utility_load_partition_table(bs)) {
        ESP_LOGE(TAG, "load partition table error!");
        return INVALID_INDEX;
    }

    // 2. Select the number of boot partition
    return bootloader_utility_get_selected_boot_partition(bs);
}

// Return global reent struct if any newlib functions are linked to bootloader
struct _reent *__getreent(void)
{
    return _GLOBAL_REENT;
}
