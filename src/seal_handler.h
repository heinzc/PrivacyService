#pragma once

#include "he_handler.h"

#include "he_controller.h"

#include "seal/seal.h"

#include <string>
#include <utility>

#include <QString>
#include <QJsonObject>


class seal_he_handler : he_handler
{
    public:
        seal_he_handler(size_t poly_modulus_degree = 16384);
        ~seal_he_handler();

        void initialize();

        QString encrypt_as_QString(double x, const QString& pkJson = QString());

        //int decrypt(std::string & ctxt);
        double decrypt(const QString& ctxt);

        QString recrypt(const QString& ctxt);

        void recrypt_for_svm(const QString& ctxt, int dimension, QJsonObject& retVal);

        QString sum(QStringList & input, const QString& pkJson = QString());

        QString getPublicKey();

    protected:

    private:
        size_t m_poly_modulus_degree; // Specific modulus
        double m_scale;

        seal::SEALContext * m_pContext = 0;
        
        seal::EncryptionParameters m_pParms;

        seal::PublicKey m_PublicKey;
        seal::SecretKey m_SecretKey;

        seal::RelinKeys m_relin_keys;
        seal::GaloisKeys m_gal_keys;

        void generate_keys();

        template <class T>
        std::string to_hexstring(T t, ios_base & (*f)(ios_base&));

        std::pair<seal::PublicKey, seal::EncryptionParameters> pubKeyParamsFromJson(const QString& pkJson);
        
        void setPublicKey(const QString& pk);
        void setPrivateKey(const QString& sk);
        
        QString getSecretKey();
        QString getEncryptionParameters();

        void StringToCipher(const QString& cipher, seal::Ciphertext& retVal, seal::SEALContext* useContext = 0);
        QString cipherToString(const seal::Ciphertext& cipher);

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


        // split vector for SVM algorithm
        inline std::vector<double> split_to_long(std::vector<double>& in, int dimension)
        {
            std::vector<double> vec;

            for (int i = 0; i < dimension; i++)
            {
                vec.push_back(in[i * dimension]);
            }

            std::vector<double> result;

            for (int i = 0; i < dimension; i++)
            {
                for (int j = 0; j < dimension; j++)
                {
                    result.push_back(vec[j]);
                }
            }
            return result;
        }
};
