// libsqlite.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "libsqlite.hpp"
#include <cstdio>



static char const* typeName(Type const type)
{
	switch (type)
	{
	case Type::Integer: return "Integers";
	case Type::Float: return "Float";
	case Type::Blob: return "Blob";
	case Type::Null: return "Null";
	case Type::Text: return "Text";
	default:
		ASSERT(false);
		return "invalid";
	}
}



void insert_example()
{
	Connection connection = Connection::Memory();
	execute(connection, "create table Users (Name)");

	execute(connection, "insert into  Users values (?)", "nithin");
	printf("Inserted %lld\n", connection.rowId());

	execute(connection, "insert into  Users values (?)", "ginto");
	printf("Inserted %lld\n", connection.rowId());

	execute(connection, "insert into  Users values (?)", 123);

	for (Row const& row : Statement(connection, "select RowId, Name from Users"))
	{
		printf("[%d] %s  - %s \n", row.getInt(0), row.getString(1), typeName(row.getType(1)));
	}
}



void trasaction()
{

	Connection connection = Connection::Memory();
	execute(connection, "create table Things (Content)");
	Statement insert(connection, "insert into Things (Content) values (?1)");

	execute(connection, "begin");
	for (int i = 0; i < 1000; i++)
	{
		insert.reset(i);
		insert.execute();
	}

	execute(connection, "commit");

	Statement count(connection, "select count(*) from Things");
	count.step();
	printf("count: %d\n", count.getInt());


}

void profile_handling()
{
	Connection connection = Connection::Memory();
	connection.profile(
		[](void*, char const* const statement, unsigned long long const time) {
			unsigned long long const ms = time / 1000'000;
			if (ms > 10)
			{
				printf("profiler - (%lld) %s\n", ms, statement);
			}
		}
	);
}


static void saveToDisk(Connection const & source, char const * const filename)
{
	Connection destination(filename);
	Backup backup(destination, source);
	backup.step();
}

void backup_handling()
{
	Connection connection = Connection::Memory();
	execute(connection, "create table things (content real)");
	Statement statement(connection, "insert into things values (?)");
	for (int i = 0; i < 10000; i++)
	{
		statement.reset(i);
		statement.execute();
	}

	saveToDisk(connection, "C:\\temp\\backup.db");


}



int main()
{


	try
	{
		backup_handling();
	}
	catch (const Exception const& e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}

	

}


