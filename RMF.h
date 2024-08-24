// Copyright (c) 2024 Matt M Halenza
// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <string_view>
#include <type_traits>
#include <stdint.h>

namespace RMF {

template <typename AddressType>
concept ValidAddressOrDataType = std::is_same_v<AddressType, uint8_t> || std::is_same_v<AddressType, uint16_t> || std::is_same_v<AddressType, uint32_t> || std::is_same_v<AddressType, uint64_t>;

template <ValidAddressOrDataType AType, ValidAddressOrDataType DType, typename ParentType>
struct BRFBase
{
public:
    using AddressType = AType;
    using DataType = DType;
protected:
    ParentType const* m_parent;
    AddressType const m_address;
    std::string_view const m_name;
protected:
    constexpr explicit BRFBase(ParentType const* parent, AddressType const offset, std::string_view name)
        : m_parent(parent)
        , m_address((m_parent ? m_parent->address() : 0) + offset)
        , m_name(name)
    {}
public:
    constexpr ParentType const* parent() const
    {
        return this->m_parent;
    }
    constexpr AddressType address() const
    {
        return this->m_address;
    }
    #ifdef RMF_EXPLICIT_ADDRESSTYPE_CONVERSION_OPERATOR
    constexpr explicit operator AddressType() const
    #else
    constexpr operator AddressType() const
    #endif
    {
        return this->m_address;
    }
    constexpr std::string_view name() const
    {
        return this->m_name;
    }
    constexpr operator std::string_view() const
    {
        return this->m_name;
    }
    constexpr std::string fullName() const
    {
        if (this->m_parent) {
            return this->m_parent->fullName().append(".").append(this->m_name);
        }
        else {
            return std::string{ this->m_name };
        }
    }
};

template <ValidAddressOrDataType AType, ValidAddressOrDataType DType>
struct Block : public BRFBase<AType, DType, Block<AType, DType>>
{
public:
    constexpr explicit Block(Block<AType, DType> const* parent, AType const offset, std::string_view name)
        : BRFBase<AType, DType, Block<AType, DType>>(parent, offset, name)
    {}
};

template <ValidAddressOrDataType AType, ValidAddressOrDataType DType>
struct Register : public BRFBase<AType, DType, Block<AType, DType>>
{
public:
    constexpr explicit Register(Block<AType, DType> const* parent, AType const offset, std::string_view name)
        : BRFBase<AType, DType, Block<AType, DType>>(parent, offset, name)
    {}
};

template <ValidAddressOrDataType AType, ValidAddressOrDataType DType>
struct Field : public BRFBase<AType, DType, Register<AType, DType>>
{
private:
    uint8_t const m_field_offset;
    uint8_t const m_size;
public:
    constexpr explicit Field(Register<AType, DType> const* parent, uint8_t const offset, uint8_t const size, std::string_view name)
        : BRFBase<AType, DType, Register<AType, DType>>(parent, 0, name)
        , m_field_offset(offset)
        , m_size(size)
    {}

    constexpr uint8_t offset() const
    {
        return this->m_field_offset;
    }
    constexpr uint8_t size() const
    {
        return this->m_size;
    }

    constexpr DType dataMask() const
    {
        return ((1 << this->m_size) - 1);
    }
    constexpr DType regMask() const
    {
        return this->dataMask() << this->m_field_offset;
    }

    constexpr DType extract(DType const reg_val) const
    {
        return (reg_val >> this->m_field_offset) & this->dataMask();
    }
    constexpr DType regVal(DType val) const
    {
        return (val & this->dataMask()) << this->m_field_offset;
    }
    constexpr void insert(DType& reg_val, DType val) const
    {
        reg_val &= ~this->regMask();
        reg_val |= this->regVal(val);
    }
};

}
