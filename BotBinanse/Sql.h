#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <mysqlx\xdevapi.h>

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

std::mutex queueMutex;


class MySQLConnector
{
private:
	std::unique_ptr<mysqlx::Session> session;  // U�yj inteligentnego wska�nika dla obiektu sesji
	const std::string dataBase;

public:
	// Konstruktor inicjalizuj�cy po��czenie z MySQL
	MySQLConnector(const std::string& host, const int& port, const std::string& user, const std::string& password, const std::string& database)
		:dataBase(database)
	{
		try
		{
			// Ustan�w sesj� z serwerem MySQL  Session from_options("host", port, "user", "pwd", "db");
			session = std::make_unique<mysqlx::Session>(host, port, user, password, dataBase);
			std::cout << "Pomy�lnie po��czono z baz� danych." << std::endl;

			executeQuery("CREATE DATABASE IF NOT EXISTS " + dataBase + "; ");
			executeQuery("USE " + dataBase + "; ");


			//std::string	query = "CREATE TABLE IF NOT EXISTS `" + symbol + "` ("
			//	"id INT AUTO_INCREMENT PRIMARY KEY, "
			//	"open_time BIGINT NOT NULL, "
			//	"open_price DECIMAL(20,10) NOT NULL, "
			//	"high_price DECIMAL(20,10) NOT NULL, "
			//	"low_price DECIMAL(20,10) NOT NULL, "
			//	"close_price DECIMAL(20,10) NOT NULL, "
			//	"volume DECIMAL(20,10) NOT NULL, "
			//	"close_time BIGINT NOT NULL, "
			//	"quote_asset_volume DECIMAL(20,10) NOT NULL, "
			//	"number_of_trades INT NOT NULL, "
			//	"taker_buy_base_asset_volume DECIMAL(20,10) NOT NULL, "
			//	"taker_buy_quote_asset_volume DECIMAL(20,10) NOT NULL, "
			//	"ignore_flag VARCHAR(255), "
			//	"order_book_data LONGTEXT, "
			//	"recent_trades_data LONGTEXT, "
			//	"currency_data LONGTEXT, "
			//	"symbol_24hr_stats LONGTEXT, "
			//	"market_stream_data LONGTEXT"
			//	");";
			//executeQuery(query);


		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "B��d po��czenia z baz� danych: " << err.what() << std::endl;
			exit(1);  // Wyjd�, je�li nie mo�na si� po��czy�
		}
		catch (std::exception& ex)
		{
			std::cerr << "Standardowy wyj�tek: " << ex.what() << std::endl;
			exit(1);
		}
	}

	// Destruktor zamykaj�cy po��czenie
	~MySQLConnector()
	{
		if (session)
		{
			session->close();  // Zamknij sesj�
			std::cout << "Po��czenie z baz� danych zamkni�te." << std::endl;
		}
	}

	bool addTable(std::string table)
	{
		try
		{
			std::string	query = "CREATE TABLE IF NOT EXISTS `" + table + "` ("
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

			mysqlx::SqlStatement stmt = session->sql(query);
			mysqlx::SqlResult result = stmt.execute();
			return true;
		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "B��d wykonania zapytania: " << err.what() << std::endl;
			return false;  // Zwr�� false, je�li wyst�pi� b��d
		}
	}

	// Metoda wykonuj�ca prost� kwerend� (tylko dla demonstracji)
	bool executeQuery(const std::string& query)
	{
		try
		{
			mysqlx::SqlStatement stmt = session->sql(query);
			mysqlx::SqlResult result = stmt.execute();
			return true;  // Zwr�� true, je�li zapytanie wykonane bez b��du

		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "B��d wykonania zapytania: " << err.what() << std::endl;
			return false;  // Zwr�� false, je�li wyst�pi� b��d
		}
	}

