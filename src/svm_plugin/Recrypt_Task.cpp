#include "Recrypt_Task.h"

Recrypt_Task::Recrypt_Task(std::thread::id in_worker_id, seal::Ciphertext in_fesh_text, seal::Ciphertext in_fresh_text_long)
{
	worker_id = in_worker_id;
	fresh_text_split = in_fesh_text;
	fesh_text_long = in_fresh_text_long;
};

Recrypt_Task::Recrypt_Task() {}