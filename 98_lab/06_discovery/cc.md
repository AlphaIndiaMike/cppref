
## Complete Architecture Layers

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ 1. USER INTERFACE (View)                                                     │
│    - Displays ViewModel                                                      │
│    - Captures user input                                                     │
│    - Knows NOTHING about business logic                                      │
└──────────────────────────────────────────────────────────────────────────────┘
        │ user action                          ▲ ViewModel
        ▼                                      │
┌──────────────────────────────────────────────────────────────────────────────┐
│ 2. CONTROLLER                               3. PRESENTER                     │
│    - Adapts UI input → Request                 - Implements IOutputPort      │
│    - Invokes Interactor                        - Adapts Response → ViewModel │
└──────────────────────────────────────────────────────────────────────────────┘
        │ Request                              ▲ ResponseModel
        ▼                                      │
┌──────────────────────────────────────────────────────────────────────────────┐
│ 4. INTERACTOR (Use Case)                                                     │
│    - Orchestrates business logic                                             │
│    - Works with Entities                                                     │
│    - Calls Repository interface                                              │
│    - Pushes to OutputPort                                                    │
└──────────────────────────────────────────────────────────────────────────────┘
        │ Entity                               ▲ Entity
        ▼                                      │
┌──────────────────────────────────────────────────────────────────────────────┐
│ 5. REPOSITORY INTERFACE (defined in Use Case layer)                          │
│    - Abstract contract for data access                                       │
└──────────────────────────────────────────────────────────────────────────────┘
        │                                      ▲
        ▼                                      │
┌──────────────────────────────────────────────────────────────────────────────┐
│ 6. DATA ADAPTER (Repository Implementation)                                  │
│    - Implements repository interface                                         │
│    - Converts Entity ←→ DataModel (DTO/ORM)                                  │
│    - Handles data source specifics                                           │
└──────────────────────────────────────────────────────────────────────────────┘
        │ DataModel / SQL                      ▲ Raw Data
        ▼                                      │
┌──────────────────────────────────────────────────────────────────────────────┐
│ 7. DATA (Database / File / External API)                                     │
│    - Actual storage mechanism                                                │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## Complete Code: Layer by Layer

### Layer 7: Data (Database)

```cpp
// ============================================================
// LAYER 7: DATA - Raw storage (e.g., SQLite, file, etc.)
// This is external - we just interact with it
// ============================================================

// Imagine this is SQLite, PostgreSQL, a file, etc.
// The Data Adapter will talk to this
```

---

### Layer 6: Data Adapter (Repository Implementation)

```cpp
// ============================================================
// LAYER 6: DATA ADAPTER - Repository Implementation
// File: infrastructure/repositories/SqlUserRepository.hpp
// ============================================================

#include "entities/User.hpp"
#include "use_cases/interfaces/IUserRepository.hpp"
#include <sqlite3.h>

namespace Infrastructure {

// Data Transfer Object - matches database schema
struct UserRow {
    std::string id;
    std::string email;
    std::string name;
    int64_t created_at_unix;
};

class SqlUserRepository : public UseCases::IUserRepository {
public:
    explicit SqlUserRepository(const std::string& dbPath) {
        sqlite3_open(dbPath.c_str(), &m_db);
    }

    ~SqlUserRepository() {
        sqlite3_close(m_db);
    }

    // Implements interface - returns ENTITY, not raw data
    std::optional<Entities::User> findById(const std::string& id) override {
        UserRow row;
        bool found = queryById(id, row);  // raw SQL query

        if (!found) return std::nullopt;

        // DATA MODEL → ENTITY conversion
        return mapToEntity(row);
    }

    std::optional<Entities::User> findByEmail(const std::string& email) override {
        UserRow row;
        bool found = queryByEmail(email, row);

        if (!found) return std::nullopt;

        return mapToEntity(row);
    }

    Entities::User save(const Entities::User& user) override {
        // ENTITY → DATA MODEL conversion
        UserRow row = mapToRow(user);

        // Raw SQL insert/update
        executeInsert(row);

        return user;
    }

    bool remove(const std::string& id) override {
        return executeDelete(id);
    }

    std::vector<Entities::User> findAll() override {
        std::vector<UserRow> rows = queryAll();
        std::vector<Entities::User> users;

        for (const auto& row : rows) {
            users.push_back(mapToEntity(row));
        }

        return users;
    }

private:
    sqlite3* m_db;

    // ---- Mapping functions ----

    Entities::User mapToEntity(const UserRow& row) {
        return Entities::User{
            .id = row.id,
            .email = row.email,
            .name = row.name,
            .createdAt = std::chrono::system_clock::from_time_t(row.created_at_unix)
        };
    }

    UserRow mapToRow(const Entities::User& entity) {
        return UserRow{
            .id = entity.id,
            .email = entity.email,
            .name = entity.name,
            .created_at_unix = std::chrono::system_clock::to_time_t(entity.createdAt)
        };
    }

    // ---- Raw SQL operations ----

    bool queryById(const std::string& id, UserRow& outRow) {
        // SELECT * FROM users WHERE id = ?
        // ... sqlite3 code ...
        return true;
    }

    bool queryByEmail(const std::string& email, UserRow& outRow) {
        // SELECT * FROM users WHERE email = ?
        return true;
    }

    std::vector<UserRow> queryAll() {
        // SELECT * FROM users
        return {};
    }

    void executeInsert(const UserRow& row) {
        // INSERT INTO users (id, email, name, created_at) VALUES (?, ?, ?, ?)
    }

    bool executeDelete(const std::string& id) {
        // DELETE FROM users WHERE id = ?
        return true;
    }
};

} // namespace Infrastructure
```

---

### Layer 5: Repository Interface (Defined in Use Case Layer)

```cpp
// ============================================================
// LAYER 5: REPOSITORY INTERFACE
// File: use_cases/interfaces/IUserRepository.hpp
// Lives in USE CASE layer - implementation is in Infrastructure
// ============================================================

#pragma once
#include "entities/User.hpp"
#include <optional>
#include <vector>

namespace UseCases {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    virtual std::optional<Entities::User> findById(const std::string& id) = 0;
    virtual std::optional<Entities::User> findByEmail(const std::string& email) = 0;
    virtual std::vector<Entities::User> findAll() = 0;
    virtual Entities::User save(const Entities::User& user) = 0;
    virtual bool remove(const std::string& id) = 0;
};

} // namespace UseCases
```

---

### Layer 4: Entity (Domain Core)

```cpp
// ============================================================
// LAYER 4: ENTITY - Core business object
// File: entities/User.hpp
// Pure domain logic - NO dependencies on outer layers
// ============================================================

#pragma once
#include <string>
#include <chrono>
#include <regex>

namespace Entities {

class User {
public:
    std::string id;
    std::string email;
    std::string name;
    std::chrono::system_clock::time_point createdAt;

    // ---- Business Rules Live Here ----

    bool isValidEmail() const {
        static const std::regex emailRegex(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
        return std::regex_match(email, emailRegex);
    }

    bool isActive() const {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::hours>(now - createdAt);
        return age.count() < 24 * 365;  // Active if less than 1 year old
    }

    std::string displayName() const {
        return name.empty() ? email : name;
    }
};

} // namespace Entities
```

---

### Layer 3: Interactor (Use Case)

```cpp
// ============================================================
// LAYER 3: INTERACTOR - Use Case
// File: use_cases/create_user/CreateUserUseCase.hpp
// ============================================================

#pragma once
#include "entities/User.hpp"
#include "use_cases/interfaces/IUserRepository.hpp"
#include <memory>

namespace UseCases::CreateUser {

// ---- Input ----
struct Request {
    std::string email;
    std::string name;
};

// ---- Output Data ----
struct ResponseModel {
    std::string userId;
    std::string email;
    std::string displayName;
};

struct ErrorModel {
    std::string code;
    std::string message;
};

// ---- Output Port Interface ----
class IOutputPort {
public:
    virtual ~IOutputPort() = default;
    virtual void presentSuccess(const ResponseModel& response) = 0;
    virtual void presentError(const ErrorModel& error) = 0;
};

// ---- Interactor Implementation ----
class Interactor {
public:
    explicit Interactor(std::shared_ptr<IUserRepository> userRepo)
        : m_userRepo(std::move(userRepo)) {}

    void execute(const Request& request, IOutputPort& outputPort) {
        // 1. Create Entity
        Entities::User user;
        user.id = generateUuid();
        user.email = request.email;
        user.name = request.name;
        user.createdAt = std::chrono::system_clock::now();

        // 2. Use Entity's business rules
        if (!user.isValidEmail()) {
            outputPort.presentError({"INVALID_EMAIL", "Email format is invalid"});
            return;
        }

        // 3. Check business rule via repository
        if (m_userRepo->findByEmail(request.email).has_value()) {
            outputPort.presentError({"DUPLICATE_EMAIL", "Email already registered"});
            return;
        }

        // 4. Persist via repository interface (goes to Data Adapter)
        Entities::User savedUser = m_userRepo->save(user);

        // 5. Create response model and push to output port
        ResponseModel response{
            .userId = savedUser.id,
            .email = savedUser.email,
            .displayName = savedUser.displayName()  // uses Entity method
        };

        outputPort.presentSuccess(response);
    }

private:
    std::shared_ptr<IUserRepository> m_userRepo;

    std::string generateUuid() {
        // UUID generation logic
        return "uuid-" + std::to_string(std::rand());
    }
};

} // namespace UseCases::CreateUser
```

---

### Layer 2: Controller & Presenter

```cpp
// ============================================================
// LAYER 2A: CONTROLLER
// File: adapters/controllers/CreateUserController.hpp
// ============================================================

#pragma once
#include "use_cases/create_user/CreateUserUseCase.hpp"

namespace Adapters {

class CreateUserController {
public:
    CreateUserController(
        std::shared_ptr<UseCases::CreateUser::Interactor> interactor,
        std::shared_ptr<UseCases::CreateUser::IOutputPort> presenter)
        : m_interactor(std::move(interactor))
        , m_presenter(std::move(presenter)) {}

    // Receives raw input from UI, adapts to Request
    void onCreateUserSubmit(const std::string& emailInput, const std::string& nameInput) {
        // Adapt/sanitize UI input → Request
        UseCases::CreateUser::Request request{
            .email = trim(emailInput),
            .name = trim(nameInput)
        };

        // Execute use case - result goes to presenter via OutputPort
        m_interactor->execute(request, *m_presenter);
    }

private:
    std::string trim(const std::string& str) {
        // Trim whitespace
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    std::shared_ptr<UseCases::CreateUser::Interactor> m_interactor;
    std::shared_ptr<UseCases::CreateUser::IOutputPort> m_presenter;
};

} // namespace Adapters
```

