//
// Created by usagitoneko on 8/30/19.
//

#include <vector>
#include <sstream>
#include "parser.h"


void BookGetParser::construct(std::vector<std::string> commands) {
    // ignore the first command
    book_id = std::stoi(commands[1]);
    if (commands.size() >= 3) {
        book_format = commands[2];
    }
}

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
