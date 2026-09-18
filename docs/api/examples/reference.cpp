#include "shroudtopia.h"

#include <cstring>
#include <vector>

namespace shroudtopia_docs {
StringView view(const char* value) {
    return {value, std::strlen(value)};
}

Result CALL echo_command(StringView arguments, void* user_data) {
    const Api* api = static_cast<const Api*>(user_data);
    if (api == nullptr) return RESULT_INVALID_ARGUMENT;
    return api->log(view("docs.example"), LOG_INFO, arguments);
}

Result example_register_command(const Api* api, Registration* registration) {
    if (api == nullptr || registration == nullptr) return RESULT_INVALID_ARGUMENT;
    const CommandDescriptor command{
        sizeof(CommandDescriptor),
        view("docs.echo"),
        view("Writes its arguments to the Shroudtopia log."),
        echo_command,
        const_cast<Api*>(api)
    };
    return api->register_command(view("docs.example"), &command, registration);
}

Result example_read_asset(const Api* api, const AssetId* asset, std::vector<char>* json) {
    if (api == nullptr || asset == nullptr || json == nullptr) return RESULT_INVALID_ARGUMENT;
    size_t required_size = 0;
    Result result = api->get_asset(view("docs.example"), asset, nullptr, 0, &required_size);
    if (result != RESULT_OK && result != RESULT_INVALID_ARGUMENT) return result;
    json->resize(required_size);
    return api->get_asset(view("docs.example"), asset, json->data(), json->size(), &required_size);
}
}
