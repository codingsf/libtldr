#ifndef TLDR_TYPES_HPP_
#define TLDR_TYPES_HPP_

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#	include <unistd.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#endif

namespace tldr {

#if defined(_WIN32)
	typedef HMODULE module_handle_t;
#elif defined(_POSIX_VERSION)
	typedef void * module_handle_t;
#endif

}

#endif
