#include "BinanceBot.h"
#include "Sql.h"
#include "ProjectDataTypes.h"
#include <functional>
#include <utility>

std::queue<MarketData> marketDataQueue; //TODO: zrobić kolejkę wektorów z damnymi z API 
bool stopFlag{ false };
std::condition_variable queueCondVar;
unsigned int inQueue{ 0 };

static void dataProducer(const std::string	interval, const std::vector<std::pair<std::string, int64_t>> all_symbols)
{
	BinanceBot bot;

	for (const auto& symbol : all_symbols)
	{
		Json::Value ticker_data = bot.get_ticker(symbol.first);
		std::cout << std::endl << "***********************************"  << std::endl;
		std::cout << "Current price of " << symbol.first << ": " << ticker_data["price"].asString() << std::endl;

		std::cout << "Fetching historical data for: " << symbol.first << std::endl;
		std::cout << "Latest opening time: " << symbol.second << std::endl;
		std::cout << "Interval: " << interval << std::endl;

		inQueue = bot.get_historical_klines(symbol.first, interval, symbol.second, marketDataQueue);

	}
	std::cout << "Queue size:  " << marketDataQueue.size() << std::endl;
	queueCondVar.notify_one();
	stopFlag = true;
}

static void dataConsumer(MySQLConnector& connector)
{
	while (true)
	{
		MarketData data;
		{
			if (stopFlag && marketDataQueue.empty())
			{
				break;
			}
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondVar.wait(lock, [] { return !marketDataQueue.empty(); });

			if (marketDataQueue.empty())
			{
				continue;
			}

			// Pobierz dane z przodu kolejki
			data = marketDataQueue.front();
			marketDataQueue.pop();
		}

		// Wysyłanie danych do bazy danych 
		connector.sendDataToDatabase(data);
	}
}

int main()
{
	setlocale(LC_CTYPE, "Polish");
	const	std::string	interval{ "1m" };

	//zrobic wektor par: waluta -> najnowsza data otwarcia
	std::vector<std::pair<std::string, int64_t>> all_symbols;

	all_symbols.push_back(std::make_pair("ETHPLN", 0));
	all_symbols.push_back(std::make_pair("BTCPLN", 0));
	// all_symbols.push_back(std::make_pair("BTCUSDT", 0);
	// all_symbols.push_back("ETHUSDT", 0);
	// all_symbols.push_back("ETHBTC", 0);
	// all_symbols.push_back("BTCETH", 0);

	// Parametry do połączenia z bazą danych
	std::string host = "localhost";  // Adres serwera bazy danych
	int port = 33060;                 // Port MySQLx
	std::string user = "Admin";       // Nazwa użytkownika
	std::string password = "1234";  // Hasło użytkownika
	std::string database = "binance_data";    // Nazwa bazy danych

	// Tworzenie obiektu klasy MySQLConnector
	MySQLConnector dbConnector(host, port, user, password, database);

	for (auto& symbol : all_symbols)
	{
		dbConnector.addTable(symbol.first);
		symbol.second = dbConnector.fetchMaxOpenTime(symbol.first);
		std::cout << "Currency pair:  " << symbol.first << " Max Open time  " << symbol.second << std::endl;
	}

	std::cout << "Start Queue size:  " << marketDataQueue.size() << std::endl;

	int64_t startTime{ 0 };
	std::cout << "Start Time:  " << marketDataQueue.size() << std::endl;


	std::thread producerThread(dataProducer, interval, all_symbols);
	std::thread consumerThread(std::bind(dataConsumer, std::ref(dbConnector)));
	
	while (!stopFlag)
	{
		if (!marketDataQueue.empty())
		{
			queueCondVar.notify_one();
		}
	}

	std::cout << "End Queue size:  " << inQueue << std::endl;

	if (producerThread.joinable())
	{
		producerThread.join();
	}
	if (consumerThread.joinable())
	{
		consumerThread.join();
	}

	std::cout << "End2 Queue size:  " << inQueue << std::endl;
	std::cout << "End3 marketDataQueue size:  " << marketDataQueue.size() << std::endl;
	return 0;
}

