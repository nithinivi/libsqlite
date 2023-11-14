#pragma once
#include "sqlite3.h"
#include "Debug.h"
#include "Handle.hpp"
#include <string>
#include "Crossplatfrom.h"


enum class Type
{
	Integer = SQLITE_INTEGER,
	Float = SQLITE_FLOAT,
	Blob = SQLITE_BLOB,
	Null = SQLITE_NULL,
	Text = SQLITE_TEXT,
};


struct Exception
{
	int Result = 0;
	std::string Message;

	explicit Exception(sqlite3* const connection) :
		Result(sqlite3_extended_errcode(connection)),
		Message(sqlite3_errmsg(connection))
	{
	}

};


class Connection
{

	struct ConnectionHandleTraits : HandleTraits<sqlite3*>
	{
		static void Close(Type value) noexcept
		{
			 sqlite3_close(value);
		}

	};

	using ConnectionHandle = Handle<ConnectionHandleTraits>;

	ConnectionHandle m_handle;

private:

	template<typename F, typename C>
	void internalOpen(F open_func, C const* const filename)
	{
		Connection temp;
		if (SQLITE_OK != open_func(filename, temp.m_handle.set()))
		{
			temp.throwLastError();
		}
		swap(m_handle, temp.m_handle);
	}

public:
	Connection() noexcept = default;

	template<typename C>
	explicit Connection(C const* const filename)
	{
		open(filename);
	}

	static Connection Memory()
	{
		return Connection(":memory:");
	}

	static Connection WideMemory()
	{
		return Connection(L":memory:");
	}

	explicit operator bool() const noexcept
	{
		return static_cast<bool>(m_handle);
	}

	void open(char const* const filename)
	{
		internalOpen(sqlite3_open, filename);
	}

	void open(wchar_t const* const filename)
	{
		internalOpen(sqlite3_open16, filename);

	}

	long long rowId() const noexcept
	{
		return sqlite3_last_insert_rowid(getAbi());
	}


	sqlite3* getAbi() const noexcept
	{
		return m_handle.get();
	}


	NORETURN_ATTRIBUTE void throwLastError() const
	{
		throw Exception(getAbi());
	}

	template<typename F>
	void profile(F callback, void* const context = nullptr)
	{
		sqlite3_profile(getAbi(), callback, context);
	}


};


template<typename T>
struct Reader // curisoly recuring template 
{
	int getInt(int const column = 0) const noexcept
	{
		return sqlite3_column_int(static_cast<T const*>(this)->getAbi(), column);
	}

	char const* getString(int const column = 0)const noexcept
	{
		return reinterpret_cast<char const*>(
			sqlite3_column_text(static_cast<T const*>(this)->getAbi(), column));
	}

	wchar_t const* getWideString(int const column = 0)const noexcept
	{
		return static_cast<wchar_t const*>(
			sqlite3_column_text16(static_cast<T const*>(this)->getAbi(), column));
	}


	int getStringLength(int const column = 0) const noexcept
	{
		return sqlite3_column_bytes(static_cast<T const*>(this)->getAbi(), column);
	}


	int getWideStringLength(int const column = 0) const noexcept
	{
		return sqlite3_column_bytes16(
			static_cast<T const*>(this)->getAbi(), column) / sizeof(wchar_t);
	}

	Type getType(int const column = 0) const noexcept
	{
		return static_cast<Type>(sqlite3_column_type(static_cast<T const*>(this)->getAbi(), column));
	}
};

class Row : public Reader<Row>
{
	sqlite3_stmt* m_statement = nullptr;
public:
	sqlite3_stmt* getAbi() const noexcept
	{
		return m_statement;
	}


	Row(sqlite3_stmt* const statement) noexcept :
		m_statement(statement)
	{
	}
};

class Statement : public Reader<Statement>   

{
	struct StatementHandleTraits : HandleTraits<sqlite3_stmt*>
	{
		static void Close(Type value) noexcept
		{
			VERIFY_(SQLITE_OK, sqlite3_finalize(value));
		}

	};

	using StatementHandle = Handle<StatementHandleTraits>;
	StatementHandle m_handle;


private:


	template<typename F, typename C, typename ...Values>
	void internalPrepare(
		Connection const& connection,
		F prepare,
		C const* const text,
		Values && ... values)
	{
		ASSERT(connection);
		if (SQLITE_OK != prepare(connection.getAbi(), text, -1, m_handle.set(), nullptr))
		{
			connection.throwLastError();
		}

		bindAll(std::forward<Values>(values)...);
	}

