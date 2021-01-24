// MonoTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
using namespace std;


//C++的类定义

class Component
{
public:
	virtual int id() const { return 0; };
	int value;
};

class Transform: public Component
{
public:
	int id() const { return 1; }
};

class GameObject
{
public:
	vector<Component *> components;
};
class Scene
{
public:
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

//Global
Scene mainScene;
MonoDomain *domain;

MonoObject * MonoCSharp_Scene_GetMainScene()
{
	MonoObject * scene = mono_object_new(domain, monoclassScene);
	mono_runtime_object_init(scene);
	void* handle = &mainScene;
	mono_field_set_value(scene, monofieldSceneHandle, &handle);
	return scene;
}

MonoArray* MonoCSharp_Scene_GetGameObjects()
{
	MonoArray* array = mono_array_new(domain, monoclassGameObject, mainScene.gameObjects.size());

	for (size_t i = 0; i < mainScene.gameObjects.size(); i++)
	{
		MonoObject * obj = mono_object_new(domain, monoclassGameObject);
		mono_runtime_object_init(obj);
		void* handle = mainScene.gameObjects[i];
		mono_field_set_value(obj, monofieldGameObjectHandle, &handle);
		mono_array_set(array, MonoObject *, i, obj);
	}

	return array;
}

int MonoCSharp_GameObject_get_ComponentCount(const GameObject * gameObject)
{
	return (int)(gameObject->components.size());
}

MonoArray * MonoCSharp_GameObject_get_components(MonoObject* objPtr)
{
	GameObject * gameobject;
	mono_field_get_value(objPtr, monofieldGameObjectHandle, reinterpret_cast<void *>(&gameobject));

	MonoArray* array = mono_array_new(domain, monoclassComponent, gameobject->components.size());

	for (size_t i = 0; i < gameobject->components.size(); ++i)
	{
		MonoObject* com = mono_object_new(domain, monoclassComponent);
		mono_runtime_object_init(com);
		void* handle = gameobject->components[i];//指针准确
		mono_field_set_value(com, monofieldComponentHandle, &handle);
		mono_array_set(array, MonoObject*, i, com);
	}

	return array;
}

int MonoCSharp_Component_get_ID(MonoObject * objPtr)
{
	Component * component;
	mono_field_get_value(objPtr, monofieldComponentHandle, reinterpret_cast<void *>(&component));

	return component->id();
}

int MonoCSharp_Transform_get_ID(MonoObject * objPtr)
{
	Transform * transform;
	mono_field_get_value(objPtr, monofieldTransformHandle, reinterpret_cast<void *>(&transform));

	return transform->id();
}



int main()
{
	// Program.cs所编译dll所在的位置
	const char* path = "C:/Users/dell-pc/Desktop/Test/MonoC++Test/MonoCSharp/MonoCSharp/Program.dll";

	//---获取应用域---
	domain = mono_jit_init("Test");

	//-----运行时配置-----
	//mono_set_dirs("C://Program Files (x86)//Mono//lib", "C://Program Files (x86)//Mono//etc");
	//mono_config_parse(NULL);

	//---加载程序集---
	MonoAssembly* assembly = mono_domain_assembly_open(domain, path);
	MonoImage* image = mono_assembly_get_image(assembly);

	monoclassScene = mono_class_from_name(image, "MonoCSharp", "Scene");
	monofieldSceneHandle = mono_class_get_field_from_name(monoclassScene, "handle_scene");
	monoclassGameObject = mono_class_from_name(image, "MonoCSharp", "GameObject");
	monofieldGameObjectHandle = mono_class_get_field_from_name(monoclassGameObject, "handle_gameobj");
	monoclassComponent = mono_class_from_name(image, "MonoCSharp", "Component");
	monofieldComponentHandle = mono_class_get_field_from_name(monoclassComponent, "handle_com");
	monoclassTransform = mono_class_from_name(image, "MonoCSharp", "Transform");
	monofieldTransformHandle = mono_class_get_field_from_name(monoclassTransform, "handle_transform");

	// =====================================================测试

	for (int i = 0; i < 6; i++)//233, 234,235
	{
		Transform * transform = new Transform();
		transform->value = 233;
		Component * component = new Component();
		component->value = 2333;
		GameObject * newObj = new GameObject();
		newObj->components.push_back(transform);
		newObj->components.push_back(component);
		mainScene.gameObjects.push_back(newObj);

		cout << "transform:id():" << transform->id() << endl;
		cout << "component:id():" << component->id() << endl;
		cout << "Transform组件指针：" << (unsigned int)transform << endl;
		cout << "Component组件指针：" << (unsigned int)component << endl;
	
		cout << "-------------" << endl;
	}
	//======================================================交互测试

	mono_add_internal_call("MonoCSharp.Scene::GetMainScene()", reinterpret_cast<void*>(MonoCSharp_Scene_GetMainScene));
	mono_add_internal_call("MonoCSharp.Scene::GetGameObjects()", reinterpret_cast<void*>(MonoCSharp_Scene_GetGameObjects));
	mono_add_internal_call("MonoCSharp.GameObject::get_ComponentCount", reinterpret_cast<void*>(MonoCSharp_GameObject_get_ComponentCount));
	mono_add_internal_call("MonoCSharp.GameObject::get_components", reinterpret_cast<void*>(MonoCSharp_GameObject_get_components));
	mono_add_internal_call("MonoCSharp.Component::get_ID", reinterpret_cast<void*>(MonoCSharp_Component_get_ID));
	mono_add_internal_call("MonoCSharp.Transform::get_ID", reinterpret_cast<void*>(MonoCSharp_Transform_get_ID));

	MonoClass* monoclassTestClass = mono_class_from_name(image, "MonoCSharp", "TestClass");

	const bool include_namespace = true;
	MonoMethodDesc* method_desc = mono_method_desc_new("MonoCSharp.TestClass:Test()", include_namespace);
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
