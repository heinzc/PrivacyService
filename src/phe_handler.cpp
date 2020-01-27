#include "../include/phe_handler.h"
#include "gmpxx.h"
#include <sstream>

#include "../include/db_access.h"

using namespace std;

phe_handler::phe_handler() :
	he_handler()
{
	pk = 0;
	sk = 0;
	hr = 0;

	mpz_inits(sum, NULL);
	mpz_set_ui(sum, 0);
}


phe_handler::~phe_handler() {
    // destructor... Cleanup all data
    pcs_free_public_key(pk);
    pcs_free_private_key(sk);
    hcs_free_random(hr);
}


void phe_handler::initialize() {  
    db_access * db = new db_access("test.db");
    // initialize data structures
    hr = hcs_init_random();
    pk = pcs_init_public_key();
    sk = pcs_init_private_key();
    
    // Generate a key pair with modulus of size 2048 bits
    pcs_generate_key_pair(pk, sk, hr, 2048);
    pcs_encrypt(pk, hr, sum, sum);
    
    if(db->get_own_key("PK") == "" || db->get_own_key("SK") == "") { //own keys missing in the database
        //save above generated keys in database
        cout << "No own keys yet in database. Generated new keys." << endl;        
        db->insert_own_key("PK", pcs_export_public_key(pk));
        db->insert_own_key("SK", pcs_export_private_key(sk));
    } else { //set keys to the values saved in the database
        setPublicKey(db->get_own_key("PK").c_str());
        setPrivateKey(db->get_own_key("SK").c_str());
    }
    
    /*
    cout << "SK" << endl;
    cout << pcs_export_private_key(sk) << endl;
    cout << db->get_own_key("SK") << endl;
    cout << "PK" << endl;
    cout << pcs_export_public_key(pk) << endl;
    cout << db->get_own_key("PK") << endl;
    */
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
	mpz_t cleartxt;
	mpz_inits(cleartxt, NULL);
	pcs_decrypt(sk, cleartxt, sum);

	return mpz_get_ui(cleartxt);
}


int phe_handler::decrypt(std::string & ctxt) {
	mpz_class value(ctxt);

	mpz_t cleartxt;
	mpz_inits(cleartxt, NULL);
	pcs_decrypt(sk, cleartxt, value.get_mpz_t());

	return mpz_get_ui(cleartxt);
}


void phe_handler::aggregate(int count) {

	return;
}

std::string phe_handler::aggregate(std::vector<std::string> & input, const char* publickey) {
    pcs_public_key * passed_pk = pk;
    //pcs_import_public_key(passed_pk, publickey);
    
	mpz_t thissum;
	mpz_inits(thissum, NULL);
	mpz_set_ui(thissum, 0);
	pcs_encrypt(passed_pk, hr, thissum, thissum);

	for (auto it = input.begin(); it != input.end(); ++it) {
			mpz_class value(*it);

			pcs_ee_add(passed_pk, thissum, thissum, value.get_mpz_t());
	}

	mpz_class stringsum(thissum);
	return stringsum.get_str();
}


void phe_handler::add(std::string & ctxt, const char* publickey) {
	mpz_class value(ctxt);
    pcs_public_key * passed_pk = pk;
    //pcs_import_public_key(passed_pk, publickey);
    pcs_ee_add(passed_pk, sum, sum, value.get_mpz_t());    // Add encrypted values into partsum
	return;
}


int phe_handler::getSum() {

	return 0;
}

//pcs_public_key consits of parts n, g, n2
std::string phe_handler::getPublicKey() {
    return pcs_export_public_key(pk);
}

void phe_handler::setPublicKey(const char* json) {
    if(pcs_import_public_key(pk, json) == 0) {
        cout << "Error setting public key" << endl;
    }
    else cout << "Public key set" << endl;
}

void phe_handler::setPrivateKey(const char* json) {
    if(pcs_import_private_key(sk, json) == 0) {
        cout << "Error setting private key" << endl;
    }
    else cout << "Private key set" << endl;
}
