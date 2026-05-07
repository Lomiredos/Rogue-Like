#pragma once

#include <vector>
#include <string>
#include "Items/Weapons/WeaponData.hpp"
#include "Items/Consomable/ConsomableData.hpp"
#include "Items/Passive/PassiveData.hpp"
#include <optional>

class Inventory {

    using SlotContent = std::variant<WeaponData, ConsomableData, PassiveData>;
    std::vector<SlotContent> m_items;
    int m_inventorySize = 3;
    int m_currentIndex = 0;

public:
    void addItem(const SlotContent& _item) { m_items.push_back(_item); }
    
    void upgradeInventory() { m_inventorySize = std::min(9, m_inventorySize + 1); }
    void downgradeInventory() { m_inventorySize = std::max(0, m_inventorySize - 1);  if (m_currentIndex == m_inventorySize) m_currentIndex--; }
    void setInventorySize(int _size) { m_inventorySize = std::max(std::min(9, _size), 0); }

    void changeCurrentIndex(int _delta) 
    { 
        if (m_inventorySize == 0) { m_currentIndex = 0; return; }
        m_currentIndex = ((m_currentIndex + _delta) % m_inventorySize + m_inventorySize) % m_inventorySize;
    }

    const std::vector<SlotContent>& getItems() const { return m_items; }

    const std::optional<SlotContent>& getEquipped() const { if (m_inventorySize > m_currentIndex) return m_items[m_currentIndex]; return std::nullopt; }
    const int getInventorySize() const { return m_inventorySize; }
    const int getCurrentIndex() const { return m_currentIndex; }
};
