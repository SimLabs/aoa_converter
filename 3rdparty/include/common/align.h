#pragma once

#if defined(_MSC_VER)

#define ALIGN(x) __declspec(align(x))

#else

#define ALIGN(x) __attribute__((aligned(x)))

#endif
