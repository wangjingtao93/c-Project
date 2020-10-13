cmake_minimum_required(VERSION 3.5)
project(wjtproject)

set(CMAKE_CXX_STANDARD 14)

execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/version.sh
        OUTPUT_VARIABLE MY_VER
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_subdirectory(c++project)


# 设置安装目录，设置安装路径, 默认/usr/local。也可以是idc经常安装的/data/home/user00/xxxserver等
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output)

message("install BLK to:${CMAKE_SOURCE_DIR}/output")
install(DIRECTORY DESTINATION  bin)
install(DIRECTORY DESTINATION  lib64)

# 将文件可执行文件 bin/下， PROGRAMS意思是赋予可执行权限
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/c++project/func_moudle/test
        DESTINATION bin/)


# 制作rpm包
# After running cmake, run make package to generate the rpm package.
execute_process(COMMAND lsb_release -is
  OUTPUT_VARIABLE LSB_DISTRIBUTOR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND lsb_release -rs
  OUTPUT_VARIABLE LSB_RELEASE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CPACK_PACKAGE_NAME "wjtproject")
set(CPACK_RPM_PACKAGE_SOURCES "OFF")
set(CPACK_PACKAGE_VERSION ${MY_VER})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "PingAn Cloud")
set(CPACK_PACKAGE_VENDOR "PAIC")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")# 这样如果安装时不加--prefix= 就会默认安装在/usr/local,如果加上--prefix=/usr下，则会安装在/usr/bin/下
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${LSB_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/local /usr/local/bin /usr/local/lib64)# 如果安装时不加上--nodeps,则会包找不到一些动态库，尤其是在/usr/local/lib64下的

include(CPack)