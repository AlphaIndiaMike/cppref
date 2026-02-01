Designing an Interactor (often called a **Use Case**) is a central part of Clean Architecture. The Interactor encapsulates a specific business rule (e.g., "Process Payment" or "Update User Profile").

The design goal is **decoupling**: The Interactor should not know about the UI (Views) or how data is physically stored (Database).

Here is how to design the interface, request data, and response data.

---

### 1. Designing the Interactor Interface

The interface defines how the outside world (UI/Presenter) talks to the Interactor. It should be simple and functional.

#### The "Single Responsibility" Approach

The most robust design is to have one Interactor per action. Avoid creating "God Interactors" like `UserInteractor` that do everything (login, logout, update, delete). Instead, creating specific classes:

* `LoginUserInteractor`
* `UpdateUserProfileInteractor`

#### The Input Boundary (The Method Signature)

The interface generally needs a single entry point, often named `execute` or `invoke`.

**Modern Approach (Async/Await or Coroutines):**
In modern languages (Kotlin, Swift, TypeScript), you often return the result directly using async primitives.

```typescript
interface UpdateUserProfileUseCase {
    execute(request: UpdateProfileRequest): Promise<UpdateProfileResponse>;
}

```

**Classic Approach (Callbacks/Output Boundary):**
In stricter implementations (like VIPER), you use an **Output Boundary** interface to decouple the return value. This is useful if the Interactor needs to report progress or multiple states.

```java
// The Input
interface UpdateUserProfileInput {
    void execute(UpdateProfileRequest request);
}

// The Output (implemented by the Presenter)
interface UpdateUserProfileOutput {
    void onProfileUpdated(UpdateProfileResponse response);
    void onError(String message);
}

```

---

### 2. Request Data Storage (The Input)

When you ask about "Request Data Storage," in the context of an Interactor, this usually refers to **Request Models** (DTOs - Data Transfer Objects). This is how data is temporarily "stored" as it travels from the UI to the Interactor.

**Best Practices for Request Data:**

* **Encapsulation:** Do not pass raw entities (e.g., a `User` database row) into the Interactor. Create a specific structure for the request.
* **Immutability:** The request data should be read-only.
* **Validation:** The Request Model is the perfect place to "store" the validation logic for *format* (e.g., "is this email valid?"), while the Interactor handles business validation (e.g., "does this email exist?").

**Example Structure:**

```typescript
// This is the "Storage" for the incoming request
class UpdateProfileRequest {
    public readonly userId: string;
    public readonly newEmail: string;

    constructor(userId: string, newEmail: string) {
        this.userId = userId;
        this.newEmail = newEmail;
        this.validate();
    }

    private validate() {
        if (!this.newEmail.includes("@")) throw new Error("Invalid Email");
    }
}

```

---

### 3. Response Data Storage (The Output)

"Response Data Storage" refers to the data passed back to the UI. The Interactor must **transform** the internal business entities into a format the UI can use.

**Best Practices for Response Data:**

* **Sanitization:** Never return the full Database Entity. You might accidentally leak a password hash or internal ID.
* **Formatting:** The Response Model should contain raw data (Strings, Ints), not UI logic (Colors, Fonts).
* **State:** The response usually includes a status (Success, Failure, Loading).

**Example Structure:**

```typescript
// This is the "Storage" for the outgoing response
class UpdateProfileResponse {
    public readonly success: boolean;
    public readonly message: string;
    public readonly updatedEmail: string | null;

    // Factory method for success
    static success(email: string) {
        return new UpdateProfileResponse(true, "Profile Updated", email);
    }

    // Factory method for failure
    static failure(reason: string) {
        return new UpdateProfileResponse(false, reason, null);
    }
}

```

---

### 4. Implementation Strategy: Connecting the Pieces

Here is how the Interactor utilizes these data structures. Note that the **actual persistence** (saving to a hard drive) is delegated to a `Repository`, not the Interactor itself.

**The Workflow:**

1. **UI** creates `RequestModel`.
2. **Interactor** receives `RequestModel`.
3. **Interactor** calls `Repository` to get/save Entities (Business logic).
4. **Interactor** converts Entity  `ResponseModel`.
5. **UI** receives `ResponseModel`.

#### Code Example (Conceptual)

