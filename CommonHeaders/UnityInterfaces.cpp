#include "ProviderInterface/IUnityXRDisplay.h"
#include "UnityInterfaces.h"


UnityInterfaces::UnityInterfaces() : m_UnityInterfaces( nullptr )
{
}


UnityInterfaces::~UnityInterfaces()
{
	m_UnityInterfaces = nullptr;
}
