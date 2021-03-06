CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(KVEngineQt)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(UNPACKER_DIR QzipUnpacker/)
SET(LAUNCHER_DIR Launcher/)
SET(SOURCE_DIR Sources/)
SET(EXEC_DIR Exec/)

FIND_PACKAGE(Qt5Network REQUIRED)
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5OpenGL REQUIRED)

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(SDLDIR ON)
ELSE()
    IF(DEFINED ENV{SDLDIR})
        SET(SDLDIR ON)
    ENDIF()
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(SDLIMAGEDIR ON)
ELSE()
    IF(DEFINED ENV{SDLIMAGEDIR})
        SET(SDLIMAGEDIR ON)
    ENDIF()
ENDIF()

IF(SDLDIR)
    FIND_PACKAGE(SDL)
ENDIF()
IF(SDLIMAGEDIR)
    FIND_PACKAGE(SDL_image)
ENDIF()

IF(SDL_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
ENDIF()
IF (SDL_IMAGE_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${SDL_IMAGE_INCLUDE_DIR})
ENDIF()

INCLUDE_DIRECTORIES(SOURCE_DIR)
INCLUDE_DIRECTORIES(LAUNCHER_DIR)
INCLUDE_DIRECTORIES(UNPACKER_DIR)

#Workaround for linux
INCLUDE_DIRECTORIES("Sources")
INCLUDE_DIRECTORIES("Launcher")
INCLUDE_DIRECTORIES("QzipUnpacker")

#INCLUDE_DIRECTORIES("Sources/Qt")
#INCLUDE_DIRECTORIES("Sources/Objects")
#INCLUDE_DIRECTORIES("Sources/representation")

INCLUDE_DIRECTORIES($ENV{PNG_INCLUDE_DIR})
INCLUDE_DIRECTORIES($ENV{SFML_INCLUDE_DIR})

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(GIT_CMD git)
ELSE()
    SET(GIT_CMD "$ENV{GIT_PATH}")
ENDIF()

EXECUTE_PROCESS(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GIT_CMD} describe --tags --abbrev=4
    OUTPUT_VARIABLE CLIENT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
MESSAGE(STATUS ${CLIENT_VERSION})

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "-DDEFINED_VERSION=\"${CLIENT_VERSION}\" ${CMAKE_CXX_FLAGS}")
ELSE()
    SET(CMAKE_CXX_FLAGS "/DDEFINED_VERSION=\"${CLIENT_VERSION}\" ${CMAKE_CXX_FLAGS}")
ENDIF()

#SET(CMAKE_EXE_LINKER_FLAGS "/DEBUG")

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(GO_CMD go)
ELSE()
    SET(GO_CMD "$ENV{GO_PATH}")
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(GOFMT_CMD gofmt)
ELSE()
    SET(GOFMT_CMD "$ENV{GOFMT_PATH}")
ENDIF()

MESSAGE(STATUS ${GOFMT_CMD})
MESSAGE(STATUS ${GO_CMD})

IF (WIN32)
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/knv-server-go
        COMMAND ${GOFMT_CMD} -w .)
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/knv-server-go
        COMMAND ${GO_CMD} build -ldflags "-X main.clientVersionBuild ${CLIENT_VERSION}")
ELSE()
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND bash build_go_server.sh ${CLIENT_VERSION})
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(PYTHON_CMD python2.7)
ELSE()
    SET(PYTHON_CMD "$ENV{PYTHON_PATH}")
ENDIF()

FILE(GLOB_RECURSE SOURCES "${SOURCE_DIR}*.cpp")
FILE(GLOB_RECURSE HEADERS "${SOURCE_DIR}*.h")
FILE(GLOB_RECURSE FORMS "${SOURCE_DIR}*.ui")