```typescript
class UpdateUserProfileInteractor implements UpdateUserProfileUseCase {

    constructor(private userRepository: UserRepository) {}

    async execute(request: UpdateProfileRequest): Promise<UpdateProfileResponse> {
        // 1. Business Logic / Validation
        const user = await this.userRepository.findById(request.userId);

        if (!user) {
            return UpdateProfileResponse.failure("User not found");
        }

        // 2. Modify Entity
        user.email = request.newEmail;

        // 3. Persist Data (Actual Storage)
        await this.userRepository.save(user);

        // 4. Return Response Data (Transport Storage)
        return UpdateProfileResponse.success(user.email);
    }
}

```

This is a classic Clean Architecture flow, but implementing it in C++ requires specific attention to **memory ownership, move semantics, and pure virtual interfaces** to ensure true decoupling without sacrificing performance.

Here is how you design that architecture in modern C++ (C++17/20).

### 1. The Data "Storage" (Request & Response)

In C++, "storage" for requests and responses usually means **Data Transfer Objects (DTOs)**. You don't want complex objects here; you want plain structs.

**Key C++ Design Choice:** Use `struct` with value semantics. We rely on **Move Semantics** to pass data efficiently without deep copying.

```cpp
#include <string>
#include <optional>

// Request Data: Immutable once created
struct LoginRequest {
    std::string username;
    std::string password;
};

// Response Data: simple state representation
struct LoginResponse {
    bool success;
    std::string message;
    std::optional<std::string> authToken; // C++17 optional for nullable data
};

```

---

### 2. The System Boundary (Data Interface)

The Interactor needs data but shouldn't know *how* it's fetched. We use a **Pure Virtual Class** (Interface) for the Repository.

**Key C++ Design Choice:** Pass this interface into the Interactor using `std::shared_ptr` or `std::unique_ptr` so the Interactor owns the connection or shares it, but doesn't manage the concrete implementation's lifecycle.

```cpp
// The Interface (Abstract Base Class)
class IAuthRepository {
public:
    virtual ~IAuthRepository() = default; // Virtual destructor is mandatory
    virtual bool validateCredentials(const std::string& user, const std::string& pass) = 0;
    virtual std::string generateToken(const std::string& user) = 0;
};

```

---

### 3. The Interactor (Single Responsibility)

The Interactor encapsulates the use case. It is generally **stateless**. It takes input, processes it via the repository, and returns output.

**Key C++ Design Choice:**

* **Dependency Injection:** Injected via constructor.
* **No UI dependencies:** Strictly standard library types.

```cpp
#include <memory>

class LoginInteractor {
private:
    std::shared_ptr<IAuthRepository> _repository;

public:
    // Inject the dependency
    explicit LoginInteractor(std::shared_ptr<IAuthRepository> repo)
        : _repository(std::move(repo)) {}

    // The "Execute" method
    LoginResponse execute(const LoginRequest& request) {
        if (request.username.empty() || request.password.empty()) {
            return {false, "Invalid input", std::nullopt};
        }

        if (_repository->validateCredentials(request.username, request.password)) {
            std::string token = _repository->generateToken(request.username);
            return {true, "Login Successful", token};
        }

        return {false, "Invalid Credentials", std::nullopt};
    }
};

```

---

### 4. The Controller (Stateful)

You mentioned the Controller is **Stateful**. In C++, this often acts as the "glue" or the ViewModel context. It holds the current state of the interaction (e.g., is it loading? what was the last error?).

**Key C++ Design Choice:**

* It owns the `Interactor`.
* It stores the `lastResponse`.
* It bridges the Presenter (UI) and Interactor.

```cpp
class LoginController {
private:
    std::unique_ptr<LoginInteractor> _interactor;

    // STATE STORAGE: The Controller remembers what happened
    LoginResponse _lastState;
    bool _isProcessing;

public:
    explicit LoginController(std::unique_ptr<LoginInteractor> interactor)
        : _interactor(std::move(interactor)), _isProcessing(false), _lastState{false, "", std::nullopt} {}

    void handleLogin(const std::string& user, const std::string& pass) {
        _isProcessing = true;

        // Create Request DTO
        LoginRequest req{user, pass};

        // Execute Use Case
        _lastState = _interactor->execute(req);

        _isProcessing = false;

        // Notify Presenter (Simplified here as a print, normally a callback or observer)
        notifyPresenter();
    }

    // Presenters read this state
    const LoginResponse& getState() const { return _lastState; }
    bool isBusy() const { return _isProcessing; }

private:
    void notifyPresenter() {
        // In a real app, calls IPresenter::updateView(_lastState)
    }
};

```

