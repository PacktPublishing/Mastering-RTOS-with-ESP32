# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "bootloader\\bootloader.bin"
  "bootloader\\bootloader.elf"
  "bootloader\\bootloader.map"
  "bootstrap.min.css.S"
  "config\\sdkconfig.cmake"
  "config\\sdkconfig.h"
  "custom.min.css.S"
  "esp-idf\\esptool_py\\flasher_args.json.in"
  "esp-idf\\mbedtls\\x509_crt_bundle"
  "favicon.ico.S"
  "flash_app_args"
  "flash_bootloader_args"
  "flash_project_args"
  "flasher_args.json"
  "fonts.css.S"
  "gauge.min.js.S"
  "index.html.S"
  "ldgen_libraries"
  "ldgen_libraries.in"
  "main.bin"
  "main.map"
  "moment.min.js.S"
  "project_elf_src_esp32.c"
  "range.min.js.S"
  "scripts.js.S"
  "x509_crt_bundle.S"
  )
endif()
