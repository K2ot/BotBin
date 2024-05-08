#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <json/json.h>
#include "HttpClient.h"
#include <thread>
#include "ProjectDataTypes.h"

class BinanceBot
{
	HttpClient http_client;
	std::string get_binance_data(const std::string& endpoint);
public:

	std::queue<MarketData>& output_queue;
	BinanceBot(std::queue<MarketData>& output_queue)
		: output_queue(output_queue)
	{};
	~BinanceBot() {};
	Json::Value get_ticker(const std::string& symbol);
	std::vector<std::string> get_all_symbols();
	unsigned int get_historical_klines(const std::string& symbol, const std::string& interval, const int64_t& extStartTime);
};

unsigned int BinanceBot::get_historical_klines(const std::string& symbol, const std::string& interval, const int64_t& extStartTime)
{
	const int limit = 1000; // Maksymalna liczba wyników na ¿¹danie
	int i{ 0 };
	int64_t startTime = extStartTime; // Ustaw startTime na 0, aby pobraæ dane od pocz¹tku istnienia waluty
	std::vector<std::vector<double>> klines;

	while (true)
	{
		std::string endpoint = "/api/v3/klines?symbol="
			+ symbol + "&interval=" + interval + "&startTime="
			+ std::to_string(startTime) + "&limit=" + std::to_string(limit);

		std::string jsonData = get_binance_data(endpoint);

		Json::CharReaderBuilder builder;
		Json::Value root;
		std::string errors;

		std::istringstream dataStream(jsonData);
		if (!Json::parseFromStream(builder, dataStream, &root, &errors))
		{
			std::cout << "Error parsing JSON data: " << errors << std::endl;
			break;
		}

		if (root.empty())
		{
			break;
		}

		for (const auto& kline : root)
		{
			try
			{
				//TODO wrzucaæ do kolejki ca³e paczki a nie pojedyñcze rekordy
				MarketData data;
				data.symbol = symbol;
				data.openTime = kline[0].asInt64();						// Open time: Czas otwarcia œwieczki (w milisekundach od 1970-01-01 UTC).
				data.open = std::stod(kline[1].asString());						// Open: Cena otwarcia œwieczki (na pocz¹tku interwa³u).
				data.high = std::stod(kline[2].asString());						// High: Najwy¿sza cena osi¹gniêta w trakcie interwa³u œwieczki.
				data.low = std::stod(kline[3].asString());						// Low: Najni¿sza cena osi¹gniêta w trakcie interwa³u œwieczki.
				data.close = std::stod(kline[4].asString());						// Close: Cena zamkniêcia œwieczki (na koñcu interwa³u).
				data.volume = std::stod(kline[5].asString());						// Volume: Wolumen handlu dla okresu œwieczki.
				data.closeTime = kline[6].asInt64();					// Close time: Czas zamkniêcia œwieczki (w milisekundach od 1970-01-01 UTC).
				data.quoteAssetVolume = std::stod(kline[7].asString());			// Quote asset volume: Wartoœæ wolumenu handlu wyra¿ona w walucie kwotowania (np. jeœli handlujesz par¹ BTC/USDT, to jest to wartoœæ w USDT).
				data.numberOfTrades = kline[8].asInt();					// Number of trades: Liczba transakcji zrealizowanych w trakcie interwa³u œwieczki.
				data.takerBuyBaseAssetVolume = std::stod(kline[9].asString());	// Taker buy base asset volume: Wolumen zakupu aktywa bazowego przez takerów (osoby akceptuj¹ce oferty) w trakcie interwa³u œwieczki.
				data.takerBuyQuoteAssetVolume = std::stod(kline[10].asString());	// Taker buy quote asset volume: Wartoœæ wolumenu zakupu aktywa bazowego przez takerów wyra¿ona w walucie kwotowania w trakcie interwa³u œwieczki.
				data.ignore = kline[11].asString();						// Ignore: Dodatkowy parametr, który mo¿na zignorowaæ.

				// Dodatkowe pola zwi¹zane z rynkiem
				data.orderBookData = kline[12].asString(); // Za³ó¿my, ¿e dane s¹ dostêpne w odpowiedzi API
				data.recentTradesData = kline[13].asString();
				data.currencyData = kline[14].asString();
				data.symbol24hrStats = kline[15].asString();
				data.marketStreamData = kline[16].asString();
				i++;
				this->output_queue.push(data);
			}
			catch (const std::exception& e)
			{
				std::cout << "E kline : root " << e.what() << std::endl;
			}
		};
		
		startTime = root[root.size() - 1][6].asInt64() + 1;	// Ustaw startTime na ostatni close time + 1

		if (root.size() < limit)
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Dodaj opóŸnienie, aby nie przekroczyæ limitu API Binance
	}
	return i;
}

std::string BinanceBot::get_binance_data(const std::string& endpoint)
{
	std::string url = "https://api.binance.com" + endpoint;
	return http_client.make_request(url);
}

Json::Value BinanceBot::get_ticker(const std::string& symbol)
{
	std::string endpoint = "/api/v3/ticker/price?symbol=" + symbol;
	std::string jsonData = get_binance_data(endpoint);

	Json::CharReaderBuilder builder;
	Json::Value root;
	std::string errors;

	std::istringstream dataStream(jsonData);
	if (!Json::parseFromStream(builder, dataStream, &root, &errors))
		std::cout << "Error parsing JSON data: " << errors << std::endl;

	return root;
}

std::vector<std::string> BinanceBot::get_all_symbols()
{
	std::string endpoint = "/api/v3/exchangeInfo";
	std::string jsonData = get_binance_data(endpoint);

	Json::CharReaderBuilder builder;
	Json::Value root;
	std::string errors;

	std::istringstream dataStream(jsonData);
	if (!Json::parseFromStream(builder, dataStream, &root, &errors))
		std::cout << "Error parsing JSON data: " << errors << std::endl;

	std::vector<std::string> symbols;
	const Json::Value& symbolsData = root["symbols"];
	for (const auto& symbol : symbolsData) {
		symbols.push_back(symbol["symbol"].asString());
	}

	return symbols;
}
