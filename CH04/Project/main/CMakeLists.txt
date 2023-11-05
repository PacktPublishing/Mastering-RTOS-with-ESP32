idf_component_register(SRCS 
                    "main.c" 
                    "configuration.c" 
                    "task_blink.c" 
                    "task_wifi.c" 
                    "utils.c" 
                    "webserver.c" 
                    "peripheral_adc.c" 
                    "peripheral_gpio.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES 
                    "../data/favicon.ico" 
                    "../data/index.html" 
                    "../data/bootstrap/bootstrap.min.css" 
                    "../data/bootstrap/custom.min.css" 
                    "../data/bootstrap/fonts.css" 
                    "../data/bootstrap/gauge.min.js" 
                    "../data/bootstrap/moment.min.js" 
                    "../data/bootstrap/range.min.js" 
                    "../data/bootstrap/scripts.js")

# spiffs_create_partition_image(spiffs ../data FLASH_IN_PROJECT)