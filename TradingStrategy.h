#pragma once

#include <vector>
#include <torch/torch.h>
#include "BinanceBot.h"

class TradingStrategy 
{
torch::nn::Sequential model;
    std::unique_ptr<torch::optim::Optimizer> optimizer;
    torch::nn::MSELoss loss_function;
    std::vector<torch::Tensor> input_tensors;
    std::vector<torch::Tensor> label_tensors;

public:
    TradingStrategy();
    ~TradingStrategy();
    void train(const std::vector<std::vector<double>>& data);
    void trade(BinanceBot& bot);

  
};

inline TradingStrategy::TradingStrategy()
{
    model = torch::nn::Sequential(
        torch::nn::Linear(10, 64),
        torch::nn::ReLU(),
        torch::nn::Linear(64, 32),
        torch::nn::ReLU(),
        torch::nn::Linear(32, 1)
    );

    optimizer = std::make_unique<torch::optim::Adam>(model->parameters(), torch::optim::AdamOptions(0.001));
    loss_function = torch::nn::MSELoss();
}

inline TradingStrategy::~TradingStrategy() {}

inline void TradingStrategy::train(const std::vector<std::vector<double>>& data)
{
    int data_size = data.size();
    std::vector<torch::Tensor> input_tensors;
    std::vector<torch::Tensor> label_tensors;

    for (size_t i = 0; i < data_size; ++i) {
        // Przyk³ad: zamieniaj dane na tensory (cechy i etykiety)
        auto input_tensor = torch::from_blob(data[i].data(), { 1, data[i].size() - 1 }, torch::kFloat32); // Ostatnia kolumna to etykieta
        auto label_tensor = torch::from_blob(&data[i].back(), { 1 }, torch::kFloat32); // U¿ywamy tylko ostatniej kolumny jako etykiety

        input_tensors.push_back(input_tensor);
        label_tensors.push_back(label_tensor);
    }

    int num_epochs = 100;
    for (int epoch = 0; epoch < num_epochs; ++epoch) {
        float epoch_loss = 0.0f;
        for (size_t i = 0; i < data_size; ++i) {
            optimizer->zero_grad();
            torch::Tensor prediction = model->forward(input_tensors[i]);
            torch::Tensor loss = loss_function(prediction, label_tensors[i]);
            loss.backward();
            optimizer->step();
            epoch_loss += loss.item<float>();
        }

        epoch_loss /= data_size;

        if (epoch % 10 == 0) {
            std::cout << "Epoch: " << epoch << ", Loss: " << epoch_loss << std::endl;
        }
    }
}


inline void TradingStrategy::trade(BinanceBot& bot) 
{
   
}
