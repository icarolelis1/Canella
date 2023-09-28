# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Tools/glTF-IBL-Sampler/thirdparty/KTX-Software"
  "D:/Tools/glTF-IBL-Sampler/build/Ktx/src/Ktx-build"
  "D:/Tools/glTF-IBL-Sampler/build/Ktx"
  "D:/Tools/glTF-IBL-Sampler/build/Ktx/tmp"
  "D:/Tools/glTF-IBL-Sampler/build/Ktx/src/Ktx-stamp"
  "D:/Tools/glTF-IBL-Sampler/build/Ktx/src"
  "D:/Tools/glTF-IBL-Sampler/build/Ktx/src/Ktx-stamp"
)

set(configSubDirs Debug;Release;MinSizeRel;RelWithDebInfo)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Tools/glTF-IBL-Sampler/build/Ktx/src/Ktx-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Tools/glTF-IBL-Sampler/build/Ktx/src/Ktx-stamp${cfgdir}") # cfgdir has leading slash
endif()
