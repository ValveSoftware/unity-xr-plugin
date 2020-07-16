#pragma once

#include <memory>
#include <mutex>

template <typename T>
class Singleton
{
public:
	virtual ~Singleton() = default;
	static T &Get()
	{
		std::call_once( m_onceFlag,
			[]
			{
				m_Instance.reset( new T() );
			} );
		return *m_Instance.get();
	}

protected:
	static std::unique_ptr<T> m_Instance;

	static std::once_flag m_onceFlag;
	Singleton() = default;
	Singleton( const Singleton &src ) = delete;
	Singleton &operator=( const Singleton &rhs ) = delete;
};
template<typename T> std::once_flag Singleton<T>::m_onceFlag;
template<typename T> std::unique_ptr<T> Singleton<T>::m_Instance;
