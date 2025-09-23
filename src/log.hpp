#pragma once

#include "ast.hpp"
#include <string>

namespace cha {

// Log levels
enum class LogLevel {
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

// Logger class
class Logger {
public:
    static Logger& instance();
    
    void log(LogLevel level, const std::string& message);
    void log(LogLevel level, const AstLocation& location, const std::string& message);
    
    // Convenience methods
    void error(const std::string& message);
    void error(const AstLocation& location, const std::string& message);
    void warning(const std::string& message);
    void warning(const AstLocation& location, const std::string& message);
    void info(const std::string& message);
    void debug(const std::string& message);
    
    void set_level(LogLevel level) { min_level_ = level; }
    
private:
    Logger() = default;
    LogLevel min_level_ = LogLevel::INFO;
    
    std::string level_to_string(LogLevel level) const;
    std::string format_location(const AstLocation& location) const;
};

// Global logging functions for compatibility
void log_error(const std::string& message);
void log_error(const AstLocation& location, const std::string& message);
void log_validation_error(const AstLocation& location, const std::string& message);
void log_warning(const std::string& message);
void log_info(const std::string& message);

} // namespace cha
