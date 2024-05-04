#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <mysqlx\xdevapi.h>

class MySQLConnector
{
private:
	std::unique_ptr<mysqlx::Session> session;  // U�yj inteligentnego wska�nika dla obiektu sesji

public:
	// Konstruktor inicjalizuj�cy po��czenie z MySQL
	MySQLConnector(const std::string& host, const int& port, const std::string& user, const std::string& password, const std::string& database)
	{
		try
		{
			// Ustan�w sesj� z serwerem MySQL  Session from_options("host", port, "user", "pwd", "db");
			session = std::make_unique<mysqlx::Session>(host, port, user, password, database);
			std::cout << "Pomy�lnie po��czono z baz� danych." << std::endl;
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
};
