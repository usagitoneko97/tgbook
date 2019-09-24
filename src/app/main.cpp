#include <nlohmann/json.hpp>
#include <string>
#include <exception>
#include <tgbot/tgbot.h>
#include <functional>

#include "book.h"
#include "calibre_api.h"
#include "goodreads_api.h"
#include "spdlog/spdlog.h"
#include "parser.h"
#include "exceptions.h"
#include "api_instance.h"

using namespace TgBot;

using CommandFunc = std::function<void(TgBot::Bot& bot, const TgBot::Message::Ptr message)>;

inline void _wrapper(CommandFunc& func, TgBot::Bot& bot, string& help_message, const TgBot::Message::Ptr& message) {
    try{
        spdlog::info("received commands : {}", message->text);
        func(bot, message);
    }
    catch (CalibreException& e) {
        throw TgBookException(bot, message->chat->id, fmt::format("{}\n{}", e.what(), help_message));
    }
    catch (GoodreadsException& e) {
        throw TgBookException(bot, message->chat->id, fmt::format("{}\n{}", e.what(), help_message));
    }
    catch (ParserException& e) {
        throw TgBookException(bot, message->chat->id, fmt::format("{}\n{}", e.what(), help_message));
    }
}

void command_book(TgBot::Bot& bot, const TgBot::Message::Ptr message) {
    CalibreApi calibre_api = ApiHub::getOrInitInstance<CalibreApi>();
    GoodreadsApi goodreads_api = ApiHub::getOrInitInstance<GoodreadsApi>();
    std::shared_ptr<BookParser> parser = Parser<BookParser>().parse(message->text);
    switch(parser->getMode()) {
        case BookParser::TITLE: {
            spdlog::info("TITLE mode selected.");
            std::string messages("books available in server:\n");
            auto located_book = calibre_api.locate_book(parser->getBookIdentifier());
            for (const auto& book : *located_book) {
                messages += book.dump() += "\n";
            }
            try{
                bot.getApi().sendMessage(message->chat->id, messages);
                spdlog::info("successfully sent message to chat id: {}", message->chat->id);
            }
            catch (std::exception& e) {
                spdlog::warn("Message length is too long.");
                messages = messages.substr(0, 4000);
                messages += "\n\n REDACTED since it's too long. Try to scope down the search";
                bot.getApi().sendMessage(message->chat->id, messages);
                spdlog::info("successfully sent message to chat id: {}", message->chat->id);
            }
            break;
        }
        case BookParser::ISBN: {
            // Search Isbn in goodreads for the title. And use it
            spdlog::info("ISBN mode selected.");
            std::shared_ptr<Book> book = goodreads_api.search(parser->getBookIdentifier());
            auto located_book = calibre_api.locate_book(book.get()->getTitle());
            std::string messages("books available in server:\n");
            for (const auto& book : *located_book) {
                messages += book.dump() += "\n";
            }
            try{
                bot.getApi().sendMessage(message->chat->id, messages);
                spdlog::info("successfully sent message to chat id: {}", message->chat->id);
            }
            catch (std::exception& e) {
                spdlog::warn("Message length is too long.");
                messages = messages.substr(0, 4000);
                messages += "\n\n REDACTED since it's too long. Try to scope down the search";
                bot.getApi().sendMessage(message->chat->id, messages);
                spdlog::info("successfully sent message to chat id: {}", message->chat->id);
            }
        }
    }
}

void command_book_dl(TgBot::Bot& bot, const TgBot::Message::Ptr message) {
    CalibreApi calibre_api = ApiHub::getOrInitInstance<CalibreApi>();
    std::shared_ptr<BookGetParser> parser = Parser<BookGetParser>().parse(message->text);
    string path = calibre_api.get_book_path(parser->getBookId(), parser->getBookFormat());
    spdlog::info("trying to get book from path: {}", path);
    auto result = bot.getApi().sendDocument(message->chat->id, TgBot::InputFile::fromFile(path, "text/plain"));
    spdlog::info("successfully sent document to chat id: {}", message->chat->id);
}


void command_info(TgBot::Bot& bot, const TgBot::Message::Ptr message) {
    CalibreApi calibre_api = ApiHub::getOrInitInstance<CalibreApi>();
    int books_size = calibre_api.get_info();
    bot.getApi().sendMessage(message->chat->id,
                             fmt::format("Total number of books available: {}", to_string(books_size)));
}

inline void register_command_handler(std::string command, CommandFunc func, TgBot::Bot& bot, string &&help_message) {
    bot.getEvents().onCommand(
        command, std::bind(_wrapper, func, bot, std::forward<std::string>(help_message), std::placeholders::_1));
}

inline std::string _getenv(const char* env_name) {
    const char* env = std::getenv(env_name);
    if (env == nullptr){
        throw InternalException("Environment Key: {} is not set.", env_name);
    }
    else {
        return std::string(env);
    }
}

using EnvMap = std::map<std::string, std::string>;
EnvMap* get_required_envs() {
    auto map = new EnvMap;
    (*map)["goodreads"] = _getenv("GOODREADS_KEY");
    (*map)["token"] = _getenv("TG_TOKEN");
    (*map)["calibre"] = _getenv("CALIBRE_IP");
    spdlog::info("calibre using IP: {}", (*map)["calibre"]);
    spdlog::info("using telegram token: {}", (*map)["token"]);
    spdlog::info("using goodreads developer key: {}", (*map)["goodreads"]);
    return map;
}

int main() {
    try{
        //initialize all interface
        EnvMap* envs = get_required_envs();
        TgBot::Bot bot((*envs)["token"]);
        spdlog::info("Bot Username: {}", bot.getApi().getMe()->username.c_str());
        ApiHub::getOrInitInstance<CalibreApi>().update((*envs)["calibre"]);
        ApiHub::getOrInitInstance<GoodreadsApi>().update((*envs)["goodreads"]);
        delete envs;

        register_command_handler("book", &command_book, bot,
            "Command: \n"
            "/book <title string>\n"
            "/book --isbn <ISBN number>\n"
            );
        register_command_handler("get", &command_book_dl, bot,
            "Command: \n"
            "/get <book-id> <book-format>\n"
            );
        register_command_handler("info", &command_info, bot, "");
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            try{
                longPoll.start();
            }
            catch (TgBookException& e) {
                e.getBot().getApi().sendMessage(e.getMessageId(), e.what());
            }
            catch (TgBot::TgException& e) {
                spdlog::error("Fatal Error in Tgbot. Message: {}", e.what());
            }
            catch (boost::system::system_error& e) {
                spdlog::error("Fatal Error in Tgbot. Message: {}", e.what());
            }
            catch (InternalException) {}
        }
        return 0;
    }
    catch (const InternalException& e) {
        spdlog::error(e.what());
    }
}