---

### 5. Putting it together (Composition Root)

This is how you wire it up in your `main()` or Application class.

```cpp
#include <iostream>

// A concrete implementation of the System Boundary
class SQLAuthRepository : public IAuthRepository {
public:
    bool validateCredentials(const std::string& user, const std::string& pass) override {
        // Imagine SQL query here
        return (user == "admin" && pass == "1234");
    }
    std::string generateToken(const std::string& user) override {
        return "ABC-123-TOKEN";
    }
};

int main() {
    // 1. Create the Database/System Boundary
    auto repo = std::make_shared<SQLAuthRepository>();

    // 2. Create the Interactor (injecting the repo)
    auto interactor = std::make_unique<LoginInteractor>(repo);

    // 3. Create the Controller (injecting the interactor)
    LoginController controller(std::move(interactor));

    // 4. User Interaction
    controller.handleLogin("admin", "1234");

    // 5. Check State
    auto state = controller.getState();
    if (state.success) {
        std::cout << "Success: " << *state.authToken << std::endl;
    } else {
        std::cout << "Error: " << state.message << std::endl;
    }

    return 0;
}

```

Your proposal is **mostly compliant**, but there is a critical distinction to make regarding **where** the Request/Response data structures are defined to strictly follow the Dependency Rule of Clean Architecture.

Here is the breakdown of your specific points.

### 1. "Usually an interactor has only one execute method right?"

**Yes, absolutely.**
This is the standard **Command Pattern**. Ideally, an Interactor represents a *single* semantic action (e.g., `RegisterUser`, `DownloadFile`, `CalculateTax`).

* **Why?** It adheres to the **Single Responsibility Principle (SRP)**. If you have `UserInteractor` with `login()`, `logout()`, and `register()`, you are creating a "God Class" that will eventually grow too large and coupling unrelated logic.
* **In C++:** It is very common to overload the `operator()` so the object itself acts like a function (a Functor).

### 2. "And response / request abstract classes?"

**Correction:** In 99% of Clean Architecture implementations, Request and Response models are **NOT abstract classes (interfaces)**. They are simple **Data Structures (DTOs - Data Transfer Objects)**.

You typically do not want polymorphism here. You want a dumb bucket of data that has no behavior. In C++, these are standard `structs`.

### 3. "Are defined by the controller? or inside the interactor?"

**They are defined by the Interactor (The Inner Circle).**

This is the most common mistake. To adhere to the **Dependency Rule**, dependencies must point **inward**.

* The **Interactor** is High-Level Policy (Inner Circle).
* The **Controller** is an Interface Adapter (Outer Circle).

If the Controller defines the Request object, and the Interactor uses it, the Interactor would depend on the Controller. **This violates the Clean Architecture.** The Interactor must stand alone. It must define its own "Input Port" and "Output Port."

### 4. "It means the controller will include the abstract classes (interfaces) and has access to methods regardless."

Since the definitions live with the Interactor, the **Controller must include the Interactor's header files**.

Here is the correct dependency flow:

1. **Interactor Layer** defines: `struct LoginRequest`, `struct LoginResponse`, and `class LoginInteractor`.
2. **Controller Layer** includes: `#include "LoginInteractor.h"`
3. **Controller** instantiates the Request struct and passes it to the Interactor.

---

### The Correct C++ Structure

Here is how you organize the headers to keep the proposal compliant. Note that the Interactor knows *nothing* about the Controller.

#### The Interactor Header (`LoginInteractor.h`)

*Lives in the Core / Domain Layer*

```cpp
#pragma once
#include <string>

// 1. Defined HERE, not in the Controller
// These are simple structs, not abstract classes.
struct LoginRequest {
    std::string username;
    std::string password;
};

struct LoginResponse {
    bool success;
    std::string errorMsg;
};

// 2. The Interactor Interface
class LoginInteractor {
public:
    // Single Execute Method
    virtual LoginResponse execute(const LoginRequest& request);
};

```

#### The Controller (`LoginController.cpp`)

*Lives in the Interface Adapter Layer*

```cpp
#include "LoginInteractor.h" // Dependency points INWARD

class LoginController {
    LoginInteractor& interactor; // usage

public:
    void onLoginButtonClicked(std::string u, std::string p) {

        // Controller adheres to the Interactor's contract
        LoginRequest req;
        req.username = u;
        req.password = p;

        LoginResponse res = interactor.execute(req);

        // Handle response...
    }
};

```


