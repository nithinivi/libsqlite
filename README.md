# libsqlite


The lib can be used in the following manner check the libsqlite.cpp file
```cpp

	Connection connection = Connection::Memory();
	execute(connection, "create table Users (Name)");
	
	execute(connection, "insert into  Users values (?)", "nithin");
	execute(connection, "insert into  Users values (?)", "ginto");
	
	for(Row const & row : Statement(connection, "select * from Users"))
	{
		printf(" %s  \n", row.getString());
	}

```
