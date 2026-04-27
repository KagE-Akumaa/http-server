#include "FileReader.hpp"
#include "fstream"
#include <iostream>

std::vector<char> fileReader(std::filesystem::path &filePath) {

    // 1. Open the file in binary mode
    std::ifstream file(filePath, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open the file " << filePath << std::endl;

        return {};
    }

    // 2. Move to end to get the size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    // 3. Move back to beginning
    file.seekg(0, std::ios::beg);

    // 4. Allocate buffer
    std::vector<char> buffer(size);

    // 5. Read file into buffer
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read the file " << filePath << std::endl;
        return {};
    }

    return buffer;
}
