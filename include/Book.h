//
// Created by usagitoneko on 8/27/19.
//

#ifndef HELLO_WORLD_BOOK_H
#define HELLO_WORLD_BOOK_H


#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

using namespace std;

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
    string                      title;
    unsigned int                calibre_id;
    std::vector<string>         author, formats;
    std::map<string, string>    format_path;

public:
    unsigned int getCalibreId() const {
        return calibre_id;
    }

    const string &getTitle() const {
        return title;
    }

    const std::vector<string> &getAuthor() const {
        return author;
    }

    const std::vector<string> &getFormat() const {
        return formats;
    }
};



#endif //HELLO_WORLD_BOOK_H
