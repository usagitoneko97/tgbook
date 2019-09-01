//
// Created by usagitoneko on 8/30/19.
//

#ifndef TGBOOK_PARSER_H
#define TGBOOK_PARSER_H

#include <string>
#include <memory>

template<class T>
class Parser {
public:
    std::shared_ptr<T> parse(const std::string& message) {
        std::istringstream iss(message);
        std::string s;
        std::vector<std::string> commands;
        while (getline(iss, s, ' ')) {
            commands.push_back(s);
        }
        std::shared_ptr<T> custom_parser = std::make_shared<T>(T());
        custom_parser->construct(commands);
        return custom_parser;
    }
    T& underlying() { return static_cast<T&>(*this); }
};

class BookGetParser {
public:
    int getBookId() const {
        return book_id;
    }

    const std::string &getBookFormat() const {
        return book_format;
    }

    void construct(std::vector<std::string> commands) ;

private:
    int book_id;
    std::string book_format;
};

std::vector<std::string> parse(std::string& message);

#endif //TGBOOK_PARSER_H
