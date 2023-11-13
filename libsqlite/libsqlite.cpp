// libsqlite.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "libsqlite.h"
#include <cstdio>



void execute(Connection const& connection)
{

	sqlite3_stmt* query = nullptr;
	int result = sqlite3_prepare_v2(connection.getAbi(), "hello World!", -1, &query, nullptr);


}
int main()
{

	try
	{
		 
		Connection connection = Connection::Memory();
		execute(connection, "create table Users (Name)");

		execute(connection, "insert into  Users values (?)", "nithin");
		execute(connection, "insert into  Users values (?)", "ginto");
		
		for(Row const & row : Statement(connection, "select * from Users"))
		{
			printf(" %s  \n", row.getString());
		}

	}
	catch (const Exception const& e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}

}