```cpp
// ============================================================
// LAYER 2B: PRESENTER
// File: adapters/presenters/GuiCreateUserPresenter.hpp
// ============================================================

#pragma once
#include "use_cases/create_user/CreateUserUseCase.hpp"
#include <functional>

namespace Adapters {

// ViewModel for the UI
struct CreateUserViewModel {
    bool success;
    std::string title;
    std::string message;
    std::string userId;

    // UI-specific flags
    bool showSuccessIcon;
    bool showErrorIcon;
    std::string backgroundColor;
};

class GuiCreateUserPresenter : public UseCases::CreateUser::IOutputPort {
public:
    using ViewCallback = std::function<void(const CreateUserViewModel&)>;

    void setViewCallback(ViewCallback callback) {
        m_viewCallback = std::move(callback);
    }

    // Called by Interactor on success
    void presentSuccess(const UseCases::CreateUser::ResponseModel& response) override {
        // Transform ResponseModel → ViewModel
        CreateUserViewModel vm{
            .success = true,
            .title = "Welcome!",
            .message = "Account created for " + response.displayName,
            .userId = response.userId,
            .showSuccessIcon = true,
            .showErrorIcon = false,
            .backgroundColor = "#E8F5E9"  // light green
        };

        notifyView(vm);
    }

    // Called by Interactor on failure
    void presentError(const UseCases::CreateUser::ErrorModel& error) override {
        CreateUserViewModel vm{
            .success = false,
            .title = "Registration Failed",
            .message = formatErrorMessage(error),
            .userId = "",
            .showSuccessIcon = false,
            .showErrorIcon = true,
            .backgroundColor = "#FFEBEE"  // light red
        };

        notifyView(vm);
    }

private:
    ViewCallback m_viewCallback;

    void notifyView(const CreateUserViewModel& vm) {
        if (m_viewCallback) {
            m_viewCallback(vm);
        }
    }

    std::string formatErrorMessage(const UseCases::CreateUser::ErrorModel& error) {
        if (error.code == "INVALID_EMAIL") {
            return "Please enter a valid email address.";
        } else if (error.code == "DUPLICATE_EMAIL") {
            return "This email is already registered. Try logging in instead.";
        }
        return error.message;
    }
};

} // namespace Adapters
```

---

### Layer 1: User Interface (View)

```cpp
// ============================================================
// LAYER 1: USER INTERFACE - View
// File: ui/CreateUserView.hpp
// ============================================================

#pragma once
#include "adapters/presenters/GuiCreateUserPresenter.hpp"
#include "adapters/controllers/CreateUserController.hpp"

namespace UI {

class CreateUserView {
public:
    CreateUserView(
        std::shared_ptr<Adapters::CreateUserController> controller,
        std::shared_ptr<Adapters::GuiCreateUserPresenter> presenter)
        : m_controller(std::move(controller))
    {
        // View subscribes to presenter updates
        presenter->setViewCallback([this](const Adapters::CreateUserViewModel& vm) {
            this->render(vm);
        });
    }

    // Simulates user filling form and clicking submit
    void simulateUserInput() {
        std::cout << "=== Create User Form ===\n";
        std::cout << "Email: ";
        std::getline(std::cin, m_emailInput);

        std::cout << "Name: ";
        std::getline(std::cin, m_nameInput);

        // User clicks submit → notify controller
        onSubmitClicked();
    }

private:
    std::shared_ptr<Adapters::CreateUserController> m_controller;

    // Form state
    std::string m_emailInput;
    std::string m_nameInput;

    // User action → Controller
    void onSubmitClicked() {
        m_controller->onCreateUserSubmit(m_emailInput, m_nameInput);
    }

    // Presenter → View (render ViewModel)
    void render(const Adapters::CreateUserViewModel& vm) {
        std::cout << "\n┌─────────────────────────────────────┐\n";
        std::cout << "│ " << (vm.showSuccessIcon ? "✓" : "✗") << " " << vm.title;
        std::cout << std::string(35 - vm.title.length() - 4, ' ') << "│\n";
        std::cout << "├─────────────────────────────────────┤\n";
        std::cout << "│ " << vm.message;
        std::cout << std::string(35 - vm.message.length() - 2, ' ') << "│\n";
        if (!vm.userId.empty()) {
            std::cout << "│ ID: " << vm.userId;
            std::cout << std::string(35 - vm.userId.length() - 6, ' ') << "│\n";
        }
        std::cout << "└─────────────────────────────────────┘\n";
    }
};

} // namespace UI
```

---

### Composition Root (Main)

```cpp
// ============================================================
// COMPOSITION ROOT - Wires everything together
// File: main.cpp
// ============================================================

#include "entities/User.hpp"
#include "use_cases/create_user/CreateUserUseCase.hpp"
#include "use_cases/interfaces/IUserRepository.hpp"
#include "infrastructure/repositories/SqlUserRepository.hpp"
#include "adapters/controllers/CreateUserController.hpp"
#include "adapters/presenters/GuiCreateUserPresenter.hpp"
#include "ui/CreateUserView.hpp"

int main() {
    // ========== LAYER 7 & 6: Data + Data Adapter ==========
    auto userRepository = std::make_shared<Infrastructure::SqlUserRepository>("app.db");

    // ========== LAYER 4 & 3: Entity + Interactor ==========
    auto createUserInteractor = std::make_shared<UseCases::CreateUser::Interactor>(
        userRepository
    );

    // ========== LAYER 2: Presenter ==========
    auto presenter = std::make_shared<Adapters::GuiCreateUserPresenter>();

    // ========== LAYER 2: Controller ==========
    auto controller = std::make_shared<Adapters::CreateUserController>(
        createUserInteractor,
        presenter
    );

    // ========== LAYER 1: View ==========
    UI::CreateUserView view(controller, presenter);

    // ========== RUN ==========
    view.simulateUserInput();

    return 0;
}
```

---

## Complete Data Flow Diagram

```
USER TYPES: "john@test.com", "John"
            │
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 1. VIEW                                                         │
│    CreateUserView::onSubmitClicked()                            │
│    Raw strings: "john@test.com", "John"                         │
└─────────────────────────────────────────────────────────────────┘
            │
            │ controller->onCreateUserSubmit(email, name)
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. CONTROLLER                                                   │
│    CreateUserController::onCreateUserSubmit()                   │
│    Adapts input → Request{email, name}                          │
└─────────────────────────────────────────────────────────────────┘
            │
            │ interactor->execute(request, outputPort)
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. INTERACTOR                                                   │
│    CreateUserInteractor::execute()                              │
│    - Creates User entity                                        │
│    - Validates via entity.isValidEmail()                        │
│    - Calls repository->save(entity)                             │
└─────────────────────────────────────────────────────────────────┘
            │
            │ repository->save(User entity)
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 4. ENTITY                                                       │
│    User { id, email, name, createdAt }                          │
│    Business methods: isValidEmail(), displayName()              │
└─────────────────────────────────────────────────────────────────┘
            │
            │ passed to repository
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 5. REPOSITORY INTERFACE                                         │
│    IUserRepository::save(User) → User                           │
│    (Contract defined in Use Case layer)                         │
└─────────────────────────────────────────────────────────────────┘
            │
            │ implementation called
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 6. DATA ADAPTER                                                 │
│    SqlUserRepository::save()                                    │
│    - Maps User entity → UserRow (DTO)                           │
│    - Executes INSERT SQL                                        │
│    - Maps result back → User entity                             │
└─────────────────────────────────────────────────────────────────┘
            │
            │ SQL: INSERT INTO users VALUES (...)
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 7. DATABASE                                                     │
│    SQLite / PostgreSQL / File / etc.                            │
│    Stores raw data                                              │
└─────────────────────────────────────────────────────────────────┘
            │
            │ rows affected: 1
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 6. DATA ADAPTER (return path)                                   │
│    Returns User entity to Interactor                            │
└─────────────────────────────────────────────────────────────────┘
            │
            │ User entity returned
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. INTERACTOR (continuation)                                    │
│    - Builds ResponseModel from entity                           │
│    - Calls outputPort.presentSuccess(response)                  │
└─────────────────────────────────────────────────────────────────┘
            │
            │ presenter->presentSuccess(ResponseModel)
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. PRESENTER                                                    │
│    GuiCreateUserPresenter::presentSuccess()                     │
│    - Maps ResponseModel → ViewModel                             │
│    - Calls viewCallback(viewModel)                              │
└─────────────────────────────────────────────────────────────────┘
            │
            │ viewCallback(CreateUserViewModel)
            ▼
┌─────────────────────────────────────────────────────────────────┐
│ 1. VIEW                                                         │
│    CreateUserView::render(ViewModel)                            │
│    Displays: "✓ Welcome! Account created for John"              │
└─────────────────────────────────────────────────────────────────┘
            │
            ▼
        USER SEES RESULT
```

---

## Folder Structure

```
project/
├── src/
│   ├── entities/                    # Layer 4: Domain core
│   │   └── User.hpp
│   │
│   ├── use_cases/                   # Layer 3 & 5: Use cases + interfaces
│   │   ├── interfaces/
│   │   │   └── IUserRepository.hpp  # Repository interface (Layer 5)
│   │   └── create_user/
│   │       └── CreateUserUseCase.hpp
│   │
│   ├── adapters/                    # Layer 2: Controllers & Presenters
│   │   ├── controllers/
│   │   │   └── CreateUserController.hpp
│   │   └── presenters/
│   │       ├── GuiCreateUserPresenter.hpp
│   │       ├── CliCreateUserPresenter.hpp
│   │       └── PdfCreateUserPresenter.hpp
│   │
│   ├── infrastructure/              # Layer 6: Data adapters
│   │   └── repositories/
│   │       ├── SqlUserRepository.hpp
│   │       └── InMemoryUserRepository.hpp
│   │
│   ├── ui/                          # Layer 1: Views
│   │   └── CreateUserView.hpp
│   │
│   └── main.cpp                     # Composition root
│
└── CMakeLists.txt
```

---
Great question! Let me show you the complete integration with SQLite3 vendored.

---

## Updated Folder Structure

```
project/
├── CMakeLists.txt
├── src/
│   ├── entities/
│   │   └── User.hpp
│   │
│   ├── use_cases/
│   │   ├── interfaces/
│   │   │   └── IUserRepository.hpp
│   │   └── create_user/
│   │       └── CreateUserUseCase.hpp
│   │
│   ├── adapters/
│   │   ├── controllers/
│   │   │   └── CreateUserController.hpp
│   │   └── presenters/
│   │       └── GuiCreateUserPresenter.hpp
│   │
│   ├── infrastructure/
│   │   ├── database/                    # <-- Database wrapper lives here
│   │   │   ├── IDatabase.hpp            # Abstract database interface
│   │   │   └── SqliteDatabase.hpp       # SQLite implementation
│   │   │
│   │   └── repositories/
│   │       └── SqlUserRepository.hpp    # Uses SqliteDatabase
│   │
│   ├── ui/
│   │   └── CreateUserView.hpp
│   │
│   └── main.cpp
│
├── vendor/                              # <-- Third-party dependencies
│   └── sqlite3/
│       ├── sqlite3.c                    # SQLite amalgamation
│       ├── sqlite3.h
│       └── CMakeLists.txt               # Build config for SQLite
│
└── tests/
    └── ...
```

---

## Step 1: Download SQLite3 Amalgamation

SQLite3 is distributed as a single `.c` and `.h` file (amalgamation), making it perfect for vendoring.

```bash
# Create vendor directory
mkdir -p vendor/sqlite3
cd vendor/sqlite3

# Download latest amalgamation (check https://sqlite.org/download.html for current version)
wget https://sqlite.org/2024/sqlite-amalgamation-3450100.zip
unzip sqlite-amalgamation-3450100.zip
mv sqlite-amalgamation-3450100/* .
rm -rf sqlite-amalgamation-3450100 sqlite-amalgamation-3450100.zip

# You should now have:
# vendor/sqlite3/sqlite3.c
# vendor/sqlite3/sqlite3.h
# vendor/sqlite3/shell.c      (optional - CLI tool)
# vendor/sqlite3/sqlite3ext.h (optional - extensions)
```

