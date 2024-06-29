$flash_args = Get-Content .\customBuild\flashArgs | Where-Object { $_ -notmatch '^\s*#' }
$flash_args_str = $flash_args -join "`n"
$temp_flash_args_file = "customBuild\flashArgsUncommented"
$flash_args_str | Out-File -FilePath $temp_flash_args_file -Encoding ASCII
# python -m esptool --chip esp32c3 -b 460800 --before default_reset --after hard_reset write_flash $temp_flash_args_file
# idf.py  monitor
