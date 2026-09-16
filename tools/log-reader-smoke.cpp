#include "config.h"
#include "utils.h"
#include "ui_text.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

json Config::jConfig = json::object();
std::filesystem::file_time_type Config::lastModifiedTime{};

int main() {
    namespace fs = std::filesystem;
    const auto previous = fs::current_path();
    const auto fixture = fs::temp_directory_path() / ("shroudtopia-log-reader-" + std::to_string(GetCurrentProcessId()));
    if (!fs::create_directory(fixture)) return 1;
    int result = 0;
    try {
        fs::current_path(fixture);
        char buffer[128]{}; size_t size = 999;
        auto check = [&](bool ok) { if (!ok) throw std::runtime_error("log reader contract failed"); };
        { std::ofstream file("shroudtopia.log", std::ios::binary); file << "previous session\n"; }
        Utils::BeginLogSession("client");
        check(fs::exists("shroudtopia.log") && fs::file_size("shroudtopia.log") == 0);
        size_t archives = 0;
        for (const auto& entry : fs::directory_iterator("shroudtopia_logs")) {
            if (!entry.is_regular_file()) continue;
            ++archives;
            std::ifstream archived(entry.path(), std::ios::binary);
            check(std::string(std::istreambuf_iterator<char>(archived), std::istreambuf_iterator<char>()) == "previous session\n");
        }
        check(archives == 1);
        check(Utils::ReadLogTail(ST_LOG_SOURCE_GAME, buffer, sizeof(buffer), &size) == ST_RESULT_NOT_FOUND && size == 0);
        { std::ofstream file("shroudtopia.log", std::ios::binary); file << "old line\nsecond line\n"; }
        check(Utils::ReadLogTail(ST_LOG_SOURCE_LOADER, buffer, 15, &size) == ST_RESULT_OK);
        check(std::string(buffer, size) == "second line\n");
        check(Utils::ReadLogTail(ST_LOG_SOURCE_LOADER, buffer, 12, &size) == ST_RESULT_OK);
        check(std::string(buffer, size) == "second line\n");
        Utils::Log(Utils::WARN, "mod.test central logger marker");
        check(Utils::ReadLogTail(ST_LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == ST_RESULT_OK);
        check(std::string(buffer, size).find("central logger marker") != std::string::npos);
        Config::jConfig = {{"enableLogging", true}};
        Utils::Log(Utils::DEBUG, "debug level marker");
        check(Utils::ReadLogTail(ST_LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == ST_RESULT_OK);
        check(std::string(buffer, size).find("[DEBUG] debug level marker") != std::string::npos);
        { std::ofstream file("enshrouded.log", std::ios::binary); file << "[I 00:00:01] game source\n"; }
        check(Utils::ReadLogTail(ST_LOG_SOURCE_GAME, buffer, sizeof(buffer), &size) == ST_RESULT_OK);
        check(std::string(buffer, size) == "[I 00:00:01] game source\n");
        { std::ofstream file("shroudtopia.log", std::ios::trunc | std::ios::binary); file << "new session\n"; }
        check(Utils::ReadLogTail(ST_LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == ST_RESULT_OK);
        check(std::string(buffer, size) == "new session\n");
        check(Utils::ReadLogTail(static_cast<ST_LogSourceV1>(42), buffer, sizeof(buffer), &size) == ST_RESULT_INVALID_ARGUMENT);
        check(Utils::ReadLogTail(ST_LOG_SOURCE_LOADER, nullptr, sizeof(buffer), &size) == ST_RESULT_INVALID_ARGUMENT);
        const auto* ui = UiText::Api();
        const ST_StringView owner{"mod.test", 8}, stranger{"mod.other", 9}, label{"Log", 3};
        const ST_TextWindowDescriptorV1 descriptor{sizeof(descriptor), label, &label, 1, 121};
        ST_TextWindow window = 0;
        check(ui->create(owner, &descriptor, &window) == ST_RESULT_OK && window != 0);
        ST_TextWindowStatusV1 status = ST_TEXT_PENDING;
        for (int attempt = 0; attempt < 100 && status == ST_TEXT_PENDING; ++attempt) {
            Sleep(10); check(ui->get_status(owner, window, &status) == ST_RESULT_OK);
        }
        check(status == ST_TEXT_READY);
        check(ui->set_text(stranger, window, 0, label) == ST_RESULT_PERMISSION_DENIED);
        check(ui->destroy(stranger, window) == ST_RESULT_PERMISSION_DENIED);
        check(ui->set_text(owner, window, 0, label) == ST_RESULT_OK);
        check(ui->set_text(owner, window, 1, label) == ST_RESULT_INVALID_ARGUMENT);
        check(ui->destroy(owner, window) == ST_RESULT_OK);
        check(ui->get_status(owner, window, &status) == ST_RESULT_NOT_FOUND);
        check(ui->create(owner, &descriptor, &window) == ST_RESULT_OK);
        UiText::ReleaseOwner(owner);
        check(ui->get_status(owner, window, &status) == ST_RESULT_NOT_FOUND);
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; result = 1; }
    UiText::Shutdown();
    fs::current_path(previous);
    fs::remove_all(fixture);
    if (!result) std::cout << "Log sessions: rotation, tail, central writer, source switch, truncation, invalid input passed.\n";
    return result;
}
