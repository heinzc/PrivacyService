#pragma once
#include "SVM_Handler.h"
#include "utility.h"



SVM_handler::SVM_handler(int worker_num) :
	QObject()
{
	utility::log("Handler Starting", verbose);
	std::cout << "Worker num: " << worker_num << std::endl;
	management_thread_running = true;
	max_worker_thread_amount = worker_num;
	running_worker_thread_amount = 0;
	m_pController = 0;
	utility::log("Handler Start up done", verbose);
}


void SVM_handler::add_task(std::shared_ptr<SVM_Task>task)
{
	const std::lock_guard<std::mutex> lock(task_list_lock);
	task_list.push_back(task);
	utility::log("Added task", verbose);
}

std::shared_ptr<SVM_Task> SVM_handler::remove_task()
{
	const std::lock_guard<std::mutex> lock(task_list_lock);
	std::shared_ptr<SVM_Task> task = task_list.front();
	task_list.pop_front();
	return task;
}


int SVM_handler::get_task_list_length()
{
	return task_list.size();
}


void SVM_handler::increment_thread_counter()
{
	const std::lock_guard<std::mutex> lock(thread_counter_lock);
	running_worker_thread_amount++;
	return;
}


void SVM_handler::decrement_thread_counter()
{
	const std::lock_guard<std::mutex> lock(thread_counter_lock);
	running_worker_thread_amount--;
	return;
}


int SVM_handler::get_current_worker_thread_amount()
{
	const std::lock_guard<std::mutex> lock(thread_counter_lock);
	return running_worker_thread_amount;
}


int SVM_handler::get_max_worker_thread_amount()
{
	return max_worker_thread_amount;
}


void SVM_handler::add_recypt_task(Recrypt_Task task)
{
	const std::lock_guard<std::mutex> lock(recrypt_map_lock);
	recrypt_tasks.insert(std::pair(task.worker_id, task));
}


bool SVM_handler::recrypt_task_done(std::thread::id worker_id)
{
	const std::lock_guard<std::mutex> lock(recrypt_map_lock);
	auto it = recrypt_tasks.find(worker_id);
	bool task_done = false;
	if(it != recrypt_tasks.end())
	{
		task_done = true;
	}
	return task_done;
}


Recrypt_Task SVM_handler::get_task(std::thread::id worker_id)
{
	const std::lock_guard<std::mutex> lock(recrypt_map_lock);
	auto it = recrypt_tasks.find(worker_id);

	auto pos = recrypt_tasks.find(worker_id);
	if (pos == recrypt_tasks.end()) {
		Recrypt_Task task;
		return task;
	}
	else {
		Recrypt_Task task = pos->second;
		recrypt_tasks.erase(pos);

		return task;
	}
}


void SVM_handler::add_done_recrypt_task(Recrypt_Task task)
{
	const std::lock_guard<std::mutex> lock(recrypt_map_lock);
	recrypt_tasks.insert(std::pair(task.worker_id, task));
}

bool SVM_handler::task_known(QString task_id)
{
	const std::lock_guard<std::mutex> lock(task_list_lock);
	bool res = false;
	for(int i=0;i<task_list.size();i++)
	{
		if(task_list[i]->id == task_id)
		{
			res = true;
			break;
		}
	}
	return res;
}

std::shared_ptr<SVM_Task> SVM_handler::get_svm_task(QString task_id)
{
	const std::lock_guard<std::mutex> lock(task_list_lock);
	std::shared_ptr<SVM_Task> res = task_list[0];
	for (int i = 0; i < task_list.size(); i++)
	{
		if (task_list[i]->id == task_id)
		{
			res = task_list[i];
			break;
		}
	}
	return res;
}

bool SVM_handler::frontaltaskcomplete()
{
	const std::lock_guard<std::mutex> lock(task_list_lock);
	bool res = false;
	res = task_list[0]->full;
	
	return res;
}

void SVM_handler::setController(he_controller* controller) {
	this->m_pController = controller;
	connect(this, SIGNAL(do_put_blocking(const QUrl&, const QJsonDocument&)), m_pController->getREST_handler(), SLOT(put_blocking(const QUrl&, const QJsonDocument&)));
}

#include "moc_svm_handler.cpp"