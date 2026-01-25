# C++ Coding Guidelines

This document defines the coding standards for the reference project. These guidelines are based on the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

---

## Table of Contents

1. [Naming Conventions](#naming-conventions)
2. [File Structure](#file-structure)
3. [Header Files](#header-files)
4. [Formatting](#formatting)
5. [Classes](#classes)
6. [Functions](#functions)
7. [Variables](#variables)
8. [Modern C++ Usage](#modern-c-usage)
9. [Comments and Documentation](#comments-and-documentation) ← **Doxygen Guidelines**
10. [Best Practices](#best-practices)

---

## Naming Conventions

### Quick Reference

| Element          | Convention          | Example                          |
|------------------|---------------------|----------------------------------|
| Files            | `snake_case`        | `user_account.cc`, `user_account.h` |
| Namespaces       | `snake_case`        | `namespace my_project`           |
| Classes          | `PascalCase`        | `UserAccount`, `HttpRequest`     |
| Structs          | `PascalCase`        | `TableInfo`, `UrlProperties`     |
| Enums            | `PascalCase`        | `enum class ErrorCode`           |
| Enum values      | `kPascalCase`       | `kSuccess`, `kNotFound`          |
| Functions        | `PascalCase`        | `ProcessRequest()`, `ValidateInput()` |
| Accessors        | `snake_case`        | `user_id()`, `set_user_id()`     |
| Local variables  | `snake_case`        | `table_name`, `num_errors`       |
| Class members    | `snake_case_`       | `user_id_`, `is_valid_`          |
| Struct members   | `snake_case`        | `num_entries`, `table_name`      |
| Constants        | `kPascalCase`       | `kMaxBufferSize`, `kDefaultTimeout` |
| Global variables | `g_snake_case`      | `g_factory_instance`             |
| Macros           | `UPPER_SNAKE_CASE`  | `MY_PROJECT_VERSION`, `DISALLOW_COPY` |

### Naming Principles

- **Be descriptive**: Favor clarity over brevity
- **Avoid abbreviations**: Unless widely understood (`num`, `id`, `http`)
- **No Hungarian notation**: Don't prefix types (e.g., `strName`, `iCount`)

```cpp
// Good
int price_count_reader;
int num_errors;
int num_dns_connections;

// Bad
int n;                    // Meaningless
int nerr;                 // Ambiguous abbreviation
int pc_reader;            // Unclear abbreviation
int cstmr_id;             // Deletes internal letters
```

---

## File Structure

### File Naming

- Use `snake_case` for all file names
- Source files: `.cc` extension
- Header files: `.h` extension
- Match file name to primary class: `class UserAccount` → `user_account.h`, `user_account.cc`

```
project/
├── src/
│   ├── core/
│   │   ├── user_account.h
│   │   ├── user_account.cc
│   │   ├── payment_processor.h
│   │   └── payment_processor.cc
│   └── utils/
│       ├── string_utils.h
│       └── string_utils.cc
└── tests/
    ├── user_account_test.cc
    └── payment_processor_test.cc
```

### Source File Layout

```cpp
// 1. Copyright notice (if applicable)

// 2. Header file for this source file
#include "project/path/my_class.h"

// 3. Blank line

// 4. C system headers
#include <sys/types.h>
#include <unistd.h>

// 5. Blank line

// 6. C++ standard library headers
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

// 7. Blank line

// 8. Other libraries' headers
#include <boost/optional.hpp>

// 9. Blank line

// 10. Project headers
#include "project/base/logging.h"
#include "project/utils/string_utils.h"

namespace my_project {

// Implementation...

}  // namespace my_project
```

---

## Header Files

### Header Guards

Every header file must have a `#define` guard to prevent multiple inclusion:

```cpp
#ifndef PROJECT_PATH_FILE_H_
#define PROJECT_PATH_FILE_H_

// Header contents...

#endif  // PROJECT_PATH_FILE_H_
```

Format: `<PROJECT>_<PATH>_<FILE>_H_`

### Self-Contained Headers

Headers must compile independently. Include all necessary dependencies.

```cpp
// user_account.h
#ifndef MY_PROJECT_CORE_USER_ACCOUNT_H_
#define MY_PROJECT_CORE_USER_ACCOUNT_H_

#include <string>  // Required for std::string
#include <vector>  // Required for std::vector

namespace my_project {

class UserAccount {
 public:
    explicit UserAccount(std::string name);
    // ...
 private:
    std::string name_;
    std::vector<int> order_ids_;
};

}  // namespace my_project

#endif  // MY_PROJECT_CORE_USER_ACCOUNT_H_
```

### Include What You Use

Include headers for all symbols you reference. Don't rely on transitive includes.

### Forward Declarations

Avoid forward declarations when possible. Prefer `#include`.

---

## Formatting

### Indentation

- Use **2 spaces** for indentation
- **Never use tabs**

### Line Length

- Maximum **80 characters** per line
- Exceptions: long URLs, paths, or string literals that can't be split

### Braces

Opening brace on the same line:

```cpp
// Good
if (condition) {
    DoSomething();
}

void MyFunction() {
    // ...
}

class MyClass {
 public:
    // ...
};

// Bad
if (condition)
{
    DoSomething();
}
```

### Spaces

```cpp
// Around operators
int x = 5 + 3;
if (x == 8) { }

// After keywords
if (condition) { }
for (int i = 0; i < 10; ++i) { }
while (running) { }

// No space before parentheses in function calls
DoSomething(arg1, arg2);

// No spaces inside parentheses
if (condition) { }     // Good
if ( condition ) { }   // Bad

// No spaces inside angle brackets
std::vector<int> numbers;      // Good
std::vector< int > numbers;    // Bad
```

### Class Format

```cpp
class MyClass {
 public:
    MyClass();
    explicit MyClass(int value);
    ~MyClass();

    void PublicMethod();
    int public_accessor() const { return value_; }
    void set_public_accessor(int value) { value_ = value; }

 protected:
    void ProtectedMethod();

 private:
    void PrivateMethod();

    int value_;
    std::string name_;
};
```

### Namespace Format

```cpp
namespace my_project {
namespace internal {

// No indentation for namespace contents
void MyFunction() {
    // ...
}

}  // namespace internal
}  // namespace my_project
```

---

## Classes

### Constructor Initialization

Use member initializer lists. Order must match declaration order.

```cpp
class UserAccount {
 public:
    UserAccount(int id, std::string name, double balance)
        : id_(id),
          name_(std::move(name)),
          balance_(balance),
          is_active_(true) {}

 private:
    int id_;
    std::string name_;
    double balance_;
    bool is_active_;
};
```

### Explicit Constructors

Mark single-argument constructors `explicit` to prevent implicit conversions:

```cpp
class MyClass {
 public:
    explicit MyClass(int value);           // Good
    MyClass(int x, int y);                 // No explicit needed (multiple args)
    explicit MyClass(std::string name);    // Good
};
```

### Copy and Move Operations

Be explicit about copyability and movability:

```cpp
// Copyable and movable (default)
class Copyable {
 public:
    Copyable(const Copyable&) = default;
    Copyable& operator=(const Copyable&) = default;
    Copyable(Copyable&&) = default;
    Copyable& operator=(Copyable&&) = default;
};

// Move-only
class MoveOnly {
 public:
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;
};

// Non-copyable and non-movable
class NonCopyable {
 public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
```

### Structs vs Classes

- Use `struct` for passive data holders with no invariants
- Use `class` when there are invariants, methods, or access control

```cpp
// Struct: simple data holder
struct Point {
    int x;
    int y;
};

// Class: has invariants and behavior
class Rectangle {
 public:
    Rectangle(int width, int height);
    int Area() const;

 private:
    int width_;
    int height_;
};
```

---

## Functions

### Function Naming

```cpp
// Regular functions: PascalCase
void ProcessUserInput();
bool ValidateEmail(const std::string& email);
std::string FormatDate(const Date& date);

// Accessors: snake_case matching member variable
class User {
 public:
    int user_id() const { return user_id_; }
    void set_user_id(int id) { user_id_ = id; }

    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

 private:
    int user_id_;
    std::string name_;
};
```

### Function Length

- Prefer small, focused functions
- If a function exceeds ~40 lines, consider splitting it

### Parameter Ordering

Input parameters first, then output parameters:

```cpp
// Good: inputs before outputs
void ProcessData(const std::string& input,
                 const Options& options,
                 Result* output);
```

### Parameter Passing

| Type | How to Pass |
|------|-------------|
| Primitive types (`int`, `bool`, etc.) | By value |
| Input objects | `const` reference |
| Output objects | Pointer (non-null) |
| Optional output | Pointer (nullable) |
| Sink parameters (takes ownership) | By value, then `std::move` |

```cpp
void ProcessData(int count,                      // Primitive: by value
                 const std::string& name,        // Input: const ref
                 const Options& options,         // Input: const ref
                 Result* result);                // Output: pointer

void SetName(std::string name) {                 // Sink: by value
    name_ = std::move(name);
}
```

---

## Variables

### Initialization

Initialize variables at declaration:

```cpp
// Good
int count = 0;
std::string name = "default";
std::vector<int> numbers = {1, 2, 3};

// Bad
int count;
count = 0;
```

### Scope

Declare variables in the narrowest scope possible:

```cpp
// Good
for (const auto& item : items) {
    std::string formatted = FormatItem(item);
    Process(formatted);
}

// Bad
std::string formatted;
for (const auto& item : items) {
    formatted = FormatItem(item);
    Process(formatted);
}
```

### Constants

```cpp
// Compile-time constants
constexpr int kMaxBufferSize = 1024;
constexpr double kPi = 3.14159265358979323846;

// Class constants
class MyClass {
 public:
    static constexpr int kDefaultTimeout = 30;
    static const std::string kDefaultName;  // Define in .cc file
};

// Enum constants
enum class HttpStatus {
    kOk = 200,
    kNotFound = 404,
    kInternalError = 500
};
```

---

## Modern C++ Usage

### Use `nullptr`

```cpp
// Good
int* ptr = nullptr;
if (ptr == nullptr) { }

// Bad
int* ptr = NULL;
int* ptr = 0;
```

### Use `auto` Appropriately

Use `auto` when it improves readability:

```cpp
// Good: type is obvious or verbose
auto it = my_map.find(key);
auto ptr = std::make_unique<MyClass>();
auto lambda = [](int x) { return x * 2; };

// Good: avoids repetition
std::vector<std::pair<std::string, int>> pairs;
for (const auto& pair : pairs) { }

// Bad: type is not obvious
auto result = ProcessData();  // What type is result?
```

### Range-Based For Loops

```cpp
// Good
for (const auto& item : items) {
    Process(item);
}

// For modification
for (auto& item : items) {
    item.Update();
}

// For primitives
for (int value : numbers) {
    Process(value);
}
```

### Smart Pointers

Prefer smart pointers over raw pointers for ownership:

```cpp
// Unique ownership
std::unique_ptr<MyClass> ptr = std::make_unique<MyClass>();

// Shared ownership (use sparingly)
std::shared_ptr<MyClass> shared = std::make_shared<MyClass>();

// Non-owning reference: use raw pointer or reference
void Process(MyClass* obj);      // Non-owning, nullable
void Process(MyClass& obj);      // Non-owning, non-null
```

### Use `override` and `final`

```cpp
class Base {
 public:
    virtual void Process();
    virtual void Update() = 0;
};

class Derived : public Base {
 public:
    void Process() override;           // Good: explicit override
    void Update() override final;      // Good: cannot be overridden further
};
```

### Prefer `enum class`

```cpp
// Good: scoped enum
enum class Color {
    kRed,
    kGreen,
    kBlue
};
Color c = Color::kRed;

// Avoid: unscoped enum
enum Color {
    RED,    // Pollutes namespace
    GREEN,
    BLUE
};
```

---

## Comments and Documentation

Our project uses **Doxygen** for automatic documentation generation. Follow these guidelines to ensure maximum extraction with minimal effort.

### Documentation Style

Use `/** */` for documentation comments (Doxygen-style), and `//` for implementation comments.

```cpp
/** @brief This is a documentation comment (extracted by Doxygen) */

// This is an implementation comment (not extracted)
```

### File Documentation

Every header file **must** have a file documentation block:

```cpp
/**
 * @file user_account.h
 * @brief User account management and authentication.
 * @author Your Name
 * @date 2025-01-24
 *
 * @details This module provides user account functionality including
 *          creation, authentication, and session management.
 *
 * @copyright Copyright (c) 2025 Your Company
 *
 * @req REQ-USER-001 User account management
 * @req REQ-AUTH-001 Authentication system
 */

#ifndef MY_PROJECT_USER_ACCOUNT_H_
#define MY_PROJECT_USER_ACCOUNT_H_
```

### Class Documentation

Every class **must** have a documentation block:

```cpp
/**
 * @class UserAccount
 * @brief Manages user account data and authentication state.
 *
 * @details The UserAccount class handles user credentials, session tokens,
 *          and account metadata. It is thread-safe for concurrent access.
 *
 * @invariant user_id_ > 0 after successful construction
 * @invariant state_ is always a valid AccountState
 *
 * @req REQ-USER-001
 * @design DES-USER-001
 *
 * @par Example Usage
 * @code
 *     UserAccount account("john_doe");
 *     if (account.Authenticate(password)) {
 *         auto token = account.CreateSession();
 *     }
 * @endcode
 *
 * @see SessionManager
 * @see AuthenticationService
 */
class UserAccount {
```

### Function Documentation

Every public function **must** be documented. Private functions **should** be documented.

```cpp
/**
 * @brief Authenticates the user with the provided credentials.
 *
 * @details Validates the password against the stored hash and updates
 *          the last login timestamp on success. Implements rate limiting
 *          to prevent brute force attacks.
 *
 * @param[in] password The plaintext password to verify
 * @param[out] error_code Error code if authentication fails (optional)
 *
 * @return true if authentication successful, false otherwise
 *
 * @retval true Authentication successful, session created
 * @retval false Authentication failed, see error_code for details
 *
 * @throws std::invalid_argument if password is empty
 * @throws ConnectionException if database is unavailable
 *
 * @pre password.length() >= kMinPasswordLength
 * @post If returns true: IsAuthenticated() == true
 * @post If returns true: last_login_ is updated
 *
 * @note This function may block for up to 5 seconds
 * @warning Do not call from interrupt context
 *
 * @req REQ-AUTH-002
 * @test TC-AUTH-001, TC-AUTH-002, TC-AUTH-003
 * @complexity O(1) average, O(n) worst case for hash verification
 *
 * @since 1.0.0
 */
bool Authenticate(const std::string& password, ErrorCode* error_code = nullptr);
```

### Member Variable Documentation

Document member variables, especially non-obvious ones:

```cpp
class UserAccount {
 private:
    int user_id_;                    ///< Unique user identifier (database primary key)
    std::string username_;           ///< Login username (immutable after creation)
    std::string password_hash_;      ///< BCrypt hash of user password
    AccountState state_;             ///< Current account state @see AccountState
    std::atomic<int> login_attempts_;///< Failed login counter (reset on success)

    /**
     * @brief Session tokens for active sessions.
     * @details Maps session ID to expiration timestamp.
     *          Tokens older than kSessionTimeout are automatically purged.
     */
    std::unordered_map<std::string, Timestamp> sessions_;
};
```

### Enum Documentation

```cpp
/**
 * @enum AccountState
 * @brief Represents the lifecycle state of a user account.
 *
 * @req REQ-USER-002
 */
enum class AccountState {
    kPending,      ///< Account created, awaiting email verification
    kActive,       ///< Account verified and in good standing
    kSuspended,    ///< Account temporarily disabled by admin
    kLocked,       ///< Account locked due to failed login attempts
    kDeleted       ///< Account marked for deletion (soft delete)
};
```

### Struct Documentation

```cpp
/**
 * @struct LoginResult
 * @brief Contains the result of an authentication attempt.
 *
 * @req REQ-AUTH-003
 */
struct LoginResult {
    bool success;           ///< True if authentication succeeded
    ErrorCode error;        ///< Error code if success is false
    std::string token;      ///< Session token if success is true
    Timestamp expires_at;   ///< Token expiration timestamp
};
```

### Traceability Tags

Use these custom tags for PLM traceability:

| Tag | Usage | Description |
|-----|-------|-------------|
| `@req{ID}` | `@req REQ-USER-001` | Links to requirement |
| `@test{ID}` | `@test TC-AUTH-001` | Links to test case |
| `@design{ID}` | `@design DES-USER-001` | Links to design element |
| `@safety{ID}` | `@safety SAF-001` | Links to safety requirement |

```cpp
/**
 * @brief Validates user input for SQL injection.
 *
 * @req REQ-SEC-001
 * @req REQ-SEC-002
 * @test TC-SEC-001, TC-SEC-002
 * @design DES-SEC-001
 * @safety SAF-INPUT-001
 */
bool ValidateInput(const std::string& input);
```

### Common Doxygen Tags Reference

| Tag | Purpose | Example |
|-----|---------|---------|
| `@brief` | Short description (required) | `@brief Calculates the sum.` |
| `@details` | Extended description | `@details This function...` |
| `@param[in]` | Input parameter | `@param[in] value The input value` |
| `@param[out]` | Output parameter | `@param[out] result The output` |
| `@param[in,out]` | Input/output parameter | `@param[in,out] buffer Modified buffer` |
| `@return` | Return value description | `@return The calculated sum` |
| `@retval` | Specific return value | `@retval nullptr if not found` |
| `@throws` | Exception thrown | `@throws std::runtime_error on failure` |
| `@pre` | Precondition | `@pre input != nullptr` |
| `@post` | Postcondition | `@post size() > 0` |
| `@invariant` | Class invariant | `@invariant count_ >= 0` |
| `@note` | Important note | `@note Thread-safe` |
| `@warning` | Warning message | `@warning Not reentrant` |
| `@deprecated` | Deprecated item | `@deprecated Use NewFunc() instead` |
| `@see` | Cross-reference | `@see RelatedClass` |
| `@since` | Version introduced | `@since 2.0.0` |
| `@todo` | TODO item | `@todo Implement caching` |
| `@bug` | Known bug | `@bug Fails on empty input` |
| `@code` | Code example start | `@code ... @endcode` |

### Code Examples in Documentation

```cpp
/**
 * @brief Parses a configuration file.
 *
 * @par Example
 * @code
 *     ConfigParser parser;
 *     auto config = parser.Parse("config.yaml");
 *     if (config.IsValid()) {
 *         int port = config.GetInt("server.port");
 *     }
 * @endcode
 *
 * @par Error Handling
 * @code
 *     try {
 *         auto config = parser.Parse("config.yaml");
 *     } catch (const ParseException& e) {
 *         LOG(ERROR) << "Parse failed: " << e.what();
 *     }
 * @endcode
 */
Config Parse(const std::string& filename);
```

### Grouping Related Items

Use groups to organize related classes/functions:

```cpp
/**
 * @defgroup Authentication Authentication Module
 * @brief Classes and functions for user authentication.
 * @{
 */

/** @brief Authenticates users */
class Authenticator { };

/** @brief Manages sessions */
class SessionManager { };

/** @} */ // end of Authentication group
```

### Design for Diagram Generation

To ensure **clang-uml** generates useful sequence and class diagrams:

#### 1. Use Clear Method Calls (Not Lambdas for Key Logic)

```cpp
// Good: Clear call chain for sequence diagrams
void ProcessOrder(const Order& order) {
    validator_.Validate(order);
    calculator_.CalculateTotal(order);
    repository_.Save(order);
    notifier_.SendConfirmation(order);
}

// Bad: Lambda hides the call chain
void ProcessOrder(const Order& order) {
    auto process = [&]() { /* all logic here */ };
    process();
}
```

#### 2. Prefer Composition Over Deep Inheritance

```cpp
// Good: Clear relationships for class diagrams
class OrderService {
 private:
    OrderValidator validator_;      ///< Validates orders
    PriceCalculator calculator_;    ///< Calculates prices
    OrderRepository repository_;    ///< Persists orders
};

// Avoid: Deep inheritance hierarchies are harder to diagram
class OrderService : public BaseService<Order, OrderValidator> { };
```

#### 3. Use Interfaces for Key Dependencies

```cpp
/**
 * @interface IPaymentGateway
 * @brief Interface for payment processing.
 */
class IPaymentGateway {
 public:
    virtual ~IPaymentGateway() = default;
    virtual PaymentResult Process(const Payment& payment) = 0;
};

/**
 * @class PaymentService
 * @brief Processes payments through configured gateway.
 */
class PaymentService {
 public:
    explicit PaymentService(IPaymentGateway* gateway);
 private:
    IPaymentGateway* gateway_;  ///< Payment gateway implementation
};
```

#### 4. Keep Functions Focused (Single Responsibility)

```cpp
// Good: Each function is a clear step (good sequence diagrams)
void CompleteCheckout(Cart& cart) {
    ValidateCart(cart);
    auto order = CreateOrder(cart);
    ProcessPayment(order);
    SendConfirmation(order);
    UpdateInventory(order);
}

// Bad: Monolithic function (poor sequence diagrams)
void CompleteCheckout(Cart& cart) {
    // 200 lines of mixed logic
}
```

### Inline Comments

Use inline comments sparingly for non-obvious code:

```cpp
// Compute the average, avoiding division by zero.
double average = count > 0 ? total / count : 0.0;

int timeout = 30;  // seconds

// SAFETY: Pointer validated by caller (see contract)
data->Process();
```

### TODO/FIXME Comments

```cpp
// TODO(username): Implement retry logic with exponential backoff.
// TODO(REQ-123): Add validation per requirement REQ-123.
// FIXME(BUG-456): This causes memory leak on early return.
// HACK: Workaround for library bug, remove after v2.0 upgrade.
```

### What NOT to Document

Don't document the obvious:

```cpp
// Bad: Obvious from the code
/** @brief Gets the name */
std::string name() const { return name_; }

/** @brief Sets the name */
void set_name(std::string name) { name_ = std::move(name); }

// Good: Only document if there's something non-obvious
/**
 * @brief Gets the display name.
 * @return Formatted as "LastName, FirstName" or username if name not set.
 */
std::string display_name() const;
```

### Minimum Documentation Checklist

| Element | Required | Recommended |
|---------|----------|-------------|
| File header | ✓ `@file`, `@brief` | `@author`, `@date`, `@req` |
| Class | ✓ `@class`, `@brief` | `@details`, `@req`, `@see` |
| Public function | ✓ `@brief`, `@param`, `@return` | `@throws`, `@pre`, `@post`, `@req` |
| Private function | | `@brief` |
| Member variable | | `///< brief description` |
| Enum | ✓ `@enum`, `@brief` | Document each value |
| Struct | ✓ `@struct`, `@brief` | Document each member |

---

## Best Practices

### Error Handling

- Check return values
- Use exceptions sparingly (Google style avoids them)
- Document error conditions

### Const Correctness

- Mark methods `const` when they don't modify state
- Use `const` references for input parameters
- Declare variables `const` when possible

```cpp
class MyClass {
 public:
    int GetValue() const;                         // Const method
    void Process(const std::string& input);       // Const input
};

void Function() {
    const int max_iterations = 100;               // Const variable
}
```

### Avoid Magic Numbers

```cpp
// Bad
if (status == 404) { }
sleep(30);

// Good
constexpr int kHttpNotFound = 404;
constexpr int kRetryDelaySeconds = 30;

if (status == kHttpNotFound) { }
sleep(kRetryDelaySeconds);
```

### Include Order Enforcement

Use tools like `clang-format` and `include-what-you-use` to enforce these rules automatically.

---

## Tools

### clang-format

Create a `.clang-format` file in your project root:

```yaml
BasedOnStyle: Google
ColumnLimit: 80
IndentWidth: 2
```

Run with:
```bash
clang-format -i src/*.cc src/*.h
```

### cpplint

Use `cpplint` to check style compliance:

```bash
cpplint --recursive src/
```

---

## References

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Doxygen Special Commands](https://www.doxygen.nl/manual/commands.html)
- [clang-uml Documentation](https://github.com/bkryza/clang-uml)
- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)

---

*Last updated: January 2025*
