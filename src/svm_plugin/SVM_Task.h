#pragma once
#include <seal/seal.h>
#include <QString>

class SVM_Task
{
public:

	SVM_Task(QString in_id);

	SVM_Task(seal::EncryptionParameters in_encrypton_parms,
		seal::RelinKeys  in_relin_keys,
		seal::GaloisKeys in_gal_keys,
		std::vector<seal::Ciphertext> in_C_li,
		std::vector<seal::Ciphertext> in_d_li,
		seal::Ciphertext in_long_x,
		seal::Ciphertext in_split_x,
		std::vector<seal::Ciphertext> in_g_li,
		std::vector<std::vector<seal::Ciphertext>> in_e_one_li,
		int in_local_rounds,
		int in_global_rounds,
		int in_dimension,
		QString in_id);
	
	QString id;
	QString client_address;
	bool full;
	int local_rounds;
	int global_rounds;
	int dimension;
	bool alg_parms_set;
	bool environment_set;

	std::shared_ptr<seal::SEALContext> working_context;
	bool working_context_set;
	seal::EncryptionParameters parms;
	bool parms_set;
	seal::RelinKeys relin_keys;
	bool relin_keys_set;
	seal::GaloisKeys gal_keys;
	bool gal_keys_set;
	std::vector<seal::Ciphertext> C_li;
	bool C_li_set;
	std::vector<seal::Ciphertext> d_li;
	bool d_li_set;
	std::vector<seal::Ciphertext> g_li;
	bool g_li_set;
	std::vector<std::vector<seal::Ciphertext>> e_one_li;
	bool e_one_li_set;
	seal::Ciphertext long_x;
	bool long_x_set;
	seal::Ciphertext split_x;
	bool split_x_set;

	void set_environment_variables(QString in_client_address);
	void set_alg_parms(int in_local_rounds, int in_global_rounds, int in_dimensions);
	void set_parms(QString& in_parms);
	void set_relin_keys(QString& in_relin);
	void set_gal_keys(QString& in_gal_keys);
	void set_C_li(std::vector<QString>& in_C_li);
	void set_d_li(std::vector<QString>& in_d_li);
	void set_g_li(std::vector<QString>& in_g_li);
	void set_e_one_li(std::vector<std::vector<QString>>& e_one_li);
	void set_long_x(QString& in_long_x);
	void set_split_x(QString& in_split_x);
	bool full_check();
	void print_task_status();
};

