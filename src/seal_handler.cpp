#include "seal_handler.h"
#include <sstream>
#include <cassert>
#include "base64.h"
#include <QtCore>

using namespace std;
using namespace seal;

seal_he_handler::seal_he_handler(size_t poly_modulus_degree) :
	he_handler()
{
    m_poly_modulus_degree = poly_modulus_degree;

    m_pContext = 0;
    
    m_pParms = EncryptionParameters (scheme_type::ckks);
}


seal_he_handler::~seal_he_handler() {
    // destructor

}

/**
 * @brief Initialize the Handler and generate or reuse Keys
*/
void seal_he_handler::initialize() {
    
    m_pParms.set_poly_modulus_degree(m_poly_modulus_degree);
    m_pParms.set_coeff_modulus(CoeffModulus::Create(m_poly_modulus_degree, { 60, 40, 40, 60 }));
    m_scale = pow(2.0, 40);
//    m_pParms.set_plain_modulus(8192); //change this, if your own encrypted values can be larger than this!

    m_pContext = new SEALContext(m_pParms);

    //print_parameters(m_pContext);
    
    //try to get saved keys from database. if there are none, use newly generated ones
    if(m_pController->getDB_access()->get_own_key("PK") == "" || m_pController->getDB_access()->get_own_key("SK") == "") { //own keys missing in the database
        //save above generated keys in database
        cout << "No own keys yet in database. Generated new keys." << endl;   
        generate_keys();
        m_pController->getDB_access()->insert_own_key("PK", getPublicKey());
        m_pController->getDB_access()->insert_own_key("SK", getSecretKey());
    } else { //set keys to the values saved in the database
        cout << "Found own keys in database. Use them." << endl;  
        setPublicKey(m_pController->getDB_access()->get_own_key("PK").c_str());
        setPrivateKey(m_pController->getDB_access()->get_own_key("SK").c_str());
    }
	
    cout << "*** END INITIALIZATION ***" << endl;
}

/**
 * @brief Encrypt the given value and return the Ciphertext
 * @param x is the value to encrypt
 * @param pubKey is the key to use for encryption. If empty use stored key
 * @return ciphertext as Qt::QString
*/
QString seal_he_handler::encrypt_as_QString(double x, string pubKey) {
    // for convenience
    using nljson = nlohmann::json;
    
    PublicKey useKey;
    SEALContext * useContext;
    
    if (pubKey.empty()) {
        std::cout << "using existing key" << std::endl;
        useKey = m_PublicKey;
        useContext = m_pContext;
    }
    else {
        std::string key, parms;
        try {
            nljson keyDict = nljson::parse(std::string(pubKey));
            key = keyDict["key"].get<std::string>();
            parms = keyDict["parms"].get<std::string>();
        }
        catch (...) {
            std::cout << "Problem with public key json format when encrypting." << std::endl;
            return QString("error");
        }    
        
        EncryptionParameters useParms = EncryptionParameters (scheme_type::ckks);
        std::stringstream ss2(base64_decode(parms));
        useParms.load(ss2);
        useContext = new SEALContext(useParms);
        
        std::stringstream ss(base64_decode(key));
        useKey.load(*useContext, ss);
        std::cout << "using given key" << std::endl;
    }
    
    CKKSEncoder encoder(*useContext);
    Plaintext x_plain;

    encoder.encode(x, m_scale, x_plain);

    Encryptor encryptor(*useContext, useKey);
    Ciphertext x_encrypted;
    encryptor.encrypt(x_plain, x_encrypted);

    std::stringstream ss;
	x_encrypted.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString result(b.toBase64());

    return result;
}

/*
int seal_he_handler::decrypt(std::string & ctxt) {
    Decryptor decryptor(*m_pContext, m_SecretKey);

    std::stringstream ss (base64_decode(ctxt));
    Ciphertext val;
    Plaintext plain;

    val.load(*m_pContext, ss);

    decryptor.decrypt(val, plain);

    //std::cout << "Test to string plain: " << plain.to_string() << std::endl;

    return std::stoi(plain.to_string(), nullptr, 16);
}
*/

/**
 * @brief Decrypts the given ciphertext
 * @param ctxt Ciphertext as Qt::QString. Base64 encoded String of the Binary Ciphertext
 * @return decrypted double value
*/
double seal_he_handler::decrypt(QString& ctxt) {
    std::stringstream ss ( QByteArray::fromBase64(ctxt.toUtf8()).toStdString() );

    Ciphertext val;
    val.load(*m_pContext, ss);

    Decryptor decryptor(*m_pContext, m_SecretKey);
    Plaintext plain;
    decryptor.decrypt(val, plain);

    CKKSEncoder encoder(*m_pContext);
    std::vector<double> result;
    encoder.decode(plain, result);

    return result.at(0);
}

