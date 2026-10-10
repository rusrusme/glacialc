//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <format>
#include <iostream>

namespace bshc {
    template<typename... Args>
    void BSH_MESSAGE(std::format_string<Args...> fmt, Args&&... args) { std::cout << "BSHC: " << std::format(fmt, std::forward<Args>(args)...) << "\n"; }

    template<typename... Args>
    void BSH_ERROR(std::format_string<Args...> fmt, Args&&... args) { std::cout << "BSHC Error: " << std::format(fmt, std::forward<Args>(args)...) << "\n"; }

    template<typename... Args>
    void BSH_WARNING(std::format_string<Args...> fmt, Args&&... args) { std::cout << "BSHC Warning: " << std::format(fmt, std::forward<Args>(args)...) << "\n"; }
}
