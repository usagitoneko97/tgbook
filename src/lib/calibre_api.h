//
// Created by usagitoneko on 8/29/19.
//

#ifndef TGBOOK_CALIBRE_API_H
#define TGBOOK_CALIBRE_API_H

#include "book.h"
#include <exception>
#include <memory>

class CalibreApi {
  public:
    explicit CalibreApi(const string &ip) : calibre_ip(ip) {}
    CalibreApi() = default;
    CalibreApi &update(const string &ip) {
        calibre_ip = ip;
        return *this;
    }

    std::vector<int> search(const string &title);
    std::unique_ptr<Book> locate_book(int book_id);
    std::unique_ptr<std::vector<Book>> locate_book(const std::string &title);
    string get_book_path(int book_id, const string &format);
    int get_info();

  private:
    string calibre_ip;
};

#endif // TGBOOK_CALIBRE_API_H
