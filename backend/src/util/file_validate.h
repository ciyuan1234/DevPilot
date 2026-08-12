#pragma once

#include <string>

namespace devpilot::util {

// 校验上传的源文件名是否安全、合法。
// 规则：
//   - 非空，长度不超过 255
//   - 不含 '/' 或 '\'（禁止目录穿越）
//   - 不是 "." 或 ".."
//   - 扩展名在源代码文件白名单内（不区分大小写）
// 返回空字符串表示合法；否则返回错误原因。
std::string validate_source_file_name(const std::string& name);

}
