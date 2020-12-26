#pragma once
/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBlackboard.h: Blackboard implementation
/*=============================================================================*/

//Includes
#include <unordered_map>
//#include "../stdafx.h"

namespace Elite
{
	//-----------------------------------------------------------------
	// BLACKBOARD TYPES (BASE)
	//-----------------------------------------------------------------
	class IBlackBoardField
	{
	public:
		IBlackBoardField() = default;
		virtual ~IBlackBoardField() = default;
	};

	//BlackboardField does not take ownership of pointers whatsoever!
	template<typename T>
	class BlackboardField : public IBlackBoardField
	{
	public:
		explicit BlackboardField(T data) : m_Data(data)
		{}
		T GetData() { return m_Data; };
		void SetData(T data) { m_Data = data; }

	private:
		T m_Data;
	};

	//-----------------------------------------------------------------
	// BLACKBOARD (BASE)
	//-----------------------------------------------------------------
	class Blackboard final
	{
	public:
		~Blackboard()
		{
			for (auto el : m_BlackboardData)
				//SAFE_DELETE(el.second);
			{
				if (el.second)
					delete el.second;
			}
			m_BlackboardData.clear();
		}

		//Add data to the blackboard
		template<typename T> bool AddData(const std::string& name, T data)
		{
			auto it = m_BlackboardData.find(name);
			if (it == m_BlackboardData.end())
			{
				m_BlackboardData[name] = new BlackboardField<T>(data);
				return true;
			}
			printf("WARNING: Data '%s' of type '%s' already in Blackboard \n", name.c_str(), typeid(T).name());
			return false;
		}

		//Change the data of the blackboard
		template<typename T> bool ChangeData(const std::string& name, T data)
		{
			auto it = m_BlackboardData.find(name);
			if (it != m_BlackboardData.end())
			{
				BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(m_BlackboardData[name]);
				if (p)
				{
					p->SetData(data);
					return true;
				}
			}
			printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
			return false;
		}

		//Get the data from the blackboard
		template<typename T> bool GetData(const std::string& name, T& data)
		{
			BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(m_BlackboardData[name]);
			if (p != nullptr)
			{
				data = p->GetData();
				return true;
			}
			printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
			return false;
		}

		//Custom: Get a ptr to data from the blackboard (ptr obj seems to break in range-based for-loops)
		template<typename T> bool GetDataPtr(const std::string& name, T* &pPtr)
		{
			if (m_BlackboardData[name])
			{
				//pPtr now points to adres of data from BlackboardField<T>*
				pPtr = &dynamic_cast<BlackboardField<T>*>(m_BlackboardData[name])->GetData();
				return true;
			}

			//TODO: return T* upon success or nullptr upon fail?

			//BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(m_BlackboardData[name]);
			//if (p != nullptr)
			//{
			//	data = p->GetData();
			//	return true;
			//}
			printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
			return false;
		}

	private:
		std::unordered_map<std::string, IBlackBoardField*> m_BlackboardData;
	};
}