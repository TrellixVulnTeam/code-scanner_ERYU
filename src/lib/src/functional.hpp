#pragma once

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <string>

namespace code {
namespace analyzer {

// Retrieve the declaration of a cursor
CXCursor reference(const CXCursor &cursor);

// Retrieve the definition
CXCursor definition(const CXCursor &cursor);

// TODO Retrieve the declaration of a cursor
CXCursor declaration(const CXCursor &cursor);

// Retrieve a type of cursor
std::string type(const CXCursor &cursor);

// Retrieve the location as file, line, column
std::tuple<std::string, unsigned int, unsigned int>
location(const CXCursor &cursor);

bool is_identifier(CXCursor & cursor);

bool is_declaration_locate_in_other_file(CXCursor & cursor);

} // namespace analyzer
} // namespace code
