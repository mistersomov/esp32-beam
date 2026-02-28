set(INCLUDES
    "include/"
)
set(SOURCES
    "src/"
)

idf_component_register(SRC_DIRS ${SOURCES}
                       INCLUDE_DIRS ${INCLUDES}
                    )
