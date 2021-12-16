//
// Created by finnm on 17.08.2021.
//

#ifndef ADSNEXTTRY_ADS_SET_H
#define ADSNEXTTRY_ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

template <typename Key, size_t N = 3>
class ADS_set {
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = key_type &;
    using const_reference = const key_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Iterator;
    using const_iterator = Iterator;
    using key_compare = std::less<key_type>;
    using key_equal = std::equal_to<key_type>;
    using hasher = std::hash<key_type>;

private:
    struct Bucket{
        key_type entries[N];
        size_type used_spaces;
        Bucket* next_bucket;
        Bucket() : entries{}, used_spaces{0}, next_bucket{nullptr} {}
        ~Bucket(){
            if(next_bucket != nullptr) delete next_bucket;
        }
    };
    Bucket** table;
    size_type table_size;
    size_type nextToSplit;
    size_type d_round;
    size_type curr_size;
    size_type usable_rows;

    size_type first_hash(const key_type &key) const{
        size_type hash = hasher{}(key)% (1ul<<d_round);
        if(hash < nextToSplit)
            return second_hash(key);
        return hash;
    }

    size_type second_hash(const key_type &key) const{
        return hasher{}(key) % (1ul<<(d_round+1));
    }

    void rehash(){
        if(table_size == usable_rows)
            expand();
        table[table_size] = new Bucket();
        ++table_size;
        Bucket* row = table[nextToSplit];
        std::vector<key_type> all;
        do{
            for(size_t i{0}; i < row->used_spaces; ++i){
                all.push_back(row->entries[i]);
            }
        }while((row = row->next_bucket) != nullptr);
        delete table[nextToSplit];
        table[nextToSplit] = new Bucket();
        ++nextToSplit;
        if(nextToSplit == (1ul<<d_round)){
            ++d_round;
            nextToSplit=0;
        }
        for(size_t i{0}; i < all.size(); ++i){
            insert_hash(all.at(i));
        }
    }
    void expand(){
        Bucket** expanded_table = new Bucket*[usable_rows *= 1.5];
        for(size_t i{0}; i < table_size; i++){
            expanded_table[i] = table[i];
        }
        delete[] table;
        table = expanded_table;
    }

    bool insert_hash(key_type &key) const{
        size_type pos = first_hash(key);
        Bucket* row = table[pos];
        while(row->next_bucket != nullptr){
            row = row->next_bucket;
        }
        if(row->used_spaces == N){
            row->next_bucket = new Bucket();
            row = row->next_bucket;
            row->entries[0] = key;
            ++row->used_spaces;
            return true;
        }
        row->entries[row->used_spaces] = key;
        ++row->used_spaces;
        return true;
    }

    bool insert_key(const key_type &key) {
        if(count(key)) return false;
        size_type pos = first_hash(key);
        Bucket* row = table[pos];
        while(row->next_bucket != nullptr){
            row = row->next_bucket;
        }
        if(row->used_spaces == N){
            row->next_bucket = new Bucket();
            row = row->next_bucket;
            row->entries[0] = key;
            ++row->used_spaces;
            rehash();
            ++curr_size;
            return true;
        }
        row->entries[row->used_spaces] = key;
        ++row->used_spaces;
        ++curr_size;
        return true;
    }

    std::pair<Bucket*, size_type> find_bucket_with_key2(const key_type &key) const{
        if(curr_size== 0) return std::make_pair(nullptr, 15);
        size_type pos = first_hash(key);
        Bucket* row = table[pos];
        do{
            for(size_type i{0}; i < row->used_spaces; ++i){
                if(key_equal{}(row->entries[i], key))
                    return std::make_pair(row, i);
            }
        }while((row = row->next_bucket) != nullptr);
        return std::make_pair(nullptr, -1);
    }


    Bucket* find_bucket_with_key(const key_type &key) const{
        if(curr_size== 0) return nullptr;
        size_type pos = first_hash(key);
        Bucket* row = table[pos];
        do{
            for(size_t i{0}; i < row->used_spaces; ++i){
                if(key_equal{}(row->entries[i], key))
                    return row;
            }
        }while((row = row->next_bucket) != nullptr);
        return nullptr;
    }
public:

    ADS_set() : table{}, table_size{2}, nextToSplit{0}, d_round{1}, curr_size{0}, usable_rows{16} {
        table = new Bucket*[usable_rows];
        table[0] = new Bucket();
        table[1] = new Bucket();
    }
                                                 