---

## Step 2: CMake Configuration

### vendor/sqlite3/CMakeLists.txt

```cmake
# SQLite3 as a static library
add_library(sqlite3 STATIC sqlite3.c)

target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Recommended compile options for SQLite
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_DQS=0                    # Disable double-quoted string literals
    SQLITE_THREADSAFE=1             # Thread-safe mode
    SQLITE_DEFAULT_MEMSTATUS=0      # Disable memory tracking (faster)
    SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
    SQLITE_LIKE_DOESNT_MATCH_BLOBS
    SQLITE_MAX_EXPR_DEPTH=0
    SQLITE_OMIT_DECLTYPE
    SQLITE_OMIT_DEPRECATED
    SQLITE_OMIT_PROGRESS_CALLBACK
    SQLITE_OMIT_SHARED_CACHE
    SQLITE_USE_ALLOCA
)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(sqlite3 PRIVATE pthread dl m)
elseif(APPLE)
    target_link_libraries(sqlite3 PRIVATE pthread)
endif()
```

### Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(CleanArchApp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Vendored Dependencies
# ============================================================
add_subdirectory(vendor/sqlite3)

# ============================================================
# Main Application
# ============================================================
add_executable(${PROJECT_NAME}
    src/main.cpp

    # Infrastructure
    src/infrastructure/database/SqliteDatabase.cpp
    src/infrastructure/repositories/SqlUserRepository.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link vendored SQLite3
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)

# ============================================================
# Tests (optional)
# ============================================================
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

---

## Step 3: Database Abstraction Layer

### src/infrastructure/database/IDatabase.hpp

```cpp
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <functional>
#include <memory>
#include <stdexcept>

namespace Infrastructure::Database {

// ============================================================
// Database value types
// ============================================================
using DbValue = std::variant
    std::nullptr_t,
    int64_t,
    double,
    std::string,
    std::vector<uint8_t>  // BLOB
>;

using DbRow = std::vector<DbValue>;
using DbResult = std::vector<DbRow>;

// ============================================================
// Database exceptions
// ============================================================
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message)
        : std::runtime_error(message) {}
};

class ConnectionException : public DatabaseException {
public:
    explicit ConnectionException(const std::string& message)
        : DatabaseException("Connection error: " + message) {}
};

class QueryException : public DatabaseException {
public:
    explicit QueryException(const std::string& message)
        : DatabaseException("Query error: " + message) {}
};

// ============================================================
// Prepared statement interface
// ============================================================
class IStatement {
public:
    virtual ~IStatement() = default;

    virtual IStatement& bind(int index, std::nullptr_t) = 0;
    virtual IStatement& bind(int index, int64_t value) = 0;
    virtual IStatement& bind(int index, double value) = 0;
    virtual IStatement& bind(int index, const std::string& value) = 0;
    virtual IStatement& bind(int index, const std::vector<uint8_t>& blob) = 0;

    virtual DbResult execute() = 0;
    virtual int64_t executeInsert() = 0;  // Returns last insert rowid
    virtual int executeUpdate() = 0;      // Returns rows affected

    virtual void reset() = 0;
};

// ============================================================
// Database interface
// ============================================================
class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual void open(const std::string& path) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    virtual std::unique_ptr<IStatement> prepare(const std::string& sql) = 0;

    virtual void execute(const std::string& sql) = 0;
    virtual DbResult query(const std::string& sql) = 0;

    virtual void beginTransaction() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;

    virtual int64_t lastInsertRowId() const = 0;
    virtual int changesCount() const = 0;
};

// ============================================================
// RAII Transaction guard
// ============================================================
class Transaction {
public:
    explicit Transaction(IDatabase& db) : m_db(db), m_committed(false) {
        m_db.beginTransaction();
    }

    ~Transaction() {
        if (!m_committed) {
            try {
                m_db.rollback();
            } catch (...) {
                // Suppress exceptions in destructor
            }
        }
    }

    void commit() {
        m_db.commit();
        m_committed = true;
    }

    // Non-copyable, non-movable
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

private:
    IDatabase& m_db;
    bool m_committed;
};

} // namespace Infrastructure::Database
```

### src/infrastructure/database/SqliteDatabase.hpp

```cpp
#pragma once

#include "IDatabase.hpp"
#include <sqlite3.h>

namespace Infrastructure::Database {

// ============================================================
// SQLite Prepared Statement
// ============================================================
class SqliteStatement : public IStatement {
public:
    SqliteStatement(sqlite3* db, const std::string& sql);
    ~SqliteStatement() override;

    // Non-copyable
    SqliteStatement(const SqliteStatement&) = delete;
    SqliteStatement& operator=(const SqliteStatement&) = delete;

    // Movable
    SqliteStatement(SqliteStatement&& other) noexcept;
    SqliteStatement& operator=(SqliteStatement&& other) noexcept;

    IStatement& bind(int index, std::nullptr_t) override;
    IStatement& bind(int index, int64_t value) override;
    IStatement& bind(int index, double value) override;
    IStatement& bind(int index, const std::string& value) override;
    IStatement& bind(int index, const std::vector<uint8_t>& blob) override;

    DbResult execute() override;
    int64_t executeInsert() override;
    int executeUpdate() override;

    void reset() override;

private:
    sqlite3* m_db;
    sqlite3_stmt* m_stmt;

    void checkError(int result, const std::string& context);
    DbValue extractColumn(int col);
};

// ============================================================
// SQLite Database Implementation
// ============================================================
class SqliteDatabase : public IDatabase {
public:
    SqliteDatabase() = default;
    explicit SqliteDatabase(const std::string& path);
    ~SqliteDatabase() override;

    // Non-copyable
    SqliteDatabase(const SqliteDatabase&) = delete;
    SqliteDatabase& operator=(const SqliteDatabase&) = delete;

    // Movable
    SqliteDatabase(SqliteDatabase&& other) noexcept;
    SqliteDatabase& operator=(SqliteDatabase&& other) noexcept;

    void open(const std::string& path) override;
    void close() override;
    bool isOpen() const override;

    std::unique_ptr<IStatement> prepare(const std::string& sql) override;

    void execute(const std::string& sql) override;
    DbResult query(const std::string& sql) override;

    void beginTransaction() override;
    void commit() override;
    void rollback() override;

    int64_t lastInsertRowId() const override;
    int changesCount() const override;

    // SQLite-specific
    void enableForeignKeys(bool enable = true);
    void setJournalMode(const std::string& mode);  // WAL, DELETE, etc.

private:
    sqlite3* m_db = nullptr;

    void checkError(int result, const std::string& context);
};

} // namespace Infrastructure::Database
```

### src/infrastructure/database/SqliteDatabase.cpp

```cpp
#include "SqliteDatabase.hpp"
#include <utility>

namespace Infrastructure::Database {

// ============================================================
// SqliteStatement Implementation
// ============================================================

SqliteStatement::SqliteStatement(sqlite3* db, const std::string& sql)
    : m_db(db), m_stmt(nullptr)
{
    int result = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_stmt, nullptr);
    checkError(result, "prepare statement");
}

SqliteStatement::~SqliteStatement() {
    if (m_stmt) {
        sqlite3_finalize(m_stmt);
    }
}

SqliteStatement::SqliteStatement(SqliteStatement&& other) noexcept
    : m_db(other.m_db), m_stmt(other.m_stmt)
{
    other.m_stmt = nullptr;
}

SqliteStatement& SqliteStatement::operator=(SqliteStatement&& other) noexcept {
    if (this != &other) {
        if (m_stmt) {
            sqlite3_finalize(m_stmt);
        }
        m_db = other.m_db;
        m_stmt = other.m_stmt;
        other.m_stmt = nullptr;
    }
    return *this;
}

IStatement& SqliteStatement::bind(int index, std::nullptr_t) {
    checkError(sqlite3_bind_null(m_stmt, index), "bind null");
    return *this;
}

IStatement& SqliteStatement::bind(int index, int64_t value) {
    checkError(sqlite3_bind_int64(m_stmt, index), "bind int64");
    return *this;
}

IStatement& SqliteStatement::bind(int index, double value) {
    checkError(sqlite3_bind_double(m_stmt, index, value), "bind double");
    return *this;
}

IStatement& SqliteStatement::bind(int index, const std::string& value) {
    checkError(
        sqlite3_bind_text(m_stmt, index, value.c_str(), -1, SQLITE_TRANSIENT),
        "bind text"
    );
    return *this;
}

IStatement& SqliteStatement::bind(int index, const std::vector<uint8_t>& blob) {
    checkError(
        sqlite3_bind_blob(m_stmt, index, blob.data(), static_cast<int>(blob.size()), SQLITE_TRANSIENT),
        "bind blob"
    );
    return *this;
}

DbResult SqliteStatement::execute() {
    DbResult results;
    int columnCount = sqlite3_column_count(m_stmt);

    int result;
    while ((result = sqlite3_step(m_stmt)) == SQLITE_ROW) {
        DbRow row;
        row.reserve(columnCount);

        for (int i = 0; i < columnCount; ++i) {
            row.push_back(extractColumn(i));
        }

        results.push_back(std::move(row));
    }

    if (result != SQLITE_DONE) {
        checkError(result, "execute");
    }

    return results;
}

int64_t SqliteStatement::executeInsert() {
    int result = sqlite3_step(m_stmt);
    if (result != SQLITE_DONE) {
        checkError(result, "executeInsert");
    }
    return sqlite3_last_insert_rowid(m_db);
}

int SqliteStatement::executeUpdate() {
    int result = sqlite3_step(m_stmt);
    if (result != SQLITE_DONE) {
        checkError(result, "executeUpdate");
    }
    return sqlite3_changes(m_db);
}

void SqliteStatement::reset() {
    sqlite3_reset(m_stmt);
    sqlite3_clear_bindings(m_stmt);
}

void SqliteStatement::checkError(int result, const std::string& context) {
    if (result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
        throw QueryException(context + ": " + sqlite3_errmsg(m_db));
    }
}

DbValue SqliteStatement::extractColumn(int col) {
    int type = sqlite3_column_type(m_stmt, col);

    switch (type) {
        case SQLITE_NULL:
            return nullptr;

        case SQLITE_INTEGER:
            return static_cast<int64_t>(sqlite3_column_int64(m_stmt, col));

        case SQLITE_FLOAT:
            return sqlite3_column_double(m_stmt, col);

        case SQLITE_TEXT: {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, col));
            return std::string(text ? text : "");
        }

        case SQLITE_BLOB: {
            const uint8_t* data = static_cast<const uint8_t*>(sqlite3_column_blob(m_stmt, col));
            int size = sqlite3_column_bytes(m_stmt, col);
            return std::vector<uint8_t>(data, data + size);
        }

        default:
            return nullptr;
    }
}

// ============================================================
// SqliteDatabase Implementation
// ============================================================

SqliteDatabase::SqliteDatabase(const std::string& path) {
    open(path);
}

SqliteDatabase::~SqliteDatabase() {
    close();
}

SqliteDatabase::SqliteDatabase(SqliteDatabase&& other) noexcept
    : m_db(other.m_db)
{
    other.m_db = nullptr;
}

SqliteDatabase& SqliteDatabase::operator=(SqliteDatabase&& other) noexcept {
    if (this != &other) {
        close();
        m_db = other.m_db;
        other.m_db = nullptr;
    }
    return *this;
}

void SqliteDatabase::open(const std::string& path) {
    if (m_db) {
        close();
    }

    int result = sqlite3_open(path.c_str(), &m_db);
    if (result != SQLITE_OK) {
        std::string error = m_db ? sqlite3_errmsg(m_db) : "Unknown error";
        if (m_db) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }
        throw ConnectionException(error);
    }

    // Enable foreign keys by default
    enableForeignKeys(true);
}

void SqliteDatabase::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool SqliteDatabase::isOpen() const {
    return m_db != nullptr;
}

std::unique_ptr<IStatement> SqliteDatabase::prepare(const std::string& sql) {
    if (!m_db) {
        throw ConnectionException("Database not open");
    }
    return std::make_unique<SqliteStatement>(m_db, sql);
}

void SqliteDatabase::execute(const std::string& sql) {
    if (!m_db) {
        throw ConnectionException("Database not open");
    }

    char* errorMsg = nullptr;
    int result = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errorMsg);

    if (result != SQLITE_OK) {
        std::string error = errorMsg ? errorMsg : "Unknown error";
        sqlite3_free(errorMsg);
        throw QueryException(error);
    }
}

DbResult SqliteDatabase::query(const std::string& sql) {
    auto stmt = prepare(sql);
    return stmt->execute();
}

void SqliteDatabase::beginTransaction() {
    execute("BEGIN TRANSACTION");
}

void SqliteDatabase::commit() {
    execute("COMMIT");
}

void SqliteDatabase::rollback() {
    execute("ROLLBACK");
}

int64_t SqliteDatabase::lastInsertRowId() const {
    return m_db ? sqlite3_last_insert_rowid(m_db) : 0;
}

int SqliteDatabase::changesCount() const {
    return m_db ? sqlite3_changes(m_db) : 0;
}

void SqliteDatabase::enableForeignKeys(bool enable) {
    execute(enable ? "PRAGMA foreign_keys = ON" : "PRAGMA foreign_keys = OFF");
}

void SqliteDatabase::setJournalMode(const std::string& mode) {
    execute("PRAGMA journal_mode = " + mode);
}

} // namespace Infrastructure::Database
```

