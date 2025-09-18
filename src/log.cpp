#include "log.hpp"
#include <iostream>
#include <sstream>

namespace cha {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level <= min_level_) {
        std::cerr << level_to_string(level) << ": " << message << std::endl;
    }
}

void Logger::log(LogLevel level, const AstLocation& location, const std::string& message) {
    if (level <= min_level_) {
        std::cerr << level_to_string(level) << ": " << format_location(location) 
                  << " " << message << std::endl;
    }
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::error(const AstLocation& location, const std::string& message) {
    log(LogLevel::ERROR, location, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::warning(const AstLocation& location, const std::string& message) {
    log(LogLevel::WARNING, location, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::ERROR:   return "Error";
        case LogLevel::WARNING: return "Warning";
        case LogLevel::INFO:    return "Info";
        case LogLevel::DEBUG:   return "Debug";
        default:                return "Unknown";
    }
}

std::string Logger::format_location(const AstLocation& location) const {
    std::stringstream ss;
    ss << location.file << ":" << location.line_begin << ":" << location.column_begin;
    return ss.str();
}

// Global logging functions for compatibility
void log_error(const std::string& message) {
    Logger::instance().error(message);
}

void log_error(const AstLocation& location, const std::string& message) {
    Logger::instance().error(location, message);
}

void log_validation_error(const AstLocation& location, const std::string& message) {
    Logger::instance().error(location, message);
}

void log_warning(const std::string& message) {
    Logger::instance().warning(message);
}

void log_info(const std::string& message) {
    Logger::instance().info(message);
}

} // namespace cha

void log_error(const char *format, ...) {
  fprintf(stderr, "[error] ");

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}
