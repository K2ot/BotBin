#include "BinanceBot.h"
#include "Sql.h"
#include "ProjectDataTypes.h"
#include <functional>

std::queue<MarketData> marketDataQueue;
bool stopFlag{ false };
std::condition_variable queueCondVar;
unsigned int inQueue{ 0 };

static void dataProducer(const int64_t& startTime )
{

	BinanceBot bot;
	const	std::string	interval{ "5m" };

	for (const auto& symbol : all_symbols)
	{
		Json::Value ticker_data = bot.get_ticker(symbol);
		std::cout << "Current price of " << symbol << ": " << ticker_data["price"].asString() << std::endl;

		std::cout << "Fetching historical data for " << symbol << std::endl;
		inQueue = bot.get_historical_klines(symbol, interval, startTime ,marketDataQueue);

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
	const	std::string	interval{ "5m" };

	std::vector<std::string> all_symbols;
	all_symbols.push_back("ETHPLN");
	// all_symbols.push_back("BTCPLN");
	// all_symbols.push_back("BTCUSDT");
	// all_symbols.push_back("ETHUSDT");
	// all_symbols.push_back("ETHBTC");
	// all_symbols.push_back("BTCETH");
	 
	// TODO: 1 zrobic wektor par: waluta-> najnowsza data otwarcia
	// TODO: 2 przekazać ją do dataProducer
	// TODO: 3 przekazać interwał

	
	// Parametry do połączenia z bazą danych
	std::string host = "localhost";  // Adres serwera bazy danych
	int port = 33060;                 // Port MySQLx
	std::string user = "Admin";       // Nazwa użytkownika
	std::string password = "1234";  // Hasło użytkownika
	std::string database = "binance_data";    // Nazwa bazy danych

	// Tworzenie obiektu klasy MySQLConnector
	MySQLConnector dbConnector(host, port, user, password, database);


	std::cout << "Start Queue size:  " << marketDataQueue.size() << std::endl;

	int64_t startTime{ 0 };
	std::cout << "Start Time:  " << marketDataQueue.size() << std::endl;
	std::thread producerThread(dataProducer, std::ref(startTime));
	std::thread consumerThread(dataConsumer, std::ref(dbConnector));



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
	std::cout << "End3 Queue size:  " << marketDataQueue.size() << std::endl;
	return 0;
}

