# Register Map Framework

RMF is a C++1? framework for working with register maps, common in embedded and hardware-centric applications.

## Table of Contents
- [Getting Started](#getting-started)
- [Components](#components)
    - [BRFBase](#brfbase)
    - [Block](#block)
    - [Register](#register)
    - [Field](#field)


## Getting Started
RMF is a header-only library, and as such it can simply be copied to your project's source tree.

Because RMF is a framework, on it's own it doesn't provide much functionality but rather provides useful interface definitions and processes that projects can standardize around.

Register Maps that use RMF are not expected to be written by hand, but rather auto generated from some other definition file (such as IP-XACT or SystemRDL or custom in-house tools).

## Components
RMF provides a couple base classes for use in register maps:
- [BRFBase](#brfbase)
- [Block](#block)
- [Register](#register)
- [Field](#field)

### BRFBase
`RMF::BRFBase` is an internal base class for `Block`, `Register`, and `Field`.
It provides functionality common to all three:
- parent pointer
- address
- name

Accessors:
- `constexpr ParentType const* parent() const` returns a pointer to the parent object (if unparented, returns nullptr, so make sure to check that in generic code)
    The `ParentType` is specific to the subclass:
    Blocks have Blocks as parents, Registers have Blocks as parents, and Fields have Registers as parents.
- `constexpr AddressType address() const` returns the absolute address of the block or register (and for fields, returns the address of the containing register)
- `constexpr std::string_view name() const` returns the name of the block/register/field
- `constexpr operator AddressType() const` same as `address()` but as an operator overload, so that it can be used where numeric addresses are expected
- `constexpr operator std::string_view() const` same as `name()` but as an operator overload
- `constexpr std::string fullName() const` returns the full "path" to the block/register/field: it traverses up the parent chain, grabs the name from each object and concatenates them all with '.'

### Block
`RMF::Block` provides a base class for blocks / maps.

Some systems differentiate "blocks" from "maps": "Blocks" are just chunks of registers (and only registers), while "Maps" are groups of submaps and blocks (and no registers).
RMF doesn't enforce this distinction, though; both categories should subclass `RMF::Block`.

`Block` provides no other functionality on top of `RMF::BRFBase`.

Subclasses should provide constructors consistent with the `RMF::Block` constructor:
```c++
constexpr explicit BlockSubclass(::RMF::Block<AddressType, DataType> const* parent, AddressType const offset, std::string_view name) : Block(parent, offset, name) {}
```

### Register
`RMF::Register` provides a base class for registers.
It can also be used directly if registers have no fields, or have only a single field that is the whole width of the register.

Subclasses should provide constructors consistent with the `RMF::Register` constructor:
```c++
constexpr explicit RegisterSubclass(::RMF::Block<AddressType, DataType> const* parent, AddressType const offset, std::string_view name) : Block(parent, offset, name) {}
```

Register definitions that do not contain fields could be as such:
```c++
    ::RMF::Register<AddressType, DataType> scratchpad{ this, 0x4, "scratch" };
```
While register definitions that contain fields could be written like this:
```c++
    struct R_control : public ::RMF::Register<AddressType, DataType>
    {
        constexpr explicit R_control(::RMF::Block<AddressType, DataType> const* parent, uint32_t offset, std::string_view name) : Register(parent, offset, name) {}
        ::RMF::Field<AddressType, DataType> div{ this, 0, 4, "div" };
        ::RMF::Field<AddressType, DataType> mult{ this, 4, 4, "nult" };
        ::RMF::Field<AddressType, DataType> active{ this, 8, 1, "active" };
    };
    R_control ctl{ this, 0x10, "ctl" };
```
The two definitions can be combined into one, however this is not as useful as one would hope because the Register struct still needs a name, in order to name the constructor.

### Field
`RMF::Field` provides a class for fields.
It is most likely used directly, though users can subclass it to add functionality.

Fields provide a number of accessors to help manipulate registers:

- `constexpr uint8_t offset() const` returns the offset within the register (the bit position of the Least Significant Bit)
- `constexpr uint8_t size() const` returns the width of the field (in bits)
- `constexpr DType dataMask() const` returns a mask that can be used to mask field-aligned data values, ie, for a 5-bit field, this would return `0x1F`
- `constexpr DType regMask() const` returns a mask that can be used to mask register-aligned data values, ie, for a 5-bit field with an offset of 3, this would return `0xF8`
- `constexpr DType extract(DType const reg_val) const` is given the value of the whole containing register and returns just the field value, shifted down to bit 0
    Equivalent to `(reg_val >> offset()) & dataMask()`
- `constexpr DType regVal(DType val) const` is given the value of the field and returns that field inserted into the proper position
    Equivalent to `(val & dataMask()) << offset()`
- `constexpr void insert(DType& reg_val, DType val) const` is given the value of the field and a reference to the register value and overwrites the field's position with the supplied data
    Equivalent to `reg_val = reg_val & ~regMak() | regVal(val)`

Subclasses should provide constructors consistent with the `RMF::Field` constructor:
```c++
constexpr explicit Field(::RMF::Register<AddressType, DataType> const* parent, uint8_t const offset, uint8_t const size, std::string_view name) : Register(parent, offset, size, name) {}
```
