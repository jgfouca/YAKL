
#pragma once
// Included by YAKL.h

namespace yakl {

  // Allows the user to throw an exception from the host or the device
  YAKL_INLINE void yakl_throw(const char * msg) {
    // If we're on the host, then let's throw a real exception
    #if YAKL_CURRENTLY_ON_HOST()
      fence();
      std::cerr << "YAKL FATAL ERROR:\n";
      std::cerr << msg << std::endl;
      throw msg;
    // Otherwise, we need to be more careful with printf and intentionally segfaulting to stop the program
    #else
      #ifdef YAKL_ARCH_SYCL
        // SYCL cannot printf like the other backends quite yet
        const CL_CONSTANT char format[] = "KERNEL CHECK FAILED:\n   %s\n";
        sycl::ext::oneapi::experimental::printf(format,msg);
      #else
        printf("%s\n",msg);
      #endif
      // Intentionally cause a segfault to kill the run
      int *segfault = nullptr;
      *segfault = 10;
    #endif
  }


  // Check if any errors have been thrown by the runtimes
  inline void check_last_error() {
    #ifdef YAKL_DEBUG
      fence();
      #ifdef YAKL_ARCH_CUDA
        auto ierr = cudaGetLastError();
        if (ierr != cudaSuccess) { yakl_throw( cudaGetErrorString( ierr ) ); }
      #elif defined(YAKL_ARCH_HIP)
        auto ierr = hipGetLastError();
        if (ierr != hipSuccess) { yakl_throw( hipGetErrorString( ierr ) ); }
      #elif defined(YAKL_ARCH_SYCL)
      #endif
    #endif
  }


  // Determine if this is the main process in the case of multiple MPI tasks
  // This is nearly always used just to avoid printing to stdout or stderr from all MPI tasks
  inline bool yakl_mainproc() {
    // Only actually check if the user says MPI is available. Otherwise, always return true
    #ifdef HAVE_MPI
      int is_initialized;
      MPI_Initialized(&is_initialized);
      if (!is_initialized) {
        return true;
      } else {
        int myrank;
        MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
        return myrank == 0;
      }
    #else
      return true;
    #endif
  }

}

