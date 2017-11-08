#include "Parser_Impl.hpp"

#include <algorithm>

#include "compile_database_t.hpp"
#include "config.hpp"
#include "repository.hpp"
#include "task_system.hpp"
#include <iostream>

namespace code::analyzer {

Parser_Impl::Parser_Impl()  = default;
Parser_Impl::~Parser_Impl() = default;

std::experimental::optional<std::error_code>
Parser_Impl::initialize(const std::string &             build_uri,
                        const std::vector<std::string> &compile_commands,
                        const std::vector<std::string> &flags_to_ignore)
{
    auto ec = config::builder(build_uri, compile_commands, flags_to_ignore);
    std::string prefix;
    for (const auto &compile_command : compile_commands)
    {
        prefix += compile_command + " ";
    }
    m_compile_db = std::make_unique<compile_database_t>(
        build_uri, prefix, flags_to_ignore);
    if (ec)
    {
        return ec;
    }

    task_system task;

    {
        // auto all_filenames = compile_database_t::source_filenames();
        auto acc = m_compile_db->all_compile_commands();
        acc      = m_repository.check_file_timestamp(acc);

        // TODO call commands
        for (auto &cmd : acc)
        {
            task.async([cmd, this]() {
                auto usrs =
                    translation_unit_t(cmd, true).retrieve_all_identifier_usr();
                m_repository.emplace(cmd.m_file, usrs);
            });
        }
    }

    // serialize the repository
    m_repository.serialize();

    return std::experimental::nullopt;
}

Location Parser_Impl::definition(const TextDocumentPositionParams &params)
{
    auto cmds = m_compile_db->compile_commands2(params.textDocument.uri);

    // TODO handle all compile cmds
    m_tu.compile_cmd(cmds[0]);
    Location location = m_tu.definition(params.position);
    if (!location.is_valid())
    {
        auto usr = m_tu.usr(params.position);
        // search in repository
        auto defs = m_repository.definitions(usr);

        for (auto def : defs)
        {
            // TODO handle multiple compile commands
            location = translation_unit_t(cmds[0]).definition(usr);
            if (location.is_valid())
            {
                return location;
            }
        }
    }
    return location;
}

Location Parser_Impl::reference(const TextDocumentPositionParams &params)
{
    auto cmds = m_compile_db->compile_commands2(params.textDocument.uri);

    // TODO handle all compile cmds
    m_tu.compile_cmd(cmds[0]);
    Location location = m_tu.reference(params.position);
    return location;
}

} // namespace code::analyzer