---

## Step 4: Repository Using the Database

### src/infrastructure/repositories/SqlUserRepository.hpp

```cpp
#pragma once

#include "use_cases/interfaces/IUserRepository.hpp"
#include "infrastructure/database/IDatabase.hpp"
#include <memory>

namespace Infrastructure::Repositories {

class SqlUserRepository : public UseCases::IUserRepository {
public:
    explicit SqlUserRepository(std::shared_ptr<Database::IDatabase> database);

    void initializeSchema();

    std::optional<Entities::User> findById(const std::string& id) override;
    std::optional<Entities::User> findByEmail(const std::string& email) override;
    std::vector<Entities::User> findAll() override;
    Entities::User save(const Entities::User& user) override;
    bool remove(const std::string& id) override;

private:
    std::shared_ptr<Database::IDatabase> m_db;

    Entities::User mapRowToEntity(const Database::DbRow& row);
};

} // namespace Infrastructure::Repositories
```

### src/infrastructure/repositories/SqlUserRepository.cpp

```cpp
#include "SqlUserRepository.hpp"
#include <chrono>

namespace Infrastructure::Repositories {

SqlUserRepository::SqlUserRepository(std::shared_ptr<Database::IDatabase> database)
    : m_db(std::move(database))
{
}

void SqlUserRepository::initializeSchema() {
    m_db->execute(R"(
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            email TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            created_at INTEGER NOT NULL
        )
    )");

    m_db->execute(R"(
        CREATE INDEX IF NOT EXISTS idx_users_email ON users(email)
    )");
}

std::optional<Entities::User> SqlUserRepository::findById(const std::string& id) {
    auto stmt = m_db->prepare("SELECT id, email, name, created_at FROM users WHERE id = ?");
    stmt->bind(1, id);

    auto results = stmt->execute();

    if (results.empty()) {
        return std::nullopt;
    }

    return mapRowToEntity(results[0]);
}

std::optional<Entities::User> SqlUserRepository::findByEmail(const std::string& email) {
    auto stmt = m_db->prepare("SELECT id, email, name, created_at FROM users WHERE email = ?");
    stmt->bind(1, email);

    auto results = stmt->execute();

    if (results.empty()) {
        return std::nullopt;
    }

    return mapRowToEntity(results[0]);
}

std::vector<Entities::User> SqlUserRepository::findAll() {
    auto results = m_db->query("SELECT id, email, name, created_at FROM users ORDER BY created_at DESC");

    std::vector<Entities::User> users;
    users.reserve(results.size());

    for (const auto& row : results) {
        users.push_back(mapRowToEntity(row));
    }

    return users;
}

Entities::User SqlUserRepository::save(const Entities::User& user) {
    auto stmt = m_db->prepare(R"(
        INSERT INTO users (id, email, name, created_at)
        VALUES (?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            email = excluded.email,
            name = excluded.name
    )");

    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        user.createdAt.time_since_epoch()
    ).count();

    stmt->bind(1, user.id)
        .bind(2, user.email)
        .bind(3, user.name)
        .bind(4, static_cast<int64_t>(timestamp));

    stmt->executeUpdate();

    return user;
}

bool SqlUserRepository::remove(const std::string& id) {
    auto stmt = m_db->prepare("DELETE FROM users WHERE id = ?");
    stmt->bind(1, id);

    int affected = stmt->executeUpdate();
    return affected > 0;
}

Entities::User SqlUserRepository::mapRowToEntity(const Database::DbRow& row) {
    auto timestamp = std::get<int64_t>(row[3]);

    return Entities::User{
        .id = std::get<std::string>(row[0]),
        .email = std::get<std::string>(row[1]),
        .name = std::get<std::string>(row[2]),
        .createdAt = std::chrono::system_clock::from_time_t(timestamp)
    };
}

} // namespace Infrastructure::Repositories
```

---

## Step 5: Wire Everything in Main

### src/main.cpp

```cpp
#include "entities/User.hpp"
#include "use_cases/create_user/CreateUserUseCase.hpp"
#include "infrastructure/database/SqliteDatabase.hpp"
#include "infrastructure/repositories/SqlUserRepository.hpp"
#include "adapters/controllers/CreateUserController.hpp"
#include "adapters/presenters/GuiCreateUserPresenter.hpp"
#include "ui/CreateUserView.hpp"

#include <iostream>
#include <memory>

int main() {
    try {
        // ========== Layer 7 & 6: Database + Data Adapter ==========

        // Create and configure database
        auto database = std::make_shared<Infrastructure::Database::SqliteDatabase>();
        database->open("app.db");
        database->setJournalMode("WAL");  // Better concurrent performance

        // Create repository and initialize schema
        auto userRepository = std::make_shared<Infrastructure::Repositories::SqlUserRepository>(database);
        userRepository->initializeSchema();

        // ========== Layer 3: Interactor ==========
        auto createUserInteractor = std::make_shared<UseCases::CreateUser::Interactor>(
            userRepository
        );

        // ========== Layer 2: Presenter ==========
        auto presenter = std::make_shared<Adapters::GuiCreateUserPresenter>();

        // ========== Layer 2: Controller ==========
        auto controller = std::make_shared<Adapters::CreateUserController>(
            createUserInteractor,
            presenter
        );

        // ========== Layer 1: View ==========
        UI::CreateUserView view(controller, presenter);

        // ========== Run Application ==========
        std::cout << "=== User Registration System ===\n\n";
        view.simulateUserInput();

        // Show all users
        std::cout << "\n=== All Users ===\n";
        for (const auto& user : userRepository->findAll()) {
            std::cout << "- " << user.displayName() << " <" << user.email << ">\n";
        }

    } catch (const Infrastructure::Database::DatabaseException& e) {
        std::cerr << "Database error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
```

---

## Build and Run

```bash
# Configure
mkdir build && cd build
cmake ..

# Build
cmake --build .

# Run
./CleanArchApp
```

---

## Final Architecture Visualization

```
┌────────────────────────────────────────────────────────────────────────────┐
│                            YOUR APPLICATION                                │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  src/                                                                      │
│  ├── entities/           ◄─── Pure domain, no dependencies                 │
│  │                                                                         │
│  ├── use_cases/          ◄─── Business logic + interfaces (IUserRepository)│
│  │                                                                         │
│  ├── adapters/           ◄─── Controllers, Presenters                      │
│  │                                                                         │
│  ├── infrastructure/                                                       │
│  │   ├── database/       ◄─── IDatabase, SqliteDatabase                    │
│  │   └── repositories/   ◄─── SqlUserRepository (implements IUserRepository)│
│  │                                                                         │
│  └── ui/                 ◄─── Views                                        │
│                                                                            │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  vendor/                                                                   │
│  └── sqlite3/            ◄─── Vendored SQLite3 amalgamation                │
│      ├── sqlite3.c                                                         │
│      ├── sqlite3.h                                                         │
│      └── CMakeLists.txt                                                    │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘

Dependency Flow:

    View ──► Controller ──► Interactor ◄── IUserRepository (interface)
                                │                   ▲
                                │                   │ implements
                                ▼                   │
                            Entity        SqlUserRepository
                                                    │
                                                    │ uses
                                                    ▼
                                          IDatabase (interface)
                                                    ▲
                                                    │ implements
                                                    │
                                            SqliteDatabase
                                                    │
                                                    │ links
                                                    ▼
                                          vendor/sqlite3 (library)
```

---

## The Dependency Rule

In Clean Architecture, dependencies point **inward**:

```
Infrastructure → Use Cases → Entities
    (outer)       (middle)     (inner)
```

**Outer layers CAN depend on inner layers.** This is correct and expected.

So Infrastructure depending on Entities is **allowed**.

---

## What's NOT Allowed

Inner layers depending on outer layers:

```
❌ Entities depending on Infrastructure
❌ Use Cases depending on Infrastructure
❌ Entities depending on Use Cases
```

---

## Your Current Structure (Valid)

```
┌─────────────────────────────────────────────────────────────────┐
│ INFRASTRUCTURE (outer)                                          │
│   SqlUserRepository                                             │
│       │                                                         │
│       │ implements                                              │
│       ▼                                                         │
├─────────────────────────────────────────────────────────────────┤
│ USE CASES (middle)                                              │
│   IUserRepository ←── interface defined here                    │
│       │                                                         │
│       │ uses                                                    │
│       ▼                                                         │
├─────────────────────────────────────────────────────────────────┤
│ ENTITIES (inner)                                                │
│   User                                                          │
└─────────────────────────────────────────────────────────────────┘

Dependencies flow INWARD ✓
```

This is **valid** Clean Architecture.

---

## But You Raise a Good Point

There's a **stricter** interpretation where infrastructure has its own data models and doesn't directly touch Entities. This adds a mapping layer.

---

## Stricter Approach: Data Models + Mappers

