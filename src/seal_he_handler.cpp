#include "../include/seal_he_handler.h"
#include <sstream>
#include <cassert>

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
	
    cout << "*** END INITIALIZATION ***" << endl;
}

void seal_he_handler::encrypt_and_store(long x, int id) {
	return;
}

string seal_he_handler::encrypt_as_string(long x, std::string pubKey) {
    PublicKey useKey;
    if ( pubKey.empty() ) {
        useKey = m_PublicKey;
        std::cout << "using existing key" << std::endl;
    } else {
        std::stringstream ss(pubKey);
        useKey.load(m_pContext, ss);
        std::cout << "using given key" << std::endl;
    }
    
    Encryptor encryptor(m_pContext, useKey);
    Plaintext x_plain(to_string(x));

    Ciphertext x_encrypted;
    encryptor.encrypt(x_plain, x_encrypted);

    std::stringstream ss;
	x_encrypted.save(ss);

    return ss.str();
}

int seal_he_handler::decrypt() {
	return 0;
}

int seal_he_handler::decrypt(std::string & ctxt) {
    Decryptor decryptor(m_pContext, m_SecretKey);

    std::stringstream ss (ctxt);
    Ciphertext val;
    Plaintext plain;

    val.load(m_pContext, ss);

    decryptor.decrypt(val, plain);

    return std::stoi(plain.to_string());
}

void seal_he_handler::aggregate(int count)
{
	return;
}

std::string seal_he_handler::aggregate(std::vector<std::string> & input, const char* publickey) {
    Encryptor encryptor(m_pContext, m_PublicKey);
    Evaluator evaluator(m_pContext);

    Plaintext zero_plain(to_string(0));
    Ciphertext sum;

    encryptor.encrypt(zero_plain, sum);

    for(auto it = input.begin(); it != input.end(); ++it) {
        std::stringstream ss (*it);
        Ciphertext val;

        val.load(m_pContext, ss);

        evaluator.add_inplace(sum, val);
    }

    std::stringstream ss;
	sum.save(ss);

    return ss.str();
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

    return ss.str();
}


void seal_he_handler::generate_keys() {

    std::cout << "Generating keys... " << std::flush;

    KeyGenerator keygen(m_pContext);
    m_PublicKey = keygen.public_key();
    m_SecretKey = keygen.secret_key();

	std::cout << "OK!" << std::endl;
}


void seal_he_handler::setPublicKey(const char* json) {

}


void seal_he_handler::setPrivateKey(const char* json) {

}