	void sendDataToDatabase(const MarketData& data)
	{
		try
		{
			//TODO: wysy�a� ca�e paczki do bazy 
			mysqlx::Table table = session->getDefaultSchema().getTable(data.symbol);
			table.insert("open_time", "open_price", "high_price", "low_price", "close_price",
				"volume", "close_time", "quote_asset_volume", "number_of_trades",
				"taker_buy_base_asset_volume", "taker_buy_quote_asset_volume",
				"ignore_flag", "order_book_data", "recent_trades_data", "currency_data",
				"symbol_24hr_stats", "market_stream_data")
				.values(data.openTime, data.open, data.high, data.low, data.close,
					data.volume, data.closeTime, data.quoteAssetVolume, data.numberOfTrades,
					data.takerBuyBaseAssetVolume, data.takerBuyQuoteAssetVolume,
					data.ignore, data.orderBookData, data.recentTradesData,
					data.currencyData, data.symbol24hrStats, data.marketStreamData)
				.execute();
		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "Error inserting data: " << err.what() << std::endl;
		}
		catch (std::exception& ex)
		{
			std::cerr << "Exception occurred: " << ex.what() << std::endl;
		}
	}


	int64_t fetchMaxOpenTime(const std::string& tableName)
	{
		try
		{
			// Pobieranie schematu bazy danych
			mysqlx::Schema db = session->getSchema(dataBase);
			// Pobieranie tabeli
			mysqlx::Table table = db.getTable(tableName);

			// Wykonanie zapytania
			mysqlx::RowResult result = table.select("MAX(open_time) AS max_open_time").execute();

			// Pobranie pierwszego wiersza wynik�w
			mysqlx::Row row = result.fetchOne();

			// Zwr�cenie wyniku
			if (!row.isNull())
			{
				return row[0].get<int64_t>();
			}
			else
			{
				std::cout << "No data found in " << tableName << std::endl;
				return 0;  // Zwraca 0, gdy brak danych
			}
		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "Database Error: " << err.what() << std::endl;
			return 0;  // Zwraca 0 w przypadku b��du bazy danych
		}
		catch (const std::exception& ex)
		{
			std::cerr << "STD Exception: " << ex.what() << std::endl;
			return 0;  // Zwraca 0 w przypadku og�lnego wyj�tku
		}
		catch (...)
		{
			std::cerr << "Unknown exception occurred" << std::endl;
			return 0;  // Zwraca 0 w przypadku nieznanych wyj�tk�w
		}
	}



};


//std::string query = "CREATE DATABASE IF NOT EXISTS " + database + "; ";
//dbConnector.executeQuery(query);
////std::vector<std::string> all_symbols = bot.get_all_symbols();
//std::vector<std::string> all_symbols;
//all_symbols.push_back("ETHPLN");
//// all_symbols.push_back("BTCPLN");
//// all_symbols.push_back("BTCUSDT");
//// all_symbols.push_back("ETHUSDT");
//// all_symbols.push_back("ETHBTC");
//// all_symbols.push_back("BTCETH");

//for (const auto& symbol : all_symbols)
//{
//	query = "CREATE TABLE IF NOT EXISTS `" + symbol + "` ("
//		"id INT AUTO_INCREMENT PRIMARY KEY, "
//		"open_time BIGINT NOT NULL, "
//		"open_price DECIMAL(20,10) NOT NULL, "
//		"high_price DECIMAL(20,10) NOT NULL, "
//		"low_price DECIMAL(20,10) NOT NULL, "
//		"close_price DECIMAL(20,10) NOT NULL, "
//		"volume DECIMAL(20,10) NOT NULL, "
//		"close_time BIGINT NOT NULL, "
//		"quote_asset_volume DECIMAL(20,10) NOT NULL, "
//		"number_of_trades INT NOT NULL, "
//		"taker_buy_base_asset_volume DECIMAL(20,10) NOT NULL, "
//		"taker_buy_quote_asset_volume DECIMAL(20,10) NOT NULL, "
//		"ignore_flag VARCHAR(255), "
//		"order_book_data LONGTEXT, "
//		"recent_trades_data LONGTEXT, "
//		"currency_data LONGTEXT, "
//		"symbol_24hr_stats LONGTEXT, "
//		"market_stream_data LONGTEXT"
//		");";
//	dbConnector.executeQuery(query);
//}