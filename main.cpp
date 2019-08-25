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
    Book(const string &title, std::vector<string> &author, unsigned int calibreId=-1)
            : title(title), author(author), calibre_id(calibreId) {}

    Book(const string &title, const string &_author, unsigned int calibreId=-1)
            : title(title), calibre_id(calibreId){
        author.push_back(_author);
    }
    friend ostream& operator<<(ostream& os, const Book& book);
    string dump() const;

private:
    string title;
    unsigned int calibre_id;
    std::vector<string> author;

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
};

string Book::dump() const {
    string authors_repr;
    for (const string& a : author) {
        authors_repr = authors_repr + a + ',';
    }
    return std::to_string(calibre_id) + ": " + title + " by " + authors_repr;
}
ostream& operator<<(ostream& os, const Book& book) {
    os << book.dump();
    return os;
}

Book& get_book_info(const string& title, const string& author="") {
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
    nlohmann::json json = nlohmann::json::parse(r.text);
    std::vector<int> value = json.value("book_ids", std::vector<int>{-1});
    std::vector<Book> books_found;
    for (int book_id : value) {
        string url = "192.168.1.105:8080/ajax/book/" + std::to_string(book_id);
        cpr::Response book_response = cpr::Get(cpr::Url{url},
                                               cpr::Parameters{{"query", title},
                                                               {"sort_order", "desc"}}
        );
        nlohmann::json book_json = nlohmann::json::parse(book_response.text);
        string title = book_json.value("title", "invalid_title");
        std::vector<string> authors = book_json.value("authors", std::vector<string>{"invalid author"});
        books_found.emplace_back(Book(title, authors, book_id));
    }
    return books_found;
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
    bot.getEvents().onCommand("send", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendDocument(message->chat->id, TgBot::InputFile::fromFile("/media/Download/something.txt", "text/plain"));
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
