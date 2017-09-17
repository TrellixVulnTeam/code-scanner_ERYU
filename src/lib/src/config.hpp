#pragma once

#include <string>
#include <vector>

struct config
{
    static std::vector<std::string> flags_to_ignore();
    static std::vector<std::string> compile_commands();
    static std::string              root_uri();
    static void builder(const std::string &             root_uri,
                        const std::vector<std::string> &compile_commands,
                        const std::vector<std::string> &flags_to_ignore);
};
