#pragma once

#ifdef FARTASK_LIBRARY_EXPORTS
#define FARTASK_LIBRARY_API __declspec(dllexport)
#else
#define FARTASK_LIBRARY_API __declspec(dllimport)
#endif

// reference:
// https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp

namespace far { namespace task { extern "C"
{
    // the exported APIs


}}}
