#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <mysqlx\xdevapi.h>

class MySQLConnector
{
private:
	std::unique_ptr<mysqlx::Session> session;  // U¿yj inteligentnego wskaŸnika dla obiektu sesji

public:
	// Konstruktor inicjalizuj¹cy po³¹czenie z MySQL
	MySQLConnector(const std::string& host, const int& port, const std::string& user, const std::string& password, const std::string& database)
	{
		try
		{
			// Ustanów sesjê z serwerem MySQL  Session from_options("host", port, "user", "pwd", "db");
			session = std::make_unique<mysqlx::Session>(host, port, user, password, database);
			std::cout << "Pomyœlnie po³¹czono z baz¹ danych." << std::endl;
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

	// Metoda wykonuj¹ca prost¹ kwerendê (tylko dla demonstracji)
	bool executeQuery(const std::string& query)
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
};
