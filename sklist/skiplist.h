#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <fstream>
#include <memory>

/*你会在.h文件中声明函数和类，而将它们的定义放置在一个单独的.cpp文件中。
但是在使用模板时，这种习惯性做法将变得不再有用，因为当实例化一个模板时，
编译器必须看到模板确切的定义，而不仅仅是它的声明。
因此，最好的办法就是将模板的声明和定义都放置在同一个.h文件中。
这就是为什么所有的STL头文件都包含模板定义的原因.
*/

#define STORE_FILE "store/dumpFile"

// mutex for critical section
std::mutex mtx;
std::string delimiter = ":";

template <typename K, typename V>
class Node
{
private:
    K key;

    V value;

public:
    Node(){};

    Node(K k, V v, int);

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);

    Node<K, V> **forward;

    int node_level;
};

//function of Node
//初始化
template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level)
{
    this->key = k;
    this->value = v;
    this->node_level = level;

    this->forward = new Node<K, V> *[level + 1]; //数组大小/ 树高level+1

    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
};

//TODO
template <typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
};

template <typename K, typename V>
K Node<K, V>::get_key() const
{
    return key;
};

template <typename K, typename V>
V Node<K, V>::get_value() const
{
    return value;
}

template <typename K, typename V>
void Node<K, V>::set_value(V value)
{
    this->value = value;
}

/**
 * @brief  skiplist
 * 
 * @tparam K-key
 * @tparam V -value
 * 
 * --------------------------------skiplist------------------------------------
 * level 4         1                                                       100
 *                 
 *                                       
 * level 3         1          10                    50            70       100
 *                                                    
 *                                                    
 * level 2         1          10         30         50            70       100
 *                                                    
 *                                                    
 * level 1         1    4     10         30         50           70        100
 *                                                     
 *                                                    
 * level 0         1    4  9  10         30   40   50    60      70        100
 * value:$         $    $  $   $          $    $    $    $        $         $ 
 * ----------------------------------------------------------------------------                                               
 **/

template <typename K, typename V>
class SkipList
{
public:
    SkipList(int);

    ~SkipList();

    int get_random_level();

    Node<K, V> *create_node(K, V, int);

    int insert_element(K, V);

    void display_list();

    bool search_element(K);

    void delete_element(K);

    void dump_file();

    void load_file();

    void clear_list();

    int size();

private:
    void get_key_value_from_string(const std::string &str, std::shared_ptr<int> key, std::string *value);
    bool is_valid_string(const std::string &str);

private:
    // Maximum level of the skip list
    int _max_level;

    // current level of skip list
    int _skip_list_level;

    // pointer to header node
    Node<K, V> *_header;

    // file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // skiplist current element count
    int _element_count;
};

//function of Skip list

template <typename K, typename V>
Node<K, V> *SkipList<K, V>::create_node(const K k, const V v, int level)
{
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
};

