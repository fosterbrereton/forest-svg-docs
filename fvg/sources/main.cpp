/**************************************************************************************************/

// stdc++
#include <iostream>

// application
#include "write.hpp"

/**************************************************************************************************/

int main(int argc, const char * argv[]) try {
    if (argc != 3) {
        throw std::runtime_error(std::string("Usage: ") + argv[0] + " input output");
    }

    std::filesystem::path srcpath{argv[1]};
    std::filesystem::path dstpath{argv[2]};

    if (!exists(srcpath)) {
        throw std::runtime_error(srcpath.string() + " does not exist");
    }

    fvg::write_svg(fvg::make_state(fvg::slurp_json(srcpath)), std::move(dstpath));

    return EXIT_SUCCESS;
} catch (const std::exception& error) {
    std::cerr << "Fatal exception: " << error.what() << '\n';
} catch (...) {
    std::cerr << "Fatal exception: unknown\n";
}

/**************************************************************************************************/
