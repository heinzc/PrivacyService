#include "svm_plugin.h"

#include <QDebug>
#include "utility.h"
#include "tasks.h"

void svm_plugin::handle_incoming_svm_task(const QHttpServerRequest& request)
{
	QJsonDocument body_json = QJsonDocument::fromJson(request.body());
	QJsonObject bod_obj = body_json.object();
	QString id = body_json["task_id"].toString();
	// Check if we already have that task
	if (handler.task_known(id)) 
	{
		qDebug() << "task already known";
		shared_ptr<SVM_Task> task = handler.get_svm_task(id);
		handle_sub_key(bod_obj, task);
		task->print_task_status();
	}
	else
	{
		shared_ptr<SVM_Task> task = std::make_shared<SVM_Task>(SVM_Task(id));
		handler.add_task(task);
		handle_sub_key(bod_obj,task);
		task->print_task_status();
	}
	
}

void svm_plugin::handle_sub_key(QJsonObject& body,shared_ptr<SVM_Task> task)
{
	qDebug() << "Sub key handling";
	qDebug() << task->id;
	if(body.contains("parms"))
	{
		qDebug() << "parms case";
		QString parms_string = body["parms"].toString();
		qDebug() << parms_string;
		task->set_parms(parms_string);
	}
	else if(body.contains("local_rounds"))
	{
		qDebug() << "alg parms case";
		int local_rounds = body["local_rounds"].toInt();
		int global_rounds = body["global_rounds"].toInt();
		int dimension = body["dimension"].toInt();
		qDebug() << local_rounds;
		qDebug() << global_rounds;
		qDebug() << dimension;
		task->set_alg_parms(local_rounds, global_rounds, dimension);
	}
	else if(body.contains("relin_keys"))
	{
		qDebug() << "relin keys case";
		QString relin_keys_str = body["relin_keys"].toString();
		task->set_relin_keys(relin_keys_str);
	}
	else if (body.contains("gal_keys"))
	{
		qDebug() << "gal keys case";
		QString gal_keys_str = body["gal_keys"].toString();
		task->set_gal_keys(gal_keys_str);
	}
	else if (body.contains("c_li"))
	{
		qDebug() << "cli case";
		QJsonArray c_li_array = body["c_li"].toArray();
		std::vector<QString> c_li;
		for(int i=0;i<c_li_array.size();i++)
		{
			QString c_mat_str = c_li_array[i].toString();
			c_li.push_back(c_mat_str);
		}
		task->set_C_li(c_li);
	}
	else if (body.contains("d_li"))
	{	
		qDebug() << "dli case";
		QJsonArray d_li_array = body["d_li"].toArray();
		std::vector<QString> d_li;
		for (int i = 0; i < d_li_array.size(); i++)
		{
			QString d_vec_str = d_li_array[i].toString();
			d_li.push_back(d_vec_str);
		}
		task->set_d_li(d_li);
	}
	else if (body.contains("g_li"))
	{
		qDebug() << "gli case";
		QJsonArray g_li_array = body["g_li"].toArray();
		std::vector<QString> g_li;
		for (int i = 0; i < g_li_array.size(); i++)
		{
			QString g_vec_str = g_li_array[i].toString();
			g_li.push_back(g_vec_str);
		}
		task->set_g_li(g_li);
	}
	else if (body.contains("e_one_li"))
	{
		qDebug() << "e_one_li case";
		QJsonArray e_one_li_array = body["e_one_li"].toArray();
		std::vector<std::vector<QString>> e_one_li;
		for(int i=0;i< e_one_li_array.size();i++)
		{
			QJsonArray e_one_single_it = e_one_li_array[i].toArray();
			std::vector<QString> e_one_li_sit;
			for(int j=0;j<e_one_single_it.size();j++)
			{
				QString e_one_str = e_one_single_it[j].toString();
				e_one_li_sit.push_back(e_one_str);
			}
			e_one_li.push_back(e_one_li_sit);
		}
		task->set_e_one_li(e_one_li);
	}
	else if (body.contains("long_x"))
	{
		qDebug() << "longx case";
		QString long_x_str = body["long_x"].toString();
		task->set_long_x(long_x_str);
	}
	else if (body.contains("split_x"))
	{
		qDebug() << "splitx case";
		QString split_x_str = body["split_x"].toString();
		task->set_split_x(split_x_str);
	}
	else if (body.contains("client_address"))
	{
		qDebug() << "client address incoming case";
		QString caddress = body["client_address"].toString();
		task->set_environment_variables(caddress);
	}

}

void svm_plugin::handle_incoming_recrypt_task(const QHttpServerRequest& request)
{
	QJsonDocument body_json = QJsonDocument::fromJson(request.body());
	qDebug() << body_json;
}


svm_plugin::svm_plugin() :
	QObject(), PrivacyPluginInterface(),handler(1)
{
	handler_thread_ = std::thread(thread_tasks::management_task, std::ref(handler));
}
svm_plugin::~svm_plugin()
{
	handler_thread_.join();
}

void svm_plugin::initialize()
{
	handler.setController(this->m_pController);

	addRoute("/svm/testing", QHttpServerRequest::Method::GET, [=](const QHttpServerRequest& request) {
		return test();
		});
	qDebug() << "SVM Plugin initialize: " << "testing route added ";

	addRoute("/svm/svm_task", QHttpServerRequest::Method::POST, [=](const QHttpServerRequest& request) {
		handle_incoming_svm_task(request);
		return "ok";
		});
	qDebug() << "SVM Plugin initialize: " << "svm task endpoint route added";

	addRoute("/svm/recrypt_task", QHttpServerRequest::Method::POST, [=](const QHttpServerRequest& request) {
		handle_incoming_recrypt_task(request);
		return "ok";
		});
	qDebug() << "SVM Plugin initialize: " << "svm recrypt task endpoint route added";

	qDebug() << "Starting SVM task handler";
}



QString svm_plugin::test()
{
	qDebug() << "Hello there 2 electro bogaloo";

	QString mstring = "This is a test";
	return mstring;
}

#include "moc_svm_plugin.cpp"