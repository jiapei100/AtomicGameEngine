include_directories (${ATOMIC_SOURCE_DIR}/Source/ThirdParty/rapidjson/include
                     ${ATOMIC_SOURCE_DIR}/Source/ThirdParty
                     ${ATOMIC_SOURCE_DIR}/Source/ThirdParty/Assimp/include
                     ${ATOMIC_SOURCE_DIR}/Source/ThirdParty/nativefiledialog
                     ${ATOMIC_SOURCE_DIR}/Source/ThirdParty/libcurl/include)

add_definitions(-DCPLUSPLUS_WITHOUT_QT -DCURL_STATICLIB)

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

if (APPLE)
    file (GLOB_RECURSE OBJC_FILES *.mm )
    set (SOURCE_FILES ${SOURCE_FILES} ${OBJC_FILES})
endif()

add_library(ToolCore ${SOURCE_FILES})

target_link_libraries(ToolCore Assimp Poco libcurl)

GroupSources("Assets")
GroupSources("Build")
GroupSources("Command")
GroupSources("Import")
GroupSources("JSBind")
GroupSources("License")
GroupSources("Net")
GroupSources("Platform")
GroupSources("Project")
GroupSources("Subprocess")
GroupSources("NETTools")
