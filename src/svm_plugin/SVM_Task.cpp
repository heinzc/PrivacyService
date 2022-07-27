#include "SVM_Task.h"
#include <QDebug>

SVM_Task::SVM_Task(seal::EncryptionParameters in_encrypton_parms,
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
    QString in_id)
{
    parms = in_encrypton_parms;
    relin_keys = in_relin_keys;
    gal_keys = in_gal_keys;
    C_li = in_C_li;
    d_li = in_d_li;
    long_x = in_long_x;
    split_x = in_split_x;
    g_li = in_g_li;
    local_rounds = in_local_rounds;
    global_rounds = in_global_rounds;
    e_one_li = in_e_one_li;
    dimension = in_dimension;
    working_context = std::make_shared<seal::SEALContext>(seal::SEALContext(parms));
    full = true;
    id = in_id;
}

SVM_Task::SVM_Task(QString in_id)
{
    id = in_id;
    working_context_set = false;
    parms_set = false;
    relin_keys_set = false;
    gal_keys_set = false;
    C_li_set = false;
    d_li_set = false;
    g_li_set = false;
    e_one_li_set = false;
    long_x_set = false;
    split_x_set = false;
    alg_parms_set = false;
    environment_set = false;
    full = full_check();

}

bool SVM_Task::full_check()
{
    bool concatinated = working_context_set && parms_set && relin_keys_set && gal_keys_set && C_li_set && d_li_set && g_li_set && e_one_li_set && long_x_set && split_x_set && alg_parms_set && environment_set;
    return concatinated;
}

void SVM_Task::set_alg_parms(int in_local_rounds, int in_global_rounds, int in_dimensions)
{
    local_rounds = in_local_rounds;
    global_rounds = in_global_rounds;
    dimension = in_dimensions;
    alg_parms_set = true;
    full = full_check();
    std::cout << "local rounds: " << local_rounds << std::endl;
    std::cout << "global rounds: " << global_rounds << std::endl;
    std::cout << "dimnsions: " << dimension << std::endl;
}

void SVM_Task::set_parms(QString& in_parms)
{
    qDebug() << "SVM_Task id: " << id << " parms loading";
    
    try
    {
        std::stringstream ss(QByteArray::fromBase64(in_parms.toUtf8()).toStdString());
        parms.load(ss);
       
        try
        {
            working_context = std::make_shared<seal::SEALContext>(parms);
            qDebug() << "SVM_Task id: " << id << " parms loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " working context loading failed due to irregular input" ;
            std::cout << e.what() << std::endl;
        }

        parms_set = true;
        working_context_set = true;
        full = full_check();
    }
    catch(std::exception e)
    {
        qDebug() << "SVM_Task id: " << id << " parms loading failed due to irregular input" ;
        std::cout<< e.what() << std::endl;
    }
    
};

void SVM_Task::set_relin_keys(QString& in_relin) 
{
    qDebug() << "SVM_Task id: " << id << " relin keys loading" ;
    if(working_context_set)
    {
        try
        {
            std::stringstream ss(QByteArray::fromBase64(in_relin.toUtf8()).toStdString());
            relin_keys.load(*working_context, ss);
            relin_keys_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " relin keys loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " relin keys loading failed due to irregular input" ;
            std::cout << e.what() << std::endl;
        }
    }
    else 
    {
        qDebug() << "SVM_Task id: " << id << " relin keys loading failed working context not initialized" ;
    }
};

void SVM_Task::set_gal_keys(QString& in_gal_keys) 
{
    qDebug() << "SVM_Task id: " << id << " gal keys loading" ;
    if(working_context_set)
    {
        try
        {
            std::stringstream ss(QByteArray::fromBase64(in_gal_keys.toUtf8()).toStdString());
            gal_keys.load(*working_context, ss);
            gal_keys_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " gal keys loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " gal keys loading failed due to irregular input" ;
            std::cout << e.what() << std::endl;

        }

        
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " gal keys loading failed due to working context not initialized";
    }
    
};

void SVM_Task::set_C_li(std::vector<QString>& in_C_li) 
{
    qDebug() << "SVM_Task id: " << id << " C_li loading" ;
    if(working_context_set)
    {
        try
        {
            for (int i = 0; i < in_C_li.size(); i++)
            {
                std::stringstream ss(QByteArray::fromBase64(in_C_li[i].toUtf8()).toStdString());
                seal::Ciphertext cipher;
                cipher.load(*working_context, ss);
                C_li.push_back(cipher);
            }
            C_li_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " C_li loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " C_li loading failed due to irregular input";
            std::cout << e.what() << std::endl;
        }    
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " C_li loading failed due to working context not initialized";
    }
};

