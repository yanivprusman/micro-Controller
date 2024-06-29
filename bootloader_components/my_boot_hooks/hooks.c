#include "esp_log.h"
// #include <stdio.h>
/* Function used to tell the linker to include this file
 * with all its symbols.
 */
void bootloader_hooks_include(void){
}


void bootloader_before_init(void) {
    /* Keep in my mind that a lot of functions cannot be called from here
     * as system initialization has not been performed yet, including
     * BSS, SPI flash, or memory protection. */
    ESP_LOGI("HOOK", "This hook is called BEFORE bootloader initialization");
    
    // printf("asdf\n");
}
// extern function_ptr_t _mySection_start;
#include "../../main/mySection.h"
// typedef void (*function_ptr_t)(void);
// function_ptr_t *section_start = (function_ptr_t *)&_mySection_start;
// *section_start = theDo;
void bootloader_after_init(void) {
    ESP_LOGI("HOOK", "This hook is called AFTER bootloader initialization");
    // _mySection_start = 4 ;
    // (*section_start)();
// ESP_LOGI("HOOK", "%p",theDo);
    // typedef void (*function_ptr_t)(void);
    // RTC_DATA_ATTR function_ptr_t data1 =&theDo;
    // theDo();

//    if (rtc_function_pointer != NULL) {
//         rtc_function_pointer();  // Call the function stored in RTC memory
//     } else {
//         ESP_LOGI("HOOK"," error\n");
//         // Handle error or default behavior if no function pointer is set
//     }    

}