ADD_CUSTOM_COMMAND(
    OUTPUT AutogenMetadata.cpp
    COMMAND ${PYTHON_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/metadata_generator.py ${CMAKE_CURRENT_SOURCE_DIR} metadata.json
    COMMAND ${PYTHON_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/autogen.py
    DEPENDS ${HEADERS})

LIST(APPEND SOURCES "${CMAKE_BINARY_DIR}/AutogenMetadata.cpp")

QT5_WRAP_UI(FORMS_HEADERS ${FORMS})

ADD_EXECUTABLE(KVEngine WIN32 ${SOURCES} ${HEADERS} ${FORMS_HEADERS})

ADD_DEPENDENCIES(KVEngine metadata)

qt5_use_modules(KVEngine Core)
qt5_use_modules(KVEngine Network)
qt5_use_modules(KVEngine Widgets)
qt5_use_modules(KVEngine OpenGL)

IF(${SDLDIR})
    TARGET_LINK_LIBRARIES(KVEngine ${SDL_LIBRARY})
ENDIF()
IF(${SDLIMAGEDIR})
    TARGET_LINK_LIBRARIES(KVEngine ${SDL_IMAGE_LIBRARY})
ENDIF()

FILE(GLOB_RECURSE LAUNCHER_SOURCES "${LAUNCHER_DIR}*.cpp")
FILE(GLOB_RECURSE LAUNCHER_HEADERS "${LAUNCHER_DIR}*.h")
FILE(GLOB_RECURSE LAUNCHER_FORMS "${LAUNCHER_DIR}*.ui")

QT5_WRAP_UI(LAUNCHER_FORMS_HEADERS ${LAUNCHER_FORMS})

ADD_EXECUTABLE(Launcher WIN32 ${LAUNCHER_SOURCES} ${LAUNCHER_HEADERS} ${LAUNCHER_FORMS_HEADERS})

qt5_use_modules(Launcher Core)
qt5_use_modules(Launcher Widgets)

FILE(GLOB_RECURSE UNPACKER_SOURCES "${UNPACKER_DIR}*.cpp")
ADD_EXECUTABLE(Unpacker ${UNPACKER_SOURCES})
qt5_use_modules(Unpacker Core)

IF(WIN32)
    TARGET_LINK_LIBRARIES(Launcher Qt5::WinMain)
ENDIF()

IF(WIN32)
    TARGET_LINK_LIBRARIES(KVEngine $ENV{PNG_LIBRARY})
    TARGET_LINK_LIBRARIES(KVEngine $ENV{SFML_LIBRARY})
    TARGET_LINK_LIBRARIES(KVEngine $ENV{SFML_AUDIO_LIBRARY})

    TARGET_LINK_LIBRARIES(KVEngine opengl32)

ELSE()
    TARGET_LINK_LIBRARIES(KVEngine png)
    TARGET_LINK_LIBRARIES(KVEngine sfml-system)
    TARGET_LINK_LIBRARIES(KVEngine sfml-audio)

    TARGET_LINK_LIBRARIES(KVEngine GL)

ENDIF()

IF(WIN32)
    SET(EXECUTABLE_SUFFIX ".exe")
ELSE()
    SET(EXECUTABLE_SUFFIX "")
ENDIF()

ADD_CUSTOM_COMMAND(
    TARGET KVEngine
    COMMAND
        ${PYTHON_CMD}
        ${CMAKE_CURRENT_SOURCE_DIR}/copy_file.py
        KVEngine${EXECUTABLE_SUFFIX}
        ${CMAKE_CURRENT_SOURCE_DIR}/${EXEC_DIR}/KVEngine${EXECUTABLE_SUFFIX}
)
ADD_CUSTOM_COMMAND(
    TARGET KVEngine
    COMMAND
        ${PYTHON_CMD}
        ${CMAKE_CURRENT_SOURCE_DIR}/copy_file.py
        metadata.json
        ${CMAKE_CURRENT_SOURCE_DIR}/${EXEC_DIR}/metadata.json
)
ADD_CUSTOM_COMMAND(
    TARGET Launcher
    COMMAND
        ${PYTHON_CMD}
        ${CMAKE_CURRENT_SOURCE_DIR}/copy_file.py
        Launcher${EXECUTABLE_SUFFIX}
        ${CMAKE_CURRENT_SOURCE_DIR}/${EXEC_DIR}/Launcher${EXECUTABLE_SUFFIX}
)

