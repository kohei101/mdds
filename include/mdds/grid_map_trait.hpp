/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#ifndef __MDDS_GRID_MAP_TRAIT_HPP__
#define __MDDS_GRID_MAP_TRAIT_HPP__

#include "grid_map_types.hpp"

#include <vector>

namespace mdds { namespace gridmap {

struct numeric_cell_block : public base_cell_block, public std::vector<double>
{
public:
    numeric_cell_block() : base_cell_block(celltype_numeric), std::vector<double>(1) {}
    numeric_cell_block(size_t n) : base_cell_block(celltype_numeric), std::vector<double>(n) {}
};

struct string_cell_block : public base_cell_block, public std::vector<std::string>
{
public:
    string_cell_block() : base_cell_block(celltype_string), std::vector<std::string>(1) {}
    string_cell_block(size_t n) : base_cell_block(celltype_string), std::vector<std::string>(n) {}
};

struct index_cell_block : public base_cell_block, public std::vector<size_t>
{
public:
    index_cell_block() : base_cell_block(celltype_index), std::vector<size_t>(1) {}
    index_cell_block(size_t n) : base_cell_block(celltype_index), std::vector<size_t>(n) {}
};

struct boolean_cell_block : public base_cell_block, public std::vector<bool>
{
public:
    boolean_cell_block() : base_cell_block(celltype_boolean), std::vector<bool>(1) {}
    boolean_cell_block(size_t n) : base_cell_block(celltype_boolean), std::vector<bool>(n) {}
};

numeric_cell_block* get_numeric_block(base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<numeric_cell_block*>(block);
}

const numeric_cell_block* get_numeric_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<const numeric_cell_block*>(block);
}

string_cell_block* get_string_block(base_cell_block* block)
{
    if (!block || block->type != celltype_string)
        throw general_error("block is not of string type!");

    return static_cast<string_cell_block*>(block);
}

const string_cell_block* get_string_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_string)
        throw general_error("block is not of string type!");

    return static_cast<const string_cell_block*>(block);
}

index_cell_block* get_index_block(base_cell_block* block)
{
    if (!block || block->type != celltype_index)
        throw general_error("block is not of index type!");

    return static_cast<index_cell_block*>(block);
}

const index_cell_block* get_index_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_index)
        throw general_error("block is not of index type!");

    return static_cast<const index_cell_block*>(block);
}

boolean_cell_block* get_boolean_block(base_cell_block* block)
{
    if (!block || block->type != celltype_boolean)
        throw general_error("block is not of boolean type!");

    return static_cast<boolean_cell_block*>(block);
}

const boolean_cell_block* get_boolean_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_boolean)
        throw general_error("block is not of boolean type!");

    return static_cast<const boolean_cell_block*>(block);
}

cell_t get_cell_type(double)
{
    return celltype_numeric;
}

cell_t get_cell_type(const std::string&)
{
    return celltype_string;
}

cell_t get_cell_type(size_t)
{
    return celltype_index;
}

cell_t get_cell_type(bool)
{
    return celltype_boolean;
}

void set_value(base_cell_block* block, size_t pos, double val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk[pos] = val;
}

void set_value(base_cell_block* block, size_t pos, const std::string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk[pos] = val;
}

void set_value(base_cell_block* block, size_t pos, size_t val)
{
    index_cell_block& blk = *get_index_block(block);
    blk[pos] = val;
}

void set_value(base_cell_block* block, size_t pos, bool val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk[pos] = val;
}

void get_value(base_cell_block* block, size_t pos, double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    val = blk[pos];
}

void get_value(base_cell_block* block, size_t pos, std::string& val)
{
    string_cell_block& blk = *get_string_block(block);
    val = blk[pos];
}

void get_value(base_cell_block* block, size_t pos, size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    val = blk[pos];
}

void get_value(base_cell_block* block, size_t pos, bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    val = blk[pos];
}

struct cell_block_func_base
{
    static cell_t get_block_type(const base_cell_block& block);

    static base_cell_block* create_new_block(cell_t type, size_t init_size);

