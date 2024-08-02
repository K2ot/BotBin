#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <mysqlx\xdevapi.h>

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include "ProjectDataTypes.h"
#include <fstream>

std::mutex queueMutex;


class MySQLConnector
{
	std::unique_ptr<mysqlx::Session> session;  // U¿yj inteligentnego wskaŸnika dla obiektu sesji
	const std::string dataBase;

	bool createQuery(const std::string& query)
	{
		try
		{
			mysqlx::SqlStatement stmt = session->sql(query);
			mysqlx::SqlResult result = stmt.execute();
			return true;  // Zwróæ true, jeœli zapytanie wykonane bez b³êdu

		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "B³¹d wykonania zapytania: " << err.what() << std::endl;
			return false;  // Zwróæ false, jeœli wyst¹pi³ b³¹d
		}
	}

public:
	// Konstruktor inicjalizuj¹cy po³¹czenie z MySQL
	MySQLConnector(const std::string& host, const int& port, const std::string& user, const std::string& password, const std::string& database)
		:dataBase(database)
	{
		try
		{
			// Ustanów sesjê z serwerem MySQL  Session from_options("host", port, "user", "pwd", "db");
			session = std::make_unique<mysqlx::Session>(host, port, user, password, dataBase);
			std::cout << "Pomyœlnie po³¹czono z baz¹ danych." << std::endl;

			createQuery("CREATE DATABASE IF NOT EXISTS " + dataBase + "; ");
			createQuery("USE " + dataBase + "; ");


		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "B³¹d po³¹czenia z baz¹ danych: " << err.what() << std::endl;
			exit(1);  // WyjdŸ, jeœli nie mo¿na siê po³¹czyæ
		}
		catch (std::exception& ex)
		{
			std::cerr << "Standardowy wyj¹tek: " << ex.what() << std::endl;
			exit(1);
		}
	}

	// Destruktor zamykaj¹cy po³¹czenie
	~MySQLConnector()
	{
		if (session)
		{
			session->close();  // Zamknij sesjê
			std::cout << "Po³¹czenie z baz¹ danych zamkniête." << std::endl;
		}
	}

	bool addTable(std::string table)
	{
		try
		{
			std::string	query = "CREATE TABLE IF NOT EXISTS `" + table + "` ("
				"open_time BIGINT NOT NULL PRIMARY KEY "
				", openTimeStamp VARCHAR(255) "
				", open_price DECIMAL(20,6) NOT NULL "
				", high_price DECIMAL(20,6) NOT NULL "
				", low_price DECIMAL(20,6) NOT NULL "
				", close_price DECIMAL(20,6) NOT NULL "
				", volume DECIMAL(20,6) NOT NULL "
				", close_time BIGINT NOT NULL "
				", closeTimeStamp VARCHAR(255) "
				", quote_asset_volume DECIMAL(20,6) NOT NULL "
				", number_of_trades INT NOT NULL "
				", taker_buy_base_asset_volume DECIMAL(20,6) NOT NULL "
				", taker_buy_quote_asset_volume DECIMAL(20,6) NOT NULL "
				");";

			mysqlx::SqlStatement stmt = session->sql(query);
			mysqlx::SqlResult result = stmt.execute();
			return true;
		}
		catch (const mysqlx::Error& err)
		{
			std::cerr << "B³¹d wykonania zapytania: " << err.what() << std::endl;
			return false;  // Zwróæ false, jeœli wyst¹pi³ b³¹d
		}
	}

