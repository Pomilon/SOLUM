#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "parser.hpp"
#include "renderer.hpp"

void printUsage() {
    std::cout << "SOLUM CLI Tool - High Performance Unambiguous Markup\n\n";
    std::cout << "Usage: solum_cli [options] <input_file.sm> [output_file]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help           Show this help message\n";
    std::cout << "  -f, --format <fmt>   Output format: html (default), text, json\n";
    std::cout << "  -v, --version        Show version information\n\n";
    std::cout << "If output_file is omitted, the result is printed to stdout.\n";
}

int main(int argc, char* argv[]) {
    std::string inputPath;
    std::string outputPath;
    std::string format = "html";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "SOLUM v" << libsolum_VERSION << "\n";
            return 0;
        }
 else if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) {
                format = argv[++i];
            } else {
                std::cerr << "Error: --format requires an argument\n";
                return 1;
            }
        } else if (inputPath.empty()) {
            inputPath = arg;
        } else if (outputPath.empty()) {
            outputPath = arg;
        }
    }

    if (inputPath.empty()) {
        printUsage();
        return 1;
    }

    std::ifstream solumFile(inputPath);
    if (!solumFile.is_open()) {
        std::cerr << "Error: Could not open file " << inputPath << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << solumFile.rdbuf();
    std::string input = buffer.str();
    solumFile.close();

    solum::Parser parser;
    auto ast = parser.parse(input);
    
    solum::RenderConfig config;
    config.resolveAsset = [](const std::string& p) { return p; };

    std::string output;
    if (format == "html") {
        std::string content = solum::SolumRenderer::render(ast, config);
        output = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>SOLUM Render</title>\n"
                 "<style>body { font-family: sans-serif; padding: 40px; line-height: 1.6; max-width: 800px; margin: auto; color: #333; }\n"
                 "h1, h2, h3 { color: #222; }\n"
                 "table { border-collapse: collapse; margin-bottom: 1em; width: 100%; }\n"
                 "th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }\n"
                 "th { background-color: #f8f8f8; }\n"
                 "pre { background: #f4f4f4; padding: 15px; border-radius: 5px; overflow-x: auto; border: 1px solid #ddd; }\n"
                 "code { background: #eee; padding: 2px 4px; border-radius: 3px; font-family: monospace; }\n"
                 "ul, ol { margin-bottom: 1em; }\n"
                 "li { margin-bottom: 0.5em; }</style>\n"
                 "</head>\n<body>\n" + content + "\n</body>\n</html>";
    } else if (format == "text") {
        solum::PlainTextRenderer renderer;
        output = renderer.render(ast, config);
    } else if (format == "json") {
        output = parser.serialize(ast).dump(4);
    } else {
        std::cerr << "Error: Unknown format " << format << std::endl;
        return 1;
    }

    if (!outputPath.empty()) {
        std::ofstream outFile(outputPath);
        if (outFile.is_open()) {
            outFile << output;
            outFile.close();
            std::cout << "Successfully rendered " << inputPath << " to " << outputPath << " (format: " << format << ")" << std::endl;
        } else {
            std::cerr << "Error: Could not write to " << outputPath << std::endl;
            return 1;
        }
    } else {
        std::cout << output << std::endl;
    }

    return 0;
}
