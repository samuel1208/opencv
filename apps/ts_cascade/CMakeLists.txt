set(OPENCV_TRAINCASCADE_DEPS opencv_core opencv_ml opencv_imgproc opencv_photo opencv_objdetect opencv_highgui opencv_calib3d opencv_video opencv_features2d opencv_flann opencv_legacy opencv_nonfree)
ocv_check_dependencies(${OPENCV_TRAINCASCADE_DEPS})

if(NOT OCV_DEPENDENCIES_FOUND)
  return()
endif()

project(ts_cascade)

ocv_include_directories("${CMAKE_CURRENT_SOURCE_DIR}" "${OpenCV_SOURCE_DIR}/include/opencv" "/home/samuel/Project/project_self/opencv/modules/nonfree/include" "/home/samuel/Project/project_self/opencv/apps/ts_cascade/feature/src" )
ocv_include_modules(${OPENCV_TRAINCASCADE_DEPS})

include_directories(include)
include_directories(./feature/inc)
include_directories(./cascade/inc)



#generate feature testbed
set(fea_files  ./feature/src/CHOGFea.cpp
  ./feature/main.cpp)

set(the_target ts_fea)
add_executable(${the_target} ${fea_files})
target_link_libraries(${the_target} ${OPENCV_TRAINCASCADE_DEPS} opencv_haartraining_engine)

#generate weakerLearner testbed
set(fea_files  ./cascade/src/CLRWeakLearner.cpp
               ./cascade/test.cpp)

set(the_target ts_CLRWeakLearner)
add_executable(${the_target} ${fea_files})
target_link_libraries(${the_target} ${OPENCV_TRAINCASCADE_DEPS} opencv_haartraining_engine)


#set_target_properties(${the_target} PROPERTIES
 #                     DEBUG_POSTFIX "${OPENCV_DEBUG_POSTFIX}"
  #                    ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}
   #                   RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    #                  INSTALL_NAME_DIR lib
     #                 OUTPUT_NAME "opencv_traincascade")

#if(ENABLE_SOLUTION_FOLDERS)
 # set_target_properties(${the_target} PROPERTIES FOLDER "applications")
#endif()

#install(TARGETS ${the_target} RUNTIME DESTINATION bin COMPONENT main)

