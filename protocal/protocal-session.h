#ifndef __PROTOCAL_SESSION_H__
#define __PROTOCAL_SESSION_H__

#include "protocal-base.h"

#include <string>

namespace Protocal
{
    enum class SessionType : uint16_t
    {
        LOGIN = 1,  // 登录
        REGIST = 2, // 注册
        UPLOAD = 3, // 上传
    };

    struct SessionCommand : public Command
    {
        SessionCommand() : Command(TO_UNDERLYING(BaseCommandType::SESSION_SERVER_BASE))
        {
            type = 0;
        }

        SessionCommand(SessionType commanType) : SessionCommand()
        {
            type = TO_UNDERLYING(commanType);
        }

        SessionCommand(const std::string &data)
        {
            unserialize(data);
        }

        std::string serialize(const std::string &data)
        {
            std::string res(sizeof(Command) + sizeof(this->type), '\0');
            ::memcpy(res.data(), static_cast<void *>(this), sizeof(Command));
            ::memcpy(res.data() + sizeof(Command), &type, sizeof(type));
            res.append(data);
            return std::move(res);
        }

        const std::string &unserialize(const std::string &data)
        {
            if (data.size() >= sizeof(Command) + sizeof(this->type))
            {
                ::memcpy(&this->base_type, data.data(), sizeof(Command));
                ::memcpy(&this->type, data.data() + sizeof(Command), sizeof(this->type));
                this->data = data.substr(sizeof(Command) + sizeof(this->type));
            }
            return this->data;
        }

        std::string &unserialize()
        {
            return this->data;
        }

        const std::string &unserialize() const
        {
            return this->data;
        }

        uint16_t type;
        std::string data;
    };
};

#endif //__PROTOCAL_SESSION_H__