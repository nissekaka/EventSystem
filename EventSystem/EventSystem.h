#pragma once

#include <typeindex>
#include <unordered_map>
#include <vector>

namespace EventSystem
{
	// Event System by Anton Eriksson
	// -----------------------------------------
	// Uses a raw pointer to an unordered_map to
	// give the user control over the allocated
	// memory. When used correctly, it should not
	// result in any memory leaks!

	// Note on unordered_map:
	// unordered_map uses a hash function to map
	// the type of the event to the corresponding
	// observer vector, which can be faster than
	// traversing a vector or a map

	// Event base
	struct Event
	{
		virtual ~Event() = default;
	};

	// Observer base
	class Observer
	{
	public:
		virtual ~Observer() = default;

		virtual void OnNotify(Event& anEvent) = 0;
		virtual void OnInit() = 0;
		virtual void OnDestroy() = 0;

		template <class EventClass>
		void OnNotify(EventClass& anEvent)
		{
			OnNotify(static_cast<Event&>(anEvent));
		}

	protected:
		bool myIsActive{};
	};

	// Subject
	class Subject
	{
	public:
		// Attach an Observer to an Event (a subclass to Event)
		// The Observer will be notified when this Event is called
		template <class EventClass>
		static void Attach(Observer* anObserver)
		{
			// If myObservers has no Observers attached to it
			// it needs to be initialized in order to be able
			// to hold any data
			if (myObservers == nullptr)
			{
				myObservers = new std::unordered_map<std::type_index, std::vector<Observer*>>();
			}

			// Get the vector in myObservers at the given key
			// _typeid_ determines the type that the Observer expects
			std::vector<Observer*>& observers = (*myObservers)[typeid(EventClass)];
			// Check if Observer is already attached to this event
			const std::vector<Observer*>::iterator it = std::find(observers.begin(), observers.end(), anObserver);
			if (it != observers.end())
			{
#ifdef _DEBUG
				printf("\n-------------------------------------------------------------------");
				printf("\nObserver is already attached to event in " __FUNCSIG__);
				printf("\nThis means you have probably missed calling Detach on the observer!");
				printf("\n-------------------------------------------------------------------");
#endif
				return;
			}
			// Adds observer to the vector for the type is expects
			observers.push_back(anObserver);
		}

		// Detach an Observer from an Event, it will no longer be notified
		template <class EventClass>
		static void Detach(Observer* anObserver)
		{
			if (myObservers == nullptr)
			{
#ifdef _DEBUG
				printf("\nmyObservers is nullptr in " __FUNCSIG__);
#endif
				return;
			}

			// Get the vector in myObservers at the given key
			// _typeid_ determines the type that the Observer expects
			std::vector<Observer*>& observers = (*myObservers)[typeid(EventClass)];
			// Check if Observer is attached to this event
			const std::vector<Observer*>::iterator it = std::remove(observers.begin(), observers.end(), anObserver);
			if (it != observers.end())
			{
				observers.erase(it);
			}
			else
			{
#ifdef _DEBUG
				printf("\n-------------------------------------");
				printf("\nObserver of type %s", typeid(EventClass).name());
				printf(" could not be detached in " __FUNCSIG__);
				printf("\nThis may result in memory leaks!");
				printf("\n-------------------------------------");
#endif
			}

			// If the vector is empty, we remove its key from myObservers
			if (observers.empty())
			{
				myObservers->erase(typeid(EventClass));
			}

			// If the map is empty, deallocate its memory and reset its address
			if (myObservers->empty())
			{
				delete myObservers;
				myObservers = nullptr;
			}
		}

		// Notify all Observers (if any) that are attached to the Event
		template <class EventClass>
		static void Notify(EventClass& anEvent)
		{
			if (myObservers == nullptr)
			{
#ifdef _DEBUG
				printf("\nmyObservers is nullptr in " __FUNCSIG__);
#endif
				return;
			}

			// Get the type from anEvent
			const type_info& type = typeid(anEvent);
			// Check if anEvent is a key in myObservers
			const std::unordered_map<std::type_index, std::vector<Observer*>>::iterator it = myObservers->find(type);
			if (it != myObservers->end())
			{
				// Get the value from key-value pair (key = first, value = second)
				const std::vector<Observer*>& observers = it->second;
				// Notify all Observers for this event type
				for (Observer* observer : observers)
				{
					observer->OnNotify(anEvent);
				}
			}
			else
			{
				//#ifdef _DEBUG
				//				printf("\nNo observers attached to this event in " __FUNCSIG__);
				//#endif
			}
		}

	private:
		inline static std::unordered_map<std::type_index, std::vector<Observer*>>* myObservers = nullptr;
	};
}
