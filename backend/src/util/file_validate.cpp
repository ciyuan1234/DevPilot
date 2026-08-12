#include "util/file_validate.h"

#include <algorithm>
#include <cctype>

namespace devpilot::util {
namespace {

bool ends_with_ignore_case(const std::string& s, const std::string& suffix)
{
    if (s.size() < suffix.size())
    {
        return false;
    }
    const auto start = s.end() - static_cast<std::ptrdiff_t>(suffix.size());
    return std::equal(start, s.end(), suffix.begin(), [](char a, char b)
                      { return std::tolower(static_cast<unsigned char>(a)) ==
                               std::tolower(static_cast<unsigned char>(b)); });
}

} // namespace

std::string validate_source_file_name(const std::string& name)
{
    if (name.empty())
    {
        return "file name is empty";
    }
    if (name.size() > 255)
    {
        return "file name is too long (max 255)";
    }
    if (name == "." || name == "..")
    {
        return "file name must not be \".\" or \"..\"";
    }
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos)
    {
        return "file name must not contain path separators";
    }

    static const char* const kAllowedExtensions[] = {
        ".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hxx",
    };
    for (const auto* ext : kAllowedExtensions)
    {
        if (ends_with_ignore_case(name, ext))
        {
            return "";
        }
    }
    return "file extension is not in the allowed source file list";
}

} // namespace devpilot::util