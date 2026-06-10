#define TRY(expr, ReturnType) ({                       \
    auto _r = (expr);                                  \
    if (_r.isErr())                                    \
    {                                                  \
        auto _e = _r.unwrapErr();                      \
        _e.pushFrame(std::source_location::current()); \
        return Result<ReturnType>::Err(_e);            \
    }                                                  \
    _r.unwrap();                                       \
})

#define TEST(expr) ({                                  \
    auto _r = (expr);                                  \
    if (_r.isErr())                                    \
    {                                                  \
        auto _e = _r.unwrapErr();                      \
        _e.pushFrame(std::source_location::current()); \
        return Result<void>::Err(_e);                  \
    }                                                  \
})
