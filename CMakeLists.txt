cmake_minimum_required(VERSION 3.11)

project(stocky C)
set(CMAKE_C_STANDARD 99)

add_executable(stocky
    main.c
    stocks.c
    font.c
    image.c
    log.c
    deps/cJSON/cJSON.c)

target_compile_options(stocky PRIVATE
	-Wall -Wextra)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/SDL_src)
set(FETCHCONTENT_QUIET FALSE)

include(FindPkgConfig)
set(PKGCONFIG_LIBRARIES sdl3 sdl3-ttf)

set(GIT_TAGS
	6cc9ce183d904489bf8e33e26b91d6012667e1b0  # SDL
	0ad0c2518be948300ed1e133741932f1ba2a4e7b) # SDL_ttf

set(GIT_REPOS  SDL SDL_ttf)
set(GIT_HASHES
	f164f590589a09d92922dbcd504daaa6
	01f465502c14817f39aad2fc5f2a84f7
)

set(LNK_NAMES SDL3::SDL3 SDL3_ttf::SDL3_ttf)

foreach(INDEX RANGE 1)
	list(GET PKGCONFIG_LIBRARIES ${INDEX} LIBRARY)
	list(GET GIT_REPOS  ${INDEX} REPO)
	list(GET GIT_HASHES ${INDEX} HASH)
	list(GET GIT_TAGS   ${INDEX} TAG)
	list(GET LNK_NAMES  ${INDEX} LINK)

    pkg_check_modules(PKG_${LIBRARY} QUIET IMPORTED_TARGET ${LIBRARY})
    if (NOT PKG_${LIBRARY}_FOUND)
        message(WARNING
        	"${LIBRARY} not found via pkg-config, trying to fetch manually...")

        FetchContent_Declare(
            "av_${LIBRARY}"
            URL https://github.com/libsdl-org/${REPO}/archive/${TAG}.zip
            URL_HASH MD5=${HASH}
		)
        FetchContent_MakeAvailable("av_${LIBRARY}")

     	target_link_libraries(stocky PUBLIC ${LINK})
    else()
    	target_link_libraries(stocky PUBLIC PkgConfig::PKG_${LIBRARY})
    endif()
endforeach()

target_link_libraries(stocky PUBLIC m)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/stocks_request.py
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
