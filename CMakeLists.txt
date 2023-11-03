include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
)

add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
#  	${MAX_SDK_INCLUDES}/common/commonsyms.c
)


IF(APPLE)
	file(GLOB PROJECT_SRC "*.h" "*.m" "*.cpp")
ENDIF(WIN32)

IF(WIN32)
	file(GLOB PROJECT_SRC "*.h" "*.c" "*.cpp")
	# TODO: adapt dwmapi.lib path to your system
	target_link_libraries(${PROJECT_NAME} PRIVATE "c:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64/dwmapi.lib")
ENDIF(WIN32)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
