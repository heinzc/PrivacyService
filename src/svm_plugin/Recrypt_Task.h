#pragma once
#include<thread>
#include <seal/seal.h>


class Recrypt_Task
{
public:
	std::thread::id worker_id;
	seal::Ciphertext fresh_text_split;
	seal::Ciphertext fesh_text_long;
	Recrypt_Task(std::thread::id in_worker_id, seal::Ciphertext in_fesh_text, seal::Ciphertext in_fresh_text_long);
	Recrypt_Task();
private:
};