void SVM_Task::set_d_li(std::vector<QString>& in_d_li) 
{
    qDebug() << "SVM_Task id: " << id << " d_li loading";
    if(working_context_set)
    {
        try
        {
            for (int i = 0; i < in_d_li.size(); i++)
            {
                std::stringstream ss(QByteArray::fromBase64(in_d_li[i].toUtf8()).toStdString());
                seal::Ciphertext cipher;
                cipher.load(*working_context, ss);
                d_li.push_back(cipher);
            }
            d_li_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " d_li loading done" ;
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " d_li loading failed due to irregular input" ;
            std::cout << e.what() << std::endl;
        }
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " d_li loading failed due to working context not initialized";
    }
};

void SVM_Task::set_g_li(std::vector<QString>& in_g_li) 
{
    qDebug() << "SVM_Task id: " << id << " g_li loading";
    if(working_context_set)
    {
        try
        {
            for (int i = 0; i < in_g_li.size(); i++)
            {
                std::stringstream ss(QByteArray::fromBase64(in_g_li[i].toUtf8()).toStdString());
                seal::Ciphertext cipher;
                cipher.load(*working_context, ss);
                g_li.push_back(cipher);
            }
            g_li_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " g_li loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " g_li loading failed due to irregular input";
            std::cout << e.what() << std::endl;
        }
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " g_li loading failed due to working context not initialized";
    }
};

void SVM_Task::set_e_one_li(std::vector<std::vector<QString>>& in_e_one_li) 
{
    qDebug() << "SVM_Task id: " << id << " e_one_li loading";
    if(working_context_set)
    {
        try
        {
            for (int i = 0; i < in_e_one_li.size(); i++)
            {
                std::vector<seal::Ciphertext> local_round_e_ones;
                for (int j = 0; j < in_e_one_li[i].size(); j++)
                {
                    std::stringstream ss(QByteArray::fromBase64(in_e_one_li[i][j].toUtf8()).toStdString());
                    seal::Ciphertext cipher;
                    cipher.load(*working_context, ss);
                    local_round_e_ones.push_back(cipher);
                }
                e_one_li.push_back(local_round_e_ones);
            }
            e_one_li_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " e_one_li loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " e_one_li loading failed due to irregular input";
            std::cout << e.what() << std::endl;
        }
        
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " e_one_li loading failed due to working context not initialized";
    }
};

void SVM_Task::set_long_x(QString& in_long_x) 
{
    qDebug() << "SVM_Task id: " << id << " long_x loading";
    if(working_context_set)
    {
        try
        {
            std::stringstream ss(QByteArray::fromBase64(in_long_x.toUtf8()).toStdString());
            long_x.load(*working_context, ss);
            long_x_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " long_x loading done";
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " long_x loading failed due to iregular input";
            std::cout << e.what() << std::endl;
        }
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " long_x loading failed due to working context not initialized";
    }
    
};

void SVM_Task::set_split_x(QString& in_split_x)
{
    qDebug() << "SVM_Task id: " << id << " split_x loading";
    if(working_context_set)
    {
        try
        {
            std::stringstream ss(QByteArray::fromBase64(in_split_x.toUtf8()).toStdString());
            split_x.load(*working_context, ss);
            split_x_set = true;
            full = full_check();
            qDebug() << "SVM_Task id: " << id << " split_x loading done" ;
        }
        catch(std::exception e)
        {
            qDebug() << "SVM_Task id: " << id << " split_x loading failed due to irregular input";
            std::cout << e.what() << std::endl;
        }
        
    }
    else
    {
        qDebug() << "SVM_Task id: " << id << " split_x loading failed due to working context not initialized";
    }
    
};

void SVM_Task::print_task_status()
{
    std::cout << "Task Status of Task: " << id.toStdString() << std::endl;
    std::cout << "alg_parms_set: " << alg_parms_set << std::endl;
    std::cout << "working_context_set: " << working_context_set << std::endl;
    std::cout << "parms_set: " << parms_set << std::endl;
    std::cout << "relin_keys_set: " << relin_keys_set << std::endl;
    std::cout << "gal_keys_set: " << gal_keys_set << std::endl;
    std::cout << "C_li_set: " << C_li_set << std::endl;
    std::cout << "d_li_set: " << d_li_set << std::endl;
    std::cout << "g_li_set: " << g_li_set << std::endl;
    std::cout << "e_one_li_set: " << e_one_li_set << std::endl;
    std::cout << "long_x_set: " << long_x_set << std::endl;
    std::cout << "split_x_set: " << split_x_set << std::endl;
    std::cout << "environmnt_set: " << environment_set << std::endl;
    std::cout << std::endl;

};

void SVM_Task::set_environment_variables(QString in_client_address)
{
    qDebug() << "clint address set";
    client_address = in_client_address;
    environment_set = true;
    full = full_check();
};
