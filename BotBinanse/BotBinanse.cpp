#include "BinanceBot.h"
#include "Sql.h"
#include "ProjectDataTypes.h"

std::queue<MarketData> marketDataQueue;
bool stopFlag{ false };
std::condition_variable queueCondVar;

void dataProducer()
{
	std::vector<std::string> all_symbols;
	all_symbols.push_back("ETHPLN");
	// all_symbols.push_back("BTCPLN");
	// all_symbols.push_back("BTCUSDT");
	// all_symbols.push_back("ETHUSDT");
	// all_symbols.push_back("ETHBTC");
	// all_symbols.push_back("BTCETH");

	BinanceBot bot;
	const	std::string	interval{ "12h" };

	for (const auto& symbol : all_symbols)
	{
		Json::Value ticker_data = bot.get_ticker(symbol);
		std::cout << "Current price of " << symbol << ": " << ticker_data["price"].asString() << std::endl;

		std::cout << "Fetching historical data for " << symbol << std::endl;
		bot.get_historical_klines(symbol, interval, marketDataQueue);

	}
	std::cout << "Queue size:  " << marketDataQueue.size() << std::endl;
	queueCondVar.notify_one();
	stopFlag = true;
}

void dataConsumer()
{
	// Parametry do połączenia z bazą danych
	std::string host = "localhost";  // Adres serwera bazy danych
	int port = 33060;                 // Port MySQL
	std::string user = "Admin";       // Nazwa użytkownika
	std::string password = "1234";  // Hasło użytkownika
	std::string database = "binance_data";    // Nazwa bazy danych

	// Tworzenie obiektu klasy MySQLConnector
	MySQLConnector dbConnector(host, port, user, password, database);

	std::unique_lock<std::mutex> lock(queueMutex);
	while (true)
	{
		queueCondVar.wait(lock, [] { return !marketDataQueue.empty() || stopFlag; });

		if (marketDataQueue.empty())
		{
			break;
		}

		// Pobierz dane z przodu kolejki
		MarketData data = marketDataQueue.front();
		marketDataQueue.pop();

		// Odblokuj mutex podczas wykonywania operacji na bazie danych
		lock.unlock();

		// Wysyłanie danych do bazy danych (funkcja symulująca)
		dbConnector.sendDataToDatabase(data);

		// Ponownie zablokuj mutex na kolejne operacje
		lock.lock();
	}

}


int main()
{
	setlocale(LC_CTYPE, "Polish");
	std::cout << "Start Queue size:  " << marketDataQueue.size() << std::endl;
	while (!marketDataQueue.empty())
	{
		marketDataQueue.pop();
	}

	std::thread producerThread(dataProducer);
	std::thread consumerThread(dataConsumer);
	while (!stopFlag)
	{
		if (!marketDataQueue.empty())
		{
			queueCondVar.notify_one();
		}
	}

	if (producerThread.joinable())
	{
		producerThread.join();
	}
	if (consumerThread.joinable())
	{
		consumerThread.join();
	}
	return 0;
}

