#ifndef MY_SECTION_H
#define MY_SECTION_H
typedef void (*function_ptr_t)(void);
// function_ptr_t __attribute__((section(".mySection"))) myFunctionPointer;
// extern function_ptr_t _mySection_start;
extern function_ptr_t _mySection_end;

// extern function_ptr_t _mySection_start ;
void theDo();
#endif // MY_SECTION_H
