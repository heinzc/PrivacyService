#include "../include/he_handler.h"

using namespace std;

he_handler::he_handler(long m, long p, long r, long L, long c, long w, long d, long k, long s)
{
    m_m = m;                   // Specific modulus
	m_p = p;                // Plaintext base [default=2], should be a prime number
	m_r = r;                   // Lifting [default=1]
	m_L = L;                  // Number of levels in the modulus chain [default=heuristic]
	m_c = c;                   // Number of columns in key-switching matrix [default=2]
	m_w = w;                  // Hamming weight of secret key
	m_d = d;                   // Degree of the field extension [default=1]
	m_k = k;                 // Security parameter [default=80] 
    m_s = s;                   // Minimum number of slots [default=0]

    m_pContext = 0;
    m_pPrivateKey = 0;
	m_pController = 0;
}

/* he_handler::he_handler(long m, long p, long r, long L, long c, long w, long d, long k, long s){
	long m = 0;                   // Specific modulus
	long p = 1021;257;                // Plaintext base [default=2], should be a prime number
	long r = 1;                   // Lifting [default=1]
	long L = 16;                  // Number of levels in the modulus chain [default=heuristic]
	long c = 3;                   // Number of columns in key-switching matrix [default=2]
	long w = 64;                  // Hamming weight of secret key
	long d = 0;                   // Degree of the field extension [default=1]
	long k = 128;                 // Security parameter [default=80] 
    long s = 0;                   // Minimum number of slots [default=0]
} */

he_handler::~he_handler() {
    // destructor

}

void he_handler::setController(he_controller * controller) {
	m_pController = controller;
}


void he_handler::initialize() {

    initialize_m();

    initialize_context();

    //initialize_polinomial();

    generate_keys();
	
    cout << "*** END INITIALIZATION ***" << endl;
}


Ctxt he_handler::encrypt(long x) {
	//const FHEPubKey& publicKey = *m_pPrivateKey;

	// Parameters needed to reconstruct the context
	unsigned long m, p, r;
	vector<long> gens, ords;
	
	fstream pubKeyFile("pubkey.txt", fstream::in);
	assert(pubKeyFile.is_open());
	
	// Initializes a context object with some parameters from the file
	readContextBase(pubKeyFile, m, p, r, gens, ords);
	FHEcontext context(m, p, r, gens, ords);
	
	// Reads the context itself
	pubKeyFile >> context;

	FHEPubKey publicKey(context);
	pubKeyFile >> publicKey;
	
	pubKeyFile.close();

	Ctxt ctxt(publicKey); // Initialize ciphertext using publicKey
	publicKey.Encrypt(ctxt, to_ZZX(x));

	return ctxt;
}

void he_handler::encrypt_and_store(long x, int id) {
	Ctxt ctxt = encrypt(x);

	std::fstream ciphertext("ciphertext" + to_string(id) + ".txt", fstream::out|fstream::trunc);
    assert(ciphertext.is_open());
    ciphertext << ctxt;
    ciphertext.close();
}

int he_handler::decrypt() {

	// Parameters needed to reconstruct the context
	unsigned long m, p, r;
	vector<long> gens, ords;
	
	fstream pubKeyFile("pubkey.txt", fstream::in);
	assert(pubKeyFile.is_open());
	
	// Initializes a context object with some parameters from the file
	readContextBase(pubKeyFile, m, p, r, gens, ords);
	FHEcontext context(m, p, r, gens, ords);
	
	// Reads the context itself
	pubKeyFile >> context;

	FHEPubKey publicKey(*m_pContext);
	pubKeyFile >> publicKey;
	
	pubKeyFile.close();
	
	fstream ciphertextFile("ciphertextsum.txt", fstream::in);
	assert(ciphertextFile.is_open());
	Ctxt ctsum = Ctxt(publicKey);
	ciphertextFile >> ctsum;
	ciphertextFile.close();

	ZZX ptsum = decrypt(ctsum);

	int sum;
	conv(sum, ptsum[0]);

	return sum;
}

