set(INCLUDES
    "include/"
)
set(SOURCES
    "src/"
)

idf_component_register(SRC_DIRS ${SOURCES}
                       REQUIRES esp_adc
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ${INCLUDES}
                    )
