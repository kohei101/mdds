/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2018 Kohei Yoshida
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

#include "test_global.hpp"
#include "test_global_rtree.hpp"

#include <vector>
#include <fstream>
#include <unordered_map>

using namespace mdds::draft;
using namespace std;

void rtree_test_bl_empty()
{
    stack_printer __stack_printer__("::rtree_test_bl_empty");
    using rt_type = rtree<int16_t, std::string>;
    using integrity_check_type = rt_type::integrity_check_type;

    // Load nothing.
    rt_type::bulk_loader loader;
    rt_type tree = loader.pack();
    assert(tree.empty());
    tree.check_integrity(integrity_check_type::whole_tree);
}

void rtree_test_bl_insert_points_move()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_points_move");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    using integrity_check_type = rt_type::integrity_check_type;
    using key_type = rt_type::key_type;

    rt_type::bulk_loader loader;
    for (key_type x = 0; x < 20; ++x)
    {
        key_type yn = (x == 0) ? 19 : 20;
        for (key_type y = 0; y < yn; ++y)
        {
            std::ostringstream os;
            os << '(' << x << ',' << y << ')';
            loader.insert({x, y}, os.str());
        }
    }

    auto tree = loader.pack();
    assert(tree.size() == 399);
    tree.check_integrity(integrity_check_type::whole_tree);
    export_tree(tree, "rtree-test-bl-insert-points");
}

void rtree_test_bl_insert_points_copy()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_points_copy");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    using integrity_check_type = rt_type::integrity_check_type;
    using point_type = rt_type::point_type;
    using search_type = rt_type::search_type;

    struct kv
    {
        point_type point;
        std::string value;
    };

    std::vector<kv> values =
    {
        { {  0,    0}, "origin"      },
        { {125,  125}, "middle"      },
        { { 22,  987}, "somewhere"   },
        { {-34, -200}, "negative"    },
        { {  2,    3}, "near origin" },
    };

    // Insert less than max node size in order to test the packing
    // implementation that doesn't involve per-level packing.
    tiny_trait_2d_forced_reinsertion t;
    assert(values.size() <= t.max_node_size);

    for (size_t n_values = 1; n_values <= values.size(); ++n_values)
    {
        auto loader = rt_type::bulk_loader();

        // Insert specified number of value(s).
        for (size_t i = 0; i < n_values; ++i)
            loader.insert(values[i].point, values[i].value);

        // Populate and pack the tree.
        auto tree = loader.pack();
        tree.check_integrity(integrity_check_type::whole_tree);
        assert(tree.size() == n_values);

        // Make sure the inserted values are all there.
        for (size_t i = 0; i < n_values; ++i)
        {
            auto res = tree.search(values[i].point, search_type::match);
            assert(std::distance(res.begin(), res.end()) == 1);
            auto it = res.begin();
            assert(*it == values[i].value);

            // The values should all be the immediate children of the root
            // directory node.
            assert(it.depth() == 1);
        }
    }
}

int main(int argc, char** argv)
{
    rtree_test_bl_empty();
    rtree_test_bl_insert_points_move();
    rtree_test_bl_insert_points_copy();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

