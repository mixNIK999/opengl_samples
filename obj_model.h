#pragma once

#include <memory>

struct SomeModel
{
    virtual ~SomeModel(){}
    virtual void draw() = 0;
};

struct IObjModel : SomeModel{};

std::shared_ptr<IObjModel> create_model(char const * fileaname);