    ADS_set(std::initializer_list<key_type> ilist): ADS_set{std::begin(ilist), std::end(ilist)}   {}
    template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} {
        insert(first, last);
    }
    ADS_set(const ADS_set &other):ADS_set{} {
        *this = other;
    }

    ~ADS_set(){
        for(size_t i{0}; i < table_size; ++i){
            delete table[i];
        }
        delete[] table;
    }

    ADS_set &operator=(const ADS_set &other){
        if(this == &other) return *this;
        clear();
        for (auto i : other){
            insert_key(i);
        }
        return *this;
    }
    ADS_set &operator=(std::initializer_list<key_type> ilist){
        ADS_set temp(ilist);
        swap(temp);
        return *this;
    }

    size_type size() const{
        return curr_size;
    }
    bool empty() const{
        return curr_size==0;
    }

    void insert(std::initializer_list<key_type> ilist){
        insert(std::begin(ilist), std::end(ilist));
    }


    std::pair<iterator,bool> insert(const key_type &key){
        std::pair<Bucket*, size_type> elem = find_bucket_with_key2(key);
        if(elem.first){
            return std::make_pair(find(key), false);
        }
        insert_key(key);
        return std::make_pair(find(key), true);
    }


    template<typename InputIt> void insert(InputIt first, InputIt last){
        for(auto i{first}; i != last; ++i){
            insert_key(*i);
        }
    }

    void clear(){
        ADS_set leer;
        swap(leer);
    }
    size_type erase(const key_type &key){
        std::pair<Bucket*, size_type>lul = find_bucket_with_key2(key);
        Bucket* bucketkey = lul.first;
        if(!bucketkey) return 0;
        for(size_type i{lul.second+1}; i < bucketkey->used_spaces; ++i){
            bucketkey->entries[i-1] = bucketkey->entries[i];
        }
        --(bucketkey->used_spaces);
        --curr_size;
        return 1;
    }

    size_type count(const key_type &key) const{
        return !!find_bucket_with_key(key);
    }
    iterator find(const key_type &key) const{
        std::pair<Bucket*, size_type> elem_bucket = find_bucket_with_key2(key);
        if(!elem_bucket.first) return end();

        size_type pos = first_hash(key);
        Bucket* buck = elem_bucket.first;
        size_type n = elem_bucket.second;

        return Iterator(table_size, &key, pos, n, buck, table);
    }

    void swap(ADS_set &other){
        using std::swap;
        swap(table, other.table);
        swap(table_size, other.table_size);
        swap(curr_size, other.curr_size);
        swap(nextToSplit, other.nextToSplit);
        swap(usable_rows, other.usable_rows);
        swap(d_round, other.d_round);
    }

    const_iterator begin() const{
        if(curr_size==0) return end();
        size_type i{0};
        Bucket* currBucket = table[i];
        while(currBucket->used_spaces==0){
            if(currBucket->next_bucket != nullptr){
                currBucket = currBucket->next_bucket;
            }
            else{
                ++i;
                currBucket = table[i];
            }
        }
        key_type* k = &currBucket->entries[0];
        return Iterator(table_size, k, i, 0, currBucket, table);
    }
    const_iterator end() const{
        return Iterator(table, table_size);
    }

    void dump(std::ostream &o = std::cerr) const{
        o << curr_size;
    }

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs){         
        if(lhs.curr_size != rhs.curr_size) return false;
        for (size_t k{0}; k < lhs.table_size; k++) {
            Bucket *last_bucket = lhs.table[k];
            do {
                for (size_t i{0}; i < last_bucket->used_spaces; ++i) {
                        if(!rhs.find_bucket_with_key(last_bucket->entries[i]))
                            return false;
                    }
            } while ((last_bucket = last_bucket->next_bucket) != nullptr);
        }
        return true;
    }
    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs){
        if(lhs == rhs) return false;
        return true;
    }
};

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;
private:
    Bucket** table;
    const key_type* key;
    size_type row_in_table;
    size_type pos_in_bucket;
    size_type table_size;
    Bucket* currBucket;
public:

    explicit Iterator(){ //default
        table = nullptr;
        key = nullptr;
        table_size = 0;
        currBucket = nullptr;
    }

    explicit Iterator(Bucket** table, size_type table_size){ //end
        this->table = table;
        this->table_size = table_size;
        key = nullptr;
        currBucket = nullptr;
        row_in_table = table_size;
        pos_in_bucket = N;
    }

    explicit Iterator(size_type table_size, const key_type* key, size_type row_in_table, size_type pos_in_bucket, Bucket* currBucket, Bucket** table){ //iterator on something specific
        this->table = table;
        this->table_size = table_size;
        this->key = key;
        this->currBucket = currBucket;
        this->row_in_table = row_in_table;
        this->pos_in_bucket = pos_in_bucket;
    }

    reference operator*() const{
        return *key;
    }
    pointer operator->() const{
        return key;
    }
    Iterator &operator++(){
        if(currBucket == nullptr) return *this;
        ++pos_in_bucket;
        while(pos_in_bucket == currBucket->used_spaces){
            if(!currBucket->next_bucket){
                ++row_in_table;
                if(row_in_table == table_size){
                    key = nullptr;
                    currBucket = nullptr;
                    row_in_table = table_size;
                    pos_in_bucket = N;
                    return *this;
                }
                currBucket = table[row_in_table];
            }
            else{currBucket = currBucket->next_bucket;}
            pos_in_bucket = 0;
        }
        key = currBucket->entries+pos_in_bucket;
        return *this;
    }
    Iterator operator++(int){
        Iterator old(this->table_size, this->key, this->row_in_table, this->pos_in_bucket, this->currBucket, this->table);
        ++(*this);
        return old;
    }
    friend bool operator==(const Iterator &lhs, const Iterator &rhs){
        return (rhs.currBucket == lhs.currBucket && rhs.row_in_table == lhs.row_in_table && rhs.pos_in_bucket == lhs.pos_in_bucket);
    }
    friend bool operator!=(const Iterator &lhs, const Iterator &rhs){
        return !(lhs ==rhs);
    }
};

template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }
#endif //ADSNEXTTRY_ADS_SET_H