/**
 * @brief Adds up the given List of Inputs and returns the Sum as Ciphertext
 * @param input List of Inputs 
 * @param publickey Publickey under which the Inputs are encrypted. If left empty, the local one is used.
 * @return Ciphertext of the Sum
*/
QString seal_he_handler::sum(QStringList& input, string pubKey) {
    std::cout << "he handler sum" << std::endl;
    PublicKey useKey;
    SEALContext * useContext;
    
    //std::cout << std::string("pk: ") + publickey << std::endl;
    if (pubKey.empty()) {
        std::cout << "using existing key" << std::endl;
        useKey = m_PublicKey;
        useContext = m_pContext;
    }
    else {
        std::string key, parms;
        try {
            nlohmann::json keyDict = nlohmann::json::parse(std::string(pubKey));
            key = keyDict["key"].get<std::string>();
            parms = keyDict["parms"].get<std::string>();
        }
        catch (...) {
            std::cout << "Problem with public key json format when encrypting." << std::endl;
            return QString("error");
        }

        EncryptionParameters useParms = EncryptionParameters(scheme_type::ckks);
        std::stringstream ss2(base64_decode(parms));
        useParms.load(ss2);
        useContext = new SEALContext(useParms);

        std::stringstream ss(base64_decode(key));
        useKey.load(*useContext, ss);
        std::cout << "using given key" << std::endl;
    }

    CKKSEncoder encoder(*useContext);
    Encryptor encryptor(*useContext, useKey);
    Evaluator evaluator(*useContext);

    Plaintext zero_plain;
    encoder.encode(0, m_scale, zero_plain);
    
    Ciphertext sum;
    encryptor.encrypt(zero_plain, sum);
    
    for(auto it = input.begin(); it != input.end(); ++it) {
        std::stringstream ss(QByteArray::fromBase64(it->toUtf8()).toStdString());

        Ciphertext val;
        val.load(*useContext, ss);
        evaluator.add_inplace(sum, val);
    }
    std::stringstream ss;
    sum.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString result(b.toBase64());

    return result;
}

/**
 * @brief Get the local Public Key and the Encryption Parameters. Can be retrieved via API or stored in the DB
 * @return JSON String containing Public Key and the Parameters
*/
std::string seal_he_handler::getPublicKey() {
    std::stringstream ss;
    m_PublicKey.save(ss);

    const std::string& s = ss.str();   

    std::string key = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
    std::string parms = getEncryptionParameters();
    
    return std::string("{\"key\":\"") + key + std::string("\",\"parms\":\"") + parms + std::string("\"}");
}

/**
 * @brief Get the local Secret Key. This is only used to stored it in the local DB and not accessible from outside.
 * @return The Secret Key
*/
std::string seal_he_handler::getSecretKey() {
    std::stringstream ss;
    m_SecretKey.save(ss);

    const std::string& s = ss.str();   

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

/**
 * @brief Get the used Encryption Parameters. This also shared, when sharing a public key.
 * @return the encryption parameters
*/
std::string seal_he_handler::getEncryptionParameters() {
    std::stringstream ss;
    //m_pContext->last_context_data()->parms().save(ss);
    m_pParms.save(ss);

    const std::string& s = ss.str();   

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

/**
 * @brief Setup Method to generate a new keypair.
*/
void seal_he_handler::generate_keys() {
    std::cout << "Generating keys... " << std::flush;

    KeyGenerator keygen(*m_pContext);
    m_SecretKey = keygen.secret_key();
    keygen.create_public_key(m_PublicKey);


    keygen.create_relin_keys(m_relin_keys);
    keygen.create_galois_keys(m_gal_keys);

	std::cout << "OK!" << std::endl;
}

/**
 * @brief Used when loading an existing Public Key and Encryption Paramters from the Database.
 * @param json String containing the pub key and the Encryption Paramters
*/
void seal_he_handler::setPublicKey(const char* json) {
    // for convenience
    using nljson = nlohmann::json;
    
    std::cout << "Setting public key... " << std::flush;
    
    std::string key, parms;
    
    try {
        nljson keyDict = nljson::parse(std::string(json));
        key = keyDict["key"].get<std::string>();
        parms = keyDict["parms"].get<std::string>();
        //std::cout << std::string("Key: ") + key << std::endl;
        //std::cout << std::string("Parms: ") + parms << std::endl;
    }
    catch (...) {
        std::cout << "Problem with public key json format." << std::endl;
        exit(1);
    }    
    
    std::stringstream ss(base64_decode(key));
    std::stringstream ss2(base64_decode(parms));
    //following can throw an exception, but is ok here, as it is called when the program begins.
    m_pParms.load(ss2);
    m_pContext = new SEALContext(m_pParms);
    m_PublicKey.load(*m_pContext, ss);
    
    std::cout << "OK!" << std::endl;
}

/**
 * @brief Used when loading existing Keys from the local Database.
 * @param sk Secret Key String 
*/
void seal_he_handler::setPrivateKey(const char* sk) {
    std::cout << "Setting private key... " << std::flush;
    
    std::stringstream ss(base64_decode(sk));
    m_SecretKey.load(*m_pContext, ss);
    
    std::cout << "OK!" << std::endl;
}


template <class T>
string seal_he_handler::to_hexstring(T t, ios_base & (*f)(ios_base&))
{
  ostringstream oss;
  oss << f << t;
  return oss.str();
}
