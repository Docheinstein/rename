#include <algorithm>
#include <argp.h>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <regex>

namespace {
const char* error_prefix = "rename: ";

const char* argp_args_doc = "PATTERN REPLACEMENT [PATH]";
const char* argp_doc = "\n"
                       "Rename files by replacing PATTERN with REPLACEMENT.\n"
                       "Regular expressions are supported as well.\n"
                       "\n"
                       "Examples:\n"
                       "  rename txt txt.old .\n"
                       "  rename file_(\\d+).txt File_$1.txt /path/to/directory\n";

const char* argp_program_version = "1.0";

constexpr char OPTION_DRY_RUN = 'n';
constexpr char OPTION_RECURSIVE = 'r';
constexpr char OPTION_QUIET = 'q';

argp_option argp_options[] = {{"dry-run", OPTION_DRY_RUN, nullptr, 0, "Perform a trial run with no changes"},
                              {"recursive", OPTION_RECURSIVE, nullptr, 0, "Rename recursively in subdirectories"},
                              {"quiet", OPTION_QUIET, nullptr, 0, "Suppress non-error messages"},
                              {nullptr}};

struct arguments {
    const char* pattern {};
    const char* replacement {};
    const char* path {};

    bool dry_run {};
    bool recursive {};
    bool quiet {};
};

error_t parse_option(int key, char* const arg, argp_state* const state) {
    auto* const args = static_cast<arguments*>(state->input);

    switch (key) {
    case OPTION_DRY_RUN:
        args->dry_run = true;
        break;
    case OPTION_RECURSIVE:
        args->recursive = true;
        break;
    case OPTION_QUIET:
        args->quiet = true;
        break;
    case ARGP_KEY_ARG:
        switch (state->arg_num) {
        case 0:
            args->pattern = arg;
            break;
        case 1:
            args->replacement = arg;
            break;
        case 2:
            args->path = arg;
            break;
        default:
            argp_usage(state);
        }
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 2) {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

void rename(const arguments& arguments) {
    std::regex pattern;
    try {
        pattern = arguments.pattern;
    } catch (const std::regex_error& e) {
        std::cerr << error_prefix << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    std::filesystem::path path;
    try {
        if (arguments.path) {
            path = arguments.path;
        } else {
            path = std::filesystem::current_path();
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << error_prefix << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    constexpr auto iterator_to_files_transform =
        std::ranges::views::filter([](const std::filesystem::directory_entry& e) {
            return std::filesystem::is_regular_file(e);
        }) |
        std::ranges::to<std::vector<std::filesystem::path>>();

    std::vector<std::filesystem::path> entries;
    try {
        if (arguments.recursive) {
            entries = std::filesystem::recursive_directory_iterator(path) | iterator_to_files_transform;
        } else {
            entries = std::filesystem::directory_iterator(path) | iterator_to_files_transform;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << error_prefix << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ranges::sort(entries);

    for (const auto& in_entry : entries) {
        std::string in_filename = in_entry.filename();
        std::string out_filename;
        try {
            std::regex_replace(std::back_inserter(out_filename), in_filename.begin(), in_filename.end(), pattern,
                               arguments.replacement);
        } catch (const std::regex_error& e) {
            std::cerr << error_prefix << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }

        if (in_filename != out_filename) {
            std::filesystem::path out_entry = in_entry;
            out_entry.replace_filename(out_filename);

            if (!arguments.quiet) {
                const auto path_display_name = [recursive = arguments.recursive](const std::filesystem::path& p) {
                    return recursive ? std::filesystem::absolute(p).string() : p.filename().string();
                };

                std::cout << path_display_name(in_entry) << " -> " << (arguments.dry_run ? "(dry run)" : "")
                          << path_display_name(out_entry) << std::endl;
            }

            if (!arguments.dry_run) {
                try {
                    std::filesystem::rename(in_entry, out_entry);
                } catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << error_prefix << e.what() << std::endl;
                }
            }
        }
    }
}
} // namespace

int main(int argc, char* argv[]) {
    const argp argp {argp_options, parse_option, argp_args_doc, argp_doc};
    arguments arguments;
    argp_parse(&argp, argc, argv, 0, nullptr, &arguments);
    rename(arguments);
    return 0;
}
