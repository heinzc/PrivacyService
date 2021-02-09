#include "seal_handler.h"

#include "db_access.h"

#include <iostream>
#include <sstream>
#include <cassert>

#include <QtCore>
#include <QJsonObject>

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
    m_pParms.set_coeff_modulus(CoeffModulus::Create(m_poly_modulus_degree, { 60, 60, 60, 60, 60, 60 }));
    m_scale = pow(2.0, 60);
//    m_pParms.set_plain_modulus(8192); //change this, if your own encrypted values can be larger than this!

    m_pContext = new SEALContext(m_pParms);

    //print_parameters(m_pContext);
    
    //try to get saved keys from database. if there are none, use newly generated ones
    QString pk = m_pController->getDB_access()->get_own_key("PK");
    QString sk = m_pController->getDB_access()->get_own_key("SK");
    if(pk.isEmpty() || sk.isEmpty()) { //own keys missing in the database
        //save above generated keys in database
        cout << "No own keys yet in database. Generated new keys." << endl;   
        generate_keys();
        m_pController->getDB_access()->insert_own_key("PK", getPublicKey());
        m_pController->getDB_access()->insert_own_key("SK", getSecretKey());
    } else { //set keys to the values saved in the database
        cout << "Found own keys in database. Use them." << endl;  
        setPublicKey(m_pController->getDB_access()->get_own_key("PK"));
        setPrivateKey(m_pController->getDB_access()->get_own_key("SK"));
    }
	
    cout << "*** END INITIALIZATION ***" << endl;
}

