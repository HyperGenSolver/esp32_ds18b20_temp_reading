# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/cedri/esp/v5.2.1/esp-idf/components/bootloader/subproject"
  "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader"
  "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix"
  "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix/tmp"
  "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix/src"
  "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Git/ESP32_DS18B20_Temp_reading/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
