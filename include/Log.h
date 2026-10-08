/*
 * Log.h
 *
 *  Created on: Jan 14, 2014
 *      Author: sigi
 */

#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <exception>

#define LOG(x) std::cout << "LOG: " << x << std::endl
#define CUDACHECK(x)             \
{                                \
   cudaError_t err = x;          \
   if (err != cudaSuccess) {     \
      LOG("CUDA ERROR at " << __FILE__ << ":" << __LINE__ << " '" << cudaGetErrorString(err) << "' (" << err << ")"); \
      throw std::exception();    \
   }                             \
}

#define CUDAFREE_AND_CHECK(x)    \
{                                \
   cudaError_t err = cudaSuccess;\
   if (x) err = cudaFree(x);     \
   if (err != cudaSuccess) {     \
      LOG("CUDA ERROR at " << __FILE__ << ":" << __LINE__ << " '" << cudaGetErrorString(err) << "' (" << err << ")"); \
      throw std::exception();    \
   }                             \
   x = 0;                        \
}

#ifdef TRACE
#define LOGTRACE(x) LOG(x)
#else
#define LOGTRACE(x)
#endif

#ifdef IN_KERNEL_PRINTF
#define cudaPrintf(...) printf(__VA_ARGS__);
#else
#define cudaPrintf(...)
#endif

#endif /* LOG_H_ */
