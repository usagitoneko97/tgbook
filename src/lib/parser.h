//
// Created by usagitoneko on 8/30/19.
//

#ifndef TGBOOK_PARSER_H
#define TGBOOK_PARSER_H

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <boost/assign/list_of.hpp>
#include <string>
#include <memory>
#include <map>

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


class BookParser {
    /*
     * Format:
     *      /book <book_title_string>
     *      /book --isbn <isbn_int>
     * */
public:
    typedef enum Mode {TITLE, ISBN, ASIN} Mode;

    void construct(std::vector<std::string> commands) {
        // ignore the first command
        std::string content;
        for (std::string &c : std::vector<std::string>(commands.begin() + 1, commands.end())) {
            if (c.rfind("--", 0) == 0) {
                std::string book_method = c.substr(2);
                boost::algorithm::to_lower(book_method);
                mode = _mode_map[book_method];
            }
            else {
                content = fmt::format("{} {}", content, c);
            }
        }
        book_identifier = content;
    }
    const std::string &getBookIdentifier() const {
        return book_identifier;
    }

    Mode getMode() const {
        return mode;
    }

private:
    std::map<std::string, Mode> _mode_map = boost::assign::map_list_of("isbn", ISBN)("asin", ASIN);
    Mode mode = TITLE;
    std::string book_identifier;
};

class BookGetParser {
public:
    int getBookId() const {
        return book_id;
    }

    const std::string &getBookFormat() const {
        return book_format;
    }

    void construct(std::vector<std::string> commands) {
        // ignore the first command
        book_id = std::stoi(commands[1]);
        if (commands.size() >= 3) {
            book_format = commands[2];
        }
    }


private:
    int             book_id;
    std::string     book_format;
};

std::vector<std::string> parse(std::string& message) {
    // TODO: more sophisticated parsing method
    std::istringstream iss(message);
    std::string s;
    std::vector<std::string> commands;
    while (getline(iss, s, ' ')) {
        commands.push_back(s);
    }
    return commands;
}


#endif //TGBOOK_PARSER_H
