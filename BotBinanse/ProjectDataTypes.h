#pragma once
#include <string>
#include <queue>

struct MarketData
{
    std::string symbol;
    long long openTime;                  // Czas otwarcia œwiecy, w formacie UNIX timestamp
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
};

std::queue<MarketData> marketDataQueue;