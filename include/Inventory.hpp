#pragma once

#include <vector>
#include <string>

struct Item {
    std::string name;
    int quantity = 1;
};

class Inventory {
    std::vector<Item> m_items;

public:
    void addItem(const Item& _item) { m_items.push_back(_item); }

    const std::vector<Item>& getItems() const { return m_items; }
};
