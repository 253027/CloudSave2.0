#ifndef __MG_NONCOPYABLE_H__
#define __MG_NONCOPYABLE_H__

namespace mg
{

    class noncopyable
    {
    public:
        noncopyable(const noncopyable &) = delete;

        void operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;

        ~noncopyable() = default;
    };
};

#endif //__MG_NONCOPYABLE_H__