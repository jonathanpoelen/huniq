#include "huniq.hpp"

int main(int ac, char** av)
{
    static constexpr std::size_t bufflen = 128*1024;
    std::unique_ptr<char[]> data{new char[bufflen]};
    BufferView buffer{data.get(), bufflen};
    StringSet set;

    auto output = File::from_stdout();

    if (ac == 1) {
        huniq(File::from_stdin(), output, buffer, set);
        return 0;
    }

    int i = 1;

    if (av[1][0] == '-' && (av[1][1] == '-' || av[1][1] == 'h') && av[1][2] == '\0') {
        if (av[1][1] == 'h') {
            output.write("Usage: ");
            output.write(av[0]);
            output.write(" [FILE]...\nFilter lines present multiple times.\n");
            return 0;
        }
        ++i;
    }

    int r = 0;
    for (; i < ac; ++i) {
        File file(av[i]);
        auto input = file.ref();
        if (input.is_open()) {
            auto err = huniq(input, output, buffer, set);
            switch (err) {
                case FileHandleError::NoError:
                    continue;
                case FileHandleError::Output:
                    return r;
                case FileHandleError::Input:
                    break;
            }
        }
        input.print_error(av[0]);
        r = 1;
    }
    return r;
}
