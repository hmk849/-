cmake_minimum_required(VERSION 3.21)
project(PixelPFS LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build SDL2 from source as part of this repository's configure step.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.11
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2)

add_executable(PixelPFS MACOSX_BUNDLE main.cpp)
target_link_libraries(PixelPFS PRIVATE SDL2-static)
target_include_directories(PixelPFS PRIVATE ${SDL2_SOURCE_DIR}/include)

set_target_properties(PixelPFS PROPERTIES
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.pixelpfs"
  MACOSX_BUNDLE_BUNDLE_NAME "PIXEL PFS"
  MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
  MACOSX_BUNDLE_BUNDLE_VERSION "1"
  XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
  XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.example.pixelpfs"
  XCODE_ATTRIBUTE_INFOPLIST_KEY_CFBundleDisplayName "PIXEL PFS"
  XCODE_ATTRIBUTE_INFOPLIST_KEY_UILaunchScreen_Generation "YES"
  XCODE_ATTRIBUTE_INFOPLIST_KEY_UIRequiresFullScreen "YES"
  XCODE_ATTRIBUTE_INFOPLIST_KEY_UISupportedInterfaceOrientations "UIInterfaceOrientationLandscapeLeft;UIInterfaceOrientationLandscapeRight"
)
