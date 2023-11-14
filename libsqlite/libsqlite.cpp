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


int main()
{


	try
	{
		trasaction();
	}
	catch (const Exception const& e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}

	

}


