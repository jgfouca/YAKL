
#pragma once

namespace yakl {

  inline void yakl_throw(std::string exc) {
    std::cout << "YAKL FATAL ERROR:\n";
    std::cout << exc << std::endl;
    throw exc;
  }


  inline void check_last_error() {
    #ifdef __USE_CUDA__
      auto ierr = cudaGetLastError();
      if (ierr != cudaSuccess) { yakl_throw( cudaGetErrorString( ierr ) ); }
    #elif defined(__USE_HIP__)
      auto ierr = hipGetLastError();
      if (ierr != hipSuccess) { yakl_throw( hipGetErrorString( ierr ) ); }
    #elif defined(__USE_SYCL__)
    #elif defined(__USE_OPENMP45__)
      //auto ierr = GetLastError();
    #endif
  }

}


