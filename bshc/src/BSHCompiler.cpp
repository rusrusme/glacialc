//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "bshc/BSHCompiler.hpp"

#include <algorithm>
#include <atomic>
#include <execution>
#include <fstream>

#include "bshc/BinaryPackager.hpp"
#include "bshc/Log.hpp"
#include "bshc/Preprocessor.hpp"
#include "bshc/Utility.hpp"

namespace bshc {
    std::vector<uint8_t> BSHCompiler::compile_source(std::string_view source, RenderBackend target_backend, const std::filesystem::path& include_dir) {
        const std::filesystem::path base_path = include_dir.empty() ? std::filesystem::current_path() : include_dir;
        Preprocessor preprocessor(base_path);
        ParsedData parsed = preprocessor.process(std::string(source), base_path / "Shader.bsh");

        if (!parsed.success) {
            BSH_ERROR("BSHCompiler: Preprocessing Error: {}", parsed.error_message);
            return {};
        }

        parsed.header.backend = target_backend;

        const Unique<Compiler> compiler = Compiler::create(target_backend);
        if (!compiler) {
            BSH_ERROR("BSHCompiler: Unsupported render backend: {}", to_string(target_backend));
            return {};
        }

        const CompilerOutput compiled = compiler->compile(parsed);
        if (!compiled.success) {
            BSH_ERROR("BSHCompiler: Compilation Error: {}", compiled.error_message);
            return {};
        }
        return BinaryPackager::package_binary(parsed.header, compiled.stages);
    }

    bool BSHCompiler::compile_file(const std::filesystem::path& input_path, const std::filesystem::path& output_path, RenderBackend target_backend) {
        if (!std::filesystem::exists(input_path)) {
            BSH_ERROR("Input file does not exist: {}", input_path.string());
            return false;
        }

        std::ifstream file(input_path);
        if (!file.is_open()) {
            BSH_ERROR("Failed to open file: {}", input_path.string());
            return false;
        }

        std::string source((std::istreambuf_iterator(file)), {});

        Preprocessor preprocessor(input_path.parent_path());
        ParsedData parsed = preprocessor.process(source, input_path);

        if (!parsed.success) {
            BSH_ERROR("Preprocessing Error [{}]: {}", input_path.filename().string(), parsed.error_message);
            return false;
        }

        parsed.header.backend = target_backend;

        Unique<Compiler> compiler = Compiler::create(target_backend);
        if (!compiler) {
            BSH_ERROR("Unsupported or uncompiled render backend: {}", to_string(target_backend));
            return false;
        }

        const CompilerOutput compiled = compiler->compile(parsed);
        if (!compiled.success) {
            BSH_ERROR("Compilation Error [{}]: {}", input_path.filename().string(), compiled.error_message);
            return false;
        }

        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path());
        }

        auto bin = BinaryPackager::package_binary(parsed.header, compiled.stages);
        if (!BinaryPackager::save(output_path, bin)) {
            BSH_ERROR("Failed to write binary output file: {}", output_path.string());
            return false;
        }

        BSH_MESSAGE("Successfully compiled {} [{}] to {}",
                    input_path.filename().string(),
                    to_string(target_backend),
                    output_path.filename().string());
        return true;
    }

    bool BSHCompiler::compile_batch(const std::vector<BatchJob>& jobs, RenderBackend target_backend) {
        std::atomic has_error{false};

        std::for_each(std::execution::par, jobs.begin(), jobs.end(), [&](const BatchJob& job) {
            if (!compile_file(job.input, job.output, target_backend)) {
                has_error = true;
            }
        });
        return !has_error;
    }
}