    static base_cell_block* clone_block(base_cell_block* p);

    static void delete_block(base_cell_block* p);

    static void resize_block(base_cell_block* p, size_t new_size);

    static void print_block(base_cell_block* p);

    static void erase(base_cell_block* block, size_t pos);

    static void erase(base_cell_block* block, size_t pos, size_t size);

    template<typename T>
    static void set_values(base_cell_block* block, size_t pos, const T& it_begin, const T& it_end);

    template<typename T>
    static void prepend_value(base_cell_block* block, const T& val);

    template<typename T>
    static void prepend_values(base_cell_block* block, const T& it_begin, const T& it_end);

    template<typename T>
    static void append_value(base_cell_block* block, const T& val);

    static void append_values(base_cell_block* dest, const base_cell_block* src);

    template<typename T>
    static void append_values(base_cell_block* block, const T& it_begin, const T& it_end);

    static void append_values(base_cell_block* dest, const base_cell_block* src,
                              size_t begin_pos, size_t len);

    static void assign_values(base_cell_block* dest, const base_cell_block* src,
                              size_t begin_pos, size_t len);

    template<typename T>
    static void insert_values(base_cell_block* block, size_t pos, const T& it_begin, const T& it_end);

    template<typename T>
    static void assign_values(base_cell_block* dest, const T& it_begin, const T& it_end);

    template<typename T>
    static void get_value(base_cell_block* block, size_t pos, T& val);

    template<typename T>
    static void get_empty_value(T& val);

    static bool equal_block(const base_cell_block* left, const base_cell_block* right);
};

cell_t cell_block_func_base::get_block_type(const base_cell_block& block)
{
    return block.type;
}

base_cell_block* cell_block_func_base::create_new_block(cell_t type, size_t init_size)
{
    switch (type)
    {
        case celltype_numeric:
            return new numeric_cell_block(init_size);
        case celltype_string:
            return new string_cell_block(init_size);
        case celltype_index:
            return new index_cell_block(init_size);
        case celltype_boolean:
            return new boolean_cell_block(init_size);
        default:
            throw general_error("create_new_block: failed to create a new block of unknown type.");
    }
    return NULL;
}

base_cell_block* cell_block_func_base::clone_block(base_cell_block* p)
{
    if (!p)
        return NULL;

    switch (p->type)
    {
        case celltype_numeric:
            return new numeric_cell_block(*get_numeric_block(p));
        case celltype_string:
            return new string_cell_block(*get_string_block(p));
        case celltype_index:
            return new index_cell_block(*get_index_block(p));
        case celltype_boolean:
            return new boolean_cell_block(*get_boolean_block(p));
        default:
            throw general_error("clone_block: failed to clone a block of unknown type.");
    }
    return NULL;
}

void cell_block_func_base::delete_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
            delete static_cast<numeric_cell_block*>(p);
        break;
        case celltype_string:
            delete static_cast<string_cell_block*>(p);
        break;
        case celltype_index:
            delete static_cast<index_cell_block*>(p);
        break;
        case celltype_boolean:
            delete static_cast<boolean_cell_block*>(p);
        break;
        default:
            throw general_error("delete_block: failed to delete a block of unknown type.");
    }
}

void cell_block_func_base::resize_block(base_cell_block* p, size_t new_size)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
            static_cast<numeric_cell_block*>(p)->resize(new_size);
        break;
        case celltype_string:
            static_cast<string_cell_block*>(p)->resize(new_size);
        break;
        case celltype_index:
            static_cast<index_cell_block*>(p)->resize(new_size);
        break;
        case celltype_boolean:
            static_cast<boolean_cell_block*>(p)->resize(new_size);
        break;
        default:
            throw general_error("resize_block: failed to resize a block of unknown type.");
    }
}

template<typename T>
struct print_block_array
{
    void operator() (const T& val) const
    {
        std::cout << val << " ";
    }
};