ZZX he_handler::decrypt(Ctxt & ctxt) {
	ZZX ptxt;                            //	Create a plaintext to hold the plaintext of the sum
	// Parameters used to reconstruct the context
	unsigned long m, p, r;
	vector<long> gens, ords;
	
	// Read the context to reconstruct the secret key
	fstream secKeyFile("privkey.txt", fstream::in);
	readContextBase(secKeyFile, m, p, r, gens, ords);
	FHEcontext context(m, p, r, gens, ords);
	secKeyFile >> context;
	
	// Initializes the secret key object using the context and reads the key from the file
	FHESecKey secretKey(*m_pContext);
	secKeyFile >> secretKey;

	secretKey.Decrypt(ptxt, ctxt);
	//m_pPrivateKey->Decrypt(ptxt, *ctxt);

	return ptxt;
}

void he_handler::aggregate(int count)
 {
	// Parameters needed to reconstruct the context
	unsigned long m, p, r;
	vector<long> gens, ords;
	
	fstream pubKeyFile("pubkey.txt", fstream::in);
	assert(pubKeyFile.is_open());
	
	// Initializes a context object with some parameters from the file
	readContextBase(pubKeyFile, m, p, r, gens, ords);
	FHEcontext context(m, p, r, gens, ords);
	
	// Reads the context itself
	pubKeyFile >> context;

	FHEPubKey publicKey(*m_pContext);
	pubKeyFile >> publicKey;
	
	pubKeyFile.close();

	// Initializes a ciphertext object using the public key
	fstream ciphertextFile("ciphertext0.txt", fstream::in);
	assert(ciphertextFile.is_open());
	Ctxt ctsum = Ctxt(publicKey);
	ciphertextFile >> ctsum;
	ciphertextFile.close();

	for(int i = 1; i<count; ++i){
		// Initializes a ciphertext object using the public key
		fstream ciphertextFile2("ciphertext" + to_string(i) + ".txt", fstream::in);
		assert(ciphertextFile2.is_open());
		Ctxt ctxt2 = Ctxt(publicKey);
		ciphertextFile2 >> ctxt2;
		ciphertextFile2.close();

		ctsum += ctxt2;
	}

	std::fstream ciphertext("ciphertextsum.txt", fstream::out|fstream::trunc);
    assert(ciphertext.is_open());
    ciphertext << ctsum;
    ciphertext.close();
}


void he_handler::initialize_m() {

	std::cout << "Finding m... " << std::flush;
	m_m = FindM(m_k, m_L, m_c, m_p, m_d, m_s, 0); // Find a value for m given the specified values
	std::cout << "m = " << m_m << std::endl;
}

void he_handler::initialize_context() {

	std::cout << "Initializing context... " << std::flush;
	m_pContext = new FHEcontext(m_m, m_p, m_r); // Initialize context
	buildModChain(*m_pContext, m_L, m_c); // Modify the context, adding primes to the modulus chain
	std::cout << "OK!" << std::endl;
}

void he_handler::initialize_polinomial() {

	std::cout << "Creating polynomial... " << std::flush;
	ZZX G =  m_pContext->alMod.getFactorsOverZZ()[0]; // Creates the polynomial used to encrypt the data
	std::cout << "OK!" << std::endl;
}

void he_handler::generate_keys() {

    std::cout << "Generating keys... " << std::flush;

	// Files that will contain the public and secret keys
	fstream secKeyFile("privkey.txt", fstream::out|fstream::trunc);
	fstream pubKeyFile("pubkey.txt", fstream::out|fstream::trunc);
	assert(secKeyFile.is_open());
	assert(pubKeyFile.is_open());

	// Write the context to the files that will contain the keys
	// The context information is used to recreate the keys later
	writeContextBase(secKeyFile, *m_pContext);
	writeContextBase(pubKeyFile, *m_pContext);
	secKeyFile << *m_pContext << std::endl;
	pubKeyFile << *m_pContext << std::endl;

    m_pPrivateKey = new FHESecKey(* m_pContext); // Construct a secret key structure
    //m_pPublicKey = new FHEPubKey(* m_pPrivateKey); // An "upcast": FHESecKey is a subclass of FHEPubKey
	const FHEPubKey& publicKey = *m_pPrivateKey;
	m_pPrivateKey->GenSecKey(m_w); // Actually generate a secret key with Hamming weight w
	addSome1DMatrices(*m_pPrivateKey);
	
	// Writes both the secret and the public keys to files
	secKeyFile << *m_pPrivateKey << std::endl;
	pubKeyFile << publicKey << std::endl;

	secKeyFile.close();
	pubKeyFile.close();
	std::cout << "OK!" << std::endl;
}