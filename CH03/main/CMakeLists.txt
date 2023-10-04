idf_component_register(SRCS "main.c" "task_blink.c" "task_wifi.c" "utils.c" "webserver.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "favicon.ico" "../data/index.html" "../data/bootstrap/bootstrap.min.css" "../data/bootstrap/fonts.css" "../data/bootstrap/gauge.min.js" "../data/bootstrap/scripts.js")

# spiffs_create_partition_image(spiffs ../data FLASH_IN_PROJECT)