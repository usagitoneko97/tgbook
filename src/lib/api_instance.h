//
// Created by usagitoneko on 9/17/19.
//

#ifndef TGBOOK_API_INSTANCE_H
#define TGBOOK_API_INSTANCE_H

#include "calibre_api.h"
#include "goodreads_api.h"

class ApiHub {
  public:
    // getOrInitInstance will also be used to initialize the api. Pass the constructor after the api.
    template <typename ApiRet>
    static ApiRet &getOrInitInstance() {
        static ApiRet api_ret = ApiRet();
        // Instantiated on first use.
        return api_ret;
    }

  private:
    ApiHub() = default; // Constructor? (the {} brackets) are needed here.

  public:
    ApiHub(ApiHub const &) = delete;
    void operator=(ApiHub const &) = delete;
};

#endif // TGBOOK_API_INSTANCE_H