void cell_block_func_base::print_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<double>());
            std::cout << std::endl;
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<std::string>());
            std::cout << std::endl;
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<size_t>());
            std::cout << std::endl;
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& blk = *static_cast<boolean_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<bool>());
            std::cout << std::endl;
        }
        break;
        default:
            throw general_error("print_block: failed to print a block of unknown type.");
    }
}

void cell_block_func_base::erase(base_cell_block* block, size_t pos)
{
    if (!block)
        return;

    switch (block->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& blk = *static_cast<boolean_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        default:
            throw general_error("erase: failed to erase an element from a block of unknown type.");
    }
}

void cell_block_func_base::erase(base_cell_block* block, size_t pos, size_t size)
{
    if (!block)
        return;

    switch (block->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& blk = *static_cast<boolean_cell_block*>(block);
            blk.erase(blk.begin()+pos, blk.begin()+pos+size);
        }
        break;
        default:
            throw general_error("erase: failed to erase elements from a block of unknown type.");
    }
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, const typename _Iter::value_type&,
    const _Iter& it_begin, const _Iter& it_end)
{
    throw general_error("non-specialized version of _set_values called.");
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void _set_values(
    base_cell_block* block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename T>
void cell_block_func_base::set_values(base_cell_block* block, size_t pos, const T& it_begin, const T& it_end)
{
    if (!block)
        throw general_error("destination cell block is NULL.");

    if (it_begin == it_end)
        // Nothing to do.
        return;

    _set_values(block, pos, *it_begin, it_begin, it_end);
}

template<typename T>
void cell_block_func_base::prepend_value(base_cell_block* block, const T& val)
{
    throw general_error("non-specialized version of prepend_value called.");
}

template<>
void cell_block_func_base::prepend_value<double>(base_cell_block* block, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func_base::prepend_value<std::string>(base_cell_block* block, const std::string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func_base::prepend_value<size_t>(base_cell_block* block, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func_base::prepend_value<bool>(base_cell_block* block, const bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk.insert(blk.begin(), val);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void _prepend_values(base_cell_block* block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename T>
void cell_block_func_base::prepend_values(base_cell_block* block, const T& it_begin, const T& it_end)
{
    assert(it_begin != it_end);
    _prepend_values(block, *it_begin, it_begin, it_end);
}

template<typename T>
void cell_block_func_base::append_value(base_cell_block* block, const T& val)
{
    throw general_error("non-specialized version of append_value called.");
}

template<>
void cell_block_func_base::append_value<double>(base_cell_block* block, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func_base::append_value<std::string>(base_cell_block* block, const std::string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func_base::append_value<size_t>(base_cell_block* block, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func_base::append_value<bool>(base_cell_block* block, const bool& val)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk.push_back(val);
}

void cell_block_func_base::append_values(base_cell_block* dest, const base_cell_block* src)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = *get_boolean_block(dest);
            const boolean_cell_block& s = *get_boolean_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        default:
            throw general_error("append_values: failed to append values to a block of unknown type.");
    }
}

template<typename _Iter>
void _append_values(base_cell_block* block, const typename _Iter::value_type&, const _Iter& it_begin, const _Iter& it_end)
{
    throw general_error("non-specialized version of _append_values called.");
}

template<typename _Iter>
void _append_values(base_cell_block* block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(block);
    numeric_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void _append_values(base_cell_block* block, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(block);
    string_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void _append_values(base_cell_block* block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(block);
    index_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void _append_values(base_cell_block* block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(block);
    boolean_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename T>
void cell_block_func_base::append_values(base_cell_block* block, const T& it_begin, const T& it_end)
{
    assert(it_begin != it_end);
    _append_values(block, *it_begin, it_begin, it_end);
}

void cell_block_func_base::append_values(base_cell_block* dest, const base_cell_block* src, size_t begin_pos, size_t len)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            numeric_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            numeric_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            string_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            string_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            index_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            index_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = *get_boolean_block(dest);
            const boolean_cell_block& s = *get_boolean_block(src);
            boolean_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            boolean_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        default:
            throw general_error("append_values: failed to append values to a block of unknown type.");
    }
}

void cell_block_func_base::assign_values(base_cell_block* dest, const base_cell_block* src, size_t begin_pos, size_t len)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            numeric_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            numeric_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            string_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            string_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            index_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            index_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = *get_boolean_block(dest);
            const boolean_cell_block& s = *get_boolean_block(src);
            boolean_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            boolean_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        default:
            throw general_error("assign_values: failed to assign values to a block of unknown type.");
    }
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, const typename _Iter::value_type&, const _Iter& it_begin, const _Iter& it_end)
{
    throw general_error("non-specialized version of _assign_values called.");
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = *get_numeric_block(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = *get_string_block(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = *get_index_block(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void _assign_values(base_cell_block* dest, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = *get_boolean_block(dest);
    d.assign(it_begin, it_end);
}

template<typename T>
void cell_block_func_base::assign_values(base_cell_block* dest, const T& it_begin, const T& it_end)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    if (it_begin == it_end)
        // Nothing to do.
        return;

    _assign_values(dest, *it_begin, it_begin, it_end);
}

template<typename _Iter>
void _insert_values(
    base_cell_block*, size_t, const typename _Iter::value_type&, const _Iter&, const _Iter&)
{
    throw general_error("non-specialized version of _insert_values called.");
}

template<typename _Iter>
void _insert_values(
    base_cell_block* block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void _insert_values(
    base_cell_block* block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& blk = *get_string_block(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void _insert_values(
    base_cell_block* block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& blk = *get_index_block(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void _insert_values(
    base_cell_block* block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& blk = *get_boolean_block(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename T>
void cell_block_func_base::insert_values(
    base_cell_block* block, size_t pos, const T& it_begin, const T& it_end)
{
    if (!block)
        throw general_error("destination cell block is NULL.");

    if (it_begin == it_end)
        // Nothing to do.
        return;

    _insert_values(block, pos, *it_begin, it_begin, it_end);
}

template<typename T>
void cell_block_func_base::get_value(base_cell_block* block, size_t pos, T& val)
{
    mdds::gridmap::get_value(block, pos, val);
}

template<typename T>
void cell_block_func_base::get_empty_value(T& val)
{
    throw general_error("non-specialized version of get_empty_value called.");
}

template<>
void cell_block_func_base::get_empty_value<double>(double& val)
{
    val = 0.0;
}

template<>
void cell_block_func_base::get_empty_value<std::string>(std::string& val)
{
    val = std::string();
}

template<>
void cell_block_func_base::get_empty_value<size_t>(size_t& val)
{
    val = 0;
}

template<>
void cell_block_func_base::get_empty_value<bool>(bool& val)
{
    val = false;
}

bool cell_block_func_base::equal_block(const base_cell_block* left, const base_cell_block* right)
{
    if (!left && !right)
        return true;

    if (left && !right)
        return false;

    if (!left && right)
        return false;

    assert(left && right);
    cell_t block_type = left->type;
    if (block_type != right->type)
        return false;

    switch (block_type)
    {
        case celltype_numeric:
            return *get_numeric_block(left) == *get_numeric_block(right);
        case celltype_string:
            return *get_string_block(left) == *get_string_block(right);
        case celltype_index:
            return *get_index_block(left) == *get_index_block(right);
        case celltype_boolean:
            return *get_boolean_block(left) == *get_boolean_block(right);
        default:
            ;
    }
    return false;
}


struct cell_block_func : public cell_block_func_base
{
    template<typename T>
    static mdds::gridmap::cell_t get_cell_type(const T& cell)
    {
        return mdds::gridmap::get_cell_type(cell);
    }

    template<typename T>
    static void set_value(mdds::gridmap::base_cell_block* block, size_t pos, const T& val)
    {
        mdds::gridmap::set_value(block, pos, val);
    }

    template<typename T>
    static void get_value(mdds::gridmap::base_cell_block* block, size_t pos, T& val)
    {
        mdds::gridmap::get_value(block, pos, val);
    }
};

}}

#endif
