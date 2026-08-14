#include <gtest/gtest.h>

#include "gateway/fake_provider.h"
#include "gateway/gateway.h"

using devpilot::gateway::ChatMessage;
using devpilot::gateway::ChatRequest;
using devpilot::gateway::FakeProvider;
using devpilot::gateway::Gateway;

TEST(GatewayTest, RoutesToRegisteredProvider)
{
    Gateway gw;
    gw.register_provider(std::make_shared<FakeProvider>("fake"));

    ChatRequest req;
    req.model = "fake-model";
    req.messages.push_back(ChatMessage{.role = "user", .content = "hello"});

    const auto result = gw.complete("fake", req);
    EXPECT_EQ(result.content, "fake reply to: hello");
    EXPECT_GT(result.prompt_tokens, 0);
}

TEST(GatewayTest, UnknownProviderThrows)
{
    Gateway gw;
    gw.register_provider(std::make_shared<FakeProvider>("fake"));

    ChatRequest req;
    req.messages.push_back(ChatMessage{.role = "user", .content = "hi"});
    EXPECT_THROW(gw.complete("openai", req), std::out_of_range);
}

TEST(GatewayTest, ListsRegisteredProviders)
{
    Gateway gw;
    gw.register_provider(std::make_shared<FakeProvider>("fake"));
    gw.register_provider(std::make_shared<FakeProvider>("ollama"));

    const auto names = gw.providers();
    ASSERT_EQ(names.size(), 2);
    EXPECT_EQ(names[0], "fake");
    EXPECT_EQ(names[1], "ollama");
}

TEST(GatewayTest, SameNameProviderOverwrites)
{
    Gateway gw;
    gw.register_provider(std::make_shared<FakeProvider>("fake"));
    gw.register_provider(std::make_shared<FakeProvider>("fake"));
    EXPECT_EQ(gw.providers().size(), 1);
}