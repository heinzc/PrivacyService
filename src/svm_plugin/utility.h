#pragma once
#include <seal/seal.h>
#include "SVM_Task.h"
#include <QtCore>
#include <QJsonObject>
#include "SVM_Handler.h"
#include<thread>

#include "SVM_Handler.h"
#include "Recrypt_Task.h"

namespace utility
{
    inline bool verbose = true;

    inline void log(std::string log_string, bool log_flag)
    {
        if (log_flag)
        {
            std::cout << "DEBUG: " << log_string << std::endl;
        }
    };

    //-------------------------------------------------------------------------------------------------------------------------------
    inline seal::Ciphertext encrypted_linear_transformation_singleciphertext(seal::Ciphertext matrix, seal::Ciphertext encrypted_vector, seal::Evaluator& evaluator, seal::RelinKeys relin_keys, seal::GaloisKeys gal_keys, int dimension)
    {
        seal::Ciphertext result;

        evaluator.multiply(matrix, encrypted_vector, result);
        evaluator.relinearize_inplace(result, relin_keys);
        evaluator.rescale_to_next_inplace(result);

        seal::Ciphertext rotater = result;

        for (int i = 0; i < dimension - 1; i++)
        {
            evaluator.rotate_vector_inplace(rotater, 1, gal_keys);
            evaluator.add(result, rotater, result);

        }

        return result;
    };
    //-------------------------------------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------------------------------------
    inline void gradientdecent_v4(std::shared_ptr<SVM_Task> task)
    {

        seal::Evaluator evaluator(*task->working_context);

        seal::Ciphertext bb;

        for (int i = 0; i < task->local_rounds; i++)
        {
            std::cout << "Local iteration: " << i << std::endl;

            seal::Ciphertext Cx = utility::encrypted_linear_transformation_singleciphertext(task->C_li[i], task->long_x, evaluator, task->relin_keys, task->gal_keys, task->dimension);// reuslt is lvl - 2

            evaluator.sub(Cx, task->d_li[i], bb);

            evaluator.multiply(bb, task->g_li[i], bb); // result is lvl -1
            evaluator.relinearize_inplace(bb, task->relin_keys);
            evaluator.rescale_to_next_inplace(bb);


            evaluator.sub(task->split_x, bb, task->split_x);

        }
    };
    //-------------------------------------------------------------------------------------------------------------------------------
    
    


}