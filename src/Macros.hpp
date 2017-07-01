#ifndef ROSA_MACROS_HPP_
#define ROSA_MACROS_HPP_

#include <execinfo.h>
#include <unistd.h>

#include <Rcpp.h>

#define ROSA_DEBUG

#define DISALLOW_COPY_AND_ASSIGN(classname) \
  classname(const classname &orig) = delete;\
  classname & operator=(const classname &rhs) = delete

#if defined(__GNUC__) && !(defined(__clang__) || defined(__INTEL_COMPILER))
#define ROSA_UNREACHABLE()  \
  std::exit(1)
#else
#define ROSA_UNREACHABLE()  \
  (void)0
#endif

#define FATAL_ERROR(message) do { \
  Rcpp::Rcout << "FATAL ERROR: " << message \
              << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
  void *array[64]; \
  std::size_t size = backtrace(array, 64); \
  backtrace_symbols_fd(array, size, STDERR_FILENO); \
  std::exit(1); \
} while(false)

#define LOG_WARNING(message) \
  Rcpp::Rcout << "WARNING: " << message << "\n";

#ifdef ROSA_DEBUG
#define DCHECK(condition) { \
  if (!(condition)) { \
    FATAL_ERROR("Condition " #condition " failed"); \
  } \
}
#else
#define DCHECK(condition)
#endif

#endif  // ROSA_MACROS_HPP_
