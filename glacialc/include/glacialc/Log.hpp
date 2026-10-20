//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <format>
#include <iostream>

namespace glacialc {
    template<typename... Args>
    void GSH_MESSAGE(std::format_string<Args...> fmt, Args&&... args) { std::cout << "glacialc: " << std::format(fmt, std::forward<Args>(args)...) << "\n"; }

    template<typename... Args>
    void GSH_ERROR(std::format_string<Args...> fmt, Args&&... args) { std::cout << "glacialc error: " << std::format(fmt, std::forward<Args>(args)...) << "\n"; }

    template<typename... Args>
    void GSH_WARNING(std::format_string<Args...> fmt, Args&&... args) { std::cout << "glacialc warning: " << std::format(fmt, std::forward<Args>(args)...) << "\n"; }
}
