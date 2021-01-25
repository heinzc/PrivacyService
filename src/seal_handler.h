#pragma once

#include <iostream>
#include "he_handler.h"

#include "he_controller.h"

#include "seal/seal.h"

#include <nlohmann/json.hpp>



// nur zum test!
//#include <algorithm>
//#include <chrono>
//#include <cstddef>
//#include <fstream>
//#include <iomanip>
#include <iostream>
//#include <limits>
//#include <memory>
//#include <mutex>
//#include <numeric>
//#include <random>
#include <sstream>
#include <string>
//#include <thread>
#include <vector>

using namespace seal;

class seal_he_handler : he_handler
{
    public:
        seal_he_handler(size_t poly_modulus_degree = 8192);
        ~seal_he_handler();

        void initialize();

        QString encrypt_as_QString(double x, std::string pubKey = std::string());

        //int decrypt(std::string & ctxt);
        double decrypt(QString& ctxt);

        QString sum(QStringList & input, std::string pubKey = std::string());

        std::string getPublicKey();

    protected:

    private:
        size_t m_poly_modulus_degree; // Specific modulus
        double m_scale;

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

/*
Helper function: Prints a vector of floating-point values.
*/
        template <typename T>
        inline void print_vector(std::vector<T> vec, std::size_t print_size = 4, int prec = 3)
        {
            /*
            Save the formatting information for std::cout.
            */
            std::ios old_fmt(nullptr);
            old_fmt.copyfmt(std::cout);

            std::size_t slot_count = vec.size();

            std::cout << std::fixed << std::setprecision(prec);
            std::cout << std::endl;
            if (slot_count <= 2 * print_size)
            {
                std::cout << "    [";
                for (std::size_t i = 0; i < slot_count; i++)
                {
                    std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
                }
            }
            else
            {
                vec.resize(std::max(vec.size(), 2 * print_size));
                std::cout << "    [";
                for (std::size_t i = 0; i < print_size; i++)
                {
                    std::cout << " " << vec[i] << ",";
                }
                if (vec.size() > 2 * print_size)
                {
                    std::cout << " ...,";
                }
                for (std::size_t i = slot_count - print_size; i < slot_count; i++)
                {
                    std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
                }
            }
            std::cout << std::endl;

            /*
            Restore the old std::cout formatting.
            */
            std::cout.copyfmt(old_fmt);
        }
};
