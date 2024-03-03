#pragma once

#include <string>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <json/json.h>
#include "HttpClient.h"
#include <thread>

class BinanceBot
{
	HttpClient http_client;
	std::string get_binance_data(const std::string& endpoint);

public:

	BinanceBot() {};
	~BinanceBot() {};
	Json::Value get_ticker(const std::string& symbol);
	std::vector<std::string> get_all_symbols();
	void get_historical_klines(const std::string& symbol, const std::string& interval, const std::string& output_file);

};

void BinanceBot::get_historical_klines(const std::string& symbol, const std::string& interval, const std::string& output_file) {
	const int limit = 1000; // Maksymalna liczba wynik�w na ��danie
	int64_t startTime = 0; // Ustaw startTime na 0, aby pobra� dane od pocz�tku istnienia waluty
	std::vector<std::vector<double>> klines;

	while (true) {
		std::string endpoint = "/api/v3/klines?symbol=" + symbol + "&interval=" + interval + "&startTime=" + std::to_string(startTime) + "&limit=" + std::to_string(limit);
		std::string jsonData = get_binance_data(endpoint);

		Json::CharReaderBuilder builder;
		Json::Value root;
		std::string errors;

		std::istringstream dataStream(jsonData);
		if (!Json::parseFromStream(builder, dataStream, &root, &errors)) {
			std::cout << "Error parsing JSON data: " << errors << std::endl;
			break;
		}

		if (root.empty()) {
			break;
		}

		for (const auto& kline : root) {
			std::vector<double> kline_data = {
				static_cast<double>(kline[0].asInt64()),         // Open time: Czas otwarcia �wieczki (w milisekundach od 1970-01-01 UTC).
				std::stod(kline[1].asString()),					 // Open: Cena otwarcia �wieczki (na pocz�tku interwa�u).
				std::stod(kline[2].asString()),					 // High: Najwy�sza cena osi�gni�ta w trakcie interwa�u �wieczki.
				std::stod(kline[3].asString()),					// Low: Najni�sza cena osi�gni�ta w trakcie interwa�u �wieczki.
				std::stod(kline[4].asString()), // Close: Cena zamkni�cia �wieczki (na ko�cu interwa�u).
				std::stod(kline[5].asString()), // Volume: Wolumen handlu dla okresu �wieczki.
				static_cast<double>(kline[6].asInt64()),        // Close time: Czas zamkni�cia �wieczki (w milisekundach od 1970-01-01 UTC).
				std::stod(kline[7].asString()), // Quote asset volume: Warto�� wolumenu handlu wyra�ona w walucie kwotowania (np. je�li handlujesz par� BTC/USDT, to jest to warto�� w USDT).
				static_cast<double>(kline[8].asInt()), // Number of trades: Liczba transakcji zrealizowanych w trakcie interwa�u �wieczki.
				std::stod(kline[9].asString()), // Taker buy base asset volume: Wolumen zakupu aktywa bazowego przez taker�w (osoby akceptuj�ce oferty) w trakcie interwa�u �wieczki.
				std::stod(kline[10].asString()), // Taker buy quote asset volume: Warto�� wolumenu zakupu aktywa bazowego przez taker�w wyra�ona w walucie kwotowania w trakcie interwa�u �wieczki.
				std::stod(kline[11].asString())  // Ignore: Dodatkowy parametr, kt�ry mo�na zignorowa�.
			};
			klines.push_back(kline_data);
		}
		startTime = klines.back()[6] + 1; // Ustaw startTime na ostatni close time + 1

		if (root.size() < limit) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Dodaj op�nienie, aby nie przekroczy� limitu API Binance
	}

	// Zapisz dane do pliku CSV
	std::ofstream csv_file(output_file);
	csv_file << "Open time,Open,High,Low,Close,Volume,Close time,Quote asset volume,Number of trades,Taker buy base asset volume,Taker buy quote asset volume,Ignore" << std::endl;

	for (const auto& kline : klines) {
		for (size_t i = 0; i < kline.size(); ++i) {
			csv_file << kline[i];
			if (i < kline.size() - 1) {
				csv_file << ",";
			}
		}
		csv_file << std::endl;
	}
	csv_file.close();
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
