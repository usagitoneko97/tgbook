//
// Created by usagitoneko on 9/17/19.
//

#ifndef TGBOOK_LOGGING_UTILS_H
#define TGBOOK_LOGGING_UTILS_H

#define LOG_REQ(meth, url)                  spdlog::info("Sending {} request: {}", meth, url)
#define LOG_REQ_PARAMS(meth, url, params)   spdlog::info("Sending {} request: {}", meth, url)

#endif // TGBOOK_LOGGING_UTILS_H
