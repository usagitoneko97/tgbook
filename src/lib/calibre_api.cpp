//
// Created by usagitoneko on 8/29/19.
//

#include <exception>
#include <nlohmann/json.hpp>
#include <cpr/response.h>
#include <cpr/cpr.h>
#include "spdlog/spdlog.h"
#include "calibre_api.h"

std::vector<int> CalibreApi::search(const string &title) {
    spdlog::info("searching book title {} in calibre ip: {}", title, calibre_ip);
    string url = calibre_ip + "/ajax/search";
    cpr::Response r = cpr::Get(cpr::Url{url},
                               cpr::Parameters{{"query", title},
                                               {"sort_order", "desc"}});
    if (r.status_code == 200){
        try {
            nlohmann::json json = nlohmann::json::parse(r.text);
            std::vector<int> value = json.value("book_ids", std::vector<int>{-1});
            string book_ids_str = "";
            for (int id : value)
                book_ids_str += std::to_string(id) + ',';
            spdlog::info("found books in server :{}", book_ids_str);
            return value;
        }
        catch (std::exception& e) {
            spdlog::error("Couldn't process response for query {}", title);
            throw CalibreException();
        }
    }
    else if (r.status_code == 404) {
        spdlog::error("Couldn't get a response from {}. Please check calibre connection", calibre_ip);
        throw CalibreException();
    }
    else {
        spdlog::error("unknown response code");
        throw CalibreException();
    }
}


Book CalibreApi::locate_book(int book_id) {
    spdlog::info("trying to locate book: {}", book_id);
    std::map<string ,string> format_path;

    string url = calibre_ip + "/ajax/book/" + std::to_string(book_id);
    cpr::Response book_response = cpr::Get(cpr::Url{url});
    if (book_response.status_code == 200) {
        try {
            nlohmann::json book_json = nlohmann::json::parse(book_response.text);
            string title = book_json.value("title", "invalid_title");
            std::vector<string> formats = book_json.value("formats", std::vector<string>{"empty formats"});
            std::vector<string> authors = book_json.value("authors", std::vector<string>{"invalid author"});
            for (string& f : formats) {
                string p = book_json.at("format_metadata").at(f).at("path").get<string>();
                format_path[f] = p;
            }
            return Book(title, authors, book_id, formats, format_path);
        }
        catch (std::exception& e) {
            spdlog::error("Couldn't process response for book id {}", book_id);
            throw CalibreException();
        }
    }
    else if (book_response.status_code == 404) {
        spdlog::error("Book ids {} does not exist in calibre library: {]", book_id, calibre_ip);
        throw CalibreException();
    }
    else {
        spdlog::error("unknown response code while running {}", url);
        throw CalibreException();
    }
}

string CalibreApi::get_book_path(int book_id, const string& format) {
    if (format.length() == 0) {
        spdlog::error("Format unspecified.");
        throw CalibreException("Format unspecified.");
    }
    std::map<string ,string> format_path;
    string url = calibre_ip + "/ajax/book/" + std::to_string(book_id);
    spdlog::info("Getting response from {}", url);
    try{
        cpr::Response book_response = cpr::Get(cpr::Url{url});
        if (book_response.status_code == 200) {
            nlohmann::json book_json = nlohmann::json::parse(book_response.text);
            string p = book_json.at("format_metadata").at(format).at("path").get<string>();
            return p;
        }
        else {
            spdlog::error("unknown response code while running {}", url);
            throw CalibreException();
        }

    }
    catch (nlohmann::detail::parse_error& e) {
        spdlog::error("Error parsing the response. Is the server running? Is the url correct?");
    }
}