	void dataToCSV(const std::string& tableName, const int64_t& randomMiesiac, std::ofstream& csvFile)
	{
		if (!csvFile.is_open())
		{
			std::cerr << "Nie mo¿na otworzyæ pliku CSV " << std::endl;
			return;
		}
		try
		{
			csvFile << "opentimeStamp,open_price,high_price,low_price,close_price,volume,closetimeStamp,quote_asset_volume,number_of_trades,taker_buy_base_asset_volume,taker_buy_quote_asset_volume\n";

			// Pobieranie schematu bazy danych
			mysqlx::Schema db = session->getSchema(dataBase);
			// Pobieranie tabeli
			mysqlx::Table table = db.getTable(tableName);
			mysqlx::RowResult result = table.select("*")
				.where("open_time >= :open_time")
				.orderBy("open_time")
				.limit(3000)
				.bind("open_time", randomMiesiac * 1000)
				.execute();

			for (mysqlx::Row row : result)
			{
				csvFile
					//<< row[0].get<int64_t>() // open_time
					       << row[1].get<std::string>()// timeStamp
					<< "," << row[2].get<double>()// open_price
					<< "," << row[3].get<double>()// high_price
					<< "," << row[4].get<double>() // low_price
					<< "," << row[5].get<double>()  // close_price
					<< "," << row[6].get<double>()   // volume
					//<< "," << row[7].get<int64_t>()  // close_time
					<< "," << row[8].get<std::string>()   //close timeStamp
					<< "," << row[9].get<double>()   // quote_asset_volume
					<< "," << row[10].get<int>()  // number_of_trades
					<< "," << row[11].get<double>()  // taker_buy_base_asset_volume
					<< "," << row[12].get<double>()  // taker_buy_quote_asset_volume
					<< "\n";
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Exception occurred: " << e.what() << std::endl;
		}
	}

	void sendDataToDatabase(const std::vector<MarketData>& dataBatch)
	{
		try
		{
			// U¿yjemy pierwszego symbolu dla przyk³adu, zak³adaj¹c, ¿e wszystkie dane maj¹ ten sam symbol
			mysqlx::Table table = session->getDefaultSchema().getTable(dataBatch.front().symbol);

			// Rozpocznij konstruowanie zapytania
			mysqlx::TableInsert insert = table.insert(
				"open_time", "opentimeStamp", "open_price", "high_price", "low_price", "close_price",
				"volume", "close_time", "closeTimeStamp", "quote_asset_volume", "number_of_trades",
				"taker_buy_base_asset_volume", "taker_buy_quote_asset_volume");

			// Dodaj wszystkie wiersze do zapytania
			for (const auto& data : dataBatch)
			{
				insert.values(
					data.openTime, data.openTimeStamp, data.open, data.high, data.low, data.close,
					data.volume,data.closeTime, data.closeTimeStamp, data.quoteAssetVolume, data.numberOfTrades,
					data.takerBuyBaseAssetVolume, data.takerBuyQuoteAssetVolume
				);
			}

			// Wykonaj zapytanie z wszystkimi wartoœciami

			insert.execute();
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

			// Pobranie pierwszego wiersza wyników
			mysqlx::Row row = result.fetchOne();

			// Zwrócenie wyniku
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
			return 0;  // Zwraca 0 w przypadku b³êdu bazy danych
		}
		catch (const std::exception& ex)
		{
			std::cerr << "STD Exception: " << ex.what() << std::endl;
			return 0;  // Zwraca 0 w przypadku ogólnego wyj¹tku
		}
		catch (...)
		{
			std::cerr << "Unknown exception occurred" << std::endl;
			return 0;  // Zwraca 0 w przypadku nieznanych wyj¹tków
		}
	}

	int64_t fetchMinOpenTime(const std::string& tableName)
	{
		try
		{
			// Pobieranie schematu bazy danych
			mysqlx::Schema db = session->getSchema(dataBase);
			// Pobieranie tabeli
			mysqlx::Table table = db.getTable(tableName);

			// Wykonanie zapytania
			mysqlx::RowResult result = table.select("MIN(open_time) AS max_open_time").execute();

			// Pobranie pierwszego wiersza wyników
			mysqlx::Row row = result.fetchOne();

			// Zwrócenie wyniku
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
			return 0;  // Zwraca 0 w przypadku b³êdu bazy danych
		}
		catch (const std::exception& ex)
		{
			std::cerr << "STD Exception: " << ex.what() << std::endl;
			return 0;  // Zwraca 0 w przypadku ogólnego wyj¹tku
		}
		catch (...)
		{
			std::cerr << "Unknown exception occurred" << std::endl;
			return 0;  // Zwraca 0 w przypadku nieznanych wyj¹tków
		}
	}

};