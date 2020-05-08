#include "../include/seal_he_handler.h"
#include <sstream>
#include <cassert>
#include "../include/base64.h"

using namespace std;

seal_he_handler::seal_he_handler(size_t poly_modulus_degree) :
	he_handler()
{
    m_poly_modulus_degree = poly_modulus_degree;

    m_pContext = 0;
}

seal_he_handler::~seal_he_handler() {
    // destructor

}


void seal_he_handler::initialize() {
    
    EncryptionParameters parms(scheme_type::BFV);
    parms.set_poly_modulus_degree(m_poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(m_poly_modulus_degree));
    parms.set_plain_modulus(1024);

    m_pContext = SEALContext::Create(parms);

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
    std::cout << "TEST: " + std::to_string(x) << std::endl; //TESTING
    PublicKey useKey;
    if ( pubKey.empty() ) {
        useKey = m_PublicKey;
        std::cout << "using existing key" << std::endl;
    } else {
        std::stringstream ss(base64_decode(pubKey));
        useKey.load(m_pContext, ss);
        std::cout << "using given key" << std::endl;
    }
    
    Encryptor encryptor(m_pContext, useKey);
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
    Decryptor decryptor(m_pContext, m_SecretKey);

    std::stringstream ss (base64_decode(ctxt));
    Ciphertext val;
    Plaintext plain;

    val.load(m_pContext, ss);

    decryptor.decrypt(val, plain);

    std::cout << "TEST TO STRING PLAIN: " << plain.to_string() << std::endl;

    return std::stoi(plain.to_string(), nullptr, 16);
}

void seal_he_handler::aggregate(int count)
{
	return;
}

std::string seal_he_handler::aggregate(std::vector<std::string> & input, const char* publickey) {
    /*
    std::cout << "he handler aggregate" << std::endl;
    PublicKey useKey;
    //std::cout << std::string("pk: ") + publickey << std::endl;
    if (std::string(publickey) == std::string("")) {
        std::cout << "using existing key" << std::endl;
        useKey = m_PublicKey;
    } else {
        std::cout << "using given key" << std::endl;
        std::stringstream ss(base64_decode(std::string(publickey)));
        useKey.load(m_pContext, ss);
    }
    Encryptor encryptor(m_pContext, useKey);
    Evaluator evaluator(m_pContext);
    */
    Encryptor encryptor(m_pContext, m_PublicKey);
    Evaluator evaluator(m_pContext);
    
    Plaintext zero_plain(to_string(0));
    
    Ciphertext sum;
    encryptor.encrypt(zero_plain, sum);
    
    for(auto it = input.begin(); it != input.end(); ++it) {
        std::stringstream ss (base64_decode(*it));
        Ciphertext val;
        val.load(m_pContext, ss);
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

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}

std::string seal_he_handler::getSecretKey() {
    std::stringstream ss;
    m_SecretKey.save(ss);

    const std::string& s = ss.str();   

    return base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
}


void seal_he_handler::generate_keys() {
    std::cout << "Generating keys... " << std::flush;

    KeyGenerator keygen(m_pContext);
    m_PublicKey = keygen.public_key();
    m_SecretKey = keygen.secret_key();

	std::cout << "OK!" << std::endl;
}

void seal_he_handler::setPublicKey(const char* json) {
    std::cout << "Setting public key... " << std::flush;
    
    std::stringstream ss(base64_decode(json));
    m_PublicKey.load(m_pContext, ss);
    
    std::cout << "OK!" << std::endl;
}


void seal_he_handler::setPrivateKey(const char* json) {
    std::cout << "Setting private key... " << std::flush;
    
    std::stringstream ss(base64_decode(json));
    m_SecretKey.load(m_pContext, ss);
    
    std::cout << "OK!" << std::endl;
}

template <class T>
string seal_he_handler::to_hexstring(T t, ios_base & (*f)(ios_base&))
{
  ostringstream oss;
  oss << f << t;
  return oss.str();
}
