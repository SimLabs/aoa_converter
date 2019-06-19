FILE(GLOB HEADER_FILES
    ${ACTUAL_3RDPARTY_DIR}/include/alloc/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/binary/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/common/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/cpp_utils/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/geometry/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/logger/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/reflection/*.h
    ${ACTUAL_3RDPARTY_DIR}/include/serialization/*.h
)

SET(TARGET_H
	${HEADER_FILES}
)