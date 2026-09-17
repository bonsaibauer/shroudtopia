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
        Config::jConfig = {{"enableLogging", true}, {"logLevel", "ALL"}};
        check(fs::exists("shroudtopia.log") && fs::file_size("shroudtopia.log") == 0);
        size_t archives = 0;
        for (const auto& entry : fs::directory_iterator("shroudtopia_logs")) {
            if (!entry.is_regular_file()) continue;
            ++archives;
            std::ifstream archived(entry.path(), std::ios::binary);
            check(std::string(std::istreambuf_iterator<char>(archived), std::istreambuf_iterator<char>()) == "previous session\n");
        }
        check(archives == 1);
        check(Utils::ReadLogTail(LOG_SOURCE_GAME, buffer, sizeof(buffer), &size) == RESULT_NOT_FOUND && size == 0);
        { std::ofstream file("shroudtopia.log", std::ios::binary); file << "old line\nsecond line\n"; }
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, buffer, 15, &size) == RESULT_OK);
        check(std::string(buffer, size) == "second line\n");
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, buffer, 12, &size) == RESULT_OK);
        check(std::string(buffer, size) == "second line\n");
        Utils::Log(LOG_WARNING, "mod.test central logger marker");
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == RESULT_OK);
        check(std::string(buffer, size).find("central logger marker") != std::string::npos);
        Utils::Log(LOG_DEBUG, "debug level marker");
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == RESULT_OK);
        check(std::string(buffer, size).find("] [shroudtopia] debug level marker") != std::string::npos);
        Config::jConfig["logLevel"] = "INFO";
        Utils::Log(LOG_DEBUG, "filtered debug marker");
        Utils::Log(LOG_INFO, "visible info marker");
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == RESULT_OK);
        check(std::string(buffer, size).find("filtered debug marker") == std::string::npos);
        check(std::string(buffer, size).find("visible info marker") != std::string::npos);
        { std::ofstream file("enshrouded.log", std::ios::binary); file << "[I 00:00:01] game source\n"; }
        check(Utils::ReadLogTail(LOG_SOURCE_GAME, buffer, sizeof(buffer), &size) == RESULT_OK);
        check(std::string(buffer, size) == "[I 00:00:01] game source\n");
        { std::ofstream file("shroudtopia.log", std::ios::trunc | std::ios::binary); file << "new session\n"; }
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, buffer, sizeof(buffer), &size) == RESULT_OK);
        check(std::string(buffer, size) == "new session\n");
        check(Utils::ReadLogTail(static_cast<LogSource>(42), buffer, sizeof(buffer), &size) == RESULT_INVALID_ARGUMENT);
        check(Utils::ReadLogTail(LOG_SOURCE_LOADER, nullptr, sizeof(buffer), &size) == RESULT_INVALID_ARGUMENT);
        const StringView owner{"mod.test", 8}, stranger{"mod.other", 9}, label{"Log", 3};
        const TextWindowOptions descriptor{sizeof(descriptor), label, &label, 1, 121};
        TextWindow window = 0;
        check(UiText::Create(owner, &descriptor, &window) == RESULT_OK && window != 0);
        TextWindowStatus status = TEXT_PENDING;
        for (int attempt = 0; attempt < 100 && status == TEXT_PENDING; ++attempt) {
            Sleep(10); check(UiText::Status(owner, window, &status) == RESULT_OK);
        }
        check(status == TEXT_READY);
        check(UiText::SetText(stranger, window, 0, label) == RESULT_PERMISSION_DENIED);
        check(UiText::Destroy(stranger, window) == RESULT_PERMISSION_DENIED);
        check(UiText::SetText(owner, window, 0, label) == RESULT_OK);
        check(UiText::SetText(owner, window, 1, label) == RESULT_INVALID_ARGUMENT);
        check(UiText::Destroy(owner, window) == RESULT_OK);
        check(UiText::Status(owner, window, &status) == RESULT_NOT_FOUND);
        check(UiText::Create(owner, &descriptor, &window) == RESULT_OK);
        UiText::ReleaseOwner(owner);
        check(UiText::Status(owner, window, &status) == RESULT_NOT_FOUND);
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; result = 1; }
    UiText::Shutdown();
    fs::current_path(previous);
    fs::remove_all(fixture);
    if (!result) std::cout << "Log sessions: rotation, tail, central writer, source switch, truncation, invalid input passed.\n";
    return result;
}
