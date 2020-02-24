#pragma once

#include <iostream>
#include "he_handler.h"

#include <NTL/ZZ.h>
#include <NTL/BasicThreadPool.h>
#include "/Users/kchand/Desktop/HME/homomorphic-enc/third-party/HElib/src/FHE.h"
#include "/Users/kchand/Desktop/HME/homomorphic-enc/third-party/HElib/src/timing.h"
#include "/Users/kchand/Desktop/HME/homomorphic-enc/third-party/HElib/src/EncryptedArray.h"
#include <NTL/lzz_pXFactoring.h>
#include "he_controller.h"

class fhe_handler : he_handler
{
    public:
        fhe_handler(long m = 0, long p = 257, long r = 1, long L = 16, long c = 2, long w = 64, long d = 1, long k = 20, long s = 0);
        ~fhe_handler();

        void initialize();

        void encrypt_and_store(long x, int id);
        std::string encrypt_as_string(long x);

        int decrypt();
        int decrypt(std::string & ctxt);

        void aggregate(int count);
        std::string aggregate(std::vector<std::string> & input);
        void add(std::string & ctxt);

        int getSum();

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

        Ctxt * m_pPartSum;

        FHEcontext * m_pContext;
        FHESecKey * m_pPrivateKey;

        void initialize_m();
        void initialize_context();
        void initialize_polinomial();
        void generate_keys();

        Ctxt encrypt(long x);
        NTL::ZZX decrypt(Ctxt & ctxt);
};