	void internalBind(int) const noexcept
	{

	}

	template <typename First, typename ...Rest>
	void internalBind(int const index, First&& first, Rest && ... rest) const
	{
		bind(index, std::forward<First>(first));
		internalBind(index + 1, std::forward<Rest>(rest) ...);
	}


public:

	Statement() noexcept = default;

	template<typename C, typename ...Values>
	Statement(
		Connection const& connection,
		C const* const text,
		Values && ... values)
	{
		prepare(connection, text, std::forward<Values>(values) ...);
	}


	explicit operator bool() const noexcept
	{
		return static_cast<bool>(m_handle);
	}

	sqlite3_stmt* getAbi() const noexcept
	{
		return m_handle.get();
	}

	NORETURN_ATTRIBUTE void throwLastError() const
	{
		throw Exception(sqlite3_db_handle(getAbi()));
	}


	template<typename ...Values>
	void prepare(
		Connection const& connection, 
		char const* const text,
		Values && ... values)
	{
		internalPrepare(connection, sqlite3_prepare_v2, text, std::forward<Values>(values) ...);
	}



	template<typename ...Values>
	void prepare(
		Connection const& connection,
		wchar_t const* const text,
		Values && ... values)
	{
		internalPrepare(connection, sqlite3_prepare16_v2, text, std::forward<Values>(values) ...);
	}

	bool step() const
	{
		int const result = sqlite3_step(getAbi());
		if (result == SQLITE_ROW) return true;
		if (result == SQLITE_DONE) return false;

		// todo locks considerations are to managed
		throwLastError();

	}

	void execute() const
	{
		VERIFY(!step());
	}

	void bind(int const index, int const value) const
	{
		if (SQLITE_OK != sqlite3_bind_int(getAbi(), index, value))
		{
			throwLastError();
		}
	}


	void bind(int const index, char const* const val, int const size = -1) const
	{
		if (SQLITE_OK != sqlite3_bind_text(getAbi(), index, val, size, SQLITE_STATIC))
		{
			throwLastError();
		}

	}

		// size is in bytes
	void bind(int const index, wchar_t const* const val, int const size = -1) const
	{
		if (SQLITE_OK != sqlite3_bind_text16(getAbi(), index, val, size, SQLITE_STATIC))
		{
			throwLastError();
		}

	}

	void bind(int const index, std::string const& value) const
	{
		bind(index, value.c_str(), value.size());
	}

	void bind(int const index, std::wstring const& value) const
	{
		bind(index, value.c_str(), value.size() * sizeof(wchar_t));
	}

	void bind(int const index, std::string&& value) const
	{
		if (SQLITE_OK != sqlite3_bind_text(getAbi(), index, value.c_str(), value.size(), SQLITE_TRANSIENT))
		{
			throwLastError();
		}
	}
	
	void bind(int const index, std::wstring&& value) const
	{
		if (SQLITE_OK != sqlite3_bind_text16(getAbi(), index, value.c_str(), value.size() * sizeof(wchar_t), SQLITE_TRANSIENT))
		{
			throwLastError();
		}
	}

	template <typename ...V>
	void bindAll(V && ... values) const
	{
		internalBind(1, std::forward<V>(values) ...);
	}


	template<typename ...V>
	void reset(V && ... values) const
	{
		if (SQLITE_OK != sqlite3_reset(getAbi()))
		{
			throwLastError();

		}

		bindAll(values ...);
	}
};

class RowIterator
{
	Statement const* m_statement = nullptr;
public:
	RowIterator() noexcept = default;

	RowIterator(Statement const& statement) noexcept
	{
		if (statement.step())
		{
			m_statement = &statement;
		}
	}

	RowIterator& operator++() noexcept
	{
		if (!m_statement->step())
		{
			m_statement = nullptr;
		}
		return *this;
	}

	bool operator!=(RowIterator other) noexcept
	{
		return m_statement != other.m_statement;
	}

	Row operator*() const noexcept
	{
		return Row(m_statement->getAbi());
	}

};


inline RowIterator begin(Statement const& statement) noexcept
{
	return RowIterator(statement);
}

inline RowIterator end(Statement const& statement) noexcept
{
	return RowIterator();
}



template<typename C, typename ... Values>
void execute(
	Connection const& connection,
	C const* const text,
	Values && ... values)
{
	Statement(connection, text, std::forward<Values>(values)...).execute();
}

