#include "EventSystem.hpp"

EventSystem* g_theEventSystem = nullptr;

void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	SubscriptionList& subscribersForThisEvent = m_subscriptionListByEventName[eventName];
	subscribersForThisEvent.push_back(functionPtr);
}

void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	SubscriptionList& subscribersForThisEvent = m_subscriptionListByEventName[eventName];
	for (int subscriberIndex = 0; subscriberIndex < (int)subscribersForThisEvent.size(); ++subscriberIndex)
	{
		if (subscribersForThisEvent[subscriberIndex] == functionPtr)
		{
			subscribersForThisEvent[subscriberIndex] = nullptr;
		}
	}
}

void EventSystem::UnsubscribeFromAllEvents(EventCallbackFunction functionPtr)
{
	std::map<std::string, SubscriptionList>::iterator eventIter;
	for (eventIter = m_subscriptionListByEventName.begin(); eventIter != m_subscriptionListByEventName.end(); ++eventIter)
	{
		std::string const& eventName = eventIter->first;
		UnsubscribeEventCallbackFunction(eventName, functionPtr);
	}
}

void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	SubscriptionList& subscribersForThisEvent = m_subscriptionListByEventName[eventName];
	for (int subscriberIndex = 0; subscriberIndex < (int)subscribersForThisEvent.size(); ++subscriberIndex)
	{
		EventCallbackFunction callbackFunctionPtr = subscribersForThisEvent[subscriberIndex];
		if (callbackFunctionPtr != nullptr)
		{
			bool wasConsumed = callbackFunctionPtr(args);
			if (wasConsumed)
			{
				break;
			}
		}
		else
		{
			// Print error message
		}
	}
}

void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs emptyArgs;
	FireEvent(eventName, emptyArgs);
}

void EventSystem::GetAllRegisteredCommands(std::vector<std::string>& out_registeredCommandList)
{
	// for iterator and then use .first()
	std::map<std::string, SubscriptionList>::iterator eventIter;
	for (eventIter = m_subscriptionListByEventName.begin(); eventIter != m_subscriptionListByEventName.end(); ++eventIter)
	{
		out_registeredCommandList.push_back(eventIter->first);
	}
}

void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	g_theEventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
}

void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);
}

void UnsubscribeFromAllEvents(EventCallbackFunction functionPtr)
{
	g_theEventSystem->UnsubscribeFromAllEvents(functionPtr);
}

void FireEvent(std::string const& eventName, EventArgs& args)
{
	g_theEventSystem->FireEvent(eventName, args);
}

void FireEvent(std::string const& eventName)
{
	g_theEventSystem->FireEvent(eventName);
}
