#include "../include/phe_handler.h"
#include "gmpxx.h"
#include <sstream>

using namespace std;

phe_handler::phe_handler() :
	he_handler()
{

}


phe_handler::~phe_handler() {
    // destructor... Cleanup all data
    pcs_free_public_key(pk);
    pcs_free_private_key(sk);
    hcs_free_random(hr);
}


void phe_handler::initialize() {
	// initialize data structures
    pk = pcs_init_public_key();
    sk = pcs_init_private_key();
    hr = hcs_init_random();

    // Generate a key pair with modulus of size 2048 bits
    pcs_generate_key_pair(pk, sk, hr, 2048);
    cout << "*** END INITIALIZATION ***" << endl;
}


string phe_handler::encrypt_as_string(long x) {
    // libhcs works directly with gmp mpz_t types, so initialize some
	mpz_t value;
	mpz_class ctxt;
	mpz_inits(value, NULL);
	mpz_set_ui(value, x);
	
	pcs_encrypt(pk, hr, value, value);

	ctxt = mpz_class(value);

	return ctxt.get_str();
}

int phe_handler::decrypt() {

	return 0;
}


int phe_handler::decrypt(std::string & ctxt) {

	return 0;
}

void phe_handler::aggregate(int count) {

	return;
}



void phe_handler::add(std::string & ctxt) {



    mpz_t a, b, c;
    mpz_inits(a, b, c, NULL);

    mpz_set_ui(a, 50);
    mpz_set_ui(b, 76);

    pcs_encrypt(pk, hr, a, a);  // Encrypt a (= 50) and store back into a
    pcs_encrypt(pk, hr, b, b);  // Encrypt b (= 76) and store back into b
    gmp_printf("a = %Zd\nb = %Zd\n", a, b); // can use all gmp functions still

    pcs_ee_add(pk, c, a, b);    // Add encrypted a and b values together into c
    pcs_decrypt(sk, c, c);      // Decrypt c back into c using private key
    gmp_printf("%Zd\n", c);     // output: c = 126

	return;
}

int phe_handler::getSum() {

	return 0;
}