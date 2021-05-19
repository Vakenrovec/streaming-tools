#pragma once

#include "IClient.h"

class MainWindow;

class GuiClient: public IClient
{
public:
    GuiClient(){};
    ~GuiClient(){};

    int Run();
    
    void StartAsync() override { Run(); };
    void HandleEvents() override {};
    void Destroy() override {};

private:
    friend class MainWindow; 
};