---

## 1. Data Storage Structures (Request/Response)

```cpp
#include <string>
#include <optional>
#include <variant>
#include <chrono>

// ============== Error Handling ==============
struct DomainError {
    std::string code;
    std::string message;

    static DomainError validation(const std::string& msg) {
        return {"VALIDATION_ERROR", msg};
    }
    static DomainError notFound(const std::string& msg) {
        return {"NOT_FOUND", msg};
    }
};

// Result type - either success with data or failure with error
template<typename T>
class Result {
public:
    static Result success(T data) {
        return Result(std::move(data));
    }

    static Result failure(DomainError error) {
        return Result(std::move(error));
    }

    bool isSuccess() const { return std::holds_alternative<T>(m_value); }
    bool isFailure() const { return !isSuccess(); }

    const T& data() const { return std::get<T>(m_value); }
    T&& extractData() { return std::get<T>(std::move(m_value)); }
    const DomainError& error() const { return std::get<DomainError>(m_value); }

private:
    explicit Result(T data) : m_value(std::move(data)) {}
    explicit Result(DomainError error) : m_value(std::move(error)) {}

    std::variant<T, DomainError> m_value;
};
```

---

## 2. Base Interactor Interface

```cpp
// ============== Interactor Interface ==============
template<typename TRequest, typename TResponse>
class IInteractor {
public:
    virtual ~IInteractor() = default;
    virtual Result<TResponse> execute(const TRequest& request) = 0;
};

// For async operations
template<typename TRequest, typename TResponse>
class IAsyncInteractor {
public:
    virtual ~IAsyncInteractor() = default;
    virtual std::future<Result<TResponse>> executeAsync(const TRequest& request) = 0;
};
```

---

## 3. System Boundary / Repository Interface

```cpp
// ============== Entity ==============
struct User {
    std::string id;
    std::string email;
    std::string name;
    std::chrono::system_clock::time_point createdAt;
};

// ============== Data Gateway Interface ==============
template<typename TEntity, typename TId = std::string>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual std::optional<TEntity> findById(const TId& id) = 0;
    virtual std::vector<TEntity> findAll() = 0;
    virtual TEntity save(const TEntity& entity) = 0;
    virtual bool remove(const TId& id) = 0;
};

// Specific repository interface with domain-specific queries
class IUserRepository : public IRepository<User> {
public:
    virtual std::optional<User> findByEmail(const std::string& email) = 0;
};
```

---

## 4. Concrete Interactor Example

```cpp
// ============== Request / Response DTOs ==============
struct CreateUserRequest {
    std::string email;
    std::string name;
};

struct CreateUserResponse {
    std::string userId;
    std::string email;
    std::string name;
};

// ============== Interactor Implementation ==============
class CreateUserInteractor : public IInteractor<CreateUserRequest, CreateUserResponse> {
public:
    explicit CreateUserInteractor(std::shared_ptr<IUserRepository> userRepo)
        : m_userRepo(std::move(userRepo)) {}

    Result<CreateUserResponse> execute(const CreateUserRequest& request) override {
        // Validation
        if (request.email.empty()) {
            return Result<CreateUserResponse>::failure(
                DomainError::validation("Email cannot be empty")
            );
        }

        // Check uniqueness
        if (m_userRepo->findByEmail(request.email).has_value()) {
            return Result<CreateUserResponse>::failure(
                DomainError::validation("Email already exists")
            );
        }
        // Create entity
        User user;
        user.id = generateId();  // your ID generation
        user.email = request.email;
        user.name = request.name;
        user.createdAt = std::chrono::system_clock::now();

        // Persist
        auto savedUser = m_userRepo->save(user);

        // Map to response
        return Result<CreateUserResponse>::success({
            savedUser.id,
            savedUser.email,
            savedUser.name
        });
    }

private:
    std::shared_ptr<IUserRepository> m_userRepo;

    std::string generateId() { /* UUID generation */ return "uuid-xxx"; }
};
```

---

## 5. Stateful Controller

