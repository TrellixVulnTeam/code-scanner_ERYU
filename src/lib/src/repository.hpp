#pragma once

#include <fstream>
#include <set>
#include <unordered_map>

#include "serializer.hpp"

namespace code::analyzer {
template <class T, class Container = std::set<T>> class repository
{
  public:
    repository() { deserialize(); };
    ~repository() { serialize(); };
    repository(const repository &) = default;
    repository(repository &&)      = default;
    repository &operator=(const repository &) = default;
    repository &operator=(repository &&) = default;

    // emplace definitions in the repository database
    void emplace(const T &filename, const Container &definitions)
    {
        for (auto &e : definitions)
        {
            m_database[e].emplace(filename);
        }
    }

    // retrieve all filenames in which key is defined
    Container definitions(const T &key)
    {
        auto it = m_database.find(key);
        if (it != std::end(m_database))
        {
            return it->second;
        }

        // empty container
        return Container();
    }

    // serialize the database to a file
    void serialize() { m_serializer.serialize(m_database); }

    // deserialize the database from a file
    void deserialize() { m_serializer.deserialize(m_database); }

  private:
    // stores [usr string, set of filenames]
    std::unordered_map<T, Container> m_database{};
    serializer m_serializer{};
};
}