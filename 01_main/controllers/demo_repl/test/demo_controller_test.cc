#include "demo_controller.h"

#include <gtest/gtest.h>

namespace presenter {
namespace {

class DemoControllerTest : public ::testing::Test {
 protected:
  DemoController controller_;
};

// Test: Constructor initializes commands
TEST_F(DemoControllerTest, ConstructorRegistersCommands) {
  auto commands = controller_.GetAvailableCommands();
  EXPECT_EQ(commands.size(), 3);

  // Check that all expected commands are present
  bool has_add = false, has_delete = false, has_list = false;
  for (const auto& cmd : commands) {
    if (cmd.find("add") != std::string::npos) has_add = true;
    if (cmd.find("delete") != std::string::npos) has_delete = true;
    if (cmd.find("list") != std::string::npos) has_list = true;
  }

  EXPECT_TRUE(has_add);
  EXPECT_TRUE(has_delete);
  EXPECT_TRUE(has_list);
}

// Test: GetAvailableCommands returns formatted strings
TEST_F(DemoControllerTest, GetAvailableCommandsReturnsFormattedStrings) {
  auto commands = controller_.GetAvailableCommands();

  for (const auto& cmd : commands) {
    EXPECT_NE(cmd.find(" - "), std::string::npos)
        << "Command should contain ' - ' separator: " << cmd;
  }
}

// Test: Unknown command returns error
TEST_F(DemoControllerTest, HandleRequestWithUnknownCommand) {
  Request req{"unknown", {}};
  Response resp = controller_.HandleRequest(req);

  EXPECT_FALSE(resp.success);
  EXPECT_NE(resp.message.find("Unknown command"), std::string::npos);
}

// Test: Add with single argument
TEST_F(DemoControllerTest, HandleAddSingleWord) {
  Request req{"add", {"hello"}};
  Response resp = controller_.HandleRequest(req);

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("Added"), std::string::npos);
  EXPECT_NE(resp.message.find("hello"), std::string::npos);
}

// Test: Add with multiple arguments
TEST_F(DemoControllerTest, HandleAddMultipleWords) {
  Request req{"add", {"hello", "world", "test"}};
  Response resp = controller_.HandleRequest(req);

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("hello world test"), std::string::npos);
}

// Test: Add with no arguments returns error
TEST_F(DemoControllerTest, HandleAddNoArguments) {
  Request req{"add", {}};
  Response resp = controller_.HandleRequest(req);

  EXPECT_FALSE(resp.success);
  EXPECT_NE(resp.message.find("at least one argument"), std::string::npos);
}

// Test: Add multiple items
TEST_F(DemoControllerTest, HandleAddMultipleItems) {
  controller_.HandleRequest({"add", {"item1"}});
  controller_.HandleRequest({"add", {"item2"}});

  Response resp = controller_.HandleRequest({"list", {}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("item1"), std::string::npos);
  EXPECT_NE(resp.message.find("item2"), std::string::npos);
  EXPECT_NE(resp.message.find("(2)"), std::string::npos);
}

// Test: List when empty
TEST_F(DemoControllerTest, HandleListWhenEmpty) {
  Response resp = controller_.HandleRequest({"list", {}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("No items stored"), std::string::npos);
}

// Test: List with items
TEST_F(DemoControllerTest, HandleListWithItems) {
  controller_.HandleRequest({"add", {"first"}});
  controller_.HandleRequest({"add", {"second"}});

  Response resp = controller_.HandleRequest({"list", {}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("Stored items"), std::string::npos);
  EXPECT_NE(resp.message.find("1. first"), std::string::npos);
  EXPECT_NE(resp.message.find("2. second"), std::string::npos);
}

// Test: Delete when empty
TEST_F(DemoControllerTest, HandleDeleteWhenEmpty) {
  Response resp = controller_.HandleRequest({"delete", {}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("Deleted 0 item(s)"), std::string::npos);
}

// Test: Delete with items
TEST_F(DemoControllerTest, HandleDeleteWithItems) {
  controller_.HandleRequest({"add", {"item1"}});
  controller_.HandleRequest({"add", {"item2"}});
  controller_.HandleRequest({"add", {"item3"}});

  Response resp = controller_.HandleRequest({"delete", {}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("Deleted 3 item(s)"), std::string::npos);

  // Verify items are actually deleted
  Response list_resp = controller_.HandleRequest({"list", {}});
  EXPECT_NE(list_resp.message.find("No items stored"), std::string::npos);
}

// Test: Add, delete, add workflow
TEST_F(DemoControllerTest, AddDeleteAddWorkflow) {
  // Add items
  controller_.HandleRequest({"add", {"first"}});
  controller_.HandleRequest({"add", {"second"}});

  // Delete all
  controller_.HandleRequest({"delete", {}});

  // Add new item
  Response add_resp = controller_.HandleRequest({"add", {"new"}});
  EXPECT_TRUE(add_resp.success);

  // Verify only new item exists
  Response list_resp = controller_.HandleRequest({"list", {}});
  EXPECT_NE(list_resp.message.find("new"), std::string::npos);
  EXPECT_EQ(list_resp.message.find("first"), std::string::npos);
  EXPECT_EQ(list_resp.message.find("second"), std::string::npos);
  EXPECT_NE(list_resp.message.find("(1)"), std::string::npos);
}

// Test: Arguments are ignored for list command
TEST_F(DemoControllerTest, HandleListIgnoresArguments) {
  controller_.HandleRequest({"add", {"item"}});

  Response resp = controller_.HandleRequest({"list", {"ignored", "args"}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("item"), std::string::npos);
}

// Test: Arguments are ignored for delete command
TEST_F(DemoControllerTest, HandleDeleteIgnoresArguments) {
  controller_.HandleRequest({"add", {"item"}});

  Response resp = controller_.HandleRequest({"delete", {"ignored"}});

  EXPECT_TRUE(resp.success);
  EXPECT_NE(resp.message.find("Deleted 1 item(s)"), std::string::npos);
}

}  // namespace
}  // namespace presenter
