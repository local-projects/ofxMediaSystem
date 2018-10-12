//
//  MemoryConfig.h
//  Belron
//
//  Created by Michael Allison on 9/25/18.
//

#pragma once

#include "Allocator.hpp"

#if !defined(MS_ALLOW_ALLOCATION_MIDDLEWARE)
#define MS_ALLOW_ALLOCATION_MIDDLEWARE
#endif

namespace mediasystem {
    
    template<typename T>
    using Allocator = _Allocator<T>;
    
//    template<typename T>
//    using Allocator = std::allocator<T>;
    
}//end namespace mediasystem