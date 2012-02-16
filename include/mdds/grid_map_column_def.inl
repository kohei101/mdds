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

#include <stdexcept>
#include <boost/numeric/conversion/cast.hpp>

#if UNIT_TEST
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace mdds { namespace __gridmap {

template<typename _Trait>
column<_Trait>::block::block() : m_size(0), mp_data(NULL) {}

template<typename _Trait>
column<_Trait>::block::block(size_type _size) : m_size(_size), mp_data(NULL) {}

template<typename _Trait>
column<_Trait>::block::block(const block& other) :
    m_size(other.m_size), mp_data(NULL)
{
    mp_data = cell_block_modifier::clone_block(other.mp_data);
}

template<typename _Trait>
column<_Trait>::block::~block()
{
    cell_block_modifier::delete_block(mp_data);
}

template<typename _Trait>
column<_Trait>::column() : m_cur_size(0) {}

template<typename _Trait>
column<_Trait>::column(size_type init_row_size) : m_cur_size(init_row_size)
{
    if (!init_row_size)
        return;

    // Initialize with an empty block that spans from 0 to max.
    m_blocks.push_back(new block(init_row_size));
}

template<typename _Trait>
column<_Trait>::column(const column& other) :
    m_cur_size(other.m_cur_size)
{
    // Clone all the blocks.
    m_blocks.reserve(other.m_blocks.size());
    typename blocks_type::const_iterator it = other.m_blocks.begin(), it_end = other.m_blocks.end();
    for (; it != it_end; ++it)
        m_blocks.push_back(new block(**it));
}

template<typename _Trait>
column<_Trait>::~column()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell(row_key_type row, const _T& cell)
{
    size_type _row = check_row_range(row);

    cell_category_type cat = get_type(cell);

    // Find the right block ID from the row ID.
    size_type start_row = 0; // row ID of the first cell in a block.
    size_type block_index = 0;
    get_block_position(_row, start_row, block_index);

    block* blk = m_blocks[block_index];
    assert(blk->m_size > 0); // block size should never be zero at any time.

    assert(_row >= start_row);
    size_type pos_in_block = _row - start_row;
    assert(pos_in_block < blk->m_size);

    if (!blk->mp_data)
    {
        // This is an empty block.
        set_cell_to_empty_block(block_index, pos_in_block, cell);
        return;
    }

    assert(blk->mp_data);
    cell_category_type blk_cat = get_block_type(*blk->mp_data);

    if (blk_cat == cat)
    {
        // This block is of the same type as the cell being inserted.
        row_key_type i = _row - start_row;
        cell_block_modifier::set_value(blk->mp_data, i, cell);
        return;
    }

    assert(blk_cat != cat);

    if (_row == start_row)
    {
        // Insertion point is at the start of the block.
        if (blk->m_size == 1)
        {
            set_cell_to_block_of_size_one(block_index, cell);
            return;
        }

        assert(blk->m_size > 1);
        if (block_index == 0)
        {
            // No preceding block.
            set_cell_to_top_of_data_block(0, cell);
            return;
        }

        // Append to the previous block if the types match.
        block* blk_prev = m_blocks[block_index-1];
        if (!blk_prev->mp_data)
        {
            // Previous block is empty.
            set_cell_to_top_of_data_block(block_index, cell);
            return;
        }

        cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append to the previous block.
            blk->m_size -= 1;
            cell_block_modifier::erase(blk->mp_data, 0);
            blk_prev->m_size += 1;
            cell_block_modifier::append_value(blk_prev->mp_data, cell);
            return;
        }

        set_cell_to_top_of_data_block(block_index, cell);
        return;
    }

    if (_row < (start_row + blk->m_size - 1))
    {
        // Insertion point is somewhere in the middle of the block.
        set_cell_to_middle_of_block(block_index, pos_in_block, cell);
        return;
    }

    // Insertion point is at the end of the block.
    assert(_row == (start_row + blk->m_size - 1));
    assert(_row > start_row);
    assert(blk->m_size > 1);

    if (block_index == 0)
    {
        if (m_blocks.size() == 1)
        {
            // This is the only block.  Pop the last value from the
            // previous block, and insert a new block for the cell being
            // inserted.
            set_cell_to_bottom_of_data_block(0, cell);
            return;
        }

        assert(block_index < m_blocks.size()-1);
        block* blk_next = m_blocks[block_index+1];
        if (!blk_next->mp_data)
        {
            // Next block is empty.  Pop the last cell of the current
            // block, and insert a new block with the new cell.
            set_cell_to_bottom_of_data_block(0, cell);
            return;
        }

        // Next block is not empty.
        cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
        if (blk_cat_next != cat)
        {
            set_cell_to_bottom_of_data_block(0, cell);
            return;
        }

        // Pop the last cell off the current block, and prepend the
        // new cell to the next block.
        cell_block_modifier::erase(blk->mp_data, blk->m_size-1);
        blk->m_size -= 1;
        cell_block_modifier::prepend_value(blk_next->mp_data, cell);
        return;
    }

    assert(block_index > 0);

    if (block_index == m_blocks.size()-1)
    {
        // This is the last block.
        set_cell_to_bottom_of_data_block(block_index, cell);
        return;
    }

    block* blk_next = m_blocks[block_index+1];
    if (!blk_next->mp_data)
    {
        // Next block is empty.
        set_cell_to_bottom_of_data_block(block_index, cell);
        return;
    }

    cell_category_type cat_blk_next = get_block_type(*blk_next->mp_data);
    if (cat_blk_next != cat)
    {
        // Next block is of different type than that of the cell being inserted.
        set_cell_to_bottom_of_data_block(block_index, cell);
        return;
    }

    // Pop the last element from the current block, and prepend the cell
    // into the next block.
    cell_block_modifier::erase(blk->mp_data, blk->m_size-1);
    blk->m_size -= 1;
    cell_block_modifier::prepend_value(blk_next->mp_data, cell);
    blk_next->m_size += 1;
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cells(row_key_type row, const _T& it_begin, const _T& it_end)
{
    size_type _row = check_row_range(row);
    set_cells_impl(_row, it_begin, it_end);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::insert_cells(row_key_type row, const _T& it_begin, const _T& it_end)
{
    assert(!"not implemented yet.");
}

template<typename _Trait>
typename column<_Trait>::size_type
column<_Trait>::check_row_range(row_key_type row) const
{
    static const char* msg = "Specified row index is out-of-bound.";
    if (row < 0)
        throw std::out_of_range(msg);

    size_type row_internal = boost::numeric_cast<size_type>(row);
    if (row_internal >= m_cur_size)
        throw std::out_of_range(msg);

    return row_internal;
}

template<typename _Trait>
void column<_Trait>::get_block_position(
    size_type row, size_type& start_row, size_type& block_index, size_type start_block, size_type start_block_row) const
{
    start_row = start_block_row;
    for (size_type i = start_block, n = m_blocks.size(); i < n; ++i)
    {
        const block& blk = *m_blocks[i];
        if (row < start_row + blk.m_size)
        {
            // Row is in this block.
            block_index = i;
            return;
        }

        // Specified row is not in this block.
        start_row += blk.m_size;
    }

    assert(!"Block position not found.");
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::create_new_block_with_new_cell(cell_block_type*& data, const _T& cell)
{
    cell_category_type cat = get_type(cell);

    if (data)
        cell_block_modifier::delete_block(data);

    // New cell block is always size 1.
    data = cell_block_modifier::create_new_block(cat);
    if (!data)
        throw general_error("Failed to create new block.");

    cell_block_modifier::set_value(data, 0, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell_to_middle_of_block(
    size_type block_index, size_type pos_in_block, const _T& cell)
{
    block* blk = m_blocks[block_index];

    assert(pos_in_block > 0 && pos_in_block < blk->m_size - 1);
    assert(blk->m_size >= 3);
    row_key_type orig_size = blk->m_size;

    m_blocks.insert(m_blocks.begin()+block_index+1, new block(1));
    block* blk_new = m_blocks[block_index+1];
    m_blocks.insert(m_blocks.begin()+block_index+2, new block(orig_size-pos_in_block-1));
    block* blk_tail = m_blocks[block_index+2];

    if (blk->mp_data)
    {
        cell_category_type blk_cat = get_block_type(*blk->mp_data);

        // Transfer the tail values from the original to the new block.
        blk_tail->mp_data = cell_block_modifier::create_new_block(blk_cat);
        cell_block_modifier::assign_values(
            blk_tail->mp_data, blk->mp_data, pos_in_block+1, orig_size-pos_in_block-1);

        // Shrink the original block.
        cell_block_modifier::resize_block(blk->mp_data, pos_in_block);
    }

    blk->m_size = pos_in_block;

    create_new_block_with_new_cell(blk_new->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::append_cell_to_block(size_type block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    blk->m_size += 1;
    cell_block_modifier::append_value(blk->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell_to_empty_block(
    size_type block_index, size_type pos_in_block, const _T& cell)
{
    block* blk = m_blocks[block_index];

    if (block_index == 0)
    {
        // Topmost block.
        if (m_blocks.size() == 1)
        {
            // this is the only block.
            assert(blk->m_size == m_cur_size);
            if (m_cur_size == 1)
            {
                // This column is allowed to have only one row!
                assert(pos_in_block == 0);
                create_new_block_with_new_cell(blk->mp_data, cell);
            }
            else
            {
                // block has multiple rows.
                if (pos_in_block == 0)
                {
                    // Insert into the first cell in block.
                    blk->m_size -= 1;
                    assert(blk->m_size > 0);

                    m_blocks.insert(m_blocks.begin(), new block(1));
                    blk = m_blocks[block_index];
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else if (pos_in_block == blk->m_size - 1)
                {
                    // Insert into the last cell in block.
                    blk->m_size -= 1;
                    assert(blk->m_size > 0);

                    m_blocks.push_back(new block(1));
                    blk = m_blocks.back();

                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else
                {
                    // Insert into the middle of the block.
                    set_cell_to_middle_of_block(block_index, pos_in_block, cell);
                }
            }
        }
        else
        {
            // this empty block is followed by a non-empty block.
            assert(block_index < m_blocks.size()-1);
            if (pos_in_block == 0)
            {
                if (blk->m_size == 1)
                {
                    // Top empty block with only one cell size.
                    block* blk_next = m_blocks[block_index+1];
                    assert(blk_next->mp_data);
                    cell_category_type cat = get_type(cell);
                    cell_category_type cat_next = get_block_type(*blk_next->mp_data);

                    if (cat == cat_next)
                    {
                        // Remove this one-cell empty block from the top, and
                        // prepend the cell to the next block.
                        delete m_blocks.front();
                        m_blocks.erase(m_blocks.begin());
                        blk = m_blocks.front();
                        blk->m_size += 1;
                        cell_block_modifier::prepend_value(blk->mp_data, cell);
                    }
                    else
                        create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else
                {
                    assert(blk->m_size > 1);
                    blk->m_size -= 1;
                    m_blocks.insert(m_blocks.begin(), new block(1));
                    blk = m_blocks.front();
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else if (pos_in_block == blk->m_size - 1)
            {
                // Immediately above a non-empty block.
                block* blk_next = m_blocks[block_index+1];
                assert(blk_next->mp_data);
                cell_category_type cat = get_type(cell);
                cell_category_type cat_next = get_block_type(*blk_next->mp_data);
                assert(blk->m_size > 1);

                if (cat == cat_next)
                {
                    // Shrink this empty block by one, and prepend the cell to the next block.
                    blk->m_size -= 1;
                    blk_next->m_size += 1;
                    cell_block_modifier::prepend_value(blk_next->mp_data, cell);
                }
                else
                {
                    blk->m_size -= 1;
                    typename blocks_type::iterator it = m_blocks.begin();
                    std::advance(it, block_index+1);
                    m_blocks.insert(it, new block(1));
                    blk = m_blocks[block_index+1];
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else
            {
                // Inserting into the middle of an empty block.
                set_cell_to_middle_of_block(block_index, pos_in_block, cell);
            }
        }

        return;
    }

    // This empty block is right below a non-empty block.
    assert(block_index > 0 && m_blocks[block_index-1]->mp_data != NULL);

    if (pos_in_block == 0)
    {
        // New cell is right below the non-empty block.
        cell_category_type blk_cat_prev = get_block_type(*m_blocks[block_index-1]->mp_data);
        cell_category_type cat = get_type(cell);
        if (blk_cat_prev == cat)
        {
            // Extend the previous block by one to insert this cell.
            if (blk->m_size == 1)
            {
                // Check if we need to merge with the following block.
                if (block_index == m_blocks.size()-1)
                {
                    // Last block.  Delete this block and extend the previous
                    // block by one.
                    delete m_blocks[block_index];
                    m_blocks.pop_back();
                    append_cell_to_block(block_index-1, cell);
                }
                else
                {
                    // Block exists below.
                    block* blk_next = m_blocks[block_index+1];
                    cell_block_type* data_next = blk_next->mp_data;
                    assert(data_next); // Empty block must not be followed by another empty block.
                    cell_category_type blk_cat_next = get_block_type(*data_next);
                    if (blk_cat_prev == blk_cat_next)
                    {
                        // We need to merge the previous and next blocks, then
                        // delete the current and next blocks.
                        block* blk_prev = m_blocks[block_index-1];
                        blk_prev->m_size += 1 + blk_next->m_size;
                        cell_block_modifier::append_value(blk_prev->mp_data, cell);
                        cell_block_modifier::append_values(blk_prev->mp_data, data_next);

                        delete blk;
                        delete blk_next;
                        typename blocks_type::iterator it = m_blocks.begin() + block_index;
                        m_blocks.erase(it, it+2);
                    }
                    else
                    {
                        // Ignore the next block. Just extend the previous block.
                        delete m_blocks[block_index];
                        m_blocks.erase(m_blocks.begin() + block_index);
                        append_cell_to_block(block_index-1, cell);
                    }
                }
            }
            else
            {
                // Extend the previous block to append the cell.
                assert(blk->m_size > 1);
                blk->m_size -= 1;
                append_cell_to_block(block_index-1, cell);
            }
        }
        else
        {
            // Cell type is different from the type of the previous block.
            if (blk->m_size == 1)
            {
                if (block_index == m_blocks.size()-1)
                {
                    // There is no more block below.
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else
                {
                    // Check the type of the following non-empty block.
                    assert(block_index < m_blocks.size()-1);
                    block* blk_next = m_blocks[block_index+1];
                    assert(blk_next->mp_data);
                    cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
                    if (cat == blk_cat_next)
                    {
                        // Remove this empty block, and prepend the cell to the next block.
                        blk_next->m_size += 1;
                        cell_block_modifier::prepend_value(blk_next->mp_data, cell);
                        delete m_blocks[block_index];
                        m_blocks.erase(m_blocks.begin()+block_index);
                    }
                    else
                        create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else
            {
                create_new_block_with_new_cell(blk->mp_data, cell);
                m_blocks.push_back(new block(blk->m_size-1));
                blk->m_size = 1;
            }
        }
    }
    else if (pos_in_block == blk->m_size - 1)
    {
        // New cell is at the last cell position.
        assert(blk->m_size > 1);
        if (block_index == m_blocks.size()-1)
        {
            // This is the last block.
            blk->m_size -= 1;
            m_blocks.push_back(new block(1));
            blk = m_blocks.back();
            create_new_block_with_new_cell(blk->mp_data, cell);
        }
        else
        {
            // A non-empty block exists below.
            cell_category_type cat = get_type(cell);
            block* blk_next = m_blocks[block_index+1];
            assert(blk_next->mp_data);
            cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
            if (cat == blk_cat_next)
            {
                // Shrink this empty block and extend the next block.
                blk->m_size -= 1;
                blk_next->m_size += 1;
                cell_block_modifier::prepend_value(blk_next->mp_data, cell);
            }
            else
            {
                // Just insert this new cell.
                blk->m_size -= 1;
                m_blocks.insert(m_blocks.begin()+1, new block(1));
                blk = m_blocks[block_index+1];
                create_new_block_with_new_cell(blk->mp_data, cell);
            }
        }
    }
    else
    {
        // New cell is somewhere in the middle of an empty block.
        set_cell_to_middle_of_block(block_index, pos_in_block, cell);
    }
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell_to_block_of_size_one(size_type block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    assert(blk->m_size == 1);
    assert(blk->mp_data);
    cell_category_type cat = get_type(cell);
    cell_category_type blk_cat = get_block_type(*blk->mp_data);
    assert(blk_cat != cat);

    if (block_index == 0)
    {
        // This is the topmost block of size 1.
        if (block_index == m_blocks.size()-1)
        {
            // This is the only block.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // There is an existing block below.
        block* blk_next = m_blocks[block_index+1];
        if (!blk_next->mp_data)
        {
            // Next block is empty.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Next block is not empty.
        cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
        if (blk_cat_next != cat)
        {
            // Cell being inserted is of different type than that of the next block.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Delete the current block, and prepend the cell to the next block.
        blk_next->m_size += 1;
        cell_block_modifier::prepend_value(blk_next->mp_data, cell);
        delete blk;
        m_blocks.erase(m_blocks.begin()+block_index);
        return;
    }

    assert(block_index > 0);

    if (block_index == m_blocks.size()-1)
    {
        // This is the last block, and a block exists above.
        block* blk_prev = m_blocks[block_index-1];
        if (!blk_prev->mp_data)
        {
            // Previous block is empty. Replace the current block with a new one.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append the cell to the previos block, and remove the
            // current block.
            cell_block_modifier::append_value(blk_prev->mp_data, cell);
            blk_prev->m_size += 1;
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index);
            return;
        }

        // Simply replace the current block with a new block of new type.
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    // Remove the current block, and check if the cell can be append
    // to the previous block, or prepended to the following block.
    // Also check if the blocks above and below need to be combined.

    block* blk_prev = m_blocks[block_index-1];
    block* blk_next = m_blocks[block_index+1];
    if (!blk_prev->mp_data)
    {
        // Previous block is empty.
        if (!blk_next->mp_data)
        {
            // Next block is empty too.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Previous block is empty, but the next block is not.
        cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
        if (blk_cat_next == cat)
        {
            // Delete the current block, and prepend the new cell to the next block.
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index);
            blk = m_blocks[block_index];
            blk->m_size += 1;
            cell_block_modifier::prepend_value(blk->mp_data, cell);
            return;
        }

        assert(blk_cat_next != cat);
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    if (!blk_next->mp_data)
    {
        // Next block is empty.
        assert(blk_prev->mp_data);
        cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append to the previous block.
            blk_prev->m_size += 1;
            cell_block_modifier::append_value(blk_prev->mp_data, cell);
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index);
            return;
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    assert(blk_prev && blk_prev->mp_data);
    assert(blk_next && blk_next->mp_data);
    cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
    cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);

    if (blk_cat_prev == blk_cat_next)
    {
        if (blk_cat_prev == cat)
        {
            // Merge the previous block with the cell being inserted and
            // the next block.
            blk_prev->m_size += 1 + blk_next->m_size;
            cell_block_modifier::append_value(blk_prev->mp_data, cell);
            cell_block_modifier::append_values(blk_prev->mp_data, blk_next->mp_data);

            // Delete the current and next blocks.
            delete blk;
            delete blk_next;
            typename blocks_type::iterator it = m_blocks.begin() + block_index;
            typename blocks_type::iterator it_last = it + 2;
            m_blocks.erase(it, it_last);
            return;
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    assert(blk_cat_prev != blk_cat_next);

    if (blk_cat_prev == cat)
    {
        // Append to the previous block.
        blk_prev->m_size += 1;
        cell_block_modifier::append_value(blk_prev->mp_data, cell);
        delete blk;
        m_blocks.erase(m_blocks.begin()+block_index);
        return;
    }

    if (blk_cat_next == cat)
    {
        // Prepend to the next block.
        blk_next->m_size += 1;
        cell_block_modifier::prepend_value(blk_next->mp_data, cell);
        delete blk;
        m_blocks.erase(m_blocks.begin()+block_index);
        return;
    }

    // Just overwrite the current block.
    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell_to_top_of_data_block(size_type block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    blk->m_size -= 1;
    cell_block_modifier::erase(blk->mp_data, 0);
    m_blocks.insert(m_blocks.begin()+block_index, new block(1));
    blk = m_blocks[block_index];
    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell_to_bottom_of_data_block(size_type block_index, const _T& cell)
{
    assert(block_index < m_blocks.size());
    block* blk = m_blocks[block_index];
    cell_block_modifier::erase(blk->mp_data, blk->m_size-1);
    blk->m_size -= 1;
    m_blocks.insert(m_blocks.begin()+block_index+1, new block(1));
    blk = m_blocks[block_index+1];
    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::get_cell(row_key_type row, _T& cell) const
{
    size_type _row = check_row_range(row);

    size_type start_row = 0;
    size_type block_index = static_cast<size_type>(-1);
    get_block_position(_row, start_row, block_index);
    const block* blk = m_blocks[block_index];
    assert(blk);

    if (!blk->mp_data)
    {
        // empty cell block.
        cell_block_modifier::get_empty_value(cell);
        return;
    }

    assert(_row >= start_row);
    assert(blk->mp_data); // data for non-empty blocks should never be NULL.
    row_key_type idx = _row - start_row;
    cell_block_modifier::get_value(blk->mp_data, idx, cell);
}

template<typename _Trait>
bool column<_Trait>::is_empty(row_key_type row) const
{
    size_type _row = check_row_range(row);

    size_type start_row;
    size_type block_index;
    get_block_position(_row, start_row, block_index);

    return m_blocks[block_index]->mp_data == NULL;
}

template<typename _Trait>
void column<_Trait>::set_empty(row_key_type start_row, row_key_type end_row)
{
    size_type _start_row = check_row_range(start_row);
    size_type _end_row = check_row_range(end_row);

    if (_start_row > _end_row)
        throw std::out_of_range("Start row is larger than the end row.");

    size_type start_row_in_block1, start_row_in_block2;
    size_type block_pos1, block_pos2;
    get_block_position(_start_row, start_row_in_block1, block_pos1);
    get_block_position(_end_row, start_row_in_block2, block_pos2, block_pos1, start_row_in_block1);

    if (block_pos1 == block_pos2)
    {
        set_empty_in_single_block(_start_row, _end_row, block_pos1, start_row_in_block1);
        return;
    }

    assert(block_pos1 < block_pos2);

    {
        // Empty the lower part of the first block.
        block* blk = m_blocks[block_pos1];
        if (blk->mp_data)
        {
            if (start_row_in_block1 == _start_row)
            {
                // Empty the whole block.
                cell_block_modifier::delete_block(blk->mp_data);
                blk->mp_data = NULL;
            }
            else
            {
                // Empty the lower part.
                size_type new_size = _start_row - start_row_in_block1;
                cell_block_modifier::resize_block(blk->mp_data, new_size);
                blk->m_size = new_size;
            }
        }
        else
        {
            // First block is already empty.  Adjust the start row of the new
            // empty range.
            _start_row = start_row_in_block1;
        }
    }

    {
        // Empty the upper part of the last block.
        block* blk = m_blocks[block_pos2];
        size_type last_row_in_block = start_row_in_block2 + blk->m_size - 1;
        if (blk->mp_data)
        {
            if (last_row_in_block == _end_row)
            {
                // Delete the whole block.
                delete blk;
                m_blocks.erase(m_blocks.begin()+block_pos2);
            }
            else
            {
                // Empty the upper part.
                size_type size_to_erase = _end_row - start_row_in_block2 + 1;
                cell_block_modifier::erase(blk->mp_data, 0, size_to_erase);
                blk->m_size -= size_to_erase;
            }
        }
        else
        {
            // Last block is empty.  Delete this block and adjust the end row
            // of the new empty range.
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_pos2);
            _end_row = last_row_in_block;
        }
    }

    if (block_pos2 - block_pos1 > 1)
    {
        // Remove all blocks in-between, from block_pos1+1 to block_pos2-1.

        for (size_type i = block_pos1 + 1; i < block_pos2; ++i)
            delete m_blocks[i];

        typename blocks_type::iterator it = m_blocks.begin() + block_pos1 + 1;
        typename blocks_type::iterator it_end = m_blocks.begin() + block_pos2;
        m_blocks.erase(it, it_end);
    }

    // Insert a single empty block.
    block* blk = m_blocks[block_pos1];
    size_type empty_block_size = _end_row - _start_row + 1;
    if (blk->mp_data)
    {
        // Insert a new empty block after the first block.
        m_blocks.insert(m_blocks.begin()+block_pos1+1, new block(empty_block_size));
    }
    else
    {
        // Current block is already empty. Just extend its size.
        blk->m_size = empty_block_size;
    }
}

template<typename _Trait>
void column<_Trait>::erase(row_key_type start_row, row_key_type end_row)
{
    size_type _start_row = check_row_range(start_row);
    size_type _end_row = check_row_range(end_row);

    if (_start_row > _end_row)
        throw std::out_of_range("Start row is larger than the end row.");

    erase_impl(_start_row, _end_row);
}

template<typename _Trait>
void column<_Trait>::erase_impl(size_type start_row, size_type end_row)
{
    assert(start_row <= end_row);

    // Keep the logic similar to set_empty().

    size_type start_row_in_block1, start_row_in_block2;
    size_type block_pos1, block_pos2;
    get_block_position(start_row, start_row_in_block1, block_pos1);
    get_block_position(end_row, start_row_in_block2, block_pos2, block_pos1, start_row_in_block1);

    if (block_pos1 == block_pos2)
    {
        // Range falls within the same block.
        block* blk = m_blocks[block_pos1];
        size_type size_to_erase = end_row - start_row + 1;
        if (blk->mp_data)
        {
            // Erase data in the data block.
            size_type offset = start_row - start_row_in_block1;
            cell_block_modifier::erase(blk->mp_data, offset, size_to_erase);
        }

        blk->m_size -= size_to_erase;
        m_cur_size -= size_to_erase;

        if (blk->m_size == 0)
        {
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_pos1);
        }
        return;
    }

    assert(block_pos1 < block_pos2);

    // Initially, we set to erase all blocks between the first and the last.
    typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_pos1 + 1;
    typename blocks_type::iterator it_erase_end   = m_blocks.begin() + block_pos2;

    // First, inspect the first block.
    if (start_row_in_block1 == start_row)
    {
        // Erase the whole block.
        --it_erase_begin;
    }
    else
    {
        // Erase the lower part of the first block.
        block* blk = m_blocks[block_pos1];
        size_type new_size = start_row - start_row_in_block1;
        if (blk->mp_data)
        {
            // Shrink the data array.
            cell_block_modifier::resize_block(blk->mp_data, new_size);
        }
        blk->m_size = new_size;
    }

    // Then inspect the last block.
    block* blk = m_blocks[block_pos2];
    size_type last_row_in_block = start_row_in_block2 + blk->m_size - 1;
    if (last_row_in_block == end_row)
    {
        // Delete the whole block.
        ++it_erase_end;
    }
    else
    {
        size_type size_to_erase = end_row - start_row_in_block2 + 1;
        blk->m_size -= size_to_erase;
        if (blk->mp_data)
        {
            // Erase the upper part.
            cell_block_modifier::erase(blk->mp_data, 0, size_to_erase);
        }
    }

    // Now, erase all blocks in between.
    std::for_each(it_erase_begin, it_erase_end, default_deleter<block>());
    m_blocks.erase(it_erase_begin, it_erase_end);
    m_cur_size -= end_row - start_row + 1;
}

template<typename _Trait>
void column<_Trait>::insert_empty(row_key_type row, size_type length)
{
    if (!length)
        // Nothing to insert.
        return;

    size_type _row = check_row_range(row);
    insert_empty_impl(_row, length);
}

template<typename _Trait>
void column<_Trait>::insert_empty_impl(size_type row, size_type length)
{
    assert(row < m_cur_size);

    size_type start_row, block_index;
    get_block_position(row, start_row, block_index);

    block* blk = m_blocks[block_index];
    if (!blk->mp_data)
    {
        // Insertion point is already empty.  Just expand its size and be done
        // with it.
        blk->m_size += length;
        m_cur_size += length;
        return;
    }

    if (start_row == row)
    {
        // Insertion point is at the top of an existing non-empty block.
        if (block_index > 0)
        {
            block* blk_prev = m_blocks[block_index-1];
            if (!blk_prev->mp_data)
            {
                // Previous block is empty.  Expand the size of the previous
                // block and bail out.
                blk_prev->m_size += length;
                m_cur_size += length;
                return;
            }
        }

        // Insert a new empty block.
        m_blocks.insert(m_blocks.begin()+block_index, new block(length));
        m_cur_size += length;
        return;
    }

    assert(blk->mp_data);
    assert(row > start_row);

    size_type size_blk_prev = row - start_row;
    size_type size_blk_next = blk->m_size - size_blk_prev;

    // Insert two new block below the current; one for the empty block being
    // inserted, and one for the lower part of the current non-empty block.
    m_blocks.insert(m_blocks.begin()+block_index+1, 2, NULL);

    m_blocks[block_index+1] = new block(length);
    m_blocks[block_index+2] = new block(size_blk_next);

    block* blk_next = m_blocks[block_index+2];
    blk_next->mp_data = cell_block_modifier::create_new_block(get_block_type(*blk->mp_data));
    cell_block_modifier::assign_values(blk_next->mp_data, blk->mp_data, size_blk_prev, size_blk_next);

    cell_block_modifier::resize_block(blk->mp_data, size_blk_prev);
    blk->m_size = size_blk_prev;

    m_cur_size += length;
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cells_impl(size_type row, const _T& it_begin, const _T& it_end)
{
    size_type length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return;

    size_type end_row = row + length - 1;
    if (end_row >= m_cur_size)
        throw std::out_of_range("Data array is too long.");

    size_t block_index1, block_index2, start_row1, start_row2;
    get_block_position(row, start_row1, block_index1);
    get_block_position(end_row, start_row2, block_index2, block_index1, start_row1);

    if (block_index1 == block_index2)
    {
        // The whole data array will fit in a single block.
        set_cells_to_single_block(row, end_row, block_index1, start_row1, it_begin, it_end);
        return;
    }
    assert(!"I'm working on this.");
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cells_to_single_block(
    size_type start_row, size_type end_row, size_type block_index,
    size_type start_row_in_block, const _T& it_begin, const _T& it_end)
{
    assert(it_begin != it_end);

    cell_category_type cat = get_type(*it_begin);
    block* blk = m_blocks[block_index];

    if (blk->mp_data)
    {
        cell_category_type blk_cat = get_block_type(*blk->mp_data);
        if (cat == blk_cat)
        {
            // simple overwrite.
            size_type offset = start_row - start_row_in_block;
            cell_block_modifier::set_values(blk->mp_data, offset, it_begin, it_end);
            return;
        }
    }

    size_type end_row_in_block = start_row_in_block + blk->m_size - 1;
    if (start_row == start_row_in_block)
    {
        if (end_row == end_row_in_block)
        {
            // Replace the whole block.
            if (blk->mp_data)
                cell_block_modifier::delete_block(blk->mp_data);

            cell_category_type cat = get_type(*it_begin);
            blk->mp_data = cell_block_modifier::create_new_block(cat);
            cell_block_modifier::assign_values(blk->mp_data, it_begin, it_end);
            return;
        }

        // Replace the upper part of the block.

        // Shrink the current block first.
        size_type length = end_row_in_block - end_row;
        blk->m_size = length;
        if (blk->mp_data)
        {
            // Erase the upper part of the data from the current data array.
            cell_block_type* new_data = cell_block_modifier::create_new_block(
                get_block_type(*blk->mp_data));
            if (!new_data)
                throw std::logic_error("failed to instantiate a new data array.");

            cell_block_modifier::assign_values(new_data, blk->mp_data, end_row+1, length);
            cell_block_modifier::delete_block(blk->mp_data);
            blk->mp_data = new_data;
        }

        if (block_index > 0)
        {
            block* blk_prev = m_blocks[block_index-1];
            if (blk_prev->mp_data)
            {
                cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
                if (blk_cat_prev == cat)
                {
                    // Append to the previous block.
                    assert(!"not implemented yet.");
                    return;
                }
            }
        }

        // Insert a new block before the current block, and populate it with
        // the new data.
        length = end_row - start_row + 1;
        m_blocks.insert(m_blocks.begin()+block_index, new block(length));
        blk = m_blocks[block_index];
        blk->mp_data = cell_block_modifier::create_new_block(cat);
        blk->m_size = length;
        cell_block_modifier::assign_values(blk->mp_data, it_begin, it_end);
        return;
    }

    assert(!"I'm working on this.");
}

template<typename _Trait>
void column<_Trait>::clear()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
    m_blocks.clear();
    m_cur_size = 0;
}

template<typename _Trait>
typename column<_Trait>::size_type
column<_Trait>::size() const
{
    return m_cur_size;
}

template<typename _Trait>
typename column<_Trait>::size_type
column<_Trait>::block_size() const
{
    return m_blocks.size();
}

template<typename _Trait>
bool column<_Trait>::empty() const
{
    return m_blocks.empty();
}

template<typename _Trait>
void column<_Trait>::resize(size_type new_size)
{
    if (new_size == m_cur_size)
        return;

    if (!new_size)
    {
        clear();
        return;
    }

    if (new_size > m_cur_size)
    {
        // Append empty cells.
        if (m_blocks.empty())
        {
            // No existing block. Create a new one.
            assert(m_cur_size == 0);
            m_blocks.push_back(new block(new_size));
            m_cur_size = new_size;
            return;
        }

        block* blk_last = m_blocks.back();
        size_type delta = new_size - m_cur_size;

        if (!blk_last->mp_data)
        {
            // Last block is empty.  Just increase its size.
            blk_last->m_size += delta;
        }
        else
        {
            // Append a new empty block.
            m_blocks.push_back(new block(delta));
        }
        m_cur_size = new_size;
        return;
    }

    assert(new_size < m_cur_size && new_size > 0);

    // Find out in which block the new end row will be.
    size_type new_end_row = new_size - 1;
    size_type start_row_in_block, block_index;
    get_block_position(new_end_row, start_row_in_block, block_index);

    block* blk = m_blocks[block_index];
    size_type end_row_in_block = start_row_in_block + blk->m_size - 1;

    if (new_end_row < end_row_in_block)
    {
        // Shrink the size of the current block.
        size_type new_block_size = new_end_row - start_row_in_block + 1;
        cell_block_modifier::resize_block(blk->mp_data, new_block_size);
        blk->m_size = new_block_size;
    }

    // Remove all blocks that are below this one.
    typename blocks_type::iterator it = m_blocks.begin() + block_index + 1;
    std::for_each(it, m_blocks.end(), default_deleter<block>());
    m_blocks.erase(it, m_blocks.end());
    m_cur_size = new_size;
}

template<typename _Trait>
void column<_Trait>::swap(column& other)
{
    std::swap(m_cur_size, other.m_cur_size);
    m_blocks.swap(other.m_blocks);
}

template<typename _Trait>
bool column<_Trait>::operator== (const column& other) const
{
    if (this == &other)
        // Comparing to self is always equal.
        return true;

    if (m_blocks.size() != other.m_blocks.size())
        // Block sizes differ.
        return false;

    if (m_cur_size != other.m_cur_size)
        // Row sizes differ.
        return false;

    typename blocks_type::const_iterator it = m_blocks.begin(), it_end = m_blocks.end();
    typename blocks_type::const_iterator it2 = other.m_blocks.begin();
    for (; it != it_end; ++it, ++it2)
    {
        const block* blk1 = *it;
        const block* blk2 = *it2;
        if (!cell_block_modifier::equal_block(blk1->mp_data, blk2->mp_data))
            return false;
    }

    return true;
}

template<typename _Trait>
bool column<_Trait>::operator!= (const column& other) const
{
    return !operator== (other);
}

template<typename _Trait>
column<_Trait>& column<_Trait>::operator= (const column& other)
{
    column assigned(other);
    swap(assigned);
    return *this;
}

template<typename _Trait>
void column<_Trait>::set_empty_in_single_block(
    size_type start_row, size_type end_row, size_type block_index, size_type start_row_in_block)
{
    // Range is within a single block.
    block* blk = m_blocks[block_index];
    if (!blk->mp_data)
        // This block is already empty.  Do nothing.
        return;

    assert(start_row_in_block + blk->m_size >= 1);
    size_type end_row_in_block = start_row_in_block + blk->m_size - 1;
    size_type empty_block_size = end_row - start_row + 1;

    if (start_row == start_row_in_block)
    {
        // start row coincides with the start of a block.

        if (end_row == end_row_in_block)
        {
            // Set the whole block empty.
            cell_block_modifier::delete_block(blk->mp_data);
            blk->mp_data = NULL;
            return;
        }

        // Set the upper part of the block empty.
        cell_block_modifier::erase(blk->mp_data, 0, empty_block_size);
        blk->m_size -= empty_block_size;

        // Insert a new empty block before the current one.
        m_blocks.insert(m_blocks.begin()+block_index, new block(empty_block_size));
        return;
    }

    if (end_row == end_row_in_block)
    {
        // end row coincides with the end of a block.
        assert(start_row > start_row_in_block);

        // Set the lower part of the block empty.
        cell_block_modifier::erase(blk->mp_data, end_row_in_block-empty_block_size+1, empty_block_size);
        blk->m_size -= empty_block_size;

        // Insert a new empty block after the current one.
        m_blocks.insert(m_blocks.begin()+block_index+1, new block(empty_block_size));
        return;
    }

    // Empty the middle part of a block.
    assert(end_row_in_block - end_row > 0);

    // First, insert two new blocks at position past the current block.
    size_type lower_block_size = end_row_in_block - end_row;
    m_blocks.insert(m_blocks.begin()+block_index+1, 2, NULL);
    m_blocks[block_index+1] = new block(empty_block_size); // empty block.
    m_blocks[block_index+2] = new block(lower_block_size);

    // Copy the lower values from the current block to the new non-empty block.
    block* blk_lower = m_blocks[block_index+2];
    assert(blk_lower->m_size == lower_block_size);
    cell_category_type blk_cat = get_block_type(*blk->mp_data);
    blk_lower->mp_data = cell_block_modifier::create_new_block(blk_cat);
    cell_block_modifier::assign_values(
        blk_lower->mp_data, blk->mp_data, end_row_in_block-lower_block_size+1, lower_block_size);

    // Shrink the current data block.
    cell_block_modifier::erase(
        blk->mp_data, start_row-start_row_in_block, end_row_in_block-start_row+1);
    blk->m_size = start_row - start_row_in_block;
}

}}
