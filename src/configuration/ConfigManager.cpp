#include "configuration/ConfigManager.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace bike_dashcam::configuration {
namespace {

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](const unsigned char character) {
        return std::isspace(character) != 0;
    });

    if (first == value.end()) {
        return {};
    }

    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](const unsigned char character) {
        return std::isspace(character) != 0;
    }).base();

    return std::string(first, last);
}

std::string stripComments(const std::string& line) {
    bool in_single_quotes = false;
    bool in_double_quotes = false;

    for (std::size_t index = 0; index < line.size(); ++index) {
        const char character = line[index];

        if (character == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
        } else if (character == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
        } else if (character == '#' && !in_single_quotes && !in_double_quotes) {
            return line.substr(0, index);
        }
    }

    return line;
}

std::string unquote(const std::string& value) {
    if (value.size() >= 2) {
        const char first = value.front();
        const char last = value.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return value.substr(1, value.size() - 2);
        }
    }

    return value;
}

std::size_t leadingWhitespace(const std::string& value) {
    std::size_t count = 0;
    while (count < value.size() && std::isspace(static_cast<unsigned char>(value[count])) != 0) {
        ++count;
    }
    return count;
}

std::string errorMessage(const std::size_t line_number, const std::string& message) {
    std::ostringstream stream;
    stream << "Line " << line_number << ": " << message;
    return stream.str();
}

bool parseInt(const std::string& raw_value, int& target) {
    try {
        std::size_t processed = 0;
        const int parsed = std::stoi(raw_value, &processed);
        if (processed != raw_value.size()) {
            return false;
        }

        target = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parseBool(const std::string& raw_value, bool& target) {
    std::string normalized;
    normalized.reserve(raw_value.size());
    for (const char character : raw_value) {
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    }

    if (normalized == "true" || normalized == "yes" || normalized == "on" || normalized == "1") {
        target = true;
        return true;
    }
    if (normalized == "false" || normalized == "no" || normalized == "off" || normalized == "0") {
        target = false;
        return true;
    }

    return false;
}

bool isSupportedLogLevel(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (const char character : value) {
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));
    }

    return normalized == "DEBUG" || normalized == "INFO" || normalized == "WARN" ||
           normalized == "WARNING" || normalized == "ERROR";
}

}  // namespace

bool ConfigManager::loadFromFile(const std::filesystem::path& file_path) {
    config_ = DashcamConfig{};
    config_path_ = file_path;
    errors_.clear();

    std::ifstream input(file_path);
    if (!input.is_open()) {
        errors_.push_back("Unable to open config file: " + file_path.string());
        return false;
    }

    std::string current_section;
    std::size_t line_number = 0;

    for (std::string raw_line; std::getline(input, raw_line);) {
        ++line_number;

        const std::string line_without_comments = stripComments(raw_line);
        const std::string trimmed_line = trim(line_without_comments);
        if (trimmed_line.empty()) {
            continue;
        }

        const std::size_t indentation = leadingWhitespace(raw_line);
        if (trimmed_line.back() == ':') {
            if (indentation != 0) {
                errors_.push_back(errorMessage(line_number, "Nested sections are not supported in Milestone 1 config."));
                continue;
            }

            current_section = normalizeKey(trim(trimmed_line.substr(0, trimmed_line.size() - 1)));
            continue;
        }

        const std::size_t separator = trimmed_line.find(':');
        if (separator == std::string::npos) {
            errors_.push_back(errorMessage(line_number, "Expected a key/value pair."));
            continue;
        }

        if (indentation == 0) {
            errors_.push_back(errorMessage(line_number, "Root scalar values are not supported; use a named section."));
            continue;
        }

        if (current_section.empty()) {
            errors_.push_back(errorMessage(line_number, "Key/value pair found before any section header."));
            continue;
        }

        const std::string key = normalizeKey(trim(trimmed_line.substr(0, separator)));
        const std::string value = trim(trimmed_line.substr(separator + 1));
        if (value.empty()) {
            errors_.push_back(errorMessage(line_number, "Missing value for key '" + key + "'."));
            continue;
        }

        assignValue(current_section, key, unquote(value), line_number);
    }

    return errors_.empty();
}

const DashcamConfig& ConfigManager::config() const {
    return config_;
}

const std::filesystem::path& ConfigManager::configPath() const {
    return config_path_;
}

const std::vector<std::string>& ConfigManager::errors() const {
    return errors_;
}

void ConfigManager::assignValue(
    const std::string& section,
    const std::string& key,
    const std::string& value,
    const std::size_t line_number) {
    int parsed_int = 0;
    bool parsed_bool = false;

    if (section == "application") {
        if (key == "name") {
            config_.application.name = value;
            return;
        }
        if (key == "mode") {
            config_.application.mode = value;
            return;
        }
        if (key == "recordings_directory") {
            config_.application.recordings_directory = value;
            return;
        }
        if (key == "logs_directory") {
            config_.application.logs_directory = value;
            return;
        }
    } else if (section == "cameras") {
        if (key == "expected_camera_count" && parseInt(value, parsed_int) && parsed_int > 0) {
            config_.cameras.expected_camera_count = parsed_int;
            return;
        }
        if (key == "target_width" && parseInt(value, parsed_int) && parsed_int > 0) {
            config_.cameras.target_width = parsed_int;
            return;
        }
        if (key == "target_height" && parseInt(value, parsed_int) && parsed_int > 0) {
            config_.cameras.target_height = parsed_int;
            return;
        }
        if (key == "target_fps" && parseInt(value, parsed_int) && parsed_int > 0) {
            config_.cameras.target_fps = parsed_int;
            return;
        }
    } else if (section == "recording") {
        if (key == "codec") {
            config_.recording.codec = value;
            return;
        }
        if (key == "segment_duration_seconds" && parseInt(value, parsed_int) && parsed_int > 0) {
            config_.recording.segment_duration_seconds = parsed_int;
            return;
        }
        if (key == "continuous" && parseBool(value, parsed_bool)) {
            config_.recording.continuous = parsed_bool;
            return;
        }
    } else if (section == "logging") {
        if (key == "level" && isSupportedLogLevel(value)) {
            config_.logging.level = value;
            return;
        }
        if (key == "console_enabled" && parseBool(value, parsed_bool)) {
            config_.logging.console_enabled = parsed_bool;
            return;
        }
    }

    errors_.push_back(errorMessage(
        line_number,
        "Unsupported or invalid setting '" + section + "." + key + "' with value '" + value + "'."));
}

std::string ConfigManager::normalizeKey(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());
    for (const char character : value) {
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    }
    return normalized;
}

}  // namespace bike_dashcam::configuration
