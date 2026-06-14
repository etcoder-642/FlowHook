#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <optional>

#include "error.h"

namespace flowhook
{
    template <typename T>
    class Result
    {
    private:
        std::variant<T, FWError> data;

        Result() = default;

    public:
        // Factory methods
        // Factory methods
        static Result<T> Ok(T value)
        {
            Result r;
            r.data = std::move(value);
            return r;
        }

        static Result<T> Err(ErrorCode code, std::string message)
        {
            Result r;
            r.data = FWError(code, message);
            return r;
        }

        static Result<T> Err(FWError err)
        {
            Result r;
            r.data = err;
            return r;
        }

        // State Checks
        bool isOk() const
        {
            return std::holds_alternative<T>(data);
        }

        bool isErr() const
        {
            return std::holds_alternative<FWError>(data);
        }

        // Extraction
        T unwrap()
        {
            return std::get<T>(data);
        }

        FWError unwrapErr()
        {
            return std::get<FWError>(data);
        }

        ErrorCode getErrCode()
        {
            return std::get<FWError>(data).code;
        }

        std::string getErrMessage()
        {
            return std::get<FWError>(data).message;
        }
    };

    // Void Specialization
    template <>
    class Result<void>
    {
    private:
        std::optional<FWError> err_;

        Result() = default;

    public:
        // Factory methods
        // Factory methods
        static Result<void> Ok()
        {
            return Result{};
        }

        static Result<void> Err(ErrorCode code, std::string message)
        {
            Result r;
            r.err_ = FWError(code, message);
            return r;
        }

        static Result<void> Err(FWError err)
        {
            Result r;
            r.err_ = err;
            return r;
        }

        // State Checks
        bool isOk() const
        {
            return !err_.has_value();
        }

        bool isErr() const
        {
            return err_.has_value();
        }

        // Extraction
        FWError unwrapErr()
        {
            return err_.value();
        }

        ErrorCode getErrCode()
        {
            return err_.value().code;
        }

        std::string getErrMessage()
        {
            return err_.value().message;
        }

    };
}
