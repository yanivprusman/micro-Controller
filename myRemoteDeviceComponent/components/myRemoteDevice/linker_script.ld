
MEMORY
{
  rtc_data_mem : ORIGIN = 0x50000000, LENGTH = 0x4000
}

/* Define a section for RTC memory */
SECTIONS
{
  .mySection :
  {
    KEEP(*(mySection))  /* Keep the content of this section */
  } > rtc_data_mem
}