/**
 * @brief Encrypt the given value and return the Ciphertext
 * @param x is the value to encrypt
 * @param pubKey is the key to use for encryption. If empty use stored key
 * @return ciphertext as Qt::QString
*/
QString seal_he_handler::encrypt_as_QString(double x, const QString& pkJson) {
    PublicKey useKey;
    SEALContext * useContext;
    
    if (pkJson.isEmpty()) {
        useKey = m_PublicKey;
        useContext = m_pContext;

        qDebug() << "using existing key";
    }
    else {
        pair<PublicKey, EncryptionParameters> pubKeyParams = pubKeyParamsFromJson(pkJson);

        useKey = pubKeyParams.first;
        EncryptionParameters useParms = pubKeyParams.second;
        useContext = new SEALContext(useParms);

        qDebug() << "using given key";
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


/**
 * @brief Decrypts the given ciphertext
 * @param ctxt Ciphertext as Qt::QString. Base64 encoded String of the Binary Ciphertext
 * @return decrypted double value
*/
double seal_he_handler::decrypt(const QString& ctxt) {
    stringstream ss ( QByteArray::fromBase64(ctxt.toUtf8()).toStdString() );

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
 * @brief Recrypt the given ciphertext, refreshing its error term
 * @param ctxt input ciphertext with error
 * @return fresh encryption of the given ciphertext
*/
QString seal_he_handler::recrypt(const QString& ctxt) {
    CKKSEncoder encoder(*m_pContext);
    Encryptor encryptor(*m_pContext, m_PublicKey);
    Decryptor decryptor(*m_pContext, m_SecretKey);

    Ciphertext old_cipher;
    StringToCipher(ctxt, old_cipher, m_pContext);

    Plaintext old_plain;
    decryptor.decrypt(old_cipher, old_plain);

    vector<double> decoded;
    encoder.decode(old_plain, decoded);

    Plaintext return_plain;

    encoder.encode(decoded, m_scale, return_plain);

    Ciphertext return_encrypted;
    encryptor.encrypt(return_plain, return_encrypted);

    return cipherToString(return_encrypted);
}

/**
 * @brief Recrypt the given ciphertext and the given dimension for the SVM algorithm
 * @param ctxt input ciphertext with error, corresponding to the original split vector
 * @param dimension dimension for the split
 * @param retVal return jsonObject. Will contain "split_vector" and "long_vector" after call to this method
*/
void seal_he_handler::recrypt_for_svm(const QString& ctxt, int dimension, QJsonObject& retVal) {
    CKKSEncoder encoder(*m_pContext);
    Encryptor encryptor(*m_pContext, m_PublicKey);
    Decryptor decryptor(*m_pContext, m_SecretKey);
    
    Ciphertext old_cipher;
    StringToCipher(ctxt, old_cipher, m_pContext);

    Plaintext old_plain;
    decryptor.decrypt(old_cipher, old_plain);
        
    vector<double> decoded;
    encoder.decode(old_plain, decoded);

    // long vector
    vector<double> new_long = split_to_long(decoded, dimension);
    Plaintext p_new_long;
    Ciphertext e_new_long;
    encoder.encode(new_long, m_scale, p_new_long);
    encryptor.encrypt(p_new_long, e_new_long);

    // split vector
    Plaintext p_new_split;
    Ciphertext e_new_split;
    encoder.encode(decoded, m_scale, p_new_split);
    encryptor.encrypt(p_new_split, e_new_split);
    

    retVal.insert("vector_long", cipherToString(e_new_long));
    retVal.insert("vector_split", cipherToString(e_new_split));
}

/**
 * @brief Adds up the given List of Inputs and returns the Sum as Ciphertext
 * @param input List of Inputs 
 * @param publickey Publickey under which the Inputs are encrypted. If left empty, the local one is used.
 * @return Ciphertext of the Sum
*/
QString seal_he_handler::sum(QStringList& input, const QString& pkJson) {
    PublicKey useKey;
    SEALContext * useContext;
    
    //std::cout << std::string("pk: ") + publickey << std::endl;
    if (pkJson.isEmpty()) {
        useKey = m_PublicKey;
        useContext = m_pContext;

        qDebug() << "using existing key";
    }
    else {
        pair<PublicKey, EncryptionParameters> pubKeyParams = pubKeyParamsFromJson(pkJson);

        useKey = pubKeyParams.first;
        EncryptionParameters useParms = pubKeyParams.second;
        useContext = new SEALContext(useParms);

        qDebug() << "using given key";
    }

    CKKSEncoder encoder(*useContext);
    Encryptor encryptor(*useContext, useKey);
    Evaluator evaluator(*useContext);

    Plaintext zero_plain;
    encoder.encode(0, m_scale, zero_plain);
    
    Ciphertext sum;
    encryptor.encrypt(zero_plain, sum);
    
    for(auto it = input.begin(); it != input.end(); ++it) {
        stringstream ss(QByteArray::fromBase64(it->toUtf8()).toStdString());

        Ciphertext val;
        val.load(*useContext, ss);
        evaluator.add_inplace(sum, val);
    }

    stringstream ss;
    sum.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString result(b.toBase64());

    return result;
}


pair<PublicKey, EncryptionParameters> seal_he_handler::pubKeyParamsFromJson(const QString& pkJson) {
    QString key, params;

    try {
        QJsonDocument json = QJsonDocument::fromJson(pkJson.toUtf8());

        // check if parsing succeeded
        if (json.isNull()) {
            throw std::invalid_argument("JSON parse error");
        }
        if (!json.isObject()) {
            throw std::invalid_argument("invalid JSON provided");
        }
        if (!json.object().contains("key") || !json.object().contains("params")) {
            throw std::invalid_argument("JSON does not contain a value");
        }

        key = json.object().value("key").toString();
        params = json.object().value("params").toString();
    }
    catch (...) {
        qDebug() << "Problem with public key json format.";
    }

    stringstream ss(QByteArray::fromBase64(key.toUtf8()).toStdString());
    stringstream ss2(QByteArray::fromBase64(params.toUtf8()).toStdString());

    EncryptionParameters sealParams;
    sealParams.load(ss2);
    SEALContext context(sealParams);

    PublicKey sealKey;
    sealKey.load(context, ss);

    return make_pair(sealKey, sealParams);
}

/**
 * @brief Get the local Public Key and the Encryption Parameters. Can be retrieved via API or stored in the DB
 * @return JSON String containing Public Key and the Parameters
*/
QString seal_he_handler::getPublicKey() {
    stringstream ss;
    m_PublicKey.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString key(b.toBase64());

    QString params = getEncryptionParameters();

    QJsonObject keyJson;
    keyJson.insert("key", key);
    keyJson.insert("params", params);
    
    QJsonDocument returnDoc(keyJson);
    return returnDoc.toJson();
}

/**
 * @brief Get the local Secret Key. This is only used to stored it in the local DB and not accessible from outside.
 * @return The Secret Key
*/
QString seal_he_handler::getSecretKey() {
    std::stringstream ss;
    m_SecretKey.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString key(b.toBase64());

    return key;
}

/**
 * @brief Get the used Encryption Parameters. This also shared, when sharing a public key.
 * @return the encryption parameters
*/
QString seal_he_handler::getEncryptionParameters() {
    std::stringstream ss;
    //m_pContext->last_context_data()->parms().save(ss);
    m_pParms.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString params(b.toBase64());

    return params;
}


void seal_he_handler::StringToCipher(const QString& cipher, seal::Ciphertext& retVal, seal::SEALContext* useContext) {
    if (useContext == 0) {
        useContext = m_pContext;
    }

    stringstream ss(QByteArray::fromBase64(cipher.toUtf8()).toStdString());

    retVal.load(*useContext, ss);
}


QString seal_he_handler::cipherToString(const seal::Ciphertext& cipher) {
    stringstream ss;
    cipher.save(ss);

    QByteArray b = QByteArray::fromStdString(ss.str());
    QString result(b.toBase64());

    return result;
}



/**
 * @brief Setup Method to generate a new keypair.
*/
void seal_he_handler::generate_keys() {
    qDebug() << "Generating keys... ";

    KeyGenerator keygen(*m_pContext);
    m_SecretKey = keygen.secret_key();
    keygen.create_public_key(m_PublicKey);


    keygen.create_relin_keys(m_relin_keys);
    keygen.create_galois_keys(m_gal_keys);

	qDebug() << "OK!";
}

/**
 * @brief Used when loading an existing Public Key and Encryption Paramters from the Database.
 * @param json String containing the pub key and the Encryption Paramters
*/
void seal_he_handler::setPublicKey(const QString& pk) {
    qDebug() << "Setting public key... ";

    pair<PublicKey, EncryptionParameters> pubKeyParams = pubKeyParamsFromJson(pk);

    m_PublicKey = pubKeyParams.first;
    m_pParms = pubKeyParams.second;
    
    qDebug() << "OK!";
}

/**
 * @brief Used when loading existing Keys from the local Database.
 * @param sk Secret Key String 
*/
void seal_he_handler::setPrivateKey(const QString& sk) {
    qDebug() << "Setting private key... ";
    
    std::stringstream ss(QByteArray::fromBase64(sk.toUtf8()).toStdString());
    m_SecretKey.load(*m_pContext, ss);
    
    qDebug() << "OK!";
}


template <class T>
string seal_he_handler::to_hexstring(T t, ios_base & (*f)(ios_base&))
{
  ostringstream oss;
  oss << f << t;
  return oss.str();
}
