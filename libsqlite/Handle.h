#pragma once
#include "Debug.h"

template<typename T>
struct HandleTraits
{
	using Type = T;

	static Type Invalid() noexcept
	{
		return nullptr;
	}

	// static void Close(Type value) noexcept
};


template<typename T>
class Handle
{
	using Type = decltype(T::Invalid());
	Type m_value;
private:

	void close() noexcept
	{
		if (*this)
		{
			T::Close(m_value);

		}
	}


public:

	Handle(Handle const&) = delete;
	Handle& operator=(Handle const&) = delete;

	explicit Handle(Type value = T::Invalid())noexcept :
		m_value(value)
	{
	}


	Handle(Handle&& other) noexcept :
		m_value(other.detach())
	{
	}


	Handle& operator=(Handle&& other) noexcept
	{
		if (this != &other)
		{
			reset(other.detach());
		}
		return *this;
	}

	~Handle() noexcept
	{
		close();
	}

	explicit operator bool() const noexcept
	{
		return m_value != T::Invalid();
	}


	Type get() const noexcept
	{
		return m_value;
	}

	Type * set() noexcept
	{
		ASSERT(!*this);
		return &m_value;
	}

	Type detach() noexcept
	{
		Type value = m_value;
		m_value = T::Invalid();
		return value;
	}

	bool reset(Type value = T::Invalid()) noexcept
	{
		if (m_value != value)
		{
			close();
			m_value = value;
		}
		return static_cast<bool>(*this);
	}

	void swap(Handle<T>& other) noexcept
	{
		Type temp = m_value;
		m_value = other.m_value;
		other.m_value = temp;
	}

};

template<typename T>
void swap(Handle<T>& left, Handle<T>& right)noexcept
{
	left.swap(right);
}


template<typename T>
bool operator==(Handle<T>& const left, Handle<T>& const right)noexcept
{
	return left.get() == right.get();
}



template<typename T>
bool operator!=(Handle<T>& const left, Handle<T>& const right)noexcept
{
	return !(left == right);
}