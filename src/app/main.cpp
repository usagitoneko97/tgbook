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

using namespace std;
using namespace TgBot;

using CommandFunc = std::function<void(TgBot::Bot& bot, const TgBot::Message::Ptr message)>;

inline void _wrapper(CommandFunc& func, TgBot::Bot& bot, const TgBot::Message::Ptr& message) {
    try{
        spdlog::info("received commands : {}", message->text);
        func(bot, message);
    }
    catch (CalibreException& e) {
        throw TgBookException(bot, message->chat->id, e.what());
    }
}

void command_book(TgBot::Bot& bot, const TgBot::Message::Ptr message) {
    CalibreApi calibre_api = ApiHub::getOrInitInstance<CalibreApi>();
    GoodreadsApi goodreads_api = ApiHub::getOrInitInstance<GoodreadsApi>();
    std::shared_ptr<BookParser> parser = Parser<BookParser>().parse(message->text);
    switch(parser->getMode()) {
        case BookParser::TITLE: {
            std::vector<int> book_ids = calibre_api.search(parser->getBookIdentifier());
            std::vector<Book> books_found;
            string messages("books available in server:\n");
            for (int id : book_ids) {
                Book located_book = calibre_api.locate_book(id);
                books_found.emplace_back(located_book);
                messages += located_book.dump() += "\n";
            }
            try{
                bot.getApi().sendMessage(message->chat->id, messages);
            }
            catch (std::exception& e) {
                spdlog::warn("Message length is too long.");
                messages = messages.substr(0, 4000);
                messages += "\n\n REDACTED since it's too long. Try to scope down the search";
                bot.getApi().sendMessage(message->chat->id, messages);
            }
        }
        case BookParser::ISBN: {
            // Search Isbn in goodreads for the title. And use it
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

inline void register_command_handler(std::string command, CommandFunc func, TgBot::Bot& bot) {
    bot.getEvents().onCommand(command, std::bind(_wrapper, func, bot, std::placeholders::_1));
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

        register_command_handler("book", &command_book, bot);
        register_command_handler("get", &command_book_dl, bot);
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
