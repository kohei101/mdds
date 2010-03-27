/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#include "segmenttree.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

#include <sys/time.h>

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}

template<typename key_type, typename value_type>
void build_and_dump(segment_tree<key_type, value_type>&db)
{
    cout << "build and dump (start) -----------------------------------------" << endl;
    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();
    cout << "build and dump (end) -------------------------------------------" << endl;
}

struct test_data
{
    string name; // data structure expects the data to have 'name' data member.

    test_data(const string& s) : name(s) {}
};

template<typename key_type, typename data_type>
bool check_leaf_nodes(
    const segment_tree<key_type, data_type>& db, 
    const key_type* keys, data_type** data_chain, size_t key_size)
{
    typedef segment_tree<key_type, data_type> st_type;
    vector<typename st_type::leaf_node_check> checks;
    checks.reserve(key_size);
    size_t dcid = 0;
    for (size_t i = 0; i < key_size; ++i)
    {    
        typename st_type::leaf_node_check c;
        c.key = keys[i];
        data_type* p = data_chain[dcid];
        while (p)
        {    
            c.data_chain.push_back(p);
            p = data_chain[++dcid];
        }
        checks.push_back(c);
        ++dcid;
    }

    return db.verify_keys(checks);
}

void st_test_insert_segments()
{
    typedef long key_type;
    typedef test_data data_type;

    StackPrinter __stack_printer__("::st_test_insert_segments");
    segment_tree<key_type, data_type> db;
    data_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    build_and_dump(db);
    assert(node_base::get_instance_count() == 0);

    db.insert(0, 10, &A);
    build_and_dump(db);
    {
        key_type keys[] = {0, 10};
        data_type* data_chain[] = {&A, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 3);
    }

    db.insert(0, 5, &B);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10};
        data_type* data_chain[] = {&A, &B, 0, &A, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 6);
    }

    db.insert(5, 12, &C);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10, 12};
        data_type* data_chain[] = {&A, &B, 0, &A, &C, 0, &C, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 7);
    }

    db.insert(10, 24, &D);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10, 12, 24};
        data_type* data_chain[] = {&A, &B, 0, &A, &C, 0, &C, &D, 0, &D, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 11);
    }

    db.insert(4, 24, &E);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24};
        data_type* data_chain[] = {&A, &B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 12);
    }

    db.insert(0, 26, &F);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24, 26};
        data_type* data_chain[] = {&A, &B, &F, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, 0, &F, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 14);
    }

    db.insert(12, 26, &G);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24, 26};
        data_type* data_chain[] = {&A, &B, &F, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, &G, 0, &F, &G, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 14);
    }
}

int main()
{
    st_test_insert_segments();
    assert(node_base::get_instance_count() == 0);
    fprintf(stdout, "Test finished successfully!\n");
    return EXIT_SUCCESS;
}

