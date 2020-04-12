# Copyright (c) 2020 Miku AuahDark
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.1)

###############
# Some checks #
###############

# Prevent in-tree build.
if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build!")
endif()

# Check FFmpeg includes
if (NOT LIBAV_INCLUDE_DIR)
	message(FATAL_ERROR "Missing libav/FFmpeg include directories")
endif()

# Check FFmpeg libs
if (NOT LIBAV_LIB_DIR)
	if (NOT AVCODEC_LIB)
		message(FATAL_ERROR "Missing avcodec.lib")
	endif()

	if (NOT AVFORMAT_LIB)
		message(FATAL_ERROR "Missing avformat.lib")
	endif()

	if (NOT AVUTIL_LIB)
		message(FATAL_ERROR "Missing avutil.lib")
	endif()

	if (NOT SWRESAMPLE_LIB)
		message(FATAL_ERROR "Missing swresample.lib")
	endif()
else()
	set(AVCODEC_LIB "${LIBAV_LIB_DIR}/avcodec.lib")
	set(AVFORMAT_LIB "${LIBAV_LIB_DIR}/avformat.lib")
	set(AVUTIL_LIB "${LIBAV_LIB_DIR}/avutil.lib")
	set(SWRESAMPLE_LIB "${LIBAV_LIB_DIR}/swresample.lib")
endif()

###########
# Project #
###########

project(SAFFTracks LANGUAGES CXX)

# MSVC only
if (NOT MSVC)
	message(FATAL_ERROR "Currently only MSVC is supported")
endif()

# 32-bit only
if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "Make sure to target 32-bit")
endif()

add_library(SAFFTracks MODULE
	src/CAEDataStream.cpp
	src/CAELAVDecoder.cpp
	src/CAEStreamingDecoder.cpp
	src/CUtrax.cpp
	src/Main.cpp
	src/SADefaultAllocator.cpp
)
target_include_directories(SAFFTracks PRIVATE ${LIBAV_INCLUDE_DIR})
target_link_libraries(SAFFTracks ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWRESAMPLE_LIB})
set_target_properties(SAFFTracks PROPERTIES PREFIX "" OUTPUT_NAME "SAFFTracks" SUFFIX ".asi")
install(TARGETS SAFFTracks DESTINATION .)
