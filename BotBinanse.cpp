#include <iostream>
#include <string>
#include <json/json.h>
#include "BinanceBot.h"

int main()
{
    BinanceBot bot;
    //std::vector<std::string> all_symbols = bot.get_all_symbols();
    std::vector<std::string> all_symbols;
   // all_symbols.push_back("ETHPLN");
   // all_symbols.push_back("BTCPLN");
   // all_symbols.push_back("BTCUSDT");
   // all_symbols.push_back("ETHUSDT");
    all_symbols.push_back("ETHBTC");
   // all_symbols.push_back("BTCETH");

    for (const auto& symbol : all_symbols) 
    {
        Json::Value ticker_data = bot.get_ticker(symbol);
        std::cout << "Current price of " << symbol << ": " << ticker_data["price"].asString() << std::endl;
 
        std::cout << "Fetching historical data for " << symbol << std::endl;
        std::string output_file = "./historical_data/historical_data_" + symbol + ".csv";
        bot.get_historical_klines(symbol, "5m", output_file);
    }

    // Tutaj możesz dodać własną strategię handlową

    return 0;
}
