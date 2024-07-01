SECTIONS
{
    .mySection (NOLOAD) :
    {
        . = ALIGN(4);
        _mySection_start = .;
        KEEP(*(.mySection))
        . = _mySection_start + 0x1000; 
        _mySection_end = .;
    } > rtc_slow_seg
    
}