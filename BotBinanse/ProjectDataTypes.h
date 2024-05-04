#pragma once
#include <string>
#include <queue>

struct MarketData
{
    std::string symbol;
    long long openTime;                  // Czas otwarcia �wiecy, w formacie UNIX timestamp
    double open;                         // Cena otwarcia
    double high;                         // Najwy�sza cena
    double low;                          // Najni�sza cena
    double close;                        // Cena zamkni�cia
    double volume;                       // Obj�to�� handlu
    long long closeTime;                 // Czas zamkni�cia �wiecy, w formacie UNIX timestamp
    double quoteAssetVolume;             // Ca�kowita warto�� obrotu walut� kwotowan�
    int numberOfTrades;                  // Liczba transakcji
    double takerBuyBaseAssetVolume;      // Obj�to�� zakupu przez taker�w dla bazowej waluty
    double takerBuyQuoteAssetVolume;     // Obj�to�� zakupu przez taker�w dla waluty kwotowanej
    std::string ignore;                  // Dodatkowe informacje

    // Dodatkowe dane o rynku
    std::string orderBookData;           // Dane z ksi�gi zlece�, zwykle jako JSON lub inny z�o�ony format
    std::string recentTradesData;        // Dane o ostatnich transakcjach
    std::string currencyData;            // Informacje o dost�pnych walutach i parach walutowych
    std::string symbol24hrStats;         // Statystyki 24-godzinne dla symbolu
    std::string marketStreamData;        // Dane strumieniowe rynku
};

std::queue<MarketData> marketDataQueue;