```
┌─────────────────────────────────────────────────────────────────┐
│ INFRASTRUCTURE                                                  │
│                                                                 │
│   UserDataModel (DTO)     ←── Infrastructure's own model        │
│        ↕ mapping                                                │
│   SqlUserRepository                                             │
│        │                                                        │
│        │ implements                                             │
│        ▼                                                        │
├─────────────────────────────────────────────────────────────────┤
│ USE CASES                                                       │
│                                                                 │
│   IUserRepository         ←── Works with Entity                 │
│        │                                                        │
│        │ references                                             │
│        ▼                                                        │
├─────────────────────────────────────────────────────────────────┤
│ ENTITIES                                                        │
│                                                                 │
│   User                                                          │
└─────────────────────────────────────────────────────────────────┘
```

### Implementation

```cpp
// ============================================================
// ENTITIES - Pure domain
// ============================================================
namespace Entities {

struct User {
    std::string id;
    std::string email;
    std::string name;
    std::chrono::system_clock::time_point createdAt;

    bool isValidEmail() const { /* ... */ }
};

} // namespace Entities
```

```cpp
// ============================================================
// USE CASES - Interface uses Entity
// ============================================================
namespace UseCases {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual std::optional<Entities::User> findById(const std::string& id) = 0;
    virtual Entities::User save(const Entities::User& user) = 0;
    // ...
};

} // namespace UseCases
```

```cpp
// ============================================================
// INFRASTRUCTURE - Has its own data model
// ============================================================
namespace Infrastructure {

// Infrastructure's internal model - matches database schema
// Does NOT depend on Entities::User
struct UserDataModel {
    std::string id;
    std::string email;
    std::string name;
    int64_t createdAtUnix;
};

// Mapper - this is where the "dependency" on Entity lives
// But it's isolated to this one translation point
class UserMapper {
public:
    static Entities::User toEntity(const UserDataModel& model) {
        return Entities::User{
            .id = model.id,
            .email = model.email,
            .name = model.name,
            .createdAt = std::chrono::system_clock::from_time_t(model.createdAtUnix)
        };
    }

    static UserDataModel toDataModel(const Entities::User& entity) {
        return UserDataModel{
            .id = entity.id,
            .email = entity.email,
            .name = entity.name,
            .createdAtUnix = std::chrono::system_clock::to_time_t(entity.createdAt)
        };
    }
};

// Repository works internally with DataModel, exposes Entity
class SqlUserRepository : public UseCases::IUserRepository {
public:
    explicit SqlUserRepository(std::shared_ptr<Database::IDatabase> db)
        : m_db(std::move(db)) {}

    std::optional<Entities::User> findById(const std::string& id) override {
        auto stmt = m_db->prepare("SELECT id, email, name, created_at FROM users WHERE id = ?");
        stmt->bind(1, id);
        auto results = stmt->execute();

        if (results.empty()) {
            return std::nullopt;
        }

        // Internal: row → DataModel
        UserDataModel model = rowToDataModel(results[0]);

        // Boundary: DataModel → Entity
        return UserMapper::toEntity(model);
    }

    Entities::User save(const Entities::User& user) override {
        // Boundary: Entity → DataModel
        UserDataModel model = UserMapper::toDataModel(user);

        // Internal: work with DataModel
        auto stmt = m_db->prepare(R"(
            INSERT INTO users (id, email, name, created_at)
            VALUES (?, ?, ?, ?)
        )");

        stmt->bind(1, model.id)
            .bind(2, model.email)
            .bind(3, model.name)
            .bind(4, model.createdAtUnix);

        stmt->executeInsert();

        return user;
    }

private:
    std::shared_ptr<Database::IDatabase> m_db;

    // Internal conversion - no Entity involved
    UserDataModel rowToDataModel(const Database::DbRow& row) {
        return UserDataModel{
            .id = std::get<std::string>(row[0]),
            .email = std::get<std::string>(row[1]),
            .name = std::get<std::string>(row[2]),
            .createdAtUnix = std::get<int64_t>(row[3])
        };
    }
};

} // namespace Infrastructure
```

---

## Updated Folder Structure

```
project/
├── src/
│   ├── entities/
│   │   └── User.hpp
│   │
│   ├── use_cases/
│   │   ├── interfaces/
│   │   │   └── IUserRepository.hpp      # References Entity
│   │   └── create_user/
│   │       └── CreateUserUseCase.hpp
│   │
│   ├── infrastructure/
│   │   ├── database/
│   │   │   ├── IDatabase.hpp
│   │   │   └── SqliteDatabase.hpp
│   │   │
│   │   ├── models/                       # Infrastructure's own models
│   │   │   └── UserDataModel.hpp
│   │   │
│   │   ├── mappers/                      # Entity ↔ DataModel conversion
│   │   │   └── UserMapper.hpp
│   │   │
│   │   └── repositories/
│   │       └── SqlUserRepository.hpp
│   │
│   └── ...
```

---

## Comparison

| Aspect | Simple Approach | Strict Approach |
|--------|-----------------|-----------------|
| Infrastructure uses Entity directly | ✓ | ✗ |
| Separate DataModel per entity | ✗ | ✓ |
| Mapper layer required | ✗ | ✓ |
| Code complexity | Lower | Higher |
| Flexibility to change DB schema | Lower | Higher |
| Valid Clean Architecture? | ✓ Yes | ✓ Yes |

---

## When to Use Which?

**Simple approach (Infrastructure uses Entity):**
- Small to medium projects
- Entity closely matches database schema
- Team is small
- Faster development

**Strict approach (DataModel + Mapper):**
- Large projects with multiple teams
- Database schema differs significantly from domain model
- Need to support multiple data sources with different schemas
- Entity has complex behavior that shouldn't leak to infrastructure

---

## The Key Insight

Both approaches satisfy the dependency rule because:

> **The interface is defined in Use Cases, not Infrastructure.**

Infrastructure *implements* an interface defined by an inner layer. The inner layer doesn't know Infrastructure exists.

```cpp
// Use Cases defines WHAT it needs
class IUserRepository {
    virtual Entities::User save(const Entities::User&) = 0;
};

// Infrastructure provides HOW
class SqlUserRepository : public IUserRepository { /* ... */ };
class MongoUserRepository : public IUserRepository { /* ... */ };
class InMemoryUserRepository : public IUserRepository { /* ... */ };
```

The Interactor depends on `IUserRepository`, not `SqlUserRepository`. That's the inversion.

---

## Mapping to Embedded BMS

| Clean Architecture Layer | Desktop App | Embedded BMS |
|--------------------------|-------------|--------------|
| **UI / External Interface** | GUI, CLI | EMOTAS CAN, UART, LEDs |
| **Controller** | Form handler | CAN Message Handler |
| **Presenter** | ViewModel builder | CAN Response Builder |
| **Interactor (Use Case)** | Business logic | BMS Logic (SOC, balancing, protection) |
| **Entity** | Domain objects | Cell, Pack, Fault, BMSState |
| **Repository Interface** | IUserRepository | ICellDataProvider, IConfigStorage |
| **Infrastructure** | SQLite, Filesystem | ADC Driver, EEPROM, GPIO, CAN HAL |

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         EXTERNAL INTERFACE                                  │
│                                                                             │
│   EMOTAS CAN Stack ◄──► CAN Transceiver ◄──► External ECUs / Charger / VCU │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                    CAN RX Interrupt │ CAN TX
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         INTERFACE ADAPTERS                                  │
│                                                                             │
│   ┌─────────────────────┐                    ┌─────────────────────┐        │
│   │  CANMessageHandler  │                    │  CANResponseBuilder │        │
│   │    (Controller)     │                    │     (Presenter)     │        │
│   │                     │                    │                     │        │
│   │ - Parses CAN frames │                    │ - Builds CAN frames │        │
│   │ - Maps to Request   │                    │ - Maps from Response│        │
│   │ - Calls Interactor  │                    │ - Queues TX         │        │
│   └─────────────────────┘                    └─────────────────────┘        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                            USE CASES                                        │
│                                                                             │
│   ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────────┐     │
│   │ ReadCellVoltages│  │ CalculateSOC    │  │ ExecuteCellBalancing    │     │
│   ├─────────────────┤  ├─────────────────┤  ├─────────────────────────┤     │
│   │ CheckOverVoltage│  │ CheckUnderTemp  │  │ ControlCharging         │     │
│   ├─────────────────┤  ├─────────────────┤  ├─────────────────────────┤     │
│   │ HandleFault     │  │ GetPackStatus   │  │ SetConfiguration        │     │
│   └─────────────────┘  └─────────────────┘  └─────────────────────────┘     │
│                                                                             │
│   Interfaces:  ICellDataProvider   IConfigStorage   IOutputPort             │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             ENTITIES                                        │
│                                                                             │
│   ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌───────────────────────┐    │
│   │   Cell    │  │   Pack    │  │   Fault   │  │      BMSState         │    │
│   │           │  │           │  │           │  │                       │    │
│   │ - voltage │  │ - cells[] │  │ - code    │  │ - mode (IDLE/CHARGE/  │    │
│   │ - temp    │  │ - soc     │  │ - severity│  │         DISCHARGE)    │    │
│   │ - balance │  │ - current │  │ - timestamp│ │ - faults[]            │    │
│   └───────────┘  └───────────┘  └───────────┘  └───────────────────────┘    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                          INFRASTRUCTURE                                     │
│                                                                             │
│   ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│   │  ADC Driver │  │ SPI Driver  │  │ GPIO Driver │  │EEPROM Driver│        │
│   │  (LTC6811)  │  │ (AFE comm)  │  │ (Contactors)│  │ (Config)    │        │
│   └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘        │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────┐       │
│   │                     EMOTAS CAN Driver (HAL)                     │       │
│   └─────────────────────────────────────────────────────────────────┘       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             HARDWARE                                        │
│                                                                             │
│   MCU (STM32/NXP)  │  CAN Transceiver  │  AFE (LTC6811)  │  EEPROM          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Folder Structure

