#include "../loader/pch.h"
#include "ui_pages.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace {
std::string Copy(StringView value) { return value.data ? std::string(value.data, value.size) : std::string{}; }
bool Valid(StringView value, size_t maximum, bool empty = false) {
    return value.size <= maximum && (empty ? value.size == 0 || value.data : value.data && value.size != 0);
}

struct Control {
    std::string id, label, description;
};
struct Tab {
    std::string id, title;
    std::vector<Control> control_text;
    std::vector<UiControlDescriptor> controls;
};
struct Page {
    Registration registration{};
    std::string owner, id, title, description;
    UiPageDescriptor descriptor{};
    std::vector<Tab> tab_storage;
    std::vector<UiTabDescriptor> tabs;
};

std::recursive_mutex mutex;
std::vector<std::shared_ptr<Page>> pages;

Result Validate(const UiPageDescriptor* page) {
    if (!page || page->struct_size < sizeof(*page) || !Valid(page->id, 128) || !Valid(page->title, 128) ||
        !Valid(page->description, 1024, true) || page->tab_count > 32 ||
        (page->tab_count && !page->tabs) || (!page->tab_count && !page->render)) return RESULT_INVALID_ARGUMENT;
    for (size_t t = 0; t < page->tab_count; ++t) {
        const auto& tab = page->tabs[t];
        if (tab.struct_size < sizeof(tab) || !Valid(tab.id, 128) || !Valid(tab.title, 128) ||
            tab.control_count > 512 || (tab.control_count && !tab.controls)) return RESULT_INVALID_ARGUMENT;
        for (size_t c = 0; c < tab.control_count; ++c) {
            const auto& control = tab.controls[c];
            if (control.struct_size < sizeof(control) || !Valid(control.id, 128) ||
                !Valid(control.label, 512, control.type == UI_CONTROL_SEPARATOR) ||
                !Valid(control.description, 1024, true) || control.type > UI_CONTROL_STATUS ||
                !std::isfinite(control.minimum) || !std::isfinite(control.maximum) ||
                !std::isfinite(control.step) || !std::isfinite(control.number_value)) return RESULT_INVALID_ARGUMENT;
        }
    }
    return RESULT_OK;
}

void Bind(Page& page) {
    page.descriptor.id = {page.id.data(), page.id.size()};
    page.descriptor.title = {page.title.data(), page.title.size()};
    page.descriptor.description = {page.description.data(), page.description.size()};
    page.descriptor.tabs = page.tabs.empty() ? nullptr : page.tabs.data();
    for (size_t t = 0; t < page.tabs.size(); ++t) {
        auto& descriptor = page.tabs[t]; auto& storage = page.tab_storage[t];
        descriptor.id = {storage.id.data(), storage.id.size()};
        descriptor.title = {storage.title.data(), storage.title.size()};
        descriptor.controls = storage.controls.empty() ? nullptr : storage.controls.data();
        for (size_t c = 0; c < storage.controls.size(); ++c) {
            auto& control = storage.controls[c]; const auto& text = storage.control_text[c];
            control.id = {text.id.data(), text.id.size()};
            control.label = {text.label.data(), text.label.size()};
            control.description = {text.description.data(), text.description.size()};
        }
    }
}
}

namespace UiPages {
Result Register(StringView owner, const UiPageDescriptor* source, Registration registration) {
    if (!Valid(owner, 256) || !registration) return RESULT_INVALID_ARGUMENT;
    const auto validation = Validate(source); if (validation != RESULT_OK) return validation;
    try {
        std::scoped_lock lock(mutex); const auto owner_id = Copy(owner), page_id = Copy(source->id);
        if (std::ranges::any_of(pages, [&](const auto& page) { return page->owner == owner_id && page->id == page_id; }))
            return RESULT_CONFLICT;
        auto page = std::make_shared<Page>(); page->registration = registration; page->owner = owner_id;
        page->id = page_id; page->title = Copy(source->title); page->description = Copy(source->description);
        page->descriptor = *source; page->tab_storage.reserve(source->tab_count); page->tabs.reserve(source->tab_count);
        for (size_t t = 0; t < source->tab_count; ++t) {
            const auto& input_tab = source->tabs[t];
            Tab tab; tab.id = Copy(input_tab.id); tab.title = Copy(input_tab.title);
            tab.control_text.reserve(input_tab.control_count); tab.controls.reserve(input_tab.control_count);
            for (size_t c = 0; c < input_tab.control_count; ++c) {
                const auto& input_control = input_tab.controls[c];
                tab.control_text.push_back({Copy(input_control.id), Copy(input_control.label), Copy(input_control.description)});
                tab.controls.push_back(input_control);
            }
            page->tab_storage.push_back(std::move(tab)); page->tabs.push_back(input_tab);
        }
        Bind(*page); pages.push_back(std::move(page));
        std::ranges::sort(pages, [](const auto& a, const auto& b) {
            return a->descriptor.order != b->descriptor.order ? a->descriptor.order < b->descriptor.order : a->title < b->title;
        });
        return RESULT_OK;
    } catch (...) { return RESULT_INTERNAL_ERROR; }
}

Result Visit(StringView owner, UiPageVisitor visitor, void* user_data) {
    if (!Valid(owner, 256) || !visitor) return RESULT_INVALID_ARGUMENT;
    std::scoped_lock lock(mutex);
    for (const auto& page : pages) {
        const StringView page_owner{page->owner.data(), page->owner.size()};
        const auto result = visitor(page_owner, &page->descriptor, user_data);
        if (result != RESULT_OK) return result;
    }
    return RESULT_OK;
}

Result Release(Registration registration) {
    std::scoped_lock lock(mutex);
    const auto removed = std::erase_if(pages, [&](const auto& page) { return page->registration == registration; });
    return removed ? RESULT_OK : RESULT_NOT_FOUND;
}

void ReleaseOwner(StringView owner) {
    if (!Valid(owner, 256)) return;
    const auto owner_id = Copy(owner); std::scoped_lock lock(mutex);
    std::erase_if(pages, [&](const auto& page) { return page->owner == owner_id; });
}

void Shutdown() { std::scoped_lock lock(mutex); pages.clear(); }
}
