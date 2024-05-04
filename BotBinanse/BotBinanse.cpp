#include "BinanceBot.h"
#include "Sql.h"
#include "ProjectDataTypes.h"

int main()
{
	setlocale(LC_CTYPE, "Polish");
	std::queue<MarketData> marketDataQueue;
	// Parametry do połączenia z bazą danych
	std::string host = "localhost";  // Adres serwera bazy danych
	int port = 33060;                 // Port MySQL
	std::string user = "Admin";       // Nazwa użytkownika
	std::string password = "1234";  // Hasło użytkownika
	std::string database = "sakila";    // Nazwa bazy danych

	// Tworzenie obiektu klasy MySQLConnector
	MySQLConnector dbConnector(host, port, user, password, database);


	std::string query = "CREATE DATABASE IF NOT EXISTS binance_data;";
	dbConnector.executeQuery(query);

	query = "USE binance_data;";
	dbConnector.executeQuery(query);



	dbConnector.executeQuery(query);

	BinanceBot bot;
	//std::vector<std::string> all_symbols = bot.get_all_symbols();
	std::vector<std::string> all_symbols;
	all_symbols.push_back("ETHPLN");
	// all_symbols.push_back("BTCPLN");
	// all_symbols.push_back("BTCUSDT");
	// all_symbols.push_back("ETHUSDT");
	// all_symbols.push_back("ETHBTC");
	// all_symbols.push_back("BTCETH");

	for (const auto& symbol : all_symbols)
	{
		query = "CREATE TABLE IF NOT EXISTS `" + symbol + "` ("
			"id INT AUTO_INCREMENT PRIMARY KEY, "
			"open_time BIGINT NOT NULL, "
			"open_price DECIMAL(20,10) NOT NULL, "
			"high_price DECIMAL(20,10) NOT NULL, "
			"low_price DECIMAL(20,10) NOT NULL, "
			"close_price DECIMAL(20,10) NOT NULL, "
			"volume DECIMAL(20,10) NOT NULL, "
			"close_time BIGINT NOT NULL, "
			"quote_asset_volume DECIMAL(20,10) NOT NULL, "
			"number_of_trades INT NOT NULL, "
			"taker_buy_base_asset_volume DECIMAL(20,10) NOT NULL, "
			"taker_buy_quote_asset_volume DECIMAL(20,10) NOT NULL, "
			"ignore_flag VARCHAR(255), "
			"order_book_data LONGTEXT, "
			"recent_trades_data LONGTEXT, "
			"currency_data LONGTEXT, "
			"symbol_24hr_stats LONGTEXT, "
			"market_stream_data LONGTEXT"
			");";
		dbConnector.executeQuery(query);
	}

	for (const auto& symbol : all_symbols)
	{
		Json::Value ticker_data = bot.get_ticker(symbol);
		std::cout << "Current price of " << symbol << ": " << ticker_data["price"].asString() << std::endl;

		std::cout << "Fetching historical data for " << symbol << std::endl;
		bot.get_historical_klines(symbol, "5m", marketDataQueue);
	}

	return 0;
}