```
bms_firmware/
├── CMakeLists.txt
│
├── src/
│   ├── entities/
│   │   ├── Cell.hpp
│   │   ├── Pack.hpp
│   │   ├── Fault.hpp
│   │   └── BMSState.hpp
│   │
│   ├── use_cases/
│   │   ├── interfaces/
│   │   │   ├── ICellDataProvider.hpp
│   │   │   ├── IContactorControl.hpp
│   │   │   ├── IConfigStorage.hpp
│   │   │   └── IOutputPort.hpp
│   │   │
│   │   ├── read_cell_voltages/
│   │   │   └── ReadCellVoltagesUseCase.hpp
│   │   │
│   │   ├── calculate_soc/
│   │   │   └── CalculateSOCUseCase.hpp
│   │   │
│   │   ├── cell_balancing/
│   │   │   └── CellBalancingUseCase.hpp
│   │   │
│   │   ├── protection/
│   │   │   ├── OverVoltageProtectionUseCase.hpp
│   │   │   ├── UnderVoltageProtectionUseCase.hpp
│   │   │   ├── OverTempProtectionUseCase.hpp
│   │   │   └── OverCurrentProtectionUseCase.hpp
│   │   │
│   │   └── charging/
│   │       └── ChargingControlUseCase.hpp
│   │
│   ├── adapters/
│   │   ├── can/
│   │   │   ├── CANMessageHandler.hpp       # Controller
│   │   │   ├── CANMessageHandler.cpp
│   │   │   ├── CANResponseBuilder.hpp      # Presenter
│   │   │   ├── CANResponseBuilder.cpp
│   │   │   └── CANMessageIds.hpp           # CAN ID definitions
│   │   │
│   │   └── diagnostics/
│   │       └── UARTDiagnosticsPresenter.hpp  # Optional debug output
│   │
│   ├── infrastructure/
│   │   ├── drivers/
│   │   │   ├── hal/
│   │   │   │   ├── IHAL_ADC.hpp
│   │   │   │   ├── IHAL_SPI.hpp
│   │   │   │   ├── IHAL_GPIO.hpp
│   │   │   │   ├── IHAL_CAN.hpp
│   │   │   │   └── IHAL_Timer.hpp
│   │   │   │
│   │   │   ├── stm32/                      # Platform-specific
│   │   │   │   ├── STM32_ADC.hpp
│   │   │   │   ├── STM32_SPI.hpp
│   │   │   │   ├── STM32_GPIO.hpp
│   │   │   │   ├── STM32_CAN.hpp
│   │   │   │   └── STM32_Timer.hpp
│   │   │   │
│   │   │   └── afe/
│   │   │       └── LTC6811Driver.hpp       # Battery AFE
│   │   │
│   │   ├── repositories/
│   │   │   ├── AFECellDataProvider.hpp     # Implements ICellDataProvider
│   │   │   ├── EEPROMConfigStorage.hpp     # Implements IConfigStorage
│   │   │   └── GPIOContactorControl.hpp    # Implements IContactorControl
│   │   │
│   │   └── can/
│   │       └── EmotasCANStack.hpp          # EMOTAS integration
│   │
│   ├── app/
│   │   ├── BMSApplication.hpp              # Main orchestrator
│   │   ├── BMSApplication.cpp
│   │   └── Scheduler.hpp                   # Task scheduling
│   │
│   └── main.cpp
│
├── vendor/
│   └── emotas/                             # EMOTAS CAN stack
│       ├── co_canopen.h
│       └── ...
│
├── config/
│   ├── bms_config.hpp                      # Compile-time config
│   └── can_object_dictionary.hpp           # CANopen OD
│
└── platform/
    └── stm32f4/
        ├── startup.cpp
        ├── system_init.cpp
        └── linker.ld
```

---

## Code Implementation

### Entities

```cpp
// ============================================================
// src/entities/Cell.hpp
// ============================================================

#pragma once
#include <cstdint>

namespace Entities {

struct CellVoltage {
    uint16_t millivolts;

    bool isOverVoltage(uint16_t threshold) const {
        return millivolts > threshold;
    }

    bool isUnderVoltage(uint16_t threshold) const {
        return millivolts < threshold;
    }
};

struct CellTemperature {
    int16_t deciCelsius;  // 0.1°C resolution

    float toCelsius() const {
        return static_cast<float>(deciCelsius) / 10.0f;
    }

    bool isOverTemp(int16_t threshold) const {
        return deciCelsius > threshold;
    }

    bool isUnderTemp(int16_t threshold) const {
        return deciCelsius < threshold;
    }
};

struct Cell {
    uint8_t index;
    CellVoltage voltage;
    CellTemperature temperature;
    bool balancing;

    // Domain logic
    bool needsBalancing(uint16_t targetMillivolts, uint16_t thresholdMillivolts) const {
        if (voltage.millivolts <= targetMillivolts) {
            return false;
        }
        return (voltage.millivolts - targetMillivolts) > thresholdMillivolts;
    }
};

} // namespace Entities
```

```cpp
// ============================================================
// src/entities/Pack.hpp
// ============================================================

#pragma once
#include "Cell.hpp"
#include <array>
#include <cstdint>
#include <algorithm>

namespace Entities {

// Compile-time configuration
inline constexpr uint8_t MAX_CELLS = 16;
inline constexpr uint8_t MAX_TEMP_SENSORS = 8;

struct Pack {
    std::array<Cell, MAX_CELLS> cells;
    uint8_t cellCount;

    int32_t currentMilliamps;
    uint16_t socPermille;  // 0-1000 (0.1% resolution)

    // Domain logic
    uint16_t totalVoltageMillivolts() const {
        uint32_t total = 0;
        for (uint8_t i = 0; i < cellCount; ++i) {
            total += cells[i].voltage.millivolts;
        }
        return static_cast<uint16_t>(total);
    }

    uint16_t minCellVoltage() const {
        uint16_t min = UINT16_MAX;
        for (uint8_t i = 0; i < cellCount; ++i) {
            min = std::min(min, cells[i].voltage.millivolts);
        }
        return min;
    }

    uint16_t maxCellVoltage() const {
        uint16_t max = 0;
        for (uint8_t i = 0; i < cellCount; ++i) {
            max = std::max(max, cells[i].voltage.millivolts);
        }
        return max;
    }

    uint16_t voltageDeltaMillivolts() const {
        return maxCellVoltage() - minCellVoltage();
    }

    int16_t maxTemperature() const {
        int16_t max = INT16_MIN;
        for (uint8_t i = 0; i < cellCount; ++i) {
            max = std::max(max, cells[i].temperature.deciCelsius);
        }
        return max;
    }

    bool isCharging() const {
        return currentMilliamps < 0;  // Negative = charging
    }

    bool isDischarging() const {
        return currentMilliamps > 0;
    }
};

} // namespace Entities
```

```cpp
// ============================================================
// src/entities/Fault.hpp
// ============================================================

#pragma once
#include <cstdint>

namespace Entities {

enum class FaultCode : uint8_t {
    NONE = 0x00,

    // Voltage faults
    CELL_OVERVOLTAGE      = 0x10,
    CELL_UNDERVOLTAGE     = 0x11,
    PACK_OVERVOLTAGE      = 0x12,
    PACK_UNDERVOLTAGE     = 0x13,
    VOLTAGE_IMBALANCE     = 0x14,

    // Temperature faults
    CELL_OVERTEMP         = 0x20,
    CELL_UNDERTEMP        = 0x21,
    TEMP_SENSOR_FAULT     = 0x22,

    // Current faults
    OVERCURRENT_CHARGE    = 0x30,
    OVERCURRENT_DISCHARGE = 0x31,
    SHORT_CIRCUIT         = 0x32,

    // Communication faults
    AFE_COMM_FAULT        = 0x40,
    CAN_COMM_FAULT        = 0x41,

    // System faults
    EEPROM_FAULT          = 0x50,
    CONTACTOR_FAULT       = 0x51,
    INTERNAL_ERROR        = 0xFF
};

enum class FaultSeverity : uint8_t {
    WARNING  = 0,  // Log only, continue operation
    ERROR    = 1,  // Limit operation
    CRITICAL = 2   // Immediate shutdown
};

struct Fault {
    FaultCode code;
    FaultSeverity severity;
    uint32_t timestampMs;
    uint8_t cellIndex;  // 0xFF if not cell-specific

    bool isCritical() const {
        return severity == FaultSeverity::CRITICAL;
    }

    bool isVoltageRelated() const {
        return (static_cast<uint8_t>(code) & 0xF0) == 0x10;
    }

    bool isTemperatureRelated() const {
        return (static_cast<uint8_t>(code) & 0xF0) == 0x20;
    }
};

} // namespace Entities
```

```cpp
// ============================================================
// src/entities/BMSState.hpp
// ============================================================

#pragma once
#include "Pack.hpp"
#include "Fault.hpp"
#include <array>

namespace Entities {

enum class BMSMode : uint8_t {
    INIT       = 0,
    IDLE       = 1,
    CHARGING   = 2,
    DISCHARGING= 3,
    BALANCING  = 4,
    FAULT      = 5,
    SHUTDOWN   = 6
};

inline constexpr uint8_t MAX_ACTIVE_FAULTS = 8;

struct BMSState {
    BMSMode mode;
    Pack pack;

    std::array<Fault, MAX_ACTIVE_FAULTS> activeFaults;
    uint8_t faultCount;

    bool contactorsClosed;
    bool chargerConnected;

    uint32_t uptimeMs;

    // Domain logic
    bool canClose Contactors() const {
        return mode != BMSMode::FAULT &&
               mode != BMSMode::SHUTDOWN &&
               faultCount == 0;
    }

    bool hasActiveFaults() const {
        return faultCount > 0;
    }

    bool hasCriticalFault() const {
        for (uint8_t i = 0; i < faultCount; ++i) {
            if (activeFaults[i].isCritical()) {
                return true;
            }
        }
        return false;
    }

    void addFault(const Fault& fault) {
        if (faultCount < MAX_ACTIVE_FAULTS) {
            activeFaults[faultCount++] = fault;
            if (fault.isCritical()) {
                mode = BMSMode::FAULT;
            }
        }
    }

    void clearFaults() {
        faultCount = 0;
        if (mode == BMSMode::FAULT) {
            mode = BMSMode::IDLE;
        }
    }
};

} // namespace Entities
```

---

### Use Case Interfaces

```cpp
// ============================================================
// src/use_cases/interfaces/ICellDataProvider.hpp
// ============================================================

#pragma once
#include "entities/Cell.hpp"
#include "entities/Pack.hpp"
#include <cstdint>

namespace UseCases {

class ICellDataProvider {
public:
    virtual ~ICellDataProvider() = default;

    virtual bool readAllCellVoltages(Entities::Pack& pack) = 0;
    virtual bool readAllTemperatures(Entities::Pack& pack) = 0;
    virtual bool readPackCurrent(int32_t& currentMilliamps) = 0;

    virtual bool startBalancing(uint8_t cellIndex) = 0;
    virtual bool stopBalancing(uint8_t cellIndex) = 0;
    virtual bool stopAllBalancing() = 0;
};

} // namespace UseCases
```

```cpp
// ============================================================
// src/use_cases/interfaces/IContactorControl.hpp
// ============================================================

#pragma once

namespace UseCases {

class IContactorControl {
public:
    virtual ~IContactorControl() = default;

    virtual bool closeMainContactor() = 0;
    virtual bool openMainContactor() = 0;
    virtual bool closePrechargeContactor() = 0;
    virtual bool openPrechargeContactor() = 0;

    virtual bool isMainContactorClosed() = 0;
    virtual bool isPrechargeComplete() = 0;
};

} // namespace UseCases
```

```cpp
// ============================================================
// src/use_cases/interfaces/IOutputPort.hpp
// Generic output port for all use cases
// ============================================================

#pragma once
#include "entities/BMSState.hpp"
#include "entities/Fault.hpp"

namespace UseCases {

// Response models for CAN transmission
struct PackStatusResponse {
    uint16_t totalVoltageMillivolts;
    int32_t currentMilliamps;
    uint16_t socPermille;
    uint8_t mode;
    uint8_t faultCount;
};

struct CellVoltagesResponse {
    uint8_t startIndex;
    uint8_t count;
    uint16_t voltages[4];  // 4 cells per CAN message
};

struct FaultResponse {
    uint8_t faultCode;
    uint8_t severity;
    uint8_t cellIndex;
    uint32_t timestamp;
};

class IOutputPort {
public:
    virtual ~IOutputPort() = default;

    virtual void presentPackStatus(const PackStatusResponse& response) = 0;
    virtual void presentCellVoltages(const CellVoltagesResponse& response) = 0;
    virtual void presentFault(const FaultResponse& response) = 0;
    virtual void presentError(uint8_t errorCode) = 0;
};

} // namespace UseCases
```

