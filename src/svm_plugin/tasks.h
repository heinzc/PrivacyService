#pragma once
#include <iostream>
#include <seal/seal.h>
#include "utility.h"
#include  "SVM_Task.h"
#include "SVM_Handler.h"
#include "../he_controller.h"
#include "../rest_handler.h"
#include <QObject>
#include <QJsonDocument>

namespace thread_tasks
{
    inline void recrypt(std::shared_ptr<SVM_Task> task, SVM_handler& handler, std::thread::id worker_id) {
        QJsonObject obj;
        std::stringstream ss;
        task->split_x.save(ss);
        QByteArray b = QByteArray::fromStdString(ss.str());
        QString split_x_str(b.toBase64());

        obj.insert("split_x", split_x_str);
        obj.insert("dimensions", task->dimension);

        stringstream worker_ss;
        worker_ss << worker_id;
        QString worker_id_qstr = QString::fromStdString(worker_ss.str());
        obj.insert("thread_id", worker_id_qstr);

        qDebug() << worker_id_qstr;

        // debugging
        QUrl newendpoint("http://127.0.0.1:34568");
        qDebug() << "Sending request to PUT " << newendpoint;
        QNetworkRequest req(newendpoint);
        //QJsonDocument jsondoc(obj);

        qDebug() << "emitting do put signal on handler ";
        //qDebug() << "emitting with payload: " << jsondoc.toJson();

        //emit(handler.do_put_blocking(task->client_address, QJsonDocument(obj)));
        emit(handler.do_put_blocking(newendpoint, QJsonDocument(obj)));
        //put_blocking(task->client_address, QJsonDocument(obj));
    }


    //-------------------------------------------------------------------------------------------------------------------------------
    inline void worker_task(std::shared_ptr<SVM_Task> task, SVM_handler& handler)
    {
        std::cout << "WORKER STARTING ID: " << std::this_thread::get_id() << std::endl;
        std::cout << task->global_rounds << std::endl;
        std::cout << task->local_rounds << std::endl;
        std::cout << task->dimension << std::endl;

        /* CORE ALGORITHM OF SVM CREATION */
        try
        {
            seal::Evaluator evaluator(*task->working_context);

            for (int i = 0; i < task->global_rounds; i++)
            {
                std::cout << "WORKER " << std::this_thread::get_id() << " starting global round " << i + 1 << std::endl;

                std::cout << "WORKER " << std::this_thread::get_id() << " Starting Global Iteration " << i + 1 << " With Following Vector: " << std::endl;
                //vicinity_interface.decrypt_print(task->split_x, task->dimension, std::this_thread::get_id());

                utility::gradientdecent_v4(task);


                /* Send out our recrypt task with our new model vector split_x */
                recrypt(task, handler, std::this_thread::get_id());

                //vicinity_interface.recrypt_task(std::this_thread::get_id(), task->split_x, task->dimension);
                /* Wait until our recrypt task is finished */

                while (!handler.recrypt_task_done(std::this_thread::get_id()))
                {
                    /* Wait a few mili seconds so not to much spam */
                    std::cout << "WORKER " << std::this_thread::get_id() << " waiting for recrypt task to be done" << std::endl;
                    std::this_thread::sleep_for(1000ms);
                }

                /* We now know that our recrypt task is done and in the handlers map */
                Recrypt_Task fresh = handler.get_task(std::this_thread::get_id());

                task->split_x = fresh.fresh_text_split;
                task->long_x = fresh.fesh_text_long;

                /* split_x ciphertext is now the highest level again and needs to be brought down*/


                evaluator.multiply(task->split_x, task->e_one_li[0][0], task->split_x);
                evaluator.relinearize_inplace(task->split_x, task->relin_keys);
                evaluator.rescale_to_next_inplace(task->split_x);



                evaluator.multiply(task->split_x, task->e_one_li[0][1], task->split_x);
                evaluator.relinearize_inplace(task->split_x, task->relin_keys);
                evaluator.rescale_to_next_inplace(task->split_x);

                std::cout << "WORKER " << std::this_thread::get_id() << " : Finished Global Iteration " << i + 1 << "With the following result: " << std::endl;


            }
        }
        catch(std::exception e)
        {
            std::cout << "some error wtf" << std::endl;
        }
        


        /* CORE ALGORITHM OF SVM CREATION END */
        handler.decrement_thread_counter();
        std::cout << "WORKER DONE ID: " << std::this_thread::get_id() << std::endl;
    }
    //-------------------------------------------------------------------------------------------------------------------------------
    
    

    //-------------------------------------------------------------------------------------------------------------------------------
    inline void management_task(SVM_handler& handler)
    {
        using namespace std::chrono_literals;
        while (handler.management_thread_running)
        {
            //std::cout << "Task list length: " << handler.get_task_list_length() << std::endl;
            //std::cout << "Current Worker thread amount: " << handler.get_current_worker_thread_amount() << std::endl;
            std::this_thread::sleep_for(2000ms);
            if (handler.get_task_list_length() > 0)
            {
                if (handler.get_current_worker_thread_amount() < handler.get_max_worker_thread_amount())
                {
                    if(handler.frontaltaskcomplete())
                    {
                        std::cout << "happ task is full" << std::endl;
                        std::shared_ptr<SVM_Task> new_task = handler.remove_task();
                        std::cout << "Starting Worker" << std::endl;
                        std::thread worker(worker_task, new_task, std::ref(handler));
                        worker.detach();
                        handler.increment_thread_counter();
                        std::cout << "Task removed and worker spawned" << std::endl;
                    }
                    else
                    {
                        std::cout << "tasks not yet full" << std::endl;
                    }
                    std::this_thread::sleep_for(5000ms);
                }
                else
                {
                    //std::cout << "Tasks to be done but all workers in use" << std::endl;
                }


            }
            else
            {
                //std::cout << "no new tasks" << std::endl;
            }
        }
    }
    //-------------------------------------------------------------------------------------------------------------------------------
}