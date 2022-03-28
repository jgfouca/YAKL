
#pragma once
// Included by YAKL.h

namespace yakl {
  // This exists to create a queue for SYCL kernels and data transfers
  #ifdef YAKL_ARCH_SYCL
    // Triggered when asynchronous exceptions arise from the SYCL runtime
    auto asyncHandler = [](sycl::exception_list exceptions) {
      for (std::exception_ptr const &e : exceptions) {
        try {
          std::rethrow_exception(e);
        } catch (sycl::exception const &e) {
          std::cerr << "Caught asynchronous SYCL exception:" << std::endl
          << e.what() << std::endl
          << "Exception caught at file:" << __FILE__
          << ", line:" << __LINE__ << std::endl;
        }
      }
    };

    // Singleton pattern to initialize a SYCL queue upon first instantiation
    class dev_mgr {
    public:
      sycl::queue &default_queue() {
        check_id(DEFAULT_DEVICE_ID);
        return *(_queues[DEFAULT_DEVICE_ID].get());
      }
      unsigned int device_count() { return _devs.size(); }

      /// Returns the instance of device manager singleton.
      static dev_mgr &instance() {
        static dev_mgr d_m;
        return d_m;
      }
      dev_mgr(const dev_mgr &) = delete;
      dev_mgr &operator=(const dev_mgr &) = delete;
      dev_mgr(dev_mgr &&) = delete;
      dev_mgr &operator=(dev_mgr &&) = delete;

    private:
      dev_mgr() {
        auto gpu_devs = sycl::device::get_devices(sycl::info::device_type::gpu);
        for (auto &dev : gpu_devs) {
          if (dev.get_info<sycl::info::device::partition_max_sub_devices>() > 0) {
            auto subDevs = dev.create_sub_devices<sycl::info::partition_property::partition_by_affinity_domain>(sycl::info::partition_affinity_domain::numa);
            for (auto &sub_dev : subDevs) {
              _devs.push_back(std::make_shared<sycl::device>(sub_dev));
              _queues.push_back(std::make_shared<sycl::queue>(sub_dev,
                                                              asyncHandler,
                                                              sycl::property_list{sycl::property::queue::in_order{}}));
            }
          } else {
            _devs.push_back(std::make_shared<sycl::device>(dev));
            _queues.push_back(std::make_shared<sycl::queue>(dev,
                                                            asyncHandler,
                                                            sycl::property_list{sycl::property::queue::in_order{}}));
          }
        }

#ifdef HAVE_MPI
        int is_initialized;
        MPI_Initialized(&is_initialized);
        if (is_initialized) {
          MPI_Comm shmcomm;
          MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0,
                              MPI_INFO_NULL, &shmcomm);
          int shmrank;
          MPI_Comm_rank(shmcomm, &shmrank);
          DEFAULT_DEVICE_ID = (shmrank % _devs.size());
        }
#endif // HAVE_MPI
      }
      void check_id(unsigned int id) const {
        if (id >= _devs.size()) {
          std::cerr << "ERROR: invalid SYCL device id \n";
        }
      }
      std::vector<std::shared_ptr<sycl::device>> _devs;
      std::vector<std::shared_ptr<sycl::queue>> _queues;

      const unsigned int DEFAULT_DEVICE_ID = 0;
    };

    static inline sycl::queue &sycl_default_stream() {
      return dev_mgr::instance().default_queue();
    }
  #endif // YAKL_ARCH_SYCL
}
