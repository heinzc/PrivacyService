#pragma once

#include <iostream>
#include <vector>
#include "he_handler.h"

#include <gmpxx.h>    // gmp is included implicitly
#include <libhcs.h> // master header includes everything

#include "he_controller.h"

class phe_handler : he_handler
{
    public:
        phe_handler(db_access * database);
        ~phe_handler();

        void initialize();

        std::string encrypt_as_string(long x);

        int decrypt();
        int decrypt(std::string & ctxt);

        void aggregate(int count);
        std::string aggregate(std::vector<std::string> & input, const char* publickey);
        void add(std::string & ctxt, const char* publickey);

        int getSum();
        
        std::string getPublicKey();

    protected:

    private:
        pcs_public_key * pk = 0;
        pcs_private_key * sk = 0;
        hcs_random * hr = 0;

        mpz_t sum;
        
        void setPublicKey(const char* json);
        void setPrivateKey(const char* json);
};
