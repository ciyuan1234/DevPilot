#include "auth/jwt.h"

#include <chrono>
#include <jwt-cpp/jwt.h>

namespace devpilot::auth {

std::string sign_jwt(uint64_t user_id, const std::string& username,
                     const std::string& secret, std::chrono::seconds ttl)
{
    const auto now = std::chrono::system_clock::now();
    auto token = jwt::create()
                     .set_issuer("devpilot")
                     .set_issued_at(now)
                     .set_expires_at(now + ttl)
                     .set_subject(std::to_string(user_id))
                     .set_payload_claim("username", jwt::claim(username))
                     .sign(jwt::algorithm::hs256{secret});
    return token;
}

std::optional<uint64_t> verify_jwt(const std::string& token, const std::string& secret)
{
    try
    {
        auto decoded = jwt::decode(token);
        const auto verifier = jwt::verify()
                                  .allow_algorithm(jwt::algorithm::hs256{secret})
                                  .with_issuer("devpilot");
        verifier.verify(decoded);

        const auto sub = decoded.get_subject();
        if (sub.empty())
        {
            return std::nullopt;
        }
        return std::stoull(sub);
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

} // namespace devpilot::auth