---

### Use Case Implementation

```cpp
// ============================================================
// src/use_cases/protection/OverVoltageProtectionUseCase.hpp
// ============================================================

#pragma once
#include "entities/BMSState.hpp"
#include "use_cases/interfaces/ICellDataProvider.hpp"
#include "use_cases/interfaces/IContactorControl.hpp"
#include "use_cases/interfaces/IOutputPort.hpp"
#include <cstdint>

namespace UseCases::Protection {

struct OverVoltageConfig {
    uint16_t cellMaxMillivolts;      // e.g., 4200mV
    uint16_t cellWarningMillivolts;  // e.g., 4150mV
    uint16_t packMaxMillivolts;
};

class OverVoltageProtectionUseCase {
public:
    OverVoltageProtectionUseCase(
        ICellDataProvider& cellDataProvider,
        IContactorControl& contactorControl,
        const OverVoltageConfig& config)
        : m_cellDataProvider(cellDataProvider)
        , m_contactorControl(contactorControl)
        , m_config(config)
    {}

    void execute(Entities::BMSState& state, IOutputPort& outputPort) {
        // Check each cell
        for (uint8_t i = 0; i < state.pack.cellCount; ++i) {
            const auto& cell = state.pack.cells[i];

            if (cell.voltage.isOverVoltage(m_config.cellMaxMillivolts)) {
                // Critical fault - open contactors immediately
                m_contactorControl.openMainContactor();

                Entities::Fault fault{
                    .code = Entities::FaultCode::CELL_OVERVOLTAGE,
                    .severity = Entities::FaultSeverity::CRITICAL,
                    .timestampMs = state.uptimeMs,
                    .cellIndex = i
                };

                state.addFault(fault);

                outputPort.presentFault({
                    .faultCode = static_cast<uint8_t>(fault.code),
                    .severity = static_cast<uint8_t>(fault.severity),
                    .cellIndex = i,
                    .timestamp = state.uptimeMs
                });

                return;  // Stop checking on critical fault
            }
            else if (cell.voltage.isOverVoltage(m_config.cellWarningMillivolts)) {
                // Warning - log but continue
                Entities::Fault fault{
                    .code = Entities::FaultCode::CELL_OVERVOLTAGE,
                    .severity = Entities::FaultSeverity::WARNING,
                    .timestampMs = state.uptimeMs,
                    .cellIndex = i
                };

                state.addFault(fault);

                outputPort.presentFault({
                    .faultCode = static_cast<uint8_t>(fault.code),
                    .severity = static_cast<uint8_t>(fault.severity),
                    .cellIndex = i,
                    .timestamp = state.uptimeMs
                });
            }
        }

        // Check pack total
        uint16_t packVoltage = state.pack.totalVoltageMillivolts();
        if (packVoltage > m_config.packMaxMillivolts) {
            m_contactorControl.openMainContactor();

            Entities::Fault fault{
                .code = Entities::FaultCode::PACK_OVERVOLTAGE,
                .severity = Entities::FaultSeverity::CRITICAL,
                .timestampMs = state.uptimeMs,
                .cellIndex = 0xFF
            };

            state.addFault(fault);

            outputPort.presentFault({
                .faultCode = static_cast<uint8_t>(fault.code),
                .severity = static_cast<uint8_t>(fault.severity),
                .cellIndex = 0xFF,
                .timestamp = state.uptimeMs
            });
        }
    }

private:
    ICellDataProvider& m_cellDataProvider;
    IContactorControl& m_contactorControl;
    OverVoltageConfig m_config;
};

} // namespace UseCases::Protection
```

```cpp
// ============================================================
// src/use_cases/cell_balancing/CellBalancingUseCase.hpp
// ============================================================

#pragma once
#include "entities/BMSState.hpp"
#include "use_cases/interfaces/ICellDataProvider.hpp"

namespace UseCases::Balancing {

struct BalancingConfig {
    uint16_t deltaThresholdMillivolts;  // Start balancing if delta > this
    uint16_t targetDeltaMillivolts;     // Stop balancing when delta < this
    bool balanceDuringCharge;
    bool balanceDuringDischarge;
};

class CellBalancingUseCase {
public:
    CellBalancingUseCase(
        ICellDataProvider& cellDataProvider,
        const BalancingConfig& config)
        : m_cellDataProvider(cellDataProvider)
        , m_config(config)
    {}

    void execute(Entities::BMSState& state) {
        // Check if balancing is allowed in current mode
        if (!shouldBalance(state)) {
            m_cellDataProvider.stopAllBalancing();
            return;
        }

        uint16_t minVoltage = state.pack.minCellVoltage();
        uint16_t delta = state.pack.voltageDeltaMillivolts();

        // Check if balancing is needed
        if (delta < m_config.targetDeltaMillivolts) {
            m_cellDataProvider.stopAllBalancing();
            return;
        }

        // Balance cells that are above minimum + threshold
        uint16_t balanceThreshold = minVoltage + m_config.deltaThresholdMillivolts;

        for (uint8_t i = 0; i < state.pack.cellCount; ++i) {
            auto& cell = state.pack.cells[i];

            if (cell.voltage.millivolts > balanceThreshold) {
                m_cellDataProvider.startBalancing(i);
                cell.balancing = true;
            } else {
                m_cellDataProvider.stopBalancing(i);
                cell.balancing = false;
            }
        }
    }

private:
    ICellDataProvider& m_cellDataProvider;
    BalancingConfig m_config;

    bool shouldBalance(const Entities::BMSState& state) const {
        if (state.mode == Entities::BMSMode::FAULT) {
            return false;
        }

        if (state.pack.isCharging() && !m_config.balanceDuringCharge) {
            return false;
        }

        if (state.pack.isDischarging() && !m_config.balanceDuringDischarge) {
            return false;
        }

        return true;
    }
};

} // namespace UseCases::Balancing
```

---

### CAN Adapter (Controller / Presenter)

```cpp
// ============================================================
// src/adapters/can/CANMessageIds.hpp
// ============================================================

#pragma once
#include <cstdint>

namespace Adapters::CAN {

// CAN IDs (example - adjust to your protocol)
namespace RxId {
    constexpr uint32_t REQUEST_PACK_STATUS   = 0x100;
    constexpr uint32_t REQUEST_CELL_VOLTAGES = 0x101;
    constexpr uint32_t REQUEST_TEMPERATURES  = 0x102;
    constexpr uint32_t COMMAND_CONTACTORS    = 0x110;
    constexpr uint32_t COMMAND_CHARGING      = 0x111;
    constexpr uint32_t SET_CONFIGURATION     = 0x120;
}

namespace TxId {
    constexpr uint32_t PACK_STATUS           = 0x180;
    constexpr uint32_t CELL_VOLTAGES_1_4     = 0x181;
    constexpr uint32_t CELL_VOLTAGES_5_8     = 0x182;
    constexpr uint32_t CELL_VOLTAGES_9_12    = 0x183;
    constexpr uint32_t CELL_VOLTAGES_13_16   = 0x184;
    constexpr uint32_t TEMPERATURES          = 0x190;
    constexpr uint32_t FAULT_BROADCAST       = 0x1F0;
}

struct CANMessage {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
};

} // namespace Adapters::CAN
```

```cpp
// ============================================================
// src/adapters/can/CANMessageHandler.hpp (Controller)
// ============================================================

#pragma once
#include "CANMessageIds.hpp"
#include "use_cases/interfaces/IOutputPort.hpp"
#include "entities/BMSState.hpp"

// Forward declare use cases
namespace UseCases {
    class ReadPackStatusUseCase;
    class ReadCellVoltagesUseCase;
}

namespace Adapters::CAN {

class CANMessageHandler {
public:
    CANMessageHandler(
        UseCases::ReadPackStatusUseCase& packStatusUC,
        UseCases::ReadCellVoltagesUseCase& cellVoltagesUC,
        UseCases::IOutputPort& outputPort,
        Entities::BMSState& state)
        : m_packStatusUC(packStatusUC)
        , m_cellVoltagesUC(cellVoltagesUC)
        , m_outputPort(outputPort)
        , m_state(state)
    {}

    // Called from CAN RX interrupt or polling
    void handleMessage(const CANMessage& msg) {
        switch (msg.id) {
            case RxId::REQUEST_PACK_STATUS:
                handlePackStatusRequest(msg);
                break;

            case RxId::REQUEST_CELL_VOLTAGES:
                handleCellVoltagesRequest(msg);
                break;

            case RxId::COMMAND_CONTACTORS:
                handleContactorCommand(msg);
                break;

            default:
                // Unknown message - ignore or log
                break;
        }
    }

private:
    UseCases::ReadPackStatusUseCase& m_packStatusUC;
    UseCases::ReadCellVoltagesUseCase& m_cellVoltagesUC;
    UseCases::IOutputPort& m_outputPort;
    Entities::BMSState& m_state;

    void handlePackStatusRequest(const CANMessage& msg) {
        // No request data needed - just execute use case
        m_packStatusUC.execute(m_state, m_outputPort);
    }

    void handleCellVoltagesRequest(const CANMessage& msg) {
        // Parse which cell range is requested
        uint8_t startIndex = msg.data[0];
        m_cellVoltagesUC.execute(m_state, startIndex, m_outputPort);
    }

    void handleContactorCommand(const CANMessage& msg) {
        // Parse command
        // bool closeContactors = msg.data[0] != 0;
        // ... invoke appropriate use case
    }
};

} // namespace Adapters::CAN
```

