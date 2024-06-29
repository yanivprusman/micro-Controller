#include "test.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "esp_image_format.h"
#include "esp_flash.h"
#include "esp_partition.h"
#include "esp_system.h"
esp_err_t read_image_metadata(const esp_partition_t *partition, esp_image_metadata_t *metadata) {
    if (partition == NULL || metadata == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Initialize the metadata structure to zero
    memset(metadata, 0, sizeof(esp_image_metadata_t));
    metadata->start_addr = partition->address;

    // Read the image header
    esp_err_t err = esp_partition_read(partition, 0, &metadata->image, sizeof(esp_image_header_t));
    if (err != ESP_OK) {
        printf("read_image_metadata Failed to read image header: %s", esp_err_to_name(err));
        return err;
    }

    // Read segment headers and calculate offsets
    uint32_t offset = sizeof(esp_image_header_t);
    for (int i = 0; i < metadata->image.segment_count; i++) {
        err = esp_partition_read(partition, offset, &metadata->segments[i], sizeof(esp_image_segment_header_t));
        if (err != ESP_OK) {
            printf("read_image_metadata Failed to read segment header %d: %s", i, esp_err_to_name(err));
            return err;
        }
        metadata->segment_data[i] = offset + sizeof(esp_image_segment_header_t);
        offset += sizeof(esp_image_segment_header_t) + metadata->segments[i].data_len;
    }

    // Calculate the total image length
    metadata->image_len = offset;

    // Read the SHA-256 digest
    err = esp_partition_read(partition, offset, metadata->image_digest, sizeof(metadata->image_digest));
    if (err != ESP_OK) {
        printf("read_image_metadata Failed to read image digest: %s", esp_err_to_name(err));
        return err;
    }

    // Read the secure version if anti-rollback is enabled
    #if CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK
    err = esp_partition_read(partition, offset + sizeof(metadata->image_digest), &metadata->secure_version, sizeof(metadata->secure_version));
    if (err != ESP_OK) {
        ESP_LOGE("read_image_metadata", "Failed to read secure version: %s", esp_err_to_name(err));
        return err;
    }
    #endif

    return ESP_OK;
}
void printPartitions(void) {
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (it == NULL) {
        printf("app_main No application partitions found\n");
        return;
    }

    while (it != NULL) {
        const esp_partition_t *partition = esp_partition_get(it);
        esp_image_metadata_t metadata;

        esp_err_t err = read_image_metadata(partition, &metadata);
        if (err == ESP_OK) {
            printf("app_main Partition: %s\n", partition->label);
            printf("app_main Image Start Address: 0x%08lx\n", metadata.start_addr);
            printf("app_main Image Length: %ld bytes\n", metadata.image_len);
            printf("app_main Secure Version: %ld\n", metadata.secure_version);
            printf("app_main Image Digest: ");
            for (int i = 0; i < sizeof(metadata.image_digest); i++) {
                printf("%02x", metadata.image_digest[i]);
            }
            printf("\n\n");
        } else {
            printf("app_main Failed to read image metadata: %s\n", esp_err_to_name(err));
        }

        it = esp_partition_next(it);
    }

    esp_partition_iterator_release(it);
}


void tryToCallAppMain(void) {
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    if (partition == NULL) {
        printf("app_main OTA_1 partition not found\n");
        return;
    }

    esp_image_header_t image_header;
    esp_err_t err = esp_partition_read(partition, 0, &image_header, sizeof(esp_image_header_t));
    if (err != ESP_OK) {
        printf("app_main Failed to read image header: %s\n", esp_err_to_name(err));
        return;
    }

    printf("app_main OTA_1 Partition Entry Point (app_main) Address: 0x%08lx\n", image_header.entry_addr);
    void(*f)(void)=image_header.entry_addr;
    f();
}

typedef void (*app_main_function_t)(void);

void execute_app_main(const esp_partition_t *partition) {
    if (partition == NULL) {
        printf("Partition is NULL, cannot execute app_main.\n");
        return;
    }

    esp_image_header_t image_header;
    esp_err_t err = esp_partition_read(partition, 0, &image_header, sizeof(esp_image_header_t));
    if (err != ESP_OK) {
        printf("Failed to read image header: %s\n", esp_err_to_name(err));
        return;
    }

    app_main_function_t app_main = (app_main_function_t)image_header.entry_addr;
    printf("app_main Entry Address: 0x%08lx\n", image_header.entry_addr);

    if (app_main != NULL) {
        printf("Executing app_main...\n");
        app_main(); // Execute app_main function
    } else {
        printf("app_main function pointer is NULL, cannot execute.\n");
    }
}
// #include "bootloader_init.h"
// #include "bootloader_utility.h"
// #include "bootloader_common.h"

void test(void) {
    printf("in the program\n");
    return;
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    if (partition == NULL) {
        printf("OTA_1 partition not found\n");
        return;
    }

    // execute_app_main(partition);
    // bootloader_state_t bs = {0};
    // bootloader_utility_load_boot_image(&bs, boot_index);

}