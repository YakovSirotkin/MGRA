/*
 * logging.hpp
 *
 *  Created on: 02.03.2011
 *      Author: Mikhail Dvorkin
 */

#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include "log4cxx/logger.h"
#include <string>

using namespace log4cxx;

#define LOGGER(category) static const LoggerPtr _default_logger(Logger::getLogger(category));

#define TRACE(message) LOG4CXX_TRACE(_default_logger, message);
#define DEBUG(message) LOG4CXX_DEBUG(_default_logger, message);
#define INFO(message)  LOG4CXX_INFO (_default_logger, message);
#define WARN(message)  LOG4CXX_WARN (_default_logger, message);
#define ERROR(message) LOG4CXX_ERROR(_default_logger, message);
#define LOG_ASSERT(condition, message) LOG4CXX_ASSERT(_default_logger, condition, message);
#define FATAL(message) LOG4CXX_FATAL(_default_logger, message);

#endif /* LOGGING_HPP_ */