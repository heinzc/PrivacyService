#include "../include/seal_handler.h"
#include <sstream>
#include <cassert>
#include "../include/base64.h"

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


void seal_he_handler::initialize() {
    
    m_pParms.set_poly_modulus_degree(m_poly_modulus_degree);
    m_pParms.set_coeff_modulus(CoeffModulus::Create(m_poly_modulus_degree, { 60, 40, 40, 60 }));
    double scale = pow(2.0, 40);
//    m_pParms.set_plain_modulus(8192); //change this, if your own encrypted values can be larger than this!

    m_pContext = new SEALContext(m_pParms);

    //print_parameters(m_pContext);

    generate_keys();
    
    //try to get saved keys from database. if there are none, use newly generated ones
    if(m_pController->getDB_access()->get_own_key("PK") == "" || m_pController->getDB_access()->get_own_key("SK") == "") { //own keys missing in the database
        //save above generated keys in database
        cout << "No own keys yet in database. Generated new keys." << endl;   
        std::string pk = getPublicKey();
        std::string sk = getSecretKey();
        m_pController->getDB_access()->insert_own_key("PK", pk);
        m_pController->getDB_access()->insert_own_key("SK", sk);
    } else { //set keys to the values saved in the database
        cout << "Found own keys in database. Use them." << endl;  
        setPublicKey(m_pController->getDB_access()->get_own_key("PK").c_str());
        setPrivateKey(m_pController->getDB_access()->get_own_key("SK").c_str());
    }
	
    cout << "*** END INITIALIZATION ***" << endl;
}

void seal_he_handler::encrypt_and_store(long x, int id) {
	return;
}

string seal_he_handler::encrypt_as_string(long x, std::string pubKey) {
    // for convenience
    using nljson = nlohmann::json;
    
    PublicKey useKey;
    SEALContext * useContext;
    
    if ( pubKey.empty() ) {
        useKey = m_PublicKey;
        useContext = m_pContext;
        std::cout << "using existing key" << std::endl;
    } else {
        std::string key, parms;
        try {
            nljson keyDict = nljson::parse(std::string(pubKey));
            key = keyDict["key"].get<std::string>();
            parms = keyDict["parms"].get<std::string>();
        }
        catch (...) {
            std::cout << "Problem with public key json format when encrypting." << std::endl;
            return std::string("");
        }    
        
        EncryptionParameters useParms = EncryptionParameters (scheme_type::ckks);
        std::stringstream ss2(base64_decode(parms));
        useParms.load(ss2);
        useContext = new SEALContext(useParms);
        
        std::stringstream ss(base64_decode(key));
        useKey.load(*useContext, ss);
        std::cout << "using given key" << std::endl;
    }
    
    Encryptor encryptor(*useContext, useKey);
    Plaintext x_plain(to_hexstring<long>(x, hex));

    Ciphertext x_encrypted;
    encryptor.encrypt(x_plain, x_encrypted);

    std::stringstream ss;
	x_encrypted.save(ss);

    const std::string& s = ss.str();

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

int seal_he_handler::decrypt() {
	return 0;
}

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

void seal_he_handler::aggregate(int count)
{
	return;
}

std::string seal_he_handler::aggregate(std::vector<std::string> & input, const char* publickey) {
    // for convenience
    using nljson = nlohmann::json;
    
    std::cout << "he handler aggregate" << std::endl;
    PublicKey useKey;
    SEALContext * useContext;
    
    //std::cout << std::string("pk: ") + publickey << std::endl;
    if (std::string(publickey) == std::string("")) {
        std::cout << "using existing key" << std::endl;
        useKey = m_PublicKey;
    } else {
        std::string key, parms;
        try {
            nljson keyDict = nljson::parse(std::string(publickey));
            key = keyDict["key"].get<std::string>();
            parms = keyDict["parms"].get<std::string>();
        }
        catch (...) {
            std::cout << "Problem with public key json format when encrypting." << std::endl;
            return std::string("");
        }    
        
        EncryptionParameters useParms = EncryptionParameters (scheme_type::ckks);
        std::stringstream ss2(base64_decode(parms));
        useParms.load(ss2);
        useContext = new SEALContext(useParms);
        
        std::stringstream ss(base64_decode(key));
        useKey.load(*useContext, ss);
        std::cout << "using given key" << std::endl;
    }
        
    Encryptor encryptor(*useContext, useKey);
    Evaluator evaluator(*useContext);
    
    //Encryptor encryptor(m_pContext, m_PublicKey);
    //Evaluator evaluator(m_pContext);
    
    Plaintext zero_plain(to_string(0));
    
    Ciphertext sum;
    encryptor.encrypt(zero_plain, sum);
    
    for(auto it = input.begin(); it != input.end(); ++it) {
        std::stringstream ss (base64_decode(*it));
        Ciphertext val;
        val.load(*useContext, ss);
        evaluator.add_inplace(sum, val);
    }
    std::stringstream ss;
	sum.save(ss);
    const std::string& s = ss.str();
    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

void seal_he_handler::add(std::string & ctxt, const char* publickey) {
	return;
}

int seal_he_handler::getSum() {
	
	return 0;
}

std::string seal_he_handler::getPublicKey() {
    std::stringstream ss;
    m_PublicKey.save(ss);

    const std::string& s = ss.str();   

    std::string key = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
    std::string parms = getEncryptionParameters();
    
    return std::string("{\"key\":\"") + key + std::string("\",\"parms\":\"") + parms + std::string("\"}");
}

std::string seal_he_handler::getSecretKey() {
    std::stringstream ss;
    m_SecretKey.save(ss);

    const std::string& s = ss.str();   

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

//returns the encryption parameters. is also shared, when sharing a public key.
std::string seal_he_handler::getEncryptionParameters() {
    std::stringstream ss;
    //m_pContext->last_context_data()->parms().save(ss);
    m_pParms.save(ss);

    const std::string& s = ss.str();   

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

void seal_he_handler::generate_keys() {
    std::cout << "Generating keys... " << std::flush;

    KeyGenerator keygen(*m_pContext);
    m_SecretKey = keygen.secret_key();
    keygen.create_public_key(m_PublicKey);


    keygen.create_relin_keys(m_relin_keys);
    keygen.create_galois_keys(m_gal_keys);

	std::cout << "OK!" << std::endl;
}

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

void seal_he_handler::setPrivateKey(const char* json) {
    std::cout << "Setting private key... " << std::flush;
    
    std::stringstream ss(base64_decode(json));
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