```cpp
// ============================================================
// src/adapters/can/CANResponseBuilder.hpp (Presenter)
// ============================================================

#pragma once
#include "CANMessageIds.hpp"
#include "use_cases/interfaces/IOutputPort.hpp"
#include "infrastructure/can/ICANTransmitter.hpp"

namespace Adapters::CAN {

class CANResponseBuilder : public UseCases::IOutputPort {
public:
    explicit CANResponseBuilder(Infrastructure::ICANTransmitter& transmitter)
        : m_transmitter(transmitter)
    {}

    void presentPackStatus(const UseCases::PackStatusResponse& response) override {
        CANMessage msg;
        msg.id = TxId::PACK_STATUS;
        msg.dlc = 8;

        // Pack data into CAN frame (little-endian example)
        msg.data[0] = response.totalVoltageMillivolts & 0xFF;
        msg.data[1] = (response.totalVoltageMillivolts >> 8) & 0xFF;
        msg.data[2] = response.currentMilliamps & 0xFF;
        msg.data[3] = (response.currentMilliamps >> 8) & 0xFF;
        msg.data[4] = (response.currentMilliamps >> 16) & 0xFF;
        msg.data[5] = (response.currentMilliamps >> 24) & 0xFF;
        msg.data[6] = response.socPermille & 0xFF;
        msg.data[7] = ((response.socPermille >> 8) & 0x03) |
                      (response.mode << 2) |
                      (response.faultCount << 5);

        m_transmitter.transmit(msg);
    }

    void presentCellVoltages(const UseCases::CellVoltagesResponse& response) override {
        CANMessage msg;

        // Select CAN ID based on cell range
        msg.id = TxId::CELL_VOLTAGES_1_4 + (response.startIndex / 4);
        msg.dlc = 8;

        // 4 voltages × 2 bytes = 8 bytes
        for (uint8_t i = 0; i < 4; ++i) {
            msg.data[i * 2]     = response.voltages[i] & 0xFF;
            msg.data[i * 2 + 1] = (response.voltages[i] >> 8) & 0xFF;
        }

        m_transmitter.transmit(msg);
    }

    void presentFault(const UseCases::FaultResponse& response) override {
        CANMessage msg;
        msg.id = TxId::FAULT_BROADCAST;
        msg.dlc = 7;

        msg.data[0] = response.faultCode;
        msg.data[1] = response.severity;
        msg.data[2] = response.cellIndex;
        msg.data[3] = response.timestamp & 0xFF;
        msg.data[4] = (response.timestamp >> 8) & 0xFF;
        msg.data[5] = (response.timestamp >> 16) & 0xFF;
        msg.data[6] = (response.timestamp >> 24) & 0xFF;

        m_transmitter.transmit(msg);
    }

    void presentError(uint8_t errorCode) override {
        // Could send a generic error frame
        CANMessage msg;
        msg.id = TxId::FAULT_BROADCAST;
        msg.dlc = 2;
        msg.data[0] = 0xFF;  // Internal error marker
        msg.data[1] = errorCode;

        m_transmitter.transmit(msg);
    }

private:
    Infrastructure::ICANTransmitter& m_transmitter;
};

} // namespace Adapters::CAN
```

---

### Infrastructure (HAL + Drivers)

```cpp
// ============================================================
// src/infrastructure/can/ICANTransmitter.hpp
// ============================================================

#pragma once
#include "adapters/can/CANMessageIds.hpp"

namespace Infrastructure {

class ICANTransmitter {
public:
    virtual ~ICANTransmitter() = default;
    virtual bool transmit(const Adapters::CAN::CANMessage& msg) = 0;
    virtual bool isTxBufferFull() const = 0;
};

} // namespace Infrastructure
```

```cpp
// ============================================================
// src/infrastructure/can/EmotasCANStack.hpp
// ============================================================

#pragma once
#include "ICANTransmitter.hpp"
#include <co_canopen.h>  // EMOTAS header

namespace Infrastructure {

class EmotasCANStack : public ICANTransmitter {
public:
    EmotasCANStack() = default;

    bool initialize(uint8_t nodeId, uint16_t baudrate) {
        // Initialize EMOTAS stack
        // coCanOpenInit(...);
        return true;
    }

    bool transmit(const Adapters::CAN::CANMessage& msg) override {
        // Use EMOTAS API to transmit
        // coCanSend(msg.id, msg.dlc, msg.data);
        return true;
    }

    bool isTxBufferFull() const override {
        // Check EMOTAS TX buffer status
        return false;
    }

    // Called from CAN RX interrupt
    void processRxMessage() {
        // EMOTAS will call registered callbacks
        // coCanProcess();
    }

    // Register callback for received messages
    using RxCallback = void(*)(const Adapters::CAN::CANMessage&);
    void setRxCallback(RxCallback callback) {
        m_rxCallback = callback;
    }

private:
    RxCallback m_rxCallback = nullptr;
};

} // namespace Infrastructure
```

```cpp
// ============================================================
// src/infrastructure/repositories/AFECellDataProvider.hpp
// Implements ICellDataProvider using LTC6811 AFE
// ============================================================

#pragma once
#include "use_cases/interfaces/ICellDataProvider.hpp"
#include "infrastructure/drivers/afe/LTC6811Driver.hpp"

namespace Infrastructure {

class AFECellDataProvider : public UseCases::ICellDataProvider {
public:
    explicit AFECellDataProvider(LTC6811Driver& afeDriver)
        : m_afe(afeDriver)
    {}

    bool readAllCellVoltages(Entities::Pack& pack) override {
        uint16_t rawVoltages[Entities::MAX_CELLS];

        if (!m_afe.startCellConversion()) {
            return false;
        }

        // Wait for conversion (could be async in real implementation)
        m_afe.waitConversionComplete();

        if (!m_afe.readCellVoltages(rawVoltages, pack.cellCount)) {
            return false;
        }

        // Map to entities
        for (uint8_t i = 0; i < pack.cellCount; ++i) {
            pack.cells[i].voltage.millivolts = rawVoltages[i];
        }

        return true;
    }

    bool readAllTemperatures(Entities::Pack& pack) override {
        int16_t rawTemps[Entities::MAX_TEMP_SENSORS];

        if (!m_afe.readTemperatures(rawTemps, Entities::MAX_TEMP_SENSORS)) {
            return false;
        }

        // Map temperatures to cells (depends on sensor placement)
        for (uint8_t i = 0; i < pack.cellCount; ++i) {
            pack.cells[i].temperature.deciCelsius = rawTemps[i % Entities::MAX_TEMP_SENSORS];
        }

        return true;
    }

    bool readPackCurrent(int32_t& currentMilliamps) override {
        return m_afe.readCurrent(currentMilliamps);
    }

    bool startBalancing(uint8_t cellIndex) override {
        return m_afe.enableBalancing(cellIndex, true);
    }

    bool stopBalancing(uint8_t cellIndex) override {
        return m_afe.enableBalancing(cellIndex, false);
    }

    bool stopAllBalancing() override {
        return m_afe.disableAllBalancing();
    }

private:
    LTC6811Driver& m_afe;
};

} // namespace Infrastructure
```

---

### Main Application

```cpp
// ============================================================
// src/app/BMSApplication.hpp
// ============================================================

#pragma once
#include "entities/BMSState.hpp"
#include "use_cases/protection/OverVoltageProtectionUseCase.hpp"
#include "use_cases/cell_balancing/CellBalancingUseCase.hpp"
#include "adapters/can/CANMessageHandler.hpp"
#include "adapters/can/CANResponseBuilder.hpp"

namespace App {

class BMSApplication {
public:
    BMSApplication(
        UseCases::ICellDataProvider& cellDataProvider,
        UseCases::IContactorControl& contactorControl,
        Infrastructure::ICANTransmitter& canTransmitter);

    void initialize();

    // Called from main loop - periodic tasks
    void runCycle();

    // Called from CAN RX interrupt
    void onCANMessageReceived(const Adapters::CAN::CANMessage& msg);

private:
    Entities::BMSState m_state;

    // Use cases
    std::unique_ptr<UseCases::Protection::OverVoltageProtectionUseCase> m_ovProtection;
    std::unique_ptr<UseCases::Balancing::CellBalancingUseCase> m_balancing;

    // Adapters
    std::unique_ptr<Adapters::CAN::CANResponseBuilder> m_presenter;
    std::unique_ptr<Adapters::CAN::CANMessageHandler> m_controller;

    // Task timing
    uint32_t m_lastVoltageReadMs = 0;
    uint32_t m_lastProtectionCheckMs = 0;

    static constexpr uint32_t VOLTAGE_READ_INTERVAL_MS = 100;
    static constexpr uint32_t PROTECTION_CHECK_INTERVAL_MS = 10;
};

} // namespace App
```

```cpp
// ============================================================
// src/main.cpp
// ============================================================

#include "app/BMSApplication.hpp"
#include "infrastructure/drivers/stm32/STM32_CAN.hpp"
#include "infrastructure/drivers/afe/LTC6811Driver.hpp"
#include "infrastructure/repositories/AFECellDataProvider.hpp"
#include "infrastructure/repositories/GPIOContactorControl.hpp"
#include "infrastructure/can/EmotasCANStack.hpp"

// Global application instance (embedded often uses globals)
static App::BMSApplication* g_app = nullptr;

// CAN RX interrupt handler
extern "C" void CAN1_RX0_IRQHandler() {
    Adapters::CAN::CANMessage msg;
    // Read message from hardware...

    if (g_app) {
        g_app->onCANMessageReceived(msg);
    }
}

int main() {
    // ========== Hardware Init ==========
    SystemClock_Config();

    // ========== Infrastructure ==========
    Infrastructure::STM32_SPI spi(SPI1);
    Infrastructure::LTC6811Driver afeDriver(spi);

    Infrastructure::EmotasCANStack canStack;
    canStack.initialize(0x10, 500);  // Node ID 0x10, 500kbps

    // ========== Repositories ==========
    Infrastructure::AFECellDataProvider cellDataProvider(afeDriver);
    Infrastructure::GPIOContactorControl contactorControl;

    // ========== Application ==========
    App::BMSApplication app(cellDataProvider, contactorControl, canStack);
    g_app = &app;

    app.initialize();

    // ========== Main Loop ==========
    while (true) {
        app.runCycle();

        // Optional: sleep or wait for interrupt
        __WFI();
    }

    return 0;
}
```

---

## Embedded-Specific Considerations

| Concern | Solution |
|---------|----------|
| **No dynamic allocation** | Use `std::array`, stack allocation, or static objects |
| **Real-time constraints** | Keep use cases deterministic, no blocking calls |
| **Interrupt safety** | Use message queues between ISR and main loop |
| **Limited RAM** | Entities use fixed-size arrays, no `std::vector` |
| **Testability** | Mock HAL interfaces for unit tests on host |

---

## Summary

```
┌─────────────────────────────────────────────────────────────────┐
│ EMOTAS CAN (External Interface)                                 │
│     │                                        ▲                  │
│     │ CAN RX                                 │ CAN TX           │
│     ▼                                        │                  │
├─────────────────────────────────────────────────────────────────┤
│ CANMessageHandler ──────────────────► CANResponseBuilder        │
│   (Controller)                            (Presenter)           │
│     │                                        ▲                  │
│     │ Request                                │ Response         │
│     ▼                                        │                  │
├─────────────────────────────────────────────────────────────────┤
│ Use Cases (Protection, Balancing, SOC, etc.)                    │
│     │                                        ▲                  │
│     │ Uses                                   │ Returns          │
│     ▼                                        │                  │
├─────────────────────────────────────────────────────────────────┤
│ Entities (Cell, Pack, Fault, BMSState)                          │
├─────────────────────────────────────────────────────────────────┤
│ ICellDataProvider    IContactorControl    IConfigStorage        │
│     ▲                      ▲                   ▲                │
│     │ implements           │                   │                │
├─────────────────────────────────────────────────────────────────┤
│ AFECellDataProvider  GPIOContactorControl  EEPROMConfigStorage  │
│     │                      │                   │                │
│     ▼                      ▼                   ▼                │
├─────────────────────────────────────────────────────────────────┤
│ LTC6811 Driver       GPIO HAL              EEPROM Driver        │
│     │                      │                   │                │
│     ▼                      ▼                   ▼                │
├─────────────────────────────────────────────────────────────────┤
│                        HARDWARE                                 │
│   SPI ◄──► LTC6811    GPIO ◄──► Contactors    I2C ◄──► EEPROM  │
└─────────────────────────────────────────────────────────────────┘
```

This architecture gives you:
- **Testable** business logic (use cases) without hardware
- **Swappable** CAN stacks (EMOTAS ↔ other vendors)
- **Portable** across MCUs (just reimplement HAL layer)
- **Maintainable** separation of concerns