```cpp
// ============== Controller State ==============
enum class ControllerState {
    Idle,
    Processing,
    Success,
    Error
};

// ============== Controller ==============
template<typename TInteractor, typename TRequest, typename TResponse>
class Controller {
public:
    explicit Controller(std::shared_ptr<TInteractor> interactor)
        : m_interactor(std::move(interactor))
        , m_state(ControllerState::Idle) {}

    void submit(const TRequest& request) {
        m_state = ControllerState::Processing;
        m_lastRequest = request;

        auto result = m_interactor->execute(request);

        if (result.isSuccess()) {
            m_state = ControllerState::Success;
            m_lastResponse = result.data();
            m_lastError.reset();
        } else {
            m_state = ControllerState::Error;
            m_lastError = result.error();
            m_lastResponse.reset();
        }

        notifyPresenter();
    }

    // State accessors
    ControllerState state() const { return m_state; }
    const std::optional<TResponse>& response() const { return m_lastResponse; }
    const std::optional<DomainError>& error() const { return m_lastError; }

    // Presenter binding
    void setPresenter(std::function<void(const Controller&)> presenter) {
        m_presenterCallback = std::move(presenter);
    }

private:
    void notifyPresenter() {
        if (m_presenterCallback) {
            m_presenterCallback(*this);
        }
    }

    std::shared_ptr<TInteractor> m_interactor;
    ControllerState m_state;

    std::optional<TRequest> m_lastRequest;
    std::optional<TResponse> m_lastResponse;
    std::optional<DomainError> m_lastError;

    std::function<void(const Controller&)> m_presenterCallback;
};
```

---

## 6. Presenter Interface

```cpp
// ============== View Model ==============
struct UserViewModel {
    std::string displayName;
    std::string emailDisplay;
    bool isValid = false;
    std::string errorMessage;
};

// ============== Presenter Interface ==============
template<typename TViewModel>
class IPresenter {
public:
    virtual ~IPresenter() = default;
    virtual void present(const TViewModel& viewModel) = 0;
};

// ============== Concrete Presenter ==============
class CreateUserPresenter : public IPresenter<UserViewModel> {
public:
    void present(const UserViewModel& vm) override {
        if (vm.isValid) {
            std::cout << "User created: " << vm.displayName
                      << " <" << vm.emailDisplay << ">\n";
        } else {
            std::cerr << "Error: " << vm.errorMessage << "\n";
        }
    }

    // Maps controller state to view model
    template<typename TController>
    UserViewModel mapToViewModel(const TController& controller) {
        UserViewModel vm;

        if (controller.state() == ControllerState::Success && controller.response()) {
            vm.isValid = true;
            vm.displayName = controller.response()->name;
            vm.emailDisplay = controller.response()->email;
        } else if (controller.error()) {
            vm.isValid = false;
            vm.errorMessage = controller.error()->message;
        }

        return vm;
    }
};
```

---

## 7. Wiring It All Together

```cpp
int main() {
    // === Dependency Injection / Composition Root ===

    // System boundary (would be a real DB implementation)
    auto userRepo = std::make_shared<InMemoryUserRepository>();

    // Interactor
    auto createUserInteractor = std::make_shared<CreateUserInteractor>(userRepo);

    // Controller
    using UserController = Controller<CreateUserInteractor, CreateUserRequest, CreateUserResponse>;
    auto controller = std::make_shared<UserController>(createUserInteractor);

    // Presenter
    auto presenter = std::make_shared<CreateUserPresenter>();

    // Bind presenter to controller
    controller->setPresenter([presenter](const UserController& ctrl) {
        auto viewModel = presenter->mapToViewModel(ctrl);
        presenter->present(viewModel);
    });

    // === Execute Use Case ===
    CreateUserRequest request{"john@example.com", "John Doe"};
    controller->submit(request);

    return 0;
}
```

---

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         PRESENTATION                            │
│  ┌──────────┐        ┌────────────────┐        ┌─────────────┐  │
│  │   View   │◄───────│    Presenter   │◄───────│  ViewModel  │  │
│  └──────────┘        └────────────────┘        └─────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                               ▲
                               │ notifies
┌─────────────────────────────────────────────────────────────────┐
│                          CONTROLLER                             │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  State: Idle | Processing | Success | Error              │   │
│  │  LastRequest / LastResponse / LastError                  │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                               │
                               │ execute(request)
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                          INTERACTOR                             │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  IInteractor<TRequest, TResponse>                        │   │
│  │  - Single Responsibility                                 │   │
│  │  - Returns Result<TResponse>                             │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                               │
                               │ uses interface
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                      SYSTEM BOUNDARY                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │ IRepository │  │ IFileSystem │  │ IExternalServiceGateway │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

