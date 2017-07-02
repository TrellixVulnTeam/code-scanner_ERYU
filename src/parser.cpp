#include "parser.hpp"
#include "string_array.hpp"

#include <iostream>

namespace {
std::string to_string(const CXString &cx_str)
{
    std::string str = clang_getCString(cx_str);
    clang_disposeString(cx_str);
    return str;
}
} // anonymous namespace

namespace code {
namespace analyzer {

Parser::Parser(const std::string &filename)
    : m_filename{filename}
    , m_index{clang_createIndex(1, 1)}
{
    CXCompilationDatabase_Error c_error = CXCompilationDatabase_NoError;
    m_db = clang_CompilationDatabase_fromDirectory("/home/njeneza/workspace/cpp-parser/tests/data/build", &c_error);

    if(c_error == CXCompilationDatabase_CanNotLoadDatabase)
    {
        //TODO Handle errors in ctor
        //return;
    }

    const std::string _filename = "/home/njeneza/workspace/cpp-parser/tests/data/test_arg.cpp";
    CXCompileCommands compile_commands = clang_CompilationDatabase_getCompileCommands(m_db, _filename.c_str());
    unsigned size = clang_CompileCommands_getSize(compile_commands);
    if(size == 0)
    {
        // TODO better handle errors
        return;
    }

    CXCompileCommand compile_command = clang_CompileCommands_getCommand(compile_commands, 0);
    std::cout << to_string(clang_CompileCommand_getFilename(compile_command)) << std::endl;
    unsigned number_args = clang_CompileCommand_getNumArgs(compile_command);
    std::cout << "size " << size << std::endl;
    std::cout << "number_args " << number_args << std::endl;
    char ** args = new char*[number_args];
    for(unsigned i = 0; i< number_args; ++i)
    {
        //args[i] = const_cast<char*>(to_string(clang_CompileCommand_getArg(compile_command, i)).c_str());
        std::cout << to_string(clang_CompileCommand_getArg(compile_command, i)) << std::endl;
    }

    clang_CompileCommands_dispose(compile_commands);
    delete args;
    m_unit = clang_parseTranslationUnit(m_index,
                                        m_filename.c_str(),
                                        nullptr,
                                        0,
                                        nullptr,
                                        0,
                                        CXTranslationUnit_None);
}

Parser::Parser(const std::string &filename, const std::string &command_line_arg)
    : m_filename{filename}
    , m_index{clang_createIndex(1, 1)}
{
    string_array      argument(command_line_arg);
    auto              error = clang_parseTranslationUnit2(m_index,
                                             filename.c_str(),
                                             argument.data(),
                                             argument.size(),
                                             nullptr,
                                             0,
                                             CXTranslationUnit_None,
                                             &m_unit);

    if (error != 0)
    {
        return;
    }
}

Parser::~Parser()
{
    clang_disposeTranslationUnit(m_unit);
    clang_disposeIndex(m_index);
    clang_CompilationDatabase_dispose(m_db);
}

// Retrieve a cursor from a file/line/column
CXCursor Parser::cursor(const unsigned long &line, const unsigned long &column)
{
    CXFile           file     = clang_getFile(m_unit, m_filename.c_str());
    CXSourceLocation location = clang_getLocation(m_unit, file, line, column);
    return clang_getCursor(m_unit, location);
}

std::vector<CXCursor> Parser::callers(const CXCursor &cursor) const
{
    // get cursor declaration
    CXCursor              cursor_decl = declaration(cursor);
    std::vector<CXCursor> cursors;
    std::tuple<CXCursor *, std::vector<CXCursor> *> cursor_data = {&cursor_decl,
                                                                   &cursors};

    // get translation unit cursor
    CXCursor unit_cursor = clang_getTranslationUnitCursor(m_unit);

    // traverse the AST and check every cursor if it is equal to the
    // cursor declaration
    // ignore the cursor which point to himself
    CXClientData user_data = static_cast<CXClientData>(&cursor_data);
    clang_visitChildren(
        unit_cursor,
        // visitor
        [](CXCursor cursor, CXCursor, CXClientData client_data) {
            CXCursorKind cursor_kind = clang_getCursorKind(cursor);
            if (cursor_kind != CXCursor_CallExpr)
            {
                return CXChildVisit_Recurse;
            }
            // current cursor declaration
            CXCursor current_cursor_decl = declaration(cursor);
            using UserData = std::tuple<CXCursor *, std::vector<CXCursor> *>;
            UserData *             data = static_cast<UserData *>(client_data);
            CXCursor *             cursor_decl = std::get<0>(*data);
            std::vector<CXCursor> *cursors     = std::get<1>(*data);
            unsigned               equal =
                clang_equalCursors(current_cursor_decl, *cursor_decl);
            if (equal)
            {
                (*cursors).push_back(cursor);
            }
            // Continue to search more
            return CXChildVisit_Recurse;
        },
        user_data);

    return cursors;
}

std::string Parser::filename() const
{
    CXFile      file     = clang_getFile(m_unit, m_filename.c_str());
    std::string spelling = to_string(clang_getFileName(file));
    return spelling;
}
CXCursor declaration(const CXCursor &cursor)
{
    auto cur = clang_getCursorDefinition(cursor);
    return clang_getCanonicalCursor(cur);
}

CXCursor reference(const CXCursor &cursor)
{
    return clang_getCursorReferenced(cursor);
}

CXCursor definition(const CXCursor &cursor)
{
    return clang_getCursorDefinition(cursor);
}

// Retrieve a type of cursor
std::string type(const CXCursor &cursor)
{
    // cursor type
    CXType type = clang_getCursorType(cursor);
    // cursor type spelling
    std::string spelling = to_string(clang_getTypeSpelling(type));
    return spelling;
}

std::tuple<std::string, unsigned long, unsigned long>
location(const CXCursor &cursor)
{
    CXSourceLocation location = clang_getCursorLocation(cursor);

    CXFile   file;
    unsigned line;
    unsigned column;
    clang_getSpellingLocation(location, &file, &line, &column, nullptr);

    // filename
    std::string _filename = to_string(clang_getFileName(file));
    return std::make_tuple(_filename, line, column);
}

} // namespace analyzer
} // namespace code
