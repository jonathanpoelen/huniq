/*
SPDX-FileCopyrightText: 2021 Jonathan Poelen <jonathan.poelen@gmail.com>
SPDX-License-Identifier: MIT
*/

#pragma once

#include <unordered_set>
#include <string_view>
#include <memory>
#include <new>

#include <cstring>
#include <cerrno>
#include <cstdio>


struct File;

struct FileRef
{
    bool is_open() const noexcept
    {
        return m_file;
    }

    std::size_t read(char* buffer, std::size_t n)
    {
        return std::fread(buffer, 1, n, m_file);
    }

    std::size_t write(char const* buffer, std::size_t n)
    {
        return std::fwrite(buffer, 1, n, m_file);
    }

    std::size_t write(std::string_view s)
    {
        return write(s.data(), s.size());
    }

    bool flush()
    {
        return fflush(m_file) == 0;
    }

    bool is_eof() const
    {
        return feof(m_file);
    }

    void print_error(char const* progname)
    {
        std::string_view err = strerror(errno);
        std::string_view prog = progname;
        std::string_view sep = ": ";
        std::string_view filename = m_filename;
        std::string_view end = "\n";
        FileRef f{stderr, ""};
        for (auto s : {prog, sep, filename, err, end}) {
            f.write(s);
        }
    }

    explicit FileRef(FILE* f, char const* filename)
    : m_file(f)
    , m_filename(filename)
    {}

private:
    friend File;

    FILE* m_file;

    char const* m_filename;
};

struct File
{
    explicit File(char const* filename)
    : File{fopen(filename, "r"), filename}
    {}

    explicit File(FILE* file, char const* filename)
    : m_fileRef{file, filename}
    {}

    File(File const&) = delete;
    File& operator = (File const&) = delete;

    ~File()
    {
        std::fclose(m_fileRef.m_file);
    }

    static FileRef from_stdin()
    {
        return FileRef{stdin, "STDIN"};
    }

    static FileRef from_stdout()
    {
        return FileRef{stdout, "STDOUT"};
    }

    FileRef ref()
    {
        return m_fileRef;
    }

private:
    FileRef m_fileRef;
};

struct StringSet
{
    bool add(std::string_view str)
    {
        auto pair = set.insert(String(str));
        if (!pair.second) {
            return false;
        }
        pair.first->str = strings.push(str);
        return true;
    }

private:
    struct Strings
    {
        std::string_view push(std::string_view str)
        {
            void* p = operator new(sizeof(Node) + str.size());
            char* s = new(static_cast<char*>(p) + sizeof(Node)) char[str.size()];
            node = new (p) Node{node};
            memcpy(s, str.data(), str.size());
            return {s, str.size()};
        }

        ~Strings()
        {
            while (node) {
                auto* next = node->next;
                operator delete(node);
                node = next;
            }
        }

    private:
        // intrusive string list
        // layout: {Node, char[N]}
        struct Node
        {
            Node* next;
        };

        Node* node = nullptr;
    };

    struct String
    {
        String(std::string_view str) noexcept
        : str(str)
        , hash(std::hash<std::string_view>()(str))
        {}

        bool operator == (String const& other) const noexcept
        {
            return str == other.str;
        }

        mutable std::string_view str;
        std::size_t hash;
    };

    struct Hash
    {
        std::size_t operator()(String const& str) const noexcept
        {
            return str.hash;
        }
    };

    Strings strings;
    std::unordered_set<String, Hash> set;
};

struct BufferView
{
    char* data;
    std::size_t len;
};

enum class FileHandleError
{
    NoError,
    Input,
    Output,
};

inline FileHandleError huniq(FileRef input, FileRef output, BufferView buffer, StringSet& set)
{
    char* p = buffer.data;
    char* first = p;

    for (;;) {
        auto len = input.read(p, buffer.len - (p - buffer.data + 1));
        if (len) {
            char* last = p + len;
            *last = '\n';
            for (;;) {
                while (*p != '\n') {
                    ++p;
                }

                std::string_view str(first, std::size_t(p - first));
                if (p != last) {
                    if (set.add(str)) {
                        if (!output.write(str.data(), str.size() + 1)) {
                            return FileHandleError::Output;
                        }
                    }
                }
                else {
                    if (buffer.data != first) {
                        memmove(buffer.data, str.data(), str.size());
                        p = buffer.data + str.size();
                        first = buffer.data;
                    }
                    // line too long
                    else {
                        if (!output.write(str)) {
                            return FileHandleError::Output;
                        }
                        p = buffer.data;
                        first = buffer.data;
                    }
                    break;
                }
                ++p;
                first = p;
            }

            if (!output.flush()) {
                return FileHandleError::Output;
            }
        }
        else {
            return input.is_eof() ? FileHandleError::NoError : FileHandleError::Input;
        }
    }
}
