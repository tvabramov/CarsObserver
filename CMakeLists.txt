cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME CarsObserver)

project(${PROJECT_NAME} VERSION 1.0.0.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

#enable_testing()

#https://stackoverflow.com/questions/17844085/boost-log-with-cmake-causing-undefined-reference-error
add_definitions(-DBOOST_LOG_DYN_LINK)

find_package(OpenCV 4.1.0 REQUIRED)
find_package(Boost 1.65 COMPONENTS program_options log REQUIRED)
find_package(Threads)
find_package(nlohmann_json 3.2.0 REQUIRED)

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Should I include headers? - https://stackoverflow.com/questions/13703647/how-to-properly-add-include-directories-with-cmake
set(PROJECT_SRCS
	main.cpp
	capturer.cpp
	capturer.h
	capturerfactory.cpp
	capturerfactory.h
	recognizer.cpp
	recognizer.h
	recognizerfactory.cpp
	recognizerfactory.h
	tracker.cpp
	tracker.h
	launchparams.h
	utilities.cpp
	utilities.h
	trackerfactory.cpp
	trackerfactory.h
	crosscounter.cpp
	crosscounter.h
	crosscounterfactory.cpp
	crosscounterfactory.h
	recognizers/abstractrecognizer.h
	recognizers/cafferecognizer.cpp
	recognizers/cafferecognizer.h
	recognizers/mobilenetssdrecognizer.cpp
	recognizers/mobilenetssdrecognizer.h
	recognizers/facerecognizer.cpp
	recognizers/facerecognizer.h
	trackers/abstracttracker.h
	trackers/cvtracker.cpp
	trackers/cvtracker.h
	verifiers/abstractverifier.h
	verifiers/hungarian.cpp
	verifiers/hungarian.h
	verifiers/hunverifier.cpp
	verifiers/hunverifier.h
)

# For using "recognizers/abstractrecognizer.h" in includes
include_directories(.)
# Boost directories
include_directories( ${Boost_INCLUDE_DIR} )

add_executable(${PROJECT_NAME} ${PROJECT_SRCS})

target_link_libraries(${PROJECT_NAME}
	LINK_PRIVATE ${OpenCV_LIBS}
	LINK_PRIVATE ${Boost_LIBRARIES}
	LINK_PRIVATE ${CMAKE_THREAD_LIBS_INIT}
	LINK_PRIVATE nlohmann_json::nlohmann_json
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES
	models/MobileNetSSD_deploy.caffemodel
	models/MobileNetSSD_deploy.prototxt.txt
	models/res10_300x300_ssd_iter_140000.caffemodel
	models/res10_300x300_ssd_iter_140000_deploy.prototxt.txt
DESTINATION bin/models)

install(FILES config_example.json DESTINATION bin RENAME config.json)
