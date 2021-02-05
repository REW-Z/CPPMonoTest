// MonoTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <regex>
#include <cassert>
#include <io.h>
#include <vector>
#include <list>
#include <string>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
using namespace std;
//前向声明

class Object;
class GameObject;
class Component;
class Transfrom;
struct vec3;


//Assembly
MonoDomain * domain;
MonoImage * image_core;
MonoImage * image_scripts;


void getAllFilePath(std::string path, regex flag, vector<string>& files)
{
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (regex_search(fileinfo.name, flag))
			{
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

//结构体定义
struct vec3
{
public:

	float x;
	float y;
	float z;
	vec3() { x = 0; y = 0; z = 0; }
	vec3(float newx, float newy, float newz) { x = newx; y = newy; z = newz; }
};


//C++的类定义

class Object
{
public:
	virtual int tid() const { return 0; }
};

class Component:public Object
{
public:
	virtual int tid() const { return 2; }

	GameObject * gameObject;

	Component() { this->gameObject = nullptr; }
};

class Transform: public Component
{
public:
	int tid() const { return 3; }
	vec3 position;
	vec3 rotation;
	vec3 scale;
	Transform() { position = vec3(); rotation = vec3(); scale = vec3(1.0f, 1.0f, 1.0f); }
};

class MonoScript : public Component
{
public:
	int tid() const { return 4; }
	MonoObject * objInstance;
	MonoScript(const char * name)
	{
		this->objInstance = nullptr;

		MonoClass * monoclassCurrentScript = mono_class_from_name(image_scripts, "MonoCSharp", name);
		this->objInstance = mono_object_new(domain, monoclassCurrentScript);

		MonoClassField * field = NULL;
		void * itor = NULL;

		while (field = mono_class_get_fields(monoclassCurrentScript, &itor))
		{
			const char * field_name = mono_field_get_name(field);
			MonoType * type = mono_field_get_type(field);
			MonoTypeEnum typeEnum = static_cast<MonoTypeEnum> (mono_type_get_type(type));

			switch (typeEnum)
			{
			case MONO_TYPE_OBJECT:
			{
				MonoClass * monoclassTmp = mono_class_from_mono_type(type);
				MonoObject * obj = mono_object_new(domain, monoclassTmp);
				mono_field_set_value(objInstance, field, &obj);
				break;
			}
			case MONO_TYPE_I4:
			{
				int value = 0;
				mono_field_set_value(objInstance, field, &value);
				break;
			}
			default:
				break;
			}

		}
	}
};

class GameObject :public Object
{
public:
	int tid() const { return 1; }
	const char * name;
	vector<Component *> components;
};
class Scene
{
public:
	int tid() const { return 999; }
	vector<GameObject *> gameObjects;
};





//MonoClasses
MonoClass * monoclassScene;
MonoClassField * monofieldSceneHandle;

MonoClass * monoclassGameObject;
MonoClassField * monofieldGameObjectHandle;

MonoClass * monoclassComponent;
MonoClassField * monofieldComponentHandle;

MonoClass * monoclassTransform;
MonoClassField * monofieldTransformHandle;

MonoClass * monoclassMonoScript;
MonoClassField * monofieldMonoScriptHandle;

MonoClass * monoclassVec3;
MonoClassField * monofieldVec3x;
MonoClassField * monofieldVec3y;
MonoClassField * monofieldVec3z;

//Global
Scene mainScene;


//Internal Calls
void MonoCSharp_Object_Destroy(Object * obj)
{
	cout << "try to del.." << (unsigned int)obj  << endl;

	switch (obj->tid())
	{
	case 1://gameObject
	{
		GameObject * gameObj = reinterpret_cast<GameObject *>(obj);
		for (size_t i = 0; i < gameObj->components.size(); i++)
		{
			delete gameObj->components[i];
		}
		break;
	}
	case 2://component
	case 3:
	{
		cout << "is component" << endl;
		Component * com = reinterpret_cast<Component *>(obj);
		GameObject * ownerGameObj = com->gameObject;
		if (ownerGameObj != nullptr)
		{
			for (vector<Component *>::iterator it = ownerGameObj->components.begin(); it != ownerGameObj->components.end();)
			{
				cout << "it:::" << (unsigned int)*it << endl;
				cout << "com::::" << (unsigned int)com << endl;
				if (*it == com)
				{
					ownerGameObj->components.erase(it);
					break;
				}
				it++;
			}
			cout << "C++ comsize:" << ownerGameObj->components.size() << endl;
		}
		delete com;
		break;
	}
	default:
		break;
	}
}

MonoObject * MonoCSharp_Scene_GetMainScene()
{
	MonoObject * mobjScene = mono_object_new(domain, monoclassScene);
	mono_runtime_object_init(mobjScene);//调用构造函数.ctor
	void* handle = &mainScene;
	mono_field_set_value(mobjScene, monofieldSceneHandle, &handle);
	return  mobjScene;
}

MonoArray* MonoCSharp_Scene_GetGameObjects()
{
	MonoArray* array = mono_array_new(domain, monoclassGameObject, mainScene.gameObjects.size());

	for (size_t i = 0; i < mainScene.gameObjects.size(); i++)
	{
		MonoObject * obj = mono_object_new(domain, monoclassGameObject);
		//mono_runtime_object_init(obj);
		void* handle = mainScene.gameObjects[i];
		mono_field_set_value(obj, monofieldGameObjectHandle, &handle);
		mono_array_set(array, MonoObject *, i, obj);
	}

	return array;
}

MonoString * MonoCSharp_GameObject_get_Name(const GameObject * gameObject)
{
	const char * str = gameObject->name;
	MonoString * monostr = mono_string_new(domain, str);
	return monostr;
}

void MonoCSharp_GameObject_set_Name(GameObject * gameObject, MonoString * monostr)
{
	gameObject->name = mono_string_to_utf8(monostr);
}

int MonoCSharp_GameObject_get_ComponentCount(const GameObject * gameObject)
{
	return (int)(gameObject->components.size());
}

MonoArray * MonoCSharp_GameObject_get_Components(GameObject * gameobject)
{
	/*GameObject * gameobject;
	mono_field_get_value(objPtr, monofieldGameObjectHandle, reinterpret_cast<void *>(&gameobject));*/

	MonoArray* array = mono_array_new(domain, monoclassComponent, gameobject->components.size());

	for (size_t i = 0; i < gameobject->components.size(); ++i)
	{
		if (typeid(*(gameobject->components[i])) == typeid(Transform))
		{
			MonoObject* com = mono_object_new(domain, monoclassTransform);
			//mono_runtime_object_init(com);
			void* handle = gameobject->components[i];
			mono_field_set_value(com, monofieldTransformHandle, &handle);
			mono_array_set(array, MonoObject*, i, com);
		}
		else if (typeid(*(gameobject->components[i])) == typeid(MonoScript))
		{
			MonoObject * com = mono_object_new(domain, monoclassMonoScript);
			MonoObject * objInstance = reinterpret_cast<MonoScript *>(gameobject->components[i])->objInstance;

			void* handle = gameobject->components[i];
			mono_field_set_value(objInstance, monofieldMonoScriptHandle, &handle);
			mono_array_set(array, MonoObject*, i, objInstance);
		}
		else if (typeid(*(gameobject->components[i])) == typeid(Component))
		{
			MonoObject* com = mono_object_new(domain, monoclassComponent);
			//mono_runtime_object_init(com);
			void* handle = gameobject->components[i];
			mono_field_set_value(com, monofieldComponentHandle, &handle);
			mono_array_set(array, MonoObject*, i, com);
		}
	}

	return array;
}

void MonoCSharp_GameObject_AddComponent(GameObject * gameObj, Component * com)
{
	gameObj->components.push_back(com);
	com->gameObject = gameObj;
}

int MonoCSharp_Component_get_TID(MonoObject * objPtr)
{
	Component * component;
	mono_field_get_value(objPtr, monofieldComponentHandle, reinterpret_cast<void *>(&component));
	return component->tid();
}

void * MonoCSharp_Component_Construct()
{
	Transform * com = new Transform();
	return reinterpret_cast<void *>(com);
}

int MonoCSharp_Transform_get_TID(MonoObject * objPtr)
{
	Transform * transform;
	mono_field_get_value(objPtr, monofieldTransformHandle, reinterpret_cast<void *>(&transform));

	return transform->tid();
}

vec3 MonoCSharp_Transform_get_Position(Transform * transformPtr)
{
	return transformPtr->position;
}

void MonoCSharp_Transform_set_Position(Transform * transformPtr, vec3 pos)
{
	transformPtr->position = pos;
}

vec3 MonoCSharp_Transform_get_Rotation(Transform * transformPtr)
{
	return transformPtr->rotation;
}

void MonoCSharp_Transform_set_Rotation(Transform * transformPtr, vec3 rot)
{
	transformPtr->rotation = rot;
}

vec3 MonoCSharp_Transform_get_Scale(Transform * transformPtr)
{
	return transformPtr->scale;
}

void MonoCSharp_Transform_set_Scale(Transform * transformPtr, vec3 scale)
{
	transformPtr->scale = scale;
}



int main()
{
	// Program.cs所编译dll所在的位置
	//const char * path1 = "C:/Users/dell-pc/Desktop/Test/MonoC++Test/MonoCSharp/MonoCSharp/MonoCSharp.dll";
	const char * path1 = "C:/Users/dell-pc/Desktop/Test/MonoC++Test/MonoCSharp/MonoCSharp/Assembly_Engine.dll";
	const char * path2 = "C:/Users/dell-pc/Desktop/Test/MonoC++Test/MonoCSharp/MonoCSharp/Assembly_Scripts.dll";

	//---获取应用域---
	domain = mono_jit_init("Test");


	//-----运行时配置----- //不设置使用默认
	//mono_set_dirs("C:/Program Files (x86)/Mono/lib", "C:/Program Files (x86)/Mono/etc");
	//mono_config_parse(NULL);
	//mono_domain_set_config(...);
	mono_domain_set_config(domain, "C:/Users/dell-pc/Desktop/Test/MonoC++Test/MonoTest", "Config");
	
	//---加载主要程序集---
	MonoAssembly* assembly_engine = mono_domain_assembly_open(domain, path1);
	image_core = mono_assembly_get_image(assembly_engine);
	MonoAssembly * assembly_scripts = mono_domain_assembly_open(domain, path2);
	image_scripts = mono_assembly_get_image(assembly_scripts);

	monoclassScene = mono_class_from_name(image_core, "MonoCSharp", "Scene");
	monofieldSceneHandle = mono_class_get_field_from_name(monoclassScene, "handle");
	monoclassGameObject = mono_class_from_name(image_core, "MonoCSharp", "GameObject");
	monofieldGameObjectHandle = mono_class_get_field_from_name(monoclassGameObject, "handle");
	monoclassComponent = mono_class_from_name(image_core, "MonoCSharp", "Component");
	monofieldComponentHandle = mono_class_get_field_from_name(monoclassComponent, "handle");
	monoclassTransform = mono_class_from_name(image_core, "MonoCSharp", "Transform");
	monofieldTransformHandle = mono_class_get_field_from_name(monoclassTransform, "handle");
	monoclassMonoScript = mono_class_from_name(image_core, "MonoCSharp", "MonoScript");
	monofieldMonoScriptHandle = mono_class_get_field_from_name(monoclassMonoScript, "handle");

	monoclassVec3 = mono_class_from_name(image_core, "MonoCSharp", "Vec3");
	monofieldVec3x = mono_class_get_field_from_name(monoclassVec3, "x");
	monofieldVec3y = mono_class_get_field_from_name(monoclassVec3, "y");
	monofieldVec3z = mono_class_get_field_from_name(monoclassVec3, "z");




	////?????????????????????
	//const char* path1 = "C:/Users/dell-pc/Desktop/Test/MonoC++Test/MonoCSharp/MonoCSharp/TestScript.dll";
	//MonoAssembly* assembly1 = mono_domain_assembly_open(domain, path1);
	//MonoImage* image1 = mono_assembly_get_image(assembly1);

	//MonoClass * monoclassTestScript = mono_class_from_name(image1, "MonoCSharp", "TestScript");
	//const bool include_namespace1 = true;
	//MonoMethodDesc* method_test_desc = mono_method_desc_new("MonoCSharp.TestScript:Update()", include_namespace1);
	//MonoMethod* method_test = mono_method_desc_search_in_class(method_test_desc, monoclassTestScript);
	//mono_method_desc_free(method_test_desc);

	//MonoObject * obj1 = mono_object_new(domain, monoclassTestScript);
	//mono_runtime_object_init(obj1);
	//mono_runtime_invoke(method_test, &obj1, NULL, NULL);


	
	// =====================================================测试


	// =====================================================初始化物体

	cout << "*********************初始化场景*****************************" << endl;
	for (int i = 0; i < 3; i++)//233, 234,235
	{
		Transform * transform = new Transform();
		Component * component = new Component();
		MonoScript * script = new MonoScript("TestScript");
		GameObject * newObj = new GameObject();
		newObj->components.push_back(transform);
		newObj->components.push_back(component);
		newObj->components.push_back(script);
		newObj->name = "null";
		transform->gameObject = newObj;
		component->gameObject = newObj;
		mainScene.gameObjects.push_back(newObj);

		cout << "transform:id():" << transform->tid() << endl;
		cout << "component:id():" << component->tid() << endl;
		cout << "Transform组件指针：" << (unsigned int)transform << endl;
		cout << "Component组件指针：" << (unsigned int)component << endl;
	
		cout << "----游戏对象添加完毕----" << endl;
	}
	cout << "**************************************************************" << endl;
	//======================================================交互测试

	mono_add_internal_call("MonoCSharp.Scene::GetMainScene()", reinterpret_cast<void*>(MonoCSharp_Scene_GetMainScene));
	mono_add_internal_call("MonoCSharp.Object::DestroyViaPtr", reinterpret_cast<void*>(MonoCSharp_Object_Destroy));
	mono_add_internal_call("MonoCSharp.Scene::GetGameObjects()", reinterpret_cast<void*>(MonoCSharp_Scene_GetGameObjects));
	mono_add_internal_call("MonoCSharp.GameObject::get_Name", reinterpret_cast<void*>(MonoCSharp_GameObject_get_Name));
	mono_add_internal_call("MonoCSharp.GameObject::set_Name", reinterpret_cast<void*>(MonoCSharp_GameObject_set_Name));
	mono_add_internal_call("MonoCSharp.GameObject::get_ComponentCount", reinterpret_cast<void*>(MonoCSharp_GameObject_get_ComponentCount));
	mono_add_internal_call("MonoCSharp.GameObject::get_Components", reinterpret_cast<void*>(MonoCSharp_GameObject_get_Components));
	mono_add_internal_call("MonoCSharp.GameObject::AddComponentViaPtr", reinterpret_cast<void*>(MonoCSharp_GameObject_AddComponent));
	mono_add_internal_call("MonoCSharp.Component::get_TID", reinterpret_cast<void*>(MonoCSharp_Component_get_TID));
	mono_add_internal_call("MonoCSharp.Component::ComponentConstruct", reinterpret_cast<void*>(MonoCSharp_Component_Construct));
	mono_add_internal_call("MonoCSharp.Transform::get_TID", reinterpret_cast<void*>(MonoCSharp_Transform_get_TID));
	mono_add_internal_call("MonoCSharp.Transform::get_Position", reinterpret_cast<void*>(MonoCSharp_Transform_get_Position));
	mono_add_internal_call("MonoCSharp.Transform::set_Position", reinterpret_cast<void*>(MonoCSharp_Transform_set_Position));
	mono_add_internal_call("MonoCSharp.Transform::get_Rotation", reinterpret_cast<void*>(MonoCSharp_Transform_get_Rotation));
	mono_add_internal_call("MonoCSharp.Transform::set_Rotation", reinterpret_cast<void*>(MonoCSharp_Transform_set_Rotation));
	mono_add_internal_call("MonoCSharp.Transform::get_Scale", reinterpret_cast<void*>(MonoCSharp_Transform_get_Scale));
	mono_add_internal_call("MonoCSharp.Transform::set_Scale", reinterpret_cast<void*>(MonoCSharp_Transform_set_Scale));

	MonoClass* monoclassTestClass = mono_class_from_name(image_core, "MonoCSharp", "TestClass");

	const bool include_namespace = true;
	MonoMethodDesc* method_desc = mono_method_desc_new("MonoCSharp.TestClass:Start()", include_namespace);
	MonoMethod* method = mono_method_desc_search_in_class(method_desc, monoclassTestClass);
	mono_method_desc_free(method_desc);

	mono_runtime_invoke(method, NULL, NULL, NULL);



	//======================================================END
	//释放应用域
	mono_jit_cleanup(domain);
	return 0;


}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
