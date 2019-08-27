#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml.hpp>
#include <nlohmann/json.hpp>
#include <cpr/response.h>
#include <cpr/cpr.h>
#include <iostream>
#include <string>
#include <list>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>

#include <tgbot/tgbot.h>

using namespace std;
using namespace TgBot;

class BookHandler {
    string name;
public:
    explicit BookHandler(string& s): name(s) {};
    string get_name() {return name;}
    string get_book_summary();
};

string BookHandler::get_book_summary() {
    string s = "the chosen book is " + name;
    return s;
}


class Book {
public:
    Book(const string &title, std::vector<string> &author, unsigned int calibreId = -1,
            std::vector<string> format={""}, std::map<string, string> format_path=std::map<string, string>())
            : title(title), author(author), calibre_id(calibreId), formats(format), format_path(format_path){}

    Book(const string &title, const string &_author, unsigned int calibreId=-1, std::vector<string> format={""},
         std::map<string, string> format_path=std::map<string, string>())
            : title(title), calibre_id(calibreId), formats(format), format_path(format_path){
        author.push_back(_author);
    }
    friend ostream& operator<<(ostream& os, const Book& book);
    string dump() const;

private:
    string title;
    unsigned int calibre_id;
    std::vector<string> author, formats;
    std::map<string, string> format_path;

public:
    unsigned int getCalibreId() const {
        return calibre_id;
    }

    const string &getTitle() const {
        return title;
    }

    const vector<string> &getAuthor() const {
        return author;
    }

    const vector<string> &getFormat() const {
        return formats;
    }
};

string Book::dump() const {
    string authors_repr;
    for (const string& a : author) {
        authors_repr = authors_repr + a + ',';
    }
    string first_line = std::to_string(calibre_id) + ": " + title + " by " + authors_repr;
    string _formats = formats[0];
    for (auto f = (formats.begin() + 1); f != formats.end(); ++f) {
        _formats.append(", " + *f);
    }
    string format_repr = "\n    available formats: " + _formats;
    return first_line + format_repr;
}
ostream& operator<<(ostream& os, const Book& book) {
    os << book.dump();
    return os;
}

Book& query_goodreads(const string& title, const string& author="") {
    cpr::Response r = cpr::Get(cpr::Url{"https://www.goodreads.com/search/index.xml"},
                               cpr::Parameters{{"key", "rlrW2MX64G1FMRxPJKVL7w"},
                                               {"q", title}}
    );
    rapidxml::xml_document<> doc;
    doc.parse<0>(&r.text[0]);
    rapidxml::xml_node<>* book = doc.first_node("GoodreadsResponse")->first_node("search")->first_node("results")->first_node("work")->first_node("best_book");
    Book* b = new Book(book->first_node("title")->value(), book->first_node("author")->first_node("name")->value());
    return *b;
}

std::vector<Book> search_calibre(const string &title) {
    cpr::Response r = cpr::Get(cpr::Url{"192.168.1.105:8080/ajax/search"},
                               cpr::Parameters{{"query", title},
                                               {"sort_order", "desc"}}
    );
    std::vector<Book> books_found={};
    if (r.text.length() > 0) {
        nlohmann::json json = nlohmann::json::parse(r.text);
        std::vector<int> value = json.value("book_ids", std::vector<int>{-1});
        std::map<string ,string> format_path;
        for (int book_id : value) {
            string url = "192.168.1.105:8080/ajax/book/" + std::to_string(book_id);
            cpr::Response book_response = cpr::Get(cpr::Url{url});
            nlohmann::json book_json = nlohmann::json::parse(book_response.text);
            string title = book_json.value("title", "invalid_title");
            std::vector<string> formats = book_json.value("formats", std::vector<string>{"empty formats"});
            std::vector<string> authors = book_json.value("authors", std::vector<string>{"invalid author"});
            for (string f : formats) {
                string p = book_json.at("format_metadata").at(f).at("path").get<string>();
                format_path[f] = p;
            }
            books_found.emplace_back(Book(title, authors, book_id, formats, format_path));
        }
        return books_found;
    }
}

string get_book_path(int book_id, string& format) {
    std::map<string ,string> format_path;
    string url = "192.168.1.105:8080/ajax/book/" + std::to_string(book_id);
    cpr::Response book_response = cpr::Get(cpr::Url{url});
    nlohmann::json book_json = nlohmann::json::parse(book_response.text);
    string p = book_json.at("format_metadata").at(format).at("path").get<string>();
    return p;
}


std::vector<Book> search_calibre(Book& book) {
    return search_calibre(book.getTitle());
}


int main() {
    string token("964403607:AAFaCL75bqTie0r8FBJAsHcC0c3nkNtS0j4");
    TgBot::Bot bot(token);
    bot.getEvents().onCommand("book", [&bot](TgBot::Message::Ptr message) {
        string book_title = message->text.substr(6, message->text.length());
        cout << book_title;
        std::vector<Book> calibre_books = search_calibre(book_title);
        string messages("books available in server:\n");
        for (unsigned long i=0; i<calibre_books.size(); i++) {
            messages += calibre_books[i].dump() += "\n";
        }
        bot.getApi().sendMessage(message->chat->id, messages);
    });
    bot.getEvents().onCommand("book-get", [&bot](TgBot::Message::Ptr message) {
        std:istringstream iss(message->text.substr(10, message->text.length()));
        string s;
        std::vector<string> commands;
        while (getline(iss, s, ' ')) {
            commands.push_back(s);
        }
        if (commands.size() >= 2){
            int book_id = std::stoi(commands[0]);
            string book_format = commands[1];
            string path = get_book_path(book_id, book_format);
            bot.getApi().sendDocument(message->chat->id, TgBot::InputFile::fromFile(path, "text/plain"));
        }
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}
