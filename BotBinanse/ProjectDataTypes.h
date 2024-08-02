#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

struct MarketData
{
	std::string symbol;
	long long openTime;                  // Czas otwarcia œwiecy, w formacie UNIX timestamp
	std::string openTimeStamp;			 // Czas otwarcia œwiecy, w formacie UNIX timestamp -> RRRR/MM/DD_T_GG/MM/SS
	std::string closeTimeStamp;			 // Czas zamkniêcia œwiecy, w formacie UNIX timestamp -> RRRR/MM/DD_T_GG/MM/SS
	double open;                         // Cena otwarcia
	double high;                         // Najwy¿sza cena
	double low;                          // Najni¿sza cena
	double close;                        // Cena zamkniêcia
	double volume;                       // Objêtoœæ handlu
	long long closeTime;                 // Czas zamkniêcia œwiecy, w formacie UNIX timestamp
	double quoteAssetVolume;             // Ca³kowita wartoœæ obrotu walut¹ kwotowan¹
	int numberOfTrades;                  // Liczba transakcji
	double takerBuyBaseAssetVolume;      // Objêtoœæ zakupu przez takerów dla bazowej waluty
	double takerBuyQuoteAssetVolume;     // Objêtoœæ zakupu przez takerów dla waluty kwotowanej
	std::string ignore;                  // Dodatkowe informacje

	// Dodatkowe dane o rynku
	std::string orderBookData;           // Dane z ksiêgi zleceñ, zwykle jako JSON lub inny z³o¿ony format
	std::string recentTradesData;        // Dane o ostatnich transakcjach
	std::string currencyData;            // Informacje o dostêpnych walutach i parach walutowych
	std::string symbol24hrStats;         // Statystyki 24-godzinne dla symbolu
	std::string marketStreamData;        // Dane strumieniowe rynku

	bool isSymbol()
	{
		if (symbol.empty())
		{
			std::cerr << "No Symbol" << std::endl;
			return false;
		}
		return true;
	}



 static	std::string convertEpochToDateTime(long long Time)
	{
		// Konwersja milisekund na sekundy
		std::chrono::seconds sec(Time / 1000);

		// Utworzenie punktu czasowego na podstawie sekund
		std::chrono::time_point<std::chrono::system_clock> tp(sec);

		// Pobranie czasu jako tm (struktura C)
		std::time_t tt = std::chrono::system_clock::to_time_t(tp);
		std::tm dt; // Struktura czasu

		// U¿ywanie localtime_s zamiast localtime
		localtime_s(&dt, &tt);

		// Formatowanie daty i czasu
		std::stringstream ss;
		ss << std::put_time(&dt, "%Y/%m/%dT%T"); // Uwaga na dodanie adresu struktury dt

		return ss.str();
	}

	void roundMarketData()
	{
		open = roundDecimal(open);
		high = roundDecimal(high);
		low = roundDecimal(low);
		close = roundDecimal(close);
		volume = roundDecimal(volume);
		quoteAssetVolume = roundDecimal(quoteAssetVolume);
		takerBuyBaseAssetVolume = roundDecimal(takerBuyBaseAssetVolume);
		takerBuyQuoteAssetVolume = roundDecimal(takerBuyQuoteAssetVolume);
	}

private:
	double roundDecimal(double value)
	{
		const int precision{ 6 };
		std::stringstream ss;
		ss << std::fixed << std::setprecision(precision) << value;
		ss >> value;
		return value;
	}
};