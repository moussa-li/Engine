#include <iostream>

#include "Core/Application.hpp"
#include "DataBase/Context.hpp"


int main()
{
    auto app = EgLab::Platform::Context::instance().getApplication();
    app.exec();
}