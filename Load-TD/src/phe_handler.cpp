#include "../include/phe_handler.h"
#include "gmpxx.h"
#include <sstream>
#include<assert.h>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

phe_handler::phe_handler() :
	he_handler()
{
	pk = 0;
	sk = 0;
	hr = 0;
    pkey = 0;
    skey = 0;
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
   /* ifstream file1("pubkey.txt");
    ifstream file2("privkey.txt");
    //std::string pubk;
    //std::string seck;
      if(file1 && file2){
          getline(file1,pubk);
          const char *pk1 = pubk.c_str();
          cout <<"pubk= "<<pk1 <<endl;
          getline(file2,seck);
          const char *sk1 = seck.c_str();
          cout <<"seck= "<<sk1 <<endl;
          
          hr = hcs_init_random();
          pcs_generate_key_pair(pk, sk, hr, 2048);
          pcs_encrypt(pk, hr, sum, sum); 
          
          std::fstream pubKeyFile("pubkey.txt", std::fstream::in);
          std::fstream secKeyFile("privkey.txt", std::fstream::in);
          assert(pubKeyFile.is_open());
          assert(secKeyFile.is_open());
          std::string hexPubKey;
          std::string hexPrivKey;
          getline(pubKeyFile, hexPubKey);
          getline(secKeyFile, hexPrivKey);
          //cout << "pk from file= " <<hexPubKey<< endl;
          //cout << "sk from file=" <<hexPrivKey << endl;
          pkey = pcs_init_public_key();
          skey = pcs_init_private_key();
          hr = hcs_init_random();
          pk = pkey;
          sk = skey;
          cout << "pkey= " <<pk<< endl;
          cout << "skey=" <<sk << endl;
          // Generate a key pair with modulus of size 2048 bits
          pcs_generate_key_pair(pk, sk, hr, 2048);
          pcs_encrypt(pk, hr, sum, sum);
          pubKeyFile.close();
          secKeyFile.close();
      
      }
    else {*/
        
    // initialize data structures
        pk = pcs_init_public_key();
        sk = pcs_init_private_key();
        hr = hcs_init_random();
    // Generate a key pair with modulus of size 2048 bits
    pcs_generate_key_pair(pk, sk, hr, 2048);
    pcs_encrypt(pk, hr, sum, sum);
    
    cout << "pk= " <<pk<< endl;
    cout << "sk=" <<sk << endl;   // create file
    fstream secKeyFile("privkey.txt", fstream::out|fstream::trunc);
    fstream pubKeyFile("pubkey.txt", fstream::out|fstream::trunc);
    assert(secKeyFile.is_open());
    assert(pubKeyFile.is_open());
    secKeyFile << sk << std::endl; //write to a file
    pubKeyFile << pk << std::endl;
       /* pubkey = pcs_export_public_key(pk);
        seckey = pcs_export_private_key(sk);
        cout << "Pubkey exported = " <<pubkey<< endl;
        cout << "Seckey exported =" <<seckey << endl;
        pkey = pcs_import_public_key(pk, pubkey);
        skey = pcs_import_private_key(sk, seckey);
        cout << "Pubkey imported = " <<pkey<< endl;
        cout << "Seckey imported =" <<skey << endl; */
    cout << "*** END INITIALIZATION ***" << endl;
    //}
    
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

std::string phe_handler::aggregate(std::vector<std::string> & input) {
	mpz_t thissum;
	mpz_inits(thissum, NULL);
	mpz_set_ui(thissum, 0);
	pcs_encrypt(pk, hr, thissum, thissum);

	for (auto it = input.begin(); it != input.end(); ++it) {
			mpz_class value(*it);

			pcs_ee_add(pk, thissum, thissum, value.get_mpz_t());
	}

	mpz_class stringsum(thissum);
	return stringsum.get_str();
}


void phe_handler::add(std::string & ctxt) {
	mpz_class value(ctxt);

    pcs_ee_add(pk, sum, sum, value.get_mpz_t());    // Add encrypted values into partsum
	return;
}


int phe_handler::getSum() {

	return 0;
}
