/* ************************************************************************
> File Name:     main.cpp
> Author:        hengzhu
> Created Time:  2021/4/21
> Description:   
 ************************************************************************/
#include <iostream>
#include <string.h>
#include "./sklist/skiplist.h"
#define FILE_PATH "./store/dumpFile"
//using namespace std;
void print()
{
    std::cout << std::endl;
    std::cout << "********************************************************************************************************" << std::endl;
    std::cout << "* 1.insert  2.search  3.delete  4.display_list  5.List_size  6.dump_file  7.load_file  8.clear_list *" << std::endl;
    std::cout << "********************************************************************************************************" << std::endl;
    std::cout << "Please enter the number to start the operation:  ";
}

bool keyIsValid(const std::string &str)
{
    if (str.size() > 5)
        return false;
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "usage: ./main1 10" << std::endl;
        return 1;
    }
    SkipList<int, std::string> skipList(atoi(argv[1]));

    std::cout << "\n*****************   A tiny KV storage based on skiplist   ********************"
              << "\n";

    print();
    std::string flagstr = "";
    int flag = 0;
    while (std::cin >> flagstr)
    {
        if (flagstr == "exit")
        {
            break;
        }
        if (!keyIsValid(flagstr))
        {
            std::cout << "The enter is invalid,  please enter again:" << std::endl;
            continue;
        }
        flag = stoi(flagstr);
        std::cout << std::endl;
        switch (flag)
        {
        case 1:
        {
            int key;
            std::string keystr;
            std::string value;
            std::cout << "Please enter key:  ";

            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            std::cout << "Please enter value:  ";
            std::cin >> value;
            skipList.insert_element(key, value);
            break;
        }
        case 2:
        {
            int key;
            std::string keystr;
            std::cout << "Please enter search_key:  ";
            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            skipList.search_element(key);
            break;
        }
        case 3:
        {
            int key;
            std::string keystr;
            std::cout << "Please enter delete_key:  ";
            std::cin >> keystr;
            if (!keyIsValid(keystr))
            {
                std::cout << "The key is invalid,  please check again" << std::endl;
                break;
            }
            key = stoi(keystr);
            skipList.delete_element(key);
            break;
        }
        case 4:
        {
            skipList.display_list();
            break;
        }
        case 5:
        {
            std::cout << "skipList size:" << skipList.size() << std::endl;
            break;
        }
        case 6:
        {
            skipList.dump_file();
            break;
        }
        case 7:
        {
            skipList.load_file();
            break;
        }
        case 8:
        {
            skipList.clear_list();
            break;
        }
        case 888:
        {
            return 0;
        }
        default:
            std::cout << "your input is not right,  please input again" << std::endl;
            break;
        }
        print();
    }

    system("pause");
    return 0;
}
