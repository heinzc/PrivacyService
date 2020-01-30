#pragma once

#include <iostream>
#include "he_handler.h"

#include <NTL/ZZ.h>
#include <NTL/BasicThreadPool.h>
#include "helib/helib.h"
#include "helib/timing.h"
#include <NTL/lzz_pXFactoring.h>
#include "he_controller.h"

using namespace helib;

class fhe_handler : he_handler
{
    public:
        fhe_handler(db_access * database, long m = 0, long p = 257, long r = 1, long L = 16, long c = 2, long w = 64, long d = 1, long k = 20, long s = 0);
        ~fhe_handler();

        void initialize();

        void encrypt_and_store(long x, int id);
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
        long m_m; // Specific modulus
        long m_p; // Plaintext base [default=2], should be a prime number
        long m_r; // Lifting [default=1]
        long m_L; // Number of levels in the modulus chain [default=heuristic]
        long m_c; // Number of columns in key-switching matrix [default=2]
        long m_w; // Hamming weight of secret key
        long m_d; // Degree of the field extension [default=1]
        long m_k; // Security parameter [default=80] 
        long m_s; // Minimum number of slots [default=0]

        Ctxt * m_pPartSum = 0;
        
        void setPublicKey(const char* json);
        void setPrivateKey(const char* json);

        FHEcontext * m_pContext = 0;
        FHESecKey * m_pPrivateKey = 0;

        void initialize_m();
        void initialize_context();
        void initialize_polinomial();
        void generate_keys();

        Ctxt encrypt(long x);
        NTL::ZZX decrypt(Ctxt & ctxt);
};
