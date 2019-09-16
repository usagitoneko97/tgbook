//
// Created by usagitoneko on 9/16/19.
//

#ifndef TGBOOK_EXCEPTIONS_H
#define TGBOOK_EXCEPTIONS_H

#include <tgbot/tgbot.h>
//use for reporting to the execution point.
class InternalException: std::exception{
public:
    virtual const char* what() const noexcept {
        // TODO: implement custom exception message
        return message.c_str();
    }
    virtual std::string getMessage() {return message;}
    explicit InternalException(const string message="TgBook exception") : message(message) {}

    template<typename... Args>
    explicit InternalException(const string &m, const Args &... args) {
        message = fmt::format(m, args...);
    }
private:
    std::string message;
};

//use for reporting to user.
class TgBookException: public InternalException {
public:
    template<typename... Args>

    explicit TgBookException
    (TgBot::Bot& _bot, int32_t message_id, const string &m, const Args &... args)
    : bot(_bot), message_id(message_id), InternalException(m, args...){}

    explicit TgBookException
    (TgBot::Bot& _bot, int32_t message_id, const string &m)
    : bot(_bot), message_id(message_id), InternalException(m){}

    const TgBot::Bot &getBot() const {
        return bot;
    }

    int32_t getMessageId() const {
        return message_id;
    }

private:
    TgBot::Bot bot;
    int32_t message_id;
};

class CalibreException : public InternalException {
  public:
    explicit CalibreException(const string message="TgBook exception") : InternalException(message) {
        spdlog::error(message);
    }

    template<typename... Args>
    explicit CalibreException(const string &m, const Args &... args) : InternalException(m, args...) {
        spdlog::error(this->what());
    }
};

#endif //TGBOOK_EXCEPTIONS_H