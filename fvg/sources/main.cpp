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

    bool src_dir{is_directory(srcpath)};
    bool dst_dir{is_directory(dstpath)};

    if (!src_dir) {
        if (dst_dir) {
            throw std::runtime_error("output type (file/directory) mismatch");
        }

        fvg::write_svg(fvg::make_state(fvg::slurp_json(srcpath)), std::move(dstpath));
    } else {
        if (!exists(dstpath)) {
            create_directory(dstpath);
        } else if (!is_directory(dstpath)) {
            throw std::runtime_error("output type (file/directory) mismatch");
        }

        for (const auto& src : std::filesystem::directory_iterator{srcpath}) {
            if (!is_regular_file(src)) continue;
            const auto& srcpath{src.path()};
            auto dst{(dstpath / srcpath.stem()).replace_extension("svg")};
            fvg::write_svg(fvg::make_state(fvg::slurp_json(srcpath)), std::move(dst));
        }
    }

    return EXIT_SUCCESS;
} catch (const std::exception& error) {
    std::cerr << "Fatal exception: " << error.what() << '\n';
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Fatal exception: unknown\n";
    return EXIT_FAILURE;
}

/**************************************************************************************************/
