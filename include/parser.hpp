#pragma once

#include "ast.hpp"
#include <string>
#include <vector>
#include <array>
#include <nlohmann/json.hpp>

namespace solum {

class Parser {
public:
    Parser();
    
    Document parse(const std::string& input);
    nlohmann::json serialize(const Document& doc);

private:
    std::array<int, 3> headingCounts;
    std::vector<Node> parseLine(const std::string& line, size_t lineNum);
    std::vector<Node> parseInline(const std::string& text);
    std::string trim(const std::string& s);
};

} // namespace solum
