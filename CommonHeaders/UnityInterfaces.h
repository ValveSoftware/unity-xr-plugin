#pragma once

#include "ProviderInterface/IUnityInterface.h"
#include "Singleton.h"

#include <cassert>


class UnityInterfaces : public Singleton<UnityInterfaces>
{
public:
	UnityInterfaces();
	~UnityInterfaces();

	void SetUnityInterfaces( IUnityInterfaces *interfaces ) { m_UnityInterfaces = interfaces; }

	template<typename T>
	T *GetInterface()
	{
		assert( m_UnityInterfaces );
		return static_cast< T * >( m_UnityInterfaces->GetInterface( GetUnityInterfaceGUID<T>() ) );
	}
private:

	IUnityInterfaces *m_UnityInterfaces;
};
