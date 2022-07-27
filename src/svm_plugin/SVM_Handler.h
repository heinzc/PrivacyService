#pragma once
#include <seal/seal.h>
#include <deque>
#include "SVM_Task.h"
#include "utility.h"
#include "Recrypt_Task.h"
#include <Qstring>
#include <QJsonObject>
#include <QObject>

#include "../he_controller.h"
#include "../rest_handler.h"

using namespace utility;

class SVM_handler : public QObject
{
	Q_OBJECT

signals:
	void do_put_blocking(const QUrl& endpoint, const QJsonDocument& payload);


public:
	SVM_handler(int worker_num);
	void add_task(std::shared_ptr<SVM_Task>);
	std::shared_ptr<SVM_Task> remove_task();
	bool management_thread_running;
	int get_task_list_length();

	int get_current_worker_thread_amount();
	int get_max_worker_thread_amount();
	void increment_thread_counter();
	void decrement_thread_counter();

	void add_done_recrypt_task(Recrypt_Task task);
	bool recrypt_task_done(std::thread::id worker_id);
	Recrypt_Task get_task(std::thread::id worker_id);
	

	bool task_known(QString task_id);
	std::shared_ptr<SVM_Task> get_svm_task(QString task_id);
	bool frontaltaskcomplete();
	void setController(he_controller* controller);
	void add_recypt_task(Recrypt_Task task);
	he_controller* m_pController;

private:

	int max_worker_thread_amount;
	int running_worker_thread_amount;
	std::mutex thread_counter_lock;

	std::deque<std::shared_ptr<SVM_Task>> task_list;
	std::mutex task_list_lock;

	std::mutex recrypt_map_lock;
	std::map<std::thread::id, Recrypt_Task> recrypt_tasks;
	
	
};