// Insert given key and value in skip list
// return 1 means element exists
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+
*/
template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K, V> *current = this->_header;

    // create update array and initialize it
    // update is array which put node that the node->forward[i] should be operated later

    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // int i = sizeof(Node<K, V>*) * (_max_level + 1); // 8*7 = 56
    //std::cout << "node size: " << i << std::endl;

    // start form highest level of skip list

    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
        // record node of every level to update
        update[i] = current;
    }
    // reached level 0 and forward pointer to right node, which is desired to insert key.
    current = current->forward[0];

    // if current node have key equal to searched key, we get it, and not insert
    if (current != nullptr && current->get_key() == key)
    {
        std::cout << "key: " << key << "exists" << std::endl;
        mtx.unlock();
        return -1;
    }

    // if current is nullptr that means we have reached to end of the level
    // if current's key is not equal to key that means we have to insert node between update[0] and current node
    if (current == nullptr || current->get_key() != key)
    {

        // Generate a random level for node
        int random_level = get_random_level();
        if (random_level > _skip_list_level)
        {
            for (int i = _skip_list_level + 1; i <= random_level; i++)
            {
                //将所有新的开辟层级的节点的头结点设置好
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // create new node with random level generated
        Node<K, V> *inserted_node = create_node(key, value, random_level);

        //insert node
        for (int i = 0; i <= random_level; i++)
        {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key: " << key << ", value: " << value << std::endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
};

// Display skip list
template <typename K, typename V>
void SkipList<K, V>::display_list()
{
    std::cout << "\n*****Skip List*****" << std::endl;
    if (_element_count == 0)
    {
        std::cout << " List is Null" << std::endl;
        return;
    }

    //自顶向下展示整个表
    for (int i = _skip_list_level; i >= 0; i--)
    {
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != nullptr)
        {
            std::cout << node->get_key() << ": " << node->get_value() << "; ";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
};

// Dump data in memory to file
template <typename K, typename V>
void SkipList<K, V>::dump_file()
{
    std::cout << "dump_file----------------" << std::endl;
    _file_writer.open(STORE_FILE);

    Node<K, V> *node = this->_header->forward[0];

    while (node != nullptr)
    {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ": " << node->get_value() << std::endl;
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return;
};

// Load data from disk / file

template <typename K, typename V>
void SkipList<K, V>::load_file()
{
    _file_reader.open(STORE_FILE);
    std::cout << "load_file---------------" << std::endl;
    std::string line;

    //TODO
    std::shared_ptr<int> key = std::make_shared<int>();

    std::string *value = new std::string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (value->empty())
        {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key: " << key << " ,value: " << *value << std::endl;
    }

    delete value;
    _file_reader.close();
}

// Get current SkipList size

template <typename K, typename V>
int SkipList<K, V>::size()
{
    return _element_count;
}

template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str)
{
    if (str.empty())
    {
        return false;
    }
    //TODO
    if (str.find(delimiter) == std::string::npos)
    {
        return false;
    }
    return true;
}

// Delete element from skip list

template <typename K, typename V>
void SkipList<K, V>::delete_element(K key)
{
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];

    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }

        update[i] = current;
    }

    current = current->forward[0];

    if (current != nullptr && current->get_key() == key)
    {
        // start for lowest level and delete the current node of each level
        /*  for (int i = 0; i <= _skip_list_level; i++) {
            // if at level i, next node is not target node, break the loop.
            if (update[i]->forward[i] != current)
                break;
            update[i]->forward[i] = current->forward[i];
        } */
        for (int i = current->node_level; i >= 0; i--)
        {
            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        //TODO
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr)
        {
            _skip_list_level--;
        }

        std::cout << "Successfully deleted key" << key << std::endl;
        delete current;
        _element_count--;
    }
    else
    {
        std::cout << "The key is not existed: " << key << std::endl;
    }
    mtx.unlock();
    return;
};

// Search for element in skip list
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/

template <typename K, typename V>
bool SkipList<K, V>::search_element(K key)
{
    std::cout << "search_element--------------" << std::endl;
    Node<K, V> *current = _header;

    // start from highest level of skip list

    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        {
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];

    if (current != nullptr && current->get_key() == key)
    {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
};

//clear the list

template <typename K, typename V>
void SkipList<K, V>::clear_list()
{
    Node<K, V> *current = _header->forward[0];

    while (current != nullptr)
    {
        Node<K, V> *tmp = current->forward[0];
        delete current;
        current = tmp;
    }

    //若不清空，_header->forward[0]会保存非法值，插入时调用_header->forward[0]->get_key出错
    memset(_header->forward, 0, sizeof(Node<K, V> *) * (_skip_list_level + 1));
    _skip_list_level = 0;
    _element_count = 0;
}

// construct skip list
template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level)
{
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // create header node and initialize key and value to nullptr

    K k{};
    V v{};
    this->_header = new Node<K, V>(k, v, _max_level);
};

//析构跳表
template <typename K, typename V>
SkipList<K, V>::~SkipList()
{
    if (_file_reader.is_open())
    {
        _file_writer.close();
    }
    if (_file_writer.is_open())
    {
        _file_writer.flush();
        _file_writer.close();
    }
    //删除所有的节点信息
    Node<K, V> *current = _header->forward[0];
    while (current)
    {
        Node<K, V> *tmp = current->forward[0];
        delete current;
        current = tmp;
    }
    delete _header;
};

//获得随机层级
template <typename K, typename V>
int SkipList<K, V>::get_random_level()
{
    //TODO 为什么从1开始而不是从0开始
    int random_level = 1;
    while (rand() % 2)
    {
        random_level++;
    }
    random_level = (random_level < _max_level) ? random_level : _max_level;
    return random_level;
};

template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::shared_ptr<int> key, std::string *value)
{

    if (!is_valid_string(str))
    {
        return;
    }
    *key = stoi(str.substr(0, str.find(delimiter)));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

#endif