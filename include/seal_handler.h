#pragma once

#include <iostream>
#include "he_handler.h"

#include "he_controller.h"

#include "seal/seal.h"

#include <nlohmann/json.hpp>

using namespace seal;

class seal_he_handler : he_handler
{
    public:
        seal_he_handler(size_t poly_modulus_degree = 8192);
        ~seal_he_handler();

        void initialize();

        void encrypt_and_store(long x, int id);
        std::string encrypt_as_string(long x, std::string pubKey = std::string());

        int decrypt();
        int decrypt(std::string & ctxt);

        void aggregate(int count);
        std::string aggregate(std::vector<std::string> & input, const char* publickey);
        void add(std::string & ctxt, const char* publickey);

        int getSum();

        std::string getPublicKey();

    protected:

    private:
        size_t m_poly_modulus_degree; // Specific modulus

        SEALContext * m_pContext = 0;
        
        EncryptionParameters m_pParms;

        PublicKey m_PublicKey;
        SecretKey m_SecretKey;

        RelinKeys m_relin_keys;
        GaloisKeys m_gal_keys;

        void generate_keys();

        template <class T>
        std::string to_hexstring(T t, ios_base & (*f)(ios_base&));

        //Ctxt encrypt(long x);
        //NTL::ZZX decrypt(Ctxt & ctxt);
        
        void setPublicKey(const char* json);
        void setPrivateKey(const char* json);
        
        std::string getSecretKey();
        std::string getEncryptionParameters();
};
